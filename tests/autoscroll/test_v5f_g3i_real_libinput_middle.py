"""G3I: real libinput button ingress negative proof only, not compositor."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
TEST=ROOT/"tests/autoscroll"
SRC=(TEST/"ci_v5f_g3i_real_libinput_middle_native.c").read_text()
MODEL=(ROOT/"MODEL/autoscroll-v5f-g3i-real-libinput-middle-negative.md").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
TOPO=(ROOT/"sway/input/post_apollo_autoscroll_topology.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()

class RealLibinputMiddleNegative(unittest.TestCase):
    def test_real_kernel_middle_input_and_real_libinput_observation(self):
        for token in (
            "UI_SET_EVBIT", "UI_SET_KEYBIT", "UI_SET_RELBIT",
            "UI_DEV_SETUP", "UI_DEV_CREATE", "UI_GET_SYSNAME",
            "UI_DEV_DESTROY", "BUS_USB",
            "udev_device_new_from_syspath(",
            "libinput_path_create_context(",
            "libinput_path_add_device(",
            "libinput_device_get_udev_device(",
            "libinput_dispatch(",
            "libinput_get_event(",
            "LIBINPUT_EVENT_POINTER_BUTTON",
            "LIBINPUT_BUTTON_STATE_PRESSED",
            "LIBINPUT_BUTTON_STATE_RELEASED",
            "libinput_event_pointer_get_time_usec(",
            "libinput_event_pointer_get_button(",
            "libinput_event_pointer_get_button_state(",
            "libinput_event_get_device(event) != expected",
            "emit_kernel_event(fd, EV_KEY, BTN_MIDDLE, value)",
            "emit_kernel_event(fd, EV_SYN, SYN_REPORT, 0)",
            "send_one_button_transition(fd, 1)",
            "send_one_button_transition(fd, 0)",
        ):
            self.assertIn(token,SRC)

    def test_never_promote_source_or_send_axis(self):
        for forbidden in (
            "wlr_seat_pointer_notify_axis",
            "wlr_seat_pointer_notify_frame",
            "pa_scroll_dispatch_enable(",
            "pa_autoscroll_guard_bridge_begin(",
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
            "emit_kernel_event(fd, EV_REL,",
            "emit_kernel_event(fd, EV_ABS,",
            "libinput_event_pointer_get_axis_value(",
        ):
            self.assertNotIn(forbidden,SRC)
        self.assertIn("pa_autoscroll_kernel_tree_candidate(real_sysfs, real_devnode)",SRC)
        self.assertIn("pa_autoscroll_origin_classify(&facts)",SRC)
        self.assertIn("PA_G3I_REAL_LIBINPUT_MIDDLE_PRESS_RELEASE_SOURCE_DENIED_PASS",SRC)
        self.assertIn("PA_G3I_REAL_MIDDLE_INPUT_NEGATIVE_NO_AXIS_PASS",SRC)
        self.assertIn("compositor_libinput_backend=NOT_TESTED",SRC)
        self.assertIn("physical_attestation=NONE",SRC)
        self.assertIn("if (created && ioctl(fd, UI_DEV_DESTROY) < 0)",SRC)

    def test_ci_guard_real_source_and_shipped_hard_off(self):
        for token in ("GITHUB_ACTIONS","CI","RUNNER_ENVIRONMENT",
                      "github-hosted","RUNNER_TEMP",
                      "/home/runner/work/_temp",
                      "PA_G3I_NATIVE_UINPUT_NEGATIVE",
                      "geteuid() == 0"):
            self.assertIn(token,SRC)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        self.assertIn('strstr(udev_syspath, "/virtual/")',TOPO)
        self.assertNotIn("ci_v5f_g3i_real_libinput_middle_native.c",MESON)

    def test_document_scope_not_confusing_libinput_with_hardware_or_compositor(self):
        for token in (
            "OS-managed", "enrolled-device", "Nurse 2",
            "HARD OFF", "not genuine physical hardware",
            "real libinput", "BTN_MIDDLE",
            "not prove acceptance of a genuine physical mouse",
            "not prove an actual", "wlr_input_device_is_libinput",
            "Company Map", "TimeW", "does not use the SwayPX compositor",
        ):
            self.assertIn(token.lower(),MODEL.lower())

if __name__=="__main__":
    unittest.main()
