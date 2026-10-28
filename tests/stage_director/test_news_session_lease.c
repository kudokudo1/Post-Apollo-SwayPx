#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_news_session_lease.h"

struct fake_proc {
	struct stage_director_process_identity root, child;
	int calls;
	int disrupt_after;
	bool deny_child;
};
static bool read_proc(int64_t pid,
		struct stage_director_process_identity *out, void *context) {
	struct fake_proc *f = context;
	if (!f || !out) return false;
	++f->calls;
	if (pid == f->root.pid) *out = f->root;
	else if (pid == f->child.pid && !f->deny_child) *out = f->child;
	else return false;
	if (f->disrupt_after && f->calls >= f->disrupt_after &&
			pid == f->child.pid) {
		out->start_time_ticks++;
	}
	return true;
}
static struct fake_proc fixture(void) {
	return (struct fake_proc) {
		.root = { .pid = 4000, .parent_pid = 100, .start_time_ticks = 600,
			.uid = 1000 },
		.child = { .pid = 4001, .parent_pid = 4000, .start_time_ticks = 620,
			.uid = 1000 },
	};
}
static struct stage_director_news_lease_observation native(void) {
	return (struct stage_director_news_lease_observation) {
		.epoch = 7,
		.native_host_ref = 0x10,
		.native_host_client_ref = 0x11,
		.native_output_ref = 0x12,
		.native_actor_ref = 0x20,
		.native_actor_client_ref = 0x21,
		.native_root_surface_ref = 0x22,
		.native_wayland_client_pid = 4001,
		.native_wayland_client_start = 620,
		.exact_xdg_toplevel_root = true,
	};
}
static struct stage_director_process_launch_root root(void) {
	return (struct stage_director_process_launch_root) {
		.pid = 4000, .start_time_ticks = 600, .uid = 1000,
	};
}
static void never_authorized(struct stage_director_news_lease_probe p) {
	assert(!p.operator_watch_authenticated);
	assert(!p.browser_launch_authorized);
	assert(!p.owner_verified);
	assert(!p.can_launch);
	assert(!p.can_bind_viewer);
	assert(!p.can_clip);
	assert(!p.can_render);
	assert(!p.can_accept_input);
	assert(!p.can_show_popup);
}
static void denied(struct stage_director_news_lease_probe p,
		enum stage_director_news_lease_probe_state state) {
	never_authorized(p);
	assert(!p.candidate_correlated);
	assert(p.state == state);
}

int main(void) {
	struct stage_director_news_session_lease s;
	stage_director_news_session_lease_init(&s);
	const struct stage_director_process_launch_root launch = root();
	struct fake_proc f = fixture();
	struct stage_director_news_lease_observation o = native();

	denied(stage_director_news_session_lease_probe(
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_NO_SESSION);
	assert(!stage_director_news_session_lease_reserve_candidate(
		NULL, 7, 0x10, 0x11, 0x12, "bbc"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 0, 0x10, 0x11, 0x12, "bbc"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0, 0x11, 0x12, "bbc"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0, 0x12, "bbc"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0x11, 0, "bbc"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0x11, 0x12, "unknown-source"));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0x11, 0x12, "https://example.com"));
	assert(stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0x11, 0x12, "bbc"));
	assert(s.state == SD_NEWS_LEASE_HOST_CANDIDATE);
	assert(s.last_epoch == 7 && s.epoch == 7);
	assert(strcmp(s.source_id, "bbc") == 0);
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 8, 0x10, 0x11, 0x12, "bbc"));
	denied(stage_director_news_session_lease_probe(
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_NOT_BOUND);

	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 6, &launch));
	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 7, NULL));
	struct stage_director_process_launch_root bad = launch;
	bad.pid = 0;
	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 7, &bad));
	bad = launch; bad.start_time_ticks = 0;
	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 7, &bad));
	bad = launch; bad.uid = 0;
	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 7, &bad));
	assert(stage_director_news_session_lease_record_process_candidate(
		&s, 7, &launch));
	assert(!stage_director_news_session_lease_record_process_candidate(
		&s, 7, &launch));

#define CHECK_BAD_OBSERVATION(change) do { \
	o = native(); change; \
	assert(!stage_director_news_session_lease_record_actor_candidate( \
		&s, &o, read_proc, &f)); \
	assert(s.state == SD_NEWS_LEASE_PROCESS_CANDIDATE); \
} while (0)

	CHECK_BAD_OBSERVATION(o.epoch = 8);
	CHECK_BAD_OBSERVATION(o.native_host_ref = 0x80);
	CHECK_BAD_OBSERVATION(o.native_host_client_ref = 0x81);
	CHECK_BAD_OBSERVATION(o.native_output_ref = 0x82);
	CHECK_BAD_OBSERVATION(o.native_actor_ref = 0);
	CHECK_BAD_OBSERVATION(o.native_actor_ref = o.native_host_ref);
	CHECK_BAD_OBSERVATION(o.native_actor_client_ref = 0);
	CHECK_BAD_OBSERVATION(o.native_root_surface_ref = 0);
	CHECK_BAD_OBSERVATION(o.native_root_surface_ref = o.native_actor_ref);
	CHECK_BAD_OBSERVATION(o.native_root_surface_ref = o.native_host_ref);
	CHECK_BAD_OBSERVATION(o.exact_xdg_toplevel_root = false);
	CHECK_BAD_OBSERVATION(o.native_wayland_client_pid = 0);
	CHECK_BAD_OBSERVATION(o.native_wayland_client_pid = 9000);
	CHECK_BAD_OBSERVATION(o.native_wayland_client_start = 0);
	CHECK_BAD_OBSERVATION(o.native_wayland_client_start = 999);
	o = native();
	f.deny_child = true;
	assert(!stage_director_news_session_lease_record_actor_candidate(
		&s, &o, read_proc, &f));
	f.deny_child = false;
	f.child.uid = 1001;
	assert(!stage_director_news_session_lease_record_actor_candidate(
		&s, &o, read_proc, &f));
	f.child.uid = 1000;
	f.child.parent_pid = 9200;
	assert(!stage_director_news_session_lease_record_actor_candidate(
		&s, &o, read_proc, &f));
	f.child.parent_pid = 4000;
	f.disrupt_after = 2;
	f.calls = 0;
	assert(!stage_director_news_session_lease_record_actor_candidate(
		&s, &o, read_proc, &f));
	f.disrupt_after = 0; f.calls = 0;
	assert(stage_director_news_session_lease_record_actor_candidate(
		&s, &o, read_proc, &f));
	assert(f.calls >= 4);
	assert(s.state == SD_NEWS_LEASE_ACTOR_CANDIDATE);
	struct stage_director_news_lease_probe result =
		stage_director_news_session_lease_probe(&s, &o, read_proc, &f);
	never_authorized(result);
	assert(result.candidate_correlated);
	assert(result.state == SD_NEWS_LEASE_PROBE_MATCHED_CANDIDATE_ONLY);

#define CHECK_DENIED(change) do { \
	o = native(); change; \
	denied(stage_director_news_session_lease_probe( \
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_MISMATCH); \
} while (0)
	CHECK_DENIED(o.epoch = 6);
	CHECK_DENIED(o.native_host_ref = 0x99);
	CHECK_DENIED(o.native_host_client_ref = 0x99);
	CHECK_DENIED(o.native_output_ref = 0x99);
	CHECK_DENIED(o.native_actor_ref = 0x99);
	CHECK_DENIED(o.native_actor_client_ref = 0x99);
	CHECK_DENIED(o.native_root_surface_ref = 0x99);
	CHECK_DENIED(o.native_wayland_client_pid = 2000);
	CHECK_DENIED(o.native_wayland_client_start = 621);
	CHECK_DENIED(o.exact_xdg_toplevel_root = false);
	o = native();
	f.child.start_time_ticks = 1;
	denied(stage_director_news_session_lease_probe(
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_PROCESS_UNSTABLE);
	f.child.start_time_ticks = 620;
	f.deny_child = true;
	denied(stage_director_news_session_lease_probe(
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_PROCESS_UNSTABLE);
	f.deny_child = false;

	/* The identity graph is candidate-only even after success. */
	assert(!stage_director_news_session_lease_close(&s, 6));
	assert(!stage_director_news_session_lease_host_lost(&s, 0x99));
	assert(!stage_director_news_session_lease_output_lost(&s, 0x99));
	assert(!stage_director_news_session_lease_actor_lost(&s, 0x99));
	assert(!stage_director_news_session_lease_process_lost(&s, 9999));
	assert(stage_director_news_session_lease_actor_lost(&s, 0x20));
	denied(stage_director_news_session_lease_probe(
		&s, &o, read_proc, &f), SD_NEWS_LEASE_PROBE_NO_SESSION);
	assert(!stage_director_news_session_lease_close(&s, 7));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 7, 0x10, 0x11, 0x12, "bbc"));
	assert(stage_director_news_session_lease_reserve_candidate(
		&s, 8, 0x10, 0x11, 0x12, "nasa"));
	assert(!stage_director_news_session_lease_close(&s, 7));
	assert(stage_director_news_session_lease_host_lost(&s, 0x10));
	assert(s.last_epoch == 8);
	assert(stage_director_news_session_lease_reserve_candidate(
		&s, 9, 0x10, 0x11, 0x12, "bbc"));
	assert(stage_director_news_session_lease_output_lost(&s, 0x12));
	assert(stage_director_news_session_lease_reserve_candidate(
		&s, 10, 0x10, 0x11, 0x12, "bbc"));
	assert(stage_director_news_session_lease_record_process_candidate(
		&s, 10, &launch));
	assert(stage_director_news_session_lease_process_lost(&s, 4000));
	assert(stage_director_news_session_lease_reserve_candidate(
		&s, 11, 0x10, 0x11, 0x12, "bbc"));
	assert(stage_director_news_session_lease_close(&s, 11));
	assert(!stage_director_news_session_lease_reserve_candidate(
		&s, 10, 0x10, 0x11, 0x12, "bbc"));
	puts("SD_NEWS_LEASE: source allowlist, single host/epoch, "
		"distinct native clients/root surface, fresh live lineage, "
		"actor/host/output/process teardown and replay gates PASS; "
		"operator/launch/render/input authority NONE");
	return 0;
}
