#include <stdint.h>
#include "sway/stage_director_company_lineage.h"

/*
 * Kernel process observations plus exact M20 Wayland snapshots are evidence,
 * not an authorization signal. The actor's *direct* parent must be the
 * currently mapped Company host client. No app-id/title/path trust.
 */
static struct stage_director_company_lineage_result result(
		enum stage_director_company_lineage_status state) {
	return (struct stage_director_company_lineage_result) {
		.status = state,
		.direct_child_candidate =
			state == SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
		/* All authority bits remain false. */
	};
}

static bool same_process(struct stage_director_process_identity a,
		struct stage_director_process_identity b) {
	return a.pid == b.pid && a.parent_pid == b.parent_pid &&
		a.start_time_ticks == b.start_time_ticks && a.uid == b.uid;
}

struct stage_director_company_lineage_result
stage_director_company_lineage_probe(
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor,
		stage_director_process_reader reader, void *context) {
	if (!host || !actor || !reader) {
		return result(SD_COMPANY_LINEAGE_INVALID);
	}
	if (!host->candidate || !actor->candidate ||
			host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!host->native_layer_ref || !host->native_layer_role_ref ||
			!host->native_surface_ref || !host->native_client_ref ||
			!host->native_output_ref || !actor->native_view_ref ||
			!actor->native_root_surface_ref || !actor->native_client_ref ||
			host->native_client_ref == actor->native_client_ref ||
			host->pid <= 0 || actor->pid <= 0 ||
			host->pid == actor->pid || !host->uid ||
			host->uid != actor->uid || !host->start_ticks ||
			!actor->start_ticks) {
		return result(SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	}
	struct stage_director_process_identity parent = {0};
	struct stage_director_process_identity child = {0};
	if (!reader(host->pid, &parent, context) ||
			!reader(actor->pid, &child, context)) {
		return result(SD_COMPANY_LINEAGE_READ_FAILED);
	}
	if (parent.pid != host->pid ||
			parent.uid != host->uid ||
			parent.start_time_ticks != host->start_ticks ||
			child.pid != actor->pid ||
			child.uid != actor->uid ||
			child.start_time_ticks != actor->start_ticks) {
		return result(SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	}
	if (child.parent_pid != parent.pid ||
			child.start_time_ticks < parent.start_time_ticks) {
		return result(SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD);
	}
	/*
	 * Reuse the existing process ancestry validator. It re-reads each
	 * process and refuses reparenting, PID reuse, changes in UID, or an
	 * unstable parent chain. The root is the Wayland host process itself,
	 * not a caller-provided PID string.
	 */
	const struct stage_director_process_launch_root root = {
		.pid = parent.pid,
		.start_time_ticks = parent.start_time_ticks,
		.uid = parent.uid,
	};
	const struct stage_director_process_evidence_result lineage =
		stage_director_process_check(
			&root, child.pid, child.start_time_ticks, reader, context);
	if (lineage.state != SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE ||
			!lineage.lineage_candidate || lineage.owner_verified ||
			lineage.can_launch || lineage.can_bind_viewer ||
			lineage.can_clip || lineage.can_accept_input) {
		return result(lineage.state == SD_PROCESS_EVIDENCE_READ_FAILED
			? SD_COMPANY_LINEAGE_READ_FAILED
			: SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	}
	/* Last double-observation after the ancestry pass. */
	struct stage_director_process_identity last_parent = {0};
	struct stage_director_process_identity last_child = {0};
	if (!reader(host->pid, &last_parent, context) ||
			!reader(actor->pid, &last_child, context) ||
			!same_process(parent, last_parent) ||
			!same_process(child, last_child)) {
		return result(SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	}
	return result(SD_COMPANY_LINEAGE_CANDIDATE_ONLY);
}

struct stage_director_company_lineage_result
stage_director_company_lineage_probe_linux(
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor) {
	return stage_director_company_lineage_probe(host, actor,
		stage_director_read_linux_process, NULL);
}

const char *stage_director_company_lineage_status_name(
		enum stage_director_company_lineage_status status) {
	switch (status) {
	case SD_COMPANY_LINEAGE_INVALID: return "INVALID";
	case SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER: return "WRONG_NATIVE_PEER";
	case SD_COMPANY_LINEAGE_READ_FAILED: return "READ_FAILED";
	case SD_COMPANY_LINEAGE_PROCESS_CHANGED: return "PROCESS_CHANGED";
	case SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD: return "NOT_DIRECT_CHILD";
	case SD_COMPANY_LINEAGE_CANDIDATE_ONLY: return "CANDIDATE_ONLY";
	}
	return "INVALID";
}
