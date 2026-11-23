#!/usr/bin/env python3
"""CI-only true wlroots output commit_seq ↔ PRESENT commit_seq observation.

Neither serial is a selected XDG surface buffer ID, receipt or authority.
Apply AFTER all parent #191 native overlays on disposable GitHub Actions only.
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
meson = root / "sway/meson.build"

def once(text, target, replacement, name):
    count = text.count(target)
    if count != 1:
        raise SystemExit(
            f"BLOCKED_OUTPUT_COMMIT_SEQ_SOURCE_DRIFT_{name} count={count}")
    return text.replace(target, replacement, 1)

o, p, s, m = (x.read_text() for x in (out, probe, smoke, meson))
if "TIMEW_CI_OUTPUT_SEQ_" in o or "TIMEW_CI_OUTPUT_SEQ_" in p:
    raise SystemExit("BLOCKED_OUTPUT_SERIAL_ALREADY_INSTRUMENTED")
if ("timew_scope_a_ci_render_commit_accepted(output);" not in o
        or "timew_scope_a_ci_output_present(output);" not in o
        or "static void ci_render_commit_seal(void)" not in p
        or "static void ci_output_present_seal(void)" not in p
        or "'timew_scope_a_native_txn_observer.c'" not in m):
    raise SystemExit("BLOCKED_MISSING_PARENT_REAL_COMMIT_AND_PRESENT")

o = once(o,
    "extern void timew_scope_a_ci_render_commit_accepted(struct sway_output *output);",
    "extern void timew_scope_a_ci_render_commit_accepted(struct sway_output *output);\n"
    "extern void timew_scope_a_ci_output_commit_serial(struct sway_output *output,\n"
    "\t\tuint32_t actual_output_commit_seq);\n"
    "extern void timew_scope_a_ci_output_present_serial(struct sway_output *output,\n"
    "\t\tuint32_t event_commit_seq);",
    "SOURCE_SERIAL_CALLBACK_DECLARATIONS",
)
o = once(o,
    "\t\ttimew_scope_a_ci_render_commit_accepted(output);",
    "\t\ttimew_scope_a_ci_render_commit_accepted(output);\n"
    "\t\t/* Actual wlroots output commit counter AFTER accepted commit.\n"
    "\t\t * Not a selected-XDG buffer or authenticated receipt serial. */\n"
    "\t\ttimew_scope_a_ci_output_commit_serial(output,\n"
    "\t\t\toutput->wlr_output->commit_seq);",
    "AFTER_REAL_BACKEND_ACCEPTANCE",
)
o = once(o,
    "\ttimew_scope_a_ci_output_present(output);",
    "\ttimew_scope_a_ci_output_present(output);\n"
    "\t/* Actual wlroots present event's referenced output commit serial. */\n"
    "\ttimew_scope_a_ci_output_present_serial(output, output_event->commit_seq);",
    "AFTER_REAL_OUTPUT_PRESENT",
)

source = r"""
/* A diagnostic sample of the actual output commit counter and the exact
 * output PRESENT event's referenced commit. The native backend can deliver
 * an event out-of-order relative to this observer. Do not retroactively
 * match an earlier event to a later diagnostic COMMIT record. */
static uint64_t ci_output_seq_commit_count;
static uint64_t ci_output_seq_present_count;

static bool ci_output_seq_same_candidate(struct sway_output *output) {
	return ci_view && ci_issuer.live && !ci_issuer.shut_down &&
		ci_last_post_scene_sequence && output && output->wlr_output &&
		output->enabled && output->wlr_output->enabled &&
		(uintptr_t)output ==
			ci_issuer.selected_candidate.native_output_ref &&
		ci_view->surface && ci_view->surface->mapped &&
		ci_view->surface->resource &&
		ci_issuer.selected_candidate.native_root_resource_ref ==
			(uintptr_t)ci_view->surface->resource &&
		ci_view->container && ci_view->container->current.workspace &&
		ci_view->container->current.workspace->output == output;
}

void timew_scope_a_ci_output_commit_serial(struct sway_output *output,
		uint32_t actual_output_commit_seq) {
	if (!ci_issuer.live || !ci_last_post_scene_sequence) {
		return;
	}
	if (!ci_output_seq_same_candidate(output)) {
		fprintf(stderr, "TIMEW_CI_OUTPUT_SEQ_DRIFT phase=COMMIT "
			"candidate_or_output_changed=1 authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_output_seq_commit_count == UINT64_MAX) {
		fail("output_serial_commit_observer_overflow");
	}
	++ci_output_seq_commit_count;
	fprintf(stderr,
		"TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime=%" PRIu64
		" sequence=%" PRIu64 " output_commit_seq=%" PRIu32
		" accepted_output_state=1 real_wlroots=1"
		" same_mapped_xdg_output=1 xdg_buffer=UNVERIFIED"
		" source=UNENROLLED c5_spent=NONE receipt=NONE"
		" move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		actual_output_commit_seq);
	fflush(stderr);
}

void timew_scope_a_ci_output_present_serial(struct sway_output *output,
		uint32_t event_commit_seq) {
	if (!ci_issuer.live || !ci_last_post_scene_sequence) {
		return;
	}
	if (!ci_output_seq_same_candidate(output)) {
		fprintf(stderr, "TIMEW_CI_OUTPUT_SEQ_DRIFT phase=PRESENT "
			"candidate_or_output_changed=1 authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_output_seq_present_count == UINT64_MAX) {
		fail("output_serial_present_observer_overflow");
	}
	++ci_output_seq_present_count;
	fprintf(stderr,
		"TIMEW_CI_OUTPUT_SEQ_PRESENT lifetime=%" PRIu64
		" sequence=%" PRIu64 " output_commit_seq=%" PRIu32
		" output_presented=1 real_wlroots_event=1"
		" same_mapped_xdg_output=1"
		" xdg_buffer=UNVERIFIED xdg_final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		event_commit_seq);
	fflush(stderr);
}

static void ci_output_seq_seal(void) {
	fprintf(stderr,
		"TIMEW_CI_OUTPUT_SEQ_SEAL lifetime=%" PRIu64
		" observed_output_commits=%" PRIu64
		" observed_output_presents=%" PRIu64
		" real_unmap=1 xdg_final_frame=UNVERIFIED authority=NONE\n",
		ci_lifetime.lifetime_sequence,
		ci_output_seq_commit_count, ci_output_seq_present_count);
	fflush(stderr);
	ci_output_seq_commit_count = 0;
	ci_output_seq_present_count = 0;
}
"""
p = once(p,
    "void timew_scope_a_ci_output_present(struct sway_output *output) {",
    source + "\nvoid timew_scope_a_ci_output_present(struct sway_output *output) {",
    "EPHEMERAL_ACTUAL_WLROOTS_SERIAL_OBSERVERS",
)
p = once(p,
    "static void ci_output_present_seal(void) {\n",
    "static void ci_output_present_seal(void) {\n"
    "\tci_output_seq_seal();\n",
    "SEALED_BEFORE_XDG_UNMAP",
)
s = once(s,
    '  python3 -B tests/timew/ci_assert_real_output_render_commit.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_output_render_commit.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_output_commit_serial.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "REAL_WLROOTS_SERIAL_CORRELATION_CHECKER",
)
# All required anchors verified before modifying disposable runner files.
for f, text in ((out, o), (probe, p), (smoke, s)):
    f.write_text(text)
print("TIMEW_CI_OUTPUT_SEQ_OVERLAY_PASS: actual wlroots output commit_seq "
      "and output PRESENT commit_seq observable, never selected-XDG settlement "
      "or authority; source hooks discarded after ephemeral GitHub run")
