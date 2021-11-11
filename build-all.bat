@ECHO OFF
CLS

IF [%1] == [] (
    SET Type=debug
) ELSE IF [%1] == [release] (
    ECHO -------- RELEASE --------
    SET Type=release
) ELSE IF [%1] == [debug] (
        SET Type=debug
) ELSE (
    ECHO ERROR: unknown build type '%1'
    GOTO :ERROR
)

PUSHD engine
CALL build.bat %Type%
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL%)

PUSHD game
CALL build.bat %Type%
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL%)

:ERROR