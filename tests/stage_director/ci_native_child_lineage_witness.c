#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sway/stage_director_process_evidence.h"

/*
 * Disposable CI only: a fixed bash launch-root directly spawns a distinct
 * native Wayland foot process. The compositor logged foot's kernel-supplied
 * wl_client credentials, and the harness checks its PID against the
 * launching bash's recorded child PID. This witness checks LIVE ancestry,
 * not a synthesized /proc fixture, and never grants launch/render/input.
 * This does not attest a production WATCH intent or allow browser control.
 */
static bool parse_u64(const char *s, uint64_t *out) {
	if (!s || !*s || *s == '-' || !out) return false;
	errno = 0;
	char *end = NULL;
	const unsigned long long value = strtoull(s, &end, 10);
	if (errno || !end || *end) return false;
	*out = (uint64_t)value;
	return true;
}

static bool isolated_ci(void) {
	const char *ci = getenv("CI");
	const char *github = getenv("GITHUB_ACTIONS");
	const char *temp = getenv("RUNNER_TEMP");
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!ci || strcmp(ci, "true") != 0 ||
			!github || strcmp(github, "true") != 0 ||
			!temp || !*temp || !runtime) {
		return false;
	}
	const size_t len = strlen(temp);
	return strlen(runtime) > len + 1 &&
		strncmp(runtime, temp, len) == 0 && runtime[len] == '/';
}

static bool all_denied(struct stage_director_process_evidence_result r) {
	return !r.owner_verified && !r.can_bind_viewer &&
		!r.can_launch && !r.can_clip && !r.can_accept_input;
}

int main(int argc, char **argv) {
	if (!isolated_ci() || argc != 5) {
		fputs("BLOCKED_CI_LINEAGE_ENV\n", stderr);
		return 2;
	}
	uint64_t root_pid = 0, peer_pid = 0, peer_uid = 0, peer_start = 0;
	if (!parse_u64(argv[1], &root_pid) ||
			!parse_u64(argv[2], &peer_pid) ||
			!parse_u64(argv[3], &peer_uid) ||
			!parse_u64(argv[4], &peer_start) ||
			root_pid == 0 || root_pid > INT_MAX ||
			peer_pid == 0 || peer_pid > INT_MAX ||
			peer_pid == root_pid || peer_uid != (uint64_t)getuid() ||
			peer_start == 0) {
		fputs("FAIL: invalid distinct launch-root/Wayland-child identities\n", stderr);
		return 1;
	}
	struct stage_director_process_identity root_identity = {0};
	struct stage_director_process_identity actor_identity = {0};
	if (!stage_director_read_linux_process((int64_t)root_pid, &root_identity, NULL) ||
			!stage_director_read_linux_process((int64_t)peer_pid, &actor_identity, NULL) ||
			root_identity.pid != (int64_t)root_pid ||
			actor_identity.pid != (int64_t)peer_pid ||
			root_identity.uid != (uint32_t)peer_uid ||
			actor_identity.uid != (uint32_t)peer_uid ||
			actor_identity.start_time_ticks != peer_start ||
			root_identity.start_time_ticks == 0) {
		fputs("FAIL: root/Wayland actor identity changed before lineage check\n", stderr);
		return 1;
	}
	const struct stage_director_process_launch_root launch_root = {
		.pid = (int64_t)root_pid,
		.start_time_ticks = root_identity.start_time_ticks,
		.uid = root_identity.uid,
	};
	const struct stage_director_process_evidence_result good =
		stage_director_process_check(&launch_root, (int64_t)peer_pid,
			peer_start, stage_director_read_linux_process, NULL);
	if (good.state != SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE ||
			!good.lineage_candidate || !all_denied(good)) {
		fprintf(stderr, "FAIL: real parent/child ancestry: %s\n",
			stage_director_process_evidence_name(good.state));
		return 1;
	}
	struct stage_director_process_launch_root wrong = launch_root;
	wrong.start_time_ticks++;
	const struct stage_director_process_evidence_result stale =
		stage_director_process_check(&wrong, (int64_t)peer_pid,
			peer_start, stage_director_read_linux_process, NULL);
	wrong = launch_root;
	wrong.uid ^= 1;
	const struct stage_director_process_evidence_result foreign_uid =
		stage_director_process_check(&wrong, (int64_t)peer_pid,
			peer_start, stage_director_read_linux_process, NULL);
	const struct stage_director_process_evidence_result wrong_actor_start =
		stage_director_process_check(&launch_root, (int64_t)peer_pid,
			peer_start + 1, stage_director_read_linux_process, NULL);
	/* Test an actual, live process with valid kernel starttime that is
	 * NOT descended from this launch root. A made-up starttime would merely
	 * trigger ACTOR_MISMATCH and would not test ancestry rejection. */
	struct stage_director_process_identity self = {0};
	if (!stage_director_read_linux_process((int64_t)getpid(), &self, NULL) ||
			self.start_time_ticks == 0) {
		fputs("FAIL: missing real unrelated witness process\n", stderr);
		return 1;
	}
	const struct stage_director_process_evidence_result unrelated =
		stage_director_process_check(&launch_root, (int64_t)getpid(),
			self.start_time_ticks, stage_director_read_linux_process, NULL);
	/*
	 * Linux runners may limit reads of *ancestors* outside the disposable
	 * process tree. For a real unrelated PID and valid start marker,
	 * UNRELATED is direct evidence, while READ_FAILED/UNSTABLE must stay
	 * denied (never silently promote on incomplete ancestry). ACTOR_MISMATCH
	 * is NOT accepted here: the witness start marker was independently read.
	 */
	const bool unrelated_denied =
		unrelated.state == SD_PROCESS_EVIDENCE_UNRELATED ||
		unrelated.state == SD_PROCESS_EVIDENCE_READ_FAILED ||
		unrelated.state == SD_PROCESS_EVIDENCE_UNSTABLE;
	printf("SD_CI_CHILD_DENIALS stale=%s foreign=%s wrong_actor=%s "
		"unrelated_live=%s authority=NONE\n",
		stage_director_process_evidence_name(stale.state),
		stage_director_process_evidence_name(foreign_uid.state),
		stage_director_process_evidence_name(wrong_actor_start.state),
		stage_director_process_evidence_name(unrelated.state));
	if (stale.state != SD_PROCESS_EVIDENCE_ROOT_MISMATCH ||
			foreign_uid.state != SD_PROCESS_EVIDENCE_ROOT_MISMATCH ||
			wrong_actor_start.state != SD_PROCESS_EVIDENCE_ACTOR_MISMATCH ||
			!unrelated_denied ||
			!all_denied(stale) || !all_denied(foreign_uid) ||
			!all_denied(wrong_actor_start) || !all_denied(unrelated)) {
		fputs("FAIL: stale/foreign/unrelated live process identity accepted\n", stderr);
		return 1;
	}
	printf("SD_CI_CHILD_LINEAGE root_pid=%" PRIu64 " client_pid=%" PRIu64
		" client_uid=%" PRIu64 " root_start=%" PRIu64
		" child_start=%" PRIu64 " distinct_processes=1 "
		"stable_proc_ancestry=1 mismatch_rejected=1 "
		"authority=NONE result=PASS\n", root_pid, peer_pid, peer_uid,
		root_identity.start_time_ticks, peer_start);
	return 0;
}
