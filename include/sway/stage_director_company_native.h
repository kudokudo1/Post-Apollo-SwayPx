#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_NATIVE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_NATIVE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

#define SD_COMPANY_NATIVE_NAMESPACE "post-apollo-company-network"

struct sway_layer_surface;
struct sway_view;
struct wl_resource;

enum stage_director_company_native_state {
	SD_COMPANY_NATIVE_MISSING,
	SD_COMPANY_NATIVE_WRONG_NAMESPACE,
	SD_COMPANY_NATIVE_UNMAPPED,
	SD_COMPANY_NATIVE_WRONG_ROLE,
	SD_COMPANY_NATIVE_WRONG_CLIENT,
	SD_COMPANY_NATIVE_OUTPUT_INVALID,
	SD_COMPANY_NATIVE_PROCESS_UNVERIFIED,
	SD_COMPANY_NATIVE_CANDIDATE_ONLY,
};

/* Read-only observation. Neither a namespace nor native process evidence
 * authorizes Company control. These refs are in-compositor and must never
 * be serialized to QML, public IPC or a launcher command. */
struct stage_director_company_native_host {
	enum stage_director_company_native_state state;
	bool candidate;
	uintptr_t native_layer_ref;
	uintptr_t native_layer_role_ref;
	uintptr_t native_surface_ref;
	uintptr_t native_client_ref;
	uintptr_t native_output_ref;
	int64_t pid;
	uint32_t uid;
	uint64_t start_ticks;
	struct stage_director_rect host_global;
	struct stage_director_rect output_global;
	bool owner_verified;
	bool can_clip;
	bool can_accept_input;
};

struct stage_director_company_native_actor {
	enum stage_director_company_native_state state;
	bool candidate;
	uintptr_t native_view_ref;
	uintptr_t native_root_surface_ref;
	uintptr_t native_client_ref;
	int64_t pid;
	uint32_t uid;
	uint64_t start_ticks;
	/* Root XDG content origin/size observed in the compositor scene.
	 * This does not itself prove the view belongs to the Company process. */
	struct stage_director_rect root_global;
	bool owner_verified;
	bool can_clip;
	bool can_accept_input;
};

enum stage_director_company_native_peer_state {
	SD_COMPANY_PEER_INVALID_RESOURCES,
	SD_COMPANY_PEER_SPLIT_HOST_CLIENT,
	SD_COMPANY_PEER_NOT_SEPARATE_ACTOR,
	SD_COMPANY_PEER_DISTINCT_CANDIDATE_ONLY,
};

struct stage_director_company_native_peer_result {
	enum stage_director_company_native_peer_state state;
	bool distinct_native_clients_candidate;
	bool owner_verified;
	bool can_clip;
	bool can_accept_input;
};

/* Real Wayland resource identity only, no label/PID/pointer string match. */
struct stage_director_company_native_peer_result
stage_director_company_native_peer_probe(
	struct wl_resource *layer_role, struct wl_resource *host_surface,
	struct wl_resource *actor_root_surface);

/* The observers are snapshots only; no physical operator pairing or actor
 * custody issuer is provided by M20. They cannot call compositor setters. */
struct stage_director_company_native_host
stage_director_company_native_observe_host(
	const struct sway_layer_surface *surface);

struct stage_director_company_native_actor
stage_director_company_native_observe_actor(
	const struct sway_view *view);

#endif
