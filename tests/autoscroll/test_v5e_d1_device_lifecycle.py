"""V5E-D1: compositor-owned input device lifetime source wiring (no actuation)."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]
MANAGER = (ROOT / "sway/input/input-manager.c").read_text()
HEADER = (ROOT / "include/sway/input/input-manager.h").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()

def section(start, end):
    return MANAGER.split(start, 1)[1].split(end, 1)[0]

class CompositorDeviceLifetime(unittest.TestCase):
    def test_issuer_owned_by_input_manager(self):
        self.assertIn("struct pa_autoscroll_lifetime_issuer autoscroll_lifetime_issuer;", HEADER)
        self.assertIn("struct pa_autoscroll_lifetime_slot autoscroll_lifetime;", HEADER)
        create = section("struct sway_input_manager *input_manager_create(", "void input_manager_finish(")
        self.assertIn("pa_autoscroll_lifetime_init(&input->autoscroll_lifetime_issuer)", create)
        self.assertLess(
            create.index("pa_autoscroll_lifetime_init("),
            create.index("wl_signal_add(&server->backend->events.new_input"))
        self.assertIn("'input/post_apollo_autoscroll_lifetime.c'", MESON)

    def test_real_pointer_bind_before_visibility(self):
        new = section("static void handle_new_input(", "static void handle_keyboard_shortcuts_inhibitor_destroy(")
        self.assertLess(new.index("pa_autoscroll_bind_pointer_lifetime(input, input_device)"),
                        new.index("wl_list_insert(&input->devices"))
        self.assertLess(new.index("pa_autoscroll_bind_pointer_lifetime(input, input_device)"),
                        new.index("wl_signal_add(&device->events.destroy"))
        helper = section("static void pa_autoscroll_bind_pointer_lifetime(", "uint64_t input_manager_autoscroll_pointer_lifetime_epoch(")
        self.assertIn("device->type == WLR_INPUT_DEVICE_POINTER", helper)
        self.assertIn("&manager->autoscroll_lifetime_issuer", helper)
        self.assertIn("PA_LIFETIME_DEVICE", helper)

    def test_virtual_pointer_is_tracked_but_never_admitted(self):
        virtual = section("void handle_virtual_pointer(", "static void handle_transient_seat_manager_create_seat(")
        self.assertLess(virtual.index("input_device->is_virtual = true"),
                        virtual.index("pa_autoscroll_bind_pointer_lifetime(input_manager, input_device)"))
        self.assertLess(virtual.index("pa_autoscroll_bind_pointer_lifetime(input_manager, input_device)"),
                        virtual.index("wl_list_insert(&input_manager->devices"))
        access = section("uint64_t input_manager_autoscroll_pointer_lifetime_epoch(", "struct input_config *current_input_config")
        self.assertIn("input_device->is_virtual", access)
        self.assertIn("WLR_INPUT_DEVICE_POINTER", access)
        self.assertIn("pa_autoscroll_lifetime_epoch(", access)
        self.assertIn("PA_LIFETIME_DEVICE", access)

    def test_revoke_precedes_seat_removal_and_free(self):
        destroy = section("static void handle_device_destroy(", "static void handle_new_input(")
        self.assertIn("pa_autoscroll_lifetime_revoke(&input_device->autoscroll_lifetime)", destroy)
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("seat_remove_device("))
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("free(input_device);"))

    def test_not_real_hardware_authentication_or_live_autoscroll(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        manager = section("struct sway_input_manager *input_manager_create(", "void input_manager_finish(")
        self.assertNotIn("pa_scroll_dispatch_enable(", manager)
        self.assertNotIn("pa_autoscroll_runtime_button(", MANAGER)
        self.assertNotIn("wlr_seat_pointer_notify_axis(", MANAGER)

if __name__ == "__main__":
    unittest.main()
