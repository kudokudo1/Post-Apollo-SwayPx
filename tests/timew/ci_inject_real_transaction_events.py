#!/usr/bin/env python3
"""Inject actual SwayPX transaction-event diagnostics in a disposable CI checkout.

Never run on the operator's machine. The checked-in native transaction,
input handlers, Wayland model, actor smoke, and production Meson stay intact.
This temporarily modifies *only* the GitHub runner's checkout.
"""
from pathlib import Path
import os

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_NON_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NON_EPHEMERAL_RUNNER")
root = Path(__file__).resolve().parents[2]
transaction = root / "sway/desktop/transaction.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
actor = root / "tests/stage_director/ci_native_news_actor_smoke.sh"
meson = root / "sway/meson.build"

def once(body: str, old: str, new: str, label: str) -> str:
    count = body.count(old)
    if count != 1:
        raise SystemExit(f"BLOCKED_SOURCE_DRIFT_{label} count={count}")
    return body.replace(old, new, 1)

original = {p: p.read_text() for p in (transaction, smoke, actor, meson)}
t = original[transaction]
s = original[smoke]
a = original[actor]
m = original[meson]
if "TIMEW_CI_TXN_" in t or "STAGE_DIRECTOR_TIMEW_TXN_MODE" in s:
    raise SystemExit("BLOCKED_ALREADY_INSTRUMENTED")
if "timew_scope_a_native_txn_observer.c" in m:
    raise SystemExit("BLOCKED_PRODUCTION_TRANSACTION_LINKAGE")

# This is a diagnostics log, not a protected transaction ID/receipt.
# The native pointer is reused and is intentionally NEVER used for trust.
t = once(t,
    "\ttransaction_apply(server.queued_transaction);\n\tarrange_root(root);",
    "\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_APPLY txn=%p waiting=%zi source=real_compositor authority=NONE\",\n"
    "\t\t\t(void *)server.queued_transaction, server.queued_transaction->num_waiting);\n"
    "\ttransaction_apply(server.queued_transaction);\n"
    "\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_CURRENT_COPIED txn=%p source=real_compositor settled=0 authority=NONE\",\n"
    "\t\t\t(void *)server.queued_transaction);\n"
    "\tarrange_root(root);\n"
    "\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_SCENE_ARRANGED txn=%p presented=0 authority=NONE\",\n"
    "\t\t\t(void *)server.queued_transaction);",
    "APPLY_ARRANGE",
)
t = once(t,
    "\tsway_log(SWAY_DEBUG, \"Transaction %p timed out (%zi waiting)\",\n"
    "\t\t\ttransaction, transaction->num_waiting);",
    "\tsway_log(SWAY_DEBUG, \"Transaction %p timed out (%zi waiting)\",\n"
    "\t\t\ttransaction, transaction->num_waiting);\n"
    "\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_TIMEOUT txn=%p waiting_before=%zi source=real_timer ack=UNKNOWN_HOLD\",\n"
    "\t\t\t(void *)transaction, transaction->num_waiting);",
    "TIMEOUT",
)
t = once(t,
    "\tif (transaction->num_waiting) {\n\t\t// Set up a timer which the views must respond within",
    "\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_COMMIT txn=%p waiting=%zi configures=%zi debug_force_wait=%d source=real_compositor authority=NONE\",\n"
    "\t\t\t(void *)transaction, transaction->num_waiting, transaction->num_configures, debug.txn_wait ? 1 : 0);\n"
    "\tif (transaction->num_waiting) {\n\t\t// Set up a timer which the views must respond within",
    "COMMIT",
)
t = once(t,
    "\tif (instruction != NULL && instruction->serial == serial) {\n"
    "\t\tset_instruction_ready(instruction);",
    "\tif (instruction != NULL && instruction->serial == serial) {\n"
    "\t\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_READY_SERIAL txn=%p serial=%u source=real_view_callback protected_ack=0\",\n"
    "\t\t\t\t(void *)instruction->transaction, serial);\n"
    "\t\tset_instruction_ready(instruction);",
    "READY_SERIAL",
)
t = once(t,
    "\t\tset_instruction_ready(instruction);\n"
    "\t\treturn true;\n"
    "\t}\n\treturn false;\n}\n\nstatic void _transaction_commit_dirty",
    "\t\tsway_log(SWAY_ERROR, \"TIMEW_CI_TXN_READY_GEOMETRY txn=%p source=real_view_callback protected_ack=0\",\n"
    "\t\t\t\t(void *)instruction->transaction);\n"
    "\t\tset_instruction_ready(instruction);\n"
    "\t\treturn true;\n"
    "\t}\n\treturn false;\n}\n\nstatic void _transaction_commit_dirty",
    "READY_GEOMETRY",
)

# Real headless compositor. "txn-wait" is Sway's *existing* native debug
# flag, forcing the event-loop timer path even if the XDG client is ready.
s = once(s,
    '"$sway_binary" -d -c "$config" > "$log" 2>&1 &',
    '''if [[ "${STAGE_DIRECTOR_TIMEW_TXN_MODE:-}" == forced_timeout ]]; then
  sway_argv=("$sway_binary" -d -D txn-wait -D txn-timeout=180 -c "$config")
elif [[ "${STAGE_DIRECTOR_TIMEW_TXN_MODE:-}" == normal ]]; then
  sway_argv=("$sway_binary" -d -c "$config")
else
  echo "BLOCKED_UNAPPROVED_TIMEW_CI_MODE" >&2
  exit 2
fi
"${sway_argv[@]}" > "$log" 2>&1 &''',
    "HEADLESS_LAUNCH",
)
s = once(s,
    '  bash tests/stage_director/ci_native_news_actor_smoke.sh \\\n'
    '    "$runtime" "${socket##*/}"\nfi',
    '  bash tests/stage_director/ci_native_news_actor_smoke.sh \\\n'
    '    "$runtime" "${socket##*/}"\n'
    '  python3 -B tests/timew/ci_assert_real_transaction_trace.py \\\n'
    '    "$log" "$STAGE_DIRECTOR_TIMEW_TXN_MODE"\nfi',
    "ACTOR_CHECKPOINT",
)
# The existing disposable foot actor is the only native geometry change
# in the test. It triggers a real configure; it cannot grant TimeW authority.
a = once(a,
    'echo "Stage Director real mapped actor smoke: PASS"',
    '''echo "Stage Director real mapped actor smoke: PASS"
if [[ -n "${STAGE_DIRECTOR_TIMEW_TXN_MODE:-}" ]]; then
  "$swaymsg_binary" -s "$ipc_socket" \\
    '[app_id="post-apollo-weather-news"] resize set width 1290px height 800px' \\
    >"$runtime/timew-ci-resize-only.log" 2>&1 || {
      echo "TIMEW_CI_TXN_FAIL resize fixture command rejected" >&2
      cat "$runtime/timew-ci-resize-only.log" >&2
      exit 1
    }
  echo "TIMEW_CI_TXN_REAL_XDG_TRIGGER mode=$STAGE_DIRECTOR_TIMEW_TXN_MODE real_foot=1 protected_resize=0"
fi''',
    "NATIVE_FIXTURE_TRIGGER",
)
# Check all anchors and forbidden production modifications before writing.
assert m == original[meson]
for p, modified in ((transaction,t),(smoke,s),(actor,a)):
    if modified == original[p]:
        raise SystemExit("BLOCKED_NOOP_INJECTION_" + p.name)
for p, modified in ((transaction,t),(smoke,s),(actor,a)):
    p.write_text(modified)
print("TIMEW_CI_TXN_DISPOSABLE_INJECTION_PASS native_callbacks=actual "
      "modes=normal,forced_timeout source_authority=NONE production_wiring=OFF")
