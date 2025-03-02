#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -fr src/lib/ttyr-terminal/Common/Data
else
#    mkdir -p src/lib/ttyr-terminal/Common/Data
#    mkdir -p src/lib/ttyr-terminal/Common/Data/GLSL
#    build/scripts/compile-glsl.sh ttyr-terminal
    mkdir -p src/lib/ttyr-terminal/Common/Data
fi
