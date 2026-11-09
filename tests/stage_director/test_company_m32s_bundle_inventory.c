#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_bundle_inventory.h"
#include "sway/stage_director_company_digest.h"

static void write_file(const char *path, const char *data, mode_t mode) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, mode);
    assert(fd >= 0);
    size_t len = strlen(data);
    assert(write(fd, data, len) == (ssize_t)len);
    assert(close(fd) == 0);
    assert(chmod(path, mode) == 0);
}

static void hex_file(const char *path, char hex[65]) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    assert(fd >= 0);
    assert(stage_director_company_sha256_fd(fd, hex));
    close(fd);
}

static void no_authority(struct stage_director_company_bundle_result r) {
    assert(!r.dependency_closure_verified);
    assert(!r.immutable_bundle_verified);
    assert(!r.approved_executable && !r.operator_authorized);
    assert(!r.launch_authenticated && !r.can_spawn);
    assert(!r.can_apply_scene && !r.can_accept_input);
    assert(!r.can_focus_keyboard && !r.can_show_popup);
}

int main(void) {
    char dir[] = "/tmp/company-m32s-test-XXXXXX";
    assert(mkdtemp(dir));
    char path[512];
    const char *folders[] = {"bin", "lib", "plugins"};
    const char *names[] = {"company-vector-probe", "libQt6CoreTest.so",
                           "libqwaylandTest.so"};
    const char *data[] = {"sample-map-exec-v1", "sample-qt-library-v1",
                          "sample-wayland-plugin-v1"};
    char files[3][512], hashes[3][65];
    for (int i = 0; i < 3; ++i) {
        snprintf(path, sizeof(path), "%s/%s", dir, folders[i]);
        assert(mkdir(path, 0700) == 0);
        snprintf(files[i], sizeof(files[i]), "%s/%s/%s",
            dir, folders[i], names[i]);
        write_file(files[i], data[i], i == 0 ? 0700 : 0600);
        hex_file(files[i], hashes[i]);
    }
    const int root = open(dir, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    assert(root >= 0);
    const uid_t uid = getuid();
    struct stage_director_company_bundle_entry entries[3] = {
        {SD_COMPANY_BUNDLE_EXECUTABLE, names[0], hashes[0]},
        {SD_COMPANY_BUNDLE_LIBRARY, names[1], hashes[1]},
        {SD_COMPANY_BUNDLE_QT_PLUGIN, names[2], hashes[2]},
    };
    struct stage_director_company_bundle_result r =
        stage_director_company_bundle_inventory_probe_fd(
            root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY);
    assert(r.listed_files_checked == 3 && r.listed_bytes_match_candidate);
    no_authority(r);

    r = stage_director_company_bundle_inventory_probe_fd(root, uid, NULL, 0);
    assert(r.status == SD_COMPANY_BUNDLE_UNCONFIGURED);
    no_authority(r);
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 2);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);
    no_authority(r);

    struct stage_director_company_bundle_entry invalid[4];
    memcpy(invalid, entries, sizeof(entries));
    invalid[3] = entries[2];
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, invalid, 4);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);
    invalid[3].basename = "../escape.so";
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, invalid, 4);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);
    invalid[3].basename = "/absolute.so";
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, invalid, 4);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);
    invalid[3].basename = "libmore.so";
    invalid[3].sha256_hex = "";
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, invalid, 4);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);

    const char *good = entries[1].sha256_hex;
    entries[1].sha256_hex =
        "0000000000000000000000000000000000000000000000000000000000000000";
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_INVALID_MANIFEST);
    entries[1].sha256_hex = good;
    char changed[65];
    strcpy(changed, hashes[1]);
    changed[0] = changed[0] == 'a' ? 'b' : 'a';
    entries[1].sha256_hex = changed;
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_HASH_MISMATCH);
    assert(r.listed_files_checked == 1);
    no_authority(r);
    entries[1].sha256_hex = good;

    r = stage_director_company_bundle_inventory_probe_fd(root,
        (uid_t)(uid + 1), entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_UNSAFE_ROOT);
    assert(chmod(files[1], 0666) == 0);
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_UNSAFE_FILE);
    assert(chmod(files[1], 0600) == 0);

    assert(unlink(files[2]) == 0);
    assert(symlink("../lib/libQt6CoreTest.so", files[2]) == 0);
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_UNSAFE_FILE);
    assert(unlink(files[2]) == 0);
    write_file(files[2], data[2], 0600);

    snprintf(path, sizeof(path), "%s/plugins", dir);
    assert(chmod(path, 0777) == 0);
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_UNSAFE_DIRECTORY);
    assert(chmod(path, 0700) == 0);
    r = stage_director_company_bundle_inventory_probe_fd(root, uid, entries, 3);
    assert(r.status == SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY);
    no_authority(r);

    close(root);
    for (int i = 0; i < 3; ++i) {
        assert(unlink(files[i]) == 0);
        snprintf(path, sizeof(path), "%s/%s", dir, folders[i]);
        assert(rmdir(path) == 0);
    }
    assert(rmdir(dir) == 0);
    puts("SD_COMPANY_M32S inventory=LISTED_ONLY "
         "missing_duplicate_path_swap=DENIED owner_modes=CHECKED "
         "qt_closure=UNPROVEN operator=OFF launch=OFF scene=OFF PASS");
    return 0;
}
