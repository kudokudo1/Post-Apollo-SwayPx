/* Linux event-node negative filter: rejects obvious virtual/mismatched paths. */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_topology.h"

static bool decimal_digits(const char *start, const char *end) {
    if (start == NULL || end == NULL || end <= start) {
        return false;
    }
    for (const char *p = start; p < end; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return true;
}

bool pa_autoscroll_kernel_tree_candidate(
        const char *udev_syspath, const char *udev_devnode) {
    const char *const sys_prefix = "/sys/devices/";
    const char *const dev_prefix = "/dev/input/event";
    if (udev_syspath == NULL || udev_devnode == NULL) {
        return false;
    }
    const size_t syslen = strnlen(udev_syspath, 4096u);
    const size_t devlen = strnlen(udev_devnode, 256u);
    if (syslen < sizeof("/sys/devices/a/input/input1/event1") - 1
            || syslen == 4096u || devlen == 256u
            || strncmp(udev_syspath, sys_prefix, strlen(sys_prefix)) != 0
            || strncmp(udev_devnode, dev_prefix, strlen(dev_prefix)) != 0
            || strstr(udev_syspath, "//") != NULL
            || strstr(udev_syspath, "/../") != NULL
            || strstr(udev_syspath, "/./") != NULL
            || strstr(udev_syspath, "/virtual/") != NULL
            || strstr(udev_syspath, "/uinput/") != NULL
            || strstr(udev_syspath, "/uhid/") != NULL) {
        return false;
    }

    /* Only accept a complete eventN leaf below a matching inputN node. */
    const char *last_slash = strrchr(udev_syspath, '/');
    const char *event = udev_devnode + strlen(dev_prefix);
    if (last_slash == NULL || strncmp(last_slash + 1, "event", 5) != 0
            || !decimal_digits(last_slash + 6, udev_syspath + syslen)
            || !decimal_digits(event, udev_devnode + devlen)
            || strcmp(last_slash + 1, udev_devnode + strlen("/dev/input/")) != 0) {
        return false;
    }

    /* Reject superficial eventN suffixes without the input/inputN parent. */
    const char *input = strstr(udev_syspath, "/input/input");
    if (input == NULL) {
        return false;
    }
    input += strlen("/input/input");
    const char *parent_end = strchr(input, '/');
    if (!decimal_digits(input, parent_end) || parent_end != last_slash) {
        return false;
    }

    /* A non-virtual kernel path only makes a CANDIDATE, never a physical
     * credential. It does not override runtime's separate hard-OFF gates. */
    return true;
}
