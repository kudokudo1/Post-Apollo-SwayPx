#ifndef _SWAY_STAGE_DIRECTOR_VIEWPORT_H
#define _SWAY_STAGE_DIRECTOR_VIEWPORT_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Stage Director SD1 V0 // Scene-independent viewport safety contract.
 *
 * Geometry is expressed in compositor-global *logical* coordinates. This
 * component NEVER mutates the scene graph, changes focus, starts a browser,
 * or claims that a true clipping/input transform exists.
 *
 * The SD0 registry supplies observed actors/layers. A future SD1 renderer
 * must separately prove its clip, input and lifecycle authority before using
 * a geometrically valid plan. Strictly separate preflight from activation.
 */
struct stage_director_rect {
	int x;
	int y;
	int width;
	int height;
};

struct stage_director_viewport_request {
	struct stage_director_rect host;
	struct stage_director_rect output;
	struct stage_director_rect actor;
	struct stage_director_rect viewport;
	bool host_mapped;
	bool actor_mapped;
	bool output_active;
	bool actor_identity_verified;
};

enum stage_director_viewport_result {
	SD_VIEWPORT_DENY_UNVERIFIED_ACTOR,
	SD_VIEWPORT_DENY_UNMAPPED,
	SD_VIEWPORT_DENY_OUTPUT_INACTIVE,
	SD_VIEWPORT_DENY_INVALID_GEOMETRY,
	SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_HOST,
	SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_OUTPUT,
	SD_VIEWPORT_DENY_ACTOR_NOT_COVERING_VIEWPORT,
	SD_VIEWPORT_GEOMETRY_ONLY,
};

struct stage_director_viewport_plan {
	enum stage_director_viewport_result result;
	/* Crop in actor-local logical coordinates. Not yet applied to compositor. */
	struct stage_director_rect actor_local_crop;
	bool geometry_valid;
	bool can_launch;
	bool can_render;
	bool can_accept_input;
};

/*
 * V1 // clip authorization is a SEPARATE contract from geometry validity.
 * All gates default to false. Nothing in SD0 currently mints an approved
 * authority, so this cannot activate through the present production hooks.
 *
 * The future compositor session owner must bind a specific mapped layer host
 * to a specific browser process and install both popup and pointer rejection
 * BEFORE setting these gates. An app_id by itself is not ownership evidence.
 */
struct stage_director_viewport_authority {
	bool operator_enabled;
	bool host_session_verified;
	bool actor_process_verified;
	bool input_hit_test_guard_installed;
	bool popup_guard_installed;
	bool teardown_guard_installed;
};

struct stage_director_viewport_clip_plan {
	bool allowed;
	struct stage_director_rect root_surface_clip;
	enum stage_director_viewport_result geometry_result;
};

/*
 * Calculates the actual crop coordinates relative to the root wlr_surface:
 * the scene origin is at view->content_tree while xdg surface geometry may
 * have a nonzero x/y offset relative to that origin.
 */
struct stage_director_viewport_clip_plan stage_director_viewport_prepare_clip(
		const struct stage_director_viewport_request *request,
		const struct stage_director_viewport_authority *authority,
		int root_surface_geometry_x,
		int root_surface_geometry_y);

bool stage_director_rect_valid(struct stage_director_rect rect);
bool stage_director_rect_contains(struct stage_director_rect outer,
		struct stage_director_rect inner);
bool stage_director_rect_contains_point(struct stage_director_rect rect,
		int x, int y);
struct stage_director_viewport_plan stage_director_viewport_plan(
		const struct stage_director_viewport_request *request);
const char *stage_director_viewport_result_name(
		enum stage_director_viewport_result result);

/*
 * SD1 FRAME POLICY // Pure decision, not compositor actuation.
 *
 * PR #28 proved that output_configure_scene() resets a 320x240 viewport
 * destination at each frame unless a trusted owner participates at that
 * boundary. These structures describe what a future authenticated session
 * would have to supply. They do NOT mint a session or a trusted credential.
 *
 * Every uintptr_t must originate from live, native compositor objects.
 * NEVER populate from a window title, app_id, QML property or public IPC.
 * The caller must own lifecycle/teardown and recheck these fields per frame.
 */
struct stage_director_viewport_frame_owner {
	uint64_t epoch;
	uintptr_t native_host_ref;
	uintptr_t native_actor_ref;
	uintptr_t native_output_ref;
	uintptr_t native_root_surface_ref;
	struct stage_director_rect authorized_host;
	struct stage_director_rect authorized_output;
	struct stage_director_rect authorized_viewport;
	/* Existing guard contract, independently established elsewhere. */
	struct stage_director_viewport_authority authority;
};

struct stage_director_viewport_frame_observation {
	uint64_t epoch;
	uintptr_t native_host_ref;
	uintptr_t native_actor_ref;
	uintptr_t native_output_ref;
	uintptr_t native_root_surface_ref;
	/* Must be the root surface of the exact actor, never a popup/subsurface. */
	bool native_root_surface;
	struct stage_director_viewport_request geometry;
	int root_surface_geometry_x;
	int root_surface_geometry_y;
};

enum stage_director_viewport_frame_state {
	SD_FRAME_DEFAULT_NO_OWNER,
	SD_FRAME_DEFAULT_STALE_OWNER,
	SD_FRAME_DEFAULT_DIFFERENT_SURFACE,
	SD_FRAME_DEFAULT_STALE_GEOMETRY,
	SD_FRAME_DEFAULT_GUARDS_MISSING,
	SD_FRAME_PLAN_DESTINATION_ONLY,
};

struct stage_director_viewport_frame_decision {
	enum stage_director_viewport_frame_state state;
	/*
	 * All outputs are a PLAN for a future privileged compositor owner.
	 * This pure function does not install geometry, crop, input or popup
	 * guards. A rejected/expired owner requires default SwayPX sizing.
	 */
	bool proposed_override;
	struct stage_director_rect root_surface_crop;
	int proposed_dest_width;
	int proposed_dest_height;
};

struct stage_director_viewport_frame_decision
stage_director_viewport_frame_policy(
		const struct stage_director_viewport_frame_owner *owner,
		const struct stage_director_viewport_frame_observation *frame);

const char *stage_director_viewport_frame_state_name(
		enum stage_director_viewport_frame_state state);

#endif
