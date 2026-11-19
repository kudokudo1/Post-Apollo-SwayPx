/* No wlroots libinput backend: every source remains an unsupported candidate.
 * There is no physical input authentication or autoscroll activation. */
#include <wlr/types/wlr_input_device.h>
#include "sway/input/post_apollo_autoscroll_source.h"

struct pa_autoscroll_origin_facts pa_autoscroll_source_probe(
        struct wlr_input_device *device) {
    (void)device;
    return (struct pa_autoscroll_origin_facts){0};
}
