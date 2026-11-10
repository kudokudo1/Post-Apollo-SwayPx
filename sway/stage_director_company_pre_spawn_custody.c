#define _GNU_SOURCE
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_pre_spawn_custody.h"
#include "sway/stage_director_company_digest.h"

/* No exec, spawn, process creation, environment or privilege handoff.
 * The only output is an inert pre-spawn observation. */
static struct stage_director_company_pre_spawn_receipt receipt(
        enum stage_director_company_pre_spawn_state state) {
    return (struct stage_director_company_pre_spawn_receipt) {
        .state = state,
        .request_and_listed_bytes_candidate =
            state == SD_COMPANY_PRE_SPAWN_LISTED_BYTES_CANDIDATE_ONLY,
    };
}

static bool same_metadata(const struct stat *a, const struct stat *b) {
    return a->st_dev == b->st_dev && a->st_ino == b->st_ino &&
        a->st_uid == b->st_uid && a->st_mode == b->st_mode &&
        a->st_nlink == b->st_nlink && a->st_size == b->st_size &&
        a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
        a->st_mtim.tv_nsec == b->st_mtim.tv_nsec &&
        a->st_ctim.tv_sec == b->st_ctim.tv_sec &&
        a->st_ctim.tv_nsec == b->st_ctim.tv_nsec;
}

static bool safe_executable(const struct stat *st, uid_t owner) {
    return S_ISREG(st->st_mode) && st->st_uid == owner &&
        st->st_nlink == 1 && st->st_size > 0 &&
        (st->st_mode & S_IXUSR) &&
        !(st->st_mode & (S_IWGRP | S_IWOTH | S_ISUID | S_ISGID));
}

void stage_director_company_pre_spawn_init(
        struct stage_director_company_pre_spawn_custody *c) {
    if (!c || c->initialized) return;
    memset(c, 0, sizeof(*c));
    c->initialized = true;
    c->bundle_fd = -1;
    c->executable_fd = -1;
}

bool stage_director_company_pre_spawn_revoke(
        struct stage_director_company_pre_spawn_custody *c,
        uint64_t generation) {
    if (!c || !c->initialized || !c->held || !generation ||
            c->generation != generation) return false;
    close(c->executable_fd);
    close(c->bundle_fd);
    const uint64_t floor = c->last_generation;
    memset(c, 0, sizeof(*c));
    c->initialized = true;
    c->last_generation = floor;
    c->bundle_fd = -1;
    c->executable_fd = -1;
    return true;
}

/* Read-only inventory scope is exactly the explicitly listed manifest.
 * Dynamic-loader/Qt dependency closure remains UNPROVEN. */
static bool listed_image_current(
        const struct stage_director_company_pre_spawn_custody *c) {
    struct stage_director_company_bundle_entry
        entries[SD_COMPANY_BUNDLE_MAX_ENTRIES] = {0};
    for (size_t i = 0; i < c->entry_count; ++i) {
        entries[i] = (struct stage_director_company_bundle_entry) {
            .kind = c->manifest[i].kind,
            .basename = c->manifest[i].basename,
            .sha256_hex = c->manifest[i].sha256_hex,
        };
    }
    const struct stage_director_company_bundle_result image =
        stage_director_company_bundle_inventory_probe_fd(c->bundle_fd,
            c->owner_uid, entries, c->entry_count);
    if (image.status != SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY ||
            !image.listed_bytes_match_candidate ||
            image.listed_files_checked != c->entry_count ||
            image.dependency_closure_verified || image.immutable_bundle_verified ||
            image.approved_executable || image.operator_authorized ||
            image.launch_authenticated || image.can_spawn ||
            image.can_apply_scene || image.can_accept_input ||
            image.can_focus_keyboard || image.can_show_popup)
        return false;
    return true;
}

static bool open_executable(int root_fd, uid_t owner,
        int *out_fd, struct stat *out_st) {
    int bin = openat(root_fd, "bin",
        O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (bin < 0) return false;
    int executable = openat(bin, "company-vector-probe",
        O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    close(bin);
    struct stat info = {0};
    if (executable < 0) return false;
    if (fstat(executable, &info) != 0 ||
            !safe_executable(&info, owner)) {
        close(executable);
        return false;
    }
    *out_fd = executable;
    *out_st = info;
    return true;
}

bool stage_director_company_pre_spawn_capture(
        struct stage_director_company_pre_spawn_custody *c,
        struct stage_director_company_launch_request *request,
        uint64_t generation,
        const struct stage_director_company_launch_request_identity *observed,
        int root_fd, uid_t owner,
        const struct stage_director_company_bundle_entry *manifest,
        size_t count, uint64_t now_ns) {
    if (!c || !c->initialized || c->held || !request ||
            !generation || generation <= c->last_generation ||
            !now_ns || !manifest || count < 3 ||
            count > SD_COMPANY_BUNDLE_MAX_ENTRIES)
        return false;
    const struct stage_director_company_launch_request_receipt req =
        stage_director_company_launch_request_probe(
            request, generation, observed, now_ns);
    if (req.state != SD_COMPANY_REQUEST_CONTEXT_ONLY ||
            !req.current_request_context || req.operator_authenticated ||
            req.launch_authenticated || req.scene_lease_issued ||
            req.can_apply_scene || req.can_accept_input ||
            req.can_focus_keyboard || req.can_show_popup ||
            request->expires_ns <= now_ns)
        return false;

    const struct stage_director_company_bundle_result image =
        stage_director_company_bundle_inventory_probe_fd(
            root_fd, owner, manifest, count);
    if (image.status != SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY ||
            !image.listed_bytes_match_candidate ||
            image.listed_files_checked != count ||
            image.dependency_closure_verified || image.immutable_bundle_verified ||
            image.approved_executable || image.operator_authorized ||
            image.launch_authenticated || image.can_spawn ||
            image.can_apply_scene || image.can_accept_input ||
            image.can_focus_keyboard || image.can_show_popup)
        return false;

    int held_root = fcntl(root_fd, F_DUPFD_CLOEXEC, 3);
    if (held_root < 0) return false;
    struct stat root_meta = {0};
    int held_exe = -1;
    struct stat exe_meta = {0};
    if (fstat(held_root, &root_meta) != 0 ||
            !open_executable(held_root, owner, &held_exe, &exe_meta)) {
        close(held_root);
        return false;
    }

    /* No caller may swap the executable between the two listed-file
     * checks. This pins an inode for observation only, NOT for fexecve. */
    struct stage_director_company_pre_spawn_custody snapshot = {0};
    snapshot.initialized = true;
    snapshot.held = true;
    snapshot.bundle_fd = held_root;
    snapshot.executable_fd = held_exe;
    snapshot.owner_uid = owner;
    snapshot.entry_count = count;
    for (size_t i = 0; i < count; ++i) {
        snapshot.manifest[i].kind = manifest[i].kind;
        const size_t n = strlen(manifest[i].basename);
        memcpy(snapshot.manifest[i].basename, manifest[i].basename, n + 1);
        memcpy(snapshot.manifest[i].sha256_hex, manifest[i].sha256_hex, 65);
    }
    const struct stage_director_company_launch_request_receipt req_again =
        stage_director_company_launch_request_probe(
            request, generation, observed, now_ns);
    struct stat root_again = {0}, exe_again = {0}, path_exe = {0};
    int bin = openat(held_root, "bin",
        O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    const bool same_path = bin >= 0 &&
        fstatat(bin, "company-vector-probe", &path_exe,
            AT_SYMLINK_NOFOLLOW) == 0;
    if (bin >= 0) close(bin);
    const bool still_current =
        req_again.state == SD_COMPANY_REQUEST_CONTEXT_ONLY &&
        fstat(held_root, &root_again) == 0 &&
        fstat(held_exe, &exe_again) == 0 &&
        same_metadata(&root_meta, &root_again) &&
        same_metadata(&exe_meta, &exe_again) &&
        same_path && same_metadata(&exe_meta, &path_exe) &&
        listed_image_current(&snapshot);
    if (!still_current) {
        close(held_exe);
        close(held_root);
        return false;
    }
    snapshot.last_generation = generation;
    snapshot.generation = generation;
    snapshot.reserved_ns = now_ns;
    snapshot.expires_ns = now_ns > UINT64_MAX - SD_COMPANY_PRE_SPAWN_MAX_NS
        ? UINT64_MAX : now_ns + SD_COMPANY_PRE_SPAWN_MAX_NS;
    if (snapshot.expires_ns > request->expires_ns)
        snapshot.expires_ns = request->expires_ns;
    snapshot.identity = *observed;
    snapshot.root_at_capture = root_meta;
    snapshot.executable_at_capture = exe_meta;
    *c = snapshot;
    return true;
}

struct stage_director_company_pre_spawn_receipt
stage_director_company_pre_spawn_probe(
        struct stage_director_company_pre_spawn_custody *c,
        struct stage_director_company_launch_request *request,
        uint64_t generation,
        const struct stage_director_company_launch_request_identity *observed,
        uint64_t now_ns) {
    if (!c || !c->initialized || !c->held)
        return receipt(SD_COMPANY_PRE_SPAWN_ABSENT);
    /* Foreign/stale asynchronous callbacks cannot revoke a new owner. */
    if (!generation || generation != c->generation)
        return receipt(SD_COMPANY_PRE_SPAWN_STALE_GENERATION);
    if (!now_ns || now_ns < c->reserved_ns ||
            now_ns >= c->expires_ns) {
        stage_director_company_pre_spawn_revoke(c, generation);
        return receipt(SD_COMPANY_PRE_SPAWN_EXPIRED);
    }
    const struct stage_director_company_launch_request_receipt req =
        stage_director_company_launch_request_probe(
            request, generation, observed, now_ns);
    if (req.state != SD_COMPANY_REQUEST_CONTEXT_ONLY ||
            !req.current_request_context ||
            req.operator_authenticated || req.launch_authenticated ||
            req.scene_lease_issued || req.can_apply_scene ||
            req.can_accept_input || req.can_focus_keyboard ||
            req.can_show_popup) {
        stage_director_company_pre_spawn_revoke(c, generation);
        return receipt(SD_COMPANY_PRE_SPAWN_REQUEST_CHANGED);
    }
    struct stat root = {0}, executable = {0}, current_path = {0};
    int bin = openat(c->bundle_fd, "bin",
        O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    const bool path_ok = bin >= 0 &&
        fstatat(bin, "company-vector-probe", &current_path,
            AT_SYMLINK_NOFOLLOW) == 0;
    if (bin >= 0) close(bin);
    if (fstat(c->bundle_fd, &root) != 0 ||
            fstat(c->executable_fd, &executable) != 0 ||
            !same_metadata(&c->root_at_capture, &root) ||
            !same_metadata(&c->executable_at_capture, &executable) ||
            !path_ok || !same_metadata(&executable, &current_path) ||
            !listed_image_current(c)) {
        stage_director_company_pre_spawn_revoke(c, generation);
        return receipt(SD_COMPANY_PRE_SPAWN_IMAGE_CHANGED);
    }
    return receipt(SD_COMPANY_PRE_SPAWN_LISTED_BYTES_CANDIDATE_ONLY);
}
