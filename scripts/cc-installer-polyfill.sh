# shellcheck shell=bash
# Temporary workaround for the blank white window of the Creative Cloud web installer
# 2.14.0.82 (issue #2), until Wine patches 0055-0057 ship in a release.
#
# The installer unpacks its UI (index.html + CCDInstaller.js) into %TEMP%\{GUID}\ and shows
# it in mshtml, where the page dies on DOM/JS features Wine 11.18 lacks. While the installer
# runs, an inotify watcher prepends scripts/cc-installer-polyfill.js to CCDInstaller.js right
# after it is written, before mshtml loads it.
#
# Based on NickPittas' run-installer.sh in https://github.com/le-birnes/AdobeWine/issues/2
#
# Sourced by bin/adobewine (needs WINEPREFIX and ADOBEWINE_SRC):
#   cc_polyfill_start; wine "Creative_Cloud_Set-Up.exe"; cc_polyfill_stop

CC_POLYFILL_MARKER='AdobeWine CC installer polyfill'
CC_POLYFILL_PID=

# cc_polyfill_apply <polyfill.js> <CCDInstaller.js>: prepend once (keeps a UTF-8 BOM first)
cc_polyfill_apply() {
  local js="$1" f="$2" tmp
  [ -s "$f" ] || return 0
  head -c 512 "$f" | grep -qF "$CC_POLYFILL_MARKER" && return 0
  tmp="$f.adobewine.$$"
  if [ "$(head -c 3 "$f" | od -An -tx1 | tr -d ' \n')" = efbbbf ]; then
    { printf '\357\273\277'; cat "$js"; tail -c +4 "$f"; } >"$tmp"
  else
    { cat "$js"; cat "$f"; } >"$tmp"
  fi && cat "$tmp" >"$f"   # rewrite in place: same file, same inode
  rm -f "$tmp"
}

cc_polyfill_start() {
  local js="$ADOBEWINE_SRC/scripts/cc-installer-polyfill.js" temp ready
  if ! command -v inotifywait >/dev/null 2>&1; then
    echo "Note: install inotify-tools so AdobeWine can fix the Creative Cloud installer's blank window; starting the installer anyway."
    return 0
  fi
  temp="$WINEPREFIX/drive_c/users/${USER:-$(id -un)}/AppData/Local/Temp"
  mkdir -p "$temp"
  ready="$(mktemp)"
  (
    inotifywait -m -r -e close_write -e moved_to --format '%w%f' "$temp" 2>"$ready" |
      while IFS= read -r f; do
        case "$f" in */CCDInstaller.js) cc_polyfill_apply "$js" "$f" ;; esac
      done
  ) &
  CC_POLYFILL_PID=$!
  trap cc_polyfill_stop EXIT
  # the installer must not start before the watches are in place
  for _ in $(seq 100); do
    grep -q 'Watches established' "$ready" && break
    kill -0 "$CC_POLYFILL_PID" 2>/dev/null || break
    sleep 0.1
  done
  grep -q 'Watches established' "$ready" || { echo "inotifywait failed:"; cat "$ready"; } >&2
  rm -f "$ready"
}

cc_polyfill_stop() {
  [ -n "$CC_POLYFILL_PID" ] || return 0
  pkill -P "$CC_POLYFILL_PID" 2>/dev/null
  kill "$CC_POLYFILL_PID" 2>/dev/null
  wait "$CC_POLYFILL_PID" 2>/dev/null
  CC_POLYFILL_PID=
}
