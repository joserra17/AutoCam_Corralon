# PLC (Arduino Opta)

Esta carpeta contiene la lógica IEC 61131-3 del Opta y la documentación necesaria para mantenerla versionada fuera del Arduino PLC IDE.

## Filosofía

- `src/`: lógica de control portable en Structured Text.
- `comms/`: adaptación de comunicaciones externas a variables internas del PLC.
- `plc-ide/`: notas y artefactos específicos del proyecto Arduino PLC IDE.
- La lógica de control no debe depender directamente de direcciones físicas ni de un protocolo concreto.
- Las capas de comunicación actualizan variables internas; la lógica consume esas variables.

## Flujo de trabajo

1. Editar y revisar la lógica ST en VS Code.
2. Versionar los cambios con Git.
3. Replicar/importar los POUs necesarios en Arduino PLC IDE.
4. Configurar allí hardware, tareas, E/S y comunicaciones.
5. Compilar y descargar el PLC code al Opta.
6. Mantener sincronizado el código del PLC IDE con estos fuentes.

> Importante: VS Code y PLC IDE no ejecutan dos programas distintos en el Opta. Los archivos de esta carpeta son la fuente versionada de la lógica; PLC IDE es la herramienta que la compila y descarga al runtime PLC.

## Arquitectura prevista

ESP32 -> Serial (actual) / Modbus RTU (futuro) -> Opta
Raspberry Pi -> Modbus TCP -> Opta

El Opta mantiene la autoridad final sobre actuadores, enclavamientos y estados seguros.
