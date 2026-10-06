# Arquitectura

## Fase 1

```text
Navegador local
      |
      v
Raspberry Pi
Dashboard + API + programación
      |
 Ethernet / Modbus TCP
      |
      v
Arduino Opta
Control, seguridad y actuadores
      |
 RS485 / Modbus RTU (previsto)
      |
      v
ESP32
Sensores y subsistemas auxiliares
```

## Responsabilidades

### Arduino Opta
- Autoridad final sobre bombas, electroválvulas y otros actuadores críticos.
- Secuencias, enclavamientos, estados seguros y validación de órdenes.
- Debe mantener un comportamiento seguro aunque la Raspberry deje de responder.

### Raspberry Pi
- Dashboard web local.
- Configuración y programación de riegos.
- Históricos, eventos y alarmas cuando se implementen.
- Comunicación con el Opta mediante una interfaz definida y versionada.

### ESP32
- Adquisición de sensores.
- Procesamiento local de subsistemas auxiliares, como el mezclador, cuando sea necesario.
- Comunicación con el Opta sin asumir autoridad sobre actuadores críticos.

## Evolución prevista

1. Control manual local de electroválvulas.
2. Órdenes temporizadas.
3. Programación de riegos.
4. Persistencia e históricos.
5. Sensores y fertirriego.
6. Robustez industrial y recuperación ante fallos.
7. Acceso remoto como fase posterior.

## Puente USB provisional para el nivel del depósito

Hasta instalar el enlace RS485, el ordenador conecta simultáneamente por USB con la ESP32 y el Opta. `tools/serial_bridge.py` valida y reenvía las medidas de nivel y la consigna. El Opta continúa siendo la autoridad sobre la salida de la bomba.

```text
ESP32 -- USB serie --> ordenador -- USB serie --> Opta -- salida 1 --> contactor bomba
```

Este puente es provisional: al perder datos durante más de 2,5 segundos, el Opta desactiva la bomba.
