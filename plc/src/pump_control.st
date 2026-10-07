FUNCTION_BLOCK FB_PumpControl
VAR_INPUT
    Command      : BOOL;
    SensorOK     : BOOL;
    LevelLitres  : REAL;
    MinimumLevel : REAL;
    AlarmGeneral : BOOL;
END_VAR

VAR_OUTPUT
    Permission : BOOL;
    Pump       : BOOL;
END_VAR

Permission :=
    SensorOK
    AND (LevelLitres > MinimumLevel)
    AND NOT AlarmGeneral;

Pump := Command AND Permission;

END_FUNCTION_BLOCK
