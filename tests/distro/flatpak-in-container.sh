#!/usr/bin/env bash
# Inside one container (see flatpak-check.sh): the distribution's Flatpak, CCINSTALLER.flatpak, setup, 64/32-bit.
# A system installation, because Flatpak refuses a per-user one as root.
set -uo pipefail
export DEBIAN_FRONTEND=noninteractive
. /etc/os-release; echo "distribution: $PRETTY_NAME"
if command -v apt-get >/dev/null; then apt-get update -qq && apt-get install -y -qq flatpak ca-certificates >/dev/null
elif command -v dnf >/dev/null; then dnf install -y -q flatpak >/dev/null
elif command -v zypper >/dev/null; then zypper -n refresh >/dev/null; zypper -n install flatpak >/tmp/pkg.log 2>&1 || tail -5 /tmp/pkg.log
elif command -v pacman >/dev/null; then pacman -Sy --noconfirm --needed flatpak >/dev/null; fi
echo "flatpak: $(flatpak --version)"
flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
ok=0
# The store is shared between containers: remove the previous one's app (Flatpak 1.14 cannot --reinstall a bundle).
flatpak uninstall -y --noninteractive io.github.le_birnes.AdobeWine >/dev/null 2>&1
flatpak install -y --noninteractive /b/CCINSTALLER.flatpak >/tmp/install.log 2>&1 \
  && echo "PASS install" || { echo "FAIL install"; tail -5 /tmp/install.log; exit 1; }
# A desktop always has the system D-Bus, and Mint's Flatpak cannot start an app without it; a bare container has
# none. Started only when needed: with it, Arch's Flatpak asks for an accounts service the container cannot start.
flatpak run --command=true io.github.le_birnes.AdobeWine 2>/dev/null \
  || { mkdir -p /run/dbus; dbus-daemon --system --fork; echo "(system D-Bus started)"; }
run() { flatpak run --command=bash io.github.le_birnes.AdobeWine -c "$1"; }
run 'adobewine setup >/tmp/setup.log 2>&1; rc=$?; [ $rc = 0 ] && tail -3 /tmp/setup.log || { echo "setup exit $rc"; grep -v "^[0-9a-f]*:\(fixme\|warn\|trace\)" /tmp/setup.log | tail -25; }; exit $rc' && echo "PASS setup" || { echo "FAIL setup"; ok=1; }
run 'WINEDEBUG=-all wine cmd /c echo hello64 2>/dev/null' | grep -q hello64 && echo "PASS 64-bit" || { echo "FAIL 64-bit"; ok=1; }
run 'WINEDEBUG=-all wine C:\\windows\\syswow64\\cmd.exe /c echo hello32 2>/dev/null' | grep -q hello32 && echo "PASS 32-bit" || { echo "FAIL 32-bit"; ok=1; }
run 'ls "$XDG_DATA_HOME/adobewine/prefix/drive_c/windows/Fonts/segoeui.ttf" >/dev/null' && echo "PASS Segoe UI (from Selawik)" || { echo "FAIL Segoe UI"; ok=1; }
exit $ok
