#!/usr/bin/env python3
"""CI-only SwayPX wlr_output PRESENT event lineage; NOT a selected-view frame."""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_OUTSIDE_GITHUB_CI")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NON_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
output = root / "sway/desktop/output.c"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
meson = root / "sway/meson.build"

def one(source, original, replace, name):
    if source.count(original) != 1:
        raise SystemExit(f"BLOCKED_OUTPUT_PRESENT_SOURCE_DRIFT_{name} count={source.count(original)}")
    return source.replace(original, replace, 1)

o, p, s, m = (x.read_text() for x in (output, probe, smoke, meson))
if "TIMEW_CI_OUTPUT_PRESENT" in o or "TIMEW_CI_OUTPUT_PRESENT" in p:
    raise SystemExit("BLOCKED_ALREADY_INSTRUMENTED_OUTPUT_PRESENT")
if "static void ci_scene_geometry(" not in p or \
        "TIMEW_CI_NATIVE_SCENE_SNAPSHOT" not in p or \
        "'timew_scope_a_native_txn_observer.c'" not in m:
    raise SystemExit("BLOCKED_PARENT_NATIVE_SCENE_MISSING")
if "TIMEW_CI_OUTPUT_PRESENT" in m:
    raise SystemExit("BLOCKED_PRODUCTION_MESON_OUTPUT_PRESENT")

# In an ephemeral CI checkout only, this hook is placed AFTER the existing
# output's event.present success check. It cannot attest an XDG frame.
o = one(o,
    '#include "sway/output.h"',
    '#include "sway/output.h"\n'
    'extern void timew_scope_a_ci_output_present(struct sway_output *output);',
    "SOURCE_PRESENT_INCLUDE",
)
o = one(o,
    '\toutput->last_presentation = output_event->when;',
    '\toutput->last_presentation = output_event->when;\n'
    '\ttimew_scope_a_ci_output_present(output);',
    "ACTUAL_PRESENTED_CALLBACK",
)
source = r"""
/* This table is transient, CI-only and lives solely in the same SwayPX
 * process. No output frame may be promoted into a selected-view receipt. */
static uint64_t ci_last_post_scene_sequence;
static uint64_t ci_last_output_present_sequence;
static uint64_t ci_output_present_count;

void timew_scope_a_ci_output_present(struct sway_output *output) {
	if (!ci_view || !ci_issuer.live || ci_issuer.shut_down ||
			!ci_last_post_scene_sequence ||
			ci_last_post_scene_sequence == ci_last_output_present_sequence) {
		return;
	}
	/* Same mapped fixture and output object: still NOT evidence that the
	 * candidate's XDG buffers contributed to the presented output frame. */
	if (!output || !output->wlr_output || !output->enabled ||
			!output->wlr_output->enabled ||
			(uintptr_t)output !=
				ci_issuer.selected_candidate.native_output_ref ||
			!ci_view->surface || !ci_view->surface->mapped ||
			!ci_view->surface->resource ||
			ci_issuer.selected_candidate.native_root_resource_ref !=
				(uintptr_t)ci_view->surface->resource ||
			!ci_view->container || !ci_view->container->current.workspace ||
			ci_view->container->current.workspace->output != output) {
		fprintf(stderr, "TIMEW_CI_OUTPUT_PRESENT_DRIFT "
			"native_output_or_view_context_changed=1 authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_output_present_count == UINT64_MAX) {
		fail("output_present_diagnostic_overflow");
	}
	++ci_output_present_count;
	ci_last_output_present_sequence = ci_last_post_scene_sequence;
	fprintf(stderr,
		"TIMEW_CI_OUTPUT_PRESENT lifetime=%" PRIu64
		" sequence=%" PRIu64
		" output_presented=1 real_callback=1 same_output_context=1"
		" selected_xdg_frame=UNVERIFIED final_view_frame=UNVERIFIED"
		" source=UNENROLLED ack=UNVERIFIED c5_spent=NONE"
		" receipt=NONE move=OFF retry=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_output_present_sequence);
	fflush(stderr);
}

static void ci_output_present_seal(void) {
	fprintf(stderr,
		"TIMEW_CI_OUTPUT_PRESENT_SEAL lifetime=%" PRIu64
		" observed_output_presentations=%" PRIu64
		" real_unmap=1 selected_xdg_frame=UNVERIFIED authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_output_present_count);
	fflush(stderr);
	ci_last_post_scene_sequence = 0;
	ci_last_output_present_sequence = 0;
	ci_output_present_count = 0;
}
"""
p = one(p,
    "static void ci_scene_geometry(const struct ci_outcome_record *record) {",
    source + "\nstatic void ci_scene_geometry(const struct ci_outcome_record *record) {",
    "PROBE_PRESENT_FUNCTION",
)
p = one(p,
    '\t/* Hex bits preserve real fractional logical coordinates exactly. They',
    '\tci_last_post_scene_sequence = record->commit_sequence;\n'
    '\t/* Hex bits preserve real fractional logical coordinates exactly. They',
    "BOUND_SCENE_BEFORE_PRESENT",
)
p = one(p,
    '\tif (ci_real_matched_this_lifetime == 0) {',
    '\tci_output_present_seal();\n'
    '\tif (ci_real_matched_this_lifetime == 0) {',
    "OUTPUT_PRESENT_SEAL_UNMAP",
)
s = one(s,
    '  python3 -B tests/timew/ci_assert_real_scene_geometry.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_scene_geometry.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_output_present.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "REAL_OUTPUT_PRESENT_ASSERT",
)
# All anchors checked before modifying the disposable runner's source.
output.write_text(o)
probe.write_text(p)
smoke.write_text(s)
print("TIMEW_CI_OUTPUT_PRESENT_OVERLAY_PASS: wlr_output event PRESENT "
      "on original SwayPX output.c, source-only temporary runner hook; "
      "selected XDG frame UNVERIFIED, protected authority NONE")
