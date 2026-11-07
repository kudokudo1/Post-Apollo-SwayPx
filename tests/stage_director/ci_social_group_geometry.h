#ifndef _SD_CI_SOCIAL_GROUP_GEOMETRY_H
#define _SD_CI_SOCIAL_GROUP_GEOMETRY_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * CI-ONLY Social W group rectangle calculation.
 *
 * This does NOT act on Sway containers, change any compositor state, bind
 * untrusted Wayland surfaces, grant keyboard/pointer focus, or promise atomic
 * movement. An eventual compositor owner must prove native surface custody
 * and scene-frame transaction semantics before using any result.
 */
enum sd_ci_social_leader {
	SD_CI_SOCIAL_LEADER_NONE = 0,
	SD_CI_SOCIAL_LEADER_CHASSIS = 1,
	SD_CI_SOCIAL_LEADER_GUEST = 2,
};

enum sd_ci_social_state {
	SD_CI_SOCIAL_NO_PAIR,
	SD_CI_SOCIAL_STALE_PAIR,
	SD_CI_SOCIAL_UNVERIFIED,
	SD_CI_SOCIAL_UNDOCKED,
	SD_CI_SOCIAL_INVALID_GEOMETRY,
	SD_CI_SOCIAL_OUTSIDE_OUTPUT,
	SD_CI_SOCIAL_GEOMETRY_ONLY,
};

struct sd_ci_social_group_observation {
	uint64_t lease_epoch;
	uint64_t observed_epoch;
	uintptr_t lease_chassis;
	uintptr_t observed_chassis;
	uintptr_t lease_guest;
	uintptr_t observed_guest;
	uintptr_t lease_output;
	uintptr_t observed_output;
	bool chassis_mapped;
	bool guest_mapped;
	bool chassis_verified;
	bool guest_verified;
	bool guest_is_main;
	bool initially_docked;
	bool output_active;
	enum sd_ci_social_leader leader;
	struct stage_director_rect chassis;
	struct stage_director_rect guest;
	struct stage_director_rect output;
	int selector_width;
	int gap;
	int inset;
	int minimum_shared_width;
	int minimum_shared_height;
};

struct sd_ci_social_group_plan {
	enum sd_ci_social_state state;
	struct stage_director_rect chassis;
	struct stage_director_rect guest;
	/* Strictly geometry-only. No authority is minted by this prototype. */
	bool can_apply_to_compositor;
	bool can_focus;
	bool can_accept_input;
};

struct sd_ci_social_group_plan sd_ci_social_group_plan(
	const struct sd_ci_social_group_observation *observation);

#endif
