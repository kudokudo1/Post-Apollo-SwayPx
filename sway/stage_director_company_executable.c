#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_executable.h"

/*
 * M15 runs a fixed, owner-only ELF on the same PID after two execs.
 * This reader checks a live kernel pidfd + stable /proc start ticks before
 * AND after comparing fstat() of the actual executing ELF vs the installed
 * owner-checked path. It is NOT a cryptographic attestation.
 */
static struct stage_director_company_executable_result result(
		enum stage_director_company_executable_status status) {
	return (struct stage_director_company_executable_result) {
		.status = status,
		.running_current_install_inode_candidate =
			status == SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE,
		/* Every authority flag intentionally stays false. */
	};
}

static bool pidfd_alive(int fd) {
#if defined(SYS_pidfd_send_signal)
	return fd >= 0 && syscall(SYS_pidfd_send_signal, fd, 0, NULL, 0) == 0;
#else
	(void)fd;
	return false;
#endif
}

static bool same_file(const struct stat *a, const struct stat *b) {
	return a->st_dev == b->st_dev && a->st_ino == b->st_ino;
}

static bool safe_owner_directory(int fd, uint32_t uid) {
	struct stat st = {0};
	return fd >= 0 && fstat(fd, &st) == 0 &&
		S_ISDIR(st.st_mode) && st.st_uid == (uid_t)uid &&
		!(st.st_mode & (S_IWGRP | S_IWOTH));
}

static bool safe_owner_executable(int fd, uint32_t uid,
		struct stat *out) {
	struct stat st = {0};
	if (fd < 0 || !out || fstat(fd, &st) != 0 ||
			!S_ISREG(st.st_mode) || st.st_uid != (uid_t)uid ||
			!(st.st_mode & S_IXUSR) ||
			(st.st_mode & (S_IWGRP | S_IWOTH | S_ISUID | S_ISGID)) ||
			!st.st_nlink || st.st_size <= 0) {
		return false;
	}
	*out = st;
	return true;
}

static bool same_process(
		const struct stage_director_company_birth_pidfd *held,
		struct stage_director_process_identity *host,
		struct stage_director_process_identity *child) {
	if (!held || !host || !child || !pidfd_alive(held->fd) ||
			!stage_director_read_linux_process(held->host_pid, host, NULL) ||
			!stage_director_read_linux_process(held->pid, child, NULL)) {
		return false;
	}
	return host->pid == held->host_pid &&
		host->uid == held->uid &&
		host->start_time_ticks == held->host_start_ticks &&
		child->pid == held->pid &&
		child->uid == held->uid &&
		child->parent_pid == held->host_pid &&
		child->start_time_ticks == held->child_start_ticks;
}

static int open_owned_dir(int parent, const char *name, uint32_t uid) {
	const int dir = openat(parent, name,
		O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (dir < 0) return -1;
	if (!safe_owner_directory(dir, uid)) {
		close(dir);
		return -1;
	}
	return dir;
}

struct stage_director_company_executable_result
stage_director_company_executable_check_owner_home(
		const struct stage_director_company_birth_pidfd *held,
		const char *owner_home) {
	if (!held || !held->initialized || !held->held ||
			held->fd < 0 || held->pid <= 0 || held->host_pid <= 0 ||
			!held->uid || !held->child_start_ticks ||
			!held->host_start_ticks || !held->selection_epoch ||
			!held->intent_sequence || !held->host_native_client_ref) {
		return result(SD_COMPANY_EXE_NO_PIDFD);
	}
	if (!owner_home || owner_home[0] != '/' ||
			strlen(owner_home) > 2047) {
		return result(SD_COMPANY_EXE_HOME_UNAVAILABLE);
	}
	struct stage_director_process_identity parent_before = {0};
	struct stage_director_process_identity child_before = {0};
	if (!same_process(held, &parent_before, &child_before)) {
		return result(SD_COMPANY_EXE_PROCESS_CHANGED);
	}
	int home = open(owner_home,
		O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (home < 0 || !safe_owner_directory(home, held->uid)) {
		if (home >= 0) close(home);
		return result(SD_COMPANY_EXE_UNSAFE_INSTALL);
	}
	int lab = open_owned_dir(home, "postapollo-vector-lab", held->uid);
	close(home);
	if (lab < 0) return result(SD_COMPANY_EXE_UNSAFE_INSTALL);
	int build = open_owned_dir(lab, "company-vector-build", held->uid);
	close(lab);
	if (build < 0) return result(SD_COMPANY_EXE_UNSAFE_INSTALL);
	const int installed = openat(build, "company-vector-probe",
		O_PATH | O_NOFOLLOW | O_CLOEXEC);
	struct stat expected = {0};
	const bool safe_file =
		safe_owner_executable(installed, held->uid, &expected);
	if (!safe_file) {
		if (installed >= 0) close(installed);
		close(build);
		return result(SD_COMPANY_EXE_UNSAFE_INSTALL);
	}

	/* Open the actual executable inode of that PID. /proc/<pid>/exe is a
	 * kernel-generated symlink: intentionally follow it with O_PATH.
	 * Opening an arbitrary caller-supplied path would be unsafe. */
	int proc = open("/proc", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	char pid_text[32];
	const int n = snprintf(pid_text, sizeof(pid_text),
		"%" PRId64, held->pid);
	int pid_dir = -1;
	int exe = -1;
	struct stat executing = {0};
	struct stat still_installed = {0};
	bool exe_ok = false;
	bool installation_stable = false;
	if (proc >= 0 && n > 0 && (size_t)n < sizeof(pid_text)) {
		pid_dir = openat(proc, pid_text,
			O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
		if (pid_dir >= 0) {
			struct stat proc_st = {0};
			if (fstat(pid_dir, &proc_st) == 0 &&
					proc_st.st_uid == (uid_t)held->uid &&
					S_ISDIR(proc_st.st_mode)) {
				exe = openat(pid_dir, "exe", O_PATH | O_CLOEXEC);
				exe_ok = exe >= 0 &&
					fstat(exe, &executing) == 0 &&
					S_ISREG(executing.st_mode);
			}
		}
	}
	if (exe_ok) {
		/* Detect installed-file replacement even if a previous open FD
		 * still points at the original inode. Reject symlink swaps. */
		installation_stable =
			fstatat(build, "company-vector-probe", &still_installed,
				AT_SYMLINK_NOFOLLOW) == 0 &&
			S_ISREG(still_installed.st_mode) &&
			same_file(&still_installed, &expected) &&
			same_file(&still_installed, &executing) &&
			same_file(&executing, &expected);
	}
	if (exe >= 0) close(exe);
	if (pid_dir >= 0) close(pid_dir);
	if (proc >= 0) close(proc);
	close(installed);
	close(build);

	struct stage_director_process_identity parent_after = {0};
	struct stage_director_process_identity child_after = {0};
	if (!same_process(held, &parent_after, &child_after) ||
			parent_before.start_time_ticks != parent_after.start_time_ticks ||
			parent_before.pid != parent_after.pid ||
			parent_before.uid != parent_after.uid ||
			child_before.start_time_ticks != child_after.start_time_ticks ||
			child_before.pid != child_after.pid ||
			child_before.parent_pid != child_after.parent_pid ||
			child_before.uid != child_after.uid) {
		return result(SD_COMPANY_EXE_PROCESS_CHANGED);
	}
	if (!exe_ok) return result(SD_COMPANY_EXE_RUNNING_EXE_UNAVAILABLE);
	if (!installation_stable) return result(SD_COMPANY_EXE_DIFFERENT_INODE);
	return result(SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE);
}

struct stage_director_company_executable_result
stage_director_company_executable_check_system_home(
		const struct stage_director_company_birth_pidfd *held) {
	if (!held || !held->initialized || !held->held || !held->uid) {
		return result(SD_COMPANY_EXE_NO_PIDFD);
	}
	/* NSS/passwd is compositor-side account metadata. Neither a client
	 * home path, argv, environment HOME nor public namespace is accepted. */
	char buffer[16384];
	struct passwd entry = {0};
	struct passwd *found = NULL;
	if (getpwuid_r((uid_t)held->uid, &entry, buffer, sizeof(buffer),
			&found) != 0 || !found || !found->pw_dir ||
			!found->pw_dir[0]) {
		return result(SD_COMPANY_EXE_HOME_UNAVAILABLE);
	}
	return stage_director_company_executable_check_owner_home(
		held, found->pw_dir);
}

const char *stage_director_company_executable_status_name(
		enum stage_director_company_executable_status state) {
	switch (state) {
	case SD_COMPANY_EXE_NO_PIDFD: return "NO_PIDFD";
	case SD_COMPANY_EXE_PROCESS_CHANGED: return "PROCESS_CHANGED";
	case SD_COMPANY_EXE_HOME_UNAVAILABLE: return "HOME_UNAVAILABLE";
	case SD_COMPANY_EXE_UNSAFE_INSTALL: return "UNSAFE_INSTALL";
	case SD_COMPANY_EXE_RUNNING_EXE_UNAVAILABLE: return "EXE_UNAVAILABLE";
	case SD_COMPANY_EXE_DIFFERENT_INODE: return "DIFFERENT_INODE";
	case SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE:
		return "CURRENT_INSTALL_INODE_CANDIDATE";
	}
	return "UNKNOWN";
}
