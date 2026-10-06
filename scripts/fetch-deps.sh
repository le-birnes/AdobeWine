#!/usr/bin/env bash
# Download the pinned graphics/GPU translation layers into $ADOBEWINE_HOME/deps and check them.
set -euo pipefail
source "$(cd "$(dirname "$0")/.." && pwd)/bin/env.sh"
D="$ADOBEWINE_HOME/deps"; mkdir -p "$D"; cd "$D"
fetch() {  # fetch <url> <sha256>
  local f="${1##*/}"
  if [ ! -f "$f" ]; then
    curl -fL --retry 3 -o "$f.part" "$1"
    mv "$f.part" "$f"
  fi
  echo "$2  $f" | sha256sum -c --quiet || { echo "checksum mismatch: $f" >&2; rm -f "$f"; exit 1; }
  tar -xf "$f"
}
# DXVK: Direct3D 8-11 -> Vulkan
fetch https://github.com/doitsujin/dxvk/releases/download/v3.1.1/dxvk-3.1.1.tar.gz \
      40565b4a724aadc4433fa4e010b4b23916d9b1f1baeee64e17186db94f54e608
# vkd3d-proton: Direct3D 12 -> Vulkan
fetch https://github.com/HansKristian-Work/vkd3d-proton/releases/download/v3.0.1/vkd3d-proton-3.0.1.tar.zst \
      3cf2315522af5e43605ef6d3c41dad91387040bf97199934f3f7ab76caaa2f0c
# nvidia-libs: CUDA / NVENC / NVAPI bridges to the Linux NVIDIA driver (used only on NVIDIA)
fetch https://github.com/SveSop/nvidia-libs/releases/download/v1.0.2/nvidia-libs-v1.0.2.tar.xz \
      01e8bb6368d088e22d8e8f1d02497214e8db436476021725ef0c0707b7cb1738
echo "dependencies ready in $D"
