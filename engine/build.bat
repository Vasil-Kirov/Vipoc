@ECHO OFF


SetLocal EnableDelayedExpansion


SET Files=
FOR /R %%f in (*.c) do (
	SET Files=!Files! %%f
)

IF [%1] == [] (
	SET CompilerFlags=--debug -shared -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE IF [%1] == [release] (
	ECHO -------- RELEASE --------
	SET CompilerFlags=-O3 -shared -Wvarargs -Wall -Werror
) ELSE IF [%1] == [debug] (
	SET CompilerFlags=--debug -shared -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE (
	ECHO ERROR: unknown build type '%1'
	GOTO :ERROR
)


	SET assembly=Vipoc
	SET Defines=-DVIPOC_EXPORT -DVIPOC_WIN32 -D_CRT_SECURE_NO_WARNINGS
	SET CompilerFlags=%CompilerFlags% -ffast-math -mavx
	SET Includes=-Isrc
	Set Libraries=-luser32 -lGdi32 -lOpenGL32



	ECHO "Building %assembly%..."
	clang %Files% %CompilerFlags% -o ../bin/%assembly%.dll %Defines% %Includes% %Libraries% -Xlinker -MAP 


