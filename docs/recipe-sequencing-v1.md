# Recetas de disolución: v1 de lógica, no desplegada

## Implementado en esta rama

- `firmware/opta/include/RecipeController.h`: máquina de estados C++ **independiente**, sin escribir GPIO, aún sin conectar al `main.cpp` que funciona en el dispositivo.
- `firmware/opta/tests/recipe_controller_test.cpp`: pruebas de secuencia, exclusión de relés, parada por pérdida de sensor y bloqueo hasta vaciado manual.
- `POST /api/dissolutions/preview`: vista previa que requiere depósito vacío y comprueba volumen y dosis.
- Dashboard: textos y validaciones correspondientes. El botón de **iniciar** permanece deshabilitado.

## Condiciones acordadas

1. Depósito de capacidad nominal 20 L, vacío antes de crear una nueva receta.
2. Vaciado entre recetas **manual**.
3. Primero RELAY1 llena hasta litros solicitados; RELAY2 nunca simultáneo.
4. Estabilización de lectura antes de dosificar.
5. Después RELAY2 dosifica agua (simula fertilizante), a caudal provisional 1 ml/s.
6. Parada en fallo de sensor, bloqueo de seguridad o timeout, sin reanudación autónoma.
7. Finalizada la mezcla, depósito bloqueado para nuevas recetas hasta descarga y confirmación manual.

## Pendiente **antes** de autorizar órdenes físicas

- Integrar `RecipeController` en el `main.cpp` **real** del Opta (la versión de GitHub main está desfasada frente a la instalada).
- No conectar `relay1()` / `relay2()` a GPIO directamente hasta revisar reglas de enclavamiento con el `TankFillController` actual; el controlador de recetas debe ser dueño exclusivo de ambas salidas durante una receta.
- Definir un `POST /api/recipes` con identificador/idempotencia y `POST /api/recipes/abort`, validación estricta del Opta, aceptación explícita y consulta de progreso en `GET /api/recipes/status`.
- Evaluar las tolerancias de vacío (0.10 L), llenado (0.20 L) y tiempo máximo de dosificación (120 s) con el sensor y las bombas reales. Son **provisionales**, no una garantía de precisión.
- Probar por etapas con las bombas desconectadas y después solo agua, incluyendo fallos de conexión y reinicios del Opta, antes de autorizar modo real.
- La calibración 1 ml/s es provisional y el ultrasonido no comprueba limpieza o residuos.

## Prueba local opcional

```bash
cd firmware/opta
g++ -std=c++17 -Wall -Wextra tests/recipe_controller_test.cpp -o /tmp/recipe_controller_test
/tmp/recipe_controller_test
```

El test está aportado, pero no se ha ejecutado en esta sesión.
