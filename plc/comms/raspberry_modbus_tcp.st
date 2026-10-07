(* Raspberry Pi <-> Opta Modbus TCP contract

   Raspberry requests actions and reads state.
   Opta remains authoritative over physical outputs.

   Planned command variables:
     CmdPump
     CmdValve1
     CmdValve2
     CmdValve3
     CmdValve4
     SystemEnable

   Planned status variables:
     LevelLitres
     LevelPercent
     LevelSensorOK
     PumpPermission
     PumpOutput
     AlarmGeneral

   Modbus addresses are intentionally not assigned yet.
*)
