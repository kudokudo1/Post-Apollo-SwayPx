"""V5F-A: fail-closed owner-snapshot native source contract, HARD OFF.

Source sequencing and full compositor build do NOT witness physical hardware.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
C = (ROOT / "sway/input/post_apollo_autoscroll_owner_snapshot.c").read_text()
H = (ROOT / "include/sway/input/post_apollo_autoscroll_owner_snapshot.h").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
INPUT = (ROOT / "sway/input/input-manager.c").read_text()
SEAT = (ROOT / "sway/input/seat.c").read_text()
TARGET = (ROOT / "sway/tree/container.c").read_text()
OUTPUT = (ROOT / "sway/tree/output.c").read_text()

class OwnerSnapshotContract(unittest.TestCase):
    def test_read_only_library_is_linked_with_all_four_producers(self):
        self.assertIn("'input/post_apollo_autoscroll_owner_snapshot.c'", MESON)
        self.assertIn("'input/post_apollo_autoscroll_lifetime.c'", MESON)
        for field in ("device", "seat", "target", "output"):
            self.assertIn(f"uint64_t {field};", H)
        self.assertIn("input_manager_autoscroll_pointer_lifetime_epoch(", INPUT)
        self.assertIn("seat_autoscroll_lifetime_epoch(", SEAT)
        self.assertIn("container_autoscroll_target_lifetime_epoch(", TARGET)
        self.assertIn("output_autoscroll_lifetime_epoch(", OUTPUT)
        for call in (
            "input_manager_autoscroll_pointer_lifetime_epoch(native)",
            "seat_autoscroll_lifetime_epoch(seat)",
            "container_autoscroll_target_lifetime_epoch(target)",
            "output_autoscroll_lifetime_epoch(output)",
        ):
            self.assertIn(call, C)

    def test_no_raw_pointer_address_or_client_id_trust(self):
        self.assertIn("wl_list_for_each(candidate, &server.input->devices, link)", C)
        self.assertIn("candidate->wlr_device == event_device", C)
        self.assertIn("candidate->is_virtual ? NULL : candidate", C)
        self.assertIn("wl_list_for_each(candidate, &server.input->seats, link)", C)
        self.assertIn("wl_list_for_each(seat_device, &seat->devices, link)", C)
        self.assertIn("seat_device->input_device == device", C)
        self.assertNotIn("event_device->data", C)
        self.assertNotIn("(uintptr_t)event_device", C)
        self.assertNotIn("(uint64_t)(uintptr_t)", C)
        self.assertNotIn("pa_autoscroll_lifetime_bind(", C)
        self.assertNotIn("pa_autoscroll_lifetime_revoke(", C)

    def test_no_partial_snapshot_or_escapes_to_virtual_pointer(self):
        fn = C.split("bool pa_autoscroll_owner_snapshot_read(", 1)[1]
        self.assertLess(fn.index("memset(snapshot, 0, sizeof(*snapshot));"),
                        fn.index("find_live_seat(cursor)"))
        self.assertIn("!device_attached_to_seat(seat, native)", fn)
        self.assertIn("cursor->simulating_pointer_from_touch", fn)
        self.assertIn("cursor->simulating_pointer_from_tool_tip", fn)
        self.assertIn("cursor->simulating_pointer_from_tool_button", fn)
        self.assertIn("cursor->active_constraint != NULL", fn)
        self.assertIn("server.session_lock.lock != NULL", fn)
        self.assertLess(fn.index("epochs_unique_and_nonzero(&candidate)"),
                        fn.index("*snapshot = candidate;"))

    def test_direct_hit_transaction_and_output_refusals(self):
        fn = C.split("bool pa_autoscroll_owner_snapshot_read(", 1)[1]
        self.assertIn("node_at_coords(", fn)
        self.assertIn("node->type != N_CONTAINER", fn)
        self.assertIn("node->dirty || node->instruction != NULL", fn)
        self.assertIn("target->view->surface != hit_surface", fn)
        self.assertIn("target->view->destroying", fn)
        self.assertIn("node_get_output(node)", fn)
        self.assertIn("output->wlr_output->enabled", fn)
        self.assertIn("root->fallback_output == output", fn)
        self.assertIn("output->node.instruction != NULL", fn)
        self.assertIn("wlr_output_layout_output_at(", fn)
        self.assertIn("layout_output != output->wlr_output", fn)
        self.assertIn("epochs_unique_and_nonzero", fn)

    def test_hard_off_and_company_first_refusal_unchanged(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", C)
        self.assertNotIn("pa_autoscroll_runtime_button(", C)
        self.assertNotIn("pa_autoscroll_epoch_begin(", C)
        self.assertNotIn("pa_autoscroll_epoch_tick(", C)
        handle = CURSOR.split("static void handle_pointer_button(",1)[1].split("void dispatch_cursor_axis(",1)[0]
        order = ["stage_director_news_pairing_pointer_button(",
                 "stage_director_company_pairing_pointer_button(",
                 "if (weather_paired || company_paired)",
                 "pa_autoscroll_runtime_button(",
                 "dispatch_cursor_button(cursor,"]
        self.assertEqual([handle.index(s) for s in order],
                         sorted(handle.index(s) for s in order))

if __name__ == "__main__":
    unittest.main()
