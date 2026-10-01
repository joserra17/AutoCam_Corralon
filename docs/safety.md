# Seguridad y estados seguros

## Principios

- El Opta conserva la autoridad sobre actuadores críticos.
- Una orden del dashboard es una solicitud, no una escritura directa sobre una salida.
- Un reinicio no debe provocar activaciones transitorias no deseadas.
- La pérdida de Raspberry, red local o ESP32 no debe dejar actuadores críticos en un estado indefinido.
- Los estados seguros concretos de cada salida se documentarán en `io-map.md`.

## Fallos a contemplar

- Pérdida de comunicación Raspberry ↔ Opta.
- Pérdida de comunicación Opta ↔ ESP32.
- Reinicio del Opta.
- Reinicio de la Raspberry.
- Sensor incoherente o fuera de rango.
- Orden caducada o duplicada.
- Actuador que no confirma el estado esperado, cuando exista realimentación.

## Validación

Antes de conectar cargas reales, cada salida deberá validarse de forma controlada y las pruebas automáticas no deberán activar hardware físico por defecto.
