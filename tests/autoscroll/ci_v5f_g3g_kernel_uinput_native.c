/*
 * V5F-G3G: disposable GITHUB RUNNER ONLY: create a real kernel uinput
 * pseudo-mouse and read its real udev/libinput identity.
 *
 * This runs under sudo exclusively inside a Github-hosted ephemeral runner.
 * Creates and destroys only its own /dev/uinput device. NEVER generates
 * pointer motion, buttons, axes or frames. No compositor binary modification.
 *
 * Even if the uinput device advertises BUS_USB, it is software-created.
 * This probe MUST NOT claim real HID provenance or positive authorization.
 */
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <libinput.h>
#include <libudev.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "sway/input/post_apollo_autoscroll_topology.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"

static int open_restricted(const char *path, int flags, void *user_data) {
    (void)user_data;
    int fd = open(path, flags | O_CLOEXEC);
    return fd < 0 ? -errno : fd;
}
static void close_restricted(int fd, void *user_data) {
    (void)user_data;
    close(fd);
}
static const struct libinput_interface li_interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static bool is_ephemeral_ci(void) {
    const char *ci = getenv("CI");
    const char *gha = getenv("GITHUB_ACTIONS");
    const char *run = getenv("RUNNER_ENVIRONMENT");
    const char *tmp = getenv("RUNNER_TEMP");
    const char *gate = getenv("PA_G3G_NATIVE_UINPUT_NEGATIVE");
    return ci != NULL && strcmp(ci, "true") == 0 &&
        gha != NULL && strcmp(gha, "true") == 0 &&
        run != NULL && strcmp(run, "github-hosted") == 0 &&
        tmp != NULL && strcmp(tmp, "/home/runner/work/_temp") == 0 &&
        gate != NULL && strcmp(gate, "1") == 0 &&
        geteuid() == 0;
}

static bool event_for_input(const char *input, char *event, size_t event_n) {
    char dir[256];
    int n = snprintf(dir, sizeof(dir), "/sys/devices/virtual/input/%s",
        input);
    if (n < 0 || (size_t)n >= sizeof(dir)) {
        return false;
    }
    DIR *d = opendir(dir);
    if (d == NULL) {
        return false;
    }
    bool found = false;
    struct dirent *ent = NULL;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0) {
            continue;
        }
        bool digits = ent->d_name[5] != '\0';
        for (const char *c = ent->d_name + 5; *c != '\0'; ++c) {
            if (*c < '0' || *c > '9') {
                digits = false;
            }
        }
        if (!digits || strlen(ent->d_name) >= event_n) {
            continue;
        }
        strcpy(event, ent->d_name);
        found = true;
        break;
    }
    closedir(d);
    return found;
}

static void tick_wait(void) {
    const struct timespec t = { .tv_sec = 0, .tv_nsec = 100000000L };
    (void)nanosleep(&t, NULL);
}

int main(void) {
    if (!is_ephemeral_ci()) {
        fputs("BLOCKED_PA_G3G_NOT_GITHUB_HOSTED_EPHEMERAL_ROOT_CI\n", stderr);
        return 2;
    }
    int fd = -1;
    bool created = false;
    struct udev *udev = NULL;
    struct libinput *li = NULL;
    struct libinput_device *li_device = NULL;
    int rc = EXIT_FAILURE;
    char input_name[128] = {0};
    char event_name[64] = {0};
    char event_sysfs[320] = {0};
    char event_path[80] = {0};
    char real_sysfs[512] = {0};
    char real_devnode[128] = {0};

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        perror("PA_G3G_NATIVE_UINPUT_OPEN_FAILED");
        goto done;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
        ioctl(fd, UI_SET_EVBIT, EV_REL) < 0 ||
        ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0 ||
        ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE) < 0 ||
        ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT) < 0 ||
        ioctl(fd, UI_SET_RELBIT, REL_X) < 0 ||
        ioctl(fd, UI_SET_RELBIT, REL_Y) < 0 ||
        ioctl(fd, UI_SET_RELBIT, REL_WHEEL) < 0) {
        perror("PA_G3G_NATIVE_UINPUT_CAPABILITIES_FAILED");
        goto done;
    }

    struct uinput_setup setup = {0};
    setup.id.bustype = BUS_USB; /* THIS IS A FAKE CLAIM, NOT USB HARDWARE. */
    setup.id.vendor = 0x1234u;
    setup.id.product = 0x5432u;
    setup.id.version = 1u;
    snprintf(setup.name, sizeof(setup.name),
        "PA-G3G-VIRTUAL-USB-CLAIM-NOT-HARDWARE");
    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0 ||
        ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("PA_G3G_NATIVE_UINPUT_CREATE_FAILED");
        goto done;
    }
    created = true;
    if (ioctl(fd, UI_GET_SYSNAME(sizeof(input_name)), input_name) < 0 ||
        strncmp(input_name, "input", 5) != 0) {
        perror("PA_G3G_NATIVE_UINPUT_SYSNAME_FAILED");
        goto done;
    }

    for (int attempt = 0; attempt < 100; ++attempt) {
        if (event_for_input(input_name, event_name, sizeof(event_name))) {
            break;
        }
        tick_wait();
    }
    if (event_name[0] == '\0' ||
        snprintf(event_sysfs, sizeof(event_sysfs),
            "/sys/devices/virtual/input/%s/%s",
            input_name, event_name) >= (int)sizeof(event_sysfs) ||
        snprintf(event_path, sizeof(event_path),
            "/dev/input/%s", event_name) >= (int)sizeof(event_path)) {
        fputs("PA_G3G_NATIVE_UINPUT_EVENT_SYSPATH_MISSING\n", stderr);
        goto done;
    }

    udev = udev_new();
    if (udev == NULL) {
        fputs("PA_G3G_NATIVE_UDEV_CONTEXT_MISSING\n", stderr);
        goto done;
    }

    /* System udev normally creates /dev/input/eventN; never mknod, change
     * host permissions, create more devices, or fallback to fake evidence. */
    for (int attempt = 0; attempt < 100; ++attempt) {
        struct udev_device *node = udev_device_new_from_syspath(
            udev, event_sysfs);
        if (node != NULL) {
            const char *syspath = udev_device_get_syspath(node);
            const char *devnode = udev_device_get_devnode(node);
            if (syspath != NULL && devnode != NULL &&
                strcmp(devnode, event_path) == 0 &&
                strlen(syspath) < sizeof(real_sysfs) &&
                strlen(devnode) < sizeof(real_devnode) &&
                access(devnode, R_OK) == 0) {
                strcpy(real_sysfs, syspath);
                strcpy(real_devnode, devnode);
                udev_device_unref(node);
                break;
            }
            udev_device_unref(node);
        }
        tick_wait();
    }

    if (real_sysfs[0] == '\0' || real_devnode[0] == '\0') {
        fputs("PA_G3G_NATIVE_UDEV_NODE_NOT_READY_NO_FAKE_PASS\n", stderr);
        goto done;
    }
    if (strstr(real_sysfs, "/virtual/") == NULL ||
        pa_autoscroll_kernel_tree_candidate(real_sysfs, real_devnode)) {
        fputs("PA_G3G_VIRTUAL_KERNEL_PATH_FALSELY_ADMITTED\n", stderr);
        goto done;
    }
    printf("PA_G3G_REAL_KERNEL_UINPUT_VIRTUAL_SYSFS_DENIED_PASS "
        "fake_bus_USB=1 real_kernel_event=1 topology_candidate=0\n");
    fflush(stdout);

    li = libinput_path_create_context(&li_interface, NULL);
    if (li == NULL) {
        fputs("PA_G3G_REAL_LIBINPUT_CONTEXT_CREATION_FAILED\n", stderr);
        goto done;
    }
    /* A real libinput context, not a caller-forged structure. */
    li_device = libinput_path_add_device(li, real_devnode);
    if (li_device == NULL ||
        !libinput_device_has_capability(li_device, LIBINPUT_DEVICE_CAP_POINTER)) {
        fputs("PA_G3G_REAL_LIBINPUT_POINTER_NOT_OBSERVED_NO_FAKE_PASS\n",
            stderr);
        goto done;
    }
    const char *native_name = libinput_device_get_name(li_device);
    struct udev_device *native = libinput_device_get_udev_device(li_device);
    if (native == NULL || native_name == NULL ||
        strcmp(native_name, setup.name) != 0) {
        if (native != NULL) {
            udev_device_unref(native);
        }
        fputs("PA_G3G_REAL_LIBINPUT_UDEV_MISMATCH\n", stderr);
        goto done;
    }
    const char *native_syspath = udev_device_get_syspath(native);
    const char *native_node = udev_device_get_devnode(native);
    bool same_origin = native_syspath != NULL && native_node != NULL &&
        strcmp(native_syspath, real_sysfs) == 0 &&
        strcmp(native_node, real_devnode) == 0;
    udev_device_unref(native);
    if (!same_origin) {
        fputs("PA_G3G_REAL_LIBINPUT_DEVICE_NOT_SAME_KERNEL_NODE\n", stderr);
        goto done;
    }

    /* The source policy already requires coherent udev topology. These
     * libinput pointers are real; the kernel node is explicitly virtual.
     * We cannot substitute "wlr_input_device_is_libinput" here: this test
     * does not start wlroots input backend or a compositor button callback.
     */
    struct pa_autoscroll_origin_facts facts = {
        .pointer_device = true,
        .libinput_backend = true, /* TEST HYPOTHETICAL, NOT WLR VERIFIED */
        .libinput_handle_present = true,
        .native_scroll_method_known = false,
        .native_scroll_on_button_down = false,
    };
    if (pa_autoscroll_origin_classify(&facts) !=
            PA_AUTOSCROLL_ORIGIN_DENIED) {
        fputs("PA_G3G_REAL_LIBINPUT_VIRTUAL_ORIGIN_PROMOTED\n", stderr);
        goto done;
    }
    printf("PA_G3G_REAL_LIBINPUT_SEES_FAKE_USB_MOUSE_BUT_SOURCE_DENIES_PASS "
        "real_libinput_path_device=1 same_udev_node=1 "
        "source_policy=DENIED hardware_attestation=NONE\n");
    printf("PA_G3G_REAL_NEGATIVE_NO_BUTTON_OR_AXIS_EMITTED_PASS "
        "compositor_input_backend=NOT_TESTED physical_origin=UNKNOWN "
        "axis_frames=0\n");
    fflush(stdout);
    rc = EXIT_SUCCESS;

done:
    if (li_device != NULL) {
        libinput_path_remove_device(li_device);
    }
    if (li != NULL) {
        libinput_unref(li);
    }
    if (udev != NULL) {
        udev_unref(udev);
    }
    if (fd >= 0) {
        if (created && ioctl(fd, UI_DEV_DESTROY) < 0) {
            perror("PA_G3G_NATIVE_UINPUT_DESTROY_FAILED");
            rc = EXIT_FAILURE;
        }
        if (close(fd) < 0) {
            perror("PA_G3G_NATIVE_UINPUT_CLOSE_FAILED");
            rc = EXIT_FAILURE;
        }
    }
    if (rc == EXIT_SUCCESS) {
        puts("PA_G3G_REAL_KERNEL_TEST_DEVICE_DESTROYED_PASS");
    }
    return rc;
}
