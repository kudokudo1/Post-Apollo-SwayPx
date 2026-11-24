"""V5F-C2: CI-only real Wayland virtual-pointer / transient-seat receipts."""
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
OVERLAY_PATH = ROOT / "tests/autoscroll/ci_v5f_c2_native_overlay.py"
SPEC = importlib.util.spec_from_file_location("pa_fc2_overlay", OVERLAY_PATH)
OVERLAY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(OVERLAY)
INPUT = (ROOT / "sway/input/input-manager.c").read_text()
SEAT = (ROOT / "sway/input/seat.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
CLIENT = (ROOT / "tests/autoscroll/ci_v5f_c2_protocol_client.c").read_text()
HARNESS = (ROOT / "tests/autoscroll/ci_v5f_c2_native_trace.py").read_text()
WRAPPER = (ROOT / "tests/autoscroll/ci_v5f_c2_udmabuf.sh").read_text()

class NativeDeviceSeatEvidenceSafety(unittest.TestCase):
    def test_virtual_pointer_owner_issues_but_eligibility_refuses(self):
        patched = OVERLAY.device_patch(INPUT)
        self.assertNotEqual(patched, INPUT)
        self.assertEqual(patched.count("PA_FC2_NATIVE DEVICE_BIND"), 1)
        self.assertEqual(patched.count("PA_FC2_NATIVE DEVICE_REVOKE"), 1)
        fn = patched.split("void handle_virtual_pointer(", 1)[1]
        self.assertLess(fn.index("pa_autoscroll_bind_pointer_lifetime(input_manager, input_device)"),
                        fn.index("PA_FC2_NATIVE DEVICE_BIND"))
        self.assertLess(fn.index("PA_FC2_NATIVE DEVICE_BIND"),
                        fn.index("wl_list_insert(&input_manager->devices"))
        self.assertIn("input_manager_autoscroll_pointer_lifetime_epoch(input_device)", fn)
        self.assertIn("input_device->is_virtual = true;", fn)
        revoke = patched.split("static void handle_device_destroy(",1)[1].split(
            "static void handle_new_input(",1)[0]
        self.assertLess(revoke.index("pa_autoscroll_lifetime_revoke("),
                        revoke.index("PA_FC2_NATIVE DEVICE_REVOKE"))
        self.assertLess(revoke.index("PA_FC2_NATIVE DEVICE_REVOKE"),
                        revoke.index("seat_remove_device("))
        self.assertIn("input_device->is_virtual", revoke)
        self.assertIn("input_device == NULL || input_device->is_virtual", INPUT)

    def test_transient_seat_owner_issues_and_revokes_before_detach(self):
        patched = OVERLAY.seat_patch(SEAT)
        self.assertEqual(patched.count("PA_FC2_NATIVE SEAT_BIND"), 1)
        self.assertEqual(patched.count("PA_FC2_NATIVE SEAT_REVOKE"), 1)
        start = patched.split("struct sway_seat *seat_create(",1)[1]
        self.assertLess(start.index("pa_autoscroll_lifetime_bind("),
                        start.index("PA_FC2_NATIVE SEAT_BIND"))
        self.assertLess(start.index("PA_FC2_NATIVE SEAT_BIND"),
                        start.index("wl_list_insert(&server.input->seats"))
        end = patched.split("static void handle_seat_destroy(",1)[1].split(
            "void seat_idle_notify_activity(",1)[0]
        self.assertLess(end.index("pa_autoscroll_lifetime_revoke("),
                        end.index("PA_FC2_NATIVE SEAT_REVOKE"))
        self.assertLess(end.index("PA_FC2_NATIVE SEAT_REVOKE"),
                        end.index("seat_device_destroy("))

    def test_client_is_actual_protocol_only_no_event_injection(self):
        for token in (
            "wl_display_connect(NULL)",
            "ext_transient_seat_manager_v1_create(",
            "ext_transient_seat_v1_add_listener(",
            "wl_registry_bind(state.registry,",
            "zwlr_virtual_pointer_manager_v1_create_virtual_pointer(",
            "zwlr_virtual_pointer_v1_destroy(pointer)",
            "ext_transient_seat_v1_destroy(state.transient)",
            "wl_display_roundtrip(",
            'strcmp(line, "stop\\n")',
        ):
            self.assertIn(token, CLIENT)
        self.assertLess(CLIENT.index("zwlr_virtual_pointer_v1_destroy(pointer)"),
                        CLIENT.index("ext_transient_seat_v1_destroy(state.transient)"))
        for prohibited in ("zwlr_virtual_pointer_v1_button(",
                           "zwlr_virtual_pointer_v1_motion(",
                           "zwlr_virtual_pointer_v1_axis(",
                           "wlr_seat_pointer_notify_axis(",
                           "wlr_input_device_init(",
                           "wl_signal_emit_mutable("):
            self.assertNotIn(prohibited, CLIENT)

    def test_ci_scopes_and_hard_off_retain_original_source(self):
        for path in (INPUT,SEAT,RUNTIME,BRIDGE):
            self.assertNotIn("PA_FC2_NATIVE", path)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        for token in ("PA_AUTOSCROLL_CI_NATIVE_TRACE", "GITHUB_ACTIONS",
                      "/home/runner/work/_temp", "shutil.copytree(",
                      "PA_FC2_FOREIGN_DIFF", "os.getuid() != 0"):
            self.assertIn(token,OVERLAY_PATH.read_text())
        self.assertIn("WLR_LIBINPUT_NO_DEVICES",HARNESS)
        self.assertIn("physical_eligible=0 PASS",HARNESS)
        self.assertIn("PA_FC2_REAL_DEVICE_AND_SEAT_LIFECYCLE_EVIDENCE_PASS",HARNESS)
        self.assertIn("original_acl=RESTORED",WRAPPER)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)

if __name__ == "__main__":
    unittest.main()
