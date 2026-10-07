(* ESP32 -> Opta communication adapter

   CURRENT:
   ESP32 sends the processed tank level to the Opta via serial.

   FUTURE:
   Replace this adapter with Modbus RTU while preserving the same
   internal process variables:
     LevelLitres
     LevelPercent
     LevelSensorOK

   The control logic in src/ must not care which transport is used.
*)
