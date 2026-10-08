"""AutoCam Raspberry local read-only proxy for Arduino Opta HTTP telemetry."""
import json
import os
from datetime import datetime, timezone
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

from fastapi import FastAPI, HTTPException
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
