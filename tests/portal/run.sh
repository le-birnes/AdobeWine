#!/usr/bin/env bash
# Portal smoke test for patches 0060a/0060b, headless (Xvfb + private session bus):
#
#   tests/portal/run.sh <bin/wine> [outdir]
#
# 1. mock:  a mock FileChooser (mock_portal.py) answers immediately. Checks that GetOpenFileNameW,
#           GetSaveFileNameW and IFileOpenDialog reach the portal and return its file, that custom
#           controls arrive as "choices" (labels, empty-label fallback, items, initial values) and
#           that the portal's choices come back to the app before OnFileOk, and that a document
#           portal path (/.../doc/<id>/<name> with user.document-portal.host-path) is returned as
#           the host path.
# 2. real:  xdg-desktop-portal + xdg-desktop-portal-gtk (if installed): the GTK dialog is driven
#           with xdotool (type a path, Enter). Records whether the call reaches the portal
#           (dbus-monitor) and returns.
# 3. off:   without WINE_FORCE_PORTAL the portal must not be called (Wine dialog, closed by xdotool).
# Needs: dbus-run-session, dbus-monitor, xvfb-run, x86_64-w64-mingw32-gcc, python3 with dbus + gi,
# xdotool and setfattr for parts of it. Prints PASS/FAIL/SKIP lines; exit status = failures.
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
WINE="$(readlink -f "${1:?usage: run.sh <bin/wine> [outdir]}")"
OUT="$(mkdir -p "${2:-/tmp/adobewine-portal}" && cd "${2:-/tmp/adobewine-portal}" && pwd)"
PY="${PORTAL_PYTHON:-}"
for p in $PY /usr/bin/python3 /usr/bin/python3.12 python3; do
  "$p" -c 'import dbus, dbus.service, dbus.mainloop.glib; from gi.repository import GLib' 2>/dev/null && { PY=$p; break; }
done
if [ -z "${DISPLAY:-}" ] && [ -z "${PORTAL_UNDER_XVFB:-}" ]; then
  PORTAL_UNDER_XVFB=1 exec xvfb-run -a -s "-screen 0 1920x1080x24" "$0" "$WINE" "$OUT"
fi
WINESERVER="$(dirname "$WINE")/wineserver"
[ -x "$WINESERVER" ] || WINESERVER="$(dirname "$WINE")/../../server/wineserver"  # build tree (tools/wine/wine)
export WINEPREFIX="$OUT/prefix" WINEDEBUG="${WINEDEBUG:--all}" WINEDLLOVERRIDES="mscoree,mshtml=" WINEARCH=win64
fails=0
pass() { echo "PASS $1"; }
fail() { echo "FAIL $1: $2"; fails=$((fails + 1)); }
skip() { echo "SKIP $1: $2"; }

x86_64-w64-mingw32-gcc -O1 -municode -o "$OUT/portal.exe" "$HERE/portal.c" -lcomdlg32 -lole32 -luuid -lshell32 ||
  { echo "FAIL build portal.exe"; exit 1; }
[ -d "$WINEPREFIX" ] || { "$WINE" wineboot -i >/dev/null 2>&1; "$WINESERVER" -w; }

# files: a plain one, and a fake document-portal entry pointing at a host file
mkdir -p "$OUT/files/host" "$OUT/files/run/doc/0a1b2c3d"
echo frame >"$OUT/files/host/frame_0001.png"; echo frame >"$OUT/files/host/frame_0002.png"
cp "$OUT/files/host/frame_0001.png" "$OUT/files/run/doc/0a1b2c3d/frame_0001.png"
XATTR=0
if command -v setfattr >/dev/null &&
   setfattr -n user.document-portal.host-path -v "$OUT/files/host/frame_0001.png" \
     "$OUT/files/run/doc/0a1b2c3d/frame_0001.png" 2>/dev/null; then XATTR=1; fi
winpath() { echo "Z:$1" | tr / '\\'; }

# --- 1. mock portal
if [ -n "$PY" ]; then
  for t in legacy save item docportal; do
    uri="file://$OUT/files/host/frame_0001.png"
    [ $t = save ] && uri="file://$OUT/files/host/new%20project.aep"
    [ $t = docportal ] && uri="file://$OUT/files/run/doc/0a1b2c3d/frame_0001.png"
    arg=$t; [ $t = docportal ] && arg=item
    rm -f "$OUT/mock-$t.json"
    WINE_FORCE_PORTAL=1 timeout 120 dbus-run-session -- sh -c "
      '$PY' '$HERE/mock_portal.py' '$OUT/mock-$t.json' '$uri' >'$OUT/mock-$t.portal.log' 2>&1 &
      for i in \$(seq 50); do grep -q ready '$OUT/mock-$t.portal.log' && break; sleep 0.1; done
      dbus-monitor \"interface='org.freedesktop.portal.FileChooser'\" \"interface='org.freedesktop.portal.Request'\" >'$OUT/mock-$t.monitor' 2>&1 &
      sleep 0.5
      '$WINE' '$OUT/portal.exe' $arg" >"$OUT/mock-$t.txt" 2>"$OUT/mock-$t.err"
    tr -d '\r' <"$OUT/mock-$t.txt" >"$OUT/mock-$t.out"
    case $t in
      legacy) want="RESULT ok $(winpath "$OUT/files/host/frame_0001.png")" ;;
      save) want="RESULT ok $(winpath "$OUT/files/host/new project.aep")" ;;
      item) want="RESULT ok $(winpath "$OUT/files/host/frame_0001.png")" ;;
      docportal) want="RESULT ok $(winpath "$OUT/files/host/frame_0001.png")"
        [ $XATTR = 1 ] || { skip "mock $t" "no user xattrs here (setfattr)"; continue; } ;;
    esac
    if ! [ -s "$OUT/mock-$t.json" ]; then fail "mock $t" "portal not called ($(tail -n1 "$OUT/mock-$t.out"))"
    elif ! grep -qiF "$want" "$OUT/mock-$t.out"; then fail "mock $t" "$(tail -n1 "$OUT/mock-$t.out"), wanted '$want'"
    else pass "mock $t: $(tail -n1 "$OUT/mock-$t.out")"; fi
  done
  # custom controls: request payload and write-back
  if [ -s "$OUT/mock-item.json" ]; then
    if "$PY" - "$OUT/mock-item.json" <<'EOF'
import json, sys
o = json.loads(open(sys.argv[1]).readline())["options"]
ch = {c[0]: c for c in o.get("choices", [])}
ok = (ch.get("101") == ["101", "Import As:", [["1", "Footage"], ["2", "Composition"],
                        ["3", "Composition - Retain Layer Sizes"]], "2"]
      and ch.get("102") == ["102", "ImporterJPEG Sequence", [], "false"]
      and ch.get("103", [0, ""])[1] != "" and ch["103"][3] == "true"
      and ch.get("105") == ["105", "Format:", [["1", "PNG"], ["2", "TIFF"]], "1"]
      and "104" not in ch)
print(json.dumps(o.get("choices")))
sys.exit(0 if ok else 1)
EOF
    then pass "mock item choices sent (labels, text-control label, fallback label, items, initial values)"
    else fail "mock item choices sent" "$(cat "$OUT/mock-item.json")"; fi
    if grep -q 'OnFileOk: import-as item 3 (hr 0), sequence 1, no-label 0' "$OUT/mock-item.out"; then
      pass "mock item choices applied before OnFileOk"
    else fail "mock item choices applied before OnFileOk" "$(grep OnFileOk: "$OUT/mock-item.out" | tail -n1)"; fi
  fi
else
  skip "mock" "no python with dbus + gi"
fi

# --- 2. real portal (xdg-desktop-portal + gtk backend), dialog driven by xdotool
if [ -x /usr/libexec/xdg-desktop-portal ] && [ -x /usr/libexec/xdg-desktop-portal-gtk ] && command -v xdotool >/dev/null; then
  target="$OUT/files/host/frame_0002.png"
  WINE_FORCE_PORTAL=1 XDG_CURRENT_DESKTOP=GNOME GTK_USE_PORTAL=0 timeout 180 dbus-run-session -- sh -c "
    dbus-monitor \"interface='org.freedesktop.portal.FileChooser'\" \"interface='org.freedesktop.portal.Request'\" >'$OUT/real.monitor' 2>&1 &
    /usr/libexec/xdg-desktop-portal-gtk >'$OUT/real.gtk.log' 2>&1 &
    /usr/libexec/xdg-desktop-portal -r >'$OUT/real.portal.log' 2>&1 &
    sleep 2
    ( for i in \$(seq 60); do
        w=\$(xdotool search --name 'Open \\(portal smoke test\\)' 2>/dev/null | head -n1)
        [ -n \"\$w\" ] && break; sleep 1; done
      [ -n \"\$w\" ] || exit 0
      echo \"dialog window \$w\" >'$OUT/real.xdotool'
      sleep 2; xdotool windowactivate --sync \$w 2>/dev/null; xdotool windowfocus --sync \$w
      xdotool key --window \$w ctrl+l; sleep 1
      xdotool type --delay 20 '$target'; sleep 1; xdotool key Return ) &
    '$WINE' '$OUT/portal.exe' legacy" >"$OUT/real.txt" 2>"$OUT/real.err"
  tr -d '\r' <"$OUT/real.txt" >"$OUT/real.out"
  reached=$(grep -c 'member=OpenFile' "$OUT/real.monitor")
  responded=$(grep -c 'member=Response' "$OUT/real.monitor")
  echo "  real portal: OpenFile calls seen $reached, Response signals $responded, app: $(tail -n1 "$OUT/real.out")"
  if [ "$reached" -gt 0 ] && grep -qF "RESULT ok $(winpath "$target")" "$OUT/real.out"; then pass "real portal (gtk) returns the typed file"
  elif [ "$reached" -gt 0 ] && [ "$responded" -gt 0 ]; then fail "real portal (gtk)" "reached and answered, app: $(tail -n1 "$OUT/real.out")"
  elif [ "$reached" -gt 0 ]; then fail "real portal (gtk)" "reached the portal, no Response (see real.*.log)"
  else fail "real portal (gtk)" "OpenFile never reached the portal (see real.portal.log, real.err)"; fi
else
  skip "real portal" "xdg-desktop-portal, xdg-desktop-portal-gtk or xdotool missing"
fi

# --- 3. portal off (default): no D-Bus call, Wine's own dialog
if [ -n "$PY" ] && command -v xdotool >/dev/null; then
  rm -f "$OUT/off.json"
  timeout 120 env -u WINE_FORCE_PORTAL dbus-run-session -- sh -c "
    '$PY' '$HERE/mock_portal.py' '$OUT/off.json' 'file://$OUT/files/host/frame_0001.png' >'$OUT/off.portal.log' 2>&1 &
    ( for i in \$(seq 60); do
        w=\$(xdotool search --onlyvisible --name 'Open \\(portal smoke test\\)' 2>/dev/null | head -n1)
        if [ -n \"\$w\" ]; then
          echo \"wine dialog \$w\" >'$OUT/off.xdotool'
          xdotool windowactivate --sync \$w 2>/dev/null; xdotool windowfocus --sync \$w 2>/dev/null; sleep 1
          xdotool key Escape
          # on a live desktop the WM may refuse the activation (and Wine ignores synthetic keys):
          # ask the WM to close the dialog, which Wine turns into a normal close = cancel
          python3 -c 'import sys; from Xlib import display, X, protocol
d = display.Display(); r = d.screen().root; a = d.intern_atom("_NET_CLOSE_WINDOW")
w = d.create_resource_object("window", int(sys.argv[1]))
r.send_event(protocol.event.ClientMessage(window=w, client_type=a, data=(32, [0, 2, 0, 0, 0])), event_mask=X.SubstructureRedirectMask | X.SubstructureNotifyMask)
d.flush()' \$w 2>/dev/null
        fi
        sleep 2; done ) &
    '$WINE' '$OUT/portal.exe' legacy" >"$OUT/off.txt" 2>"$OUT/off.err"
  tr -d '\r' <"$OUT/off.txt" >"$OUT/off.out"
  if [ -s "$OUT/off.json" ]; then fail "portal off by default" "the portal was called"
  elif [ -s "$OUT/off.xdotool" ] && grep -q 'RESULT cancel' "$OUT/off.out"; then pass "portal off by default (Wine dialog shown and cancelled)"
  else fail "portal off by default" "no Wine dialog seen ($(tail -n1 "$OUT/off.out"))"; fi
else
  skip "portal off" "no python with dbus + gi, or no xdotool"
fi
"$WINESERVER" -k 2>/dev/null
exit $fails
