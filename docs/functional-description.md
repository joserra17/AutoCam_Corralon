# Descripción funcional

## Alcance inicial

La fase 1 debe permitir operar el sistema de riego localmente sin depender de Internet.

Funciones objetivo:

- Visualizar el estado general del sistema.
- Visualizar el estado de cada electroválvula.
- Solicitar apertura y cierre manual desde el dashboard local.
- Ejecutar aperturas temporizadas.
- Definir programaciones de riego.
- Registrar eventos y alarmas cuando se incorpore persistencia.

## Principio de control

El dashboard no escribe directamente salidas físicas. Envía solicitudes al Opta y el Opta decide si pueden ejecutarse según el estado del sistema, enclavamientos y reglas de seguridad.

## Modos previstos

- PARADO
- MANUAL
- AUTOMÁTICO
- FALLO

Los detalles se concretarán en `operating-modes.md` a medida que se implemente la lógica real.
