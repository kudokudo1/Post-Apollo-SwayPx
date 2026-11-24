"""V5F-G3B native-only owner-revocation probe isolation and source audit."""
from pathlib import Path
import importlib.util
import unittest

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "tests/autoscroll"

def imported(name, filename):
    import sys
    sys.path.insert(0, str(TESTS))
    spec = importlib.util.spec_from_file_location(name, TESTS / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

C1 = imported("pa_g3b_c1_overlay", "ci_v5f_c1_native_overlay.py")
G3B = imported("pa_g3b_overlay", "ci_v5f_g3b_native_overlay.py")
TARGET = (ROOT / "sway/tree/container.c").read_text()
OUTPUT = (ROOT / "sway/tree/output.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
PROBE = (TESTS / "ci_v5f_g3b_native_probe.c").read_text()
HARNESS = (TESTS / "ci_v5f_g3b_native_trace.py").read_text()
WRAPPER = (TESTS / "ci_v5f_g3b_udmabuf.sh").read_text()

class G3BNativeOwnerRevocationSafety(unittest.TestCase):
    def test_new_probe_not_in_real_production_source(self):
        for source in (TARGET, OUTPUT, MESON, RUNTIME, BRIDGE):
            self.assertNotIn("PA_G3B_NATIVE_", source)
            self.assertNotIn("ci_v5f_g3b_native_probe.c", source)
        self.assertNotIn("post_apollo_autoscroll_continuity.c", MESON)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN", BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)

    def test_real_owner_callback_patch_orders(self):
        target = G3B.target_patch(C1.target_patch(TARGET))
        output = G3B.output_patch(C1.output_patch(OUTPUT))
        meson = G3B.meson_patch(MESON)
        self.assertIn("PA_FC1_NATIVE TARGET_BIND", target)
        self.assertIn("PA_FC1_NATIVE TARGET_REVOKE", target)
        self.assertIn("PA_FC1_NATIVE OUTPUT_CONFIG", output)
        self.assertIn("PA_FC1_NATIVE OUTPUT_DISABLE", output)
        self.assertEqual(target.count("pa_g3b_native_target_bound(c);"), 1)
        self.assertEqual(target.count("pa_g3b_native_target_revoked(con);"), 1)
        self.assertEqual(output.count("pa_g3b_native_output_changed(output, 1);"), 1)
        self.assertEqual(output.count("pa_g3b_native_output_changed(output, 2);"), 1)
        self.assertLess(target.index("pa_autoscroll_lifetime_revoke(&con->autoscroll_target_lifetime);"),
                        target.index("pa_g3b_native_target_revoked(con);"))
        self.assertLess(output.index("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime);",
                                     output.index("void output_disable(")),
                        output.index("pa_g3b_native_output_changed(output, 2);"))
        self.assertIn("'input/post_apollo_autoscroll_continuity.c'", meson)
        self.assertIn("'../tests/autoscroll/ci_v5f_g3b_native_probe.c'", meson)

    def test_probe_reads_actual_native_owner_epochs_but_not_physical(self):
        for token in (
            "container_autoscroll_target_lifetime_epoch(g3b.target)",
            "output_autoscroll_lifetime_epoch(g3b.output)",
            "pa_autoscroll_continuity_begin(",
            "pa_autoscroll_continuity_tick(",
            "pa_autoscroll_continuity_cancel(",
            "G3B_TEST_DEVICE_EPOCH",
            "G3B_TEST_SEAT_EPOCH",
            "G3B_TEST_LAYOUT_GENERATION",
            "PA_G3B_NATIVE_SYNTHETIC_BASELINE",
            "PA_G3B_NATIVE_DENY reason=%s",
            "axis_frames=0",
            "origin=FORGED_TEST_ONLY",
            "TEST FORGERY",
        ):
            self.assertIn(token, PROBE)
        for forbidden in (
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "zwlr_virtual_pointer_v1_button(",
            "pa_scroll_dispatch_enable(",
            "wl_event_loop_add_timer(",
            "libinput_dispatch(",
        ):
            self.assertNotIn(forbidden, PROBE)

    def test_real_native_xdg_and_ipc_actions_three_denials(self):
        for symbol in (
            'PA_G3B_REAL_NATIVE_OUTPUT_CONFIG_CHANGE_CANCEL_PASS',
            'PA_G3B_REAL_NATIVE_TARGET_UNMAP_CANCEL_PASS',
            'PA_G3B_REAL_NATIVE_OUTPUT_DISABLE_CANCEL_PASS',
            'PA_G3B_REAL_NATIVE_TARGET_OUTPUT_REVOCATION_PASS',
            'output HEADLESS-1 scale 1',
            'output HEADLESS-1 disable',
            'c1.start_foot(',
            'c1.wait_for_window(',
            'PA_G3B_NATIVE_POLICY_DENIAL_FAILED',
            'PA_G3B_NATIVE_SYNTHETIC_START_REFUSED',
            'native_callback_policy_denials=3',
            '"WLR_LIBINPUT_NO_DEVICES": "1"',
            "PA_G3B_NATIVE_REVOCATION",
        ):
            self.assertIn(symbol, HARNESS)
        for symbol in ("BLOCKED_G3B_UDMABUF_NOT_DISPOSABLE_CI",
                       "setfacl --restore=", "original_acl=RESTORED"):
            self.assertIn(symbol, WRAPPER)
        self.assertIn("PA_G3B_CI_ONLY_NATIVE_OWNER_CALLBACK_AND_MESON_OVERLAY_PASS",
                      (TESTS / "ci_v5f_g3b_native_overlay.py").read_text())
        self.assertIn('"/home/runner/work/_temp"',
                      (TESTS / "ci_v5f_g3b_native_overlay.py").read_text())

if __name__ == "__main__":
    unittest.main()
