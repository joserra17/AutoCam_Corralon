"""AutoCam dashboard v1: read-only mock until Opta protocol is verified."""
from datetime import datetime, timezone
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

app = FastAPI(title="AutoCam Local API", version="0.1.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173", "http://127.0.0.1:5173", "http://192.168.50.1:5173"],
    allow_credentials=False, allow_methods=["GET"], allow_headers=["*"],
)

class Status(BaseModel):
    mode: str
    timestamp: str
    opta: str
    esp32: str
    ethernet: str
    level_percent: int | None
    litres: float | None
    setpoint_percent: int | None
    pump_main: str
    pump_dosing: list[str]
    valves: list[str]
    alarms: list[str]

@app.get("/api/health")
def health():
    return {"ok": True, "mode": "simulation", "hardware_connected": False}

@app.get("/api/status", response_model=Status)
def status():
    return Status(
        mode="simulation", timestamp=datetime.now(timezone.utc).isoformat(),
        opta="not_verified", esp32="not_verified", ethernet="not_verified",
        level_percent=None, litres=None, setpoint_percent=None,
        pump_main="unknown", pump_dosing=["unknown"] * 3,
        valves=["unknown"] * 4, alarms=["Conexión con Opta no verificada"],
    )

@app.post("/api/commands/{command}")
def commands_disabled(command: str):
    raise HTTPException(
        status_code=503,
        detail="Órdenes deshabilitadas: pendiente de implementar y validar el contrato Opta-Raspberry.",
    )
