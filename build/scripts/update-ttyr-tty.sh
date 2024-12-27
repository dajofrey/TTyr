#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -f src/lib/ttyr-tty/Common/Data/default.conf.inc
    rm -fr src/lib/ttyr-tty/Common/Data
else
    mkdir -p src/lib/ttyr-tty/Common/Data
    xxd -i -n ttyr_default_conf_inc build/data/misc/default.conf src/lib/ttyr-tty/Common/Data/default.conf.inc
fi
