"""AutoCam Raspberry local read-only proxy for Arduino Opta HTTP telemetry."""
import json
import os
from datetime import datetime, timezone
from urllib.error import HTTPError, URLError
from urllib.parse import quote
from urllib.request import Request, urlopen

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field
from fastapi.middleware.cors import CORSMiddleware

OPTA_STATUS_URL = os.getenv("OPTA_STATUS_URL", "http://192.168.50.2:8080/api/status")
TIMEOUT_SECONDS = 1.5

app = FastAPI(title="AutoCam Local API", version="0.2.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173", "http://127.0.0.1:5173",
                   "http://192.168.50.1:5173"],
    allow_credentials=False,
    allow_methods=["GET", "POST"],
    allow_headers=["*"],
)

def read_opta():
    request = Request(OPTA_STATUS_URL, headers={"Accept": "application/json",
                                                  "Connection": "close"})
    try:
        with urlopen(request, timeout=TIMEOUT_SECONDS) as response:
            if response.status != 200:
                raise ValueError(f"Unexpected HTTP status {response.status}")
            raw = response.read(4097)
            if len(raw) > 4096:
                raise ValueError("Oversized response")
            data = json.loads(raw)
            if not isinstance(data, dict):
                raise ValueError("Invalid JSON payload")
            required = ("level_percent", "litres", "setpoint_percent",
                        "pump_main", "pump_dosing_1", "fault")
            if not all(key in data for key in required):
                raise ValueError("Missing telemetry fields")
            return data
    except (HTTPError, URLError, TimeoutError, OSError, ValueError,
            json.JSONDecodeError) as exc:
        raise HTTPException(status_code=503,
                            detail=f"Opta telemetry unavailable: {type(exc).__name__}") from exc

@app.get("/api/health")
def health():
    try:
        read_opta()
    except HTTPException:
        return {"ok": False, "mode": "live", "opta": "offline"}
    return {"ok": True, "mode": "live", "opta": "connected"}

@app.get("/api/status")
def status():
    data = read_opta()
    return {
        "mode": "live",
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "opta": "connected",
        "level_percent": data["level_percent"],
        "litres": data["litres"],
        "setpoint_percent": data["setpoint_percent"],
        "pump_main": data["pump_main"],
        "pump_dosing_1": data["pump_dosing_1"],
        "fault": data["fault"],
    }

@app.post("/api/commands/{command}")
def commands_disabled(command: str):
    raise HTTPException(status_code=503,
                        detail="Control físico deshabilitado: dashboard solo lectura.")

# Preview-only formulation service. No commands are sent to the Opta.
TANK_CAPACITY_L = 20.0
EMPTY_TOLERANCE_L = 0.10  # provisional; must match PLC calibration

class DissolutionPlan(BaseModel):
    water_litres: float = Field(gt=0, le=TANK_CAPACITY_L)
    fertilizer_ml: float = Field(gt=0, le=120)

@app.post("/api/dissolutions/preview")
def preview_dissolution(plan: DissolutionPlan):
    # Water target is an absolute tank volume, NOT litres to add.
    import math
    if not math.isfinite(plan.water_litres) or not math.isfinite(plan.fertilizer_ml):
        raise HTTPException(422, "Cantidades no válidas")
    opta = read_opta()
    level = opta.get("litres")
    if type(level) not in (int, float) or not (0 <= level <= TANK_CAPACITY_L):
        raise HTTPException(409, "Lectura de volumen no válida; no se puede planificar la receta")
    if level > EMPTY_TOLERANCE_L:
        raise HTTPException(409, "Depósito no vacío: vaciar manualmente antes de preparar una receta")
    if plan.fertilizer_ml > 120:  # 1 ml/s provisional, máximo 120 s por receta
        raise HTTPException(422, "Dosificación superior al máximo provisional de 120 ml")
    total = plan.water_litres + plan.fertilizer_ml / 1000.0
    if total > TANK_CAPACITY_L:
        raise HTTPException(422, "El agua más el fertilizante superan la capacidad del depósito")
    return {
        "mode": "preview_only",
        "actuators_enabled": False,
        "water_target_litres": round(plan.water_litres, 3),
        "current_tank_litres": round(level, 3),
        "estimated_water_to_add_litres": round(plan.water_litres, 3),
        "requires_empty_tank": True,
        "empty_tolerance_litres": EMPTY_TOLERANCE_L,
        "estimated_dosing_seconds": round(plan.fertilizer_ml, 1),
        "fertilizer_ml": round(plan.fertilizer_ml, 2),
        "estimated_final_volume_litres": round(total, 3),
        "steps": ["RELAY1: llenado de agua (no ejecutado)",
                  "RELAY2: dosificación calibrada (no ejecutada)"],
        "notice": "Vista previa, no se ha accionado ningún relé. Requiere calibración y control seguro del Opta.",
    }

@app.post("/api/dissolutions/start")
def start_dissolution_disabled():
    raise HTTPException(503, "Inicio remoto deshabilitado hasta calibrar la bomba y validar el firmware del Opta.")

# Dosing requires an updated Opta firmware with an explicit HTTP command interface.
# Disabled by default. Explicit opt-in is needed once bench tests are complete.
OPTA_BASE_URL = os.getenv("OPTA_BASE_URL", "http://192.168.50.2:8080")
DOSING_ENABLED = os.getenv("AUTOCAM_ENABLE_DOSING", "0") == "1"


@app.get("/api/controls/capabilities")
def capabilities():
    return {"dosing_enabled": DOSING_ENABLED,
            "available_durations_ms": [500, 1000, 2000],
            "requires_operator_key": False}

def check_dosing_enabled():
    if not DOSING_ENABLED:
        raise HTTPException(503, "Dosificación deshabilitada")

def send_dose_to_opta(operation: str):
    # Opta firmware must reject commands outside its own safety limits.
    url = OPTA_BASE_URL + "/api/dosing/" + operation
    req = Request(url, method="POST", headers={"Connection": "close", "Accept": "application/json"})
    try:
        with urlopen(req, timeout=1.5) as resp:
            if resp.status != 200:
                raise HTTPException(502, "El Opta no ha confirmado la orden")
            body = json.loads(resp.read(1025))
    except (HTTPError, URLError, TimeoutError, OSError, ValueError) as exc:
        raise HTTPException(502, "Error de comunicación con el control de dosificación del Opta") from exc
    if not isinstance(body, dict) or body.get("accepted") is not True:
        raise HTTPException(409, "Orden rechazada por el Opta")
    return {"accepted": True, "opta": body}

@app.post("/api/dosing/off")
def stop_calibration():
    check_dosing_enabled()
    return send_dose_to_opta("off")

@app.post("/api/dosing/{duration_ms}")
def calibrate_dose(duration_ms: int):
    check_dosing_enabled()
    if duration_ms not in (500, 1000, 2000):
        raise HTTPException(422, "Solo se admiten 500, 1000 o 2000 ms")
    # No user-controlled direct relay switching; Opta enforces stop timer/interlocks.
    return send_dose_to_opta(str(duration_ms))



# Recipes remain opt-in until dry relay tests have passed.
RECIPES_ENABLED = os.getenv("AUTOCAM_ENABLE_RECIPES", "0") == "1"

def recipe_request(path: str, method: str = "GET"):
    req = Request(OPTA_BASE_URL + path, method=method,
                  headers={"Connection": "close", "Accept": "application/json"})
    try:
        with urlopen(req, timeout=2.5) as resp:
            data = json.loads(resp.read(4097))
            if not isinstance(data, dict):
                raise ValueError("Respuesta JSON inválida")
            return data
    except HTTPError as exc:
        # Rejects from the PLC must not be interpreted as a successful start.
        raise HTTPException(409 if exc.code in (400, 409, 422) else 502,
                            "Orden rechazada por el Opta (HTTP %s)" % exc.code) from exc
    except (URLError, TimeoutError, OSError, ValueError) as exc:
        raise HTTPException(503, "No se ha podido confirmar la respuesta del Opta; consultar estado antes de repetir") from exc

@app.get("/api/recipes/status")
def recipe_status():
    return recipe_request("/api/recipes/status")

@app.get("/api/recipes/capabilities")
def recipe_capabilities():
    return {"start_enabled": RECIPES_ENABLED,
            "empty_tolerance_litres": EMPTY_TOLERANCE_L,
            "calibrated_ml_per_second": 1.0}

@app.post("/api/recipes/start")
def recipe_start(plan: DissolutionPlan):
    if not RECIPES_ENABLED:
        raise HTTPException(503, "Inicio de recetas deshabilitado hasta validar relés sin cargas")
    # This is a convenience pre-check, NEVER a substitute for PLC interlocks.
    state = recipe_status()
    if state.get("state") != "idle" or state.get("empty_confirmed") is not True:
        raise HTTPException(409, "El Opta no confirma depósito vacío y receta en espera")
    volume = state.get("measured_litres")
    if type(volume) not in (int, float) or not (0 <= volume <= EMPTY_TOLERANCE_L):
        raise HTTPException(409, "Lectura de depósito no válida o no vacío")
    if plan.water_litres + plan.fertilizer_ml / 1000.0 > TANK_CAPACITY_L:
        raise HTTPException(422, "Se supera la capacidad del depósito")
    # Firmware endpoint uses numeric URL components, not untrusted arbitrary paths.
    water = format(plan.water_litres, ".3f")
    dose = format(plan.fertilizer_ml, ".3f")
    return recipe_request("/api/recipes/start/" + water + "/" + dose, "POST")

@app.post("/api/recipes/abort")
def recipe_abort():
    # Emergency cancellation available even if starts are disabled.
    return recipe_request("/api/recipes/abort", "POST")

@app.post("/api/recipes/ack")
def recipe_ack():
    if not RECIPES_ENABLED:
        raise HTTPException(503, "Reconocimiento de recetas deshabilitado")
    return recipe_request("/api/recipes/ack", "POST")
