@ECHO OFF

SetLocal EnableDelayedExpansion

SET Files=
FOR /R %%f in (*.c) do (
    SET Files=!Files! %%f
)

IF [%1] == [] (
    SET CompilerFlags=-g -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE IF [%1] == [release] (
    ECHO -------- RELEASE --------
    SET CompilerFlags=-O3 -Warargs -Wall -Werror
) ELSE IF [%1] == [debug] (
    SET CompilerFlags=-g -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE (
    ECHO ERROR: unknown build type '%1'
    GOTO :ERROR
)

SET assembly=game
SET Defines=-DVIPOC_IMPORT -DVIPOC_WIN32 -D_CRT_SECURE_NO_WARNINGS
SET CompilerFlags=%CompilerFlags% -ffast-math
SET Includes=-Isrc -I../engine/src
Set Libraries=-L../bin/ -lvipoc.lib -luser32




ECHO "Building %assembly%..."
clang %Files% %CompilerFlags% -o ../bin/%assembly%.exe %defines% %Includes% %Libraries%
