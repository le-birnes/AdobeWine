#!/usr/bin/env bash
# Checks the AdobeWine Flatpak on several distributions in containers (needs Docker or Podman, runs them privileged
# because Flatpak's sandbox needs user namespaces):
#   tests/distro/flatpak-check.sh path/to/CCINSTALLER.flatpak [image ...]
# Each container installs its own distribution's Flatpak, installs CCINSTALLER.flatpak the way a user does (its
# runtime comes from Flathub; the containers share one Flatpak folder, so it is downloaded only once), runs
# `adobewine setup` and starts a 64-bit and a 32-bit Windows program. Graphics and the Adobe apps need a real GPU
# and desktop and are not tested here.
set -uo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
engine="$(command -v docker || command -v podman)"
bundle="$(realpath "$1")"; shift
store="${STORE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/adobewine-flatpak-ci}"
images=("$@")
[ ${#images[@]} -gt 0 ] || images=(fedora:43 ubuntu:24.04 debian:13 linuxmintd/mint22-amd64 opensuse/leap:16.0 archlinux:latest)
mkdir -p "$store"
for img in "${images[@]}"; do
  echo "################ $img"
  "$engine" run --rm --privileged -v "$bundle:/b/CCINSTALLER.flatpak:ro" -v "$store:/var/lib/flatpak" -v "$here:/t:ro" "$img" bash /t/flatpak-in-container.sh
done
