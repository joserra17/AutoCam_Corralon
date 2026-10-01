# Modos de operación

## PARADO

El sistema no ejecuta secuencias automáticas. Las salidas críticas deben permanecer en su estado seguro salvo acciones de mantenimiento explícitamente permitidas.

## MANUAL

Permite solicitudes locales desde el dashboard. El Opta sigue aplicando enclavamientos, límites y reglas de seguridad.

## AUTOMÁTICO

El sistema ejecuta programaciones y secuencias configuradas, siempre bajo validación del Opta.

## FALLO

Estado activado ante condiciones que impidan continuar de forma segura. Las salidas afectadas deben ir a su estado seguro y el sistema debe exponer la causa del fallo.

## Pendiente de definir

- Transiciones exactas entre modos.
- Política de rearme.
- Qué acciones manuales se permiten durante FALLO.
- Persistencia del modo tras reinicio.
