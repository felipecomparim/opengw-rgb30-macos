#!/bin/bash
source /etc/profile
systemctl stop essway.service
trap 'systemctl start essway.service' EXIT
cd /roms/ports
/usr/bin/timeout 60 bash ./OpenGW.sh --benchmark --frames 600
result=$?
grep -E 'Performance|Rendered|Renderer' /roms/ports/opengw/log.txt
exit "$result"
