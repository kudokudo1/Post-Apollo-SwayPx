#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "ci_social_group_geometry.h"

static struct sd_ci_social_group_observation fixture(void) {
	return (struct sd_ci_social_group_observation){
		.lease_epoch = 8, .observed_epoch = 8,
		.lease_chassis = 101, .observed_chassis = 101,
		.lease_guest = 202, .observed_guest = 202,
		.lease_output = 303, .observed_output = 303,
		.chassis_mapped = true, .guest_mapped = true,
		.chassis_verified = true, .guest_verified = true,
		.guest_is_main = true, .initially_docked = true,
		.output_active = true,
		.leader = SD_CI_SOCIAL_LEADER_CHASSIS,
		.chassis = {100, 50, 1210, 700},
		.guest = {211, 51, 1098, 698},
		.output = {0, 0, 2560, 1440},
		.selector_width = 110, .gap = 0, .inset = 1,
		.minimum_shared_width = 311, .minimum_shared_height = 260,
	};
}

static void geometry_both_directions(void) {
	struct sd_ci_social_group_observation r = fixture();
	struct sd_ci_social_group_plan p = sd_ci_social_group_plan(&r);
	assert(p.state == SD_CI_SOCIAL_GEOMETRY_ONLY);
	assert(p.chassis.x == 100 && p.chassis.y == 50);
	assert(p.guest.x == 211 && p.guest.y == 51);
	assert(p.guest.width == 1098 && p.guest.height == 698);
	assert(!p.can_apply_to_compositor && !p.can_focus && !p.can_accept_input);

	/* Chassis move and resize produce ONE logical pair plan. */
	r.chassis = (struct stage_director_rect){425, 180, 1000, 550};
	p = sd_ci_social_group_plan(&r);
	assert(p.state == SD_CI_SOCIAL_GEOMETRY_ONLY);
	assert(p.guest.x == 536 && p.guest.y == 181);
	assert(p.guest.width == 888 && p.guest.height == 548);

	/* After initial docking, focused main guest may lead the same plan. */
	r = fixture();
	r.leader = SD_CI_SOCIAL_LEADER_GUEST;
	r.guest = (struct stage_director_rect){480, 180, 888, 548};
	p = sd_ci_social_group_plan(&r);
	assert(p.state == SD_CI_SOCIAL_GEOMETRY_ONLY);
	assert(p.chassis.x == 369 && p.chassis.y == 179);
	assert(p.chassis.width == 1000 && p.chassis.height == 550);

	/* On a left-side output, negative global coordinates are legitimate. */
	r = fixture();
	r.output = (struct stage_director_rect){-1500, 0, 1500, 1000};
	r.chassis = (struct stage_director_rect){-1400, 50, 1000, 650};
	p = sd_ci_social_group_plan(&r);
	assert(p.state == SD_CI_SOCIAL_GEOMETRY_ONLY);
	assert(p.guest.x == -1289 && p.guest.width == 888);
}

static void fail_closed_without_leaking_old_rectangles(void) {
	struct sd_ci_social_group_observation r = fixture();
	struct sd_ci_social_group_plan p;
	p = sd_ci_social_group_plan(NULL);
	assert(p.state == SD_CI_SOCIAL_NO_PAIR && p.chassis.width == 0);

	r.observed_epoch++;
	p = sd_ci_social_group_plan(&r);
	assert(p.state == SD_CI_SOCIAL_STALE_PAIR && p.guest.width == 0);

	r = fixture(); r.observed_guest++;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_STALE_PAIR);
	r = fixture(); r.observed_output++;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_STALE_PAIR);
	r = fixture(); r.guest_is_main = false;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_UNVERIFIED);
	r = fixture(); r.guest_mapped = false;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_UNVERIFIED);
	r = fixture(); r.chassis_verified = false;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_UNVERIFIED);
	r = fixture(); r.initially_docked = false;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_UNDOCKED);
	r = fixture(); r.leader = SD_CI_SOCIAL_LEADER_NONE;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.selector_width = 0;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.inset = -1;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.chassis.width = 100;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.leader = SD_CI_SOCIAL_LEADER_GUEST;
	r.guest.width = 10;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.leader = SD_CI_SOCIAL_LEADER_GUEST;
	r.guest.x = INT_MIN;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.chassis.x = INT_MAX - 100;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_INVALID_GEOMETRY);
	r = fixture(); r.output.width = 500;
	assert(sd_ci_social_group_plan(&r).state == SD_CI_SOCIAL_OUTSIDE_OUTPUT);
}

int main(void) {
	geometry_both_directions();
	fail_closed_without_leaking_old_rectangles();
	puts("Social W Stage Director CI-only grouped geometry: PASS");
	return 0;
}
