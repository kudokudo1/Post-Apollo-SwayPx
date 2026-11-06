#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_company_candidate_session.h"

struct native_fixture {
	struct wl_display *display;
	struct wl_client *host_client, *actor_client, *foreign_client;
	int host_fd, actor_fd, foreign_fd;
	struct wl_resource *role, *host_surface, *actor_root;
	struct wl_resource *host_reporter, *foreign_reporter;
	struct stage_director_company_native_host host;
	struct stage_director_company_native_actor actor;
	struct stage_director_company_candidate_session session;
};

static struct wl_client *new_client(struct wl_display *d, int *fd) {
	int fds[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
	struct wl_client *client = wl_client_create(d, fds[0]);
	assert(client);
	*fd = fds[1];
	return client;
}

static struct wl_resource *resource(struct wl_client *client) {
	struct wl_resource *out =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(out);
	return out;
}

static struct native_fixture fixture(void) {
	struct native_fixture f = {0};
	f.display = wl_display_create();
	assert(f.display);
	f.host_client = new_client(f.display, &f.host_fd);
	f.actor_client = new_client(f.display, &f.actor_fd);
	f.foreign_client = new_client(f.display, &f.foreign_fd);
	f.role = resource(f.host_client);
	f.host_surface = resource(f.host_client);
	f.actor_root = resource(f.actor_client);
	f.host_reporter = resource(f.host_client);
	f.foreign_reporter = resource(f.foreign_client);
	f.host = (struct stage_director_company_native_host) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true,
		.native_layer_ref = 0x100,
		.native_layer_role_ref = (uintptr_t)f.role,
		.native_surface_ref = (uintptr_t)f.host_surface,
		.native_client_ref = (uintptr_t)f.host_client,
		.native_output_ref = 0x400,
		.pid = 1500,
		.uid = 1000,
		.start_ticks = 30,
		.host_global = {100, 40, 1000, 700},
		.output_global = {0, 0, 2560, 1440},
	};
	f.actor = (struct stage_director_company_native_actor) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true,
		.native_view_ref = 0x200,
		.native_root_surface_ref = (uintptr_t)f.actor_root,
		.native_client_ref = (uintptr_t)f.actor_client,
		.pid = 1501,
		.uid = 1000,
		.start_ticks = 31,
		.root_global = {280, 150, 820, 570},
	};
	stage_director_company_candidate_session_init(&f.session);
	return f;
}

/* IMPORTANT: No pointer to the fixture's session may escape or be copied
 * AFTER registration: Wayland destroy listeners are embedded in the session.
 * The returned-by-value fixture is used only before registration. */
static bool record(struct native_fixture *f) {
	return stage_director_company_candidate_session_record(
		&f->session, f->role, f->host_surface, f->actor_root,
		&f->host, &f->actor);
}

static struct stage_director_company_map_report report(
		const struct native_fixture *f) {
	return (struct stage_director_company_map_report) {
		.epoch = f->session.epoch,
		.host_geometry_revision = f->session.revision,
		.company_open = true,
		.coverage_active = true,
		.native_map_running = true,
		.frame_in_host = {12, 9, 976, 679},
		.map_in_frame = {200, 110, 650, 450},
	};
}

static struct stage_director_company_session_receipt submit(
		struct native_fixture *f,
		const struct stage_director_company_map_report *r) {
	return stage_director_company_candidate_session_report(
		&f->session, f->host_reporter,
		f->session.epoch, f->session.revision, r);
}

static void no_authority(struct stage_director_company_session_receipt r) {
	assert(!r.owner_verified);
	assert(!r.can_launch);
	assert(!r.can_render);
	assert(!r.can_clip);
	assert(!r.can_accept_input);
	assert(!r.can_show_popup);
	assert(!r.can_focus);
}

static void fixture_finish(struct native_fixture *f) {
	stage_director_company_candidate_session_close(&f->session);
	wl_display_destroy_clients(f->display);
	close(f->host_fd);
	close(f->actor_fd);
	close(f->foreign_fd);
	wl_display_destroy(f->display);
}

static void candidate_and_one_shot_receipt(void) {
	struct native_fixture f = fixture();
	assert(record(&f));
	assert(f.session.candidate);
	assert(!f.session.viewport_valid);
	struct stage_director_company_map_report r = report(&f);
	struct stage_director_company_session_receipt receipt = submit(&f, &r);
	assert(receipt.status == SD_COMPANY_SESSION_NO_CANDIDATE);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	receipt = submit(&f, &r);
	no_authority(receipt);
	assert(receipt.status == SD_COMPANY_SESSION_GEOMETRY_CANDIDATE_ONLY);
	assert(receipt.geometry_candidate);
	assert(receipt.viewport_global.x == 312);
	assert(receipt.viewport_global.y == 159);
	assert(receipt.viewport_global.width == 650);
	assert(receipt.viewport_global.height == 450);
	assert(receipt.actor_local_crop.x == 32);
	assert(receipt.actor_local_crop.y == 9);
	assert(stage_director_company_candidate_session_receipt_current(
		&f.session, &receipt));
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_NO_CANDIDATE);
	/* A candidate never authenticates the separate M19 lease. */
	struct stage_director_company_map_lease unselected = {
		.compositor_selected = false,
		.epoch = f.session.epoch,
	};
	assert(stage_director_company_map_probe(&unselected, NULL, NULL).status ==
		SD_COMPANY_MAP_NO_LEASE);
	fixture_finish(&f);
}

static void foreign_and_stale_reports_fail_closed(void) {
	struct native_fixture f = fixture();
	assert(record(&f));
	struct stage_director_company_map_report r = report(&f);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	struct stage_director_company_session_receipt bad =
		stage_director_company_candidate_session_report(
			&f.session, f.foreign_reporter, f.session.epoch,
			f.session.revision, &r);
	no_authority(bad);
	assert(bad.status == SD_COMPANY_SESSION_FOREIGN_REPORTER);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	bad = stage_director_company_candidate_session_report(
		&f.session, f.role, f.session.epoch, f.session.revision, &r);
	assert(bad.status == SD_COMPANY_SESSION_FOREIGN_REPORTER);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	bad = stage_director_company_candidate_session_report(
		&f.session, f.host_reporter, f.session.epoch + 1,
		f.session.revision, &r);
	assert(bad.status == SD_COMPANY_SESSION_STALE_NATIVE);
	fixture_finish(&f);
}

static void changed_host_and_actor_invalidate_prior_geometry(void) {
	struct native_fixture f = fixture();
	assert(record(&f));
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	struct stage_director_company_map_report r = report(&f);
	struct stage_director_company_session_receipt old = submit(&f, &r);
	assert(stage_director_company_candidate_session_receipt_current(
		&f.session, &old));
	const uint64_t previous_revision = f.session.revision;
	f.host.host_global.x += 100;
	f.actor.root_global.x += 100;
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	assert(f.session.revision > previous_revision);
	assert(!stage_director_company_candidate_session_receipt_current(
		&f.session, &old));
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_STALE_NATIVE);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	r = report(&f);
	const struct stage_director_company_session_receipt updated =
		submit(&f, &r);
	assert(updated.status == SD_COMPANY_SESSION_GEOMETRY_CANDIDATE_ONLY);
	assert(updated.viewport_global.x == 412);
	assert(stage_director_company_candidate_session_receipt_current(
		&f.session, &updated));
	/* Actor moves out of the selected viewport: cannot stretch or paint. */
	f.actor.root_global.x += 600;
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	r = report(&f);
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_ACTOR_NOT_COVERING);
	assert(!f.session.viewport_valid);
	fixture_finish(&f);
}

static void hidden_and_invalid_geometry(void) {
	struct native_fixture f = fixture();
	assert(record(&f));
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	struct stage_director_company_map_report r = report(&f);
	r.company_open = false;
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_HIDDEN);
	assert(!f.session.viewport_valid);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	r = report(&f);
	r.map_in_frame.x = -1;
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_INVALID_GEOMETRY);
	assert(stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	r = report(&f);
	r.map_in_frame.width += 400;
	assert(submit(&f, &r).status == SD_COMPANY_SESSION_INVALID_GEOMETRY);
	fixture_finish(&f);
}

static void native_peer_replacement_revokes_candidate(void) {
	struct native_fixture f = fixture();
	f.actor.native_client_ref = (uintptr_t)f.host_client;
	assert(!record(&f)); // same-host actor reference not attested
	f.actor.native_client_ref = (uintptr_t)f.actor_client;
	assert(record(&f));
	f.actor.start_ticks++; // Linux PID reuse
	assert(!stage_director_company_candidate_session_refresh(
		&f.session, &f.host, &f.actor));
	assert(!f.session.candidate);
	assert(!f.session.viewport_valid);
	/* Re-pair requires a fresh generation, never an old epoch. */
	f.actor.start_ticks--;
	assert(record(&f));
	assert(f.session.epoch == 2);
	fixture_finish(&f);
}

static void actual_native_resource_destruction_closes_lease(void) {
	for (int which = 0; which < 3; ++which) {
		struct native_fixture f = fixture();
		assert(record(&f));
		assert(stage_director_company_candidate_session_refresh(
			&f.session, &f.host, &f.actor));
		struct stage_director_company_map_report r = report(&f);
		const struct stage_director_company_session_receipt receipt =
			submit(&f, &r);
		assert(receipt.geometry_candidate);
		const uint64_t last_epoch = f.session.epoch;
		if (which == 0) wl_resource_destroy(f.role);
		if (which == 1) wl_resource_destroy(f.host_surface);
		if (which == 2) wl_resource_destroy(f.actor_root);
		assert(!f.session.candidate);
		assert(!f.session.viewport_valid);
		assert(!f.session.listeners_active);
		assert(f.session.epoch == 0);
		assert(f.session.last_epoch == last_epoch);
		assert(!stage_director_company_candidate_session_receipt_current(
			&f.session, &receipt));
		fixture_finish(&f);
	}
}

int main(void) {
	candidate_and_one_shot_receipt();
	foreign_and_stale_reports_fail_closed();
	changed_host_and_actor_invalidate_prior_geometry();
	hidden_and_invalid_geometry();
	native_peer_replacement_revokes_candidate();
	actual_native_resource_destruction_closes_lease();
	puts("STAGE_DIRECTOR_COMPANY_M21: PASS native revocation/receipt, authority=NONE");
	return 0;
}
