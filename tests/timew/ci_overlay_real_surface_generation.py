#!/usr/bin/env python3
"""Ephemeral native XDG root committed-state generation witness.

Observe actual SceneFX client-buffer object equality at root sampling and
real wlroots wlr_surface.current.seq at exact SceneFX root sample,
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
actor = root / "tests/stage_director/ci_native_news_actor_smoke.sh"

def once(src, anchor, replacement, name):
    n = src.count(anchor)
    if n != 1:
        raise SystemExit(f"BLOCKED_XDG_GENERATION_SOURCE_DRIFT_{name} count={n}")
    return src.replace(anchor, replacement, 1)

o, p, s, m, a = [x.read_text() for x in (out, probe, smoke, meson, actor)]
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

/* Conservative sample witness, not an attachment generation. Any root
 * commit revokes the observation, even a damage-only commit. The destroy
 * signal prevents allocator address reuse from preserving an old witness. */
static struct wlr_buffer *ci_xdg_watch_buffer;
static struct wlr_client_buffer *ci_xdg_watch_client;
static struct wl_listener ci_xdg_watch_destroy;
static struct wl_listener ci_xdg_watch_commit;
static bool ci_xdg_root_commit_listening;
/* Exact root wl_surface requests, diagnostic only. No object IDs, resource
 * pointers, content, or protocol arguments except attach nullness logged.
 * A wire request is not an accepted surface commit or a buffer identity. */
static struct wl_protocol_logger *ci_xdg_wire_logger;
static uint64_t ci_xdg_wire_next;
static uint64_t ci_xdg_wire_pending;
static uint64_t ci_xdg_wire_commit_epoch;
static bool ci_xdg_wire_pending_nonnull;

static void ci_xdg_wire_request(void *unused,
		enum wl_protocol_logger_type direction,
		const struct wl_protocol_logger_message *message) {
	(void)unused;
	if (direction != WL_PROTOCOL_LOGGER_REQUEST || !message ||
			!message->message || !ci_view || !ci_view->surface ||
			!ci_view->surface->mapped ||
			message->resource != ci_view->surface->resource) {
		return;
	}
	const char *name = message->message->name;
	if (strcmp(name, "attach") == 0 && message->arguments_count == 3) {
		/* Wayland permits replacement of pending attachments. This bounded
		 * diagnostic deliberately declines that ambiguous sequence rather
		 * than silently discarding its earlier request lineage. CI only. */
		if (ci_xdg_wire_pending != 0) {
			fail("xdg_wire_overlapping_pending_attach");
		}
		if (ci_xdg_wire_next == UINT64_MAX) {
			fail("xdg_wire_attach_counter_overflow");
		}
		ci_xdg_wire_pending = ++ci_xdg_wire_next;
		ci_xdg_wire_pending_nonnull = message->arguments[0].o != NULL;
		fprintf(stderr, "TIMEW_CI_XDG_WIRE_ATTACH lifetime=%" PRIu64
			" request_epoch=%" PRIu64 " nonnull=%u authority=NONE\n",
			ci_lifetime.lifetime_sequence, ci_xdg_wire_pending,
			ci_xdg_wire_pending_nonnull ? 1u : 0u);
		fflush(stderr);
	} else if (strcmp(name, "commit") == 0 && message->arguments_count == 0) {
		/* A prior nonzero request epoch must have reached the observed
		 * root commit before we correlate another protocol commit. */
		if (ci_xdg_wire_commit_epoch != 0) {
			fail("xdg_wire_unresolved_commit_epoch");
		}
		ci_xdg_wire_commit_epoch = ci_xdg_wire_pending;
		fprintf(stderr, "TIMEW_CI_XDG_WIRE_COMMIT lifetime=%" PRIu64
			" request_epoch=%" PRIu64 " nonnull=%u authority=NONE\n",
			ci_lifetime.lifetime_sequence, ci_xdg_wire_pending,
			ci_xdg_wire_pending_nonnull ? 1u : 0u);
		fflush(stderr);
		ci_xdg_wire_pending = 0;
		ci_xdg_wire_pending_nonnull = false;
	}
}
static uint64_t ci_xdg_watch_next;
static uint64_t ci_xdg_watch_active;

static void ci_xdg_watch_revoke(const char *reason) {
	if (!ci_xdg_watch_active) {
		return;
	}
	fprintf(stderr, "TIMEW_CI_XDG_WATCH_REVOKE lifetime=%" PRIu64
		" epoch=%" PRIu64 " reason=%s authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_xdg_watch_active, reason);
	fflush(stderr);
	wl_list_remove(&ci_xdg_watch_destroy.link);
	ci_xdg_watch_buffer = NULL;
	ci_xdg_watch_client = NULL;
	ci_xdg_watch_active = 0;
}

static void ci_xdg_watch_buffer_destroyed(struct wl_listener *listener,
		void *data) {
	(void)listener;
	(void)data;
	ci_xdg_watch_revoke("buffer_destroy");
}

static void ci_xdg_watch_surface_committed(struct wl_listener *listener,
		void *data) {
	(void)listener;
	(void)data;
	uint64_t prior = ci_xdg_watch_active;
	ci_xdg_watch_revoke("surface_commit");
	/* Independent root event: even when destroy revoked the sample first,
	 * this callback observes the actual committed surface state. */
	if (ci_view && ci_view->surface && ci_view->surface->mapped) {
		fprintf(stderr, "TIMEW_CI_XDG_REDRAW_COMMIT lifetime=%" PRIu64
			" surface_state_seq=%" PRIu32 " prior_watch_epoch=%" PRIu64
			" wire_request_epoch=%" PRIu64 " authority=NONE\n",
			ci_lifetime.lifetime_sequence, ci_view->surface->current.seq,
			prior, ci_xdg_wire_commit_epoch);
		ci_xdg_wire_commit_epoch = 0;
		fflush(stderr);
	}
}

static uint64_t ci_xdg_watch_current(void) {
	if (!ci_xdg_watch_active ||
			ci_xdg_gen_sample_scene != ci_last_post_scene_sequence ||
			!ci_view || !ci_view->surface || !ci_view->surface->mapped ||
			!ci_sample_root_active || !ci_sample_root_buffer ||
			ci_sample_root_buffer->buffer != ci_xdg_watch_buffer ||
			ci_view->surface->buffer != ci_xdg_watch_client) {
		return 0;
	}
	return ci_xdg_watch_active;
}

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
	ci_xdg_watch_revoke("resample");
	ci_xdg_gen_sample_scene = ci_last_post_scene_sequence;
	ci_xdg_gen_sample_value = ci_view->surface->current.seq;
	if (!ci_xdg_root_commit_listening) {
		ci_xdg_watch_commit.notify = ci_xdg_watch_surface_committed;
		wl_signal_add(&ci_view->surface->events.commit,
			&ci_xdg_watch_commit);
		ci_xdg_root_commit_listening = true;
		struct wl_client *client =
			wl_resource_get_client(ci_view->surface->resource);
		ci_xdg_wire_logger = wl_display_add_protocol_logger(
			wl_client_get_display(client), ci_xdg_wire_request, NULL);
		if (!ci_xdg_wire_logger) {
			fail("xdg_wire_logger_failed");
		}
	}
	/* Compare live objects only during the callback. SceneFX uses this same
	 * client-buffer conversion for texture lookup. Watch only while live;
	 * never lock a buffer or assume a matching wrapper means immutable pixels. */
	struct wlr_client_buffer *sampled_client =
		wlr_client_buffer_get(ci_sample_root_buffer->buffer);
	bool client_object_match = sampled_client != NULL &&
		sampled_client == ci_view->surface->buffer;
	if (client_object_match) {
		if (ci_xdg_watch_next == UINT64_MAX) {
			fail("xdg_watch_epoch_overflow");
		}
		ci_xdg_watch_buffer = ci_sample_root_buffer->buffer;
		ci_xdg_watch_client = sampled_client;
		ci_xdg_watch_destroy.notify = ci_xdg_watch_buffer_destroyed;
		wl_signal_add(&ci_xdg_watch_buffer->events.destroy,
			&ci_xdg_watch_destroy);
		ci_xdg_watch_active = ++ci_xdg_watch_next;
	}
	fprintf(stderr,
		"TIMEW_CI_XDG_GEN_SAMPLE lifetime=%" PRIu64
		" sequence=%" PRIu64 " surface_state_seq=%" PRIu32
		" root_client_object_match=%u watch_epoch=%" PRIu64
		" exact_native_root=1 real_scenefx_sample=1"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		ci_xdg_gen_sample_value, client_object_match ? 1u : 0u,
		ci_xdg_watch_active);
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
		" current_surface_seq=%" PRIu32 " watch_epoch=%" PRIu64
		" accepted_output_commit=1"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		commit_seq, sample_in_scene ? 1u : 0u, sampled,
		ci_view->surface->current.seq, ci_xdg_watch_current());
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
		" current_surface_seq=%" PRIu32 " watch_epoch=%" PRIu64
		" real_output_present=1"
		" sampled_buffer_in_presented_frame=UNVERIFIED"
		" immutable_buffer=UNVERIFIED final_frame=UNVERIFIED"
		" ack=UNVERIFIED source=UNENROLLED c5_spent=NONE"
		" receipt=NONE move=OFF authority=NONE\n",
		ci_lifetime.lifetime_sequence, ci_last_post_scene_sequence,
		commit_seq, ci_view->surface->current.seq, ci_xdg_watch_current());
	fflush(stderr);
}

static void ci_xdg_gen_seal(void) {
	ci_xdg_watch_revoke("unmap");
	if (ci_xdg_wire_logger) {
		wl_protocol_logger_destroy(ci_xdg_wire_logger);
		ci_xdg_wire_logger = NULL;
	}
	ci_xdg_wire_pending = 0;
	ci_xdg_wire_pending_nonnull = false;
	ci_xdg_wire_commit_epoch = 0;
	if (ci_xdg_root_commit_listening) {
		wl_list_remove(&ci_xdg_watch_commit.link);
		ci_xdg_root_commit_listening = false;
	}
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
    "#include <wlr/types/wlr_compositor.h>",
    "#include <wlr/types/wlr_compositor.h>\n#include <wlr/types/wlr_buffer.h>",
    "WLR_BUFFER_PUBLIC_HEADER",
)
p = once(p,
    "static void ci_sample_root_detach(void) {",
    source + "\nstatic void ci_sample_root_detach(void) {\n"
    '\tci_xdg_watch_revoke("scene_detach");',
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
a = once(a,
    '"$foot_binary" \\\n  --app-id=post-apollo-weather-news',
    '''if [[ "${TIMEW_CI_CONTROLLED_REDRAW:-0}" == 1 ]]; then
  rm -f -- "$runtime/timew-ci-redraw.go"
  ci_actor_command=(/bin/bash -c 'for ((i=0;i<300;i++)); do if [[ -f "$1" ]]; then printf "TIMEW CONTROLLED REDRAW 1234567890\\n"; sleep 30; exit 0; fi; sleep 0.05; done; exit 1' _ "$runtime/timew-ci-redraw.go")
else
  ci_actor_command=(/usr/bin/sleep 30)
fi
"$foot_binary" \\
  --app-id=post-apollo-weather-news''',
    "CONTROLLED_REDRAW_ACTOR",
)
a = once(a,
    '  /usr/bin/sleep 30 >"$actor_log" 2>&1 &',
    '  "${ci_actor_command[@]}" >"$actor_log" 2>&1 &',
    "CONTROLLED_REDRAW_COMMAND",
)
a = once(a,
    '    sleep 0.55\n  fi',
    '''    sleep 0.55
    if [[ "${TIMEW_CI_CONTROLLED_REDRAW:-0}" == 1 ]]; then
      checkpoint=""
      for _ in {1..50}; do
        if checkpoint="$(python3 -B tests/timew/ci_assert_real_root_redraw.py ready "$runtime/sway.log")"; then break; fi
        sleep 0.1
      done
      test -n "$checkpoint" || { echo "TIMEW_CI_REDRAW_FAIL no live sampled watch before trigger" >&2; exit 1; }
      touch "$runtime/timew-ci-redraw.go"
      redraw_ok=0
      for _ in {1..50}; do
        if python3 -B tests/timew/ci_assert_real_root_redraw.py after "$runtime/sway.log" "$checkpoint"; then
          redraw_ok=1
          break
        fi
        sleep 0.1
      done
      [[ "$redraw_ok" == 1 ]] || { echo "TIMEW_CI_REDRAW_FAIL no observed commit and revocation after trigger" >&2; exit 1; }
    fi
  fi''',
    "CONTROLLED_REDRAW_TRIGGER",
)
# Write ONLY after every source anchor and ephemeral runner guard succeeds.
for file, value in ((out, o), (probe, p), (smoke, s), (actor, a)):
    file.write_text(value)
print("TIMEW_CI_XDG_GEN_OVERLAY_PASS: real wlroots surface.current.seq "
      "on actual SceneFX root SAMPLE and output COMMIT/PRESENT serial; "
      "surface generation may be superseded, no final-frame authority")
