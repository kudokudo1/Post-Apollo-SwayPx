#include <stdint.h>
#include <string.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_reconcile.h"

/* This module accepts exclusively compositor-native snapshots. A public
 * namespace, app_id, title, numeric PID or arbitrary QML string is NOT
 * actor ownership. Even real M28+M21 matching remains evidence-only. */
static struct stage_director_company_reconcile_result result(
		enum stage_director_company_reconcile_status status) {
	return (struct stage_director_company_reconcile_result) {
		.status = status,
		.native_candidate = status == SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY,
	};
}
void stage_director_company_reconcile_init(
		struct stage_director_company_reconcile *s) {
	if (!s || s->initialized) return;
	memset(s, 0, sizeof(*s));
	s->initialized = true;
	wl_list_init(&s->host_role_destroy.link);
	wl_list_init(&s->host_surface_destroy.link);
	wl_list_init(&s->actor_root_destroy.link);
	stage_director_company_candidate_session_init(&s->m21);
}
void stage_director_company_reconcile_revoke(
		struct stage_director_company_reconcile *s) {
	if (!s || !s->initialized) return;
	stage_director_company_candidate_session_close(&s->m21);
	if (s->listeners_active) {
		wl_list_remove(&s->host_role_destroy.link);
		wl_list_remove(&s->host_surface_destroy.link);
		wl_list_remove(&s->actor_root_destroy.link);
		wl_list_init(&s->host_role_destroy.link);
		wl_list_init(&s->host_surface_destroy.link);
		wl_list_init(&s->actor_root_destroy.link);
		s->listeners_active = false;
	}
	s->born = false;
	s->mapped = false;
	s->birth_selection_epoch = 0;
	s->birth_intent_sequence = 0;
	s->birth_pid = 0;
	s->birth_start_ticks = 0;
	s->host_role = NULL;
	s->host_surface = NULL;
	s->actor_root = NULL;
	s->host = (struct stage_director_company_native_host){0};
	s->actor = (struct stage_director_company_native_actor){0};
}
static void destroyed_host_role(struct wl_listener *l, void *data) {
	(void)data;
	struct stage_director_company_reconcile *s =
		wl_container_of(l, s, host_role_destroy);
	stage_director_company_reconcile_revoke(s);
}
static void destroyed_host_surface(struct wl_listener *l, void *data) {
	(void)data;
	struct stage_director_company_reconcile *s =
		wl_container_of(l, s, host_surface_destroy);
	stage_director_company_reconcile_revoke(s);
}
static void destroyed_actor_root(struct wl_listener *l, void *data) {
	(void)data;
	struct stage_director_company_reconcile *s =
		wl_container_of(l, s, actor_root_destroy);
	stage_director_company_reconcile_revoke(s);
}
static bool same_host_identity(
		const struct stage_director_company_native_host *a,
		const struct stage_director_company_native_host *b) {
	return a && b && a->candidate && b->candidate &&
		a->state == SD_COMPANY_NATIVE_CANDIDATE_ONLY &&
		b->state == SD_COMPANY_NATIVE_CANDIDATE_ONLY &&
		a->native_layer_role_ref == b->native_layer_role_ref &&
		a->native_surface_ref == b->native_surface_ref &&
		a->native_client_ref == b->native_client_ref &&
		a->native_layer_ref == b->native_layer_ref &&
		a->native_output_ref == b->native_output_ref &&
		a->pid == b->pid && a->uid == b->uid &&
		a->start_ticks == b->start_ticks;
}
bool stage_director_company_reconcile_birth(
		struct stage_director_company_reconcile *s,
		const struct stage_director_company_birth_pidfd *held,
		const struct stage_director_company_native_host *host) {
	if (!s || !s->initialized || s->born || !held || !host ||
			!held->initialized || !held->held || held->fd < 0 ||
			!held->selection_epoch || !held->intent_sequence ||
			!held->pid || !held->child_start_ticks ||
			!host->candidate ||
			host->native_client_ref != held->host_native_client_ref ||
			host->pid != held->host_pid || host->uid != held->uid ||
			host->start_ticks != held->host_start_ticks) {
		return false;
	}
	if (s->mapped && !same_host_identity(&s->host, host)) {
		stage_director_company_reconcile_revoke(s);
		return false;
	}
	s->born = true;
	s->birth_selection_epoch = held->selection_epoch;
	s->birth_intent_sequence = held->intent_sequence;
	s->birth_pid = held->pid;
	s->birth_start_ticks = held->child_start_ticks;
	if (!s->mapped) s->host = *host;
	return true;
}
bool stage_director_company_reconcile_map(
		struct stage_director_company_reconcile *s,
		struct wl_resource *role, struct wl_resource *surface,
		struct wl_resource *root,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor) {
	if (!s || !s->initialized || s->mapped || !role || !surface ||
			!root || role == surface || surface == root ||
			role == root || !host || !actor ||
			!host->candidate || !actor->candidate ||
			host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			host->native_layer_role_ref != (uintptr_t)role ||
			host->native_surface_ref != (uintptr_t)surface ||
			actor->native_root_surface_ref != (uintptr_t)root ||
			wl_resource_get_client(role) != wl_resource_get_client(surface) ||
			wl_resource_get_client(role) == wl_resource_get_client(root) ||
			host->native_client_ref !=
				(uintptr_t)wl_resource_get_client(role) ||
			actor->native_client_ref !=
				(uintptr_t)wl_resource_get_client(root) ||
			host->uid != actor->uid || !host->uid ||
			host->pid == actor->pid || !actor->start_ticks ||
			!host->start_ticks) {
		return false;
	}
	if (s->born && !same_host_identity(&s->host, host)) {
		stage_director_company_reconcile_revoke(s);
		return false;
	}
	s->host = *host;
	s->actor = *actor;
	s->host_role = role;
	s->host_surface = surface;
	s->actor_root = root;
	s->mapped = true;
	s->host_role_destroy.notify = destroyed_host_role;
	s->host_surface_destroy.notify = destroyed_host_surface;
	s->actor_root_destroy.notify = destroyed_actor_root;
	wl_resource_add_destroy_listener(role, &s->host_role_destroy);
	wl_resource_add_destroy_listener(surface, &s->host_surface_destroy);
	wl_resource_add_destroy_listener(root, &s->actor_root_destroy);
	s->listeners_active = true;
	return true;
}
void stage_director_company_reconcile_unmap(
		struct stage_director_company_reconcile *s,
		struct wl_resource *root) {
	if (s && s->initialized && s->mapped && root == s->actor_root) {
		stage_director_company_reconcile_revoke(s);
	}
}
struct stage_director_company_reconcile_result
stage_director_company_reconcile_try(
		struct stage_director_company_reconcile *s,
		const struct stage_director_company_birth_pidfd *held,
		const struct stage_director_company_lineage_result *lineage,
		const struct stage_director_company_native_host *fresh_host,
		const struct stage_director_company_native_actor *fresh_actor) {
	if (!s || !s->initialized || !s->mapped || !s->born) {
		return result(SD_COMPANY_RECONCILE_WAITING);
	}
	if (!held || !held->held || held->pid != s->birth_pid ||
			held->child_start_ticks != s->birth_start_ticks ||
			held->selection_epoch != s->birth_selection_epoch ||
			held->intent_sequence != s->birth_intent_sequence ||
			!same_host_identity(&s->host, fresh_host) ||
			!fresh_actor || !fresh_actor->candidate ||
			fresh_actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			fresh_actor->native_view_ref != s->actor.native_view_ref ||
			fresh_actor->native_root_surface_ref != s->actor.native_root_surface_ref ||
			fresh_actor->native_client_ref != s->actor.native_client_ref ||
			fresh_actor->pid != s->actor.pid ||
			fresh_actor->uid != s->actor.uid ||
			fresh_actor->start_ticks != s->actor.start_ticks ||
			!s->host_role || !s->host_surface || !s->actor_root ||
			(uintptr_t)s->host_role != s->host.native_layer_role_ref ||
			(uintptr_t)s->host_surface != s->host.native_surface_ref ||
			(uintptr_t)s->actor_root != s->actor.native_root_surface_ref) {
		stage_director_company_reconcile_revoke(s);
		return result(SD_COMPANY_RECONCILE_REJECTED);
	}
	/* Re-verify held Linux pidfd, child generation, native host and
	 * actor parentage AFTER both events. The staging snapshots alone are
	 * never adequate for an actor candidate, much less ownership. */
	const struct stage_director_company_birth_result birth =
		stage_director_company_birth_pidfd_match_actor(
			(struct stage_director_company_birth_pidfd *)held,
			fresh_host, fresh_actor, lineage,
			stage_director_read_linux_process, NULL);
	if (birth.status != SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY ||
			!birth.mapped_actor_candidate || birth.owner_verified ||
			birth.can_pair_actor || birth.can_render || birth.can_clip ||
			birth.can_accept_input) {
		stage_director_company_reconcile_revoke(s);
		return result(SD_COMPANY_RECONCILE_REJECTED);
	}
	if (!s->m21.candidate) {
		if (!stage_director_company_candidate_session_record(
				&s->m21, s->host_role, s->host_surface,
				s->actor_root, fresh_host, fresh_actor)) {
			stage_director_company_reconcile_revoke(s);
			return result(SD_COMPANY_RECONCILE_REJECTED);
		}
	}
	if (!stage_director_company_candidate_session_refresh(
			&s->m21, fresh_host, fresh_actor)) {
		stage_director_company_reconcile_revoke(s);
		return result(SD_COMPANY_RECONCILE_REJECTED);
	}
	return result(SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY);
}
