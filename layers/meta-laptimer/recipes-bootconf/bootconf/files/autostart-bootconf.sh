#!/bin/sh
#
# Called at system startup
#
# Attempt to start /opt/bootconf/bootconf.py
bootconf_cmd=/opt/bootconf/bootconf.py

if [ "$1" = "start" ]; then
    if [ -e $bootconf_cmd ]; then
        # this rather complex dance is needed
        # to ensure the process is not
        # blocking
        ($bootconf_cmd /boot/system.conf >/dev/null 2>&1) &
    fi
elif [ "$1" = "stop" ]; then
    kill $(pgrep bootconf)
fi
