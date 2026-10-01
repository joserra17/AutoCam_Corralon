# Raspberry Pi

La Raspberry Pi aloja la capa local de supervisión y configuración.

## Stack inicial previsto

- Backend: Python + FastAPI.
- Frontend: React + Vite.
- Persistencia: SQLite cuando sea necesaria.
- Comunicación con Arduino Opta: Ethernet / Modbus TCP.

## Responsabilidades

- servir el dashboard local;
- exponer una API interna para el frontend;
- gestionar configuración y programación;
- almacenar históricos y eventos cuando se implemente persistencia;
- presentar estados confirmados por el Opta.

La Raspberry no controla directamente las salidas críticas.
