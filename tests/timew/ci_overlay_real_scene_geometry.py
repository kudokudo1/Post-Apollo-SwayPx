#!/usr/bin/env python3
"""Disposable native SwayPX scene-geometry observation, never a receipt.

Apply AFTER #172/#178/#179/#182 test-only overlays on a GitHub runner.
Only the runner checkout's Meson, probe and smoke are touched.
"""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_OUTSIDE_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NON_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
meson = root / "sway/meson.build"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
txn = root / "sway/desktop/transaction.c"

def one(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"BLOCKED_NATIVE_SCENE_SOURCE_DRIFT_{label} count={count}")
    return text.replace(old, new, 1)

m, p, s, t = (f.read_text() for f in (meson, probe, smoke, txn))
if "TIMEW_CI_NATIVE_SCENE_SNAPSHOT" in p or "TIMEW_CI_NATIVE_SCENE_SNAPSHOT" in t:
    raise SystemExit("BLOCKED_SCENE_ALREADY_INSTRUMENTED")
if "'timew_scope_a_observed_lineage.c'" not in m or \
        "void timew_scope_a_ci_outcome_scene(" not in p or \
        'TIMEW_CI_REAL_INSTRUCTION_BIND' not in p:
    raise SystemExit("BLOCKED_MISSING_PARENT_NATIVE_XDG_INSTRUCTION_OUTCOME")
if "'timew_scope_a_native_txn_observer.c'" in m:
    raise SystemExit("BLOCKED_NATIVE_OBSERVER_ALREADY_LINKED")

m = one(m,
    "\t'timew_scope_a_observed_lineage.c',",
    "\t'timew_scope_a_observed_lineage.c',\n"
    "\t'timew_scope_a_native_txn_observer.c',",
    "CI_NATIVE_OBSERVER_MESON",
)
p = one(p,
    '#include "sway/timew_scope_a_ci_real_outcome.h"',
    '#include "sway/timew_scope_a_ci_real_outcome.h"\n'
    '#include "sway/timew_scope_a_native_txn_observer.h"',
    "CI_NATIVE_OBSERVER_PROBE_HEADER",
)

source = r"""
/* The existing dormant native reader is run ONLY after actual
 * transaction_apply and arrange_root. A matching CURRENT value does not
 * show an ACK, frame presentation, source identity, consent or settlement. */
static void ci_scene_geometry(const struct ci_outcome_record *record) {
	if (!ci_view || !ci_issuer.booted || !ci_issuer.live ||
			ci_issuer.shut_down || !record ||
			record->lifetime != ci_lifetime.lifetime_sequence) {
		fail("scene_geometry_invalid_live_candidate");
	}
	struct timew_scope_a_native_txn_snapshot snap = {0};
	if (!timew_scope_a_native_txn_snapshot_read(ci_view, &snap)) {
		fprintf(stderr,
			"TIMEW_CI_NATIVE_SCENE_DEFER lifetime=%" PRIu64
			" sequence=%" PRIu64
			" native_snapshot_unavailable=1 authority=NONE\n",
			record->lifetime, record->commit_sequence);
		fflush(stderr);
		return;
	}
	const struct timew_scope_a_lineage_context *origin =
		&ci_issuer.selected_candidate;
	if (snap.native_container_ref != origin->native_container_ref ||
			snap.native_root_surface_ref != origin->native_root_resource_ref ||
			snap.current_workspace_ref != origin->native_workspace_ref ||
			snap.pending_workspace_ref != origin->native_workspace_ref ||
			snap.native_output_ref != origin->native_output_ref ||
			snap.output_scale_binary64_bits != origin->output_scale_bits ||
			snap.output_transform != origin->output_transform) {
		fprintf(stderr,
			"TIMEW_CI_NATIVE_SCENE_DRIFT lifetime=%" PRIu64
			" sequence=%" PRIu64
			" native_context_changed=1 authority=NONE\n",
			record->lifetime, record->commit_sequence);
		fflush(stderr);
		return;
	}
	double x, y, w, h, px, py, pw, ph;
	if (!timew_scope_a_exact_geometry_read(&snap.current, &x, &y, &w, &h) ||
			!timew_scope_a_exact_geometry_read(
				&snap.pending, &px, &py, &pw, &ph) ||
			!snap.current_layout_value_observed ||
			snap.native_client_ack_proven ||
			snap.native_transaction_identity_proven ||
			snap.native_scene_presented_proven ||
			snap.output_epoch_proven ||
			snap.physical_selection_proven ||
			snap.source_proven || snap.c5_spend_proven ||
			snap.protected_geometry_ready ||
			snap.native_resize_allowed || snap.native_move_allowed) {
		fail("scene_geometry_observer_false_positive_authority");
	}
	/* Hex bits preserve real fractional logical coordinates exactly. They
	 * are diagnostic values, not an approved signed wire representation. */
	fprintf(stderr,
		"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=%" PRIu64
		" sequence=%" PRIu64
		" current_x=%016" PRIx64 " current_y=%016" PRIx64
		" current_w=%016" PRIx64 " current_h=%016" PRIx64
		" pending_x=%016" PRIx64 " pending_y=%016" PRIx64
		" pending_w=%016" PRIx64 " pending_h=%016" PRIx64
		" scale=%016" PRIx64 " transform=%u"
		" output_lx=%d output_ly=%d output_w=%d output_h=%d"
		" native_source=REAL_XDG same_context=1"
		" ack=0 frame=0 epoch_proof=0 source=UNENROLLED"
		" selected=0 spent=0 receipt=NONE action=NONE\n",
		record->lifetime, record->commit_sequence,
		snap.current.x_bits, snap.current.y_bits,
		snap.current.width_bits, snap.current.height_bits,
		snap.pending.x_bits, snap.pending.y_bits,
		snap.pending.width_bits, snap.pending.height_bits,
		snap.output_scale_binary64_bits, snap.output_transform,
		snap.output_lx, snap.output_ly,
		snap.output_transformed_width, snap.output_transformed_height);
	fflush(stderr);
}
"""
p = one(p,
    "void timew_scope_a_ci_outcome_scene(uintptr_t ref) {",
    source + "\nvoid timew_scope_a_ci_outcome_scene(uintptr_t ref) {",
    "CI_SCENE_GEOMETRY_CALLER",
)
p = one(p,
    '\t\t\tci_outcome_emit("SCENE", record);',
    '\t\t\tci_outcome_emit("SCENE", record);\n'
    '\t\t\tci_scene_geometry(record);',
    "AFTER_REAL_SCENE_ARRANGE",
)
s = one(s,
    '  python3 -B tests/timew/ci_assert_real_instruction_outcomes.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_instruction_outcomes.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_scene_geometry.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "CI_NATIVE_GEOMETRY_ASSERT",
)
# Validate all anchors first, then write only the disposable CI files.
meson.write_text(m)
probe.write_text(p)
smoke.write_text(s)
print("TIMEW_CI_NATIVE_SCENE_OVERLAY_PASS: original SwayPX reader linked only "
      "in disposable runner, no source/selection/ACK/frame/C5/move authority")
