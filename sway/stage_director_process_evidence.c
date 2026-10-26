#include "sway/stage_director_process_evidence.h"

static bool same_identity(struct stage_director_process_identity a,
		struct stage_director_process_identity b) {
	return a.pid == b.pid &&
		a.parent_pid == b.parent_pid &&
		a.start_time_ticks == b.start_time_ticks &&
		a.uid == b.uid;
}

static struct stage_director_process_evidence_result result(
		enum stage_director_process_evidence_state state) {
	return (struct stage_director_process_evidence_result) {
		.state = state,
		.lineage_candidate = state == SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE,
		.owner_verified = false,
		.can_bind_viewer = false,
		.can_launch = false,
		.can_clip = false,
		.can_accept_input = false,
	};
}

struct stage_director_process_evidence_result stage_director_process_check(
		const struct stage_director_process_launch_root *launch_root,
		int64_t actor_pid, uint64_t observed_actor_start,
		stage_director_process_reader read_process, void *context) {
	if (!launch_root || !read_process || launch_root->pid <= 0 ||
			launch_root->start_time_ticks == 0 || actor_pid <= 0 ||
			observed_actor_start == 0) {
		return result(SD_PROCESS_EVIDENCE_INVALID);
	}

	struct stage_director_process_identity root = {0};
	if (!read_process(launch_root->pid, &root, context)) {
		return result(SD_PROCESS_EVIDENCE_READ_FAILED);
	}
	if (root.pid != launch_root->pid ||
			root.start_time_ticks != launch_root->start_time_ticks ||
			root.uid != launch_root->uid) {
		return result(SD_PROCESS_EVIDENCE_ROOT_MISMATCH);
	}

	struct stage_director_process_identity actor = {0};
	if (!read_process(actor_pid, &actor, context)) {
		return result(SD_PROCESS_EVIDENCE_READ_FAILED);
	}
	if (actor.pid != actor_pid ||
			actor.start_time_ticks != observed_actor_start) {
		return result(SD_PROCESS_EVIDENCE_ACTOR_MISMATCH);
	}

	struct stage_director_process_identity path[SD_PROCESS_MAX_ANCESTRY + 1];
	size_t used = 0;
	struct stage_director_process_identity node = actor;
	bool found_root = false;
	for (;;) {
		if (node.pid <= 0 || node.start_time_ticks == 0 ||
				node.uid != launch_root->uid ||
				used >= SD_PROCESS_MAX_ANCESTRY + 1) {
			return result(SD_PROCESS_EVIDENCE_UNRELATED);
		}
		for (size_t i = 0; i < used; ++i) {
			if (path[i].pid == node.pid) {
				return result(SD_PROCESS_EVIDENCE_UNRELATED);
			}
		}
		path[used++] = node;
		if (node.pid == root.pid) {
			if (!same_identity(node, root)) {
				return result(SD_PROCESS_EVIDENCE_UNSTABLE);
			}
			found_root = true;
			break;
		}
		if (node.parent_pid <= 0 || node.parent_pid == node.pid ||
				used > SD_PROCESS_MAX_ANCESTRY) {
			return result(SD_PROCESS_EVIDENCE_UNRELATED);
		}
		struct stage_director_process_identity parent = {0};
		if (!read_process(node.parent_pid, &parent, context)) {
			return result(SD_PROCESS_EVIDENCE_READ_FAILED);
		}
		if (parent.pid != node.parent_pid ||
				parent.start_time_ticks > node.start_time_ticks ||
				parent.start_time_ticks == 0) {
			return result(SD_PROCESS_EVIDENCE_UNSTABLE);
		}
		node = parent;
	}
	if (!found_root) {
		return result(SD_PROCESS_EVIDENCE_UNRELATED);
	}

	/* Catch process exit/reuse and mid-check reparenting (also of root). */
	for (size_t i = 0; i < used; ++i) {
		struct stage_director_process_identity again = {0};
		if (!read_process(path[i].pid, &again, context) ||
				!same_identity(path[i], again)) {
			return result(SD_PROCESS_EVIDENCE_UNSTABLE);
		}
	}
	return result(SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE);
}

const char *stage_director_process_evidence_name(
		enum stage_director_process_evidence_state state) {
	switch (state) {
	case SD_PROCESS_EVIDENCE_INVALID: return "INVALID";
	case SD_PROCESS_EVIDENCE_READ_FAILED: return "READ_FAILED";
	case SD_PROCESS_EVIDENCE_ROOT_MISMATCH: return "ROOT_MISMATCH";
	case SD_PROCESS_EVIDENCE_ACTOR_MISMATCH: return "ACTOR_MISMATCH";
	case SD_PROCESS_EVIDENCE_UNRELATED: return "UNRELATED";
	case SD_PROCESS_EVIDENCE_UNSTABLE: return "UNSTABLE";
	case SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE:
		return "LINEAGE_CANDIDATE";
	}
	return "INVALID";
}
