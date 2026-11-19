"""V5E-B narrow shared cursor seam, disabled production source assertions."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
SOURCE = (ROOT / "sway/input/post_apollo_autoscroll_source.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
POLICY = (ROOT / "sway/input/post_apollo_autoscroll_origin_policy.c").read_text()

class SharedOriginSeam(unittest.TestCase):
    def test_production_is_hard_off_and_no_input_can_be_consumed(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn(
            "if (cursor == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION)",
            RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertNotIn("getenv(", RUNTIME)

    def test_weather_and_company_first_refusal(self):
        section = CURSOR.split(
            "static void handle_pointer_button(", 1)[1].split(
            "void dispatch_cursor_axis(", 1)[0]
        order = [
            "stage_director_news_pairing_pointer_button(",
            "stage_director_company_pairing_pointer_button(",
            "if (weather_paired || company_paired)",
            "if (cursor->autoscroll != NULL)",
            "pa_autoscroll_source_probe(",
            "pa_autoscroll_runtime_button(",
            "dispatch_cursor_button(cursor,",
        ]
        positions = [section.index(item) for item in order]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("event->state == WL_POINTER_BUTTON_STATE_PRESSED",
                      section)
        self.assertIn("cursor->pressed_button_count++", section)
        self.assertIn("cursor->pressed_button_count--", section)

    def test_candidate_observation_not_physical_authority(self):
        self.assertIn("wlr_input_device_is_libinput(device)", SOURCE)
        self.assertIn("wlr_libinput_get_device_handle(device)", SOURCE)
        self.assertIn("libinput_device_config_scroll_get_method(handle)",
                      SOURCE)
        self.assertIn("LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN", SOURCE)
        self.assertIn("#if WLR_HAS_LIBINPUT_BACKEND", SOURCE)
        self.assertIn("pa_autoscroll_origin_classify(origin)", RUNTIME)
        self.assertIn("PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE",
                      RUNTIME)
        self.assertNotIn("const bool physical_verified = true;", RUNTIME)
        self.assertNotIn("pa_autoscroll_source_probe(", POLICY)
        self.assertIn("'input/post_apollo_autoscroll_source.c'", MESON)
        self.assertIn("'input/post_apollo_autoscroll_origin_policy.c'",
                      MESON)

if __name__ == "__main__":
    unittest.main()
