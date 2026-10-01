# Mapa Modbus

Este documento será la fuente de verdad de los contratos de comunicación.

## Raspberry Pi ↔ Arduino Opta

Transporte previsto: Ethernet / Modbus TCP.

| Dirección | Tipo | Nombre lógico | Sentido | Descripción | Estado |
|---|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD | Pendiente |

## Arduino Opta ↔ ESP32

Transporte previsto: RS485 / Modbus RTU.

| Dirección | Tipo | Nombre lógico | Sentido | Descripción | Estado |
|---|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD | Pendiente |

## Reglas

- No reutilizar direcciones sin actualizar este documento.
- Evitar que la Raspberry escriba directamente estados físicos de salida; debe solicitar acciones lógicas al Opta.
- Distinguir entre comandos, estados confirmados, alarmas y telemetría.
- Documentar tipo, rango, unidad, propietario y comportamiento ante timeout antes de dar una dirección por definitiva.
