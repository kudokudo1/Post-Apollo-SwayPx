#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_RECONCILE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_RECONCILE_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_birth_pidfd.h"
#include "sway/stage_director_company_candidate_session.h"
#include "sway/stage_director_company_lineage.h"

/*
 * M30b: bounded, read-only M28 birth / M20 native-map reconciliation.
 * NOT production-wired, no compositor scene/input authority.
 * Owns one candidate with real wl_resource destruction listeners and
 * delegates exact native-resource candidate lifetime to M21 on match.
 * Event ordering is arbitrary: birth-before-map or map-before-birth.
 */
enum stage_director_company_reconcile_status {
	SD_COMPANY_RECONCILE_WAITING,
	SD_COMPANY_RECONCILE_REJECTED,
	SD_COMPANY_RECONCILE_M21_CANDIDATE_ONLY,
};
struct stage_director_company_reconcile_result {
	enum stage_director_company_reconcile_status status;
	bool native_candidate;
	bool owner_verified;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_focus;
	bool can_show_popup;
};
struct stage_director_company_reconcile {
	bool initialized;
	bool born;
	bool mapped;
	bool listeners_active;
	uint64_t birth_selection_epoch;
	uint64_t birth_intent_sequence;
	int64_t birth_pid;
	uint64_t birth_start_ticks;
	struct wl_resource *host_role;
	struct wl_resource *host_surface;
	struct wl_resource *actor_root;
	struct stage_director_company_native_host host;
	struct stage_director_company_native_actor actor;
	struct wl_listener host_role_destroy;
	struct wl_listener host_surface_destroy;
	struct wl_listener actor_root_destroy;
	struct stage_director_company_candidate_session m21;
};

void stage_director_company_reconcile_init(
	struct stage_director_company_reconcile *state);
void stage_director_company_reconcile_revoke(
	struct stage_director_company_reconcile *state);

/* M28 receipt from native kernel/pidfd gate, never a client-claimed PID. */
bool stage_director_company_reconcile_birth(
	struct stage_director_company_reconcile *state,
	const struct stage_director_company_birth_pidfd *held,
	const struct stage_director_company_native_host *host);

/* M20 mapped root with real compositor-native wl_resources. */
bool stage_director_company_reconcile_map(
	struct stage_director_company_reconcile *state,
	struct wl_resource *host_role, struct wl_resource *host_surface,
	struct wl_resource *actor_root,
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor);

/* Explicit unmap/close must invalidate even if wl_resource survives. */
void stage_director_company_reconcile_unmap(
	struct stage_director_company_reconcile *state,
	struct wl_resource *actor_root);

/* May be called following either event. Rechecks held kernel pidfd/process,
 * real native resource identity and M22 lineage before connecting to M21.
 * A positive result is only M21 CANDIDATE; never a scene/seat lease. */
struct stage_director_company_reconcile_result
stage_director_company_reconcile_try(
	struct stage_director_company_reconcile *state,
	const struct stage_director_company_birth_pidfd *held,
	const struct stage_director_company_lineage_result *lineage,
	const struct stage_director_company_native_host *fresh_host,
	const struct stage_director_company_native_actor *fresh_actor);

#endif
