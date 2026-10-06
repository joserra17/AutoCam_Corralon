# Firmware Arduino Opta

Controlador principal del sistema.

Responsabilidades previstas:

- lógica crítica de riego;
- validación de órdenes recibidas desde la Raspberry Pi;
- control de electroválvulas, bombas y relés;
- enclavamientos y estados seguros;
- Modbus TCP con Raspberry Pi;
- Modbus RTU con ESP32 cuando se implemente.

La estructura interna (`src/`, `include/`, `test/`) se concretará al comenzar el primer firmware real para evitar crear módulos vacíos sin necesidad.

## Llenado por puente USB

El firmware recibe por el puerto USB serie medidas `LEVEL` y una consigna `SETPOINT`. Controla la salida 1 (`D0`) con histéresis de 5 %, confirmación temporal de arranque y parada, reposo mínimo, timeout de sensor y límite de marcha continua. El estado se publica como:

```text
STATUS,nivel_porcentaje,consigna_porcentaje,bomba_0_o_1,fallo
```

La salida permanece desactivada si no existe una consigna válida o no llegan medidas recientes.
