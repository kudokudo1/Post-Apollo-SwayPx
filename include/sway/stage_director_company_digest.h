#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_DIGEST_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_DIGEST_H

#include <stdbool.h>
#include "sway/stage_director_company_birth_pidfd.h"

/*
 * M30a: READ-ONLY content evidence, not binary approval or actor authority.
 * Production manifest is compiled into the compositor and starts UNPINNED.
 * The verifier never reads a digest, path, or approval from Wayland/QML,
 * argv, the child's environment, or a writable runtime config file.
 */
enum stage_director_company_digest_status {
	SD_COMPANY_DIGEST_UNPINNED,
	SD_COMPANY_DIGEST_INVALID_PIN,
	SD_COMPANY_DIGEST_INODE_UNVERIFIED,
	SD_COMPANY_DIGEST_HASH_UNAVAILABLE,
	SD_COMPANY_DIGEST_MISMATCH,
	SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE,
};

struct stage_director_company_digest_result {
	enum stage_director_company_digest_status status;
	bool pinned_elf_hash_candidate;
	/* Always false: a file hash is not running-memory / Qt plugin proof. */
	bool executable_approved;
	bool launch_authenticated;
	bool owner_verified;
	bool can_pair_actor;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_focus;
	bool can_show_popup;
};

/* Independent SHA-256 implementation with bounded pread and inode stability
 * checks. Used for test vectors and to inspect the kernel /proc/PID/exe fd. */
bool stage_director_company_sha256_fd(int fd, char hex[65]);

/* TEST/INTERNAL seam: the pin is a trusted test fixture, never a client
 * argument. Production must ONLY call check_system_home() below. */
struct stage_director_company_digest_result
stage_director_company_digest_check_owner_home(
	const struct stage_director_company_birth_pidfd *held,
	const char *owner_home, const char *trusted_sha256_hex);

/* Production: fixed compositor-owned manifest, blank until operator approval.
 * Returns UNPINNED by default and grants no scene/seat authority even if
 * an independently approved digest is embedded in a future build. */
struct stage_director_company_digest_result
stage_director_company_digest_check_system_home(
	const struct stage_director_company_birth_pidfd *held);

const char *stage_director_company_digest_status_name(
	enum stage_director_company_digest_status status);

#endif
