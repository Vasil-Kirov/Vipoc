#!/bin/bash



Files=$(find . -type f -name \*.c)


CompilerFlags=""
if [ $1 = "debug" ]; then
    CompilerFlags="-g -Wvarargs -Wall -DVIPOC_DEBUG"
elif [ $1 = "release" ]; then
	echo "-------- RELEASE --------"
	CompilerFlags="-O3 -Warargs -Wall -Werror"
else
    echo "UNKNOWN BUILD TYPE"

fi

assembly="game"
Defines="-DVIPOC_IMPORT -DVIPOC_LINUX -D_CRT_SECURE_NO_WARNINGS"
CompilerFlags+=" -ffast-math"
Includes="-Isrc -I../engine/src"
Libraries="-L../bin/ -lengine -Wl, -rpath,."


echo "Building $assembly..."
clang $Files $CompilerFlags -o ../bin/$assembly $Defines $Includes $Libraries

