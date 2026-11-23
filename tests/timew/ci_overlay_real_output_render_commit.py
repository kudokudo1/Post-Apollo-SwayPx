#!/usr/bin/env python3
"""Disposable SwayPX output render-build/commit-success witness (CI only).

Apply after the #189 CI-only scene-buffer traversal overlay. This observes
successful wlr_output_commit_state for an output carrying the mapped test XDG,
NOT whether that XDG buffer contributed to or settled in the presented frame.
Original checked-in production source is unchanged.
"""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_OUTSIDE_EPHEMERAL_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_UNTRUSTED_RUNNER")

root = Path(__file__).resolve().parents[2]
output = root / "sway/desktop/output.c"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
meson = root / "sway/meson.build"

def once(src, anchor, replacement, label):
    count = src.count(anchor)
    if count != 1:
        raise SystemExit(f"BLOCKED_OUTPUT_RENDER_SOURCE_DRIFT_{label} count={count}")
    return src.replace(anchor, replacement, 1)

o, p, s, m = (f.read_text() for f in (output, probe, smoke, meson))
if "TIMEW_CI_RENDER_COMMIT" in o or "TIMEW_CI_RENDER_COMMIT" in p:
    raise SystemExit("BLOCKED_RENDER_ALREADY_INSTRUMENTED")
if ("timew_scope_a_ci_view_buffer_scan(struct sway_output" not in o
        or "static void ci_view_buffer_scan_seal(void)" not in p
        or "static void ci_output_present_seal(void)" not in p
        or "TIMEW_CI_VIEW_BUFFER_SCAN" not in p
        or "'timew_scope_a_native_txn_observer.c'" not in m):
    raise SystemExit("BLOCKED_PARENT_NATIVE_TRAVERSAL_NOT_LINKED")
if "'timew_scope_a_ci_render_commit.c'" in m:
    raise SystemExit("BLOCKED_UNEXPECTED_PRODUCTION_RENDER_SOURCE")

o = once(o,
    "extern void timew_scope_a_ci_output_present(struct sway_output *output);",
    "extern void timew_scope_a_ci_output_present(struct sway_output *output);\n"
    "extern void timew_scope_a_ci_render_commit_accepted(struct sway_output *output);",
    "OUTPUT_COMMIT_HELPER_DECL",
)
o = once(o,
    '\tif (!wlr_output_commit_state(output->wlr_output, &pending)) {\n'
    '\t\tsway_log(SWAY_ERROR, "Page-flip failed on output %s", output->wlr_output->name);\n'
    '\t}',
    '\tif (!wlr_output_commit_state(output->wlr_output, &pending)) {\n'
    '\t\tsway_log(SWAY_ERROR, "Page-flip failed on output %s", output->wlr_output->name);\n'
    '\t} else {\n'
    '\t\t/* Disposable CI observation only: this output accepted a built\n'
    '\t\t * render state. The selected XDG did not necessarily contribute. */\n'
    '\t\ttimew_scope_a_ci_render_commit_accepted(output);\n'
    '\t}',
    "REAL_OUTPUT_STATE_COMMIT_ACCEPTED",
)

source = r"""
/* One process-local output commit observation per post-arrange instruction.
 * This has NO signed source, buffer contribution, ACK, presentation, frame
 * settle, grant, movement or TimeW production authority. */
static uint64_t ci_last_render_commit_sequence;
static uint64_t ci_render_commit_count;

void timew_scope_a_ci_render_commit_accepted(struct sway_output *output) {
	if (!ci_view || !ci_issuer.live || ci_issuer.shut_down ||
			!ci_last_post_scene_sequence ||
			ci_last_render_commit_sequence == ci_last_post_scene_sequence) {
		return;
	}
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
		fprintf(stderr,
			"TIMEW_CI_RENDER_COMMIT_DRIFT source_or_output=CHANGED "
			"native_authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_render_commit_count == UINT64_MAX) {
		fail("render_commit_diagnostic_overflow");
	}
	++ci_render_commit_count;
	ci_last_render_commit_sequence = ci_last_post_scene_sequence;
	fprintf(stderr,
		"TIMEW_CI_RENDER_COMMIT lifetime=%" PRIu64
		" sequence=%" PRIu64
		" scene_state_built=1 output_commit_accepted=1 real_callback=1"
		" same_mapped_output=1"
		" xdg_rendered=UNVERIFIED xdg_contributed=UNVERIFIED"
		" xdg_final_presented=UNVERIFIED"
		" client_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF retry=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_render_commit_sequence);
	fflush(stderr);
}

static void ci_render_commit_seal(void) {
	fprintf(stderr,
		"TIMEW_CI_RENDER_COMMIT_SEAL lifetime=%" PRIu64
		" accepted_output_commits=%" PRIu64
		" real_unmap=1 xdg_final_presented=UNVERIFIED authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_render_commit_count);
	fflush(stderr);
	ci_last_render_commit_sequence = 0;
	ci_render_commit_count = 0;
}
"""
p = once(p,
    "void timew_scope_a_ci_output_present(struct sway_output *output) {",
    source + "\nvoid timew_scope_a_ci_output_present(struct sway_output *output) {",
    "RENDER_PROBE_IMPLEMENTATION",
)
p = once(p,
    "static void ci_output_present_seal(void) {\n",
    "static void ci_output_present_seal(void) {\n"
    "\tci_render_commit_seal();\n",
    "OUTPUT_LIFETIME_SEAL",
)
s = once(s,
    '  python3 -B tests/timew/ci_assert_real_view_buffer_scan.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_view_buffer_scan.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_output_render_commit.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "RENDER_LOG_ASSERTION",
)

# All source-drift checks completed before touching disposable checkout files.
output.write_text(o)
probe.write_text(p)
smoke.write_text(s)
print("TIMEW_CI_RENDER_COMMIT_OVERLAY_PASS: original output render-state "
      "build and accepted backend commit observed only in disposable SwayPX; "
      "selected view rendered/presented=UNVERIFIED authority=NONE")
