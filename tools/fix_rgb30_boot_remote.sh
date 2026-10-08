#!/bin/sh
set -eu
# This image starts Sway before the late-loaded Mali device appears.
test -f /usr/lib/mali/libEGL.so.1
mkdir -p /storage/.config/system.d/sway.service.d
cat > /storage/.config/system.d/wait-rgb30-mali.sh <<'WAIT'
#!/bin/sh
remaining=30
while [ ! -c /dev/mali0 ]; do
    if [ "$remaining" -eq 0 ]; then
        echo "Mali GPU device did not appear; retry Sway startup" >&2
        exit 1
    fi
    sleep 1
    remaining=$((remaining - 1))
done
WAIT
chmod 755 /storage/.config/system.d/wait-rgb30-mali.sh
cat > /storage/.config/system.d/sway.service.d/90-rgb30-mali.conf <<'CONF'
[Service]
Environment=LD_LIBRARY_PATH=/usr/lib/mali
ExecStartPre=/storage/.config/system.d/wait-rgb30-mali.sh
CONF
systemctl daemon-reload
/storage/.config/system.d/wait-rgb30-mali.sh
systemctl show sway.service -p ExecStartPre
systemctl is-active sway.service essway.service
