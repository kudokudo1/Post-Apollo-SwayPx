#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sway/stage_director_company_digest.h"
#include "sway/stage_director_company_executable.h"

static void assert_inert(struct stage_director_company_digest_result r) {
	assert(!r.executable_approved && !r.launch_authenticated);
	assert(!r.owner_verified && !r.can_pair_actor);
	assert(!r.can_render && !r.can_clip && !r.can_accept_input);
	assert(!r.can_focus && !r.can_show_popup);
}
static void expect(struct stage_director_company_digest_result r,
		enum stage_director_company_digest_status state) {
	if (r.status != state) fprintf(stderr, "M30a expected=%s got=%s\n",
		stage_director_company_digest_status_name(state),
		stage_director_company_digest_status_name(r.status));
	assert(r.status == state);
	assert(r.pinned_elf_hash_candidate ==
		(state == SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE));
	assert_inert(r);
}
static void hash_vectors(void) {
	char path[] = "/tmp/sd-m30-sha-XXXXXX";
	int fd = mkstemp(path);
	assert(fd >= 0);
	char hex[65];
	assert(stage_director_company_sha256_fd(fd, hex));
	assert(!strcmp(hex,
		"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
	assert(write(fd, "abc", 3) == 3);
	assert(stage_director_company_sha256_fd(fd, hex));
	assert(!strcmp(hex,
		"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
	assert(ftruncate(fd, 0) == 0);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);
}
static void copy_sleep(const char *to) {
	int in = open("/usr/bin/sleep", O_RDONLY | O_CLOEXEC);
	int out = open(to, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0700);
	assert(in >= 0 && out >= 0);
	char buf[65536];
	for (;;) {
		ssize_t n = read(in, buf, sizeof(buf));
		assert(n >= 0);
		if (!n) break;
		ssize_t pos = 0;
		while (pos < n) {
			ssize_t w = write(out, buf + pos, (size_t)(n - pos));
			assert(w > 0);
			pos += w;
		}
	}
	assert(fchmod(out, 0700) == 0);
	assert(close(in) == 0 && close(out) == 0);
}
static bool same_exe(pid_t pid, const char *binary) {
	struct stat actual = {0}, expected = {0};
	char proc[64];
	int n = snprintf(proc, sizeof(proc), "/proc/%d/exe", (int)pid);
	return n > 0 && (size_t)n < sizeof(proc) &&
		stat(proc, &actual) == 0 && stat(binary, &expected) == 0 &&
		actual.st_dev == expected.st_dev && actual.st_ino == expected.st_ino;
}
static void test_real_process(void) {
	assert(getuid() != 0 && getuid() == geteuid());
	char home[] = "/tmp/sd-company-m30-XXXXXX";
	assert(mkdtemp(home));
	char lab[PATH_MAX], build[PATH_MAX], binary[PATH_MAX], old[PATH_MAX];
	assert(snprintf(lab, sizeof(lab), "%s/postapollo-vector-lab", home) > 0);
	assert(snprintf(build, sizeof(build), "%s/company-vector-build", lab) > 0);
	assert(snprintf(binary, sizeof(binary), "%s/company-vector-probe", build) > 0);
	assert(snprintf(old, sizeof(old), "%s/previous", build) > 0);
	assert(mkdir(lab, 0700) == 0 && mkdir(build, 0700) == 0);
	copy_sleep(binary);
	pid_t child = fork();
	assert(child >= 0);
	if (!child) {
		char *const args[] = {binary, "30", NULL};
		execv(binary, args);
		_exit(127);
	}
	bool ready = false;
	for (int t = 0; t < 300; ++t) {
		if (same_exe(child, binary)) { ready = true; break; }
		usleep(10000);
	}
	assert(ready);
#if defined(SYS_pidfd_open)
	int fd = (int)syscall(SYS_pidfd_open, child, 0);
#else
	int fd = -1;
#endif
	assert(fd >= 0);
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
		.observed_boot_ticks = kid.start_time_ticks, .host_revision = 4,
		.native_host_client_ref = host.native_client_ref, .mode = 0,
	};
	struct stage_director_company_birth_pidfd held = {0};
	stage_director_company_birth_pidfd_init(&held);
	struct stage_director_company_birth_result birth =
		stage_director_company_birth_pidfd_record(&held, fd, child,
			&intent, &host, stage_director_read_linux_process, NULL);
	assert(birth.status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY);
	assert(close(fd) == 0);
	expect(stage_director_company_digest_check_system_home(&held),
		SD_COMPANY_DIGEST_UNPINNED);
	expect(stage_director_company_digest_check_owner_home(&held, home, ""),
		SD_COMPANY_DIGEST_UNPINNED);
	expect(stage_director_company_digest_check_owner_home(&held, home, "abcd"),
		SD_COMPANY_DIGEST_INVALID_PIN);
	char pin[65];
	fd = open(binary, O_RDONLY | O_CLOEXEC);
	assert(fd >= 0);
	assert(stage_director_company_sha256_fd(fd, pin));
	assert(close(fd) == 0);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE);
	char wrong[65];
	memcpy(wrong, pin, sizeof(wrong));
	wrong[0] = wrong[0] == 'a' ? 'b' : 'a';
	expect(stage_director_company_digest_check_owner_home(&held, home, wrong),
		SD_COMPANY_DIGEST_MISMATCH);
	/* A file-path swap can contain identical bytes; inode checks must
	 * reject it even when its hash is unchanged. */
	assert(rename(binary, old) == 0);
	copy_sleep(binary);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	assert(unlink(binary) == 0);
	assert(rename(old, binary) == 0);
	assert(chmod(lab, 0777) == 0);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	assert(chmod(lab, 0700) == 0);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE);
	assert(kill(child, SIGTERM) == 0);
	int status = 0;
	assert(waitpid(child, &status, 0) == child);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	stage_director_company_birth_pidfd_revoke(&held);
	expect(stage_director_company_digest_check_owner_home(&held, home, pin),
		SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	assert(unlink(binary) == 0);
	assert(rmdir(build) == 0 && rmdir(lab) == 0 && rmdir(home) == 0);
}
int main(void) {
	hash_vectors();
	test_real_process();
	puts("STAGE_DIRECTOR_COMPANY_M30A: PASS SHA-256 vectors, real pidfd, pin mismatch, inode swap, revocation; authority=NONE");
	return 0;
}
