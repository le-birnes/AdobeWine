#!/usr/bin/env bash
# Add the installed Adobe apps to your desktop's application menu (any freedesktop desktop:
# KDE, GNOME, Xfce, ...). Remove them again with --remove.
set -euo pipefail
source "$(cd "$(dirname "$0")/.." && pwd)/bin/env.sh"
APPS="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICONS="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/256x256/apps"
if [ "${1:-}" = --remove ]; then
  rm -f "$APPS"/adobewine-*.desktop "$ICONS"/adobewine-*.png
  update-desktop-database "$APPS" 2>/dev/null || true
  echo "launchers removed"; exit 0
fi
mkdir -p "$APPS" "$ICONS"
PF="$WINEPREFIX/drive_c/Program Files/Adobe"
icon() {  # icon <name> <exe>: largest icon from the .exe, or an .ico next to it
  local tmp; tmp="$(mktemp -d)"
  if command -v 7z >/dev/null; then 7z x -y -o"$tmp" "$2" '.rsrc/ICON/*' '.rsrc/*/ICON/*' >/dev/null 2>&1 || true; fi
  cp "$(dirname "$2")"/*Application_Icon*.ico "$tmp"/ 2>/dev/null || true
  python3 - "$tmp" "$ICONS/adobewine-$1.png" <<'PY' || true
import sys, glob, os
from PIL import Image
best = None
for f in glob.glob(os.path.join(sys.argv[1], '**', '*.ico'), recursive=True):
    try:
        im = Image.open(f); size = max(im.info.get('sizes', {im.size}))
        if not best or size > best[0]: best = (size, f)
    except Exception: pass
if best:
    im = Image.open(best[1]); im.size = best[0]; im.save(sys.argv[2])
PY
  rm -rf "$tmp"
}
entry() {  # entry <name> <title> <categories> <mime> <wmclass> <exe>
  [ -f "$6" ] || return 0
  icon "$1" "$6"
  cat > "$APPS/adobewine-$1.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$2 (AdobeWine)
Comment=$2 through the AdobeWine Wine build
Exec=$ADOBEWINE_SRC/bin/adobewine $1 %F
Icon=adobewine-$1
Terminal=false
Categories=$3
MimeType=$4
StartupWMClass=$5
EOF
  echo "added $2"
}
entry photoshop "Adobe Photoshop" "Graphics;2DGraphics;RasterGraphics;" "image/vnd.adobe.photoshop;image/png;image/jpeg;image/tiff;" photoshop.exe "$PF/Adobe Photoshop 2026/Photoshop.exe"
entry illustrator "Adobe Illustrator" "Graphics;2DGraphics;VectorGraphics;" "application/illustrator;application/postscript;image/svg+xml;application/pdf;" illustrator.exe "$PF/Adobe Illustrator 2026/Support Files/Contents/Windows/Illustrator.exe"
entry premiere "Adobe Premiere Pro" "AudioVideo;Video;AudioVideoEditing;" "" "adobe premiere pro.exe" "$PF/Adobe Premiere Pro 2026/Adobe Premiere Pro.exe"
entry audition "Adobe Audition" "AudioVideo;Audio;AudioVideoEditing;" "audio/x-wav;audio/wav;audio/mpeg;audio/flac;" "adobe audition.exe" "$PF/Adobe Audition 2026/Adobe Audition.exe"
entry encoder "Adobe Media Encoder" "AudioVideo;Video;" "" "adobe media encoder.exe" "$PF/Adobe Media Encoder 2026/Adobe Media Encoder.exe"
entry cc "Adobe Creative Cloud" "Graphics;" "" "creative cloud.exe" "$PF/Adobe Creative Cloud/ACC/Creative Cloud.exe"
cat > "$APPS/adobewine-off.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Stop all Adobe apps (AdobeWine)
Exec=$ADOBEWINE_SRC/bin/adobewine off
Icon=process-stop
Terminal=false
Categories=Utility;
EOF
update-desktop-database "$APPS" 2>/dev/null || true
echo "launchers installed in $APPS"
