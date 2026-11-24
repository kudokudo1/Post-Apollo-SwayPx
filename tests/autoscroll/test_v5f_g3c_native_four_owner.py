"""G3C four REAL native-owner epochs with virtual physical eligibility zero."""
from pathlib import Path
import importlib.util
import sys
import unittest

ROOT=Path(__file__).resolve().parents[2]
TESTS=ROOT/"tests/autoscroll"
sys.path.insert(0,str(TESTS))

def load(name, filename):
    spec=importlib.util.spec_from_file_location(name, TESTS/filename)
    mod=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

C1=load("g3c_c1","ci_v5f_c1_native_overlay.py")
C2=load("g3c_c2","ci_v5f_c2_native_overlay.py")
G3C=load("g3c_overlay","ci_v5f_g3c_native_overlay.py")
INPUT=(ROOT/"sway/input/input-manager.c").read_text()
SEAT=(ROOT/"sway/input/seat.c").read_text()
TARGET=(ROOT/"sway/tree/container.c").read_text()
OUTPUT=(ROOT/"sway/tree/output.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
PROBE=(TESTS/"ci_v5f_g3c_native_probe.c").read_text()
CLIENT=(TESTS/"ci_v5f_g3c_protocol_client.c").read_text()
TRACE=(TESTS/"ci_v5f_g3c_native_trace.py").read_text()
WRAP=(TESTS/"ci_v5f_g3c_udmabuf.sh").read_text()

class FourNativeOwnerNegative(unittest.TestCase):
    def test_only_disposable_overlay_changes_real_owner_files(self):
        for src in (INPUT,SEAT,TARGET,OUTPUT,MESON,RUNTIME,BRIDGE):
            self.assertNotIn("pa_g3c_",src)
            self.assertNotIn("ci_v5f_g3c_native_probe.c",src)
        self.assertNotIn("post_apollo_autoscroll_continuity.c",MESON)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",BRIDGE)
        self.assertIn("'/../tests/autoscroll/ci_v5f_g3c_native_probe.c'".replace("'/", "'"),G3C.meson_patch(MESON))
        self.assertIn("post_apollo_autoscroll_continuity.c",G3C.meson_patch(MESON))

    def test_all_four_native_owner_patches_compile_boundaries(self):
        inp=G3C.device_patch(C2.device_patch(INPUT))
        seat=G3C.seat_patch(C2.seat_patch(SEAT))
        target=G3C.target_patch(C1.target_patch(TARGET))
        output=G3C.output_patch(C1.output_patch(OUTPUT))
        for content,symbol in (
            (inp,"pa_g3c_virtual_pointer_bound(input_device, seat);"),
            (inp,"pa_g3c_virtual_pointer_revoked(input_device);"),
            (seat,"pa_g3c_transient_seat_revoked(seat);"),
            (target,"pa_g3c_target_bound(c);"),
            (target,"pa_g3c_target_revoked(con);"),
            (output,"pa_g3c_output_changed(output, 1);"),
            (output,"pa_g3c_output_changed(output, 2);"),
        ):
            self.assertEqual(content.count(symbol),1)
        self.assertLess(inp.index("pa_autoscroll_lifetime_revoke(&input_device->autoscroll_lifetime);"),
                        inp.index("pa_g3c_virtual_pointer_revoked(input_device);"))
        self.assertLess(seat.index("pa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime);"),
                        seat.index("pa_g3c_transient_seat_revoked(seat);"))
        self.assertLess(target.index("pa_autoscroll_lifetime_revoke(&con->autoscroll_target_lifetime);"),
                        target.index("pa_g3c_target_revoked(con);"))
        self.assertLess(output.index("pa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime);",
                                     output.index("void output_disable(")),
                        output.index("pa_g3c_output_changed(output, 2);"))

    def test_real_native_epoch_source_virtual_refusal_and_no_axis(self):
        for key in ("input_manager_autoscroll_pointer_lifetime_epoch(device)",
                    "pa_autoscroll_lifetime_epoch(",
                    "seat_autoscroll_lifetime_epoch(g3c.seat)",
                    "container_autoscroll_target_lifetime_epoch(g3c.target)",
                    "output_autoscroll_lifetime_epoch(g3c.output)",
                    "PA_G3C_NATIVE_FOUR_REAL_EPOCHS",
                    "PA_G3C_NATIVE_DENY reason=%s",
                    "origin=FORGED_TEST_ONLY",
                    "scene_layout=FAKE_TEST_ONLY",
                    "virtual_eligible=0",
                    "INTENTIONAL FORGERY",
                    "pa_autoscroll_continuity_tick("):
            self.assertIn(key,PROBE)
        for forbidden in ("wlr_seat_pointer_notify_axis(",
                          "wlr_seat_pointer_notify_frame(",
                          "pa_scroll_dispatch_enable(",
                          "wl_event_loop_add_timer(",
                          "libinput_dispatch("):
            self.assertNotIn(forbidden,PROBE)

    def test_real_protocol_and_five_native_negative_scenarios(self):
        for key in (
            "ext_transient_seat_manager_v1_create(",
            "zwlr_virtual_pointer_manager_v1_create_virtual_pointer(",
            'strcmp(line, "seat\\n")',
            'strcmp(line, "device\\n")',
            "zwlr_virtual_pointer_v1_destroy(pointer)",
            "ext_transient_seat_v1_destroy(s.transient)",
            "no_input_events=1",
        ):
            self.assertIn(key,CLIENT)
        for forbidden in ("zwlr_virtual_pointer_v1_button(",
                          "zwlr_virtual_pointer_v1_motion(",
                          "zwlr_virtual_pointer_v1_axis("):
            self.assertNotIn(forbidden,CLIENT)
        for reason in ("output_reconfigure","target_unmap","device_destroy",
                       "seat_destroy","output_disable"):
            self.assertIn('"'+reason+'"',TRACE)
        for key in ("PA_G3C_REAL_FOUR_OWNER_NEGATIVE_LIFECYCLE_PASS",
                    "virtual_eligible=0",
                    "five_native_revocations=5",
                    "WLR_LIBINPUT_NO_DEVICES",
                    "PA_G3C_NATIVE_DENIAL_FAILURE",
                    "PA_G3C_NATIVE_FOUR_OWNER",
                    "PA_G3C_CLIENT"):
            self.assertIn(key,TRACE)
        self.assertIn("setfacl --restore=",WRAP)
        self.assertIn("original_acl=RESTORED",WRAP)
        self.assertIn("BLOCKED_G3C_UDMABUF_NOT_DISPOSABLE_CI",WRAP)

if __name__=="__main__":
    unittest.main()
