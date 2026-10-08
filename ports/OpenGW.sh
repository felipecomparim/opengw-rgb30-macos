#!/bin/bash
# OpenGW / ROCKNIX / PortMaster
controlfolder=""
for candidate in /opt/system/Tools/PortMaster /opt/tools/PortMaster /roms/ports/PortMaster /roms2/ports/PortMaster; do
    if [ -f "$candidate/control.txt" ]; then
        controlfolder="$candidate"
        break
    fi
done
if [ -z "$controlfolder" ]; then
    echo "PortMaster control.txt not found" >&2
    exit 1
fi
source "$controlfolder/control.txt"
if [ -x /storage/.config/PortMaster/mapper.txt ]; then
    get_controls
else
    # Older ROCKNIX control.txt can point to a mapper that is not installed.
    # SDL still reads PortMaster's complete database below.
    sdl_controllerconfig="${SDL_GAMECONTROLLERCONFIG:-}"
fi
gamedir="$(dirname "$(readlink -f "$0")")/opengw"
cd "$gamedir" || exit 1
export OPENGW_DATA_DIR="$gamedir"
export SDL_GAMECONTROLLERCONFIG="${sdl_controllerconfig:-${SDL_GAMECONTROLLERCONFIG:-}}"
export SDL_GAMECONTROLLERCONFIG_FILE="$controlfolder/gamecontrollerdb.txt"
export LD_LIBRARY_PATH="$gamedir/gl4es.aarch64:$gamedir/libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# GL4ES supplies the OpenGL 2.1 fixed-function calls on GLES devices.
export LIBGL_ES=2
export LIBGL_GL=21
export LIBGL_FB=4
export LIBGL_NOTEST=1
# Use the stock performance governors while playing, and restore them on exit.
cpu_governor=/sys/devices/system/cpu/cpufreq/policy0/scaling_governor
gpu_governor=/sys/devices/platform/fde60000.gpu/devfreq/fde60000.gpu/governor
cpu_previous="$(cat "$cpu_governor" 2>/dev/null)"
gpu_previous="$(cat "$gpu_governor" 2>/dev/null)"
restore_governors() {
    [ -n "$cpu_previous" ] && [ -w "$cpu_governor" ] && echo "$cpu_previous" > "$cpu_governor"
    [ -n "$gpu_previous" ] && [ -w "$gpu_governor" ] && echo "$gpu_previous" > "$gpu_governor"
}
trap restore_governors EXIT
[ -w "$cpu_governor" ] && echo performance > "$cpu_governor"
[ -w "$gpu_governor" ] && echo performance > "$gpu_governor"
./opengw "$@" > "$gamedir/log.txt" 2>&1
result=$?
if declare -F pm_finish >/dev/null; then pm_finish; fi
exit "$result"
