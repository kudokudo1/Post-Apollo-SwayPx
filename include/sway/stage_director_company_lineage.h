#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_LINEAGE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_LINEAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_process_evidence.h"

/*
 * M22 // read-only Linux parentage evidence for CompanyVectorProbe.
 *
 * The Qt process is launched by a Quickshell Process through Python execv
 * wrappers. execv preserves Linux PID and process start-time ticks. Verify
 * that the native XDG client is currently a *direct child* of the exact
 * mapped Company host wl_client, not simply some process with a matching
 * app_id or a common UID. Parentage can still be insufficient for consent
 * because Quickshell launches unrelated tools too.
 *
 * No trusted-launch-root issuer exists here. This is not an authenticated
 * ownership lease. Never convert this evidence into M19.compositor_selected.
 */
enum stage_director_company_lineage_status {
	SD_COMPANY_LINEAGE_INVALID,
	SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER,
	SD_COMPANY_LINEAGE_READ_FAILED,
	SD_COMPANY_LINEAGE_PROCESS_CHANGED,
	SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD,
	SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
};

struct stage_director_company_lineage_result {
	enum stage_director_company_lineage_status status;
	bool direct_child_candidate;
	bool owner_verified;
	bool can_launch;
	bool can_pair_actor;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_focus;
};

struct stage_director_company_lineage_result
stage_director_company_lineage_probe(
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor,
	stage_director_process_reader reader, void *context);

/* Actual /proc-backed adapter for a future compositor-owned observer,
 * no mutation and no process control. */
struct stage_director_company_lineage_result
stage_director_company_lineage_probe_linux(
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor);

const char *stage_director_company_lineage_status_name(
	enum stage_director_company_lineage_status status);

#endif
