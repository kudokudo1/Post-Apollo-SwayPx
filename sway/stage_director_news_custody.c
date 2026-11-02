#define _GNU_SOURCE
#include <limits.h>
#include <poll.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_news_custody.h"

/*
 * This is a held kernel descriptor for one candidate *process*, not a
 * compositor launch credential. Only a trusted launcher can supply the
 * expected child identity, and that launcher does not exist in SD1 yet.
 */
static bool pidfd_alive(int pidfd) {
	if (pidfd < 0) {
		return false;
	}
	struct pollfd fd = {.fd = pidfd, .events = POLLIN};
	const int ready = poll(&fd, 1, 0);
	return ready == 0;
}

void stage_director_news_custody_init(struct stage_director_news_custody *lease) {
	if (!lease) return;
	memset(lease, 0, sizeof(*lease));
	lease->pidfd = -1;
}

bool stage_director_news_custody_reserve(
		struct stage_director_news_custody *lease, uint64_t epoch,
		uintptr_t host_ref, bool unique_native_host_candidate) {
	if (!lease || lease->state != SD_NEWS_CUSTODY_EMPTY ||
			lease->pidfd != -1 || !unique_native_host_candidate ||
			epoch == 0 || epoch <= lease->last_epoch || host_ref == 0) {
		return false;
	}
	lease->state = SD_NEWS_CUSTODY_RESERVED;
	lease->epoch = epoch;
	lease->last_epoch = epoch;
	lease->host_ref = host_ref;
	return true;
}

bool stage_director_news_custody_capture(
		struct stage_director_news_custody *lease, uint64_t epoch,
		const struct stage_director_process_launch_root *expected) {
	if (!lease || !expected ||
			lease->state != SD_NEWS_CUSTODY_RESERVED ||
			lease->pidfd != -1 || lease->epoch != epoch || epoch == 0 ||
			lease->host_ref == 0 ||
			expected->pid <= 0 || expected->pid > INT_MAX ||
			expected->start_time_ticks == 0) {
		return false;
	}
#ifndef SYS_pidfd_open
	return false; /* unsupported architecture/kernel: fail closed */
#else
	/*
	 * The kernel pidfd pins process identity, preventing accidental PID-reuse
	 * ambiguity. Recheck /proc starttime and UID around descriptor capture.
	 * There is no process kill, exec, network or window mutation here.
	 */
	struct stage_director_process_identity first = {0};
	if (!stage_director_read_linux_process(expected->pid, &first, NULL) ||
			first.start_time_ticks != expected->start_time_ticks ||
			first.uid != expected->uid) {
		return false;
	}
	const int descriptor = (int)syscall(SYS_pidfd_open,
		(pid_t)expected->pid, 0);
	if (descriptor < 0) {
		return false;
	}
	struct stage_director_process_identity second = {0};
	const bool same = stage_director_read_linux_process(
		expected->pid, &second, NULL) &&
		second.pid == first.pid &&
		second.parent_pid == first.parent_pid &&
		second.start_time_ticks == first.start_time_ticks &&
		second.uid == first.uid &&
		pidfd_alive(descriptor);
	if (!same) {
		close(descriptor);
		return false;
	}
	lease->pidfd = descriptor;
	lease->root = *expected;
	lease->state = SD_NEWS_CUSTODY_PROCESS_CANDIDATE;
	return true;
#endif
}

struct stage_director_news_custody_result stage_director_news_custody_inspect(
		const struct stage_director_news_custody *lease) {
	struct stage_director_news_custody_result result = {
		.owner_verified = false,
		.can_launch = false,
		.can_bind_viewer = false,
		.can_clip = false,
		.can_accept_input = false,
	};
	if (!lease || lease->state != SD_NEWS_CUSTODY_PROCESS_CANDIDATE ||
			lease->pidfd < 0) {
		return result;
	}
	result.process_candidate = true;
	result.process_alive = pidfd_alive(lease->pidfd);
	return result;
}

bool stage_director_news_custody_release(
		struct stage_director_news_custody *lease, uint64_t epoch) {
	if (!lease || lease->state == SD_NEWS_CUSTODY_EMPTY ||
			epoch == 0 || epoch != lease->epoch) {
		return false;
	}
	if (lease->pidfd >= 0) {
		close(lease->pidfd);
	}
	const uint64_t floor = lease->last_epoch;
	memset(lease, 0, sizeof(*lease));
	lease->last_epoch = floor;
	lease->pidfd = -1;
	return true;
}
