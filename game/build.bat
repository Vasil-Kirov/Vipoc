@ECHO OFF

SetLocal EnableDelayedExpansion


IF [%1] == [] (
    SET CompilerFlags=-O0 -g -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE IF [%1] == [release] (
    ECHO -------- RELEASE --------
    SET CompilerFlags=-O3 -Wvarargs -Wall -Werror
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
Set Libraries=-L../bin/ -lVipoc.lib -luser32



ECHO "Building hot reload DLL"
clang++ src/hot_reload.cpp -shared %CompilerFlags% -o../bin/hot_reload.dll %Includes% %Defines%

ECHO "Building %assembly%..."
clang++ src/main.cpp %CompilerFlags% -o ../bin/%assembly%.exe %defines% %Includes% %Libraries% -Xlinker -MAP 
