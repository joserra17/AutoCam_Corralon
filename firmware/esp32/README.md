# Firmware ESP32

Área reservada para nodos ESP32 auxiliares.

Usos previstos:

- adquisición de sensores;
- medida de nivel, peso u otras variables del mezclador;
- procesamiento local cuando simplifique el sistema;
- comunicación con Arduino Opta mediante RS485 / Modbus RTU.

La ESP32 no debe asumir autoridad sobre actuadores críticos que correspondan al Opta.

La estructura interna (`src/`, `include/`, `test/`) se concretará cuando definamos el primer nodo real.

## LCD 1602 en paralelo

Asignación utilizada por el firmware:

| Señal LCD | GPIO ESP32 |
|---|---:|
| RS | 19 |
| EN | 18 |
| D4 | 17 |
| D5 | 16 |
| D6 | 4 |
| D7 | 27 |

Para evitar caracteres corruptos:

- conectar `RW` directamente a GND;
- compartir GND entre pantalla, ESP32 y fuente;
- colocar 100 nF entre `VDD` y `VSS` junto a la pantalla;
- mantener cortos los cables de `EN` y datos;
- usar adaptación lógica adecuada si la pantalla funciona a 5 V y no reconoce de forma fiable los 3,3 V del ESP32;
- reducir la salida `ECHO` de 5 V del HC-SR04 a 3,3 V antes del GPIO 26.

## Estabilización del nivel

La distancia publicada se calcula mediante la mediana de las últimas 5 medidas válidas. Esto rechaza ecos aislados causados por ondas o salpicaduras sin permitir que la ESP32 gobierne directamente la bomba. Se requieren 3 medidas sin eco consecutivas antes de publicar `SENSOR_ERROR`; el Opta mantiene además su propio timeout independiente.
