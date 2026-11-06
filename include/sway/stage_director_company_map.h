#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_MAP_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_MAP_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * M19 // Company native MapLibre CANDIDATE preflight, not an actor launcher.
 *
 * The future SwayPX owner must populate lease and observed from live native
 * compositor objects and independently verified Linux process generations.
 * Never accept native references, UID/PID, selection or guard flags from
 * Quickshell properties, swaymsg, window title, app_id or other public IPC.
 *
 * Company M18 publishes a map rectangle relative to companyFrame, NOT to
 * the layer-shell wl_surface. The separate frame_in_host rectangle must be
 * authenticated on the exact paired Company Wayland connection before use.
 *
 * This policy deliberately has NO path to scene, seat, popup, focus or spawn.
 */

struct stage_director_company_map_lease {
	bool compositor_selected;
	uint64_t epoch;
	uintptr_t native_host_layer_ref;
	uintptr_t native_host_surface_ref;
	uintptr_t native_host_client_ref;
	uintptr_t native_output_ref;
	uintptr_t native_actor_view_ref;
	uintptr_t native_actor_root_surface_ref;
	uintptr_t native_actor_client_ref;
	int64_t actor_pid;
	uint32_t actor_uid;
	uint64_t actor_start_ticks;
};

/* Fresh compositor observations, not caller-submitted identity assertions. */
struct stage_director_company_map_observed {
	uint64_t epoch;
	uint64_t host_geometry_revision;
	uintptr_t native_host_layer_ref;
	uintptr_t native_host_surface_ref;
	uintptr_t native_host_client_ref;
	uintptr_t native_output_ref;
	uintptr_t native_actor_view_ref;
	uintptr_t native_actor_root_surface_ref;
	uintptr_t native_actor_client_ref;
	int64_t actor_pid;
	uint32_t actor_uid;
	uint64_t actor_start_ticks;
	bool host_mapped;
	bool actor_mapped;
	bool output_active;
	bool actor_root_is_toplevel;
	bool actor_process_attested;
	struct stage_director_rect host_global;
	struct stage_director_rect output_global;
	struct stage_director_rect actor_global;
};

/* QML report: geometry plus the current opaque compositor revision. */
struct stage_director_company_map_report {
	uint64_t epoch;
	uint64_t host_geometry_revision;
	bool company_open;
	bool coverage_active;
	bool native_map_running;
	struct stage_director_rect frame_in_host;
	struct stage_director_rect map_in_frame;
};

enum stage_director_company_map_status {
	SD_COMPANY_MAP_NO_LEASE,
	SD_COMPANY_MAP_WRONG_NATIVE_PEER,
	SD_COMPANY_MAP_STALE_HOST,
	SD_COMPANY_MAP_STALE_ACTOR,
	SD_COMPANY_MAP_HIDDEN,
	SD_COMPANY_MAP_INVALID_GEOMETRY,
	SD_COMPANY_MAP_ACTOR_NOT_COVERING,
	SD_COMPANY_MAP_CANDIDATE_ONLY,
};

struct stage_director_company_map_result {
	enum stage_director_company_map_status status;
	bool geometry_candidate;
	struct stage_director_rect viewport_global;
	struct stage_director_rect actor_local_crop;
	/* All authority bits ALWAYS false, even when geometry matches. */
	bool can_launch;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_show_popup;
	bool can_change_focus;
};

/* Pure fail-closed testable policy. No scene graph, wl_client, IPC or spawn. */
struct stage_director_company_map_result
stage_director_company_map_probe(
	const struct stage_director_company_map_lease *lease,
	const struct stage_director_company_map_observed *observed,
	const struct stage_director_company_map_report *report);

const char *stage_director_company_map_status_name(
	enum stage_director_company_map_status status);

#endif
