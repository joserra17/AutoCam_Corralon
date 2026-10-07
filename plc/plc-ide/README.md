# Arduino PLC IDE

Este directorio queda reservado para los artefactos y notas específicas del proyecto que se abra en Arduino PLC IDE.

## Regla de sincronización

Los módulos ST versionados en `../src/` y `../comms/` son la referencia legible desde VS Code/Git.

Cuando un POU cambie:
1. Actualizar el fuente ST.
2. Replicar/importar el cambio en PLC IDE.
3. Compilar.
4. Descargar al Opta.
5. Probar en hardware.

La configuración de tareas, recursos, E/S físicas y protocolos se mantiene en PLC IDE y se documentará aquí cuando quede definida.
