#!/usr/bin/env bash
# Create or refresh the AdobeWine prefix. Safe to run again.
# Usage: scripts/setup-prefix.sh [--windows-fonts DIR]
#   --windows-fonts DIR   copy fonts from your own Windows installation (its C:\Windows\Fonts
#                         folder, e.g. a mounted Windows partition). Strongly recommended:
#                         Adobe's UI asks for Segoe UI, and Photoshop's menu bar crashes without it.
set -euo pipefail
source "$(cd "$(dirname "$0")/.." && pwd)/bin/env.sh"
WINFONTS=""
while [ $# -gt 0 ]; do
  case "$1" in
    --windows-fonts) WINFONTS="$2"; shift 2 ;;
    *) echo "unknown option $1" >&2; exit 1 ;;
  esac
done
[ -x "$ADOBEWINE_RUNTIME/bin/wine" ] || { echo "no AdobeWine runtime: install the adobewine package or run scripts/build-wine.sh" >&2; exit 1; }
command -v winetricks >/dev/null || { echo "winetricks is required (install it from your distribution)" >&2; exit 1; }
DEPS="$ADOBEWINE_HOME/deps"
ls -d "$DEPS"/dxvk-*/ >/dev/null 2>&1 || "$ADOBEWINE_SRC/scripts/fetch-deps.sh"
SYS="$WINEPREFIX/drive_c/windows/system32"
SYS32="$WINEPREFIX/drive_c/windows/syswow64"

[ -f "$WINEPREFIX/system.reg" ] || wineboot -i
wineserver -w

# Browser engine (wine-gecko) and .NET replacement (wine-mono): the Creative Cloud
# installer's pages stay on "Loading..." without gecko.
SHARE="$ADOBEWINE_HOME/cache"
mkdir -p "$SHARE/gecko" "$SHARE/mono"
for a in x86 x86_64; do
  f="$SHARE/gecko/wine-gecko-2.47.4-$a.msi"
  [ -f "$f" ] || curl -fsSL -o "$f" "https://dl.winehq.org/wine/wine-gecko/2.47.4/wine-gecko-2.47.4-$a.msi"
done
[ -d "$SYS/gecko/2.47.4" ] || { wine msiexec /i "$SHARE/gecko/wine-gecko-2.47.4-x86_64.msi" /qn; wine msiexec /i "$SHARE/gecko/wine-gecko-2.47.4-x86.msi" /qn; }
f="$SHARE/mono/wine-mono-11.3.0-x86.msi"
[ -f "$f" ] || curl -fsSL -o "$f" "https://dl.winehq.org/wine/wine-mono/11.3.0/wine-mono-11.3.0-x86.msi"
[ -d "$WINEPREFIX/drive_c/windows/mono" ] || wine msiexec /i "$f" /qn

# Microsoft's shader compiler (Audition compiles HLSL at run time) and the core fonts
# (the Creative Cloud renderer crashes without real Windows fonts).
[ -f "$WINEPREFIX/.adobewine-d3dcompiler" ] || { winetricks -q d3dcompiler_47 && touch "$WINEPREFIX/.adobewine-d3dcompiler"; }
[ -f "$WINEPREFIX/drive_c/windows/Fonts/arial.ttf" ] || winetricks -q corefonts tahoma
# WinRT classes added by the patches (0031 ToastNotificationManager) need registering.
wine regsvr32 /s windows.ui.dll

override() { wine reg add 'HKCU\Software\Wine\DllOverrides' /v "$1" /d native,builtin /f >/dev/null; }
# DXVK: Direct3D 8-11 -> Vulkan
for d in d3d8 d3d9 d3d10core d3d11 dxgi; do
  cp "$DEPS"/dxvk-*/x64/$d.dll "$SYS/"; cp "$DEPS"/dxvk-*/x32/$d.dll "$SYS32/"; override $d
done
# vkd3d-proton: Direct3D 12 -> Vulkan, behind a small shim that retries
# D3D_FEATURE_LEVEL_1_0_CORE device creation at 11_0 (Adobe's AI features use core devices).
for d in d3d12 d3d12core; do
  cp "$DEPS"/vkd3d-proton-*/x64/$d.dll "$SYS/"; cp "$DEPS"/vkd3d-proton-*/x86/$d.dll "$SYS32/"; override $d
done
if [ -f "$ADOBEWINE_SRC/lib/d3d12.dll" ]; then
  mv "$SYS/d3d12.dll" "$SYS/d3d12_vkd3d.dll"; cp "$ADOBEWINE_SRC/lib/d3d12.dll" "$SYS/d3d12.dll"
elif command -v x86_64-w64-mingw32-gcc >/dev/null; then
  mv "$SYS/d3d12.dll" "$SYS/d3d12_vkd3d.dll"
  x86_64-w64-mingw32-gcc -O2 -shared -o "$SYS/d3d12.dll" "$ADOBEWINE_SRC/shims/d3d12/d3d12shim.c" \
      "$ADOBEWINE_SRC/shims/d3d12/d3d12.def" -Wl,--enable-stdcall-fixup
else
  echo "note: no MinGW compiler, skipping the d3d12 shim (Adobe AI features may be unavailable)" >&2
fi
# NVIDIA only: CUDA / NVENC / NVAPI / NVML bridges to the Linux driver.
if [ -e /proc/driver/nvidia/version ]; then
  "$(ls -d "$DEPS"/nvidia-libs-*/ | head -1)/setup_nvlibs.sh" install
fi

# Windows version: 11 (build 26100, patch 0018) for everything; Photoshop 27 divides by
# zero at launch on that build, so Photoshop alone reports Windows 10.
wine winecfg /v win11 >/dev/null 2>&1
wine reg add 'HKCU\Software\Wine\AppDefaults\Photoshop.exe' /v Version /d win10 /f >/dev/null
# WebView2 (the Home screens) presents through a DirectComposition swapchain; only Wine's
# own dxgi has composition swapchains (patches 0050, 0051), DXVK does not.
wine reg add 'HKCU\Software\Wine\AppDefaults\msedgewebview2.exe\DllOverrides' /v dxgi /d builtin /f >/dev/null
wine reg add 'HKCU\Software\Wine\AppDefaults\msedgewebview2.exe\DllOverrides' /v d3d11 /d builtin /f >/dev/null
# X11 driver (through XWayland on Wayland desktops), and no window-manager decorations:
# Adobe apps start maximised and a title bar would force a resize that leaves panels undrawn.
wine reg add 'HKCU\Software\Wine\Drivers' /v Graphics /d x11 /f >/dev/null
wine reg add 'HKCU\Software\Wine\X11 Driver' /v Decorated /d N /f >/dev/null
wine reg add 'HKCU\Software\Wine\WineDbg' /v ShowCrashDialog /t REG_DWORD /d 0 /f >/dev/null

# Fonts from your own Windows installation (Segoe UI and friends).
if [ -n "$WINFONTS" ]; then
  [ -d "$WINFONTS" ] || { echo "not a folder: $WINFONTS" >&2; exit 1; }
  cp -n "$WINFONTS"/*.[tToO][tT][fFcC] "$WINEPREFIX/drive_c/windows/Fonts/" 2>/dev/null || true
elif ! ls "$WINEPREFIX/drive_c/windows/Fonts/"segoeui.ttf >/dev/null 2>&1; then
  # No Windows fonts: Segoe UI made from Selawik, Microsoft's open metric-compatible
  # fallback, if the system has it. It must really be named "Segoe UI": DirectWrite ignores
  # FontSubstitutes, and Photoshop's menu bar hangs the UI without that family.
  mapfile -t selawik < <(fc-list -f '%{file}\n' :family=Selawik 2>/dev/null)
  if [ "${#selawik[@]}" -gt 0 ] &&
     python3 "$ADOBEWINE_SRC/scripts/selawik-as-segoe.py" "$WINEPREFIX/drive_c/windows/Fonts" "${selawik[@]}" >/dev/null; then
    echo "note: no Windows fonts given; Segoe UI was made from Selawik." >&2
  else
    echo "note: Segoe UI is missing. Run again with --windows-fonts <your Windows Fonts folder>," >&2
    echo "      or install the Selawik fonts and run setup again." >&2
  fi
fi
"$ADOBEWINE_SRC/scripts/register-fonts.sh"
wineserver -w
echo "prefix ready: $WINEPREFIX ($(wine --version))"
