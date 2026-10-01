# AGENTS.md — AutoCam Corralón

## Contexto

AutoCam Corralón es un sistema modular de automatización agrícola basado inicialmente en Arduino Opta, Raspberry Pi y ESP32.

## Reglas de trabajo

1. El Arduino Opta es la autoridad final sobre actuadores críticos.
2. La Raspberry Pi puede solicitar acciones, programar y supervisar, pero no debe gobernar directamente salidas físicas críticas.
3. La ESP32 se usa para sensores, adquisición y subsistemas auxiliares; no debe puentear la autoridad del Opta.
4. Ante pérdida de Raspberry, red o ESP32, el Opta debe poder llevar la instalación a un estado seguro.
5. No inventar asignaciones de E/S. `docs/io-map.md` es la fuente de verdad cuando exista una asignación confirmada.
6. No inventar registros Modbus. `docs/modbus-map.md` es la fuente de verdad de los contratos de comunicación.
7. No activar hardware real automáticamente durante pruebas.
8. Separar lógica de dominio de acceso físico al hardware siempre que sea razonable.
9. Los cambios que afecten a seguridad, secuencias o actuadores deben incluir pruebas o un plan de validación.
10. Mantener el funcionamiento local como requisito base; el acceso remoto será una capa posterior.
11. Evitar complejidad prematura: no añadir Docker, nube, MQTT, microservicios u otras capas sin necesidad concreta.
12. Actualizar la documentación cuando cambien interfaces, E/S, protocolos o comportamiento funcional.

## Arquitectura prevista

- Raspberry Pi ↔ Opta: Ethernet / Modbus TCP.
- Opta ↔ ESP32: RS485 / Modbus RTU cuando se implemente.
- Dashboard: aplicación web propia servida desde Raspberry Pi.
- Backend inicial previsto: Python + FastAPI.
- Frontend inicial previsto: React + Vite.
- Persistencia inicial prevista: SQLite cuando sea necesaria.
