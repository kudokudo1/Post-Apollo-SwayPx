"""V5F-G1 owner trust boundary and counterfeit witness regression constraints."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
ORIGIN=(ROOT/"sway/input/post_apollo_autoscroll_origin_policy.c").read_text()
ORIGIN_H=(ROOT/"include/sway/input/post_apollo_autoscroll_origin_policy.h").read_text()
TOPO=(ROOT/"sway/input/post_apollo_autoscroll_topology.c").read_text()
WITNESS=(ROOT/"sway/input/post_apollo_autoscroll_witness.c").read_text()
EPOCH=(ROOT/"sway/input/post_apollo_autoscroll_epoch.c").read_text()
SOURCE=(ROOT/"sway/input/post_apollo_autoscroll_source.c").read_text()
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
HARNESS=(ROOT/"tests/autoscroll/ci_v5f_g1_provenance_boundary.c").read_text()

class ProvenanceBoundary(unittest.TestCase):
    def test_no_candidate_to_physical_authority_conversion(self):
        self.assertIn("PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE", ORIGIN)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", ORIGIN)
        self.assertIn("facts->native_scroll_on_button_down", ORIGIN)
        self.assertIn("kernel uinput", ORIGIN_H)
        self.assertIn("return true;", TOPO)
        self.assertIn("CANDIDATE", TOPO)
        self.assertIn("kernel_candidate", SOURCE)
        self.assertIn("return facts;", SOURCE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", SOURCE)

    def test_compositor_bridge_witness_is_explicitly_inadmissible(self):
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN", BRIDGE)
        self.assertIn(".layout_generation_verified = false", BRIDGE)
        self.assertIn(".native_scroll_lock_active = true", BRIDGE)
        self.assertIn(".layout_generation = 0", BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(", BRIDGE)
        self.assertIn("native_unverified_witness(&owner, 0)", BRIDGE)
        self.assertTrue(BRIDGE.rstrip().endswith("return false;\n}"))
        self.assertIn("pa_autoscroll_epoch_begin(", BRIDGE)
        self.assertIn("pa_autoscroll_epoch_tick(", BRIDGE)

    def test_runtime_hard_off_and_first_refusal_survive(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertIn("return NULL;", RUNTIME)
        callback=CURSOR.split("static void handle_pointer_button(",1)[1].split(
            "void dispatch_cursor_axis(",1)[0]
        order=["stage_director_news_pairing_pointer_button(",
               "stage_director_company_pairing_pointer_button(",
               "if (weather_paired || company_paired)",
               "pa_autoscroll_runtime_button(",
               "dispatch_cursor_button(cursor,"]
        self.assertEqual([callback.index(s) for s in order],
                         sorted(callback.index(s) for s in order))

    def test_portable_policy_is_not_its_own_attestation(self):
        # The portable C guard accepts caller-supplied struct fields;
        # only private native ingress can establish their provenance.
        self.assertIn("witness->origin != PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",WITNESS)
        self.assertIn("guard->started = *witness;",EPOCH)
        self.assertIn("pa_autoscroll_epoch_begin(&guard, &forged",HARNESS)
        self.assertIn("portable_guard_accepts_untrusted_forged_fields",HARNESS)
        self.assertIn("same_event_replay_denied",HARNESS)
        self.assertIn("owner_device_change_denied",HARNESS)
        self.assertIn("scene_layout_generation_change_denied",HARNESS)
        self.assertIn("timer_must_not_fabricate_fresh_event",HARNESS)
        for phrase in ("wlr_seat_pointer_notify_axis(",
                       "zwlr_virtual_pointer_v1_button(",
                       "libinput_dispatch(", "wl_display_connect("):
            self.assertNotIn(phrase,HARNESS)

if __name__=="__main__":
    unittest.main()
