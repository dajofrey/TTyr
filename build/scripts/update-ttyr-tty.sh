#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -f src/lib/ttyr-tty/Common/Data/ttyr-tty.conf.inc
    rm -fr src/lib/ttyr-tty/Common/Data
else
    mkdir -p src/lib/ttyr-tty/Common/Data
    xxd -i -n ttyr_tty_conf_inc build/data/misc/ttyr-tty.conf src/lib/ttyr-tty/Common/Data/ttyr-tty.conf.inc
fi
