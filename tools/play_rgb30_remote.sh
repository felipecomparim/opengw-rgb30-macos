#!/bin/bash
source /etc/profile
systemctl stop essway.service
nohup bash -c 'trap "systemctl start essway.service" EXIT; bash /roms/ports/OpenGW.sh --menu' >/tmp/opengw-launch.log 2>&1 </dev/null &
