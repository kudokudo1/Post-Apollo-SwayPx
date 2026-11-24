"""V5F-B: portable V5C guard linked to real owner source and runtime safety.

This is a source/native-build boundary test, not a real physical pointer
witness. Hard-OFF must survive any later accidental dispatch enable attempt.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
BRIDGE_H = (ROOT / "include/sway/input/post_apollo_autoscroll_guard_bridge.h").read_text()
EPOCH = (ROOT / "sway/input/post_apollo_autoscroll_epoch.c").read_text()
EPOCH_H = (ROOT / "include/sway/input/post_apollo_autoscroll_epoch.h").read_text()
WITNESS = (ROOT / "sway/input/post_apollo_autoscroll_witness.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
OWNER = (ROOT / "sway/input/post_apollo_autoscroll_owner_snapshot.c").read_text()

def between(src, start, end):
    return src.split(start, 1)[1].split(end, 1)[0]

class V5FBNativeGuard(unittest.TestCase):
    def test_exact_v5c_policy_is_linked_with_owner_issuer(self):
        for name in (
            "post_apollo_autoscroll_epoch.c",
            "post_apollo_autoscroll_witness.c",
            "post_apollo_autoscroll_guard_bridge.c",
            "post_apollo_autoscroll_owner_snapshot.c",
            "post_apollo_autoscroll_lifetime.c",
        ):
            self.assertIn(f"'input/{name}'", MESON)
        self.assertIn("pa_autoscroll_epoch_begin(", EPOCH)
        self.assertIn("pa_autoscroll_epoch_tick(", EPOCH)
        self.assertIn("pa_autoscroll_epoch_cancel(", EPOCH)
        self.assertIn("pa_autoscroll_witness_admissible(", WITNESS)
        self.assertIn("pa_autoscroll_witness_same_session(", WITNESS)
        self.assertIn("struct pa_autoscroll_object_epochs", EPOCH_H)

    def test_event_begin_uses_owner_source_not_raw_ptr_ids(self):
        begin = between(BRIDGE, "bool pa_autoscroll_guard_bridge_begin(",
                        "bool pa_autoscroll_guard_bridge_tick(")
        self.assertIn("pa_autoscroll_owner_snapshot_read(cursor, device, &owner)", begin)
        self.assertIn("owner_epochs(&owner)", begin)
        self.assertIn("native_unverified_witness(&owner, event_at_ms)", begin)
        self.assertLess(begin.index("pa_autoscroll_owner_snapshot_read("),
                        begin.index("pa_autoscroll_epoch_begin("))
        self.assertIn("pa_autoscroll_guard_bridge_cancel(bridge);", begin)
        self.assertTrue(begin.rstrip().endswith("return false;\n}"))
        self.assertNotIn("(uintptr_t)", BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(", BRIDGE)

    def test_native_witness_is_deny_by_default(self):
        native = between(BRIDGE, "static struct pa_autoscroll_witness native_unverified_witness(",
                         "void pa_autoscroll_guard_bridge_cancel(")
        for denial in (
            ".origin = PA_POINTER_ORIGIN_UNKNOWN",
            ".layout_generation_verified = false",
            ".native_scroll_lock_active = true",
            ".layout_generation = 0",
        ):
            self.assertIn(denial, native)
        self.assertIn(".observed_at_ms = event_at_ms", native)
        self.assertNotIn(".origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", native)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", BRIDGE)
        self.assertNotIn("pa_autoscroll_lifetime_bind(", BRIDGE)

    def test_timer_never_fabricates_new_event_freshness(self):
        tick = BRIDGE.split("bool pa_autoscroll_guard_bridge_tick(", 1)[1]
        self.assertIn("pa_autoscroll_owner_snapshot_read(cursor, device, &owner)", tick)
        self.assertIn("native_unverified_witness(&owner, 0)", tick)
        self.assertIn("pa_autoscroll_epoch_tick(", tick)
        self.assertIn("pa_autoscroll_guard_bridge_cancel(bridge);", tick)
        self.assertTrue(tick.rstrip().endswith("return false;\n}"))
        self.assertNotIn("native_unverified_witness(&owner, now_ms)", tick)

    def test_actual_runtime_guard_order_before_axis_or_frame(self):
        self.assertIn("struct pa_autoscroll_guard_bridge owner_guard;", RUNTIME)
        self.assertIn("!runtime->owner_guard.epoch.active", RUNTIME)
        timer = between(RUNTIME, "static int autoscroll_timer_tick(",
                        "struct pa_autoscroll_runtime *pa_autoscroll_runtime_create(")
        self.assertLess(timer.index("pa_autoscroll_guard_bridge_tick("),
                        timer.index("pa_scroll_dispatch_tick("))
        self.assertIn("pa_autoscroll_runtime_cancel(runtime);", timer)
        buttons = between(RUNTIME, "bool pa_autoscroll_runtime_button(",
                          "bool pa_autoscroll_runtime_motion(")
        self.assertLess(buttons.index("pa_autoscroll_guard_bridge_begin("),
                        buttons.index("pa_scroll_dispatch_middle("))
        self.assertIn("pa_autoscroll_runtime_cancel(runtime);", buttons)
        stop = between(RUNTIME, "void pa_autoscroll_runtime_cancel(",
                       "void pa_autoscroll_runtime_destroy(")
        self.assertIn("pa_autoscroll_guard_bridge_cancel(&runtime->owner_guard)", stop)
        frames = between(RUNTIME, "static void send_axis_frame(",
                         "static int autoscroll_timer_tick(")
        self.assertIn("runtime->owner_guard.epoch.active", frames)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)

    def test_default_off_and_company_first_refusal(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", BRIDGE)
        self.assertNotIn("wlr_seat_pointer_notify_frame(", BRIDGE)
        handle = between(CURSOR, "static void handle_pointer_button(",
                         "void dispatch_cursor_axis(")
        order = [
            "stage_director_news_pairing_pointer_button(",
            "stage_director_company_pairing_pointer_button(",
            "if (weather_paired || company_paired)",
            "pa_autoscroll_runtime_button(",
            "dispatch_cursor_button(cursor,",
        ]
        self.assertEqual([handle.index(z) for z in order],
                         sorted(handle.index(z) for z in order))
        self.assertNotIn("pa_autoscroll_epoch_begin(", OWNER)
        self.assertNotIn("pa_autoscroll_epoch_tick(", OWNER)

if __name__ == "__main__":
    unittest.main()
