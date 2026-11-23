"""G3M exact native second-keyboard hotplug observation must never authorize scroll."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
T=ROOT/"tests/autoscroll"
ACTOR=(T/"ci_v5f_g3m_multi_keyboard_actor.c").read_text()
NATIVE=(T/"ci_v5f_g3m_native_multikeyboard_observer.c").read_text()
OVERLAY=(T/"ci_v5f_g3m_native_overlay.py").read_text()
TRACE=(T/"ci_v5f_g3m_native_trace.py").read_text()
SHELL=(T/"ci_v5f_g3m_udmabuf.sh").read_text()
MODEL=(ROOT/"MODEL/autoscroll-v5f-g3m-native-multi-keyboard-hotplug-negative.md").read_text()
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
KEYBOARD=(ROOT/"sway/input/keyboard.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()

class G3MNativeHotplug(unittest.TestCase):
    def test_real_two_devices_and_phased_kernel_events(self):
        for token in (
            "RUNNER_ENVIRONMENT","github-hosted","PA_G3M_KERNEL_ACTOR",
            "geteuid()==0","PA-G3M-SECOND-SYNTHETIC-KEYBOARD",
            "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL",
            "PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE",
            "UI_DEV_SETUP","UI_DEV_CREATE","UI_DEV_DESTROY",
            'strcmp(command,"plug\\n")',
            'strcmp(command,"second_key\\n")',
            'strcmp(command,"unplug\\n")',
            "send_event(second_fd,EV_KEY,KEY_SCROLLLOCK,1)",
            "send_event(second_fd,EV_KEY,KEY_SCROLLLOCK,0)",
            "PA_G3M_SECOND_KEYBOARD_PLUGGED_PASS",
            "PA_G3M_SECOND_KERNEL_SCROLLLOCK_SENT_PASS",
            "PA_G3M_SECOND_KEYBOARD_UNPLUGGED_PASS",
            "round <= 4",
        ):
            self.assertIn(token,ACTOR)
        for token in ("send_event(fd,EV_REL",
                      "send_event(fd,EV_ABS",
                      "wlr_seat_pointer_notify_axis"):
            self.assertNotIn(token,ACTOR)

    def test_native_diagnostic_counts_all_attached_keyboards_not_only_primary(self):
        for token in (
            "struct wlr_pointer_button_event *event",
            "wl_list_for_each(sd,&cursor->seat->devices,link)",
            "wl_list_for_each(group,&cursor->seat->keyboard_groups,link)",
            "wlr_libinput_get_device_handle(",
            "XKB_LED_NAME_SCROLL",
            "xkb_state_led_name_is_active(",
            "unexpected++",
            "keyboards==primary+secondary",
            "secondary_led=led",
            "const bool global_lock_clear_authorized=false;",
            "global_lock_clear_authorized=%d",
            "lock_authority=UNKNOWN_DENY origin=SOFTWARE_DENIED",
            "can_consume=0 can_axis=0",
            "PA_G3M_SECOND_NATIVE_SCROLLLOCK",
            "event->keycode!=KEY_SCROLLLOCK",
        ):
            self.assertIn(token,NATIVE)
        for bad in ("pa_scroll_dispatch_enable(",
                    "wlr_seat_pointer_notify_axis(",
                    "wlr_seat_pointer_notify_frame(",
                    "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
                    "return true;"):
            self.assertNotIn(bad,NATIVE)

    def test_testonly_native_copy_isolation(self):
        for token in (
            "g3l.main()",
            "PA_G3M_NATIVE_OBSERVE",
            "pa_g3m_native_multikeyboard_observe(cursor, event)",
            "pa_g3m_native_second_keyboard_key_observe(keyboard, event)",
            "ci_v5f_g3m_native_multikeyboard_observer.c",
            "PA_G3M_TRACKED_PRODUCTION_MODIFIED",
        ):
            self.assertIn(token,OVERLAY)
        self.assertNotIn("pa_g3m_native_multikeyboard_observe",CURSOR)
        self.assertNotIn("pa_g3m_native_second_keyboard_key_observe",KEYBOARD)
        self.assertNotIn("ci_v5f_g3m_native_multikeyboard_observer.c",MESON)

    def test_real_native_hotplug_and_key_callbacks_required(self):
        for token in (
            "PA_G3M_SECOND_NATIVE_INPUT_REGISTERED_PASS",
            '"PA_G3M_KERNEL_ACTOR")}',
            '"sudo","-n","env"',
            "PA_G3M_SECOND_NATIVE_INPUT_REMOVED_PASS",
            "PA_G3M_REAL_NATIVE_MULTIKEYBOARD_HOTPLUG_DENIED_PASS",
            "PA_G3M_NATIVE_ALL_SYNTHETIC_INPUTS_REMOVED_PASS",
            "PA_G3M_ACTUAL_SEAT_KEYBOARD_HOTPLUG_WRONG",
            "PA_G3M_SECOND_NATIVE_KEY_EVENTS_MISSING",
            "PA_G3M_GUARD_BECAME_ADMITTING",
            "SECONDKEY.findall(raw)",
            "G3M.findall(raw)",
            "expected=[1,1,1,1,2,2,1,1]",
            "len(G3K.findall(raw))!=8",
            "normal_dispatch=8 axis_frames=0",
        ):
            self.assertIn(token,TRACE)
        self.assertIn("setfacl --restore",SHELL)
        self.assertIn("ci_v5f_g3m_native_trace.py",SHELL)

    def test_shipped_hard_off_and_truthful_evidence_limit(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        for token in ("Nurse 2","HARD OFF","OS-managed",
                      "UNKNOWN/DENY","uinput","1→2→1",
                      "Company Map","TimeW","layout generation"):
            self.assertIn(token.lower(),MODEL.lower())

if __name__=="__main__":
    unittest.main()
