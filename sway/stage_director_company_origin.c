#include "sway/stage_director_company_origin.h"

static struct stage_director_company_origin_result result(
		enum stage_director_company_origin_status status) {
	return (struct stage_director_company_origin_result) {
		.status = status,
		.intent_correlated_candidate =
			status == SD_COMPANY_ORIGIN_CANDIDATE_ONLY,
		/* All authorization remains false on every path. */
	};
}

struct stage_director_company_origin_result
stage_director_company_origin_check(
		const struct stage_director_company_intent_snapshot *intent,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor,
		const struct stage_director_company_lineage_result *lineage,
		uint64_t ticks_per_second) {
	if (!intent || !intent->present || !intent->selection_epoch ||
			!intent->sequence || !intent->observed_boot_ticks ||
			!intent->host_revision || intent->mode > 1 ||
			!ticks_per_second || ticks_per_second > 1000000) {
		return result(SD_COMPANY_ORIGIN_NO_INTENT);
	}
	if (!host || !host->candidate ||
			host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!host->native_layer_ref || !host->native_layer_role_ref ||
			!host->native_surface_ref || !host->native_client_ref ||
			!host->native_output_ref ||
			intent->native_host_client_ref != host->native_client_ref ||
			host->pid <= 0 || host->uid == 0 ||
			host->start_ticks == 0) {
		return result(SD_COMPANY_ORIGIN_WRONG_HOST);
	}
	if (!actor || !actor->candidate ||
			actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!actor->native_view_ref || !actor->native_root_surface_ref ||
			!actor->native_client_ref ||
			actor->native_client_ref == host->native_client_ref ||
			actor->pid <= 0 || actor->pid == host->pid ||
			actor->uid != host->uid || !actor->start_ticks) {
		return result(SD_COMPANY_ORIGIN_WRONG_ACTOR);
	}
	if (!lineage ||
			lineage->status != SD_COMPANY_LINEAGE_CANDIDATE_ONLY ||
			!lineage->direct_child_candidate ||
			lineage->owner_verified || lineage->can_launch ||
			lineage->can_pair_actor || lineage->can_render ||
			lineage->can_clip || lineage->can_accept_input ||
			lineage->can_focus) {
		return result(SD_COMPANY_ORIGIN_NO_LINEAGE);
	}
	if (actor->start_ticks < host->start_ticks) {
		return result(SD_COMPANY_ORIGIN_STALE_PROCESS);
	}
	/* The existing M26 control emits intent immediately after Quickshell
	 * accepts starting the attached child. It can arrive up to 2s AFTER
	 * birth due to scheduling; a mapped Qt XDG actor may appear up to 5s
	 * AFTER intent. Refuse old/manual Qt windows and very late windows.
	 *
	 * This temporal bound reduces ambiguity but is NOT process custody.
	 * It does not prove the actor ran the authorized M15 executable.
	 */
	const uint64_t delta = actor->start_ticks >=
			intent->observed_boot_ticks ?
		actor->start_ticks - intent->observed_boot_ticks :
		intent->observed_boot_ticks - actor->start_ticks;
	const uint64_t limit = actor->start_ticks >=
			intent->observed_boot_ticks ?
		5 * ticks_per_second : 2 * ticks_per_second;
	if (delta > limit) {
		return result(SD_COMPANY_ORIGIN_BIRTH_OUTSIDE_WINDOW);
	}
	return result(SD_COMPANY_ORIGIN_CANDIDATE_ONLY);
}

const char *stage_director_company_origin_status_name(
		enum stage_director_company_origin_status status) {
	switch (status) {
	case SD_COMPANY_ORIGIN_NO_INTENT: return "NO_INTENT";
	case SD_COMPANY_ORIGIN_WRONG_HOST: return "WRONG_HOST";
	case SD_COMPANY_ORIGIN_WRONG_ACTOR: return "WRONG_ACTOR";
	case SD_COMPANY_ORIGIN_NO_LINEAGE: return "NO_LINEAGE";
	case SD_COMPANY_ORIGIN_STALE_PROCESS: return "STALE_PROCESS";
	case SD_COMPANY_ORIGIN_BIRTH_OUTSIDE_WINDOW: return "BIRTH_OUTSIDE_WINDOW";
	case SD_COMPANY_ORIGIN_CANDIDATE_ONLY: return "CANDIDATE_ONLY";
	}
	return "DENIED_UNKNOWN";
}
