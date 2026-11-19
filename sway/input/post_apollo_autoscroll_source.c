/*
 * Compositor-private, read-only source probe. NO INPUT ACTUATION.
 *
 * The libinput backend is a necessary source candidate, NOT a hardware
 * authenticator. Kernel uinput events can also flow through libinput.
 * Device lifetime, evdev provenance and consent remain unverified.
 */
#include <stdbool.h>
#include <wlr/types/wlr_input_device.h>
#include "sway/input/post_apollo_autoscroll_source.h"

#include <libinput.h>
#include <wlr/backend/libinput.h>

struct pa_autoscroll_origin_facts pa_autoscroll_source_probe(
        struct wlr_input_device *device) {
    struct pa_autoscroll_origin_facts facts = {0};

    if (device == NULL || device->type != WLR_INPUT_DEVICE_POINTER) {
        return facts;
    }
    facts.pointer_device = true;

    if (!wlr_input_device_is_libinput(device)) {
        return facts;
    }
    facts.libinput_backend = true;
    struct libinput_device *handle = wlr_libinput_get_device_handle(device);
    if (handle == NULL) {
        return facts;
    }
    facts.libinput_handle_present = true;

    /* Reading native per-device mode is not changing the device. When
     * libinput uses button scrolling (including Scroll Lock mode), never
     * let the independent autoscroll adapter claim the same middle click.
     * Missing method evidence is always denied in origin policy. */
    enum libinput_config_scroll_method method =
        libinput_device_config_scroll_get_method(handle);
    facts.native_scroll_method_known = true;
    facts.native_scroll_on_button_down =
        method == LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN;
    return facts;
}
