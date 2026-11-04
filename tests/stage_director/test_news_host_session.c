#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_news_host_session.h"
#include "sway/stage_director_news_viewport_candidate.h"

static void no_authority(struct stage_director_news_host_session_probe p) {
	assert(!p.operator_watch_authenticated);
	assert(!p.can_reserve_lease);
	assert(!p.can_launch);
	assert(!p.can_bind_viewer);
	assert(!p.can_clip);
	assert(!p.can_render);
	assert(!p.can_accept_input);
	assert(!p.can_show_popup);
}
static void denied(struct stage_director_news_host_session_probe p,
		enum stage_director_news_host_session_state expected) {
	no_authority(p);
	assert(!p.native_candidate_matches);
	assert(p.state == expected);
}
static struct wl_client *new_client(struct wl_display *display, int *fd) {
	int pair[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
	struct wl_client *client = wl_client_create(display, pair[0]);
	assert(client != NULL);
	*fd = pair[1];
	return client;
}
static struct wl_resource *new_resource(struct wl_client *client) {
	struct wl_resource *r =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(r != NULL);
	return r;
}
static struct stage_director_news_host_session_probe probe(
		struct stage_director_news_host_session *s,
		struct wl_resource *r, uintptr_t host, uintptr_t output,
		uint64_t epoch) {
	return stage_director_news_host_session_probe_request(
		s, r, host, output, epoch);
}

int main(void) {
	struct stage_director_news_host_session state = {0};
	stage_director_news_host_session_init(&state);
	stage_director_news_host_session_init(&state); /* idempotent */
	struct wl_display *display = wl_display_create();
	assert(display != NULL);
	int a_fd = -1, b_fd = -1;
	struct wl_client *a = new_client(display, &a_fd);
	struct wl_client *b = new_client(display, &b_fd);
	struct wl_resource *role_a = new_resource(a);
	struct wl_resource *surface_a = new_resource(a);
	struct wl_resource *watch_a = new_resource(a);
	struct wl_resource *role_b = new_resource(b);
	struct wl_resource *watch_b = new_resource(b);

	denied(probe(&state, watch_a, 0x10, 0x20, 1),
		SD_NEWS_HOST_SESSION_EMPTY);
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, role_b, 0x10, 0x20, 1));
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, role_a, 0x10, 0x20, 1));
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0, 0x20, 1));
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0, 1));
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 0));

	assert(stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 1));
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 2));
	assert(stage_director_news_host_session_tracks(&state, 0x10));
	assert(!stage_director_news_host_session_tracks(&state, 0x50));
	const struct stage_director_news_host_session_probe match =
		probe(&state, watch_a, 0x10, 0x20, 1);
	no_authority(match);
	assert(match.native_candidate_matches);
	assert(match.state == SD_NEWS_HOST_SESSION_NATIVE_CANDIDATE_ONLY);
	denied(probe(&state, watch_b, 0x10, 0x20, 1),
		SD_NEWS_HOST_SESSION_FOREIGN_REQUEST);
	denied(probe(&state, NULL, 0x10, 0x20, 1),
		SD_NEWS_HOST_SESSION_INVALID);
	denied(probe(&state, watch_a, 0x10, 0x21, 1),
		SD_NEWS_HOST_SESSION_STALE);
	denied(probe(&state, watch_a, 0x99, 0x20, 1),
		SD_NEWS_HOST_SESSION_STALE);
	denied(probe(&state, watch_a, 0x10, 0x20, 2),
		SD_NEWS_HOST_SESSION_STALE);

	/* Paired host starts without geometry or browser permission. */
	struct stage_director_news_host_geometry_snapshot empty =
		stage_director_news_host_session_geometry_snapshot(
			&state, 0x10, (uintptr_t)a, 0x20, 1);
	assert(!empty.candidate_only && empty.revision == 0);
	assert(!stage_director_news_host_session_geometry_current(&state, &empty));
	const struct stage_director_rect host1 = {100, 50, 600, 400};
	const struct stage_director_rect host2 = {150, 70, 620, 420};
	const struct stage_director_rect output = {0, 0, 1920, 1080};
	assert(!stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)b, 0x20, 1, host1, output));
	assert(!stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x21, 1, host1, output));
	assert(!stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 2, host1, output));
	assert(stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 1, host1, output));
	struct stage_director_news_host_geometry_snapshot first =
		stage_director_news_host_session_geometry_snapshot(
			&state, 0x10, (uintptr_t)a, 0x20, 1);
	assert(first.candidate_only && first.revision == 1);
	assert(!first.can_launch && !first.can_clip &&
		!first.can_accept_input && !first.can_show_popup);
	assert(stage_director_news_host_session_geometry_current(&state, &first));
	assert(stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 1, host1, output));
	assert(state.geometry_revision == 1);
	assert(stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 1, host2, output));
	assert(state.geometry_revision == 2);
	assert(!stage_director_news_host_session_geometry_current(&state, &first));
	struct stage_director_news_host_geometry_snapshot second =
		stage_director_news_host_session_geometry_snapshot(
			&state, 0x10, (uintptr_t)a, 0x20, 1);
	assert(second.revision == 2 && second.host.x == 150);
	assert(stage_director_news_host_session_geometry_current(&state, &second));
	second.can_clip = true;
	assert(!stage_director_news_host_session_geometry_current(&state, &second));
	second.can_clip = false;
	/* WATCH v2 QML local rectangles are only read-only client geometry
	 * reports, rebased under the actual native host epoch and revision. */
	const struct stage_director_rect viewport_local = {20, 30, 320, 240};
	struct stage_director_news_host_viewport_receipt reported =
		stage_director_news_host_session_report_viewport(
			&state, 0x10, (uintptr_t)a, 0x20, 1, viewport_local);
	assert(reported.status == SD_NEWS_HOST_VIEWPORT_CANDIDATE_ONLY);
	assert(reported.epoch == 1 && reported.host_geometry_revision == 2);
	assert(reported.inner_global.x == 170 && reported.inner_global.y == 100);
	assert(reported.inner_global.width == 320 &&
		reported.inner_global.height == 240);
	assert(!reported.can_launch && !reported.can_clip &&
		!reported.can_render && !reported.can_accept_input &&
		!reported.can_show_popup);
	assert(stage_director_news_host_session_viewport_current(&state, &reported));
	struct stage_director_news_host_viewport_receipt hostile =
		stage_director_news_host_session_report_viewport(
			&state, 0x10, (uintptr_t)b, 0x20, 1, viewport_local);
	assert(hostile.status == SD_NEWS_HOST_VIEWPORT_DENIED);
	assert(stage_director_news_host_session_viewport_current(&state, &reported));
	hostile = stage_director_news_host_session_report_viewport(
		&state, 0x10, (uintptr_t)a, 0x21, 1, viewport_local);
	assert(hostile.status == SD_NEWS_HOST_VIEWPORT_DENIED);
	hostile = stage_director_news_host_session_report_viewport(
		&state, 0x10, (uintptr_t)a, 0x20, 2, viewport_local);
	assert(hostile.status == SD_NEWS_HOST_VIEWPORT_DENIED);
	assert(stage_director_news_host_session_viewport_current(&state, &reported));
	/* A malformed message FROM the paired client must revoke its old area. */
	hostile = stage_director_news_host_session_report_viewport(
		&state, 0x10, (uintptr_t)a, 0x20, 1,
		(struct stage_director_rect){-1, 30, 320, 240});
	assert(hostile.status == SD_NEWS_HOST_VIEWPORT_DENIED);
	assert(!stage_director_news_host_session_viewport_current(&state, &reported));
	assert(!state.viewport_candidate_valid);
	hostile = stage_director_news_host_session_report_viewport(
		&state, 0x10, (uintptr_t)a, 0x20, 1,
		(struct stage_director_rect){0, 0, 0, 0});
	assert(hostile.status == SD_NEWS_HOST_VIEWPORT_HIDDEN);
	assert(hostile.host_geometry_revision == 2);
	assert(!state.viewport_candidate_valid);
	reported = stage_director_news_host_session_report_viewport(
		&state, 0x10, (uintptr_t)a, 0x20, 1, viewport_local);
	assert(stage_director_news_host_session_viewport_current(&state, &reported));
	reported.can_clip = true; /* even a forged receipt cannot grant authority */
	assert(!stage_director_news_host_session_viewport_current(&state, &reported));
	reported.can_clip = false;
	assert(stage_director_news_host_session_viewport_current(&state, &reported));
	/* Candidate-only inner NEWS area: geometry must match the current
	 * exact native host revision and fit inside both host and actor. */
	struct stage_director_news_viewport_candidate_request inner = {
		.epoch = 1,
		.host_geometry_revision = second.revision,
		.native_host_ref = 0x10,
		.native_host_client_ref = (uintptr_t)a,
		.native_output_ref = 0x20,
		.native_inner_surface_ref = 0x300,
		.inner_native_observation = true,
		.host_mapped = true,
		.actor_mapped = true,
		.output_active = true,
		.actor_identity_verified = true,
		.inner_global = {200, 100, 320, 240},
		.actor_global = {100, 50, 850, 700},
	};
	struct stage_director_news_viewport_candidate_result candidate =
		stage_director_news_viewport_candidate_probe(&state, &inner);
	assert(candidate.status == SD_NEWS_VIEWPORT_GEOMETRY_CANDIDATE_ONLY);
	assert(candidate.geometry_candidate && candidate.actor_local_crop.x == 100);
	assert(candidate.actor_local_crop.y == 50);
	assert(candidate.actor_local_crop.width == 320 &&
		candidate.actor_local_crop.height == 240);
	assert(!candidate.can_launch && !candidate.can_render &&
		!candidate.can_clip && !candidate.can_accept_input &&
		!candidate.can_show_popup);
	inner.host_geometry_revision = first.revision;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_STALE_HOST);
	inner.host_geometry_revision = second.revision;
	inner.native_host_client_ref = (uintptr_t)b;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_NO_HOST);
	inner.native_host_client_ref = (uintptr_t)a;
	inner.inner_native_observation = false;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_INNER_UNOBSERVED);
	inner.inner_native_observation = true;
	inner.native_inner_surface_ref = 0x10;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_INNER_UNOBSERVED);
	inner.native_inner_surface_ref = 0x300;
	inner.inner_global.x = 800;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_INVALID_GEOMETRY);
	inner.inner_global.x = 200;
	inner.actor_global.width = 50;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_INVALID_GEOMETRY);
	inner.actor_global.width = 850;
	inner.actor_identity_verified = false;
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_INVALID_GEOMETRY);
	inner.actor_identity_verified = true;
	assert(!stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 1,
		(struct stage_director_rect){1800, 0, 600, 400}, output));
	assert(state.geometry_revision == 2);
	assert(!stage_director_news_host_session_geometry_snapshot(
		&state, 0x10, (uintptr_t)b, 0x20, 1).candidate_only);
	assert(!stage_director_news_host_session_geometry_snapshot(
		&state, 0x10, (uintptr_t)a, 0x20, 2).candidate_only);
	stage_director_news_host_session_host_lost(&state, 0x99);
	assert(state.candidate); /* wrong host cannot revoke */
	stage_director_news_host_session_host_lost(&state, 0x10);
	assert(!state.candidate);
	assert(!state.viewport_candidate_valid);
	assert(!stage_director_news_host_session_viewport_current(&state, &reported));
	assert(!stage_director_news_host_session_geometry_current(&state, &second));
	assert(state.geometry_revision == 0 && state.last_geometry_revision == 2);
	assert(stage_director_news_viewport_candidate_probe(&state, &inner).status ==
		SD_NEWS_VIEWPORT_NO_HOST);
	denied(probe(&state, watch_a, 0x10, 0x20, 1),
		SD_NEWS_HOST_SESSION_EMPTY);
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 1));

	/* wl_resource destruction automatically invalidates the native candidate. */
	assert(stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 2));
	assert(stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 2, host1, output));
	assert(state.geometry_revision == 3);
	assert(!stage_director_news_host_session_geometry_current(&state, &second));
	struct stage_director_news_host_viewport_receipt moved =
		stage_director_news_host_session_report_viewport(
			&state, 0x10, (uintptr_t)a, 0x20, 2, viewport_local);
	assert(moved.status == SD_NEWS_HOST_VIEWPORT_CANDIDATE_ONLY);
	assert(stage_director_news_host_session_viewport_current(&state, &moved));
	/* Panel move on the SAME output immediately revokes older player crop. */
	assert(stage_director_news_host_session_update_geometry(
		&state, 0x10, (uintptr_t)a, 0x20, 2, host2, output));
	assert(state.geometry_revision == 4);
	assert(!state.viewport_candidate_valid);
	assert(!stage_director_news_host_session_viewport_current(&state, &moved));
	wl_resource_destroy(role_a);
	assert(!state.candidate && state.last_epoch == 2);
	role_a = new_resource(a);

	/* Different destruction order must also revoke without a stale pointer. */
	assert(stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 3));
	wl_resource_destroy(surface_a);
	assert(!state.candidate && state.last_epoch == 3);
	surface_a = new_resource(a);

	/* Full client destruction revokes and retains the last used generation. */
	assert(stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 4));
	wl_client_destroy(a);
	assert(!state.candidate && state.last_epoch == 4);
	assert(!stage_director_news_host_session_tracks(&state, 0x10));
	wl_client_destroy(b);
	close(a_fd);
	close(b_fd);
	wl_display_destroy(display);
	puts("SD_NEWS_HOST_SESSION: real Wayland resource/client lifetime, "
		"foreign denial, viewport v2 reports/revocation, stale denial, and zero "
		"authority PASS");
	return 0;
}
