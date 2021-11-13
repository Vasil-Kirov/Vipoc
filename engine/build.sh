#!/bin/bash



Files=$(find . -type f -name \*.c)


CompilerFlags=""
if [ "$1" = "debug" ]; then
    CompilerFlags="--debug -shared -Wvarargs -Wall -DVIPOC_DEBUG"
elif [ "$1" = "release" ]; then
	echo "-------- RELEASE --------"
	CompilerFlags="-O3 -shared -Wvarargs -Wall -Werror"
else
    echo "UNKNOWN BUILD TYPE"

fi
 

assembly="vipoc"
Defines="-DVIPOC_EXPORT -DVIPOC_LINUX -D_CRT_SECURE_NO_WARNINGS"
CompilerFlags+=" -ffast-math"
Includes="-Isrc"
Libraries="-L../bin/ -lvipoc.lib -lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/X11R6/lib "

echo "Building $assembly..."
clang $Files $CompilerFlags -o ../bin/$assembly.so $Defines $Includes $Libraries