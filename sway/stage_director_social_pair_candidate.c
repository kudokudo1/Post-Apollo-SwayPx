#include <string.h>
#include "sway/stage_director_social_pair_candidate.h"

/*
 * Compositor-owned candidate bookkeeping only. Source fields come from
 * a *trusted native callback* in the eventual Stage Director integration.
 * This module does not validate the origin of an external request and
 * cannot authorize scene, cursor, input or compositor geometry actuation.
 */
static bool valid(const struct sd_social_native_member *m,
		enum sd_social_member_role role) {
	return m && m->role == role &&
		m->container_ref && m->client_ref && m->root_surface_ref &&
		m->output_ref && m->workspace_ref && m->seat_ref &&
		m->mapped_generation != 0 && m->peer_pid > 0 &&
		m->peer_start_ticks != 0 && m->peer_uid != 0 &&
		m->mapped && m->exact_role &&
		(role != SD_SOCIAL_GUEST_XDG || m->floating);
}

static bool distinct_same_space(const struct sd_social_native_member *host,
		const struct sd_social_native_member *guest) {
	return host->container_ref != guest->container_ref &&
		host->client_ref != guest->client_ref &&
		host->root_surface_ref != guest->root_surface_ref &&
		host->output_ref == guest->output_ref &&
		host->workspace_ref == guest->workspace_ref &&
		host->seat_ref == guest->seat_ref &&
		host->peer_uid == guest->peer_uid;
}

static bool same_member(const struct sd_social_native_member *a,
		const struct sd_social_native_member *b) {
	return a && b &&
		a->role == b->role &&
		a->container_ref == b->container_ref &&
		a->client_ref == b->client_ref &&
		a->root_surface_ref == b->root_surface_ref &&
		a->output_ref == b->output_ref &&
		a->workspace_ref == b->workspace_ref &&
		a->seat_ref == b->seat_ref &&
		a->mapped_generation == b->mapped_generation &&
		a->peer_pid == b->peer_pid &&
		a->peer_start_ticks == b->peer_start_ticks &&
		a->peer_uid == b->peer_uid &&
		a->mapped == b->mapped &&
		a->exact_role == b->exact_role &&
		a->floating == b->floating;
}

static void clear_candidate(struct sd_social_pair_candidate *p) {
	const uint64_t floor = p->generation_floor;
	memset(p, 0, sizeof(*p));
	p->state = SD_SOCIAL_CANDIDATE_EMPTY;
	p->generation_floor = floor;
}

void sd_social_pair_candidate_init(struct sd_social_pair_candidate *p) {
	if (!p) return;
	memset(p, 0, sizeof(*p));
	p->state = SD_SOCIAL_CANDIDATE_EMPTY;
}

uint64_t sd_social_pair_candidate_host(
		struct sd_social_pair_candidate *p,
		const struct sd_social_native_member *host) {
	if (!p || p->state != SD_SOCIAL_CANDIDATE_EMPTY ||
			p->generation_floor == UINT64_MAX ||
			!valid(host, SD_SOCIAL_HOST_LAYER)) {
		return 0;
	}
	const uint64_t generation = p->generation_floor + 1;
	p->generation_floor = generation;
	p->generation = generation;
	p->host = *host;
	p->state = SD_SOCIAL_CANDIDATE_HOST;
	return generation;
}

bool sd_social_pair_candidate_guest(
		struct sd_social_pair_candidate *p, uint64_t generation,
		const struct sd_social_native_member *guest) {
	if (!p || !generation || generation != p->generation ||
			p->state != SD_SOCIAL_CANDIDATE_HOST ||
			!valid(&p->host, SD_SOCIAL_HOST_LAYER) ||
			!valid(guest, SD_SOCIAL_GUEST_XDG) ||
			!distinct_same_space(&p->host, guest)) {
		return false;
	}
	p->guest = *guest;
	p->state = SD_SOCIAL_CANDIDATE_PAIR;
	return true;
}

static struct sd_social_candidate_probe probe(
		enum sd_social_candidate_probe_state state) {
	return (struct sd_social_candidate_probe) {
		.state = state,
		.candidate_correlated =
			state == SD_SOCIAL_PROBE_CORRELATED_CANDIDATE,
		/* No privilege may be inferred from a candidate match. */
		.authenticated_pairing = false,
		.can_move = false,
		.can_resize = false,
		.can_focus = false,
		.can_clip = false,
		.can_render = false,
		.can_input = false,
		.can_popup = false,
	};
}

struct sd_social_candidate_probe sd_social_pair_candidate_probe(
		const struct sd_social_pair_candidate *p,
		uint64_t generation,
		const struct sd_social_native_member *host,
		const struct sd_social_native_member *guest) {
	if (!p || p->state == SD_SOCIAL_CANDIDATE_EMPTY)
		return probe(SD_SOCIAL_PROBE_EMPTY);
	if (p->state == SD_SOCIAL_CANDIDATE_HOST)
		return probe(SD_SOCIAL_PROBE_HOST_ONLY);
	if (p->state != SD_SOCIAL_CANDIDATE_PAIR || !generation ||
			generation != p->generation ||
			!valid(host, SD_SOCIAL_HOST_LAYER) ||
			!valid(guest, SD_SOCIAL_GUEST_XDG) ||
			!same_member(host, &p->host) ||
			!same_member(guest, &p->guest) ||
			!distinct_same_space(host, guest))
		return probe(SD_SOCIAL_PROBE_MISMATCH);
	return probe(SD_SOCIAL_PROBE_CORRELATED_CANDIDATE);
}

bool sd_social_pair_candidate_revoke(
		struct sd_social_pair_candidate *p, uint64_t generation,
		enum sd_social_candidate_lost event, uintptr_t ref) {
	if (!p || p->state == SD_SOCIAL_CANDIDATE_EMPTY ||
			!generation || generation != p->generation)
		return false;
	bool match = false;
	switch (event) {
	case SD_SOCIAL_HOST_UNMAPPED:
		match = ref && ref == p->host.root_surface_ref;
		break;
	case SD_SOCIAL_GUEST_UNMAPPED:
		match = p->state == SD_SOCIAL_CANDIDATE_PAIR && ref &&
			ref == p->guest.root_surface_ref;
		break;
	case SD_SOCIAL_HOST_CLIENT_LOST:
		match = ref && ref == p->host.client_ref;
		break;
	case SD_SOCIAL_GUEST_CLIENT_LOST:
		match = p->state == SD_SOCIAL_CANDIDATE_PAIR && ref &&
			ref == p->guest.client_ref;
		break;
	case SD_SOCIAL_OUTPUT_LOST:
		match = ref && ref == p->host.output_ref;
		break;
	case SD_SOCIAL_WORKSPACE_LOST:
		match = ref && ref == p->host.workspace_ref;
		break;
	case SD_SOCIAL_SEAT_LOST:
		match = ref && ref == p->host.seat_ref;
		break;
	case SD_SOCIAL_OPERATOR_CLOSED:
		/* Caller must be the compositor's already-authenticated private
		 * close handler. There is no client-exposed "close" function. */
		match = ref && ref == p->host.client_ref;
		break;
	default:
		break;
	}
	if (!match) return false;
	clear_candidate(p);
	return true;
}
