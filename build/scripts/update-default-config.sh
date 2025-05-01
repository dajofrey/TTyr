#!/bin/sh

if [ "$1" = "clean" ]; then
    rm -f src/lib/tk-core/Common/Data/default.conf.inc
    rm -fr src/lib/tk-core/Common/Data
else
    mkdir -p src/lib/tk-core/Common/Data
    xxd -i -n termoskanne_default_conf_inc build/data/misc/default.conf src/lib/tk-core/Common/Data/default.conf.inc
fi
