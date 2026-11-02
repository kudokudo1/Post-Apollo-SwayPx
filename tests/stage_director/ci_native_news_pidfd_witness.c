#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "sway/stage_director_news_custody.h"

/*
 * CI-only, standalone PROCESS CUSTODY WITNESS for the disposable foot actor.
 * This has no caller in sway_sources, production IPC, Quickshell, or a browser.
 * The invoking headless harness obtained pid/uid/starttime from its actual
 * SwayPX wl_client credential log and separately matches the spawned PID.
 * A matching pidfd still establishes NO operator WATCH authorization.
 */
static bool number(const char *text, uint64_t *out) {
	if (!text || !*text || *text == '-' || !out) return false;
	errno = 0;
	char *end = NULL;
	const unsigned long long parsed = strtoull(text, &end, 10);
	if (errno || !end || *end) return false;
	*out = (uint64_t)parsed;
	return true;
}

static bool ci_only(const char *ready_path) {
	const char *ci = getenv("CI");
	const char *github = getenv("GITHUB_ACTIONS");
	const char *temp = getenv("RUNNER_TEMP");
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!ci || strcmp(ci, "true") != 0 || !github ||
			strcmp(github, "true") != 0 || !temp || !*temp ||
			!runtime || !ready_path) return false;
	const size_t n = strlen(temp);
	const size_t m = strlen(runtime);
	return strncmp(runtime, temp, n) == 0 &&
		runtime[n] == '/' && m > n + 1 &&
		strncmp(ready_path, runtime, m) == 0 &&
		ready_path[m] == '/' &&
		strcmp(ready_path + m + 1, "sd1-pidfd.ready") == 0;
}

static bool non_authorizing(struct stage_director_news_custody_result result) {
	return !result.owner_verified && !result.can_launch &&
		!result.can_bind_viewer && !result.can_clip &&
		!result.can_accept_input;
}

int main(int argc, char **argv) {
	if (argc != 5 || !ci_only(argv[4])) {
		fprintf(stderr, "BLOCKED_CI_PIDFD_WITNESS_ENV\n");
		return 2;
	}
	uint64_t pid = 0, uid = 0, start = 0;
	if (!number(argv[1], &pid) || !number(argv[2], &uid) ||
			!number(argv[3], &start) || !pid || pid > INT_MAX ||
			uid != (uint64_t)getuid() || uid > UINT32_MAX ||
			!start) {
		fprintf(stderr, "FAIL: invalid native peer process identity\n");
		return 1;
	}
	const struct stage_director_process_launch_root actual = {
		.pid = (int64_t)pid,
		.uid = (uint32_t)uid,
		.start_time_ticks = start,
	};
	struct stage_director_news_custody lease;
	stage_director_news_custody_init(&lease);
	const uint64_t epoch = 42;
	const uintptr_t ci_host = 0xA11; /* synthetic test reference, NOT native authority */
	if (stage_director_news_custody_reserve(&lease, epoch, ci_host, false) ||
			!stage_director_news_custody_reserve(&lease, epoch, ci_host, true) ||
			stage_director_news_custody_reserve(&lease, epoch + 1, ci_host, true)) {
		fprintf(stderr, "FAIL: candidate reservation was not unique\n");
		return 1;
	}
	const struct stage_director_news_custody_result reserved =
		stage_director_news_custody_inspect(&lease);
	if (reserved.process_candidate || !non_authorizing(reserved)) {
		fprintf(stderr, "FAIL: reserved session was incorrectly promoted\n");
		return 1;
	}
	struct stage_director_process_launch_root wrong = actual;
	++wrong.start_time_ticks;
	if (stage_director_news_custody_capture(&lease, epoch, &wrong)) {
		fprintf(stderr, "FAIL: stale starttime was accepted\n");
		return 1;
	}
	wrong = actual;
	wrong.uid ^= 1;
	if (stage_director_news_custody_capture(&lease, epoch, &wrong) ||
			!stage_director_news_custody_capture(&lease, epoch, &actual)) {
		fprintf(stderr, "FAIL: wrong UID accepted or real pidfd capture failed\n");
		return 1;
	}
	const struct stage_director_news_custody_result alive =
		stage_director_news_custody_inspect(&lease);
	if (!alive.process_candidate || !alive.process_alive ||
			!non_authorizing(alive) ||
			stage_director_news_custody_capture(&lease, epoch, &actual) ||
			stage_director_news_custody_release(&lease, epoch - 1)) {
		fprintf(stderr, "FAIL: held process evidence or epoch gate\n");
		return 1;
	}
	/* The parent harness won't terminate its actor until this file appears. */
	const int fd = open(argv[4], O_WRONLY | O_CREAT | O_EXCL |
			O_NOFOLLOW | O_CLOEXEC, 0600);
	if (fd < 0) {
		perror("FAIL: ready marker");
		return 1;
	}
	const ssize_t wrote = write(fd, "READY\n", 6);
	close(fd);
	if (wrote != 6) {
		fprintf(stderr, "FAIL: ready marker incomplete\n");
		return 1;
	}
	printf("SD_CI_PIDFD_HELD pid=%" PRIu64 " uid=%" PRIu64
		" start=%" PRIu64 " epoch=42 live=1 authority=NONE\n",
		pid, uid, start);
	fflush(stdout);

	/* This process does not signal or control foot. The harness must
	 * explicitly close its OWN child after completing all render checks. */
	bool exited = false;
	for (int attempt = 0; attempt < 240; ++attempt) {
		const struct stage_director_news_custody_result now =
			stage_director_news_custody_inspect(&lease);
		if (!now.process_candidate || !non_authorizing(now)) {
			fprintf(stderr, "FAIL: pidfd custody became authorizing\n");
			return 1;
		}
		if (!now.process_alive) {
			exited = true;
			break;
		}
		const struct timespec pause = { .tv_sec = 0, .tv_nsec = 50000000L };
		nanosleep(&pause, NULL);
	}
	if (!exited || !stage_director_news_custody_release(&lease, epoch) ||
			stage_director_news_custody_release(&lease, epoch) ||
			stage_director_news_custody_reserve(&lease, epoch, ci_host, true) ||
			!stage_director_news_custody_reserve(
				&lease, epoch + 1, ci_host, true) ||
			!stage_director_news_custody_release(&lease, epoch + 1)) {
		fprintf(stderr, "FAIL: exit/release/replay custody contract\n");
		return 1;
	}
	puts("SD_CI_PIDFD_RELEASE process_exit_observed=1 "
		"pidfd_closed=1 epoch_replay_rejected=1 authority=NONE result=PASS");
	return 0;
}
