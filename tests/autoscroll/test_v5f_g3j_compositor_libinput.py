"""G3J source guards: no production input mutation, only real native evidence."""
from pathlib import Path
import unittest
R=Path(__file__).resolve().parents[2]
T=R/"tests/autoscroll"
A=(T/"ci_v5f_g3j_uinput_actor.c").read_text()
N=(T/"ci_v5f_g3j_native_compositor_probe.c").read_text()
O=(T/"ci_v5f_g3j_native_overlay.py").read_text()
P=(T/"ci_v5f_g3j_native_trace.py").read_text()
S=(T/"ci_v5f_g3j_udmabuf.sh").read_text()
M=(R/"MODEL/autoscroll-v5f-g3j-compositor-libinput-native-negative.md").read_text()
C=(R/"sway/input/cursor.c").read_text()
MESON=(R/"sway/meson.build").read_text()
RT=(R/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
B=(R/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()

class RealSwayPxLibinputSourceTest(unittest.TestCase):
    def test_real_kernel_actor_only_in_disposable_ci(self):
        for x in ("GITHUB_ACTIONS","RUNNER_ENVIRONMENT",
                  "github-hosted","PA_G3J_KERNEL_ACTOR",
                  "UI_DEV_CREATE","UI_DEV_DESTROY",
                  "BTN_MIDDLE","EV_SYN","SYN_REPORT",
                  "BUS_USB","EV_KEY","click\\n","stop\\n"):
            self.assertIn(x,A)
        self.assertNotIn("wlr_seat_pointer_notify_axis",A)
        self.assertNotIn("send_event(fd,EV_REL",A)
        self.assertNotIn("send_event(fd,EV_ABS",A)
        self.assertIn("PA_G3J_KERNEL_UINPUT_DESTROYED_PASS",A)

    def test_actual_compositor_libinput_callback_diagnostic_no_axis(self):
        for x in ("struct wlr_pointer_button_event *event",
                  "wlr_input_device_is_libinput(device)",
                  "wlr_libinput_get_device_handle(device)",
                  "libinput_device_get_udev_device(handle)",
                  "udev_device_get_syspath(udev)",
                  "udev_device_get_devnode(udev)",
                  'strstr(sysfs,"/virtual/")',
                  "input_manager_autoscroll_pointer_lifetime_epoch(native)",
                  "seat_autoscroll_lifetime_epoch(cursor->seat)",
                  "pa_autoscroll_owner_snapshot_read(",
                  "pa_autoscroll_source_probe(device)",
                  "pa_autoscroll_origin_classify(&facts)",
                  "real_handle=1",
                  "physical_attestation=NONE",
                  "can_consume=0 can_axis=0"):
            self.assertIn(x,N)
        for x in ("wlr_seat_pointer_notify_axis","wlr_seat_pointer_notify_frame",
                  "pa_autoscroll_guard_bridge_begin",
                  "pa_scroll_dispatch_enable","PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
                  "return true;"):
            self.assertNotIn(x,N)

    def test_true_native_backend_no_mock_or_production_link(self):
        for x in ("g3h.main()","ci_v5f_g3j_native_compositor_probe.c",
                  "pa_g3j_real_libinput_compositor_observe(cursor, event",
                  "PA_G3J_NATIVE_LIBINPUT_DIAG",
                  "PA_G3J_CHECKED_IN_SOURCE_MUTATED"):
            self.assertIn(x,O)
        for x in ("WLR_BACKENDS","headless,libinput",
                  "LIBSEAT_BACKEND","seatd",
                  "WLR_LIBINPUT_NO_DEVICES",
                  "PA_G3J_PREEXISTING_SEATD_REFUSE_TO_TOUCH",
                  "PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE",
                  "get_inputs","ID_INPUT_MOUSE=1",
                  "PA_G2_NATIVE_FROM_SEATOP",
                  "PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE_SOURCE_DENIED_PASS",
                  "PA_G3J_REAL_NATIVE_INPUT_DEVICE_DESTROYED_PASS"):
            self.assertIn(x,P)
        self.assertIn("setfacl --restore",S)
        self.assertIn("ci_v5f_g3j_native_trace.py",S)
        self.assertNotIn("pa_g3j_real_libinput_compositor_observe",C)
        self.assertNotIn("ci_v5f_g3j",MESON)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RT)
        self.assertIn("const bool physical_verified = false;",RT)
        self.assertIn("const bool native_scroll_lock_active = true;",RT)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",B)

    def test_model_distinguishes_kernel_synthetic_from_hardware(self):
        for phrase in ("OS-managed", "enrolled-device",
                       "Nurse 2","HARD OFF",
                       "not physical hardware attestation",
                       "virtual sysfs","libinput",
                       "Company Map","TimeW",
                       "No production", "no fabricated positive"):
            self.assertIn(phrase.lower(),M.lower())
if __name__=="__main__":
    unittest.main()
