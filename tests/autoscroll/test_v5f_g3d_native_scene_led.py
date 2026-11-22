"""G3D source safety: native scene/XKB audit remains unattested and CI-only."""
from pathlib import Path
import importlib.util
import sys
import unittest

ROOT=Path(__file__).resolve().parents[2]
TESTS=ROOT/"tests/autoscroll"
sys.path.insert(0,str(TESTS))
spec=importlib.util.spec_from_file_location(
    "pa_g3d_native_overlay", TESTS/"ci_v5f_g3d_native_overlay.py")
OVERLAY=importlib.util.module_from_spec(spec)
spec.loader.exec_module(OVERLAY)
COMMAND=(ROOT/"sway/commands/seat/cursor.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
PROBE=(TESTS/"ci_v5f_g3d_native_probe.c").read_text()
CLIENT=(TESTS/"ci_v5f_g3d_virtual_keyboard_client.c").read_text()
TRACE=(TESTS/"ci_v5f_g3d_native_trace.py").read_text()
WRAPPER=(TESTS/"ci_v5f_g3d_udmabuf.sh").read_text()

class G3DNativeReadOnly(unittest.TestCase):
    def test_test_probe_not_compiled_into_production(self):
        for source in (COMMAND,MESON,RUNTIME,BRIDGE):
            self.assertNotIn("pa_g3d_native_sample_on_cursor_set",source)
            self.assertNotIn("ci_v5f_g3d_native_probe",source)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)

    def test_exact_copy_overlay_and_input_dispatch_order(self):
        patched=OVERLAY.command_patch(COMMAND)
        meson=OVERLAY.meson_patch(MESON)
        self.assertEqual(patched.count("pa_g3d_native_sample_on_cursor_set(cursor);"),1)
        cursor_set=patched.index("wlr_cursor_warp_absolute(cursor->cursor, NULL, x, y);")
        rebase=patched.index("cursor_rebase(cursor);",cursor_set)
        probe=patched.index("pa_g3d_native_sample_on_cursor_set(cursor);")
        frame=patched.index("wlr_seat_pointer_notify_frame(cursor->seat->wlr_seat);",probe)
        self.assertLess(cursor_set,rebase)
        self.assertLess(rebase,probe)
        self.assertLess(probe,frame)
        self.assertIn("'../tests/autoscroll/ci_v5f_g3d_native_probe.c'",meson)
        self.assertNotIn("post_apollo_autoscroll_continuity.c",meson)

    def test_compositor_native_hit_and_led_not_authorization(self):
        for token in (
            "node_at_coords(",
            "target->view->surface == surface",
            "wlr_output_layout_output_at(",
            "layout == output->wlr_output",
            "container_autoscroll_target_lifetime_epoch(",
            "output_autoscroll_lifetime_epoch(",
            "wlr_seat_get_keyboard(",
            "xkb_keymap_led_get_index(",
            "xkb_state_led_index_is_active(",
            "G3D_LOCK_UNKNOWN",
            "XKB_LED_NAME_SCROLL",
            "layout_generation=UNVERIFIED",
            "physical_origin=UNKNOWN axis_frames=0",
            "lock_authority=DIAGNOSTIC_ONLY",
        ):
            self.assertIn(token,PROBE)
        for forbidden in (
            "pa_autoscroll_continuity_begin(",
            "pa_autoscroll_guard_bridge_begin(",
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "pa_scroll_dispatch_enable(",
            "wl_event_loop_add_timer(",
        ):
            self.assertNotIn(forbidden,PROBE)

    def test_virtual_key_xkb_state_and_test_matrix(self):
        for token in (
            "zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(",
            "zwp_virtual_keyboard_v1_keymap(",
            "zwp_virtual_keyboard_v1_key(",
            "zwp_virtual_keyboard_v1_modifiers(",
            "xkb_state_update_key(",
            "KEY_SCROLLLOCK",
            "XKB_LED_NAME_SCROLL",
            "modifier_map Mod3 { <SCLK> };",
            "PA_G3D_CI_SCLK_TEST_KEYMAP_AMBIGUOUS",
            "xkb_keymap_new_from_string(",
            "PA_G3D_VIRTUAL_KEYBOARD_READY",
            "no_pointer_events=1",
        ):
            self.assertIn(token,CLIENT)
        for forbidden in (
            "zwlr_virtual_pointer_v1_button(",
            "zwlr_virtual_pointer_v1_axis(",
            "zwlr_virtual_pointer_v1_motion(",
        ):
            self.assertNotIn(forbidden,CLIENT)
        for token in (
            "PA_G3D_REAL_NATIVE_NO_KEYBOARD_SCROLL_LOCK_UNKNOWN_FAIL_CLOSED_PASS",
            "PA_G3D_REAL_NATIVE_DIRECT_XDG_AND_LAYOUT_HIT_PASS",
            "PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_OFF_OBSERVED_PASS",
            "PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_ON_DENIAL_WITNESS_PASS",
            "PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_OFF_AFTER_TOGGLE_PASS",
            "PA_G3D_REAL_NATIVE_SCENE_MISS_DENIED_PASS",
            "PA_G3D_REAL_NATIVE_XDG_UNMAP_SCENE_MISS_PASS",
            "PA_G3D_REAL_NATIVE_KEYBOARD_REMOVE_TO_UNKNOWN_PASS",
            "PA_G3D_REAL_SCENE_AND_XKB_LED_NEGATIVE_EVIDENCE_PASS",
            "WLR_LIBINPUT_NO_DEVICES",
            "PA_G3D_NATIVE_SCENE_AUDIT",
        ):
            self.assertIn(token,TRACE)
        for token in (
            "BLOCKED_G3D_UDMABUF_NOT_DISPOSABLE_CI",
            "setfacl --restore=",
            "original_acl=RESTORED",
            "PA_G3D_VK_CLIENT",
        ):
            self.assertIn(token,WRAPPER)

if __name__=="__main__":
    unittest.main()
