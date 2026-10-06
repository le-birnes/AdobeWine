#!/usr/bin/env bash
# Runs inside a distribution container: installs that distribution's documented runtime
# dependencies (docs/INSTALL.md, docs/INSTALL-ARCH.md), unpacks the release tarball and
# checks that the runtime loads and can create a prefix. Used by tests/distro/check.sh.
set -uo pipefail
distro="$1"
step() { echo "== $*"; }
ok=1

step "install documented runtime dependencies ($distro)"
case "$distro" in
  debian*|ubuntu*)
    export DEBIAN_FRONTEND=noninteractive
    # Debian keeps winetricks in "contrib" (documented in docs/INSTALL.md)
    [ -f /etc/apt/sources.list.d/debian.sources ] && sed -i 's/^Components: main$/Components: main contrib/' /etc/apt/sources.list.d/debian.sources
    apt-get update -qq >/dev/null
    apt-get install -y -qq zstd xz-utils procps >/dev/null
    apt-get install -y -qq winetricks curl zstd libvulkan1 libgnutls30 libpulse0 libasound2t64 \
      gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
      gstreamer1.0-libav libfreetype6 libfontconfig1 libunwind8 libxkbcommon0 p7zip-full python3-pil \
      >/tmp/deps.log 2>&1 || { tail -5 /tmp/deps.log; ok=0; } ;;
  fedora*)
    dnf -y -q install zstd tar procps-ng >/dev/null 2>&1
    dnf -y -q install winetricks curl zstd vulkan-loader gnutls pulseaudio-libs alsa-lib \
      gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-bad-free \
      freetype fontconfig libunwind libxkbcommon p7zip python3-pillow \
      >/tmp/deps.log 2>&1 || { tail -5 /tmp/deps.log; ok=0; } ;;
  opensuse*)
    zypper -q -n install zstd tar gzip procps >/dev/null 2>&1
    zypper -q -n install winetricks curl zstd libvulkan1 libgnutls30 libpulse0 libasound2 \
      gstreamer-plugins-base gstreamer-plugins-good gstreamer-plugins-bad \
      libfreetype6 fontconfig libunwind8 libxkbcommon0 7zip python313-Pillow \
      >/tmp/deps.log 2>&1 || { tail -5 /tmp/deps.log; ok=0; } ;;
  arch*)
    pacman -Sy --noconfirm --needed zstd procps-ng >/dev/null 2>&1
    pacman -S --noconfirm --needed desktop-file-utils fontconfig freetype2 gettext glib2 libpcap \
      libunwind libx11 libxcursor libxext libxkbcommon libxi libxrandr wayland \
      vulkan-icd-loader gnutls alsa-lib libpulse gst-plugins-base-libs winetricks curl \
      gst-plugins-good gst-plugins-bad gst-libav p7zip python-pillow \
      >/tmp/deps.log 2>&1 || { tail -5 /tmp/deps.log; ok=0; } ;;
esac
[ "$ok" = 1 ] && echo "deps: OK"

step "system"
. /etc/os-release; echo "os: $PRETTY_NAME"
echo "glibc: $(ldd --version | head -1 | sed 's/.* //')"

step "unpack release"
tar -C / -xf /release/adobewine.tar.zst && echo "unpack: OK"
R=/opt/adobewine/runtime

step "missing shared libraries"
# ntdll.so and win32u.so are Wine's own and are loaded by Wine itself, not through ldd's path
missing="$(for f in "$R"/lib/wine/x86_64-unix/*.so; do ldd "$f" 2>/dev/null | grep 'not found' | grep -vE '(ntdll|win32u)\.so ' | sed "s|^|$(basename "$f"): |"; done | sort -u)"
if [ -n "$missing" ]; then echo "$missing"; else echo "missing: none"; fi

step "run as a normal user"
useradd -m tester 2>/dev/null
su tester -c '
  export PATH=/opt/adobewine/runtime/bin:$PATH WINEPREFIX=$HOME/pfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml=" DISPLAY=
  echo "wine: $(wine --version)"
  timeout 300 wineboot -i >/dev/null 2>&1; echo "wineboot exit: $?"
  echo "64-bit: $(timeout 60 wine cmd /c echo ok64 2>/dev/null | tr -d "\r")"
  echo "32-bit (WoW64): $(timeout 60 wine C:\\windows\\syswow64\\cmd.exe /c echo ok32 2>/dev/null | tr -d "\r")"
  echo "version: $(timeout 60 wine cmd /c ver 2>/dev/null | tr -d "\r" | grep -v "^$")"
  /opt/adobewine/bin/adobewine --help >/dev/null 2>&1 && echo "adobewine --help: OK"
  wineserver -k 2>/dev/null
'
