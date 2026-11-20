"""CI-only owner callback overlay safety; no native device or feature activation."""
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
FILE = ROOT / "tests/autoscroll/ci_v5f_c1_native_overlay.py"
SPEC = importlib.util.spec_from_file_location("pa_fc1_overlay", FILE)
OVERLAY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(OVERLAY)
TARGET = (ROOT / "sway/tree/container.c").read_text()
OUTPUT = (ROOT / "sway/tree/output.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()

class DisposableTraceSafety(unittest.TestCase):
    def test_target_owner_hook_instruments_create_and_revoke(self):
        after = OVERLAY.target_patch(TARGET)
        self.assertNotEqual(TARGET, after)
        self.assertEqual(after.count("PA_FC1_NATIVE TARGET_BIND"), 1)
        self.assertEqual(after.count("PA_FC1_NATIVE TARGET_REVOKE"), 1)
        bind = after.split("struct sway_container *container_create(", 1)[1]
        self.assertLess(bind.index("pa_autoscroll_lifetime_bind("),
                        bind.index("PA_FC1_NATIVE TARGET_BIND"))
        self.assertLess(bind.index("PA_FC1_NATIVE TARGET_BIND"),
                        bind.index("wl_signal_emit_mutable(&root->events.new_node"))
        destroy = after.split("void container_begin_destroy(", 1)[1]
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("PA_FC1_NATIVE TARGET_REVOKE"))
        self.assertLess(destroy.index("PA_FC1_NATIVE TARGET_REVOKE"),
                        destroy.index('ipc_event_window(con, "close")'))
        self.assertIn("container_autoscroll_target_lifetime_epoch(con)", destroy)

    def test_output_owner_hook_instruments_generation_and_revocation(self):
        after = OVERLAY.output_patch(OUTPUT)
        for event in ("OUTPUT_ENABLE", "OUTPUT_CONFIG", "OUTPUT_DISABLE"):
            self.assertEqual(after.count("PA_FC1_NATIVE " + event), 1)
        enable = after.split("void output_enable(", 1)[1].split(
            "static void evacuate_sticky(", 1)[0]
        self.assertLess(enable.index("output->enabled = true;"),
                        enable.index("PA_FC1_NATIVE OUTPUT_ENABLE"))
        self.assertLess(enable.index("PA_FC1_NATIVE OUTPUT_ENABLE"),
                        enable.index("list_add(root->outputs, output)"))
        config = after.split("void output_autoscroll_config_committed(",1)[1].split(
            "void output_enable(",1)[0]
        self.assertLess(config.index("pa_autoscroll_lifetime_bind("),
                        config.index("PA_FC1_NATIVE OUTPUT_CONFIG"))
        disable = after.split("void output_disable(", 1)[1].split(
            "void output_begin_destroy(", 1)[0]
        self.assertLess(disable.index("pa_autoscroll_lifetime_revoke("),
                        disable.index("PA_FC1_NATIVE OUTPUT_DISABLE"))
        self.assertLess(disable.index("PA_FC1_NATIVE OUTPUT_DISABLE"),
                        disable.index("list_del(root->outputs, index)"))

    def test_test_adapter_is_never_part_of_original_production_code(self):
        for src in (TARGET, OUTPUT, RUNTIME, BRIDGE):
            self.assertNotIn("PA_FC1_NATIVE", src)
            self.assertNotIn("PA_AUTOSCROLL_CI_NATIVE_TRACE", src)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN", BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)

    def test_overlay_guards_only_disposable_ci(self):
        code = FILE.read_text()
        for token in ('GITHUB_ACTIONS', 'PA_AUTOSCROLL_CI_NATIVE_TRACE',
                      '/home/runner/work/_temp', 'os.getuid() != 0',
                      'shutil.copytree(', 'git', 'PA_FC1_OVERLAY_FOREIGN_FILES'):
            self.assertIn(token, code)
        harness = (ROOT / "tests/autoscroll/ci_v5f_c1_native_trace.py").read_text()
        self.assertIn("WLR_LIBINPUT_NO_DEVICES", harness)
        self.assertIn('PA_FC1_REAL_COMPOSITOR_LIFETIME_EVIDENCE_PASS', harness)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", harness)

if __name__ == "__main__":
    unittest.main()
