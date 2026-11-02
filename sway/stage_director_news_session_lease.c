#include <string.h>
#include "sway/stage_director_news_launch_plan.h"
#include "sway/stage_director_news_session_lease.h"

/*
 * Compositor-local candidate lifecycle. This translation unit does not
 * derive authenticated WATCH intent from a QML boolean, Wayland namespace,
 * same-UID IPC peer, window title, launch activation token or guessed PID.
 * Even a successful root/client/surface match is NEVER an authority grant.
 */
static void clear_lease(struct stage_director_news_session_lease *lease) {
	const uint64_t floor = lease->last_epoch;
	memset(lease, 0, sizeof(*lease));
	lease->state = SD_NEWS_LEASE_EMPTY;
	lease->last_epoch = floor;
}

void stage_director_news_session_lease_init(
		struct stage_director_news_session_lease *lease) {
	if (!lease) return;
	memset(lease, 0, sizeof(*lease));
	lease->state = SD_NEWS_LEASE_EMPTY;
}

bool stage_director_news_session_lease_reserve_candidate(
		struct stage_director_news_session_lease *lease,
		uint64_t epoch, uintptr_t native_host_ref,
		uintptr_t native_host_client_ref, uintptr_t native_output_ref,
		const char *registered_source_id) {
	if (!lease || lease->state != SD_NEWS_LEASE_EMPTY ||
			epoch == 0 || epoch <= lease->last_epoch ||
			!native_host_ref || !native_host_client_ref ||
			!native_output_ref || !registered_source_id) {
		return false;
	}
	/* Fixed registry, never an arbitrary URL/command or user-supplied argv. */
	const size_t len = strlen(registered_source_id);
	if (len == 0 || len > 40 ||
			!stage_director_news_source_url(registered_source_id)) {
		return false;
	}
	lease->state = SD_NEWS_LEASE_HOST_CANDIDATE;
	lease->last_epoch = epoch;
	lease->epoch = epoch;
	lease->native_host_ref = native_host_ref;
	lease->native_host_client_ref = native_host_client_ref;
	lease->native_output_ref = native_output_ref;
	memcpy(lease->source_id, registered_source_id, len + 1);
	return true;
}

bool stage_director_news_session_lease_record_process_candidate(
		struct stage_director_news_session_lease *lease,
		uint64_t epoch, const struct stage_director_process_launch_root *root) {
	if (!lease || !root ||
			lease->state != SD_NEWS_LEASE_HOST_CANDIDATE ||
			epoch == 0 || lease->epoch != epoch ||
			root->pid <= 0 || root->start_time_ticks == 0 ||
			root->uid == 0) {
		return false;
	}
	lease->launch_root = *root;
	lease->state = SD_NEWS_LEASE_PROCESS_CANDIDATE;
	return true;
}

static bool same_native(const struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *o) {
	return lease && o && o->epoch != 0 && o->epoch == lease->epoch &&
		o->native_host_ref != 0 &&
		o->native_host_ref == lease->native_host_ref &&
		o->native_host_client_ref != 0 &&
		o->native_host_client_ref == lease->native_host_client_ref &&
		o->native_output_ref != 0 &&
		o->native_output_ref == lease->native_output_ref &&
		o->native_actor_ref != 0 &&
		o->native_actor_ref != o->native_host_ref &&
		o->native_actor_client_ref != 0 &&
		o->native_actor_client_ref != o->native_host_client_ref &&
		o->native_root_surface_ref != 0 &&
		o->native_root_surface_ref != o->native_actor_ref &&
		o->native_root_surface_ref != o->native_host_ref &&
		o->exact_xdg_toplevel_root &&
		o->native_wayland_client_pid > 0 &&
		o->native_wayland_client_start != 0;
}

static bool stable_lineage(
		const struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *o,
		stage_director_process_reader reader, void *context) {
	if (!reader) return false;
	const struct stage_director_process_evidence_result p =
		stage_director_process_check(&lease->launch_root,
			o->native_wayland_client_pid, o->native_wayland_client_start,
			reader, context);
	return p.state == SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE &&
		p.lineage_candidate && !p.owner_verified && !p.can_bind_viewer &&
		!p.can_launch && !p.can_clip && !p.can_accept_input;
}

bool stage_director_news_session_lease_record_actor_candidate(
		struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *o,
		stage_director_process_reader reader, void *context) {
	if (!lease || lease->state != SD_NEWS_LEASE_PROCESS_CANDIDATE ||
			!same_native(lease, o) ||
			!stable_lineage(lease, o, reader, context)) {
		return false;
	}
	lease->native_actor_ref = o->native_actor_ref;
	lease->native_actor_client_ref = o->native_actor_client_ref;
	lease->native_root_surface_ref = o->native_root_surface_ref;
	lease->actor_pid = o->native_wayland_client_pid;
	lease->actor_start = o->native_wayland_client_start;
	lease->state = SD_NEWS_LEASE_ACTOR_CANDIDATE;
	return true;
}

static struct stage_director_news_lease_probe result(
		enum stage_director_news_lease_probe_state state) {
	return (struct stage_director_news_lease_probe) {
		.state = state,
		.candidate_correlated = state == SD_NEWS_LEASE_PROBE_MATCHED_CANDIDATE_ONLY,
		.operator_watch_authenticated = false,
		.browser_launch_authorized = false,
		.owner_verified = false,
		.can_launch = false,
		.can_bind_viewer = false,
		.can_clip = false,
		.can_render = false,
		.can_accept_input = false,
		.can_show_popup = false,
	};
}

struct stage_director_news_lease_probe stage_director_news_session_lease_probe(
		const struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *o,
		stage_director_process_reader reader, void *context) {
	if (!lease || lease->state == SD_NEWS_LEASE_EMPTY) {
		return result(SD_NEWS_LEASE_PROBE_NO_SESSION);
	}
	if (lease->state != SD_NEWS_LEASE_ACTOR_CANDIDATE) {
		return result(SD_NEWS_LEASE_PROBE_NOT_BOUND);
	}
	if (!same_native(lease, o) ||
			o->native_actor_ref != lease->native_actor_ref ||
			o->native_actor_client_ref != lease->native_actor_client_ref ||
			o->native_root_surface_ref != lease->native_root_surface_ref ||
			o->native_wayland_client_pid != lease->actor_pid ||
			o->native_wayland_client_start != lease->actor_start) {
		return result(SD_NEWS_LEASE_PROBE_MISMATCH);
	}
	if (!stable_lineage(lease, o, reader, context)) {
		return result(SD_NEWS_LEASE_PROBE_PROCESS_UNSTABLE);
	}
	return result(SD_NEWS_LEASE_PROBE_MATCHED_CANDIDATE_ONLY);
}

bool stage_director_news_session_lease_close(
		struct stage_director_news_session_lease *lease, uint64_t epoch) {
	if (!lease || lease->state == SD_NEWS_LEASE_EMPTY ||
			epoch == 0 || epoch != lease->epoch) {
		return false;
	}
	clear_lease(lease);
	return true;
}

bool stage_director_news_session_lease_host_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_host_ref) {
	if (!lease || lease->state == SD_NEWS_LEASE_EMPTY ||
			native_host_ref == 0 || native_host_ref != lease->native_host_ref) {
		return false;
	}
	clear_lease(lease);
	return true;
}

bool stage_director_news_session_lease_actor_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_actor_ref) {
	if (!lease || lease->state != SD_NEWS_LEASE_ACTOR_CANDIDATE ||
			native_actor_ref == 0 || native_actor_ref != lease->native_actor_ref) {
		return false;
	}
	clear_lease(lease);
	return true;
}

bool stage_director_news_session_lease_output_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_output_ref) {
	if (!lease || lease->state == SD_NEWS_LEASE_EMPTY ||
			native_output_ref == 0 || native_output_ref != lease->native_output_ref) {
		return false;
	}
	clear_lease(lease);
	return true;
}

bool stage_director_news_session_lease_process_lost(
		struct stage_director_news_session_lease *lease, int64_t pid) {
	if (!lease || lease->state == SD_NEWS_LEASE_EMPTY || pid <= 0 ||
			(pid != lease->launch_root.pid && pid != lease->actor_pid)) {
		return false;
	}
	clear_lease(lease);
	return true;
}
