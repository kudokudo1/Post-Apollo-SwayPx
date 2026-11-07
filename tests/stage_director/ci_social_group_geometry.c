#include <limits.h>
#include <stdint.h>
#include "ci_social_group_geometry.h"

/* All calculations are widened before any narrowing back to Sway int. */
static bool narrow_int(int64_t value, int *out) {
	if (!out || value < INT_MIN || value > INT_MAX) return false;
	*out = (int)value;
	return true;
}

struct sd_ci_social_group_plan sd_ci_social_group_plan(
		const struct sd_ci_social_group_observation *r) {
	struct sd_ci_social_group_plan p = {
		.state = SD_CI_SOCIAL_NO_PAIR,
	};
	if (!r || !r->lease_epoch || !r->lease_chassis ||
			!r->lease_guest || !r->lease_output ||
			r->lease_chassis == r->lease_guest) return p;
	if (r->lease_epoch != r->observed_epoch ||
			r->lease_chassis != r->observed_chassis ||
			r->lease_guest != r->observed_guest ||
			r->lease_output != r->observed_output) {
		p.state = SD_CI_SOCIAL_STALE_PAIR;
		return p;
	}
	if (!r->chassis_mapped || !r->guest_mapped ||
			!r->chassis_verified || !r->guest_verified ||
			!r->guest_is_main || !r->output_active) {
		p.state = SD_CI_SOCIAL_UNVERIFIED;
		return p;
	}
	if (!r->initially_docked) {
		p.state = SD_CI_SOCIAL_UNDOCKED;
		return p;
	}
	if (r->leader != SD_CI_SOCIAL_LEADER_CHASSIS &&
			r->leader != SD_CI_SOCIAL_LEADER_GUEST) {
		p.state = SD_CI_SOCIAL_INVALID_GEOMETRY;
		return p;
	}
	if (!stage_director_rect_valid(r->chassis) ||
			!stage_director_rect_valid(r->guest) ||
			!stage_director_rect_valid(r->output) ||
			r->selector_width <= 0 || r->gap < 0 ||
			r->inset < 0 || r->minimum_shared_width <= 0 ||
			r->minimum_shared_height <= 0) {
		p.state = SD_CI_SOCIAL_INVALID_GEOMETRY;
		return p;
	}
	const int64_t left = (int64_t)r->selector_width + r->gap + r->inset;
	const int64_t borders = (int64_t)r->inset * 2;
	struct stage_director_rect chassis = {0}, guest = {0};
	bool valid = true;
	if (r->leader == SD_CI_SOCIAL_LEADER_CHASSIS) {
		chassis = r->chassis;
		valid = narrow_int((int64_t)chassis.x + left, &guest.x) &&
			narrow_int((int64_t)chassis.y + r->inset, &guest.y) &&
			narrow_int((int64_t)chassis.width - left - r->inset,
				&guest.width) &&
			narrow_int((int64_t)chassis.height - borders, &guest.height);
	} else {
		guest = r->guest;
		valid = narrow_int((int64_t)guest.x - left, &chassis.x) &&
			narrow_int((int64_t)guest.y - r->inset, &chassis.y) &&
			narrow_int((int64_t)guest.width + left + r->inset,
				&chassis.width) &&
			narrow_int((int64_t)guest.height + borders, &chassis.height);
	}
	if (!valid || !stage_director_rect_valid(chassis) ||
			!stage_director_rect_valid(guest) ||
			chassis.width < r->minimum_shared_width ||
			chassis.height < r->minimum_shared_height ||
			guest.width <= 0 || guest.height <= 0 ||
			!stage_director_rect_contains(chassis, guest)) {
		p.state = SD_CI_SOCIAL_INVALID_GEOMETRY;
		return p;
	}
	if (!stage_director_rect_contains(r->output, chassis) ||
			!stage_director_rect_contains(r->output, guest)) {
		p.state = SD_CI_SOCIAL_OUTSIDE_OUTPUT;
		return p;
	}
	p.chassis = chassis;
	p.guest = guest;
	p.state = SD_CI_SOCIAL_GEOMETRY_ONLY;
	/* The caller must never treat successful arithmetic as authority. */
	return p;
}
