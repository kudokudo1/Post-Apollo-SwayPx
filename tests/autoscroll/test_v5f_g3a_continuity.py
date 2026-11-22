"""V5F-G3A: detached timer-vs-event continuity source safety."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
HEADER = (ROOT / "include/sway/input/post_apollo_autoscroll_continuity.h").read_text()
CANDIDATE = (ROOT / "sway/input/post_apollo_autoscroll_continuity.c").read_text()
TEST = (ROOT / "tests/autoscroll/ci_v5f_g3a_continuity.c").read_text()
EPOCH = (ROOT / "sway/input/post_apollo_autoscroll_epoch.c").read_text()
WITNESS = (ROOT / "sway/input/post_apollo_autoscroll_witness.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()

class ContinuityCandidateSafety(unittest.TestCase):
    def test_detached_no_runtime_or_build_wiring(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN", BRIDGE)
        for source in (RUNTIME, BRIDGE, MESON):
            self.assertNotIn("post_apollo_autoscroll_continuity", source)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", CANDIDATE)
        self.assertNotIn("wlr_seat_pointer_notify_frame(", CANDIDATE)
        self.assertNotIn("wl_event_loop_add_timer(", CANDIDATE)

    def test_real_v5c_start_validation_preserved_no_fake_tick_event(self):
        self.assertIn("pa_autoscroll_epoch_begin(&guard->start", CANDIDATE)
        self.assertIn("pa_autoscroll_epoch_tick(", EPOCH)
        self.assertIn("witness->observed_at_ms <= guard->last_observed_at_ms", EPOCH)
        self.assertNotIn("pa_autoscroll_epoch_tick(", CANDIDATE)
        self.assertNotIn("observed_at_ms =", CANDIDATE)
        self.assertIn("PA_AUTOSCROLL_WITNESS_MAX_AGE_MS", WITNESS)
        self.assertIn("PA_AUTOSCROLL_CONTINUITY_MAX_SCENE_AGE_MS", HEADER)
        self.assertIn("PA_AUTOSCROLL_CONTINUITY_MAX_TICK_GAP_MS", HEADER)
        self.assertIn("scene->captured_at_ms", CANDIDATE)
        self.assertIn("now_ms <= guard->last_tick_ms", CANDIDATE)

    def test_all_four_epochs_and_scene_binding_checked(self):
        for name in (
            "device", "seat", "target", "output"
        ):
            self.assertIn(f"started->{name} == current->{name}", CANDIDATE)
        for name in (
            "device_id", "seat_id", "target_node_id", "output_id",
            "layout_generation"
        ):
            self.assertIn(f"scene->{name} == start->{name}", CANDIDATE)
        for name in (
            "surface_hit_verified", "target_mapped", "output_enabled",
            "layout_generation_verified", "session_locked",
            "native_scroll_lock_active", "stage_director_pairing_consumed",
            "pointer_constrained"
        ):
            self.assertIn(f"scene->{name}", CANDIDATE)
        self.assertIn("pa_autoscroll_continuity_cancel(guard)", CANDIDATE)

    def test_detached_matrix_explicitly_not_hardware_evidence(self):
        for value in (
            "input_events=ONE_NO_FORGERY",
            "actual_legacy_tick_rejects_stationary_replay",
            "ticks=1000",
            "device_epoch", "seat_epoch", "target_epoch", "output_epoch",
            "session_lock", "native_scroll_lock", "layout_rotation",
            "stale_scene", "future_scene", "no_real_scene_sample",
            "virtual_start_denied", "long_schedule_gap_no_catchup_burst",
            "native_producer=ABSENT production_axis_frames=ZERO"
        ):
            self.assertIn(value, TEST)
        self.assertIn("UNLINKED, DETACHED", CANDIDATE)
        self.assertIn("nothing in this file attests a mouse", HEADER.lower())

if __name__ == "__main__":
    unittest.main()
