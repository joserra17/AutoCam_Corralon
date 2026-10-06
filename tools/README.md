# Puente USB de nivel

Conecta la ESP32 y el Opta al ordenador mediante dos cables USB. Este programa reenvía solamente tramas válidas de la ESP32 y envía al Opta la consigna elegida.

## Instalación

```powershell
py -m pip install -r tools\requirements.txt
py tools\serial_bridge.py --list
```

`pyserial` es una dependencia del programa ejecutado en el ordenador, no de los firmwares. Por eso está fijada en `tools/requirements.txt` y no en los archivos `platformio.ini`.

Después de identificar cada puerto:

```powershell
py tools\serial_bridge.py --esp COM5 --opta COM6 --setpoint 40
```

Sustituye `COM5` y `COM6` por los puertos reales. La consigna es un porcentaje de 1 a 100. Con el puente en marcha se puede escribir otro porcentaje y pulsar Intro. El comando `reset` rearma únicamente el fallo por tiempo máximo de funcionamiento.

Al cerrar el puente con `Ctrl+C`, desconectar un USB o dejar de recibir medidas, el Opta desactiva la bomba al vencer el timeout.

## Tramas

- ESP32 → Opta: `LEVEL,secuencia,porcentaje,litros,distancia_cm`
- ESP32 → Opta: `SENSOR_ERROR,secuencia`
- Ordenador → Opta: `SETPOINT,porcentaje`
- Ordenador → Opta: `RESET`
- Opta → ordenador: `STATUS,nivel,consigna,bomba,fallo`

El ordenador no gobierna directamente la salida: comunica medidas y consigna; el Opta aplica las reglas de seguridad.
