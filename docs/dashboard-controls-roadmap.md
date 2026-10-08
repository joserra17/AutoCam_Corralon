# Dashboard v0.3 — panel y contrato de control previsto

Esta rama tiene diseño profesional y **botones visibles pero deshabilitados**. No se envía ninguna orden al Opta, y no se ha cargado firmware nuevo en el PLC.

## Lectura existente y verificada

- `GET http://192.168.50.2:8080/api/status`: telemetría del Opta.
- `GET /api/status` en FastAPI: proxy de solo lectura para React.
- La interfaz se refresca automáticamente cada 2 segundos. Si el Opta no responde, no muestra los valores previos como actuales.

## Órdenes previstas (NO IMPLEMENTADAS)

El backend de la Raspberry deberá autenticar y validar peticiones y reenviarlas al Opta; este deberá validar de nuevo antes de cambiar salidas o consignas:

| Operación deseada | REST Raspberry futuro | Semántica Opta |
| --- | --- | --- |
| Consigna 40/60/80 % | `POST /api/commands/setpoint` + JSON `{ "percent": 60 }` | Misma validación que `SETPOINT,60` serie |
| Detener llenado | `POST /api/commands/stop` | Misma lógica que `STOP` serie |
| Rearmar fallo | `POST /api/commands/reset` | Misma lógica que `RESET` serie |
| Dosificar 500/1000/2000 ms | `POST /api/commands/dose` + JSON `{ "duration_ms": 1000 }` | Misma lógica que `DOSE,1000` serie |
| Parar dosificación | `POST /api/commands/dose_off` | Misma lógica que `DOSE,OFF` serie |

### Puesta en servicio de control

1. Implementar endpoints de escritura **POST**, nunca GET, y autenticación en el backend.
2. Definir un canal de control Opta compatible con la instalación actual, autenticado o estrictamente aislado y con protección frente a reenvíos.
3. Validar rangos (consigna 1–100 %, dosificación 100–3000 ms), estados, fallos, límites temporales e interbloqueos en el **Opta**; la seguridad nunca depende del navegador.
4. Probar con ambas bombas **físicamente desconectadas** que toda orden devuelve confirmación, que `STOP` funciona, y que pérdida de comunicación y reinicio provocan salida segura.
5. Añadir el historial y habilitar botones solo tras validar de extremo a extremo en QA.

No se deben exponer las API de control a Internet ni confiar únicamente en que el puerto 8080 esté en red local.

## Probar interfaz actual

```bash
cd ~/AutoCam_Corralon
git fetch origin
git switch feat/raspberry-dashboard-controls-ui
cd raspberry/frontend
npm install
npm run dev -- --host 0.0.0.0
```

Mantener el backend FastAPI que ya funciona en el puerto 8000. La interfaz usa el proxy Vite hacia `127.0.0.1:8000`.
