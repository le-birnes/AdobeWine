#!/usr/bin/env bash
# Maintainers: build the Flatpak and the one file users download, CCINSTALLER.flatpak.
# Double-clicking it opens GNOME Software / KDE Discover ("Install"), which also fetches the GNOME runtime from
# Flathub; "Open" then starts the first-run window, which sets everything up and ends at Adobe's sign-in.
#   packaging/flatpak/make-bundle.sh [build-dir] [repo-dir]     (STATE_DIR=... keeps the build cache elsewhere)
set -euo pipefail
cd "$(dirname "$0")/../.."
ID=io.github.le_birnes.AdobeWine
BUILD="${1:-build-flatpak}" REPO="${2:-flatpak-repo}"
ver="$(cat VERSION)"
# Every build command must be text: YAML reads an unquoted ": " as a mapping, and flatpak-builder skips it silently.
python3 -c "import sys, yaml; m = yaml.safe_load(open(sys.argv[1])); bad = [c for x in m['modules'] for c in x.get('build-commands', []) if not isinstance(c, str)]; sys.exit('build commands YAML did not read as text: %s' % bad if bad else 0)" "packaging/flatpak/$ID.yml"
flatpak-builder --user --force-clean --state-dir="${STATE_DIR:-.flatpak-builder}" --repo="$REPO" "$BUILD" "packaging/flatpak/$ID.yml"
flatpak build-bundle --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo "$REPO" "CCINSTALLER.flatpak" "$ID"
ls -la "CCINSTALLER.flatpak"
