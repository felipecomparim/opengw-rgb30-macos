#!/bin/bash
source /etc/profile
systemctl stop essway.service
trap 'systemctl start essway.service' EXIT
/usr/bin/timeout 30 bash /roms/ports/OpenGW.sh --menu --frames 30 --screenshot /tmp/opengw-menu.bmp
result=$?
tail -3 /roms/ports/opengw/log.txt
exit "$result"
