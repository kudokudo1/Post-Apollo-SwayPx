#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sway/stage_director_company_executable.h"

static void expect_no_authority(
		struct stage_director_company_executable_result r) {
	assert(!r.trusted_content_digest_verified);
	assert(!r.executable_approved);
	assert(!r.launch_authenticated);
	assert(!r.owner_verified);
	assert(!r.can_pair_actor);
	assert(!r.can_render && !r.can_clip);
	assert(!r.can_focus && !r.can_accept_input && !r.can_show_popup);
}
static void expect_status(struct stage_director_company_executable_result r,
		enum stage_director_company_executable_status status) {
	if (r.status != status) fprintf(stderr, "M29 expected=%s actual=%s\n",
		stage_director_company_executable_status_name(status),
		stage_director_company_executable_status_name(r.status));
	assert(r.status == status);
	assert(r.running_current_install_inode_candidate ==
		(status == SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE));
	expect_no_authority(r);
}
static void copy_binary(const char *src, const char *dst) {
	int input = open(src, O_RDONLY | O_CLOEXEC);
	assert(input >= 0);
	int output = open(dst, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0700);
	assert(output >= 0);
	char buffer[65536];
	for (;;) {
		const ssize_t n = read(input, buffer, sizeof(buffer));
		assert(n >= 0);
		if (n == 0) break;
		ssize_t done = 0;
		while (done < n) {
			const ssize_t w = write(output, buffer + done, (size_t)(n - done));
			assert(w > 0);
			done += w;
		}
	}
	assert(fchmod(output, 0700) == 0);
	assert(close(output) == 0);
	assert(close(input) == 0);
}
static bool exe_is_file(pid_t pid, const char *path) {
	struct stat want = {0}, got = {0};
	char procpath[64];
	if (snprintf(procpath, sizeof(procpath), "/proc/%d/exe", (int)pid) <= 0)
		return false;
	return stat(path, &want) == 0 && stat(procpath, &got) == 0 &&
		want.st_dev == got.st_dev && want.st_ino == got.st_ino;
}
static void actual_exec_and_swap_test(void) {
	assert(getuid() != 0 && geteuid() == getuid());
	char home[] = "/tmp/sd-company-m29-XXXXXX";
	assert(mkdtemp(home));
	char lab[PATH_MAX], build[PATH_MAX], binary[PATH_MAX],
		renamed[PATH_MAX];
	assert(snprintf(lab, sizeof(lab), "%s/postapollo-vector-lab", home) > 0);
	assert(snprintf(build, sizeof(build), "%s/company-vector-build", lab) > 0);
	assert(snprintf(binary, sizeof(binary), "%s/company-vector-probe", build) > 0);
	assert(snprintf(renamed, sizeof(renamed), "%s/old-owner-binary", build) > 0);
	assert(mkdir(lab, 0700) == 0);
	assert(mkdir(build, 0700) == 0);
	copy_binary("/usr/bin/sleep", binary);

	const pid_t child = fork();
	assert(child >= 0);
	if (child == 0) {
		char *const argv[] = {binary, "30", NULL};
		execv(binary, argv);
		_exit(127);
	}
	bool executable_ready = false;
	for (int attempt = 0; attempt < 300; ++attempt) {
		if (exe_is_file(child, binary)) {
			executable_ready = true;
			break;
		}
		usleep(10000);
	}
	assert(executable_ready);

#if defined(SYS_pidfd_open)
	const int pidfd = (int)syscall(SYS_pidfd_open, child, 0);
#else
	const int pidfd = -1;
#endif
	assert(pidfd >= 0);
	struct stage_director_process_identity parent = {0}, kid = {0};
	assert(stage_director_read_linux_process(getpid(), &parent, NULL));
	assert(stage_director_read_linux_process(child, &kid, NULL));
	assert(kid.parent_pid == parent.pid);
	struct stage_director_company_native_host host = {
		.candidate = true, .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_layer_ref = 0x11, .native_layer_role_ref = 0x12,
		.native_surface_ref = 0x13, .native_client_ref = 0x14,
		.native_output_ref = 0x15, .pid = parent.pid,
		.uid = parent.uid, .start_ticks = parent.start_time_ticks,
	};
	struct stage_director_company_intent_snapshot intent = {
		.present = true, .selection_epoch = 21, .sequence = 78,
		.observed_boot_ticks = kid.start_time_ticks,
		.host_revision = 4, .native_host_client_ref = host.native_client_ref,
		.mode = 0,
	};
	struct stage_director_company_birth_pidfd held = {0};
	stage_director_company_birth_pidfd_init(&held);
	const struct stage_director_company_birth_result birth =
		stage_director_company_birth_pidfd_record(&held, pidfd, child,
			&intent, &host, stage_director_read_linux_process, NULL);
	assert(birth.status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY);
	close(pidfd);
	/* Running ELF is the SAME inode as the current owner-only installed
	 * lab binary. This is evidence, not a trusted digest or actor grant. */
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE);

	/* Wrong home, world-writable ancestry and binary are rejected. */
	expect_status(stage_director_company_executable_check_owner_home(
		&held, "/tmp"), SD_COMPANY_EXE_UNSAFE_INSTALL);
	assert(chmod(lab, 0777) == 0);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_UNSAFE_INSTALL);
	assert(chmod(lab, 0700) == 0);
	assert(chmod(binary, 0777) == 0);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_UNSAFE_INSTALL);
	assert(chmod(binary, 0700) == 0);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE);

	/* Replacement at the approved path does not fool the inode check.
	 * The actual executing process still references the OLD inode. */
	assert(rename(binary, renamed) == 0);
	copy_binary("/usr/bin/sleep", binary);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_DIFFERENT_INODE);
	assert(unlink(binary) == 0);
	assert(symlink(renamed, binary) == 0);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_UNSAFE_INSTALL);
	assert(unlink(binary) == 0);
	assert(rename(renamed, binary) == 0);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE);

	/* Process generation mismatch, dead pidfd and exit all fail closed. */
	held.child_start_ticks++;
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_PROCESS_CHANGED);
	held.child_start_ticks--;
	assert(kill(child, SIGTERM) == 0);
	int status = 0;
	assert(waitpid(child, &status, 0) == child);
	const struct stage_director_company_executable_result after_exit =
		stage_director_company_executable_check_owner_home(&held, home);
	assert(after_exit.status == SD_COMPANY_EXE_PROCESS_CHANGED ||
		after_exit.status == SD_COMPANY_EXE_RUNNING_EXE_UNAVAILABLE);
	expect_no_authority(after_exit);

	stage_director_company_birth_pidfd_revoke(&held);
	expect_status(stage_director_company_executable_check_owner_home(
		&held, home), SD_COMPANY_EXE_NO_PIDFD);
	assert(unlink(binary) == 0);
	assert(rmdir(build) == 0);
	assert(rmdir(lab) == 0);
	assert(rmdir(home) == 0);
}
int main(void) {
	actual_exec_and_swap_test();
	puts("STAGE_DIRECTOR_COMPANY_M29: PASS running ELF inode, owner path, replacement and real pidfd; authority=NONE");
	return 0;
}
