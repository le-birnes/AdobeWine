#!/usr/bin/env bash
# usage: tools/sandbox.sh CMD [ARGS...]   e.g. tools/sandbox.sh bin/adobewine cc  — run a Wine command in its own memory-capped systemd scope.
# A runaway (process storm, X client exhaustion, memory/swap blowup) then gets killed on its own instead of
# systemd-oomd killing the whole terminal tab and everything in it (Creative Cloud's 6.11 self-update, 2026-10-20).
# Limits: SANDBOX_MEM (default 16G), SANDBOX_SWAP (2G), SANDBOX_PROCS (80 processes in the scope).
# On a trip the scope's process list is saved to $SANDBOX_OUT/runaway-ps.txt (default: logs dir) before the kill.
R=$(cd "$(dirname "$0")/.." && pwd)
source $R/bin/env.sh
U=adobe-sandbox-$$
OUT=${SANDBOX_OUT:-$ADOBEWINE_HOME/logs}
systemd-run --user --scope --quiet --unit=$U -p MemoryMax=${SANDBOX_MEM:-16G} -p MemorySwapMax=${SANDBOX_SWAP:-2G} \
  -p ManagedOOMMemoryPressure=kill -- "$@" &
for i in $(seq 20); do systemctl --user is-active -q $U.scope && break; sleep 0.5; done
CG=/sys/fs/cgroup$(systemctl --user show -p ControlGroup --value $U.scope)
while systemctl --user is-active -q $U.scope; do
  n=$(wc -l < $CG/cgroup.procs 2>/dev/null || echo 0)
  if [ "$n" -gt ${SANDBOX_PROCS:-80} ]; then
    ps -o pid,ppid,etimes,rss,args -p "$(paste -sd, $CG/cgroup.procs)" > $OUT/runaway-ps.txt 2>&1
    echo "SANDBOX: $n processes in $U, killing it (process list: $OUT/runaway-ps.txt)" >&2
    wineserver -k 2>/dev/null; sleep 2
    systemctl --user kill --signal=KILL $U.scope 2>/dev/null
    break
  fi
  sleep 2
done
wait
