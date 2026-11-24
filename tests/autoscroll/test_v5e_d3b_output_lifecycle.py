"""V5E-D3B source-contract tests for actual SwayPX output availability lifetimes.

This is compile/source evidence, NOT a native monitor hotplug token trace.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
OUT = (ROOT / "sway/tree/output.c").read_text()
OUT_H = (ROOT / "include/sway/output.h").read_text()
DESKTOP_OUT = (ROOT / "sway/desktop/output.c").read_text()
SERVER = (ROOT / "sway/server.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
INPUT_H = (ROOT / "include/sway/input/input-manager.h").read_text()

def segment(src, first, after):
    return src.split(first, 1)[1].split(after, 1)[0]

class OutputLifetimeAvailability(unittest.TestCase):
    def test_typed_slot_and_same_input_issuer(self):
        self.assertIn("struct pa_autoscroll_lifetime_slot autoscroll_output_lifetime;", OUT_H)
        self.assertIn("uint64_t output_autoscroll_lifetime_epoch(", OUT_H)
        self.assertIn("struct pa_autoscroll_lifetime_issuer autoscroll_lifetime_issuer;", INPUT_H)
        self.assertIn("server->input = input_manager_create(server);", SERVER)

    def test_enable_issues_new_token_before_exposure(self):
        enable = segment(OUT, "void output_enable(", "static void evacuate_sticky(")
        self.assertIn("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime)", enable)
        self.assertIn("if (server.input != NULL && root != NULL", enable)
        self.assertIn("output != root->fallback_output", enable)
        self.assertIn("&server.input->autoscroll_lifetime_issuer", enable)
        self.assertIn("&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT", enable)
        self.assertLess(enable.index("pa_autoscroll_lifetime_revoke("),
                        enable.index("pa_autoscroll_lifetime_bind("))
        self.assertLess(enable.index("pa_autoscroll_lifetime_bind("),
                        enable.index("output->enabled = true"))
        self.assertLess(enable.index("pa_autoscroll_lifetime_bind("),
                        enable.index("list_add(root->outputs, output)"))

    def test_disable_revokes_before_workspace_evacuations(self):
        disable = segment(OUT, "void output_disable(", "void output_begin_destroy(")
        self.assertIn("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime)", disable)
        self.assertLess(disable.index("pa_autoscroll_lifetime_revoke("),
                        disable.index("list_del(root->outputs, index)"))
        self.assertLess(disable.index("pa_autoscroll_lifetime_revoke("),
                        disable.index("output_evacuate(output)"))
        self.assertLess(disable.index("pa_autoscroll_lifetime_revoke("),
                        disable.index("output->enabled = false"))

    def test_final_destroy_revoke_before_notification(self):
        destroy = segment(OUT, "void output_begin_destroy(", "struct sway_output *output_from_wlr_output(")
        self.assertIn("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime)", destroy)
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("wl_signal_emit_mutable(&output->node.events.destroy"))
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("output->node.destroying = true"))
        native_destroy = segment(DESKTOP_OUT, "static void begin_destroy(", "static void handle_destroy(")
        self.assertLess(native_destroy.index("if (output->enabled)"),
                        native_destroy.index("output_begin_destroy(output);"))

    def test_read_only_accessor_requires_live_enabled_nonfallback(self):
        access = segment(OUT, "uint64_t output_autoscroll_lifetime_epoch(", "void output_autoscroll_config_committed(")
        for condition in ("output == NULL", "!output->enabled",
                          "output->wlr_output == NULL", "root == NULL",
                          "root->fallback_output == output", "output->node.destroying"):
            self.assertIn(condition, access)
        self.assertIn("&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT", access)
        self.assertNotIn("pa_autoscroll_lifetime_bind(", access)
        self.assertNotIn("pa_autoscroll_lifetime_revoke(", access)

    def test_no_autoscroll_activation(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", OUT)

if __name__ == "__main__":
    unittest.main()
