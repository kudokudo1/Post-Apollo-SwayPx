#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_TOPOLOGY_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_TOPOLOGY_H

#include <stdbool.h>

/*
 * Linux sysfs topology NEGATIVE FILTER. A canonical, non-virtual event node
 * is only a candidate for further trusted evidence, NOT genuine hardware
 * authentication. Software-generated USB HID and compromised kernels may
 * still look physical. The caller must supply paths from the *same* udev
 * device returned by the compositor's libinput handle, never from IPC.
 */
bool pa_autoscroll_kernel_tree_candidate(
    const char *udev_syspath, const char *udev_devnode);

#endif
