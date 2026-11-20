"""V5F-G2: native virtual middle-button negative scope and source invariants."""
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tests/autoscroll/ci_v5f_g2_native_button.py"
SPEC = importlib.util.spec_from_file_location("pa_g2", RUNNER)
G2 = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(G2)
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
CLIENT = (ROOT / "tests/autoscroll/ci_v5f_g2_virtual_middle_client.c").read_text()
WRAPPER = (ROOT / "tests/autoscroll/ci_v5f_g2_udmabuf.sh").read_text()

class NativeVirtualMiddleDenial(unittest.TestCase):
    def test_real_sway_cursor_overlay_preserves_first_refusal_and_dispatch(self):
        patched = G2.cursor_patch(CURSOR)
        self.assertNotEqual(patched, CURSOR)
        self.assertEqual(patched.count("PA_G2_NATIVE_INGRESS"), 1)
        self.assertEqual(patched.count("PA_G2_NATIVE_PAIR"), 1)
        self.assertEqual(patched.count("PA_G2_NATIVE_TO_SEATOP"), 1)
        self.assertEqual(patched.count("PA_G2_NATIVE_FROM_SEATOP"), 1)
        handler = patched.split("static void handle_pointer_button(", 1)[1].split(
            "void dispatch_cursor_axis(", 1)[0]
        order = [
            "PA_G2_NATIVE_INGRESS",
            "stage_director_news_pairing_pointer_button(",
            "stage_director_company_pairing_pointer_button(",
            "PA_G2_NATIVE_PAIR",
            "if (weather_paired || company_paired)",
            "pa_autoscroll_runtime_button(",
            "dispatch_cursor_button(cursor,"
        ]
        positions = [handler.index(p) for p in order]
        self.assertEqual(positions, sorted(positions))
        dispatch = patched.split("void dispatch_cursor_button(", 1)[1].split(
            "static void handle_pointer_button(", 1)[0]
        self.assertLess(dispatch.index("PA_G2_NATIVE_TO_SEATOP"),
                        dispatch.index("seatop_button("))
        self.assertLess(dispatch.index("seatop_button("),
                        dispatch.index("PA_G2_NATIVE_FROM_SEATOP"))

    def test_real_protocol_client_injects_only_virtual_button(self):
        for symbol in (
            "ext_transient_seat_manager_v1_create(",
            "zwlr_virtual_pointer_manager_v1_create_virtual_pointer(",
            "zwlr_virtual_pointer_v1_button(pointer, 1001, BTN_MIDDLE,",
            "WL_POINTER_BUTTON_STATE_PRESSED",
            "zwlr_virtual_pointer_v1_button(pointer, 1002, BTN_MIDDLE,",
            "WL_POINTER_BUTTON_STATE_RELEASED",
            "zwlr_virtual_pointer_v1_destroy(pointer);",
            "ext_transient_seat_v1_destroy(s.transient);",
            "PA_G2_PROTOCOL_MIDDLE_SENT pressed=1 released=1 axis=0",
        ):
            self.assertIn(symbol, CLIENT)
        self.assertLess(CLIENT.index("zwlr_virtual_pointer_v1_destroy(pointer)"),
                        CLIENT.index("ext_transient_seat_v1_destroy(s.transient)"))
        for prohibited in (
            "zwlr_virtual_pointer_v1_motion(",
            "zwlr_virtual_pointer_v1_motion_absolute(",
            "zwlr_virtual_pointer_v1_axis(",
            "zwlr_virtual_pointer_v1_axis_discrete(",
            "wlr_seat_pointer_notify_button(",
        ):
            self.assertNotIn(prohibited, CLIENT)

    def test_unchanged_native_hard_off_and_no_forged_positive(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN", BRIDGE)
        self.assertIn(".layout_generation_verified = false", BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", BRIDGE)
        self.assertNotIn("PA_G2_NATIVE", CURSOR)
        self.assertNotIn("PA_G2_NATIVE", RUNTIME)
        self.assertIn("cursor->autoscroll != NULL", CURSOR)

    def test_disposable_runner_requires_native_ingress_and_dispatch(self):
        script = RUNNER.read_text()
        for token in (
            '"GITHUB_ACTIONS"',
            '"PA_G2_NATIVE_BUTTON"',
            '"/home/runner/work/_temp"',
            'os.getuid() != 0',
            "WLR_LIBINPUT_NO_DEVICES",
            'G2_DISPOSABLE_CURSOR_INSTRUMENTATION',
            "PA_G2_NATIVE_INGRESS state=",
            "PA_G2_NATIVE_PAIR state=",
            "PA_G2_NATIVE_TO_SEATOP state=",
            "PA_G2_NATIVE_FROM_SEATOP state=",
            "PA_FC2_NATIVE DEVICE_BIND",
            "PA_FC2_NATIVE DEVICE_REVOKE",
            "PA_FC2_NATIVE SEAT_REVOKE",
            "virtual_eligible=0",
        ):
            self.assertIn(token, script)
        self.assertIn("setfacl --restore=", WRAPPER)
        self.assertIn("PA_G2_RUNNER_UDMABUF_ACL_RESTORED", WRAPPER)
        self.assertIn("BLOCKED_G2_UDMABUF_NOT_DISPOSABLE_CI", WRAPPER)

if __name__ == "__main__":
    unittest.main()
