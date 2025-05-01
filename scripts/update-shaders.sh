#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -fr src/lib/tk-terminal/Common/Data
else
#    mkdir -p src/lib/tk-terminal/Common/Data
#    mkdir -p src/lib/tk-terminal/Common/Data/GLSL
#    build/scripts/compile-glsl.sh tk-terminal
    mkdir -p src/lib/tk-terminal/Common/Data
fi
