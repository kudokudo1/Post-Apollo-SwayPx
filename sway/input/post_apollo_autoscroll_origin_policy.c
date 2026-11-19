/* Source-only origin policy. A candidate is NOT trusted physical hardware. */
#include <stddef.h>
#include "sway/input/post_apollo_autoscroll_origin_policy.h"

enum pa_autoscroll_origin_result pa_autoscroll_origin_classify(
        const struct pa_autoscroll_origin_facts *facts) {
    if (facts == NULL || !facts->pointer_device
            || !facts->libinput_backend
            || !facts->libinput_handle_present
            || !facts->native_scroll_method_known
            || facts->native_scroll_on_button_down) {
        return PA_AUTOSCROLL_ORIGIN_DENIED;
    }
    return PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;
}
