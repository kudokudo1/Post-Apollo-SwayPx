#!/usr/bin/env python3
"""CI-only exact XDG root SceneFX output_sample observation."""
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

def one(src, old, new, label):
    count = src.count(old)
    if count != 1:
        raise SystemExit(f"BLOCKED_ROOT_SAMPLE_SOURCE_DRIFT_{label} count={count}")
    return src.replace(old, new, 1)

o, p, s, m = (f.read_text() for f in (out, probe, smoke, meson))
if "TIMEW_CI_ROOT_SAMPLE" in o or "TIMEW_CI_ROOT_SAMPLE" in p:
    raise SystemExit("BLOCKED_ROOT_SAMPLE_ALREADY_INSTRUMENTED")
if ("timew_scope_a_ci_output_commit_serial(output" not in o
        or "timew_scope_a_ci_output_present_serial(output" not in o
        or "static bool ci_output_seq_same_candidate(" not in p
        or "static void ci_output_seq_seal(void)" not in p
        or "'timew_scope_a_native_txn_observer.c'" not in m):
    raise SystemExit("BLOCKED_PARENT_OUTPUT_SERIAL_OBSERVER_MISSING")

o = one(o,
    "extern void timew_scope_a_ci_output_present_serial(struct sway_output *output,\n"
    "\t\tuint32_t event_commit_seq);",
    "extern void timew_scope_a_ci_output_present_serial(struct sway_output *output,\n"
    "\t\tuint32_t event_commit_seq);\n"
    "extern void timew_scope_a_ci_root_sample_prebuild(struct sway_output *output);",
    "REAL_ROOT_SAMPLE_DECL",
)
o = one(o,
    "\twlr_output_state_init(&pending);\n"
    "\tif (!wlr_scene_output_build_state(output->scene_output, &pending, &opts)) {",
    "\twlr_output_state_init(&pending);\n"
    "\t/* Disposable CI only, attach before the real renderer samples. */\n"
    "\ttimew_scope_a_ci_root_sample_prebuild(output);\n"
    "\tif (!wlr_scene_output_build_state(output->scene_output, &pending, &opts)) {",
    "PRE_REAL_RENDER_SUBSCRIBE",
)

source = r"""
/* SceneFX emits output_sample when it adds this buffer to a render pass,
 * or when it prepares direct scanout. Neither means final presentation.
 * These listeners are explicitly revoked before unmap or buffer destruction. */
static struct wlr_scene_buffer *ci_sample_root_buffer;
static struct wl_listener ci_sample_root_listener;
static struct wl_listener ci_sample_root_destroy_listener;
static bool ci_sample_root_active;
static uint64_t ci_sample_last_scene;
static uint64_t ci_sample_count;
static uint64_t ci_sample_subscriptions;

static void ci_sample_root_detach(void) {
	if (ci_sample_root_active) {
		wl_list_remove(&ci_sample_root_listener.link);
		wl_list_remove(&ci_sample_root_destroy_listener.link);
		ci_sample_root_active = false;
		ci_sample_root_buffer = NULL;
	}
}

static void ci_sample_root_destroy(struct wl_listener *listener, void *data) {
	(void)listener;
	(void)data;
	if (!ci_sample_root_active) {
		return;
	}
	fprintf(stderr,
		"TIMEW_CI_ROOT_SAMPLE_REVOKED lifetime=%" PRIu64
		" root_buffer_destroyed=1 no_stale_reuse=1 authority=NONE\n",
		ci_lifetime.lifetime_sequence);
	fflush(stderr);
	ci_sample_root_detach();
}

static void ci_sample_root_notify(struct wl_listener *listener, void *data) {
	(void)listener;
	const struct wlr_scene_output_sample_event *event = data;
	if (!ci_sample_root_active || !ci_sample_root_buffer ||
			!event || !event->output || !event->output->output ||
			!ci_last_post_scene_sequence ||
			ci_sample_last_scene == ci_last_post_scene_sequence ||
			!ci_view || !ci_view->surface ||
			!ci_view->surface->mapped || !ci_sample_root_buffer->buffer) {
		return;
	}
	struct sway_output *output = event->output->output->data;
	struct wlr_scene_surface *ss =
		wlr_scene_surface_try_from_buffer(ci_sample_root_buffer);
	if (!ss || ss->surface != ci_view->surface ||
			!ci_output_seq_same_candidate(output) ||
			event->output != output->scene_output ||
			ci_sample_root_buffer->primary_output != output->scene_output) {
		fprintf(stderr,
			"TIMEW_CI_ROOT_SAMPLE_DRIFT source_or_output=CHANGED"
			" authority=NONE\n");
		fflush(stderr);
		return;
	}
	if (ci_sample_count == UINT64_MAX) {
		fail("root_sample_counter_overflow");
	}
	++ci_sample_count;
	ci_sample_last_scene = ci_last_post_scene_sequence;
	fprintf(stderr,
		"TIMEW_CI_ROOT_SAMPLE lifetime=%" PRIu64
		" sequence=%" PRIu64 " direct_scanout=%u"
		" exact_root_surface=1 sampled_scene_buffer=1"
		" real_scenefx_event=1 same_primary_output=1"
		" render_pass_or_scanout=1"
		" accepted_output_commit=UNVERIFIED"
		" selected_xdg_final_presented=UNVERIFIED"
		" configure_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF retry=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		event->direct_scanout ? 1u : 0u);
	fflush(stderr);
}

static void ci_sample_root_consider(struct wlr_scene_buffer *buffer,
		int x, int y, void *data) {
	(void)x;
	(void)y;
	struct sway_output *output = data;
	if (!buffer || !output || !ci_view || !ci_view->surface ||
			buffer->primary_output != output->scene_output) {
		return;
	}
	struct wlr_scene_surface *ss = wlr_scene_surface_try_from_buffer(buffer);
	if (!ss || ss->surface != ci_view->surface) {
		return; /* Reject foreign views, XDG popups, and subsurfaces. */
	}
	if (ci_sample_root_active && ci_sample_root_buffer == buffer) {
		return;
	}
	ci_sample_root_detach();
	ci_sample_root_listener.notify = ci_sample_root_notify;
	ci_sample_root_destroy_listener.notify = ci_sample_root_destroy;
	wl_signal_add(&buffer->events.output_sample, &ci_sample_root_listener);
	wl_signal_add(&buffer->node.events.destroy,
		&ci_sample_root_destroy_listener);
	ci_sample_root_active = true;
	ci_sample_root_buffer = buffer;
	if (ci_sample_subscriptions == UINT64_MAX) {
		fail("root_sample_subscribe_overflow");
	}
	++ci_sample_subscriptions;
	fprintf(stderr,
		"TIMEW_CI_ROOT_SAMPLE_BIND lifetime=%" PRIu64
		" exact_native_xdg_root=1 primary_output=1"
		" receipt=NONE authority=NONE\n",
		ci_lifetime.lifetime_sequence);
	fflush(stderr);
}

void timew_scope_a_ci_root_sample_prebuild(struct sway_output *output) {
	if (!ci_output_seq_same_candidate(output) || !output->scene_output) {
		return;
	}
	wlr_scene_output_for_each_buffer(output->scene_output,
		ci_sample_root_consider, output);
}

static void ci_sample_root_seal(void) {
	ci_sample_root_detach();
	fprintf(stderr,
		"TIMEW_CI_ROOT_SAMPLE_SEAL lifetime=%" PRIu64
		" root_samples=%" PRIu64
		" subscriptions=%" PRIu64
		" real_unmap=1 settled=0 authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_sample_count,
		ci_sample_subscriptions);
	fflush(stderr);
	ci_sample_last_scene = 0;
	ci_sample_count = 0;
	ci_sample_subscriptions = 0;
}
"""
p = one(p,
    "void timew_scope_a_ci_output_present(struct sway_output *output) {",
    source + "\nvoid timew_scope_a_ci_output_present(struct sway_output *output) {",
    "ROOT_SAMPLE_WITNESS_BODY",
)
p = one(p,
    "static void ci_output_present_seal(void) {\n",
    "static void ci_output_present_seal(void) {\n"
    "\tci_sample_root_seal();\n",
    "ROOT_SAMPLE_REVOKE_BEFORE_UNMAP",
)
s = one(s,
    '  python3 -B tests/timew/ci_assert_real_output_commit_serial.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    '  python3 -B tests/timew/ci_assert_real_output_commit_serial.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"\n'
    '  python3 -B tests/timew/ci_assert_real_root_sample.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "ROOT_SAMPLE_NATIVE_CHECKER",
)
for f, value in ((out, o), (probe, p), (smoke, s)):
    f.write_text(value)
print("TIMEW_CI_ROOT_SAMPLE_OVERLAY_PASS: true pinned SceneFX root "
      "output_sample listener, revoked before unmap; final frame NONE")
