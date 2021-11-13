#!/bin/bash
clear;


compile_type="";



if [ $# -eq 0 ]; then
    compile_type="debug";
elif [ "$1" = "release" ]; then
    echo --- RELEASE MODE ---
    compile_type="release";
else
    echo "UNKNOWN BUILD TYPE IN BUILD-ALL";
fi


pushd engine;
source build.sh $compiler_type;
popd;

pushd game;
source build.sh $compile_type;
popd;
