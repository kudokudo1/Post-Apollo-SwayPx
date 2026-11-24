/*
 * G3O disposable compositor instrumentation only. Process-local diagnostic
 * event serial, not a protected issuer, persistent epoch or admission token.
 */
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <wayland-server-core.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <libinput.h>
#include "log.h"

static uint64_t serial;
static uint64_t first;
static bool captured, poisoned;
static struct wl_listener layout_change, layout_destroy;
static bool attached;

void pa_g3o_event(const char *kind) {
    if (serial == UINT64_MAX) {
        poisoned = true;
    } else if (!poisoned) {
        ++serial;
    }
    sway_log(SWAY_INFO, "PA_G3O_EVENT kind=%s serial=%" PRIu64
        " poisoned=%d stale=%d can_axis=0", kind, serial, poisoned,
        captured && (poisoned || first != serial));
}

void pa_g3o_device_event(struct wlr_input_device *dev, bool add) {
    if (!dev || dev->type != WLR_INPUT_DEVICE_KEYBOARD) {
        return;
    }
    struct libinput_device *handle = wlr_input_device_is_libinput(dev)
        ? wlr_libinput_get_device_handle(dev) : NULL;
    const char *name = handle ? libinput_device_get_name(handle) : NULL;
    const bool secondary = name &&
        strcmp(name, "PA-G3M-SECOND-SYNTHETIC-KEYBOARD") == 0;
    pa_g3o_event(secondary ? (add ? "second_add" : "second_remove")
        : (add ? "keyboard_add" : "keyboard_remove"));
}

static void changed(struct wl_listener *listener, void *data) {
    (void)listener;
    (void)data;
    pa_g3o_event("layout_change");
}

static void destroyed(struct wl_listener *listener, void *data) {
    (void)listener;
    (void)data;
    pa_g3o_event("layout_destroy");
    wl_list_remove(&layout_change.link);
    wl_list_remove(&layout_destroy.link);
    attached = false;
    poisoned = true;
}

void pa_g3o_attach_layout(struct wlr_output_layout *layout) {
    if (!layout || attached) {
        poisoned = true;
        return;
    }
    layout_change.notify = changed;
    layout_destroy.notify = destroyed;
    wl_signal_add(&layout->events.change, &layout_change);
    wl_signal_add(&layout->events.destroy, &layout_destroy);
    attached = true;
}

void pa_g3o_middle(struct wlr_pointer_button_event *event) {
    if (!event || !event->pointer || event->button != BTN_MIDDLE) {
        return;
    }
    struct wlr_input_device *dev = &event->pointer->base;
    if (!wlr_input_device_is_libinput(dev)) {
        return;
    }
    struct libinput_device *handle = wlr_libinput_get_device_handle(dev);
    const char *name = handle ? libinput_device_get_name(handle) : NULL;
    if (!name || strcmp(name, "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) {
        return;
    }
    if (!captured) {
        first = serial;
        captured = true;
    }
    sway_log(SWAY_INFO, "PA_G3O_MIDDLE pressed=%d serial=%" PRIu64
        " initial=%" PRIu64 " stale=%d can_consume=0 can_axis=0",
        event->state == WL_POINTER_BUTTON_STATE_PRESSED, serial, first,
        poisoned || !attached || first != serial);
}
