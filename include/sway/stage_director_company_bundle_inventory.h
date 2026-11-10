#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_BUNDLE_INVENTORY_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_BUNDLE_INVENTORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * M32s // READ-ONLY artifact inventory preflight, never authorization.
 *
 * A future reviewed, immutable bundle would contain three relative
 * directories: bin/, lib/ and plugins/. This internal test/inventory seam
 * checks only explicitly listed files against caller-supplied trusted
 * manifest bytes. No approved production manifest or bundle root exists.
 *
 * A successful match does NOT prove dynamic loader closure, loaded Qt
 * objects, signing/provenance, immutability, operator consent or launch
 * custody. It MUST NOT grant scene, keyboard, pointer, popup or spawn.
 */
#define SD_COMPANY_BUNDLE_MAX_ENTRIES 24
#define SD_COMPANY_BUNDLE_NAME_MAX 95

enum stage_director_company_bundle_kind {
    SD_COMPANY_BUNDLE_EXECUTABLE = 0,
    SD_COMPANY_BUNDLE_LIBRARY = 1,
    SD_COMPANY_BUNDLE_QT_PLUGIN = 2,
};

struct stage_director_company_bundle_entry {
    enum stage_director_company_bundle_kind kind;
    const char *basename; /* one component, never a slash/path */
    const char *sha256_hex; /* exactly 64 lowercase nonzero hex digits */
};

enum stage_director_company_bundle_status {
    SD_COMPANY_BUNDLE_UNCONFIGURED,
    SD_COMPANY_BUNDLE_INVALID_MANIFEST,
    SD_COMPANY_BUNDLE_UNSAFE_ROOT,
    SD_COMPANY_BUNDLE_UNSAFE_DIRECTORY,
    SD_COMPANY_BUNDLE_UNSAFE_FILE,
    SD_COMPANY_BUNDLE_HASH_UNAVAILABLE,
    SD_COMPANY_BUNDLE_HASH_MISMATCH,
    SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY,
};

struct stage_director_company_bundle_result {
    enum stage_director_company_bundle_status status;
    size_t listed_files_checked;
    bool listed_bytes_match_candidate;
    /* No authority fields may ever be inferred from a matched inventory. */
    bool dependency_closure_verified;
    bool immutable_bundle_verified;
    bool approved_executable;
    bool operator_authorized;
    bool launch_authenticated;
    bool can_spawn;
    bool can_apply_scene;
    bool can_accept_input;
    bool can_focus_keyboard;
    bool can_show_popup;
};

/*
 * Internal test / future-review seam: bundle_fd is BORROWED. Expected UID
 * must come from a trusted installer/owner, not Wayland, QML or environment.
 * Paths use openat/O_NOFOLLOW beneath this already-opened directory, but
 * the *root directory's provenance* remains a separately unverified gate.
 * A caller-supplied manifest, FD or uid grants NO production authority.
 */
struct stage_director_company_bundle_result
stage_director_company_bundle_inventory_probe_fd(
    int bundle_fd, uid_t expected_owner_uid,
    const struct stage_director_company_bundle_entry *entries,
    size_t entry_count);

#endif
