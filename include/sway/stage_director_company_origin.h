#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_ORIGIN_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_ORIGIN_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_company_lineage.h"

/*
 * M27 // compositor-private, read-only correlation between a fresh M23
 * launch intent and one M20-observed native Qt XDG process candidate.
 *
 * This does NOT authenticate exact M15 launcher birth. Another Quickshell
 * direct child could appear during the same window; user-supplied app_id
 * and namespace remain spoofable. Never use this to enable scene/seat.
 */
struct stage_director_company_intent_snapshot {
	bool present;
	uint64_t selection_epoch;
	uint64_t sequence;
	uint64_t observed_boot_ticks;
	uint64_t host_revision;
	uintptr_t native_host_client_ref;
	uint32_t mode; /* 0 offline, 1 online, never an arbitrary URL */
};

enum stage_director_company_origin_status {
	SD_COMPANY_ORIGIN_NO_INTENT,
	SD_COMPANY_ORIGIN_WRONG_HOST,
	SD_COMPANY_ORIGIN_WRONG_ACTOR,
	SD_COMPANY_ORIGIN_NO_LINEAGE,
	SD_COMPANY_ORIGIN_STALE_PROCESS,
	SD_COMPANY_ORIGIN_BIRTH_OUTSIDE_WINDOW,
	SD_COMPANY_ORIGIN_CANDIDATE_ONLY,
};

struct stage_director_company_origin_result {
	enum stage_director_company_origin_status status;
	bool intent_correlated_candidate;
	bool launch_authenticated;
	bool owner_verified;
	bool can_pair_actor;
	bool can_render;
	bool can_clip;
	bool can_focus;
	bool can_accept_input;
	bool can_show_popup;
};

/* Clock input is read from CLOCK_BOOTTIME in the private M23 server.
 * Actor start ticks originate only from the actual /proc identity in M20. */
struct stage_director_company_origin_result
stage_director_company_origin_check(
	const struct stage_director_company_intent_snapshot *intent,
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor,
	const struct stage_director_company_lineage_result *lineage,
	uint64_t ticks_per_second);

const char *stage_director_company_origin_status_name(
	enum stage_director_company_origin_status status);

#endif
