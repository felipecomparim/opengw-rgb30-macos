#!/bin/sh
set -eu
: "${SSHPASS:?Set SSHPASS for the target}"
: "${RGB30_HOST:?Set RGB30_HOST to the target IP address}"
target="$RGB30_HOST"
cp /output/build/opengw /output/package/opengw/opengw
strip /output/package/opengw/opengw
cp /work/ports/OpenGW.sh /output/package/OpenGW.sh
chmod +x /output/package/OpenGW.sh
tar -C /output/package -cf - OpenGW.sh opengw/opengw | sshpass -e ssh -o StrictHostKeyChecking=accept-new "root@$target" 'tar -C /roms/ports -xf -'
