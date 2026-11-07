#ifndef _SWAY_STAGE_DIRECTOR_SOCIAL_PAIR_CANDIDATE_H
#define _SWAY_STAGE_DIRECTOR_SOCIAL_PAIR_CANDIDATE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Social W // compositor-local pair CANDIDATE, zero authority.
 *
 * This data model is deliberately NOT linked into production SwayPX yet.
 * Native refs and mapped lifetime generations MUST come directly from
 * trusted compositor callbacks; clients/IPC must not supply them.
 * The ledger does NOT authenticate operator intent or grant geometry,
 * focus, scene, pointer, popup, input, or render privileges.
 *
 * The existing Weather WATCH ledger follows the same candidate-only
 * pattern, but Social has a layer-shell host and a distinct XDG guest,
 * so its roles and replay conditions are separate.
 */
enum sd_social_member_role {
	SD_SOCIAL_HOST_LAYER = 1,
	SD_SOCIAL_GUEST_XDG = 2,
};

struct sd_social_native_member {
	enum sd_social_member_role role;
	uintptr_t container_ref;     /* Native, not a user-supplied con_id */
	uintptr_t client_ref;        /* Live wl_client identity */
	uintptr_t root_surface_ref;  /* Exact mapped native root surface */
	uintptr_t output_ref;
	uintptr_t workspace_ref;
	uintptr_t seat_ref;
	uint64_t mapped_generation;  /* Owner-issued native mapping lifetime */
	int64_t peer_pid;
	uint64_t peer_start_ticks;   /* Kernel start time; PID reuse defense */
	uint32_t peer_uid;
	bool mapped;
	bool exact_role;              /* Validated by trusted native role check */
	bool floating;                /* Required for the XDG guest only */
};

enum sd_social_candidate_state {
	SD_SOCIAL_CANDIDATE_EMPTY,
	SD_SOCIAL_CANDIDATE_HOST,
	SD_SOCIAL_CANDIDATE_PAIR,
};

struct sd_social_pair_candidate {
	enum sd_social_candidate_state state;
	uint64_t generation_floor;   /* Never cleared on revoke. */
	uint64_t generation;
	struct sd_social_native_member host;
	struct sd_social_native_member guest;
};

enum sd_social_candidate_probe_state {
	SD_SOCIAL_PROBE_EMPTY,
	SD_SOCIAL_PROBE_HOST_ONLY,
	SD_SOCIAL_PROBE_MISMATCH,
	SD_SOCIAL_PROBE_CORRELATED_CANDIDATE,
};

struct sd_social_candidate_probe {
	enum sd_social_candidate_probe_state state;
	bool candidate_correlated;
	bool authenticated_pairing;  /* Always false in this module. */
	bool can_move;
	bool can_resize;
	bool can_focus;
	bool can_clip;
	bool can_render;
	bool can_input;
	bool can_popup;
};

enum sd_social_candidate_lost {
	SD_SOCIAL_HOST_UNMAPPED,
	SD_SOCIAL_GUEST_UNMAPPED,
	SD_SOCIAL_HOST_CLIENT_LOST,
	SD_SOCIAL_GUEST_CLIENT_LOST,
	SD_SOCIAL_OUTPUT_LOST,
	SD_SOCIAL_WORKSPACE_LOST,
	SD_SOCIAL_SEAT_LOST,
	SD_SOCIAL_OPERATOR_CLOSED,
};

void sd_social_pair_candidate_init(struct sd_social_pair_candidate *pair);

/* Allocate a monotonically increasing generation INSIDE the compositor.
 * Return 0 if empty/invalid/overflowing. Does not grant user authority. */
uint64_t sd_social_pair_candidate_host(
	struct sd_social_pair_candidate *pair,
	const struct sd_social_native_member *observed_host);

/* An independent native guest/client must be on the same output/seat.
 * A duplicate or same-client guest cannot reserve this candidate. */
bool sd_social_pair_candidate_guest(
	struct sd_social_pair_candidate *pair, uint64_t generation,
	const struct sd_social_native_member *observed_guest);

/* Recheck exact native pointer + lifetime snapshots on every use.
 * Even a matched candidate has ALL actuation permission fields false. */
struct sd_social_candidate_probe sd_social_pair_candidate_probe(
	const struct sd_social_pair_candidate *pair, uint64_t generation,
	const struct sd_social_native_member *observed_host,
	const struct sd_social_native_member *observed_guest);

/* Revoke only the exact CURRENT generation and matching native source.
 * Stale old-generation destroy/unmap events cannot revoke a new pair. */
bool sd_social_pair_candidate_revoke(
	struct sd_social_pair_candidate *pair, uint64_t generation,
	enum sd_social_candidate_lost event, uintptr_t native_ref);

#endif
