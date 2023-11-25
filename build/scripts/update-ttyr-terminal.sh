#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -f src/lib/ttyr-terminal/Common/Data/ttyr-terminal.conf.inc
    rm -fr src/lib/ttyr-terminal/Common/Data
else
#    mkdir -p src/lib/ttyr-terminal/Common/Data
#    mkdir -p src/lib/ttyr-terminal/Common/Data/GLSL
#    build/scripts/compile-glsl.sh ttyr-terminal
    mkdir -p src/lib/ttyr-terminal/Common/Data
    xxd -i -n ttyr_terminal_conf_inc build/data/misc/ttyr-terminal.conf src/lib/ttyr-terminal/Common/Data/ttyr-terminal.conf.inc
fi
