#!/usr/bin/env python3
"""Attach a private, DENY-ONLY lineage issuer to actual mapped XDG CI callbacks.

Run AFTER tests/timew/ci_inject_real_xdg_lifecycle.py, in the disposable
GitHub Actions checkout ONLY. No production Meson/view/seat/transaction edits
are committed. The real XDG test actor's app_id is a fixture FILTER, never
a physical operator-selection or trustworthy installed-compositor source.
"""
from pathlib import Path
import os

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_NOT_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NOT_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
meson = root / "sway/meson.build"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"

def one(text: str, anchor: str, replacement: str, tag: str) -> str:
    n = text.count(anchor)
    if n != 1:
        raise SystemExit(f"BLOCKED_LINEAGE_SOURCE_DRIFT_{tag}: {n}")
    return text.replace(anchor, replacement, 1)

m, p, s = meson.read_text(), probe.read_text(), smoke.read_text()
if "'timew_scope_a_ci_real_xdg.c'" not in m:
    raise SystemExit("BLOCKED_NO_PARENT_CI_XDG_HOOK")
if "'timew_scope_a_observed_lineage.c'" in m or "TIMEW_CI_LINEAGE_OPEN" in p:
    raise SystemExit("BLOCKED_ALREADY_INSTRUMENTED")
m = one(m, "\t'timew_scope_a_ci_real_xdg.c',",
    "\t'timew_scope_a_ci_real_xdg.c',\n"
    "\t'timew_scope_a_observed_lineage.c',",
    "MESON",
)
p = one(p, '#include "sway/timew_scope_a_exact_geometry.h"',
    '#include "sway/timew_scope_a_exact_geometry.h"\n'
    '#include "sway/timew_scope_a_observed_lineage.h"\n'
    '#include "sway/output.h"\n'
    '#include <wlr/types/wlr_output.h>',
    "PRIVATE_INCLUDE",
)
p = one(p, 'static bool ci_initialized;',
    'static bool ci_initialized;\n'
    'static struct timew_scope_a_lineage_issuer ci_issuer;\n'
    'static struct timew_scope_a_lineage_tag ci_lifetime;\n'
    'static struct timew_scope_a_lineage_tag ci_transaction;',
    "ISSUER_STATE",
)
p = one(p,
    "\t\ttimew_scope_a_lifecycle_signals_init(&ci_watch);\n"
    "\t\tci_initialized = true;",
    "\t\ttimew_scope_a_lifecycle_signals_init(&ci_watch);\n"
    "\t\tif (!timew_scope_a_lineage_boot(&ci_issuer)) {\n"
    "\t\t\tfail(\"private_boot_entropy_unavailable\");\n"
    "\t\t}\n"
    "\t\tci_initialized = true;",
    "PRIVATE_BOOT",
)
native_bind = (
    '\tci_view = view;\n'
    '\tfprintf(stderr,\n'
    '\t\t"TIMEW_CI_REAL_XDG_MAPPED gen=%" PRIu64'
)
new_bind = """	if (!ws->output->wlr_output) {
		fail("no_real_native_output");
	}
	const double ci_scale = ws->output->wlr_output->scale;
	uint64_t ci_scale_bits;
	memcpy(&ci_scale_bits, &ci_scale, sizeof(ci_scale_bits));
	const struct timew_scope_a_lineage_context ci_context = {
		.native_view_ref = (uintptr_t)view,
		.native_container_ref = (uintptr_t)con,
		.native_root_resource_ref = (uintptr_t)view->surface->resource,
		.native_client_ref = (uintptr_t)
			wl_resource_get_client(view->surface->resource),
		.native_seat_ref = (uintptr_t)seat->wlr_seat,
		.native_output_ref = (uintptr_t)ws->output,
		.native_workspace_ref = (uintptr_t)ws,
		.seat_epoch = 1, .session_epoch = 1, .output_epoch = 1,
		.workspace_epoch = 1, /* CI fixtures, not enrolled epochs */
		.output_scale_bits = ci_scale_bits,
		.output_transform = (uint32_t)ws->output->wlr_output->transform,
		.pending = exact,
		.mapped = true, .exact_xdg_toplevel = true, .floating = true,
	};
	if (!timew_scope_a_lineage_begin(
			&ci_issuer, &ci_context, &ci_lifetime)) {
		fail("real_xdg_private_lineage_begin_failed");
	}
	if (!timew_scope_a_lineage_begin_transaction(
			&ci_issuer, &ci_lifetime, &ci_context, &ci_transaction)) {
		fail("real_xdg_private_transaction_begin_failed");
	}
	struct timew_scope_a_lineage_assessment ci_result =
		timew_scope_a_lineage_observe_transaction(
			&ci_issuer, &ci_transaction, &ci_context);
	if (!ci_result.candidate_correlated_only ||
			!ci_result.transaction_correlated_only ||
			ci_result.physical_selection_authenticated ||
			ci_result.native_object_origin_authenticated ||
			ci_result.source_enrolled || ci_result.c5_spent ||
			ci_result.resize_allowed || ci_result.move_allowed ||
			ci_result.protected_receipt_verified ||
			ci_result.scene_settled_verified) {
		fail("private_lineage_authority_or_correlation");
	}
	fprintf(stderr,
		"TIMEW_CI_LINEAGE_OPEN lifetime=%" PRIu64
		" transaction=%" PRIu64
		" native_xdg=1 boot_tag=local_only geometry=exact "
		"operator_selection=NONE source=UNENROLLED "
		"protected_receipt=0 native_move=OFF authority=NONE\\n",
		ci_lifetime.lifetime_sequence,
		ci_transaction.transaction_sequence);
	fflush(stderr);
"""
# Do not expose raw boot randomness, native pointers or C5 data in logs.
p = one(p, native_bind, new_bind + native_bind, "REAL_MAPPED_LIFETIME")
before_unmap = (
    '\tfprintf(stderr,\n'
    '\t\t"TIMEW_CI_REAL_XDG_UNMAPPED gen=%" PRIu64'
)
new_unmap = """	if (!timew_scope_a_lineage_revoke(&ci_issuer, &ci_lifetime) ||
			ci_issuer.live || ci_issuer.transaction_live ||
			ci_issuer.lifetime_floor != ci_generation ||
			ci_issuer.transaction_floor != ci_generation) {
		fail("private_lineage_unmap_revoke_or_floors");
	}
	fprintf(stderr,
		"TIMEW_CI_LINEAGE_REVOKED lifetime=%" PRIu64
		" transaction=%" PRIu64
		" real_unmap=1 old_generation_invalid=1 "
		"privileged_action=0\\n",
		ci_lifetime.lifetime_sequence,
		ci_transaction.transaction_sequence);
	fflush(stderr);
"""
p = one(p, before_unmap, new_unmap + before_unmap, "REAL_UNMAP")
anchor = (
    '  echo "TIMEW_CI_REAL_XDG_RUNTIME PASS: '
    '2 real mapped XDG lifetimes, synchronous unmap-before-destroy, '
    'exact pending binary64=2, zero native authority"'
)
replacement = '''  python3 -B tests/timew/ci_assert_real_observed_lineage.py "$log"
  ''' + anchor.strip() + '''
'''
s = one(s, anchor, replacement.rstrip("\n"), "REAL_RUNTIME_CHECK")
# Write only after validating all source guards.
meson.write_text(m)
probe.write_text(p)
smoke.write_text(s)
print("TIMEW_CI_LINEAGE_INJECTION_PASS: ephemeral real XDG private boot/lifetime/"
      "transaction issuer, production wiring NONE")
