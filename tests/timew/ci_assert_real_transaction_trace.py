#!/usr/bin/env python3
"""Check actual, native SwayPX callback order in a disposable XDG headless test.

The logs are diagnostic and local to the CI compositor, not authenticated
cross-process transaction IDs or a protected receipt. Pointer addresses may
be reused. Each COMMIT begins a *new ephemeral diagnostic interval*.
"""
from pathlib import Path
import re
import sys
import time

if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
    raise SystemExit("BLOCKED_TIMEW_REAL_TXN_ARGS")
logpath = Path(sys.argv[1])
mode = sys.argv[2]
pattern = re.compile(r"TIMEW_CI_TXN_(COMMIT|READY_SERIAL|READY_GEOMETRY|"
                     r"TIMEOUT|APPLY|CURRENT_COPIED|SCENE_ARRANGED) "
                     r"txn=(0x[0-9a-fA-F]+|\(nil\))")
waiting = re.compile(r"waiting_before=(\d+)")
configures = re.compile(r"configures=(\d+)")
forced = re.compile(r"debug_force_wait=(\d+)")

def trace(log: str):
    runs = []
    latest = {}
    for line in log.splitlines():
        m = pattern.search(line)
        if not m:
            continue
        event, pointer = m.groups()
        if event == "COMMIT":
            current = {"pointer": pointer, "events": [event],
                       "configures": int(configures.search(line).group(1))
                           if configures.search(line) else -1,
                       "forced": int(forced.search(line).group(1))
                           if forced.search(line) else -1,
                       "timeout_waiting": None}
            runs.append(current)
            latest[pointer] = current
        elif pointer in latest:
            current = latest[pointer]
            current["events"].append(event)
            if event == "TIMEOUT":
                current["timeout_waiting"] = (
                    int(waiting.search(line).group(1))
                    if waiting.search(line) else None
                )
    return runs

def valid_normal(t):
    e = t["events"]
    ready = [i for i,x in enumerate(e) if x in
             ("READY_SERIAL", "READY_GEOMETRY")]
    return (t["forced"] == 0 and t["configures"] >= 1
            and ready and "TIMEOUT" not in e
            and "APPLY" in e and "CURRENT_COPIED" in e
            and "SCENE_ARRANGED" in e
            and min(ready) < e.index("APPLY")
            < e.index("CURRENT_COPIED") < e.index("SCENE_ARRANGED"))

def valid_timeout(t):
    e = t["events"]
    return (t["forced"] == 1 and t["configures"] >= 1
            and t["timeout_waiting"] is not None
            and t["timeout_waiting"] >= 1000000
            and "TIMEOUT" in e and "APPLY" in e
            and "CURRENT_COPIED" in e and "SCENE_ARRANGED" in e
            and e.index("TIMEOUT") < e.index("APPLY")
            < e.index("CURRENT_COPIED") < e.index("SCENE_ARRANGED"))

predicate = valid_normal if mode == "normal" else valid_timeout
end = time.monotonic() + 15
observed = []
while time.monotonic() < end:
    observed = trace(logpath.read_text(errors="replace"))
    if any(predicate(item) for item in observed):
        break
    time.sleep(0.15)
else:
    summary = [{"events": t["events"], "configures": t["configures"],
                "forced": t["forced"], "timeout_waiting": t["timeout_waiting"]}
               for t in observed[-12:]]
    print("TIMEW_CI_TXN_REAL_FAIL: no complete actual XDG "
          f"{mode} callback sequence; observed={summary}", file=sys.stderr)
    raise SystemExit(1)

matches = [t for t in observed if predicate(t)]
# Never infer native completion from current/arrange or an output frame.
print(f"TIMEW_CI_TXN_REAL_{mode.upper()}_PASS "
      f"matching_intervals={len(matches)} "
      f"configure_ready={'1' if mode == 'normal' else 'UNTRUSTED'} "
      f"timeout_path={'1' if mode == 'forced_timeout' else '0'} "
      "current_copied=1 scene_arranged=1 "
      "presented=UNVERIFIED settled=0 "
      "operator_selection=NONE source=UNENROLLED c5_spent=NONE "
      "native_restore=OFF native_move=OFF retry=OFF")
