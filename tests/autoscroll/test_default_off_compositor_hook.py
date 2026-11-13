"""Source contract for guarded, compiled SwayPX cursor integration."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
HEADER = (ROOT / "include/sway/input/cursor.h").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()


class DefaultOffCompositorHooks(unittest.TestCase):
    def test_compiled_into_actual_compositor(self):
        for file in ("post_apollo_autoscroll.c",
                     "post_apollo_autoscroll_adapter.c",
                     "post_apollo_autoscroll_axis_dispatch.c",
                     "post_apollo_autoscroll_runtime.c"):
            self.assertIn("'input/" + file + "'", MESON)
        self.assertIn("struct pa_autoscroll_runtime *autoscroll;", HEADER)
        self.assertIn("pa_autoscroll_runtime_create(cursor)", CURSOR)
        self.assertIn("pa_autoscroll_runtime_destroy(cursor->autoscroll)", CURSOR)

    def test_live_input_does_not_activate_or_modify_mouse(self):
        self.assertRegex(RUNTIME,
                         r"#define\s+PA_AUTOSCROLL_LIVE_ACTIVATION\s+0\b")
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertNotIn("PA_AUTOSCROLL_LIVE_ACTIVATION 1", RUNTIME)
        self.assertNotIn("getenv(", RUNTIME)
        self.assertNotIn("SWAY_AUTOSCROLL_ENABLE", RUNTIME)
        for command in ("sudo", "pkexec", "system(", "popen(",
                        "uinput", "EVIOCGRAB", "swaymsg", "bindsym"):
            self.assertNotIn(command, RUNTIME)
        self.assertIn("if (!PA_AUTOSCROLL_LIVE_ACTIVATION", RUNTIME)
        self.assertIn("if (cursor == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION)", RUNTIME)
        self.assertLess(RUNTIME.index("if (cursor == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION)"),
                        RUNTIME.index("calloc(1, sizeof(*runtime))"))
        self.assertLess(RUNTIME.index("if (cursor == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION)"),
                        RUNTIME.index("wl_event_loop_add_timer("))
        self.assertIn("wl_event_source_timer_update(runtime->timer, -1)",
                      RUNTIME)

    def test_preserves_both_existing_stage_director_pairings(self):
        start = CURSOR.index("static void handle_pointer_button(")
        end = CURSOR.index("void dispatch_cursor_axis(", start)
        section = CURSOR[start:end]
        weather = section.index("stage_director_news_pairing_pointer_button(")
        company = section.index("stage_director_company_pairing_pointer_button(")
        pairing_guard = section.index("if (weather_paired || company_paired)")
        auto = section.index("pa_autoscroll_runtime_button(")
        normal = section.index("dispatch_cursor_button(cursor,")
        self.assertLess(weather, company)
        self.assertLess(company, pairing_guard)
        self.assertLess(pairing_guard, auto)
        self.assertLess(auto, normal)
        self.assertIn("event->state == WL_POINTER_BUTTON_STATE_PRESSED", section)
        self.assertIn("cursor->pressed_button_count++", section)
        self.assertIn("cursor->pressed_button_count--", section)

    def test_motion_passes_through_when_disabled(self):
        start = CURSOR.index("static void handle_pointer_motion_relative(")
        end = CURSOR.index("static void handle_pointer_motion_absolute(", start)
        motion = CURSOR[start:end]
        self.assertLess(motion.index("pa_autoscroll_runtime_motion("),
                        motion.index("pointer_motion(cursor,"))
        self.assertIn("e->delta_x, e->delta_y", motion)
        self.assertIn("return;", motion)

    def test_no_unapproved_writable_controls_or_compositor_mode(self):
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertIn("Passing layout_generation=0", RUNTIME)
        self.assertIn("false, true,", RUNTIME)
        self.assertNotIn("wl_global_create(", RUNTIME)
        self.assertNotIn("ipc_client_handle_command(", RUNTIME)


if __name__ == "__main__":
    unittest.main()
