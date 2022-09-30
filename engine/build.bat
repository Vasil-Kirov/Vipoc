@ECHO OFF


SetLocal EnableDelayedExpansion


SET Files=
FOR /R %%f in (*.c *.cpp) do (
	SET Files=!Files! %%f
)

IF [%1] == [] (
	SET CompilerFlags=-O1 -shared -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE IF [%1] == [release] (
	ECHO -------- RELEASE --------
	SET CompilerFlags=-O3 -shared -Wvarargs -Wall -Werror
) ELSE IF [%1] == [debug] (
	ECHO -------- DEBUG --------
	SET CompilerFlags=--debug -O0 -shared -Wvarargs -Wall -DVIPOC_DEBUG
) ELSE (
	ECHO ERROR: unknown build type '%1'
	GOTO :ERROR
)


	SET assembly=Vipoc
	SET Defines=-DVIPOC_EXPORT -DVIPOC_WIN32 -D_CRT_SECURE_NO_WARNINGS
	SET CompilerFlags=%CompilerFlags% -ffast-math -mavx -DRENDERER_GL
	SET Includes=-Isrc -I../include/cglm
	Set Libraries=-luser32 -lGdi32 -lOpenGL32 -ld3d12.lib -ldxgi.lib -ld3dcompiler.lib



	ECHO "Building %assembly%..."
	clang %Files% %CompilerFlags% -o ../bin/%assembly%.dll %Defines% %Includes% %Libraries% -Xlinker -MAP -Xlinker -INCREMENTAL
 


