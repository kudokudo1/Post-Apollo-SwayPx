#!/usr/bin/env python3
"""Bind actual transaction_commit instructions to #178's real XDG lifetime, CI only.

Apply AFTER parent ci_inject_real_xdg_lifecycle.py and
ci_overlay_observed_lineage.py. All C and shell changes occur only in the
disposable GitHub Actions checkout. NO production module, IPC, or input hook.
"""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_NOT_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NOT_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
txn = root / "sway/desktop/transaction.c"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
actor = root / "tests/stage_director/ci_native_news_actor_smoke.sh"
meson = root / "sway/meson.build"

def one(text, original, replacement, label):
    if text.count(original) != 1:
        raise SystemExit(f"BLOCKED_SOURCE_DRIFT_{label} matches={text.count(original)}")
    return text.replace(original, replacement, 1)

t, p, a, m = (path.read_text() for path in (txn, probe, actor, meson))
if "TIMEW_CI_REAL_INSTRUCTION_" in t or "TIMEW_CI_REAL_INSTRUCTION_BIND" in p:
    raise SystemExit("BLOCKED_PREEXISTING_INSTRUCTION_HOOK")
if "TIMEW_CI_LINEAGE_OPEN" not in p or "'timew_scope_a_observed_lineage.c'" not in m:
    raise SystemExit("BLOCKED_NO_PARENT_REAL_XDG_LINEAGE_INSTRUMENTATION")
if "'timew_scope_a_ci_real_instruction.c'" in m:
    raise SystemExit("BLOCKED_UNAPPROVED_PRODUCTION_SOURCE")

t = one(
    t,
    '#include "sway/desktop/transaction.h"',
    '#include "sway/desktop/transaction.h"\n'
    '#include "sway/timew_scope_a_ci_real_instruction.h"',
    "NATIVE_TRANSACTION_INCLUDE",
)
t = one(
    t,
    '\t\tnode->instruction = instruction;\n'
    '\t}\n\ttransaction->num_configures = transaction->num_waiting;',
    '\t\tnode->instruction = instruction;\n'
    '\t\tif (!node->destroying && node_is_view(node) &&\n'
    '\t\t\t\tnode->sway_container && node->sway_container->view) {\n'
    '\t\t\ttimew_scope_a_ci_note_real_instruction(\n'
    '\t\t\t\tnode->sway_container->view,\n'
    '\t\t\t\t(uintptr_t)transaction, instruction->serial);\n'
    '\t\t}\n'
    '\t}\n\ttransaction->num_configures = transaction->num_waiting;',
    "REAL_INSTRUCTION_COMMIT_SEAM",
)
p = one(
    p,
    '#include "sway/timew_scope_a_observed_lineage.h"',
    '#include "sway/timew_scope_a_observed_lineage.h"\n'
    '#include "sway/timew_scope_a_ci_real_instruction.h"',
    "PRIVATE_CI_PROBE_INCLUDE",
)
p = one(
    p,
    'static struct timew_scope_a_lineage_tag ci_transaction;',
    'static struct timew_scope_a_lineage_tag ci_transaction;\n'
    'static uint64_t ci_real_commit_floor;\n'
    'static uint64_t ci_real_matched_this_lifetime;',
    "REAL_INSTRUCTION_STATE",
)

# The native transaction code calls this only after its real instruction
# loop has created the view's node->instruction. We intentionally do NOT
# modify the #178 issuer or treat its at-map transaction number as a commit ID.
# The new serial is generated at the actual commit hook, but is still a
# diagnostic and not a protected command/transaction identity.
function_source = r"""
void timew_scope_a_ci_note_real_instruction(struct sway_view *view,
		uintptr_t transaction_ref, uint32_t configure_serial) {
	if (!ci_initialized || !ci_view || view != ci_view ||
			!ci_issuer.booted || !ci_issuer.live ||
			ci_issuer.shut_down) {
		return;
	}
	if (!view || !view->surface || !view->surface->mapped ||
			!view->surface->resource || !view->container ||
			view->container->view != view ||
			ci_issuer.selected_candidate.native_view_ref != (uintptr_t)view ||
			ci_issuer.selected_candidate.native_container_ref !=
				(uintptr_t)view->container ||
			ci_issuer.selected_candidate.native_root_resource_ref !=
				(uintptr_t)view->surface->resource ||
			ci_lifetime.boot_a != ci_issuer.boot_a ||
			ci_lifetime.boot_b != ci_issuer.boot_b ||
			ci_lifetime.lifetime_sequence != ci_issuer.lifetime_floor ||
			ci_lifetime.transaction_sequence != 0 ||
			!transaction_ref || ci_real_commit_floor == UINT64_MAX ||
			ci_real_matched_this_lifetime == UINT64_MAX) {
		fail("real_transaction_instruction_lineage_drift");
	}
	++ci_real_commit_floor;
	++ci_real_matched_this_lifetime;
	fprintf(stderr,
		"TIMEW_CI_REAL_INSTRUCTION_BIND lifetime=%" PRIu64
		" commit_sequence=%" PRIu64
		" native_transaction_instruction=1 mapped_xdg=1"
		" configure_serial_nonzero=%d transaction_pointer=diagnostic_only"
		" physical_selection=NONE source=UNENROLLED c5_spent=NONE"
		" protected_receipt=0 native_move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_real_commit_floor,
		configure_serial != 0 ? 1 : 0);
	fflush(stderr);
}
"""
# Declare and define the callback in the disposable CI native probe; the
# normal repository's probe source stays unchanged.
p = one(
    p,
    'void timew_scope_a_ci_real_xdg_after_unmap_signal(struct sway_view *view) {',
    function_source + '\n'
    'void timew_scope_a_ci_real_xdg_after_unmap_signal(struct sway_view *view) {',
    "INSTRUCTION_LINEAGE_CALLBACK",
)
p = one(
    p,
    '\tif (!timew_scope_a_lineage_revoke(&ci_issuer, &ci_lifetime) ||',
    '\tif (ci_real_matched_this_lifetime == 0) {\n'
    '\t\tfail("no_real_instruction_for_selected_lifetime");\n'
    '\t}\n'
    '\tfprintf(stderr,\n'
    '\t\t"TIMEW_CI_REAL_INSTRUCTION_SEAL lifetime=%" PRIu64\n'
    '\t\t" real_commit_count=%" PRIu64\n'
    '\t\t" real_unmap=1 generation_closed=1"\n'
    '\t\t" protected_receipt=0 authority=NONE\\n",\n'
    '\t\tci_lifetime.lifetime_sequence, ci_real_matched_this_lifetime);\n'
    '\tfflush(stderr);\n'
    '\tci_real_matched_this_lifetime = 0;\n'
    '\tif (!timew_scope_a_lineage_revoke(&ci_issuer, &ci_lifetime) ||',
    "REAL_UNMAP_AND_COMMIT_SEAL",
)
# Trigger a size configure while the native mapped XDG lifetime is live.
# This is an ordinary, untrusted test IPC for the disposable foot app only.
a = one(
    a,
    'echo "Stage Director real mapped actor smoke: PASS"',
    '''echo "Stage Director real mapped actor smoke: PASS"
if [[ "${TIMEW_CI_REAL_INSTRUCTION_MODE:-}" == 1 ]]; then
  "$swaymsg_binary" -s "$ipc_socket" \\
    '[app_id="post-apollo-weather-news"] resize set width 1290px height 800px' \\
    >"$runtime/timew-ci-real-instruction-resize.log" 2>&1 || {
      echo "TIMEW_CI_REAL_INSTRUCTION_FAIL untrusted XDG resize fixture rejected" >&2
      cat "$runtime/timew-ci-real-instruction-resize.log" >&2
      exit 1
    }
  echo "TIMEW_CI_REAL_INSTRUCTION_TRIGGER native_xdg=1 fixture_only=1"
fi''',
    "ACTOR_FIXTURE_RESIZE",
)
# Validate all anchors before writing any modified disposable file.
txn.write_text(t)
probe.write_text(p)
actor.write_text(a)
print("TIMEW_CI_REAL_INSTRUCTION_INJECTION_PASS: real commit instruction"
      " bound to private mapped XDG lifetime in disposable runner only;"
      " native command authority NONE")
