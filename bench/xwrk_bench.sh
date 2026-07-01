#!/usr/bin/env bash
# Accurate server benchmark using the PURE-XLANG xwrk client (compiled C, no GIL —
# unlike bench_py.py which is client-bottlenecked and undercounts ~2x).
# Measures server_http direct, through the proxy (1 backend), and LB (2 backends).
#
# Usage: xwrk_bench.sh [path/to/xlangc]
set -u
XLANGC="${1:-xlangc}"
cd "$(dirname "$0")/.."

SRV=/tmp/xwrkbench_srv
PROXY=/tmp/xwrkbench_proxy
XWRK=/tmp/xlang_xwrk
ROOT="$(mktemp -d)"
printf 'hello\n' > "$ROOT/index.html"
trap 'pkill -f "$SRV" 2>/dev/null; pkill -f "$PROXY" 2>/dev/null; rm -rf "$ROOT"' EXIT
pkill -f "$SRV" 2>/dev/null
pkill -f "$PROXY" 2>/dev/null
sleep 0.2

"$XLANGC" c servers/server_http.x  -o build/server_http.c  >/dev/null 2>&1; cc -O2 -o "$SRV" build/server_http.c
"$XLANGC" c servers/server_proxy.x -o build/server_proxy.c >/dev/null 2>&1; cc -O2 -o "$PROXY" build/server_proxy.c
# xwrk lives in the sibling xlang-linux repo; build it if the binary is missing.
if [ ! -x "$XWRK" ]; then
    XL=../xlang-linux
    "$XLANGC" c "$XL/coreutils/xwrk.x" -o build/xwrk.c >/dev/null 2>&1; cc -O2 -o "$XWRK" build/xwrk.c
fi

run() { "$XWRK" 127.0.0.1 "$1" /index.html 3 16 | sed -n 's/.*= \([0-9]*\) req\/s.*/\1/p'; }

echo "req/s @ c=16, 3s (xwrk pure-xlang client — accurate, no GIL bottleneck)"
echo "-----------------------------------------------------------------------"

"$SRV" "$ROOT" 30001 >/dev/null 2>&1 &
sleep 0.5
printf '  server_http (direct):  %s\n' "$(run 30001)"

"$PROXY" 30002 16 127.0.0.1:30001 >/dev/null 2>&1 &
sleep 0.5
printf '  proxy -> 1 backend:    %s\n' "$(run 30002)"
pkill -f "$PROXY" 2>/dev/null; sleep 0.3

"$SRV" "$ROOT" 30003 >/dev/null 2>&1 &
"$PROXY" 30004 16 127.0.0.1:30001 127.0.0.1:30003 >/dev/null 2>&1 &
sleep 0.5
printf '  proxy -> 2 backends:   %s\n' "$(run 30004)"
echo "-----------------------------------------------------------------------"
