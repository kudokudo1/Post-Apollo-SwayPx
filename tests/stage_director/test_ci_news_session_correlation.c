#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ci_news_session_correlation.h"

struct synthetic_processes {
	struct stage_director_process_identity root;
	struct stage_director_process_identity child;
	int reads;
	int corrupt_after;
};

static bool read_proc(int64_t pid, struct stage_director_process_identity *out,
		void *context) {
	struct synthetic_processes *p = context;
	if (!p || !out) return false;
	++p->reads;
	if (pid == p->root.pid) {
		*out = p->root;
	} else if (pid == p->child.pid) {
		*out = p->child;
	} else {
		return false;
	}
	if (p->corrupt_after > 0 && p->reads >= p->corrupt_after &&
			pid == p->child.pid) {
		out->start_time_ticks++;
	}
	return true;
}

static struct stage_director_rect host_rect(void) {
	return (struct stage_director_rect){0, 0, 900, 650};
}
static struct stage_director_rect output_rect(void) {
	return (struct stage_director_rect){0, 0, 1920, 1080};
}
static struct stage_director_rect bay_rect(void) {
	return (struct stage_director_rect){180, 160, 320, 240};
}
static struct stage_director_rect actor_rect(void) {
	return (struct stage_director_rect){100, 100, 900, 700};
}

static struct stage_director_weather_host_observation make_host(void) {
	return (struct stage_director_weather_host_observation) {
		.layer_namespace = STAGE_DIRECTOR_WEATHER_NAMESPACE,
		.layer_ref = 0x11,
		.output_ref = 0x33,
		.initialized = true,
		.mapped = true,
		.overlay_layer = true,
		.output_active = true,
		.host = host_rect(),
		.output = output_rect(),
	};
}

static struct stage_director_viewport_frame_observation make_frame(void) {
	return (struct stage_director_viewport_frame_observation) {
		.epoch = 7,
		.native_host_ref = 0x11,
		.native_actor_ref = 0x22,
		.native_output_ref = 0x33,
		.native_root_surface_ref = 0x44,
		.native_root_surface = true,
		.geometry = {
			.host = {0, 0, 900, 650},
			.output = {0, 0, 1920, 1080},
			.actor = {100, 100, 900, 700},
			.viewport = {180, 160, 320, 240},
			.host_mapped = true,
			.actor_mapped = true,
			.output_active = true,
			.actor_identity_verified = true,
		},
	};
}

static void check_non_authorizing(struct sd_ci_news_join_result r) {
	assert(!r.owner_verified && !r.can_launch && !r.can_bind_viewer);
	assert(!r.can_render && !r.can_accept_input && !r.can_show_popup);
}

static void check_denied(struct sd_ci_news_join_result r,
		enum sd_ci_news_join_state expected) {
	check_non_authorizing(r);
	assert(!r.candidate_correlated);
	assert(r.state == expected);
}

int main(void) {
	struct stage_director_viewer_session session;
	stage_director_viewer_session_init(&session);
	assert(stage_director_viewer_session_reserve(&session, 7, 0x11,
		4100, 1000, host_rect(), output_rect(), bay_rect()));
	/* The existing exact-PID attachment refuses an otherwise valid
	 * descendant process. Never fake the child PID as the root PID. */
	assert(!stage_director_viewer_session_attach(&session, 7, 0x11, 0x22,
		4101, 1020, actor_rect()));
	assert(session.state == SD_VIEWER_HOST_RESERVED);

	struct synthetic_processes processes = {
		.root = { .pid = 4100, .parent_pid = 100, .start_time_ticks = 1000,
			.uid = 1000 },
		.child = { .pid = 4101, .parent_pid = 4100, .start_time_ticks = 1020,
			.uid = 1000 },
	};
	struct stage_director_process_launch_root root = {
		.pid = 4100, .start_time_ticks = 1000, .uid = 1000,
	};
	struct stage_director_weather_host_observation host = make_host();
	struct stage_director_viewport_frame_observation frame = make_frame();
	struct sd_ci_news_join_observation o = {
		.session = &session,
		.host = &host,
		.frame = &frame,
		.launch_root = &root,
		.observed_wayland_client_pid = 4101,
		.observed_wayland_client_start = 1020,
		.read_process = read_proc,
		.read_process_context = &processes,
	};
	struct sd_ci_news_join_result r = sd_ci_news_correlate(&o);
	check_non_authorizing(r);
	assert(r.candidate_correlated);
	assert(r.state == SD_CI_NEWS_CORRELATED_CANDIDATE_ONLY);
	assert(processes.reads >= 4);

#define CHECK(expected) check_denied(sd_ci_news_correlate(&o), expected)
#define RESET do { host = make_host(); frame = make_frame(); \
	root.pid = 4100; root.start_time_ticks = 1000; root.uid = 1000; \
	processes.root = (struct stage_director_process_identity) \
		{4100, 100, 1000, 1000}; \
	processes.child = (struct stage_director_process_identity) \
		{4101, 4100, 1020, 1000}; \
	processes.reads = 0; processes.corrupt_after = 0; \
	o.session = &session; o.host = &host; o.frame = &frame; \
	o.launch_root = &root; o.observed_wayland_client_pid = 4101; \
	o.observed_wayland_client_start = 1020; \
	o.read_process = read_proc; o.read_process_context = &processes; \
	} while (0)

	check_denied(sd_ci_news_correlate(NULL), SD_CI_NEWS_NO_SESSION);
	RESET; o.session = NULL; CHECK(SD_CI_NEWS_NO_SESSION);
	RESET; o.host = NULL; CHECK(SD_CI_NEWS_NO_SESSION);
	RESET; o.frame = NULL; CHECK(SD_CI_NEWS_NO_SESSION);
	RESET; o.launch_root = NULL; CHECK(SD_CI_NEWS_NO_SESSION);
	RESET; o.read_process = NULL; CHECK(SD_CI_NEWS_NO_SESSION);

	RESET; host.layer_namespace = "foreign-layer"; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	RESET; host.initialized = false; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	RESET; host.mapped = false; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	RESET; host.overlay_layer = false; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	RESET; host.output_active = false; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	RESET; host.host.width = 0; CHECK(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);

	RESET; frame.epoch = 6; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.native_host_ref = 0x99; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; host.layer_ref = 0x99; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; host.output_ref = 0x99; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.native_actor_ref = 0; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.native_actor_ref = frame.native_host_ref; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.native_output_ref = 0; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.native_root_surface = false; CHECK(SD_CI_NEWS_INVALID_ROOT_SURFACE);
	RESET; frame.native_root_surface_ref = 0; CHECK(SD_CI_NEWS_INVALID_ROOT_SURFACE);
	RESET; frame.native_root_surface_ref = frame.native_actor_ref;
	CHECK(SD_CI_NEWS_INVALID_ROOT_SURFACE);
	RESET; frame.native_root_surface_ref = frame.native_host_ref;
	CHECK(SD_CI_NEWS_INVALID_ROOT_SURFACE);

	RESET; host.host.x += 2; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; host.output.width -= 2; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.host.y += 1; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.output.x += 1; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.viewport.height -= 1; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.actor.x = 901; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.actor_mapped = false; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.host_mapped = false; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.output_active = false; CHECK(SD_CI_NEWS_BAD_GEOMETRY);
	RESET; frame.geometry.actor_identity_verified = false; CHECK(SD_CI_NEWS_BAD_GEOMETRY);

	RESET; root.pid = 5100; CHECK(SD_CI_NEWS_ROOT_PROCESS_MISMATCH);
	RESET; root.start_time_ticks = 1; CHECK(SD_CI_NEWS_ROOT_PROCESS_MISMATCH);
	RESET; processes.root.start_time_ticks = 1; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; processes.child.parent_pid = 9000; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; processes.child.uid = 2000; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; processes.child.start_time_ticks = 700;
	CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; o.observed_wayland_client_pid = 6000; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; o.observed_wayland_client_start = 9999; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	RESET; processes.corrupt_after = 3; CHECK(SD_CI_NEWS_LINEAGE_NOT_STABLE);

	/* A root-PID actor can be candidate-matched through the preexisting
	 * exact-PID V2 session policy, but its native ref must still match. */
	RESET;
	assert(stage_director_viewer_session_attach(&session, 7, 0x11, 0x22,
		4100, 1000, actor_rect()));
	r = sd_ci_news_correlate(&o);
	assert(r.candidate_correlated);
	check_non_authorizing(r);
	RESET; frame.native_actor_ref = 0x55; CHECK(SD_CI_NEWS_STALE_LINK);
	RESET; frame.geometry.actor.width += 1; CHECK(SD_CI_NEWS_STALE_LINK);

	RESET;
	assert(stage_director_viewer_session_host_lost(&session, 0x11));
	CHECK(SD_CI_NEWS_NO_SESSION);
	assert(!stage_director_viewer_session_reserve(&session, 7, 0x11,
		4100, 1000, host_rect(), output_rect(), bay_rect()));
	assert(stage_director_viewer_session_reserve(&session, 8, 0x11,
		4100, 1000, host_rect(), output_rect(), bay_rect()));
	RESET; CHECK(SD_CI_NEWS_STALE_LINK);
	frame.epoch = 8;
	r = sd_ci_news_correlate(&o);
	assert(r.candidate_correlated); check_non_authorizing(r);
	assert(stage_director_viewer_session_close(&session, 8));
	check_denied(sd_ci_news_correlate(&o), SD_CI_NEWS_NO_SESSION);

	puts("SD_CI_NEWS_JOIN: candidate-only parent/child, host, surface, epoch, "
		"geometry, ancestry rereads, teardown and replay checks: PASS");
	return 0;
}
