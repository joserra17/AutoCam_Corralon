# Mapa de E/S

Este documento será la fuente de verdad de las entradas y salidas físicas.

> No asignar pines, canales o relés sin confirmación del hardware y cableado real.

| ID lógico | Dispositivo | Tipo | Canal físico | Función | Estado seguro | Estado |
|---|---|---|---|---|---|---|
| PUMP_FILL | Opta | Salida digital/relé | Salida 1 (`D0`) | Mando del contactor de la bomba de llenado | Desactivada (`LOW`) | Confirmado por el usuario; validar cableado antes de conectar carga |

## Reglas

- Toda nueva E/S debe documentarse aquí antes o junto con su implementación.
- El firmware debe usar nombres lógicos y evitar dispersar referencias a pines físicos por la lógica de dominio.
- Los cambios de cableado deben actualizar este documento.
