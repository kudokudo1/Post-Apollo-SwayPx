#!/usr/bin/env python3
"""Ephemeral native XDG root committed-state generation witness.

Observe real wlroots wlr_surface.current.seq at exact SceneFX root sample,
accepted output commit_seq and output PRESENT commit_seq. A changed/wrapped
surface generation means UNKNOWN/HOLD, not retroactive selected-frame proof.
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

def once(src, anchor, replacement, name):
    n = src.count(anchor)
    if n != 1:
        raise SystemExit(f"BLOCKED_XDG_GENERATION_SOURCE_DRIFT_{name} count={n}")
    return src.replace(anchor, replacement, 1)

o, p, s, m = [x.read_text() for x in (out, probe, smoke, meson)]
if "TIMEW_CI_XDG_GEN" in o or "TIMEW_CI_XDG_GEN" in p:
    raise SystemExit("BLOCKED_ALREADY_INSTRUMENTED_GENERATION")
if ("timew_scope_a_ci_root_sample_prebuild(output);" not in o or
        "timew_scope_a_ci_output_commit_serial(output" not in o or
        "timew_scope_a_ci_output_present_serial(output" not in o or
        "static void ci_sample_root_seal(void)" not in p or
        "static bool ci_output_seq_same_candidate(" not in p or
        "'timew_scope_a_native_txn_observer.c'" not in m):
    raise SystemExit("BLOCKED_MISSING_REAL_PARENT_SCENEFX_AND_WLROOTS")

o = once(o,
    "extern void timew_scope_a_ci_root_sample_prebuild(struct sway_output *output);",
    "extern void timew_scope_a_ci_root_sample_prebuild(struct sway_output *output);\n"
    "extern void timew_scope_a_ci_xdg_generation_output_commit(\n"
    "\t\tstruct sway_output *output, uint32_t commit_seq);\n"
    "extern void timew_scope_a_ci_xdg_generation_output_present(\n"
    "\t\tstruct sway_output *output, uint32_t commit_seq);",
    "ROOT_GENERATION_CALLBACK_DECL",
)
o = once(o,
    "\t\ttimew_scope_a_ci_output_commit_serial(output,\n"
    "\t\t\toutput->wlr_output->commit_seq);",
    "\t\ttimew_scope_a_ci_output_commit_serial(output,\n"
    "\t\t\toutput->wlr_output->commit_seq);\n"
    "\t\t/* CI-only capture: root surface generation at actual accepted\n"
    "\t\t * output serial, not a final buffer presentation receipt. */\n"
    "\t\ttimew_scope_a_ci_xdg_generation_output_commit(output,\n"
    "\t\t\toutput->wlr_output->commit_seq);",
    "AFTER_WLROOTS_ACCEPTED_OUTPUT_SERIAL",
)
o = once(o,
    "\ttimew_scope_a_ci_output_present_serial(output, output_event->commit_seq);",
    "\ttimew_scope_a_ci_output_present_serial(output, output_event->commit_seq);\n"
    "\ttimew_scope_a_ci_xdg_generation_output_present(output,\n"
    "\t\toutput_event->commit_seq);",
    "AFTER_REAL_OUTPUT_PRESENT_SERIAL",
)

source = r"""
/* A uint32 wlroots SURFACE current.seq is per-surface committed state, not
 * an immutable buffer hash, configure ACK, physical frame or signed receipt.
 * Never use it outside this test XDG's bounded in-process lifetime. */
static uint64_t ci_xdg_gen_sample_scene;
static uint32_t ci_xdg_gen_sample_value;
static uint64_t ci_xdg_gen_samples;
static uint64_t ci_xdg_gen_commits;
static uint64_t ci_xdg_gen_presents;
static uint64_t ci_xdg_gen_last_commit_scene;
static uint64_t ci_xdg_gen_last_present_scene;

static void ci_xdg_gen_sample(void) {
	if (!ci_sample_root_active || !ci_sample_root_buffer ||
			!ci_view || !ci_view->surface ||
			!ci_view->surface->mapped ||
			!ci_last_post_scene_sequence) {
		return;
	}
	if (ci_xdg_gen_samples == UINT64_MAX) {
		fail("xdg_gen_sample_counter_overflow");
	}
	++ci_xdg_gen_samples;
	ci_xdg_gen_sample_scene = ci_last_post_scene_sequence;
	ci_xdg_gen_sample_value = ci_view->surface->current.seq;
	fprintf(stderr,
		"TIMEW_CI_XDG_GEN_SAMPLE lifetime=%" PRIu64
		" sequence=%" PRIu64 " surface_state_seq=%" PRIu32
		" exact_native_root=1 real_scenefx_sample=1"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		ci_xdg_gen_sample_value);
	fflush(stderr);
}

void timew_scope_a_ci_xdg_generation_output_commit(
		struct sway_output *output, uint32_t commit_seq) {
	if (!ci_output_seq_same_candidate(output) ||
			!ci_last_post_scene_sequence ||
			ci_last_output_seq_commit_scene != ci_last_post_scene_sequence ||
			ci_xdg_gen_last_commit_scene == ci_last_post_scene_sequence) {
		return;
	}
	if (ci_xdg_gen_commits == UINT64_MAX) {
		fail("xdg_gen_commit_counter_overflow");
	}
	++ci_xdg_gen_commits;
	ci_xdg_gen_last_commit_scene = ci_last_post_scene_sequence;
	bool sample_in_scene =
		ci_xdg_gen_sample_scene == ci_last_post_scene_sequence;
	uint32_t sampled = sample_in_scene ? ci_xdg_gen_sample_value : 0;
	fprintf(stderr,
		"TIMEW_CI_XDG_GEN_COMMIT lifetime=%" PRIu64
		" sequence=%" PRIu64 " output_commit_seq=%" PRIu32
		" sampled_this_scene=%u sampled_surface_seq=%" PRIu32
		" current_surface_seq=%" PRIu32
		" accepted_output_commit=1"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		commit_seq, sample_in_scene ? 1u : 0u, sampled,
		ci_view->surface->current.seq);
	fflush(stderr);
}

void timew_scope_a_ci_xdg_generation_output_present(
		struct sway_output *output, uint32_t commit_seq) {
	if (!ci_output_seq_same_candidate(output) ||
			!ci_last_post_scene_sequence ||
			ci_last_output_seq_present_scene != ci_last_post_scene_sequence ||
			ci_xdg_gen_last_present_scene == ci_last_post_scene_sequence) {
		return;
	}
	if (ci_xdg_gen_presents == UINT64_MAX) {
		fail("xdg_gen_present_counter_overflow");
	}
	++ci_xdg_gen_presents;
	ci_xdg_gen_last_present_scene = ci_last_post_scene_sequence;
	fprintf(stderr,
		"TIMEW_CI_XDG_GEN_PRESENT lifetime=%" PRIu64
		" sequence=%" PRIu64 " output_commit_seq=%" PRIu32
		" current_surface_seq=%" PRIu32 " real_output_present=1"
		" sampled_buffer_in_presented_frame=UNVERIFIED"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		commit_seq, ci_view->surface->current.seq);
	fflush(stderr);
}

static void ci_xdg_gen_seal(void) {
	fprintf(stderr,
		"TIMEW_CI_XDG_GEN_SEAL lifetime=%" PRIu64
		" samples=%" PRIu64 " output_commits=%" PRIu64
		" output_presents=%" PRIu64
		" real_unmap=1 final_frame=UNVERIFIED authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_xdg_gen_samples,
		ci_xdg_gen_commits, ci_xdg_gen_presents);
	fflush(stderr);
	ci_xdg_gen_sample_scene = 0;
	ci_xdg_gen_sample_value = 0;
	ci_xdg_gen_samples = 0;
	ci_xdg_gen_commits = 0;
	ci_xdg_gen_presents = 0;
	ci_xdg_gen_last_commit_scene = 0;
	ci_xdg_gen_last_present_scene = 0;
}
"""
p = once(p,
    "static void ci_sample_root_detach(void) {",
    source + "\nstatic void ci_sample_root_detach(void) {",
    "SURFACE_GENERATION_PROBE_IMPLEMENTATION",
)
# The real SceneFX sample message must precede its surface generation read
# in the checker, while the underlying compositor event stays unchanged.
p = once(p,
    "\t\tci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,\n"
    "\t\tevent->direct_scanout ? 1u : 0u);\n"
    "\tfflush(stderr);\n}",
    "\t\tci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,\n"
    "\t\tevent->direct_scanout ? 1u : 0u);\n"
    "\tfflush(stderr);\n"
    "\tci_xdg_gen_sample();\n}",
    "AFTER_ACTUAL_SCENEFX_ROOT_SAMPLE",
)
p = once(p,
    "static void ci_sample_root_seal(void) {\n"
    "\tci_sample_root_detach();",
    "static void ci_sample_root_seal(void) {\n"
    "\tci_sample_root_detach();\n"
    "\tci_xdg_gen_seal();",
    "REVOKE_AND_SEAL_GENERATION_BEFORE_UNMAP",
)
s = once(s,
    '  python3 -B tests/timew/ci_assert_real_root_sample.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_root_sample.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_surface_generation.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "NATIVE_SURFACE_COMMIT_GENERATION_CHECKER",
)
# Write ONLY after every source anchor and ephemeral runner guard succeeds.
for file, value in ((out, o), (probe, p), (smoke, s)):
    file.write_text(value)
print("TIMEW_CI_XDG_GEN_OVERLAY_PASS: real wlroots surface.current.seq "
      "on actual SceneFX root SAMPLE and output COMMIT/PRESENT serial; "
      "surface generation may be superseded, no final-frame authority")
