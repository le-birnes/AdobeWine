#!/usr/bin/env bash
# Maintainers: build the prebuilt release tarball for the adobewine-bin AUR package.
#   cd packaging/aur/adobewine && makepkg -f && ../../../scripts/make-release.sh
set -euo pipefail
cd "$(dirname "$0")/../packaging/aur/adobewine"
ver="$(grep -m1 '^pkgver=' PKGBUILD | cut -d= -f2)"
out="$PWD/adobewine-$ver-x86_64.tar.zst"
tar -C pkg/adobewine -I 'zstd -19 -T0' -cf "$out" opt
sha256sum "$out"
echo "upload $out to the v$ver GitHub release, then put its sha256 in packaging/aur/adobewine-bin/PKGBUILD"
