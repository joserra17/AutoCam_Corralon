# Pruebas

Áreas previstas:

- `logic/`: pruebas de lógica sin hardware real.
- `integration/`: pruebas entre componentes y contratos.
- `hardware/`: procedimientos de validación controlada con hardware físico.

## Regla de seguridad

Las pruebas automáticas no deben activar hardware físico por defecto. Las pruebas con relés, bombas o electroválvulas reales deben ser explícitas, controladas y documentadas.
