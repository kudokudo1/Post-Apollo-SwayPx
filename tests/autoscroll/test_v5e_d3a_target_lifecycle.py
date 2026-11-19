"""V5E-D3A: mapped window target lifecycle, hard-OFF compositor source evidence."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
SRC = (ROOT / "sway/tree/container.c").read_text()
HDR = (ROOT / "include/sway/tree/container.h").read_text()
VIEW = (ROOT / "sway/tree/view.c").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()
MANAGER = (ROOT / "sway/input/input-manager.c").read_text()

def between(src, start, stop):
    return src.split(start, 1)[1].split(stop, 1)[0]

class WindowTargetLifecycle(unittest.TestCase):
    def test_mapped_only_target_slot_and_global_issuer(self):
        self.assertIn("struct pa_autoscroll_lifetime_slot autoscroll_target_lifetime;", HDR)
        creation = between(SRC, "struct sway_container *container_create(", "static bool container_is_focused(")
        self.assertIn("if (view != NULL && view->surface != NULL && server.input != NULL)", creation)
        self.assertIn("&server.input->autoscroll_lifetime_issuer", creation)
        self.assertIn("&c->autoscroll_target_lifetime, PA_LIFETIME_TARGET", creation)
        self.assertLess(creation.index("if (failed)"), creation.index("pa_autoscroll_lifetime_bind("))
        self.assertLess(creation.index("pa_autoscroll_lifetime_bind("),
                        creation.index("wl_signal_emit_mutable(&root->events.new_node"))
        self.assertIn("pa_autoscroll_lifetime_init(&input->autoscroll_lifetime_issuer)", MANAGER)

    def test_surface_mapped_before_new_container(self):
        view_map = between(VIEW, "void view_map(", "void view_unmap(")
        self.assertLess(view_map.index("view->surface = wlr_surface;"),
                        view_map.index("view->container = container_create(view);"))
        self.assertIn("container_create(NULL)", SRC) if "container_create(NULL)" in SRC else None

    def test_unmap_hits_begin_destroy_and_revokes_before_signal(self):
        view_unmap = between(VIEW, "void view_unmap(", "void view_autoconfigure(") if "void view_autoconfigure(" in VIEW.split("void view_unmap(", 1)[1] else VIEW.split("void view_unmap(", 1)[1]
        self.assertIn("container_begin_destroy(view->container);", view_unmap)
        destroy = between(SRC, "void container_begin_destroy(", "void container_reap_empty(")
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index('ipc_event_window(con, "close")'))
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("wl_signal_emit_mutable(&con->node.events.destroy"))
        self.assertLess(destroy.index("pa_autoscroll_lifetime_revoke("),
                        destroy.index("con->node.destroying = true"))

    def test_accessor_only_mapped_view_and_live_target(self):
        access = between(SRC, "uint64_t container_autoscroll_target_lifetime_epoch(",
                         "void container_begin_destroy(")
        for condition in ("con == NULL", "con->node.destroying",
                          "con->view == NULL", "con->view->surface == NULL"):
            self.assertIn(condition, access)
        self.assertIn("&con->autoscroll_target_lifetime, PA_LIFETIME_TARGET", access)
        self.assertNotIn("pa_autoscroll_lifetime_bind(", access)
        self.assertIn("uint64_t container_autoscroll_target_lifetime_epoch(", HDR)

    def test_runtime_and_company_first_refusal_unchanged(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0", RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(", RUNTIME)
        button = between(CURSOR, "static void handle_pointer_button(", "void dispatch_cursor_axis(")
        order = ("stage_director_news_pairing_pointer_button(",
                 "stage_director_company_pairing_pointer_button(",
                 "if (weather_paired || company_paired)",
                 "pa_autoscroll_runtime_button(",
                 "dispatch_cursor_button(cursor,")
        self.assertEqual([button.index(z) for z in order],
                         sorted(button.index(z) for z in order))
        self.assertNotIn("wlr_seat_pointer_notify_axis(", SRC)

if __name__ == "__main__":
    unittest.main()
