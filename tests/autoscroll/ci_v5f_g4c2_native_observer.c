/* CI-only binding: real per-seat slot, never an active runtime. */
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/post_apollo_autoscroll_os_native.h"
#include "log.h"
void pa_g4c2_observe(struct sway_cursor *cursor,struct wlr_pointer_button_event *event) {
    if (!cursor || !cursor->seat || !event || !event->pointer ||
        event->button!=BTN_MIDDLE) return;
    struct wlr_input_device *dev=&event->pointer->base;
    if (!wlr_input_device_is_libinput(dev)) return;
    struct libinput_device *handle=wlr_libinput_get_device_handle(dev);
    const char *name=handle?libinput_device_get_name(handle):NULL;
    if (!name || strcmp(name,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return;
    struct pa_os_native_observation o={0};
    if (!pa_os_native_observe(&cursor->seat->pa_g4c2_slot,cursor,event,&o)) {
        sway_log(SWAY_ERROR,"PA_G4C2_OBSERVE_FAILED");
        return;
    }
    struct pa_os_scroll_session *session=&cursor->seat->pa_g4c2_session;
    if (!session->seat_epoch && !pa_os_scroll_init(session,o.scene.seat_epoch)) {
        sway_log(SWAY_ERROR,"PA_G4C2_INIT_FAILED");
        return;
    }
    /* Read both press and release. Only actual presses attempt begin. */
    bool eligible=o.event.middle_press &&
        pa_os_scroll_begin(session,&o.event,&o.scene,o.scene.read_ms);
    sway_log(SWAY_INFO,"PA_G4C2_NATIVE phase=%d event=%" PRIu64
        " read=%" PRIu64 " device=%" PRIu64 " seat=%" PRIu64
        " registered=%d attached=%d libinput=%d handle=%d synthetic=%d topology=%d "
        "owners=%d focus=%d target=%" PRIu64 " output=%" PRIu64
        " stamp=%d mode=%d lock=%d layout=%d company=%d eligible=%d active=%d spent=%" PRIu64,
        o.event.middle_press,o.event.sequence,o.scene.read_sequence,
        o.event.device_epoch,o.scene.seat_epoch,o.registered,o.attached,
        o.event.libinput_pointer,o.event.live_handle,o.event.known_synthetic,
        o.event.coherent_kernel_tree,o.scene.owners_live,o.scene.focused_root_matches,
        o.scene.target_epoch,o.scene.output_epoch,o.event.observed_ms!=0,
        o.scene.mode,o.scene.lock,o.scene.layout_verified,o.scene.company_weather_clear,
        eligible,session->active,session->last_event_sequence);
}
