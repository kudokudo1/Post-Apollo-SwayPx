#ifndef _SWAY_STAGE_DIRECTOR_VIEWPORT_H
#define _SWAY_STAGE_DIRECTOR_VIEWPORT_H

#include <stdbool.h>

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

bool stage_director_rect_valid(struct stage_director_rect rect);
bool stage_director_rect_contains(struct stage_director_rect outer,
		struct stage_director_rect inner);
bool stage_director_rect_contains_point(struct stage_director_rect rect,
		int x, int y);
struct stage_director_viewport_plan stage_director_viewport_plan(
		const struct stage_director_viewport_request *request);
const char *stage_director_viewport_result_name(
		enum stage_director_viewport_result result);

#endif
