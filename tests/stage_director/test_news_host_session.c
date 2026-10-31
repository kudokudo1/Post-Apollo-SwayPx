#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_news_host_session.h"

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
	stage_director_news_host_session_host_lost(&state, 0x99);
	assert(state.candidate); /* wrong host cannot revoke */
	stage_director_news_host_session_host_lost(&state, 0x10);
	assert(!state.candidate);
	denied(probe(&state, watch_a, 0x10, 0x20, 1),
		SD_NEWS_HOST_SESSION_EMPTY);
	assert(!stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 1));

	/* wl_resource destruction automatically invalidates the native candidate. */
	assert(stage_director_news_host_session_record_native_candidate(
		&state, role_a, surface_a, 0x10, 0x20, 2));
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
		"foreign denial, host/output/epoch checks, revocation and zero "
		"authority PASS");
	return 0;
}
