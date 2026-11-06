#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "sway/stage_director_company_birth_pidfd.h"

/*
 * Kernel-backed lifetime evidence only. The read-only fd-info Pid field and
 * pidfd_send_signal(0) check kernel identity without signaling the child.
 * No procfs text can be supplied by a Wayland client to this function.
 */
static bool live_pidfd(int fd) {
#if defined(SYS_pidfd_send_signal)
	return fd >= 0 && syscall(SYS_pidfd_send_signal, fd, 0, NULL, 0) == 0;
#else
	(void)fd;
	return false;
#endif
}

static bool pidfd_kernel_pid(int fd, int64_t *pid) {
	if (fd < 0 || !pid || !live_pidfd(fd)) return false;
	char path[64];
	if (snprintf(path, sizeof(path), "/proc/self/fdinfo/%d", fd) <= 0) {
		return false;
	}
	int handle = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	if (handle < 0) return false;
	char info[2048];
	ssize_t n = read(handle, info, sizeof(info) - 1);
	close(handle);
	if (n <= 0 || n == (ssize_t)sizeof(info) - 1) return false;
	info[n] = '\0';
	/* fdinfo is a kernel-owned file, not a user-supplied path/JSON.
	 * Reject duplicates and malformed integers (including Pid: -1 for a
	 * process that is gone). */
	bool found = false;
	char *cursor = info;
	while (*cursor) {
		char *end = strchr(cursor, '\n');
		if (end) *end = '\0';
		if (!strncmp(cursor, "Pid:", 4)) {
			if (found) return false;
			char *start = cursor + 4;
			while (*start == ' ' || *start == '\t') start++;
			errno = 0;
			char *tail = NULL;
			long long number = strtoll(start, &tail, 10);
			if (errno || !tail || *tail || number <= 0 ||
					number > INT_MAX) return false;
			*pid = number;
			found = true;
		}
		if (!end) break;
		cursor = end + 1;
	}
	return found;
}

static struct stage_director_company_birth_result result(
		enum stage_director_company_birth_status status) {
	return (struct stage_director_company_birth_result) {
		.status = status,
		.kernel_child_candidate =
			status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY ||
			status == SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY,
		.mapped_actor_candidate =
			status == SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY,
		/* Executable and real Company launch authorization NOT proven. */
	};
}

void stage_director_company_birth_pidfd_init(
		struct stage_director_company_birth_pidfd *held) {
	if (!held || held->initialized) return;
	memset(held, 0, sizeof(*held));
	held->initialized = true;
	held->fd = -1;
}

void stage_director_company_birth_pidfd_revoke(
		struct stage_director_company_birth_pidfd *held) {
	if (!held || !held->initialized) return;
	if (held->fd >= 0) close(held->fd);
	memset(held, 0, sizeof(*held));
	held->initialized = true;
	held->fd = -1;
}

static bool same_identity(struct stage_director_process_identity a,
		struct stage_director_process_identity b) {
	return a.pid == b.pid && a.parent_pid == b.parent_pid &&
		a.uid == b.uid && a.start_time_ticks == b.start_time_ticks;
}

static bool same_host(
		const struct stage_director_company_birth_pidfd *held,
		const struct stage_director_company_native_host *host) {
	return held && held->held && host && host->candidate &&
		host->state == SD_COMPANY_NATIVE_CANDIDATE_ONLY &&
		host->native_client_ref == held->host_native_client_ref &&
		host->pid == held->host_pid && host->start_ticks == held->host_start_ticks &&
		host->uid == held->uid && host->native_layer_ref &&
		host->native_layer_role_ref && host->native_surface_ref &&
		host->native_output_ref;
}

struct stage_director_company_birth_result
stage_director_company_birth_pidfd_record(
		struct stage_director_company_birth_pidfd *held,
		int borrowed_fd, int64_t claimed_pid,
		const struct stage_director_company_intent_snapshot *intent,
		const struct stage_director_company_native_host *host,
		stage_director_process_reader reader, void *context) {
	if (!held || !held->initialized || held->held || !reader ||
			!intent || !intent->present || !intent->selection_epoch ||
			!intent->sequence || !intent->observed_boot_ticks ||
			!intent->host_revision || intent->mode > 1 ||
			!host || !host->candidate ||
			host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!host->native_layer_ref || !host->native_layer_role_ref ||
			!host->native_surface_ref || !host->native_client_ref ||
			!host->native_output_ref ||
			host->native_client_ref != intent->native_host_client_ref ||
			host->pid <= 0 || !host->uid || !host->start_ticks ||
			claimed_pid <= 0 || claimed_pid > INT_MAX ||
			claimed_pid == host->pid) {
		return result(SD_COMPANY_BIRTH_NO_INTENT);
	}
	int64_t kernel_pid = 0;
	if (!pidfd_kernel_pid(borrowed_fd, &kernel_pid)) {
		return result(SD_COMPANY_BIRTH_INVALID_PIDFD);
	}
	if (kernel_pid != claimed_pid) {
		return result(SD_COMPANY_BIRTH_WRONG_CHILD);
	}
	struct stage_director_process_identity host_before = {0};
	struct stage_director_process_identity child_before = {0};
	if (!reader(host->pid, &host_before, context) ||
			!reader(kernel_pid, &child_before, context) ||
			host_before.pid != host->pid ||
			host_before.uid != host->uid ||
			host_before.start_time_ticks != host->start_ticks ||
			child_before.pid != kernel_pid ||
			child_before.uid != host->uid ||
			child_before.parent_pid != host->pid ||
			child_before.start_time_ticks < host_before.start_time_ticks) {
		return result(SD_COMPANY_BIRTH_WRONG_CHILD);
	}
	/* No third-party launcher and no ancient manual map can satisfy a
	 * fresh selected-host intent. These bounds do not establish executable
	 * identity; future M28b must require an exact trusted binary witness. */
	const long hz = sysconf(_SC_CLK_TCK);
	if (hz <= 0 || hz > 1000000) {
		return result(SD_COMPANY_BIRTH_OUTSIDE_WINDOW);
	}
	const uint64_t born = child_before.start_time_ticks;
	const uint64_t intent_born = intent->observed_boot_ticks;
	const uint64_t delta = born >= intent_born ?
		born - intent_born : intent_born - born;
	const uint64_t max_delta = born >= intent_born ?
		5 * (uint64_t)hz : 2 * (uint64_t)hz;
	if (delta > max_delta) {
		return result(SD_COMPANY_BIRTH_OUTSIDE_WINDOW);
	}
	int owned = fcntl(borrowed_fd, F_DUPFD_CLOEXEC, 3);
	if (owned < 0) return result(SD_COMPANY_BIRTH_EXHAUSTED);
	int64_t again_pid = 0;
	struct stage_director_process_identity host_after = {0};
	struct stage_director_process_identity child_after = {0};
	if (!pidfd_kernel_pid(owned, &again_pid) || again_pid != kernel_pid ||
			!reader(host->pid, &host_after, context) ||
			!reader(kernel_pid, &child_after, context) ||
			!same_identity(host_before, host_after) ||
			!same_identity(child_before, child_after)) {
		close(owned);
		return result(SD_COMPANY_BIRTH_PROCESS_CHANGED);
	}
	held->held = true;
	held->fd = owned;
	held->pid = kernel_pid;
	held->uid = host->uid;
	held->child_start_ticks = child_after.start_time_ticks;
	held->host_pid = host->pid;
	held->host_start_ticks = host->start_ticks;
	held->host_native_client_ref = host->native_client_ref;
	held->selection_epoch = intent->selection_epoch;
	held->intent_sequence = intent->sequence;
	held->host_revision = intent->host_revision;
	held->mode = intent->mode;
	return result(SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY);
}

struct stage_director_company_birth_result
stage_director_company_birth_pidfd_match_actor(
		struct stage_director_company_birth_pidfd *held,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor,
		const struct stage_director_company_lineage_result *lineage,
		stage_director_process_reader reader, void *context) {
	if (!held || !held->initialized || !held->held || !reader ||
			!live_pidfd(held->fd)) {
		return result(SD_COMPANY_BIRTH_INVALID_PIDFD);
	}
	if (!same_host(held, host)) {
		return result(SD_COMPANY_BIRTH_INVALID_NATIVE_HOST);
	}
	if (!actor || !actor->candidate ||
			actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!actor->native_view_ref || !actor->native_root_surface_ref ||
			!actor->native_client_ref ||
			actor->native_client_ref == host->native_client_ref ||
			actor->pid != held->pid || actor->uid != held->uid ||
			actor->start_ticks != held->child_start_ticks ||
			!lineage || lineage->status != SD_COMPANY_LINEAGE_CANDIDATE_ONLY ||
			!lineage->direct_child_candidate) {
		return result(SD_COMPANY_BIRTH_WRONG_CHILD);
	}
	struct stage_director_process_identity current = {0};
	if (!reader(held->pid, &current, context) ||
			current.pid != held->pid || current.uid != held->uid ||
			current.parent_pid != held->host_pid ||
			current.start_time_ticks != held->child_start_ticks ||
			!live_pidfd(held->fd)) {
		return result(SD_COMPANY_BIRTH_PROCESS_CHANGED);
	}
	int64_t kernel_pid = 0;
	if (!pidfd_kernel_pid(held->fd, &kernel_pid) ||
			kernel_pid != current.pid) {
		return result(SD_COMPANY_BIRTH_PROCESS_CHANGED);
	}
	return result(SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY);
}

const char *stage_director_company_birth_status_name(
		enum stage_director_company_birth_status status) {
	switch (status) {
	case SD_COMPANY_BIRTH_NO_INTENT: return "NO_INTENT";
	case SD_COMPANY_BIRTH_INVALID_NATIVE_HOST: return "INVALID_NATIVE_HOST";
	case SD_COMPANY_BIRTH_INVALID_PIDFD: return "INVALID_PIDFD";
	case SD_COMPANY_BIRTH_WRONG_CHILD: return "WRONG_CHILD";
	case SD_COMPANY_BIRTH_PROCESS_CHANGED: return "PROCESS_CHANGED";
	case SD_COMPANY_BIRTH_OUTSIDE_WINDOW: return "OUTSIDE_WINDOW";
	case SD_COMPANY_BIRTH_EXHAUSTED: return "RESOURCE_EXHAUSTED";
	case SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY: return "HELD_CANDIDATE_ONLY";
	case SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY: return "ACTOR_CANDIDATE_ONLY";
	}
	return "UNKNOWN";
}
