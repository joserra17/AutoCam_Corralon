FUNCTION_BLOCK FB_IrrigationLine
VAR_INPUT
    Command      : BOOL;
    SystemEnable : BOOL;
    AlarmGeneral : BOOL;
END_VAR

VAR_OUTPUT
    Valve : BOOL;
END_VAR

Valve :=
    Command
    AND SystemEnable
    AND NOT AlarmGeneral;

END_FUNCTION_BLOCK
