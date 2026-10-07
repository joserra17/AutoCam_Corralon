FUNCTION_BLOCK FB_Alarms
VAR_INPUT
    LevelSensorOK : BOOL;
    LevelLitres   : REAL;
    MinimumLevel  : REAL;
END_VAR

VAR_OUTPUT
    AlarmSensor   : BOOL;
    AlarmLowLevel : BOOL;
    AlarmGeneral  : BOOL;
END_VAR

AlarmSensor   := NOT LevelSensorOK;
AlarmLowLevel := LevelSensorOK AND (LevelLitres <= MinimumLevel);
AlarmGeneral  := AlarmSensor OR AlarmLowLevel;

END_FUNCTION_BLOCK
