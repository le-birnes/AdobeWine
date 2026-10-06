#!/usr/bin/env bash
# AdobeWine problem/usage report.
#   adobewine report [--app NAME] [--log FILE] [--note TEXT] [--auto] [--save-only]
# Without --note it asks you what you were doing.
#
# Collects the hardware, drivers, distribution, AdobeWine and Adobe app versions, what was
# run, and the end of that run's log. Personal data is removed (user and host names, home
# paths, e-mail and IP addresses, anything that looks like a token), and you see the
# report before anything is sent. Nothing is ever sent without either your "yes" or
# REPORTS=auto in ~/.config/adobewine/config.
#
# Sending: with the GitHub CLI logged in (`gh auth login`), the report becomes an issue
# labelled "report" in the AdobeWine repository. Without it, your browser opens a
# pre-filled issue page for you to submit. Reports are always saved in
# ~/.local/share/adobewine/reports/ as well.
set -uo pipefail
source "$(cd "$(dirname "$0")/.." && pwd)/bin/env.sh"
CONF="${XDG_CONFIG_HOME:-$HOME/.config}/adobewine/config"
[ -f "$CONF" ] && source "$CONF"
REPO="${ADOBEWINE_REPO:-le-birnes/AdobeWine}"
REPORTS="${REPORTS:-ask}"

APP="" LOG="" NOTE="" AUTO=0 SAVE_ONLY=0 EXIT_CODE="" DURATION=""
while [ $# -gt 0 ]; do
  case "$1" in
    --app) APP="$2"; shift 2 ;;
    --log) LOG="$2"; shift 2 ;;
    --note) NOTE="$2"; shift 2 ;;
    --exit-code) EXIT_CODE="$2"; shift 2 ;;
    --duration) DURATION="$2"; shift 2 ;;
    --auto) AUTO=1; shift ;;
    --save-only) SAVE_ONLY=1; shift ;;
    *) echo "unknown option $1" >&2; exit 1 ;;
  esac
done
[ -n "$LOG" ] || LOG="$(ls -t "$ADOBEWINE_HOME"/logs/*.log 2>/dev/null | head -1)"
if [ -z "$NOTE" ] && [ "$AUTO" = 0 ] && [ "$SAVE_ONLY" = 0 ] && [ -t 0 ]; then
  echo "In a sentence or two: what were you doing, and what went wrong? (Enter to skip)"
  read -r -p "> " NOTE
fi
[ -n "$APP" ] || { [ -n "$LOG" ] && APP="$(basename "$LOG" | sed 's/-[0-9]\{8\}-[0-9]\{6\}\.log$//')"; }

# ---- collect ----
os="$(. /etc/os-release 2>/dev/null; echo "${PRETTY_NAME:-unknown}")"
cpu="$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2- | sed 's/^ *//')"
ram="$(awk '/MemTotal/{printf "%.0f GB", $2/1048576}' /proc/meminfo)"
gpus="$(lspci 2>/dev/null | grep -Ei 'vga compatible|3d controller|display controller' | cut -d' ' -f2- || echo 'lspci not available')"
nv="$(cat /sys/module/nvidia/version 2>/dev/null || echo none)"
vk="$(command -v vulkaninfo >/dev/null && vulkaninfo --summary 2>/dev/null | grep -E 'deviceName|driverInfo|apiVersion' | sed 's/^[[:space:]]*//' || echo 'vulkaninfo not installed')"
mesa="$(command -v pacman >/dev/null && pacman -Q mesa 2>/dev/null; command -v dpkg-query >/dev/null && dpkg-query -W libgl1-mesa-dri 2>/dev/null; command -v rpm >/dev/null && rpm -q mesa-dri-drivers 2>/dev/null)"
desktop="${XDG_CURRENT_DESKTOP:-unknown} (${XDG_SESSION_TYPE:-unknown} session)"
wine_ver="$(wine --version 2>/dev/null || echo 'no runtime')"
aw_ver="$(cat "$ADOBEWINE_SRC/VERSION" 2>/dev/null || git -C "$ADOBEWINE_SRC" describe --tags --always 2>/dev/null || echo unknown)"
deps="$(ls -d "$ADOBEWINE_HOME"/deps/*/ 2>/dev/null | xargs -r -n1 basename | tr '\n' ' ')"
apps="$(ls -d "$WINEPREFIX/drive_c/Program Files/Adobe/"*/ 2>/dev/null | xargs -r -d '\n' -n1 basename | tr '\n' ',' | sed 's/,$//; s/,/, /g')"
ntsync="$([ -e /dev/ntsync ] && echo yes || echo no)"
crash="$([ -n "$LOG" ] && grep -m3 -E 'Unhandled exception|Unhandled page fault|page fault on|Assertion .* failed' "$LOG" | cut -c1-200)"
tail_log="$([ -n "$LOG" ] && grep -vE '^(info|warn): |:(info|fixme):vkd3d-proton:|libEGL warning|MESA-|pci id for fd|^[[:space:]]*$' "$LOG" | tail -n 120)"
# memory: totals, the biggest processes (names only, no command lines) and any Wine processes left
mem="$(free -h 2>/dev/null | sed -n '1,3p')"
# Wine renames each process after its current thread ("CrBrowserMain", ...), so take the
# program name from the command line (C:\...\Illustrator.exe -> Illustrator.exe) instead.
procs="$(for d in /proc/[0-9]*; do
  r="$(sed -n 's/^VmRSS:[[:space:]]*\([0-9]*\).*/\1/p' "$d/status" 2>/dev/null)"; [ -n "$r" ] || continue
  # only Windows programs (first argument ends in .exe) use it; anything else keeps its short
  # comm name, so no other program's arguments can end up in a report
  n="$(tr '\0' '\n' < "$d/cmdline" 2>/dev/null | head -1)"
  case "$n" in *.exe|*.EXE) n="${n##*[\\/]}" ;; *) n="$(cat "$d/comm" 2>/dev/null)" ;; esac
  echo "$r $n"
done)"
top_rss="$(echo "$procs" | sort -rn | head -12 | awk '{printf "%7.0f MB  %s\n", $1/1024, substr($0, index($0,$2))}')"
wine_left="$(echo "$procs" | grep -ciE ' [^ ]*\.exe$| wineserver$' || true)"
shm="$(du -sh /dev/shm 2>/dev/null | cut -f1)"

TS="$(date +%Y%m%d-%H%M%S)"
mkdir -p "$ADOBEWINE_HOME/reports"
OUT="$ADOBEWINE_HOME/reports/report-$TS.md"
{
  echo "### AdobeWine report"
  echo
  echo "| | |"
  echo "|---|---|"
  echo "| App / command | ${APP:-unknown} |"
  [ -n "$EXIT_CODE" ] && echo "| Exit code | $EXIT_CODE |"
  [ -n "$DURATION" ] && echo "| Ran for | ${DURATION}s |"
  echo "| Crash detected | $([ -n "$crash" ] && echo yes || echo no) |"
  echo "| AdobeWine | $aw_ver ($wine_ver) |"
  echo "| Translation layers | ${deps:-not downloaded} |"
  echo "| Adobe apps in prefix | ${apps:-none} |"
  echo "| Distribution | $os |"
  echo "| Kernel | $(uname -r), ntsync: $ntsync |"
  echo "| Desktop | $desktop |"
  echo "| CPU / RAM | $cpu / $ram |"
  echo "| NVIDIA driver | $nv |"
  echo "| Mesa | ${mesa:-unknown} |"
  echo
  echo "**GPUs**"
  echo '```'
  echo "$gpus"
  echo "$vk"
  echo '```'
  echo "**Memory** (when the report was made; Wine processes still running: ${wine_left:-0}, /dev/shm: ${shm:-?})"
  echo '```'
  echo "$mem"
  echo
  echo "$top_rss"
  echo '```'
  if [ -n "$NOTE" ]; then echo "**What I was doing**"; echo; echo "$NOTE"; echo; fi
  if [ -n "$crash" ]; then echo "**Crash lines**"; echo '```'; echo "$crash"; echo '```'; fi
  echo "<details><summary>Log tail (${LOG:+$(basename "$LOG")})</summary>"
  echo
  echo '```'
  echo "${tail_log:-no log}"
  echo '```'
  echo "</details>"
} > "$OUT.raw"

# ---- remove personal data ----
host="$(uname -n)"
sed -E \
  -e "s#${HOME//#/\\#}#~#g" \
  -e "s#\b${USER:-nouser}\b#<user>#g" \
  -e "s#\b${host}\b#<host>#g" \
  -e 's#[Cc]:\\\\users\\\\[^\\\\ ]+#C:\\users\\<user>#g' \
  -e 's#[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}#<email>#g' \
  -e 's#\b([0-9]{1,3}\.){3}[0-9]{1,3}\b#<ip>#g' \
  -e 's#eyJ[A-Za-z0-9_-]{10,}(\.[A-Za-z0-9_-]+)*#<token>#g' \
  -e 's#(Authorization|Bearer|[Cc]ookie|Set-Cookie)[:=]? .*#\1 <removed>#g' \
  -e 's#([Tt]oken|access_token|refresh_token|code|password)=[^ &]*#\1=<removed>#g' \
  -e 's#~/[^ "]*#~/<path>#g' \
  -e 's#[Zz]:\\[^ "]*#Z:\\<path>#g' \
  -e 's#[0-9A-F]{24}@AdobeID#<adobe-id>#g' \
  "$OUT.raw" > "$OUT"
rm -f "$OUT.raw"
echo "report saved: $OUT"
[ "$SAVE_ONLY" = 1 ] && exit 0

title="[report] ${APP:-adobewine}: $([ -n "$crash" ] && echo crash || echo feedback) on ${os}"
send_gh() { gh issue create -R "$REPO" --title "$title" --body-file "$OUT" --label report; }
gh_ok() { command -v gh >/dev/null && gh auth status -h github.com >/dev/null 2>&1; }

if [ "$AUTO" = 1 ]; then
  [ "$REPORTS" = auto ] && gh_ok && send_gh && exit 0
  exit 0   # not allowed or not possible automatically: the report stays saved locally
fi

[ "$REPORTS" = off ] && { echo "reports are off (REPORTS=off in $CONF); nothing sent"; exit 0; }
${PAGER:-less} "$OUT"
read -r -p "Send this report to github.com/$REPO? [y/N] " ans
case "$ans" in [yY]*) ;; *) echo "not sent"; exit 0 ;; esac
if gh_ok; then
  send_gh
else
  body="$(head -c 6000 "$OUT")"
  url="https://github.com/$REPO/issues/new?labels=report&title=$(python3 -c 'import sys,urllib.parse;print(urllib.parse.quote(sys.argv[1]))' "$title")&body=$(python3 -c 'import sys,urllib.parse;print(urllib.parse.quote(sys.argv[1]))' "$body")"
  echo "Opening your browser; review and press 'Submit new issue'. (The full report is in $OUT.)"
  xdg-open "$url" >/dev/null 2>&1 || echo "open this address: $url"
fi
