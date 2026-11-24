"""G3E actual compositor virtual middle event must never become physical trust."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[2]
TEST=ROOT/"tests/autoscroll"
sys.path.insert(0,str(TEST))
def load(name,file):
    spec=importlib.util.spec_from_file_location(name,TEST/file)
    obj=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(obj)
    return obj
G2=load("pa_g3e_inherited_g2","ci_v5f_g2_native_button.py")
G3E=load("pa_g3e_overlay","ci_v5f_g3e_native_overlay.py")
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
SOURCE=(ROOT/"sway/input/post_apollo_autoscroll_source.c").read_text()
PROBE=(TEST/"ci_v5f_g3e_native_probe.c").read_text()
TRACE=(TEST/"ci_v5f_g3e_native_trace.py").read_text()
WRAP=(TEST/"ci_v5f_g3e_udmabuf.sh").read_text()

class ActualNativeVirtualMiddleNegative(unittest.TestCase):
    def test_production_hard_off_and_untrusted_source(self):
        for body in (CURSOR,MESON,RUNTIME,BRIDGE,SOURCE):
            self.assertNotIn("pa_g3e_native_middle_negative_observe",body)
            self.assertNotIn("ci_v5f_g3e_native_probe.c",body)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",BRIDGE)
        self.assertIn("Kernel uinput events",SOURCE.replace("kernel uinput events","Kernel uinput events"))
        self.assertIn("pa_autoscroll_source_probe(&event->pointer->base)",CURSOR)

    def test_first_refusal_and_button_dispatch_are_preserved(self):
        patched=G3E.cursor_patch(G2.cursor_patch(CURSOR))
        handler=patched.split("static void handle_pointer_button(",1)[1].split(
            "void dispatch_cursor_axis(",1)[0]
        order=("stage_director_news_pairing_pointer_button(",
               "stage_director_company_pairing_pointer_button(",
               "PA_G2_NATIVE_PAIR",
               "pa_g3e_native_middle_negative_observe(cursor, event,",
               "if (weather_paired || company_paired)",
               "pa_autoscroll_runtime_button(",
               "dispatch_cursor_button(cursor,")
        self.assertEqual([handler.index(t) for t in order],
                         sorted(handler.index(t) for t in order))
        self.assertEqual(patched.count(
            "pa_g3e_native_middle_negative_observe(cursor, event,"),1)
        self.assertIn("'../tests/autoscroll/ci_v5f_g3e_native_probe.c'",
                      G3E.meson_patch(MESON))
        self.assertNotIn("ci_v5f_g3e_native_probe.c",MESON)
        self.assertIn("PA_G2_NATIVE_TO_SEATOP",patched)
        self.assertIn("PA_G2_NATIVE_FROM_SEATOP",patched)

    def test_native_observer_reads_not_grants_four_owner_scene_origin(self):
        for key in (
            "wlr_input_device *device = &event->pointer->base",
            "candidate->wlr_device == device",
            "native->is_virtual",
            "pa_autoscroll_lifetime_epoch(",
            "input_manager_autoscroll_pointer_lifetime_epoch(native)",
            "seat_autoscroll_lifetime_epoch(cursor->seat)",
            "container_autoscroll_target_lifetime_epoch(target)",
            "output_autoscroll_lifetime_epoch(output)",
            "wlr_output_layout_output_at(",
            "node_at_coords(",
            "target->view->surface == surface",
            "pa_autoscroll_source_probe(device)",
            "pa_autoscroll_origin_classify(&facts)",
            "pa_autoscroll_owner_snapshot_read(",
            "XKB_LED_NAME_SCROLL",
            "PA_G3E_REAL_NATIVE_MIDDLE_NEGATIVE",
            "trust=DENIED axis_frames=0",
        ):
            self.assertIn(key,PROBE)
        for prohibited in (
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
            "pa_autoscroll_continuity_begin(",
            "pa_autoscroll_guard_bridge_begin(",
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "pa_scroll_dispatch_enable(",
            "wl_event_loop_add_timer(",
            "libinput_dispatch(",
        ):
            if prohibited=="PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT":
                # Mentioning the forbidden enum in a comment is allowed.
                self.assertNotIn("= PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",PROBE)
            else:
                self.assertNotIn(prohibited,PROBE)

    def test_genuine_wayland_negative_trace_and_runner_only_acl(self):
        for key in (
            "WLR_LIBINPUT_NO_DEVICES",
            "PA_G2_PROTOCOL_READY transient=1 virtual=1",
            "seat {seat} cursor set {x} {y}",
            "PA_G2_PROTOCOL_MIDDLE_SENT pressed=1 released=1 axis=0",
            "PA_G2_NATIVE_TO_SEATOP",
            "PA_G2_NATIVE_FROM_SEATOP",
            "PA_G3E_REAL_VIRTUAL_MIDDLE_FOUR_OWNER_SCENE_DENIAL_PASS",
            "PA_G3E_REAL_VIRTUAL_MIDDLE_INGRESS_NEGATIVE_PASS",
            "snapshot==0",
            "candidate==0",
            "eligible==0",
            "virtual_eligible=0",
            "physical_HID=ABSENT",
            "normal_seatop_calls=2",
        ):
            self.assertIn(key,TRACE)
        for key in (
            "BLOCKED_G3E_UDMABUF_NOT_DISPOSABLE_CI",
            "PA_G3E_REAL_MIDDLE_NEGATIVE",
            "PA_G2_NATIVE_BUTTON",
            "PA_G3E_SWAYMSG",
            "setfacl --restore=",
            "PA_G3E_RUNNER_UDMABUF_ACL_RESTORED",
        ):
            self.assertIn(key,WRAP)
        self.assertIn('g2.overlay() # invokes C2 real transient seat/device hooks',(
            TEST/"ci_v5f_g3e_native_overlay.py").read_text())

if __name__=="__main__":
    unittest.main()
