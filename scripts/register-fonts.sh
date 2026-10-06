#!/usr/bin/env bash
# Register every font file in the prefix's C:\windows\Fonts under
# HKLM\Software\Microsoft\Windows NT\CurrentVersion\Fonts, as Windows does at install.
# Wine's GDI scans the folder, but DirectWrite only lists fonts named in that key, so
# copied fonts (for example Segoe UI from your own Windows install) were invisible to DirectWrite apps: Camera Raw then
# skips text whose family (e.g. "Segoe UI Historic") is not in the system collection.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/bin/env.sh" >/dev/null
FONTS="$WINEPREFIX/drive_c/windows/Fonts"
KEY='HKLM\Software\Microsoft\Windows NT\CurrentVersion\Fonts'
known="$(wine reg query "$KEY" 2>/dev/null | tr -d '\r' | awk -F'REG_SZ' 'NF>1{gsub(/^ +| +$/,"",$2); n=split($2,a,"\\"); print tolower(a[n])}')"
reg="$(mktemp --suffix=.reg)"
printf 'Windows Registry Editor Version 5.00\r\n\r\n[%s]\r\n' "${KEY/HKLM/HKEY_LOCAL_MACHINE}" > "$reg"
count=0
for f in "$FONTS"/*; do
  b="$(basename "$f")"
  case "${b,,}" in *.ttf|*.ttc|*.otf) ;; *) continue ;; esac
  grep -qxF "${b,,}" <<<"$known" && continue
  name="$(fc-scan --format '%{fullname[0]}' "$f" 2>/dev/null | head -1)"
  [ -n "$name" ] || name="${b%.*}"
  case "${b,,}" in *.otf) kind=OpenType ;; *) kind=TrueType ;; esac
  printf '"%s (%s)"="%s"\r\n' "${name//\"/}" "$kind" "$b" >> "$reg"
  count=$((count + 1))
done
if [ "$count" -gt 0 ]; then
  wine reg import "$(winepath -w "$reg")" >/dev/null 2>&1
  wineserver -w
fi
rm -f "$reg"
echo "registered $count font files"
