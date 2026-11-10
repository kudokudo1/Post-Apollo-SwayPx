#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_PRE_SPAWN_CUSTODY_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_PRE_SPAWN_CUSTODY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sway/stage_director_company_bundle_inventory.h"
#include "sway/stage_director_company_launch_request.h"

/* M32t // A READ-ONLY pre-spawn CANDIDATE, not a process launcher.
 * A held descriptor pins an inode, NOT a trusted runtime, loader closure,
 * preapproved executable, operator gesture, process or Wayland scene.
 * There is no production caller and no executable FD export/spawn API.
 * A future trusted launch transaction must be designed separately. */
#define SD_COMPANY_PRE_SPAWN_MAX_NS UINT64_C(5000000000)

struct stage_director_company_pre_spawn_entry {
    enum stage_director_company_bundle_kind kind;
    char basename[SD_COMPANY_BUNDLE_NAME_MAX + 1];
    char sha256_hex[65];
};

struct stage_director_company_pre_spawn_custody {
    bool initialized;
    bool held;
    int bundle_fd;       /* owned CLOEXEC copy of a pre-opened root */
    int executable_fd;   /* owned CLOEXEC read-only verified candidate */
    uid_t owner_uid;
    uint64_t last_generation;
    uint64_t generation;
    uint64_t reserved_ns;
    uint64_t expires_ns;
    struct stat root_at_capture;
    struct stat executable_at_capture;
    struct stage_director_company_launch_request_identity identity;
    size_t entry_count;
    struct stage_director_company_pre_spawn_entry
        manifest[SD_COMPANY_BUNDLE_MAX_ENTRIES];
};

enum stage_director_company_pre_spawn_state {
    SD_COMPANY_PRE_SPAWN_ABSENT,
    SD_COMPANY_PRE_SPAWN_STALE_GENERATION,
    SD_COMPANY_PRE_SPAWN_REQUEST_CHANGED,
    SD_COMPANY_PRE_SPAWN_EXPIRED,
    SD_COMPANY_PRE_SPAWN_IMAGE_CHANGED,
    SD_COMPANY_PRE_SPAWN_LISTED_BYTES_CANDIDATE_ONLY,
};

struct stage_director_company_pre_spawn_receipt {
    enum stage_director_company_pre_spawn_state state;
    bool request_and_listed_bytes_candidate;
    /* Every grant MUST remain false, including an exact synthetic match. */
    bool dependencies_closed;
    bool bundle_immutable;
    bool executable_approved;
    bool operator_authenticated;
    bool launch_authenticated;
    bool can_spawn;
    bool can_apply_scene;
    bool can_accept_input;
    bool can_focus_keyboard;
    bool can_show_popup;
};

void stage_director_company_pre_spawn_init(
    struct stage_director_company_pre_spawn_custody *custody);

/* The root FD and manifest are BORROWED and never treated as trusted on
 * their own. Requires a current M32r request and M32s listed-file match.
 * There is NO scene, launch, physical gesture or production issuer.
 *
 * `now_ns` is from a future compositor CLOCK_BOOTTIME caller, not wall
 * clock, client input or a runtime environment. Tests use synthetic ticks.
 * This function does not read time or spawn a process. */
bool stage_director_company_pre_spawn_capture(
    struct stage_director_company_pre_spawn_custody *custody,
    struct stage_director_company_launch_request *request,
    uint64_t request_generation,
    const struct stage_director_company_launch_request_identity *observed,
    int borrowed_bundle_fd, uid_t expected_owner_uid,
    const struct stage_director_company_bundle_entry *manifest,
    size_t manifest_count, uint64_t now_ns);

/* Re-read original root/executable FDs, the live root path and every
 * listed lib/plugin. Changed request, expiry, replacement or changed
 * bytes revoke the held candidate. Never creates launch authority. */
struct stage_director_company_pre_spawn_receipt
stage_director_company_pre_spawn_probe(
    struct stage_director_company_pre_spawn_custody *custody,
    struct stage_director_company_launch_request *request,
    uint64_t request_generation,
    const struct stage_director_company_launch_request_identity *observed,
    uint64_t now_ns);

/* Exact generation only. An old asynchronous revoke may never close a
 * newer session's descriptors. Releasing never resets replay floor. */
bool stage_director_company_pre_spawn_revoke(
    struct stage_director_company_pre_spawn_custody *custody,
    uint64_t request_generation);

#endif
