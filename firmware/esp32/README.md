# Firmware ESP32

Área reservada para nodos ESP32 auxiliares.

Usos previstos:

- adquisición de sensores;
- medida de nivel, peso u otras variables del mezclador;
- procesamiento local cuando simplifique el sistema;
- comunicación con Arduino Opta mediante RS485 / Modbus RTU.

La ESP32 no debe asumir autoridad sobre actuadores críticos que correspondan al Opta.

La estructura interna (`src/`, `include/`, `test/`) se concretará cuando definamos el primer nodo real.
