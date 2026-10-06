#include <assert.h>
#include <stdint.h>
#include "TankFillController.h"

int main()
{
    TankFillController controller;

    controller.updateLevel(20, 100);
    controller.update(100);
    assert(!controller.pumpOn());
    assert(controller.fault() == TankFillController::Fault::NoSetpoint);

    controller.setSetpoint(50);
    controller.update(101);
    assert(!controller.pumpOn());

    // El nivel bajo debe mantenerse durante 10 s antes de arrancar.
    controller.updateLevel(20, 10101);
    controller.update(10101);
    assert(controller.pumpOn());

    // Un único valor en la consigna no debe parar la bomba.
    controller.updateLevel(50, 11000);
    controller.update(11000);
    assert(controller.pumpOn());

    // La parada normal requiere 2 s de nivel confirmado.
    controller.updateLevel(50, 13000);
    controller.update(13000);
    assert(!controller.pumpOn());

    // Durante los 30 s de reposo no debe volver a arrancar.
    controller.updateLevel(44, 42000);
    controller.update(42000);
    assert(!controller.pumpOn());

    // Después del reposo aún exige 10 s continuos por debajo de 45 %.
    controller.updateLevel(44, 43000);
    controller.update(43000);
    assert(!controller.pumpOn());
    controller.updateLevel(44, 53000);
    controller.update(53000);
    assert(controller.pumpOn());

    controller.update(53000 + TankFillController::SENSOR_TIMEOUT_MS + 1);
    assert(!controller.pumpOn());
    assert(controller.fault() == TankFillController::Fault::SensorTimeout);

    // Tras recuperar el sensor se vuelve a exigir la confirmación de arranque.
    controller.updateLevel(20, 56000);
    controller.update(56000);
    assert(!controller.pumpOn());
    controller.updateLevel(20, 66000);
    controller.update(66000);
    assert(controller.pumpOn());

    const uint32_t maximumRunTime = 66000 + TankFillController::MAXIMUM_RUN_TIME_MS + 1;
    controller.updateLevel(20, maximumRunTime);
    controller.update(maximumRunTime);
    assert(!controller.pumpOn());
    assert(controller.fault() == TankFillController::Fault::MaximumRunTime);

    controller.resetMaximumRunTimeFault();
    controller.updateLevel(20, maximumRunTime + 1);
    controller.update(maximumRunTime + 1);
    assert(!controller.pumpOn());
    controller.updateLevel(20, maximumRunTime + 1 + TankFillController::START_CONFIRMATION_MS);
    controller.update(maximumRunTime + 1 + TankFillController::START_CONFIRMATION_MS);
    assert(controller.pumpOn());
}
