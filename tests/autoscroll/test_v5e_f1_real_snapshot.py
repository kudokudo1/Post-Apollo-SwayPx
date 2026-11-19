"""V5E-F1: real SwayPX owner epoch snapshot, fail-closed and HARD OFF.

These are source assertions, not native observer event receipts or proof of
physical device provenance.
"""
from pathlib import Path
import unittest

R = Path(__file__).resolve().parents[2]
SOURCE = (R/"sway/input/post_apollo_autoscroll_snapshot.c").read_text()
HEADER = (R/"include/sway/input/post_apollo_autoscroll_snapshot.h").read_text()
RUNTIME = (R/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
MESON = (R/"sway/meson.build").read_text()
MANAGER = (R/"sway/input/input-manager.c").read_text()
SEAT = (R/"sway/input/seat.c").read_text()
CONTAINER = (R/"sway/tree/container.c").read_text()
OUTPUT = (R/"sway/tree/output.c").read_text()
CONFIG = (R/"sway/config/output.c").read_text()
V5C = (R/"sway/input/post_apollo_autoscroll_epoch.c").read_text()

class OwnerSnapshot(unittest.TestCase):
    def test_non_authorizing_real_compositor_snapshot(self):
        self.assertIn("pa_autoscroll_collect_compositor_epochs(", HEADER)
        self.assertIn("pa_autoscroll_collect_compositor_epochs(", SOURCE)
        self.assertIn("memset(out, 0, sizeof(*out));", SOURCE)
        self.assertIn("server.session_lock.lock != NULL", SOURCE)
        self.assertIn("cursor->active_constraint != NULL", SOURCE)
        self.assertIn("root->output_layout == NULL", SOURCE)
        self.assertIn("root->outputs == NULL", SOURCE)
        self.assertNotIn("pa_autoscroll_epoch_begin(", SOURCE)
        self.assertNotIn("pa_autoscroll_epoch_tick(", SOURCE)
        self.assertNotIn("pa_scroll_dispatch_enable(", SOURCE)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", SOURCE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT", SOURCE)
        self.assertNotIn("GenUI.", SOURCE)

    def test_never_trust_unregistered_pointer_or_seat(self):
        markers = [
            "wl_list_for_each(candidate_seat, &server.input->seats, link)",
            "candidate_seat == cursor->seat",
            "seat == NULL || seat->cursor != cursor",
            "wl_list_for_each(candidate, &server.input->devices, link)",
            "candidate->wlr_device == device",
            "input_device == NULL || input_device->is_virtual",
            "wl_list_for_each(seat_device, &seat->devices, link)",
            "seat_device->input_device == input_device",
            "if (!seat_attached)",
        ]
        positions = [SOURCE.index(x) for x in markers]
        self.assertEqual(positions, sorted(positions))
        self.assertLess(SOURCE.index("if (!seat_attached)"),
                        SOURCE.index("node_at_coords("))

    def test_actual_scene_hit_and_active_output_before_owner_epoch(self):
        markers = [
            "node_at_coords(seat, cursor->cursor->x",
            "node->type != N_CONTAINER",
            "surface == NULL",
            "wlr_output_layout_output_at(",
            "for (int i = 0; i < root->outputs->length; ++i)",
            "candidate_output->wlr_output == wlr_output",
            "node_get_output(node) != output",
            "input_manager_autoscroll_pointer_lifetime_epoch(input_device)",
            "seat_autoscroll_lifetime_epoch(seat)",
            "container_autoscroll_target_lifetime_epoch(node->sway_container)",
            "output_autoscroll_lifetime_epoch(output)",
            "snapshot.device == 0 || snapshot.seat == 0",
            "*out = snapshot",
        ]
        positions = [SOURCE.index(x) for x in markers]
        self.assertEqual(positions, sorted(positions))

    def test_actual_compositor_owners_mint_and_revoke(self):
        self.assertIn("pa_autoscroll_bind_pointer_lifetime(input, input_device)", MANAGER)
        self.assertIn("pa_autoscroll_lifetime_revoke(&input_device->autoscroll_lifetime)", MANAGER)
        self.assertIn("&seat->autoscroll_lifetime, PA_LIFETIME_SEAT", SEAT)
        self.assertIn("pa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime)", SEAT)
        self.assertIn("&c->autoscroll_target_lifetime, PA_LIFETIME_TARGET", CONTAINER)
        self.assertIn("pa_autoscroll_lifetime_revoke(&con->autoscroll_target_lifetime)", CONTAINER)
        self.assertIn("&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT", OUTPUT)
        self.assertIn("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime)", OUTPUT)
        self.assertIn("output_autoscroll_config_committed(cfg->output)", CONFIG)

    def test_compiled_guard_not_active_and_live_runtime_hard_off(self):
        for name in ("post_apollo_autoscroll_snapshot.c", "post_apollo_autoscroll_epoch.c",
                     "post_apollo_autoscroll_witness.c"):
            self.assertIn("input/" + name, MESON)
        self.assertIn("pa_autoscroll_epoch_tick(", V5C)
        self.assertIn("pa_autoscroll_epoch_cancel(guard)", V5C)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_autoscroll_collect_compositor_epochs(", RUNTIME)
        self.assertNotIn("pa_autoscroll_epoch_begin(", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)

if __name__ == "__main__":
    unittest.main()
