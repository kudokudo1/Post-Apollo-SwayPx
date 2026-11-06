#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_WIRE_POLICY_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_WIRE_POLICY_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/* Pure untrusted QML rectangle validation. This is NEVER scene/seat authority. */
enum stage_director_company_wire_state {
	SD_COMPANY_WIRE_DENIED = 0,
	SD_COMPANY_WIRE_LAUNCH_CANDIDATE = 1,
	SD_COMPANY_WIRE_VIEWPORT_CANDIDATE = 2,
	SD_COMPANY_WIRE_HIDDEN = 3,
	SD_COMPANY_WIRE_CLOSED = 4,
	SD_COMPANY_WIRE_INVALID_GEOMETRY = 5,
};

struct stage_director_company_wire_result {
	enum stage_director_company_wire_state state;
	struct stage_director_rect map_in_host;
	bool geometry_candidate;
	bool can_launch;
	bool can_render;
	bool can_clip;
	bool can_focus;
	bool can_accept_input;
	bool can_show_popup;
};

struct stage_director_company_wire_result
stage_director_company_wire_check(
	bool native_host_current, bool launch_intent_observed,
	struct stage_director_rect native_host_local,
	struct stage_director_rect frame_in_host,
	struct stage_director_rect map_in_frame);

#endif
