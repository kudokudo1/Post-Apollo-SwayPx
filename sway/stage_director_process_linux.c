#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sway/stage_director_process_evidence.h"

/*
 * Linux native read-only process evidence. /proc is used for inspection
 * only, never as a launcher, credential transport or trust source for
 * operator WATCH intent. Procfs directory ownership is fail-closed for
 * non-dumpable processes whose visible owner differs from the launcher.
 */
bool stage_director_parse_linux_proc_stat(const char *stat,
		int64_t expected_pid, int64_t *out_parent_pid,
		uint64_t *out_start_time_ticks) {
	if (!stat || !out_parent_pid || !out_start_time_ticks ||
			expected_pid <= 0 || strlen(stat) > 4095) {
		return false;
	}
	char copy[4096];
	memcpy(copy, stat, strlen(stat) + 1);
	char *comm_open = strchr(copy, '(');
	char *comm_end = strrchr(copy, ')');
	if (!comm_open || !comm_end || comm_end <= comm_open ||
			comm_end[1] != ' ') {
		return false;
	}
	char *end = NULL;
	errno = 0;
	const long long value = strtoll(copy, &end, 10);
	if (errno || value != expected_pid || !end) {
		return false;
	}
	while (*end == ' ') ++end;
	if (end != comm_open) {
		return false;
	}
	int64_t ppid = 0;
	uint64_t start = 0;
	char *save = NULL;
	for (int field = 3; field <= 22; ++field) {
		char *token = field == 3
			? strtok_r(comm_end + 2, " \t\r\n", &save)
			: strtok_r(NULL, " \t\r\n", &save);
		if (!token) {
			return false;
		}
		char *tail = NULL;
		if (field == 4) {
			errno = 0;
			const long long n = strtoll(token, &tail, 10);
			if (errno || !tail || *tail || n <= 0) {
				return false;
			}
			ppid = n;
		}
		if (field == 22) {
			if (*token == '-') {
				return false;
			}
			errno = 0;
			const unsigned long long n = strtoull(token, &tail, 10);
			if (errno || !tail || *tail || n == 0) {
				return false;
			}
			start = n;
		}
	}
	*out_parent_pid = ppid;
	*out_start_time_ticks = start;
	return true;
}

bool stage_director_read_linux_process(
		int64_t pid, struct stage_director_process_identity *out,
		void *context) {
	(void)context;
	if (!out || pid <= 0 || pid > INT_MAX) {
		return false;
	}
	char name[32];
	if (snprintf(name, sizeof(name), "%" PRId64, pid) <= 0) {
		return false;
	}

	int procfd = open("/proc", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (procfd < 0) {
		return false;
	}
	int dir = openat(procfd, name,
		O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
	close(procfd);
	if (dir < 0) {
		return false;
	}
	struct stat metadata = {0};
	if (fstat(dir, &metadata) != 0 || !S_ISDIR(metadata.st_mode)) {
		close(dir);
		return false;
	}
	const uid_t observed_uid = metadata.st_uid;
	int statfd = openat(dir, "stat", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	if (statfd < 0) {
		close(dir);
		return false;
	}
	char contents[4096];
	size_t used = 0;
	for (;;) {
		const ssize_t n = read(statfd, contents + used,
			sizeof(contents) - 1 - used);
		if (n <= 0) {
			if (n < 0) used = sizeof(contents);
			break;
		}
		used += (size_t)n;
		if (used == sizeof(contents) - 1) {
			used = sizeof(contents);
			break;
		}
	}
	close(statfd);
	close(dir);
	if (used == 0 || used >= sizeof(contents)) {
		return false;
	}
	contents[used] = '\0';

	int64_t parent_pid = 0;
	uint64_t start_time_ticks = 0;
	if (!stage_director_parse_linux_proc_stat(contents, pid,
			&parent_pid, &start_time_ticks)) {
		return false;
	}
	*out = (struct stage_director_process_identity) {
		.pid = pid,
		.parent_pid = parent_pid,
		.start_time_ticks = start_time_ticks,
		.uid = (uint32_t)observed_uid,
	};
	return true;
}
