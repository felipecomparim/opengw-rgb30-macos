#!/bin/sh
set -eu
# Temporary (until reboot) compositor override for a libmali ROCKNIX image
# whose compositor loaded Mesa and fell back to the pixman renderer.
test -f /usr/lib/mali/libEGL.so.1
mkdir -p /run/systemd/system/sway.service.d
cat > /run/systemd/system/sway.service.d/90-rgb30-mali.conf <<'EOF'
[Service]
Environment=LD_LIBRARY_PATH=/usr/lib/mali
EOF
systemctl daemon-reload
systemctl restart sway.service essway.service
