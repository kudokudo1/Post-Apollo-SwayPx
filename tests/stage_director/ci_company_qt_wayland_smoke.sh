#!/usr/bin/env bash
# Real QtWayland XDG window / kernel-pidfd baseline in a disposable SwayPX.
# NOT a MapLibre binary or a physically selected Company launch.
# Never call on a user's Fedora runtime or turn native actor evidence into
# rendering/clip/seat/input authorization.
set -euo pipefail

runtime="${1:?CI private XDG_RUNTIME_DIR required}"
qt="${STAGE_DIRECTOR_TEST_QT_COMPANY_WINDOW:-}"
swaymsg="${STAGE_DIRECTOR_TEST_SWAYMSG:-}"
one=""
two=""

cleanup() {
  local p
  for p in "$one" "$two"; do
    if [[ -n "$p" ]] && kill -0 "$p" 2>/dev/null; then
      kill -TERM "$p" 2>/dev/null || true
    fi
  done
  for p in "$one" "$two"; do
    if [[ -n "$p" ]]; then wait "$p" 2>/dev/null || true; fi
  done
}
trap cleanup EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ||
      ! -d "$runtime" || "${XDG_RUNTIME_DIR:-}" != "$runtime" ||
      "${WAYLAND_DISPLAY:-}" != wayland-* ||
      ! -x "$qt" || ! -x "$swaymsg" ]]; then
  echo "BLOCKED_NOT_DISPOSABLE_COMPANY_QT_QPA" >&2
  exit 2
fi

ipc=""
for _ in {1..80}; do
  for candidate in "$runtime"/sway-ipc.*.sock; do
    if [[ -S "$candidate" ]]; then ipc="$candidate"; break; fi
  done
  if [[ -n "$ipc" ]]; then break; fi
  sleep 0.1
done
if [[ -z "$ipc" ]]; then
  echo "BLOCKED_NO_DISPOSABLE_SWAY_IPC" >&2
  exit 2
fi
export QT_QPA_PLATFORM=wayland
export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
unset DISPLAY

wait_qt_ready() {
  local pid="$1" case_name="$2" log="$3"
  for _ in {1..100}; do
    if grep -Fxq "SD_CI_COMPANY_QT_QPA_READY case=$case_name pid=$pid wayland_display=1" "$log"; then
      return 0
    fi
    if ! kill -0 "$pid" 2>/dev/null; then break; fi
    sleep 0.1
  done
  echo "FAIL: Qt client '$case_name' did not enter real Wayland QPA" >&2
  cat "$log" >&2 || true
  return 1
}

# Read Sway's REAL XDG tree for observational verification only.
# Title/app-id can locate fixtures but NEVER authorize Company pairing.
check_tree() {
  local first="$1" second="$2"
  for _ in {1..100}; do
    if "$swaymsg" -s "$ipc" -r -t get_tree > "$runtime/company-qt-tree.json" 2>/dev/null &&
        python3 - "$runtime/company-qt-tree.json" "$first" "$second" <<'PY' >/dev/null 2>&1
import json,sys
tree=json.load(open(sys.argv[1],encoding="utf-8"))
expected={"one":int(sys.argv[2]), "two":int(sys.argv[3])}
found={}
def descend(node):
    name=node.get("name","") or ""
    for case in ("one","two"):
        if name == "SD-CI-CompanyVectorProbe-"+case:
            found.setdefault(case,[]).append(node)
    for field in ("nodes","floating_nodes"):
        for child in node.get(field,[]):
            descend(child)
descend(tree)
for case,pid in expected.items():
    instances=found.get(case,[])
    if pid == 0:
        assert not instances, (case,instances)
    else:
        assert len(instances)==1,(case,len(instances))
        assert instances[0].get("pid")==pid,(case,instances[0].get("pid"),pid)
        # A real Qt process must be an XDG Wayland view, never an xwayland app.
        assert instances[0].get("window") is None, (case,"xwayland")
PY
    then return 0; fi
    sleep 0.1
  done
  echo "FAIL: exact real native Qt Wayland roots did not match the expected processes" >&2
  "$swaymsg" -s "$ipc" -r -t get_tree >&2 || true
  return 1
}

# The wrapper from pinned Nix Qt hook uses exec, preserving the launched PID.
"$qt" one > "$runtime/company-qt-one.log" 2>&1 &
one=$!
wait_qt_ready "$one" one "$runtime/company-qt-one.log"
check_tree "$one" 0
"$qt" two > "$runtime/company-qt-two.log" 2>&1 &
two=$!
wait_qt_ready "$two" two "$runtime/company-qt-two.log"
check_tree "$one" "$two"

python3 - "$one" "$two" <<'PY'
import os,pathlib,sys
one,two=map(int,sys.argv[1:])
assert one != two and os.getuid() != 0
for pid in (one,two):
    stat=pathlib.Path(f"/proc/{pid}/stat").read_text()
    fields=stat.rsplit(")",1)[1].split()
    assert len(fields)>19 and int(fields[19])>0
    assert pathlib.Path(f"/proc/{pid}/exe").exists()
    fd=os.pidfd_open(pid,0)
    try:
        info=pathlib.Path(f"/proc/self/fdinfo/{fd}").read_text()
        lines=[s.strip().split(":",1)[1].strip()
               for s in info.splitlines() if s.startswith("Pid:")]
        assert lines == [str(pid)],(pid,lines)
    finally:
        os.close(fd)
print("SD_COMPANY_M30D_QT_PEERS real_qpa=2 live_kernel_pidfd=2 native_xdg=2 authority=NONE PASS")
PY

# An app-id/title lookalike and two same-looking Qt siblings have NOT
# received the physical M24 host selection or private M28b fd receipt.
if grep -Fq '[stage-director][COMPANY_M30C][LIVE] pidfd_m21_candidate=yes' "$runtime/sway.log" ||
    grep -Fq '[stage-director][COMPANY_M24][HOST] native Company layer physically selected' "$runtime/sway.log"; then
  echo "FAIL: Qt app-id lookalike must not acquire selected Company custody" >&2
  exit 1
fi

kill -TERM "$one"
wait "$one" 2>/dev/null || true
one=""
check_tree 0 "$two"
kill -TERM "$two"
wait "$two" 2>/dev/null || true
two=""
check_tree 0 0
echo "SD_COMPANY_M30D_QT_CLEANUP two_live_wayland_roots_removed=1 no_selected_host=1 authority=NONE PASS"
