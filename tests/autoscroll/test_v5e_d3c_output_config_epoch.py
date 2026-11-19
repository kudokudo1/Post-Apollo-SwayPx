"""V5E-D3C: output epoch rotation after successful real backend configuration.

Source-level + native build evidence. No dynamic hotplug or output-layout
event witness, no actual scrolling.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
CONFIG = (ROOT / "sway/config/output.c").read_text()
OUT = (ROOT / "sway/tree/output.c").read_text()
OUT_H = (ROOT / "include/sway/output.h").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()

def part(src, first, after):
    return src.split(first, 1)[1].split(after, 1)[0]

class OutputConfigurationLifetime(unittest.TestCase):
    def test_rotation_only_after_successful_non_test_backend_commit(self):
        apply = part(CONFIG, "static bool apply_resolved_output_configs(",
                     "bool apply_output_configs(")
        self.assertLess(apply.index("if (test_only)"),
                        apply.index("ok = wlr_backend_commit(server.backend"))
        self.assertLess(apply.index("ok = wlr_backend_commit(server.backend"),
                        apply.index("if (!ok) {\n\t\tsway_log(SWAY_ERROR, \"Backend commit failed\")"))
        self.assertLess(apply.index("if (!ok) {\n\t\tsway_log(SWAY_ERROR, \"Backend commit failed\")"),
                        apply.index("output_autoscroll_config_committed(cfg->output)"))
        self.assertLess(apply.index("finalize_output_config(cfg->config"),
                        apply.index("output_autoscroll_config_committed(cfg->output)"))
        self.assertLess(apply.index("output_update_position(cfg->output)"),
                        apply.index("output_autoscroll_config_committed(cfg->output)"))
        self.assertLess(apply.index("output_autoscroll_config_committed(cfg->output)"),
                        apply.index("arrange_layers(cfg->output)"))
        self.assertLess(apply.index("output_autoscroll_config_committed(cfg->output)"),
                        apply.index("arrange_root()"))
        self.assertEqual(apply.count("output_autoscroll_config_committed(cfg->output)"), 1)

    def test_compositor_owned_output_rotation_fails_closed(self):
        rotate = part(OUT, "void output_autoscroll_config_committed(",
                      "void output_enable(")
        self.assertIn("output == NULL", rotate)
        self.assertIn("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime)", rotate)
        for guard in ("!output->enabled", "output->node.destroying",
                      "output->wlr_output == NULL", "root == NULL",
                      "root->fallback_output == output", "server.input == NULL"):
            self.assertIn(guard, rotate)
        self.assertLess(rotate.index("pa_autoscroll_lifetime_revoke("),
                        rotate.index("pa_autoscroll_lifetime_bind("))
        self.assertIn("&server.input->autoscroll_lifetime_issuer", rotate)
        self.assertIn("&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT", rotate)
        self.assertIn("void output_autoscroll_config_committed(", OUT_H)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", rotate)

    def test_existing_enable_disable_lifetime_remains(self):
        enable = part(OUT, "void output_enable(", "static void evacuate_sticky(")
        disable = part(OUT, "void output_disable(", "void output_begin_destroy(")
        destroy = part(OUT, "void output_begin_destroy(", "struct sway_output *output_from_wlr_output(")
        self.assertIn("pa_autoscroll_lifetime_bind(", enable)
        self.assertIn("pa_autoscroll_lifetime_revoke(", enable)
        self.assertIn("pa_autoscroll_lifetime_revoke(", disable)
        self.assertIn("pa_autoscroll_lifetime_revoke(", destroy)

    def test_production_hard_off_and_company_first_refusal(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        button = part(CURSOR, "static void handle_pointer_button(",
                      "void dispatch_cursor_axis(")
        order = ["stage_director_news_pairing_pointer_button(",
                 "stage_director_company_pairing_pointer_button(",
                 "if (weather_paired || company_paired)",
                 "pa_autoscroll_runtime_button(",
                 "dispatch_cursor_button(cursor,"]
        self.assertEqual([button.index(x) for x in order],
                         sorted(button.index(x) for x in order))

if __name__ == "__main__":
    unittest.main()
