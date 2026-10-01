# AutoCam Corralón

Sistema modular de automatización agrícola para el Corralón.

## Fase 1

La primera fase funciona sin acceso remoto y se centra en:

- Arduino Opta como controlador principal y autoridad sobre actuadores críticos.
- Raspberry Pi con dashboard local propio para operación, programación e históricos.
- Comunicación Raspberry Pi ↔ Opta mediante Ethernet / Modbus TCP.
- Firmware ESP32 disponible para subsistemas auxiliares y sensores.
- Comunicación Opta ↔ ESP32 prevista mediante RS485 / Modbus RTU.

## Principios de arquitectura

- La Raspberry Pi supervisa, configura y solicita acciones; no gobierna directamente salidas críticas.
- El Opta valida órdenes, ejecuta secuencias, aplica enclavamientos y mantiene el estado seguro.
- La ESP32 se reserva para adquisición de sensores y control auxiliar distribuido.
- La instalación debe evolucionar por capas de robustez sin acoplar la lógica crítica al dashboard.
- Las asignaciones físicas de E/S y registros Modbus solo se incorporarán cuando estén confirmadas.

## Estructura

- `docs/`: arquitectura, funcionamiento, seguridad y contratos de comunicación.
- `firmware/opta/`: firmware del Arduino Opta.
- `firmware/esp32/`: firmware de nodos ESP32 auxiliares.
- `raspberry/backend/`: API local y servicios de la Raspberry Pi.
- `raspberry/frontend/`: dashboard web local.
- `hardware/`: documentación eléctrica, mecánica, datasheets y BOM.
- `config/`: configuraciones versionadas del sistema.
- `tests/`: pruebas de lógica, integración y hardware.

## Estado

Repositorio inicializado. Aún no contiene lógica de control de producción ni asignaciones reales de E/S.
