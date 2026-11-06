#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_CANDIDATE_SESSION_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_CANDIDATE_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_map.h"
#include "sway/stage_director_company_native.h"

/*
 * M21 // revocable native resource CANDIDATE ledger, NOT a pairing issuer.
 *
 * Only a future trusted compositor physical selection / M15 launch attestation
 * may authorize an actor. This module never sets M19.compositor_selected.
 * Real wl_resource destruction invalidates the borrowed pointers immediately.
 *
 * Future Company-specific private Wayland protocol may call report() only
 * after validating that the wl_resource is an actual protocol binding from the
 * selected live Company wl_client. This C function itself does not create a
 * wl_global and does not authenticate a particular QML Item or launch.
 */
enum stage_director_company_session_status {
	SD_COMPANY_SESSION_NO_CANDIDATE,
	SD_COMPANY_SESSION_FOREIGN_REPORTER,
	SD_COMPANY_SESSION_STALE_NATIVE,
	SD_COMPANY_SESSION_INVALID_GEOMETRY,
	SD_COMPANY_SESSION_ACTOR_NOT_COVERING,
	SD_COMPANY_SESSION_HIDDEN,
	SD_COMPANY_SESSION_GEOMETRY_CANDIDATE_ONLY,
};

struct stage_director_company_session_receipt {
	enum stage_director_company_session_status status;
	uint64_t epoch;
	uint64_t revision;
	struct stage_director_rect viewport_global;
	struct stage_director_rect actor_local_crop;
	bool geometry_candidate;
	/* Always false. This object is NOT a compositor operator approval. */
	bool owner_verified;
	bool can_launch;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_show_popup;
	bool can_focus;
};

struct stage_director_company_candidate_session {
	bool initialized;
	bool candidate;
	bool viewport_valid;
	uint64_t epoch;
	uint64_t last_epoch;
	uint64_t revision;
	uint64_t last_revision;
	struct wl_resource *host_role;
	struct wl_resource *host_surface;
	struct wl_resource *actor_root;
	struct wl_client *host_client;
	struct wl_client *actor_client;
	struct stage_director_company_native_host host;
	struct stage_director_company_native_actor actor;
	struct stage_director_rect viewport_global;
	struct stage_director_rect actor_local_crop;
	struct wl_listener role_destroy;
	struct wl_listener surface_destroy;
	struct wl_listener actor_destroy;
	bool listeners_active;
};

void stage_director_company_candidate_session_init(
	struct stage_director_company_candidate_session *session);

/* Revokes the entire native candidate and listeners, leaves last_epoch and
 * last_revision monotonic so a later pair cannot reuse old request numbers. */
void stage_director_company_candidate_session_close(
	struct stage_director_company_candidate_session *session);

/* Observe one exact native host/actor resource triplet. The resulting
 * candidate remains INERT and is NOT an authenticated owner/lease. */
bool stage_director_company_candidate_session_record(
	struct stage_director_company_candidate_session *session,
	struct wl_resource *host_role,
	struct wl_resource *host_surface,
	struct wl_resource *actor_root,
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor);

/* Reobserve both live native objects before accepting an inner QML rect.
 * A move/output change increments revision and invalidates prior receipt.
 * Missing, swapped, replaced or restarted actor closes the candidate. */
bool stage_director_company_candidate_session_refresh(
	struct stage_director_company_candidate_session *session,
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor);

/* Geometry remains a hint; reporter MUST be a separate Wayland resource on
 * the exact host wl_client, and the caller must verify private interface
 * membership/epoch before invoking from a real protocol handler. */
struct stage_director_company_session_receipt
stage_director_company_candidate_session_report(
	struct stage_director_company_candidate_session *session,
	struct wl_resource *reporter,
	uint64_t epoch, uint64_t revision,
	const struct stage_director_company_map_report *report);

bool stage_director_company_candidate_session_receipt_current(
	const struct stage_director_company_candidate_session *session,
	const struct stage_director_company_session_receipt *receipt);

#endif
