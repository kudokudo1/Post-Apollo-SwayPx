/*
 * G3D disposable virtual keyboard control, never a physical HID.
 * Sends XKB keymap and ScrollLock modifier toggles via real Wayland.
 * Does NOT send pointer, middle-click, axis, frame, system command or
 * application autorun. CI-only to verify compositor's own LED state.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-names.h>
#include "wlr-virtual-keyboard-unstable-v1-client-protocol.h"

struct state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct zwp_virtual_keyboard_manager_v1 *manager;
    struct wl_seat *seat;
    struct zwp_virtual_keyboard_v1 *keyboard;
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *xkb;
};

static void global(void *userdata, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct state *s = userdata;
    if (strcmp(interface, "zwp_virtual_keyboard_manager_v1") == 0) {
        s->manager = wl_registry_bind(registry,name,
            &zwp_virtual_keyboard_manager_v1_interface,
            version < 1 ? version : 1);
    } else if (strcmp(interface, "wl_seat") == 0 && s->seat == NULL) {
        s->seat = wl_registry_bind(registry,name,
            &wl_seat_interface,version < 1 ? version : 1);
    }
}
static void global_remove(void *userdata, struct wl_registry *registry,
        uint32_t name) {
    (void)userdata; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = global, .global_remove = global_remove,
};

static uint32_t locked(const struct state *s) {
    return xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_LOCKED);
}
static bool toggle(struct state *s) {
    const xkb_keycode_t key = KEY_SCROLLLOCK+8;
    if (xkb_state_update_key(s->xkb,key,XKB_KEY_DOWN) == 0) {
        /* xkb change mask may be zero on press; release is decisive. */
    }
    zwp_virtual_keyboard_v1_key(s->keyboard,100,KEY_SCROLLLOCK,
                                 WL_KEYBOARD_KEY_STATE_PRESSED);
    zwp_virtual_keyboard_v1_modifiers(s->keyboard,
        xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_DEPRESSED),
        xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_LATCHED),
        locked(s),
        xkb_state_serialize_layout(s->xkb,XKB_STATE_LAYOUT_EFFECTIVE));
    xkb_state_update_key(s->xkb,key,XKB_KEY_UP);
    zwp_virtual_keyboard_v1_key(s->keyboard,101,KEY_SCROLLLOCK,
                                 WL_KEYBOARD_KEY_STATE_RELEASED);
    zwp_virtual_keyboard_v1_modifiers(s->keyboard,
        xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_DEPRESSED),
        xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_LATCHED),
        locked(s),
        xkb_state_serialize_layout(s->xkb,XKB_STATE_LAYOUT_EFFECTIVE));
    if (wl_display_roundtrip(s->display) < 0) {
        return false;
    }
    return true;
}
int main(void) {
    struct state s={0};
    s.display=wl_display_connect(NULL);
    if (!s.display) {
        fputs("PA_G3D_PRIVATE_WAYLAND_CONNECT_DENIED\n",stderr);
        return 2;
    }
    s.registry=wl_display_get_registry(s.display);
    if (!s.registry ||
            wl_registry_add_listener(s.registry,&registry_listener,&s)!=0 ||
            wl_display_roundtrip(s.display)<0 ||
            !s.manager || !s.seat) {
        fputs("PA_G3D_REAL_VIRTUAL_KEYBOARD_MANAGER_MISSING\n",stderr);
        return 3;
    }
    s.keyboard=zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
        s.manager,s.seat);
    s.context=xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!s.keyboard || !s.context) {
        fputs("PA_G3D_KEYMAP_SETUP_FAILED\n",stderr);
        return 4;
    }
    s.keymap=xkb_keymap_new_from_names(s.context,NULL,XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!s.keymap ||
            xkb_keymap_led_get_index(s.keymap,XKB_LED_NAME_SCROLL)
                == XKB_LED_INVALID) {
        fputs("PA_G3D_NATIVE_SCROLL_LED_NOT_IN_CLIENT_KEYMAP\n",stderr);
        return 5;
    }
    s.xkb=xkb_state_new(s.keymap);
    char *map=xkb_keymap_get_as_string(s.keymap,XKB_KEYMAP_FORMAT_TEXT_V1);
    if (!s.xkb || !map) {
        fputs("PA_G3D_KEYMAP_NOT_SERIALIZABLE\n",stderr);
        return 6;
    }
    size_t size=strlen(map)+1;
    int fd=memfd_create("pa-g3d-disposable-keymap",MFD_CLOEXEC);
    if (fd < 0 || write(fd,map,size)!=(ssize_t)size) {
        fputs("PA_G3D_CI_KEYMAP_MEMFD_FAILED\n",stderr);
        return 7;
    }
    zwp_virtual_keyboard_v1_keymap(s.keyboard,
        WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,fd,(uint32_t)size);
    close(fd);
    free(map);
    if (wl_display_roundtrip(s.display)<0) {
        fputs("PA_G3D_NATIVE_KEYMAP_NOT_ACCEPTED\n",stderr);
        return 8;
    }
    if (xkb_state_led_name_is_active(s.xkb,XKB_LED_NAME_SCROLL)!=0) {
        fputs("PA_G3D_CI_INITIAL_SCROLL_LED_NOT_OFF\n",stderr);
        return 9;
    }
    puts("PA_G3D_VIRTUAL_KEYBOARD_READY physical_eligible=0 scroll_led=OFF");
    fflush(stdout);

    char *line=NULL;
    size_t cap=0;
    while (getline(&line,&cap,stdin)>=0) {
        if (strcmp(line,"toggle\n")==0) {
            if (!toggle(&s)) {
                fputs("PA_G3D_CI_TOGGLE_WAYLAND_ROUNDTRIP_FAILED\n",stderr);
                free(line); return 10;
            }
            int led=xkb_state_led_name_is_active(s.xkb,XKB_LED_NAME_SCROLL);
            printf("PA_G3D_VIRTUAL_KEYBOARD_TOGGLED expected=%s\n",
                led>0?"ON":led==0?"OFF":"UNKNOWN");
            fflush(stdout);
        } else if (strcmp(line,"stop\n")==0) {
            zwp_virtual_keyboard_v1_destroy(s.keyboard);
            if (wl_display_roundtrip(s.display)<0) {
                free(line); return 11;
            }
            puts("PA_G3D_VIRTUAL_KEYBOARD_DONE no_pointer_events=1");
            fflush(stdout);
            free(line);
            xkb_state_unref(s.xkb);
            xkb_keymap_unref(s.keymap);
            xkb_context_unref(s.context);
            wl_display_disconnect(s.display);
            return 0;
        } else {
            fputs("PA_G3D_UNKNOWN_CI_TEST_COMMAND\n",stderr);
            free(line);
            return 12;
        }
    }
    fputs("PA_G3D_CI_MISSING_STOP\n",stderr);
    free(line);
    return 13;
}
