#define _GNU_SOURCE
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_company_reconcile.h"

struct fixture {
	struct wl_display *display;
	struct wl_client *host_client, *actor_client;
	int host_fd, actor_fd;
	struct wl_resource *role, *surface, *root;
	struct stage_director_company_native_host host;
	struct stage_director_company_native_actor actor;
	struct stage_director_company_birth_pidfd held;
	struct stage_director_company_reconcile state;
	pid_t child;
};
static struct wl_client *new_client(struct wl_display *d, int *fd) {
	int pair[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
	struct wl_client *c = wl_client_create(d, pair[0]);
	assert(c);
	*fd = pair[1];
	return c;
}
static struct wl_resource *resource(struct wl_client *client) {
	struct wl_resource *r =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(r);
	return r;
}
static struct fixture fixture(void) {
	assert(getuid() != 0 && getuid() == geteuid());
	struct fixture f = {0};
	f.display = wl_display_create();
	assert(f.display);
	f.host_client = new_client(f.display, &f.host_fd);
	f.actor_client = new_client(f.display, &f.actor_fd);
	f.role = resource(f.host_client);
	f.surface = resource(f.host_client);
	f.root = resource(f.actor_client);
	f.child = fork();
	assert(f.child >= 0);
	if (!f.child) {
		execl("/usr/bin/sleep", "sleep", "30", (char *)NULL);
		_exit(127);
	}
	struct stage_director_process_identity hostp = {0}, kid = {0};
	assert(stage_director_read_linux_process(getpid(), &hostp, NULL));
	assert(stage_director_read_linux_process(f.child, &kid, NULL));
	assert(kid.parent_pid == hostp.pid);
	f.host = (struct stage_director_company_native_host) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true,
		.native_layer_ref = 0x100,
		.native_layer_role_ref = (uintptr_t)f.role,
		.native_surface_ref = (uintptr_t)f.surface,
		.native_client_ref = (uintptr_t)f.host_client,
		.native_output_ref = 0x400,
		.pid = hostp.pid, .uid = hostp.uid,
		.start_ticks = hostp.start_time_ticks,
		.host_global = {100,40,1000,700},
		.output_global = {0,0,2560,1440},
	};
	f.actor = (struct stage_director_company_native_actor) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true, .native_view_ref = 0x200,
		.native_root_surface_ref = (uintptr_t)f.root,
		.native_client_ref = (uintptr_t)f.actor_client,
		.pid = f.child, .uid = hostp.uid,
		.start_ticks = kid.start_time_ticks,
		.root_global = {280,150,820,570},
	};
#if defined(SYS_pidfd_open)
	int pidfd = (int)syscall(SYS_pidfd_open, f.child, 0);
#else
	int pidfd = -1;
#endif
	assert(pidfd >= 0);
	struct stage_director_company_intent_snapshot intent = {
		.present = true, .selection_epoch = 30, .sequence = 1,
		.observed_boot_ticks = kid.start_time_ticks, .host_revision = 1,
		.native_host_client_ref = f.host.native_client_ref, .mode = 0,
	};
	stage_director_company_birth_pidfd_init(&f.held);
	struct stage_director_company_birth_result birth =
		stage_director_company_birth_pidfd_record(
			&f.held, pidfd, f.child, &intent, &f.host,
			stage_director_read_linux_process, NULL);
	assert(birth.status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY);
	assert(close(pidfd) == 0);
	stage_director_company_reconcile_init(&f.state);
	/* All client resources here are genuine Wayland server objects, but
	 * actor native peer metadata is a fixture, not a real Qt/QPA client. */
	return f;
}
static struct stage_director_company_lineage_result lineage(void) {
	return (struct stage_director_company_lineage_result) {
		.status = SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
		.direct_child_candidate = true,
	};
}
static bool map(struct fixture *f) {
	return stage_director_company_reconcile_map(&f->state,
		f->role, f->surface, f->root, &f->host, &f->actor);
}
static bool birth(struct fixture *f) {
	return stage_director_company_reconcile_birth(
		&f->state, &f->held, &f->host);
}
static struct stage_director_company_reconcile_result try(struct fixture *f) {
	struct stage_director_company_lineage_result l = lineage();
	return stage_director_company_reconcile_try(
		&f->state, &f->held, &l, &f->host, &f->actor);
}
static void inert(struct stage_director_company_reconcile_result r) {
	assert(!r.owner_verified && !r.can_render && !r.can_clip);
	assert(!r.can_accept_input && !r.can_focus && !r.can_show_popup);
	assert(r.native_candidate ==
		(r.status == SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY));
}
static void finish(struct fixture *f) {
	stage_director_company_reconcile_revoke(&f->state);
	stage_director_company_birth_pidfd_revoke(&f->held);
	assert(kill(f->child, SIGTERM) == 0);
	int status = 0;
	assert(waitpid(f->child, &status, 0) == f->child);
	wl_display_destroy_clients(f->display);
	assert(close(f->host_fd) == 0 && close(f->actor_fd) == 0);
	wl_display_destroy(f->display);
}
static void birth_before_map(void) {
	struct fixture f = fixture();
	assert(birth(&f));
	struct stage_director_company_reconcile_result r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_WAITING);
	assert(!f.state.m21.candidate);
	assert(map(&f));
	r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY);
	assert(f.state.m21.candidate && f.state.m21.epoch == 1);
	/* M21 is still not an approved M19 scene lease. */
	assert(!f.state.m21.viewport_valid);
	wl_resource_destroy(f.root);
	assert(!f.state.mapped && !f.state.born);
	assert(!f.state.m21.candidate);
	finish(&f);
}
static void map_before_birth(void) {
	struct fixture f = fixture();
	assert(map(&f));
	struct stage_director_company_reconcile_result r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_WAITING);
	assert(!f.state.m21.candidate);
	assert(birth(&f));
	r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY);
	assert(f.state.m21.candidate);
	stage_director_company_reconcile_unmap(&f.state, f.root);
	assert(!f.state.m21.candidate);
	assert(!f.state.listeners_active);
	finish(&f);
}
static void stale_generation_rejected(void) {
	struct fixture f = fixture();
	assert(map(&f) && birth(&f));
	f.held.intent_sequence++;
	struct stage_director_company_reconcile_result r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_REJECTED);
	assert(!f.state.m21.candidate && !f.state.mapped);
	f.held.intent_sequence--;
	finish(&f);
}
static void wrong_child_rejected(void) {
	struct fixture f = fixture();
	f.actor.start_ticks++;
	assert(map(&f) && birth(&f));
	struct stage_director_company_reconcile_result r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_REJECTED);
	assert(!f.state.m21.candidate);
	finish(&f);
}
static void role_destroy_revokes_before_birth(void) {
	struct fixture f = fixture();
	assert(map(&f));
	wl_resource_destroy(f.role);
	assert(!f.state.mapped && !f.state.listeners_active);
	struct stage_director_company_reconcile_result r = try(&f);
	inert(r);
	assert(r.status == SD_COMPANY_RECONCILE_WAITING);
	finish(&f);
}
int main(void) {
	birth_before_map();
	map_before_birth();
	stale_generation_rejected();
	wrong_child_rejected();
	role_destroy_revokes_before_birth();
	puts("STAGE_DIRECTOR_COMPANY_M30B: PASS both orders, real pidfd, native resource revoke and M21 candidate; authority=NONE");
	return 0;
}
