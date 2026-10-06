#include <Arduino.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <string.h>

// ==============================
// LCD 1602
// ==============================

constexpr uint8_t LCD_RS = 19;
constexpr uint8_t LCD_EN = 18;
constexpr uint8_t LCD_D4 = 17;
constexpr uint8_t LCD_D5 = 16;
constexpr uint8_t LCD_D6 = 4;
constexpr uint8_t LCD_D7 = 27;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;
constexpr uint32_t LCD_REFRESH_MS = 1000;

void escribirLineaLCD(uint8_t fila, const char *texto)
{
    static char contenidoAnterior[LCD_ROWS][LCD_COLUMNS + 1] = {{0}, {0}};
    char linea[LCD_COLUMNS + 1];
    memset(linea, ' ', LCD_COLUMNS);
    linea[LCD_COLUMNS] = '\0';

    const size_t longitud = strlen(texto);
    memcpy(linea, texto, min(longitud, (size_t)LCD_COLUMNS));

    // Evitar tráfico innecesario. Las interrupciones del USB/serie no deben
    // partir un pulso EN del bus paralelo mientras se transmite la línea.
    if (memcmp(contenidoAnterior[fila], linea, LCD_COLUMNS) == 0)
    {
        return;
    }

    noInterrupts();
    lcd.setCursor(0, fila);
    lcd.print(linea);
    interrupts();

    memcpy(contenidoAnterior[fila], linea, LCD_COLUMNS + 1);
}

// ==============================
// HC-SR04
// ==============================

constexpr uint8_t TRIG_PIN = 25;
constexpr uint8_t ECHO_PIN = 26;

// La mediana elimina ecos aislados producidos por ondas y salpicaduras.
constexpr uint8_t DISTANCE_FILTER_SAMPLES = 5;
constexpr uint8_t SENSOR_ERRORS_BEFORE_ALARM = 3;

float filtrarDistanciaCm(float nuevaDistancia)
{
    static float muestras[DISTANCE_FILTER_SAMPLES] = {0};
    static uint8_t cantidad = 0;
    static uint8_t posicion = 0;

    muestras[posicion] = nuevaDistancia;
    posicion = (posicion + 1) % DISTANCE_FILTER_SAMPLES;
    if (cantidad < DISTANCE_FILTER_SAMPLES) cantidad++;

    float ordenadas[DISTANCE_FILTER_SAMPLES];
    memcpy(ordenadas, muestras, cantidad * sizeof(float));

    for (uint8_t i = 1; i < cantidad; i++)
    {
        const float valor = ordenadas[i];
        int8_t j = i - 1;
        while (j >= 0 && ordenadas[j] > valor)
        {
            ordenadas[j + 1] = ordenadas[j];
            j--;
        }
        ordenadas[j + 1] = valor;
    }

    return ordenadas[cantidad / 2];
}

// ==============================
// Calibración real
// ==============================

constexpr int NUM_PUNTOS = 9;

const float litrosCal[NUM_PUNTOS] = {
    0.0,
    1.0,
    2.0,
    3.0,
    4.0,
    5.0,
    6.0,
    7.0,
    8.0
};

const float distanciaCal[NUM_PUNTOS] = {
    25.9,
    23.5,
    22.0,
    20.6,
    19.1,
    18.0,
    16.4,
    14.8,
    13.3
};

// Capacidad total que queremos considerar
constexpr float CAPACIDAD_TOTAL_L = 20.0f;

// ==============================
// Medición ultrasonidos
// ==============================

float medirDistanciaCm()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    unsigned long duracion =
        pulseIn(ECHO_PIN, HIGH, 30000);

    if (duracion == 0)
    {
        return -1.0f;
    }

    return duracion * 0.0343f / 2.0f;
}

// ==============================
// Distancia -> litros
// ==============================

float calcularLitros(float distancia)
{
    // Vacío o por debajo del mínimo calibrado
    if (distancia >= distanciaCal[0])
    {
        return 0.0f;
    }

    // Buscar entre qué dos puntos estamos
    for (int i = 0; i < NUM_PUNTOS - 1; i++)
    {
        float d1 = distanciaCal[i];
        float d2 = distanciaCal[i + 1];

        if (distancia <= d1 && distancia >= d2)
        {
            float l1 = litrosCal[i];
            float l2 = litrosCal[i + 1];

            // Interpolación lineal entre los dos puntos
            float factor =
                (d1 - distancia) /
                (d1 - d2);

            return l1 + factor * (l2 - l1);
        }
    }

    // Por debajo de 13.3 cm tenemos más de 8 L.
    // Extrapolamos usando el último tramo conocido.

    float d1 = distanciaCal[NUM_PUNTOS - 2]; // 14.8
    float d2 = distanciaCal[NUM_PUNTOS - 1]; // 13.3

    float l1 = litrosCal[NUM_PUNTOS - 2];    // 7 L
    float l2 = litrosCal[NUM_PUNTOS - 1];    // 8 L

    float factor =
        (d1 - distancia) /
        (d1 - d2);

    float litros =
        l1 + factor * (l2 - l1);

    return constrain(litros, 0.0f, CAPACIDAD_TOTAL_L);
}

// ==============================
// Litros -> porcentaje
// ==============================

int calcularNivel(float litros)
{
    float porcentaje =
        litros * 100.0f /
        CAPACIDAD_TOTAL_L;

    return constrain(
        (int)round(porcentaje),
        0,
        100
    );
}

// ==============================
// Inicialización
// ==============================

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== AutoCam nivel calibrado ===");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);

    // LCD
    Serial.println("[1/4] Esperando alimentacion estable...");
    delay(2000);

    Serial.println("[2/4] Inicializando LCD...");
    lcd.begin(LCD_COLUMNS, LCD_ROWS);
    delay(250);

    lcd.display();
    lcd.noCursor();
    lcd.noBlink();
    lcd.clear();
    delay(10);
    lcd.home();

    Serial.println("[3/4] Mostrando inicio...");

    escribirLineaLCD(0, "AUTO CAM");
    escribirLineaLCD(1, "Calibrando...");

    delay(1500);

    lcd.clear();

    Serial.println("[4/4] Sistema listo.");
}

// ==============================
// Bucle principal
// ==============================

void loop()
{
    static uint32_t sequence = 0;
    static uint32_t ultimaActualizacionLCD = 0;
    static uint8_t erroresConsecutivos = 0;
    const float distanciaInstantanea = medirDistanciaCm();

    if (distanciaInstantanea < 0)
    {
        if (erroresConsecutivos < SENSOR_ERRORS_BEFORE_ALARM) erroresConsecutivos++;

        if (erroresConsecutivos >= SENSOR_ERRORS_BEFORE_ALARM)
        {
            Serial.print("SENSOR_ERROR,");
            Serial.println(sequence++);
        }

        const uint32_t ahora = millis();
        if ((uint32_t)(ahora - ultimaActualizacionLCD) >= LCD_REFRESH_MS)
        {
            ultimaActualizacionLCD = ahora;
            escribirLineaLCD(0, "SIN LECTURA");
            escribirLineaLCD(1, "Revise sensor");
        }

        delay(500);
        return;
    }

    erroresConsecutivos = 0;
    const float distancia = filtrarDistanciaCm(distanciaInstantanea);

    float litros = calcularLitros(distancia);
    int nivel = calcularNivel(litros);

    // ==========================
    // Trama para el puente USB del ordenador
    // ==========================

    Serial.print("LEVEL,");
    Serial.print(sequence++);
    Serial.print(',');
    Serial.print(nivel);
    Serial.print(',');
    Serial.print(litros, 2);
    Serial.print(',');
    Serial.println(distancia, 1);

    // ==========================
    // LCD
    // ==========================

    const uint32_t ahora = millis();
    if ((uint32_t)(ahora - ultimaActualizacionLCD) >= LCD_REFRESH_MS)
    {
        ultimaActualizacionLCD = ahora;

        char lineaSuperior[LCD_COLUMNS + 1];
        char lineaInferior[LCD_COLUMNS + 1];
        snprintf(lineaSuperior, sizeof(lineaSuperior), "L:%4.1f D:%4.1f", litros, distancia);
        snprintf(lineaInferior, sizeof(lineaInferior), "Nivel:%3d%%", nivel);

        escribirLineaLCD(0, lineaSuperior);
        escribirLineaLCD(1, lineaInferior);
    }

    delay(300);
}
