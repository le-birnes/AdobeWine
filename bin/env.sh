# shellcheck shell=bash
# AdobeWine environment. Source this file; it never touches a system-wide Wine.
#   ADOBEWINE_HOME   where the runtime, dependencies and prefix live
#                    (default: ${XDG_DATA_HOME:-~/.local/share}/adobewine)
#   WINEPREFIX       the Windows prefix (default: $ADOBEWINE_HOME/prefix)
ADOBEWINE_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ADOBEWINE_HOME="${ADOBEWINE_HOME:-${XDG_DATA_HOME:-$HOME/.local/share}/adobewine}"
export ADOBEWINE_SRC ADOBEWINE_HOME
export WINEPREFIX="${WINEPREFIX:-$ADOBEWINE_HOME/prefix}"
# Runtime: the packaged one (e.g. /opt/adobewine/runtime from the AUR package) or the one
# scripts/build-wine.sh built into $ADOBEWINE_HOME/runtime.
if [ -x "$ADOBEWINE_SRC/runtime/bin/wine" ]; then ADOBEWINE_RUNTIME="$ADOBEWINE_SRC/runtime"
else ADOBEWINE_RUNTIME="$ADOBEWINE_HOME/runtime"; fi
export ADOBEWINE_RUNTIME
export PATH="$ADOBEWINE_RUNTIME/bin:$PATH"
export WINEARCH=win64
export WINEDEBUG="${WINEDEBUG:--all,+err}"
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:-winemenubuilder.exe=d}"
mkdir -p "$ADOBEWINE_HOME/logs"
export DXVK_LOG_PATH="$ADOBEWINE_HOME/logs"

# Hybrid laptops (Intel/AMD iGPU + NVIDIA dGPU): render on the NVIDIA GPU.
# Set ADOBEWINE_NVIDIA_OFFLOAD=0 to turn this off, =1 to force it.
if [ "${ADOBEWINE_NVIDIA_OFFLOAD:-auto}" = 1 ] || { [ "${ADOBEWINE_NVIDIA_OFFLOAD:-auto}" = auto ] &&
     [ -e /proc/driver/nvidia/version ] && [ "$(ls -d /sys/class/drm/card[0-9] 2>/dev/null | wc -l)" -gt 1 ]; }; then
  export __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia __VK_LAYER_NV_optimus=NVIDIA_only
fi
[ -e /proc/driver/nvidia/version ] && export DXVK_ENABLE_NVAPI=1

# Creative Cloud and its helpers write to heap blocks after freeing them; patch 0032 keeps
# freed blocks out of reuse for a while in these processes only.
export WINE_HEAP_QUARANTINE="${WINE_HEAP_QUARANTINE-Creative Cloud.exe;Creative Cloud Helper.exe;Creative Cloud UI Helper.exe;Adobe Desktop Service.exe;node.exe;CCXProcess.exe;CoreSync.exe;CRLogTransport.exe;HDHelper.exe;AdobeIPCBroker.exe;AdobeUpdateService.exe}"
