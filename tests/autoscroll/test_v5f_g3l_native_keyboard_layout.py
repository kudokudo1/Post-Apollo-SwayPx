"""G3L real native XKB/layout observation guards, never authorization."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
T=ROOT/"tests/autoscroll"
ACTOR=(T/"ci_v5f_g3l_keyboard_mouse_actor.c").read_text()
NATIVE=(T/"ci_v5f_g3l_native_keyboard_layout_probe.c").read_text()
OVERLAY=(T/"ci_v5f_g3l_native_overlay.py").read_text()
TRACE=(T/"ci_v5f_g3l_native_trace.py").read_text()
SHELL=(T/"ci_v5f_g3l_udmabuf.sh").read_text()
MODEL=(ROOT/"MODEL/autoscroll-v5f-g3l-native-scroll-lock-layout-observation.md").read_text()
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()

class NativeKeyboardAndLayout(unittest.TestCase):
    def test_real_separate_kernel_keyboard_and_mouse_events(self):
        for word in (
            "GITHUB_ACTIONS","RUNNER_ENVIRONMENT","github-hosted",
            "PA_G3K_KERNEL_ACTOR","geteuid()==0",
            "UI_DEV_CREATE","UI_DEV_DESTROY","keyboard_fd",
            "KEY_SCROLLLOCK","PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE",
            "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL",
            "send_event(keyboard_fd,EV_KEY,KEY_SCROLLLOCK,1)",
            "send_event(keyboard_fd,EV_KEY,KEY_SCROLLLOCK,0)",
            "send_event(keyboard_fd,EV_SYN,SYN_REPORT,0)",
            "round <= 4", "PA_G3L_SYNTHETIC_KEYBOARD_DESTROYED_PASS",
        ):
            self.assertIn(word,ACTOR)
        for forbidden in ("send_event(fd,EV_REL", "send_event(fd,EV_ABS",
                          "wlr_seat_pointer_notify_axis"):
            self.assertNotIn(forbidden,ACTOR)

    def test_native_observer_reads_real_xkb_and_output_not_positive_policy(self):
        for word in (
            "struct wlr_pointer_button_event *event",
            "wlr_input_device_is_libinput(mouse)",
            "wlr_libinput_get_device_handle(mouse)",
            "wl_list_for_each(sd,&cursor->seat->devices,link)",
            "sd->keyboard->wlr->xkb_state",
            "xkb_state_led_name_is_active(",
            "XKB_LED_NAME_SCROLL","keyboard_count != 1",
            "PA_G3L_REAL_COMPOSITOR_SCROLLLOCK_KEY",
            "wlr_output_layout_get_box(root->output_layout,NULL,&live_box)",
            "pa_autoscroll_source_probe(mouse)",
            "PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE",
            "native_scroll_policy=UNKNOWN_DENY",
            "layout_generation=UNATTESTED",
            "physical_hid=NOT_ATTESTED",
            "can_consume=0 can_axis=0",
        ):
            self.assertIn(word,NATIVE)
        for word in ("wlr_seat_pointer_notify_axis",
                     "wlr_seat_pointer_notify_frame",
                     "pa_scroll_dispatch_enable(",
                     "pa_autoscroll_guard_bridge_begin(",
                     "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
                     "return true;"):
            self.assertNotIn(word,NATIVE)

    def test_real_output_mutation_and_inherited_native_guard(self):
        for word in (
            "g3k.main()",
            "pa_g3l_native_lock_layout_observe(cursor, event)",
            "ci_v5f_g3l_native_keyboard_layout_probe.c",
            "KEYBOARD_KEY_EVENT",
            "PA_G3L_TRACKED_SOURCE_MODIFIED",
        ):
            self.assertIn(word,OVERLAY)
        for word in (
            "PA_G3L_REAL_NATIVE_KEYBOARD_REGISTERED_PASS",
            "PA_G3L_REAL_KERNEL_SCROLL_LOCK_KEY_TOGGLED_PASS",
            "PA_G3L_NATIVE_SCROLLLOCK_KEY_CALLBACK_ABSENT",
            "SCROLLKEY.findall(raw)",
            "PA_G3L_NATIVE_SCROLL_LOCK_OBSERVED_UNTRUSTED_PASS",
            "PA_G3L_REAL_NATIVE_OUTPUT_LAYOUT_CHANGED_GUARD_DENIED_PASS",
            "output {output} pos {new_x} 0",
            '"-t","get_outputs"',
            "PA_G3K_NATIVE_XDG_TARGET_UNMAPPED_PASS",
            "PA_G3L_KERNEL_KEYBOARD_AND_MOUSE_DESTROYED_PASS",
            "len(G3K.findall(raw))!=8",
            "axis_frames=0",
            "PA_G3L_NATIVE_GUARD_PROMOTED",
        ):
            self.assertIn(word,TRACE)
        self.assertIn("setfacl --restore",SHELL)
        self.assertIn("ci_v5f_g3l_native_trace.py",SHELL)

    def test_shipped_hard_off_and_truthful_limitations(self):
        self.assertNotIn("pa_g3l_native_lock_layout_observe",CURSOR)
        self.assertNotIn("ci_v5f_g3l_native_keyboard_layout_probe.c",MESON)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        for word in (
            "OS-managed","Nurse 2","HARD OFF",
            "not verified multi-keyboard global Scroll Lock",
            "not a monotonic generation token",
            "UNKNOWN/DENY","Company Map","TimeW",
            "KEY_SCROLLLOCK",
        ):
            self.assertIn(word.lower(),MODEL.lower())

if __name__=="__main__":
    unittest.main()
