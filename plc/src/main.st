(* AutoCam Corralon - ciclo principal del Opta *)

(* La capa de comunicaciones actualiza las variables de proceso.
   La lógica de control trabaja solo con variables internas. *)

(* Ejemplo de integración:
   fbPump(
      Command      := CmdPump,
      SensorOK     := LevelSensorOK,
      LevelLitres  := LevelLitres,
      MinimumLevel := MinLevelLitres
   );

   PumpPermission := fbPump.Permission;
   PumpOutput     := fbPump.Pump;
*)
