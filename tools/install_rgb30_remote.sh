#!/bin/sh
set -eu
# Run inside the ARM64 build container; SSHPASS is supplied externally.
: "${SSHPASS:?Set SSHPASS for the target}"
: "${RGB30_HOST:?Set RGB30_HOST to the target IP address}"
target="$RGB30_HOST"
sshpass -e ssh -o StrictHostKeyChecking=accept-new "root@$target" 'test ! -e /roms/ports/opengw && test ! -e /roms/ports/OpenGW.sh'
mkdir -p /output/package/opengw/gl4es.aarch64
cp /output/build/opengw /output/package/opengw/opengw
strip /output/package/opengw/opengw
cp -R /work/images /work/sounds /output/package/opengw/
cp /work/ports/OpenGW.sh /output/package/OpenGW.sh
chmod +x /output/package/OpenGW.sh
sshpass -e scp -O -o StrictHostKeyChecking=accept-new "root@$target:/roms/ports/area2048/gl4es.aarch64/libGL.so.1" /output/package/opengw/gl4es.aarch64/libGL.so.1
tar -C /output/package -cf - OpenGW.sh opengw | sshpass -e ssh -o StrictHostKeyChecking=accept-new "root@$target" 'tar -C /roms/ports -xf -'
sshpass -e ssh -o StrictHostKeyChecking=accept-new "root@$target" 'mkdir -p /storage/.config/system.d/sway.service.d; cp /run/systemd/system/sway.service.d/90-rgb30-mali.conf /storage/.config/system.d/sway.service.d/90-rgb30-mali.conf; systemctl daemon-reload; ldd /roms/ports/opengw/opengw'
