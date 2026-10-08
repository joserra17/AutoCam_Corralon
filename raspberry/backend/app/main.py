"""AutoCam Raspberry local read-only proxy for Arduino Opta HTTP telemetry."""
import json
import os
from datetime import datetime, timezone
from urllib.error import HTTPError, URLError
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
    allow_methods=["GET"],
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

class DissolutionPlan(BaseModel):
    water_litres: float = Field(gt=0, le=TANK_CAPACITY_L)
    fertilizer_ml: float = Field(gt=0, le=1000)

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
    if plan.water_litres <= level:
        raise HTTPException(409, "El depósito ya contiene tanta agua como la consigna o más")
    total = plan.water_litres + plan.fertilizer_ml / 1000.0
    if total > TANK_CAPACITY_L:
        raise HTTPException(422, "El agua más el fertilizante superan la capacidad del depósito")
    return {
        "mode": "preview_only",
        "actuators_enabled": False,
        "water_target_litres": round(plan.water_litres, 3),
        "current_tank_litres": round(level, 3),
        "estimated_water_to_add_litres": round(plan.water_litres - level, 3),
        "fertilizer_ml": round(plan.fertilizer_ml, 2),
        "estimated_final_volume_litres": round(total, 3),
        "steps": ["RELAY1: llenado de agua (no ejecutado)",
                  "RELAY2: dosificación calibrada (no ejecutada)"],
        "notice": "Vista previa, no se ha accionado ningún relé. Requiere calibración y control seguro del Opta.",
    }

@app.post("/api/dissolutions/start")
def start_dissolution_disabled():
    raise HTTPException(503, "Inicio remoto deshabilitado hasta calibrar la bomba y validar el firmware del Opta.")
