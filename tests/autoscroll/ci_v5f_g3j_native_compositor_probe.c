/*
 * G3J native C cursor callback diagnostic, disposable source copy only.
 * One real kernel->libinput->wlroots->SwayPX event chain, known uinput.
 * NEVER treats a libinput pointer as physical hardware authentication.
 * NEVER consumes a button, calls an axis/frame function or changes policy.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <libudev.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wayland-server-core.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/seat.h"
#include "sway/server.h"

void pa_g3j_real_libinput_compositor_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event,
        bool weather_paired, bool company_paired) {
    if (cursor == NULL || event == NULL || event->pointer == NULL ||
            event->button != BTN_MIDDLE) {
        return;
    }
    struct wlr_input_device *device = &event->pointer->base;
    if (!wlr_input_device_is_libinput(device)) {
        return; /* G3E virtual test is independent, never relabeled. */
    }
    struct libinput_device *handle = wlr_libinput_get_device_handle(device);
    if (handle == NULL) {
        return;
    }
    const char *name = libinput_device_get_name(handle);
    if (name == NULL ||
            strcmp(name,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL") != 0) {
        return; /* Only the temporary actor's device may satisfy CI. */
    }
    struct udev_device *udev = libinput_device_get_udev_device(handle);
    if (udev == NULL) {
        sway_log(SWAY_ERROR,"PA_G3J_NATIVE_UDEV_MISSING");
        return;
    }
    const char *sysfs=udev_device_get_syspath(udev);
    const char *node=udev_device_get_devnode(udev);
    const bool kernel_virtual = sysfs != NULL &&
        strstr(sysfs,"/virtual/") != NULL;
    const bool actual_kernel_event = node != NULL &&
        strncmp(node,"/dev/input/event",16)==0;
    udev_device_unref(udev);

    struct sway_input_device *native=NULL;
    if (server.input != NULL) {
        struct sway_input_device *it=NULL;
        wl_list_for_each(it,&server.input->devices,link) {
            if(it->wlr_device == device) {
                native=it;
                break;
            }
        }
    }
    const uint64_t device_epoch = native != NULL ?
        input_manager_autoscroll_pointer_lifetime_epoch(native) : 0;
    const uint64_t seat_epoch = cursor->seat != NULL ?
        seat_autoscroll_lifetime_epoch(cursor->seat) : 0;
    const struct pa_autoscroll_origin_facts facts =
        pa_autoscroll_source_probe(device);
    const bool os_candidate = pa_autoscroll_origin_classify(&facts) ==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;

    struct pa_autoscroll_owner_snapshot snapshot = {0};
    const bool owner = pa_autoscroll_owner_snapshot_read(
        cursor, device, &snapshot);

    /* The source can be actual libinput while synthetic. A true
     * owner snapshot also NEVER authorizes synthetic input. */
    sway_log(SWAY_INFO,
        "PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE "
        "pressed=%d native_libinput=1 real_handle=1 "
        "kernel_event=%d kernel_virtual=%d registered=%d "
        "device_epoch=%" PRIu64 " seat_epoch=%" PRIu64
        " owner_snapshot=%d target_epoch=%" PRIu64
        " output_epoch=%" PRIu64
        " os_source_candidate=%d weather=%d company=%d "
        "time_nonzero=%d physical_attestation=NONE "
        "can_consume=0 can_axis=0",
        event->state == WL_POINTER_BUTTON_STATE_PRESSED,
        actual_kernel_event,kernel_virtual,native != NULL,
        device_epoch,seat_epoch,owner,
        owner ? snapshot.target : UINT64_C(0),
        owner ? snapshot.output : UINT64_C(0),
        os_candidate,weather_paired,company_paired,
        event->time_msec != 0);
}
