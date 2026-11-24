"""V5E-D2: seat identity is minted/revoked at actual compositor callbacks.

Source evidence, not an assertion that real seat events were dynamically traced.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
SEAT = (ROOT / "sway/input/seat.c").read_text()
SEAT_H = (ROOT / "include/sway/input/seat.h").read_text()
INPUT_H = (ROOT / "include/sway/input/input-manager.h").read_text()
INPUT = (ROOT / "sway/input/input-manager.c").read_text()
SERVER = (ROOT / "sway/server.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()

def between(s, before, after):
    return s.split(before, 1)[1].split(after, 1)[0]

class SeatLifetimeOwner(unittest.TestCase):
    def test_single_shared_issuer_comes_before_default_seat(self):
        created = between(SERVER, "server->dirty_nodes = create_list();",
                          "animation_manager_init(server);")
        self.assertLess(created.index("server->input = input_manager_create(server);"),
                        created.index("input_manager_get_default_seat()"))
        manager = between(INPUT, "struct sway_input_manager *input_manager_create(",
                          "void input_manager_finish(")
        self.assertLess(manager.index("pa_autoscroll_lifetime_init("),
                        manager.index("wl_signal_add(&server->backend->events.new_input"))
        self.assertIn("struct pa_autoscroll_lifetime_issuer autoscroll_lifetime_issuer;", INPUT_H)

    def test_real_seat_slot_minted_only_after_success_but_before_exposure(self):
        self.assertIn("struct pa_autoscroll_lifetime_slot autoscroll_lifetime;", SEAT_H)
        creation = between(SEAT, "struct sway_seat *seat_create(", "static void seat_update_capabilities(")
        self.assertLess(creation.index("seat->cursor = sway_cursor_create(seat)"),
                        creation.index("pa_autoscroll_lifetime_bind("))
        self.assertLess(creation.index("if (!seat->cursor)"),
                        creation.index("pa_autoscroll_lifetime_bind("))
        self.assertLess(creation.index("pa_autoscroll_lifetime_bind("),
                        creation.index("wl_signal_add(&seat->wlr_seat->events.destroy"))
        self.assertLess(creation.index("pa_autoscroll_lifetime_bind("),
                        creation.index("wl_list_insert(&server.input->seats"))
        self.assertIn("&server.input->autoscroll_lifetime_issuer", creation)
        self.assertIn("&seat->autoscroll_lifetime, PA_LIFETIME_SEAT", creation)
        self.assertIn("if (server.input != NULL)", creation)
        self.assertIn("seat->wlr_seat->data = seat;", creation)

    def test_seat_revocation_is_first_action_on_destroy(self):
        teardown = between(SEAT, "static void handle_seat_destroy(", "void seat_idle_notify_activity(")
        self.assertLess(teardown.index("pa_autoscroll_lifetime_revoke("),
                        teardown.index("config->handler_context.seat"))
        self.assertLess(teardown.index("pa_autoscroll_lifetime_revoke("),
                        teardown.index("seat_device_destroy("))
        self.assertLess(teardown.index("pa_autoscroll_lifetime_revoke("),
                        teardown.index("sway_cursor_destroy("))
        self.assertLess(teardown.index("pa_autoscroll_lifetime_revoke("),
                        teardown.index("free(seat);"))
        self.assertIn("wlr_seat_destroy(seat->wlr_seat)", SEAT)

    def test_accessor_is_read_only_and_zero_after_revocation(self):
        accessor = between(SEAT, "uint64_t seat_autoscroll_lifetime_epoch(",
                           "void seat_destroy(")
        self.assertIn("seat == NULL || seat->wlr_seat == NULL", accessor)
        self.assertIn("&seat->autoscroll_lifetime, PA_LIFETIME_SEAT", accessor)
        self.assertNotIn("pa_autoscroll_lifetime_bind(", accessor)
        self.assertNotIn("pa_scroll_dispatch_enable(", accessor)
        self.assertIn("uint64_t seat_autoscroll_lifetime_epoch(", SEAT_H)
        self.assertIn("seat_create(name)", INPUT)  # covers transient seats too

    def test_stage_director_and_production_hard_off_unchanged(self):
        buttons = between(CURSOR, "static void handle_pointer_button(",
                          "void dispatch_cursor_axis(")
        order = [
            "stage_director_news_pairing_pointer_button(",
            "stage_director_company_pairing_pointer_button(",
            "if (weather_paired || company_paired)",
            "pa_autoscroll_runtime_button(",
            "dispatch_cursor_button(cursor,",
        ]
        positions = [buttons.index(x) for x in order]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", SEAT)

if __name__ == "__main__":
    unittest.main()
