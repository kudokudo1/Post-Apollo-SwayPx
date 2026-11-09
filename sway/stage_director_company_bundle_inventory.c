#define _GNU_SOURCE
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_bundle_inventory.h"
#include "sway/stage_director_company_digest.h"

/* Bundle inventory is diagnostic only. Nothing in this module creates
 * operator, launch, library-closure or scene authority. */
static struct stage_director_company_bundle_result result(
        enum stage_director_company_bundle_status status,
        size_t count) {
    return (struct stage_director_company_bundle_result) {
        .status = status,
        .listed_files_checked = count,
        .listed_bytes_match_candidate =
            status == SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY,
    };
}

static bool safe_name(const char *name) {
    if (!name || !name[0]) return false;
    size_t i = 0;
    for (; name[i] && i <= SD_COMPANY_BUNDLE_NAME_MAX; ++i) {
        const char c = name[i];
        if (!((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-' || c == '.')) return false;
    }
    return i && i <= SD_COMPANY_BUNDLE_NAME_MAX &&
        name[0] != '.' && !strstr(name, "..");
}

static bool valid_digest(const char *hex) {
    if (!hex) return false;
    bool nonzero = false;
    for (size_t i = 0; i < 64; ++i) {
        const char c = hex[i];
        if (!c) return false;
        if (c >= '1' && c <= '9') nonzero = true;
        else if (c >= 'a' && c <= 'f') nonzero = true;
        else if (c != '0') return false;
    }
    return nonzero && hex[64] == '\0';
}

static const char *directory_for(enum stage_director_company_bundle_kind k) {
    switch (k) {
    case SD_COMPANY_BUNDLE_EXECUTABLE: return "bin";
    case SD_COMPANY_BUNDLE_LIBRARY: return "lib";
    case SD_COMPANY_BUNDLE_QT_PLUGIN: return "plugins";
    }
    return NULL;
}

static bool safe_dir(const struct stat *st, uid_t uid) {
    return S_ISDIR(st->st_mode) && st->st_uid == uid &&
        !(st->st_mode & (S_IWGRP | S_IWOTH));
}

static bool safe_file(const struct stat *st, uid_t uid, bool executable) {
    return S_ISREG(st->st_mode) && st->st_uid == uid &&
        st->st_nlink == 1 && st->st_size > 0 &&
        !(st->st_mode & (S_IWGRP | S_IWOTH | S_ISUID | S_ISGID)) &&
        (!executable || (st->st_mode & S_IXUSR));
}

static bool same_file_metadata(const struct stat *a, const struct stat *b) {
    return a->st_dev == b->st_dev && a->st_ino == b->st_ino &&
        a->st_uid == b->st_uid && a->st_mode == b->st_mode &&
        a->st_nlink == b->st_nlink && a->st_size == b->st_size &&
        a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
        a->st_mtim.tv_nsec == b->st_mtim.tv_nsec &&
        a->st_ctim.tv_sec == b->st_ctim.tv_sec &&
        a->st_ctim.tv_nsec == b->st_ctim.tv_nsec;
}

static bool valid_manifest(
        const struct stage_director_company_bundle_entry *entries,
        size_t count) {
    if (!entries || count < 3 || count > SD_COMPANY_BUNDLE_MAX_ENTRIES)
        return false;
    size_t executables = 0, libraries = 0, plugins = 0;
    for (size_t i = 0; i < count; ++i) {
        const struct stage_director_company_bundle_entry *e = &entries[i];
        if (!directory_for(e->kind) || !safe_name(e->basename) ||
                !valid_digest(e->sha256_hex)) return false;
        if (e->kind == SD_COMPANY_BUNDLE_EXECUTABLE) {
            if (strcmp(e->basename, "company-vector-probe") != 0)
                return false;
            ++executables;
        } else if (e->kind == SD_COMPANY_BUNDLE_LIBRARY) {
            ++libraries;
        } else {
            ++plugins;
        }
        for (size_t j = 0; j < i; ++j) {
            if (entries[j].kind == e->kind &&
                    strcmp(entries[j].basename, e->basename) == 0)
                return false;
        }
    }
    return executables == 1 && libraries >= 1 && plugins >= 1;
}

struct stage_director_company_bundle_result
stage_director_company_bundle_inventory_probe_fd(
        int bundle_fd, uid_t expected_owner_uid,
        const struct stage_director_company_bundle_entry *entries,
        size_t count) {
    if (!entries || !count)
        return result(SD_COMPANY_BUNDLE_UNCONFIGURED, 0);
    if (!valid_manifest(entries, count))
        return result(SD_COMPANY_BUNDLE_INVALID_MANIFEST, 0);

    struct stat root = {0};
    if (bundle_fd < 0 || fstat(bundle_fd, &root) != 0 ||
            !safe_dir(&root, expected_owner_uid))
        return result(SD_COMPANY_BUNDLE_UNSAFE_ROOT, 0);

    size_t checked = 0;
    for (size_t i = 0; i < count; ++i) {
        const struct stage_director_company_bundle_entry *e = &entries[i];
        int dir = openat(bundle_fd, directory_for(e->kind),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        struct stat dir_st = {0};
        if (dir < 0) return result(SD_COMPANY_BUNDLE_UNSAFE_DIRECTORY, checked);
        if (fstat(dir, &dir_st) != 0 ||
                !safe_dir(&dir_st, expected_owner_uid)) {
            close(dir);
            return result(SD_COMPANY_BUNDLE_UNSAFE_DIRECTORY, checked);
        }
        const int fd = openat(dir, e->basename,
            O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
        struct stat before = {0}, after = {0}, current = {0};
        const bool exec = e->kind == SD_COMPANY_BUNDLE_EXECUTABLE;
        if (fd < 0 || fstat(fd, &before) != 0 ||
                !safe_file(&before, expected_owner_uid, exec)) {
            if (fd >= 0) close(fd);
            close(dir);
            return result(SD_COMPANY_BUNDLE_UNSAFE_FILE, checked);
        }
        char actual[65] = {0};
        const bool hashed = stage_director_company_sha256_fd(fd, actual);
        const bool stable = fstat(fd, &after) == 0 &&
            fstatat(dir, e->basename, &current, AT_SYMLINK_NOFOLLOW) == 0 &&
            same_file_metadata(&before, &after) &&
            same_file_metadata(&after, &current);
        close(fd);
        close(dir);
        if (!hashed || !stable)
            return result(SD_COMPANY_BUNDLE_HASH_UNAVAILABLE, checked);
        if (strcmp(actual, e->sha256_hex) != 0)
            return result(SD_COMPANY_BUNDLE_HASH_MISMATCH, checked);
        ++checked;
    }
    /* File-list equality is NOT loader closure or immutable custody. */
    return result(SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY, checked);
}
