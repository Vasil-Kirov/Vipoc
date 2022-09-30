@ECHO OFF
CLS

Set Type=%1

PUSHD engine
CALL build.bat %Type%
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL%)

PUSHD game
CALL build.bat %Type%
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL%)

ECHO Done!

:ERROR