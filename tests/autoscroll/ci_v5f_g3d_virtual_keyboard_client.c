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

static bool toggle(struct state *s) {
    /* A default XKB map may not implement any ScrollLock key action,
     * even when it exports the Scroll Lock LED. A disposable virtual
     * keyboard can explicitly send locked modifier state instead:
     * after test-only SCLK -> Mod3 mapping, the native Scroll Lock LED
     * is driven by the xkb_state LOCKED Mod3 bit. No key or mouse events.
     */
    xkb_mod_index_t idx=xkb_keymap_mod_get_index(s->keymap,"Mod3");
    if (idx==XKB_MOD_INVALID || idx>=32) {
        return false;
    }
    uint32_t old=xkb_state_serialize_mods(s->xkb,XKB_STATE_MODS_LOCKED);
    uint32_t next=old ^ (UINT32_C(1)<<idx);
    (void)xkb_state_update_mask(s->xkb,0,0,next,0,0,0);
    int observed=xkb_state_led_name_is_active(s->xkb,XKB_LED_NAME_SCROLL);
    if (observed<0 || observed!=((next & (UINT32_C(1)<<idx))!=0)) {
        fprintf(stderr,
            "PA_G3D_CI_NATIVE_TEST_MODIFIER_MISMATCH idx=%u old=%u "
            "new=%u led=%d\\n",(unsigned)idx,old,next,observed);
        return false;
    }
    zwp_virtual_keyboard_v1_modifiers(s->keyboard,0,0,next,0);
    return wl_display_roundtrip(s->display)>=0;
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
    /* The default XKB rules expose a Scroll Lock LED, but the SCLK key
     * does not necessarily map to a locking modifier. Test-only map:
     * bind SCLK and the disposable LED indicator directly to Mod3.
     * This map is transmitted ONLY by this disposable VIRTUAL client;
     * it is NOT physical hardware, production policy or permission.
     */
    struct xkb_keymap *default_map=xkb_keymap_new_from_names(
        s.context,NULL,XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!default_map) {
        fputs("PA_G3D_DEFAULT_XKB_KEYMAP_FAILED\n",stderr);
        return 5;
    }
    char *original=xkb_keymap_get_as_string(
        default_map,XKB_KEYMAP_FORMAT_TEXT_V1);
    xkb_keymap_unref(default_map);
    if (!original) {
        fputs("PA_G3D_DEFAULT_KEYMAP_NOT_SERIALIZABLE\n",stderr);
        return 5;
    }
    char *sclk=strstr(original,"key <SCLK>");
    char *end=sclk==NULL?NULL:strchr(sclk,';');
    if (end==NULL || strstr(end+1,"key <SCLK>")!=NULL) {
        fputs("PA_G3D_CI_SCLK_TEST_KEYMAP_AMBIGUOUS\n",stderr);
        free(original);
        return 5;
    }
    const char injection[]="\n    modifier_map Mod3 { <SCLK> };";
    size_t prefix=(size_t)(end-original)+1;
    size_t total=strlen(original)+strlen(injection)+1;
    char *patched=malloc(total);
    if (!patched) {
        free(original);
        return 5;
    }
    memcpy(patched,original,prefix);
    memcpy(patched+prefix,injection,strlen(injection));
    strcpy(patched+prefix+strlen(injection),original+prefix);
    free(original);
    /* Also pin the TEST LED rule directly to Mod3. Some distributions
     * map ScrollLock as a virtual modifier without a real backing bit.
     * This conversion affects ONLY the disposable client keymap. */
    char *indicator=strstr(patched,"indicator \"Scroll Lock\" {");
    char *indicator_end=indicator==NULL?NULL:strstr(indicator,"};");
    const char old_rule[]="modifiers= ScrollLock;";
    const char new_rule[]="modifiers= Mod3;";
    char *rule=indicator==NULL?NULL:strstr(indicator,old_rule);
    if (indicator_end==NULL || rule==NULL || rule>=indicator_end) {
        fputs("PA_G3D_CI_TEST_LED_INDICATOR_RULE_MISSING\\n",stderr);
        free(patched);
        return 5;
    }
    memmove(rule+strlen(new_rule),rule+strlen(old_rule),
        strlen(rule+strlen(old_rule))+1);
    memcpy(rule,new_rule,strlen(new_rule));
    s.keymap=xkb_keymap_new_from_string(
        s.context,patched,XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    free(patched);
    if (!s.keymap ||
            xkb_keymap_led_get_index(s.keymap,XKB_LED_NAME_SCROLL)
                == XKB_LED_INVALID) {
        fputs("PA_G3D_CI_SCLK_TEST_MODIFIER_KEYMAP_INVALID\n",stderr);
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
