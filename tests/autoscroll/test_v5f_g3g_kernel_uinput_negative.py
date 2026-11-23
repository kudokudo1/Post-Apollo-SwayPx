"""G3G security invariants: native kernel uinput negative proof, no product changes."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "tests/autoscroll"
SRC = (TESTS / "ci_v5f_g3g_kernel_uinput_native.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
SOURCE = (ROOT / "sway/input/post_apollo_autoscroll_source.c").read_text()
TOPO = (ROOT / "sway/input/post_apollo_autoscroll_topology.c").read_text()
POLICY = (ROOT / "sway/input/post_apollo_autoscroll_origin_policy.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
MODEL = (ROOT / "MODEL/autoscroll-v5f-g3g-kernel-uinput-negative.md").read_text()

class NativeKernelUinputNegative(unittest.TestCase):
    def test_real_kernel_device_and_real_libinput_probe(self):
        for text in (
            "UI_SET_EVBIT", "UI_SET_KEYBIT", "UI_SET_RELBIT",
            "UI_DEV_SETUP", "UI_DEV_CREATE", "UI_GET_SYSNAME",
            "UI_DEV_DESTROY", "BUS_USB",
            "/sys/devices/virtual/input/",
            "udev_device_new_from_syspath(",
            "udev_device_get_syspath(",
            "udev_device_get_devnode(",
            "libinput_path_create_context(",
            "libinput_path_add_device(",
            "libinput_device_has_capability(",
            "LIBINPUT_DEVICE_CAP_POINTER",
            "libinput_device_get_udev_device(",
            "pa_autoscroll_kernel_tree_candidate(real_sysfs, real_devnode)",
            "pa_autoscroll_origin_classify(&facts)",
            "same_udev_node=1",
        ):
            self.assertIn(text,SRC)

    def test_no_events_no_verification_laundering(self):
        for forbidden in (
            "write(fd,", "EV_SYN", "SYN_REPORT",
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "pa_scroll_dispatch_enable(",
            "pa_autoscroll_guard_bridge_begin(",
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
            "UI_SET_ABSBIT", "UI_SET_PROPBIT",
        ):
            self.assertNotIn(forbidden,SRC)
        self.assertIn("compositor_input_backend=NOT_TESTED",SRC)
        self.assertIn("physical_origin=UNKNOWN",SRC)
        self.assertIn("hardware_attestation=NONE",SRC)
        self.assertIn("UI_DEV_DESTROY",SRC)
        self.assertIn("rc = EXIT_FAILURE;",SRC)
        self.assertIn("PA_G3G_REAL_KERNEL_TEST_DEVICE_DESTROYED_PASS",SRC)

    def test_only_github_hosted_disposable_runner_and_no_source_activation(self):
        for part in ("GITHUB_ACTIONS","CI","RUNNER_ENVIRONMENT",
                     "github-hosted","RUNNER_TEMP",
                     "/home/runner/work/_temp",
                     "PA_G3G_NATIVE_UINPUT_NEGATIVE",
                     "geteuid() == 0"):
            self.assertIn(part,SRC)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        self.assertNotIn("ci_v5f_g3g_kernel_uinput_native.c",MESON)
        self.assertIn("Kernel uinput events",SOURCE)
        self.assertIn('strstr(udev_syspath, "/virtual/")',TOPO)
        self.assertIn("PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE",POLICY)

    def test_scope_and_evidence_limits_written_down(self):
        for phrase in (
            "HARD OFF", "real kernel", "uinput", "libinput",
            "not the compositor", "not prove",
            "BUS_USB", "no mouse-button events",
            "not hardware attestation", "CI",
            "not production", "Nurse 2"
        ):
            self.assertIn(phrase.lower(),MODEL.lower())

if __name__ == "__main__":
    unittest.main()
