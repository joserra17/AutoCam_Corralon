# Seguridad y estados seguros

## Principios

- El Opta conserva la autoridad sobre actuadores críticos.
- Una orden del dashboard es una solicitud, no una escritura directa sobre una salida.
- Un reinicio no debe provocar activaciones transitorias no deseadas.
- La pérdida de Raspberry, red local o ESP32 no debe dejar actuadores críticos en un estado indefinido.
- Los estados seguros concretos de cada salida se documentarán en `io-map.md`.

## Fallos a contemplar

- Pérdida de comunicación Raspberry ↔ Opta.
- Pérdida de comunicación Opta ↔ ESP32.
- Reinicio del Opta.
- Reinicio de la Raspberry.
- Sensor incoherente o fuera de rango.
- Orden caducada o duplicada.
- Actuador que no confirma el estado esperado, cuando exista realimentación.

## Validación

Antes de conectar cargas reales, cada salida deberá validarse de forma controlada y las pruebas automáticas no deberán activar hardware físico por defecto.

## Llenado automático del depósito

- La salida de la bomba arranca desactivada y solo se activa con una consigna y una medida válidas.
- La pérdida de medidas durante más de 2,5 segundos desactiva la bomba.
- Una medida inválida desactiva la bomba.
- La ESP32 aplica una mediana de 5 muestras para rechazar ecos aislados por ondas y salpicaduras.
- Un fallo aislado de eco no genera alarma; se requieren 3 fallos consecutivos. El timeout independiente del Opta sigue protegiendo ante pérdida sostenida.
- La parada normal requiere mantener el nivel en la consigna durante 2 segundos. Una superación de 3 puntos porcentuales provoca parada inmediata.
- Para volver a arrancar, el nivel debe permanecer 5 puntos porcentuales por debajo de la consigna durante 10 segundos.
- Después de alcanzar la consigna se impone un reposo mínimo de 30 segundos antes de permitir otro arranque.
- Un funcionamiento continuo superior a 30 minutos provoca un fallo enclavado que requiere el comando explícito `RESET`.
- La salida 1 del Opta debe gobernar un contactor o relé dimensionado para la bomba. No se conectará la bomba directamente sin comprobar tensiones, intensidades y protecciones.
- La calibración actual del sensor solo contiene puntos medidos entre 0 y 8 litros. No se considerarán fiables consignas superiores a ese intervalo hasta completar la calibración del depósito.
