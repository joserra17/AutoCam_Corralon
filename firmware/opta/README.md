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
