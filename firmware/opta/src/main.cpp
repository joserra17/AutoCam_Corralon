#include <Arduino.h>
#include "TankFillController.h"

// La salida 1 del Opta se identifica como D0 en el core de Arduino.
// Debe gobernar un contactor/relé adecuado, no alimentar la bomba directamente.
constexpr pin_size_t PUMP_OUTPUT = RELAY1;
constexpr uint32_t STATUS_INTERVAL_MS = 1000;

TankFillController controller;
char inputLine[64];
size_t inputLength = 0;
uint32_t lastStatusMs = 0;

const char *faultName(TankFillController::Fault fault)
{
    switch (fault)
    {
        case TankFillController::Fault::None: return "NONE";
        case TankFillController::Fault::NoSetpoint: return "NO_SETPOINT";
        case TankFillController::Fault::SensorTimeout: return "SENSOR_TIMEOUT";
        case TankFillController::Fault::InvalidLevel: return "INVALID_LEVEL";
        case TankFillController::Fault::MaximumRunTime: return "MAX_RUN_TIME";
    }
    return "UNKNOWN";
}

void applyOutput()
{
    digitalWrite(PUMP_OUTPUT, controller.pumpOn() ? HIGH : LOW);
    digitalWrite(LED_BUILTIN, controller.pumpOn() ? HIGH : LOW);
}

void processLine(char *line)
{
    unsigned long sequence = 0;
    unsigned int level = 0;
    unsigned int setpoint = 0;

    if (sscanf(line, "LEVEL,%lu,%u", &sequence, &level) == 2)
    {
        (void)sequence;
        if (level <= 100) controller.updateLevel((uint8_t)level, millis());
        else controller.invalidateLevel();
        return;
    }
    if (sscanf(line, "SETPOINT,%u", &setpoint) == 1)
    {
        if (setpoint <= 100) controller.setSetpoint((uint8_t)setpoint);
        else controller.setSetpoint(0);
        return;
    }
    if (strncmp(line, "SENSOR_ERROR", 12) == 0)
    {
        controller.invalidateLevel();
        return;
    }
    if (strcmp(line, "RESET") == 0) controller.resetMaximumRunTimeFault();
}

void readSerialLines()
{
    while (Serial.available() > 0)
    {
        const char character = (char)Serial.read();
        if (character == '\r') continue;
        if (character == '\n')
        {
            inputLine[inputLength] = '\0';
            if (inputLength > 0) processLine(inputLine);
            inputLength = 0;
            continue;
        }
        if (inputLength < sizeof(inputLine) - 1) inputLine[inputLength++] = character;
        else inputLength = 0;
    }
}

void sendStatus(uint32_t nowMs)
{
    if ((uint32_t)(nowMs - lastStatusMs) < STATUS_INTERVAL_MS) return;
    lastStatusMs = nowMs;
    Serial.print("STATUS,");
    if (controller.levelValid()) Serial.print(controller.levelPercent());
    else Serial.print(-1);
    Serial.print(',');
    Serial.print(controller.setpointPercent());
    Serial.print(',');
    Serial.print(controller.pumpOn() ? 1 : 0);
    Serial.print(',');
    Serial.println(faultName(controller.fault()));
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PUMP_OUTPUT, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(PUMP_OUTPUT, LOW);

    Serial.begin(115200);
    delay(500);
    Serial.println("OPTA_READY");
}

void loop()
{
    readSerialLines();
    const uint32_t nowMs = millis();
    controller.update(nowMs);
    applyOutput();
    sendStatus(nowMs);
}
