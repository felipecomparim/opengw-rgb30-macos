#!/bin/bash
source /etc/profile
cd /roms/ports
/usr/bin/timeout 45 bash ./OpenGW.sh --start-game --frames 360
result=$?
tail -18 /roms/ports/opengw/log.txt
exit "$result"
