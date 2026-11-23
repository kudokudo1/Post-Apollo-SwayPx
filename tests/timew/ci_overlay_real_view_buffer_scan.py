#!/usr/bin/env python3
"""Ephemeral CI-only SwayPX selected-XDG scene-buffer iteration witness.

A frame-done *iteration* is NOT proof of contribution to a presented frame.
Applies after parent #188's CI-only output PRESENT overlay. All edits are to
the disposable GitHub runner checkout, never to committed production files.
"""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_OUTSIDE_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NON_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
out = root / "sway/desktop/output.c"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"

def replace_one(s, a, b, name):
    if s.count(a) != 1:
        raise SystemExit(f"BLOCKED_VIEW_BUFFER_SOURCE_DRIFT_{name} count={s.count(a)}")
    return s.replace(a, b, 1)

o, p, s = [x.read_text() for x in (out, probe, smoke)]
if ("TIMEW_CI_VIEW_BUFFER_SCAN" in o or "TIMEW_CI_VIEW_BUFFER_SCAN" in p):
    raise SystemExit("BLOCKED_ALREADY_INSTRUMENTED_VIEW_BUFFER")
if ("timew_scope_a_ci_output_present(struct sway_output" not in p
        or "static void ci_scene_geometry(" not in p
        or "timew_scope_a_ci_output_present(struct sway_output" not in o):
    raise SystemExit("BLOCKED_PARENT_OUTPUT_PRESENT_OBSERVER_MISSING")

# Existing renderer code already rejects buffers not on the primary output,
# non-surface scene buffers, and walks ancestor nodes to get SWAY_SCENE_DESC_VIEW.
# Insert ONLY a passive observer at its unique found-view site in disposable CI.
o = replace_one(
    o,
    "extern void timew_scope_a_ci_output_present(struct sway_output *output);",
    "extern void timew_scope_a_ci_output_present(struct sway_output *output);\n"
    "extern void timew_scope_a_ci_view_buffer_scan(struct sway_output *output,\n"
    "\t\tstruct sway_view *view, struct wlr_scene_buffer *buffer);",
    "OUTPUT_PROBE_DECLARATION",
)
o = replace_one(
    o,
    "\t\tif (view) {\n\t\t\tview_max_render_time = view->max_render_time;",
    "\t\tif (view) {\n"
    "\t\t\ttimew_scope_a_ci_view_buffer_scan(output, view, buffer);\n"
    "\t\t\tview_max_render_time = view->max_render_time;",
    "EXISTING_SCENE_BUFFER_ANCESTOR",
)

source = r"""
/* A matching ancestor means this live XDG was in the SCENE BUFFER traversal
 * for this output. It does NOT establish buffer contribution, damage,
 * rendering, commit, presentation, final animation, or terminal geometry. */
static uint64_t ci_last_view_buffer_scan_sequence;
static uint64_t ci_view_buffer_scan_count;

void timew_scope_a_ci_view_buffer_scan(struct sway_output *output,
		struct sway_view *view, struct wlr_scene_buffer *buffer) {
	if (view != ci_view || !ci_view || !ci_issuer.live ||
			ci_issuer.shut_down || !ci_last_post_scene_sequence ||
			ci_last_view_buffer_scan_sequence == ci_last_post_scene_sequence) {
		return;
	}
	if (!buffer || !output || !output->wlr_output || !output->enabled ||
			!output->wlr_output->enabled ||
			buffer->primary_output != output->scene_output ||
			(uintptr_t)output !=
				ci_issuer.selected_candidate.native_output_ref ||
			!ci_view->surface || !ci_view->surface->mapped ||
			!ci_view->surface->resource ||
			ci_issuer.selected_candidate.native_root_resource_ref !=
				(uintptr_t)ci_view->surface->resource ||
			!ci_view->container || !ci_view->container->current.workspace ||
			ci_view->container->current.workspace->output != output) {
		fprintf(stderr,
			"TIMEW_CI_VIEW_BUFFER_DRIFT same_view_diagnostic=1 "
			"native_source_or_output_drift=1 authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_view_buffer_scan_count == UINT64_MAX) {
		fail("view_buffer_scan_counter_overflow");
	}
	ci_last_view_buffer_scan_sequence = ci_last_post_scene_sequence;
	++ci_view_buffer_scan_count;
	fprintf(stderr,
		"TIMEW_CI_VIEW_BUFFER_SCAN lifetime=%" PRIu64
		" sequence=%" PRIu64
		" ancestor_view=REAL_XDG scene_surface=1 primary_output=1"
		" frame_done_iterator=1"
		" contributed_to_present=UNVERIFIED rendered=UNVERIFIED"
		" selected_final_frame=UNVERIFIED"
		" client_ack=UNVERIFIED c5_spent=NONE source=UNENROLLED"
		" receipt=NONE move=OFF retry=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence);
	fflush(stderr);
}

static void ci_view_buffer_scan_seal(void) {
	fprintf(stderr,
		"TIMEW_CI_VIEW_BUFFER_SCAN_SEAL lifetime=%" PRIu64
		" native_scans=%" PRIu64
		" real_unmap=1 selected_final_frame=UNVERIFIED authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_view_buffer_scan_count);
	fflush(stderr);
	ci_last_view_buffer_scan_sequence = 0;
	ci_view_buffer_scan_count = 0;
}
"""
p = replace_one(p,
    "void timew_scope_a_ci_output_present(struct sway_output *output) {",
    source + "\nvoid timew_scope_a_ci_output_present(struct sway_output *output) {",
    "VIEW_BUFFER_PROBE_BODY",
)
p = replace_one(p,
    "static void ci_output_present_seal(void) {\n",
    "static void ci_output_present_seal(void) {\n"
    "\tci_view_buffer_scan_seal();\n",
    "VIEW_BUFFER_UNMAP_SEAL",
)
s = replace_one(s,
    '  python3 -B tests/timew/ci_assert_real_output_present.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_output_present.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_view_buffer_scan.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "VIEW_BUFFER_LOG_ASSERTION",
)

# Only write once ALL upstream source anchors have matched.
for path, text in ((out, o), (probe, p), (smoke, s)):
    path.write_text(text)
print("TIMEW_CI_VIEW_BUFFER_OVERLAY_PASS: actual XDG ancestor in existing "
      "wlr_scene_output_for_each_buffer frame-done path; "
      "presented-selected-view=UNVERIFIED authority=NONE")
