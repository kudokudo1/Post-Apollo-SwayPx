"""G3K CI-only actual native XDG target-unmap and guard refusal invariants."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
T=ROOT/"tests/autoscroll"
N=(T/"ci_v5f_g3k_native_guard_negative.c").read_text()
A=(T/"ci_v5f_g3k_two_phase_kernel_actor.c").read_text()
O=(T/"ci_v5f_g3k_native_overlay.py").read_text()
P=(T/"ci_v5f_g3k_native_trace.py").read_text()
S=(T/"ci_v5f_g3k_udmabuf.sh").read_text()
M=(ROOT/"MODEL/autoscroll-v5f-g3k-native-owner-unmap-negative.md").read_text()
C=(ROOT/"sway/input/cursor.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
RT=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()

class G3KNativeOwnerRevocation(unittest.TestCase):
    def test_real_compositor_owner_and_actual_native_bridge(self):
        for token in (
            "struct wlr_pointer_button_event *event",
            "wlr_input_device_is_libinput(device)",
            "wlr_libinput_get_device_handle(device)",
            "libinput_device_get_name(handle)",
            "pa_autoscroll_owner_snapshot_read(",
            "pa_autoscroll_source_probe(device)",
            "pa_autoscroll_origin_classify(&source)",
            "pa_autoscroll_guard_bridge_begin(",
            "pa_autoscroll_guard_bridge_tick(",
            "pa_autoscroll_guard_bridge_cancel(&guard)",
            "guard.epoch.active",
            "layout_witness=UNKNOWN_DENY scroll_lock=UNKNOWN_DENY",
            "physical_origin=UNKNOWN_DENY can_consume=0 can_axis=0",
        ):
            self.assertIn(token,N)
        for forbidden in (
            "wlr_seat_pointer_notify_axis",
            "wlr_seat_pointer_notify_frame",
            "pa_autoscroll_runtime_button(",
            "pa_scroll_dispatch_enable(",
            "return true;",
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
        ):
            self.assertNotIn(forbidden,N)

    def test_real_kernel_two_clicks_then_device_destruct(self):
        for token in (
            "GITHUB_ACTIONS", "RUNNER_ENVIRONMENT", "github-hosted",
            "PA_G3K_KERNEL_ACTOR", "geteuid()==0",
            "UI_DEV_CREATE", "UI_DEV_DESTROY",
            "EV_KEY", "BTN_MIDDLE", "EV_SYN", "SYN_REPORT",
            "round <= 2", "strcmp(command,\"click\\n\")",
            "strcmp(command,\"stop\\n\")",
            "PA_G3K_KERNEL_UINPUT_DESTROYED_PASS",
        ):
            self.assertIn(token,A)
        self.assertIn('"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL"',A)
        self.assertNotIn("send_event(fd,EV_REL",A)
        self.assertNotIn("send_event(fd,EV_ABS",A)

    def test_real_mapped_target_removed_before_second_native_pair(self):
        for token in (
            "g3j.main()",
            "pa_g3k_native_guard_negative_observe(cursor, event)",
            "ci_v5f_g3k_native_guard_negative.c",
            "PA_G3K_CHECKED_IN_SOURCE_CHANGED",
        ):
            self.assertIn(token,O)
        for token in (
            "c1.stop(window)",
            "window=None",
            "c1.run_ipc(swaymsg,ipc,env,\"-t\",\"get_tree\")",
            "PA_G3K_NATIVE_XDG_TARGET_UNMAPPED_PASS",
            "PA_G3K_NATIVE_OWNER_TARGET_REVOKED_GUARD_DENIED_PASS",
            "PA_G3K_NATIVE_LIBINPUT_DEVICE_LIFETIME_REVOKED_PASS",
            "get_inputs",
            "normal_middle_dispatch=4 axis_frames=0",
            "guard_begin_allowed=0",
            "layout_generation=UNKNOWN_DENY",
            "G3K.findall(raw)",
        ):
            self.assertIn(token,P)
        self.assertIn("ci_v5f_g3k_native_trace.py",S)
        self.assertIn("setfacl --restore",S)

    def test_unchanged_production_hard_off_and_scoped_model(self):
        self.assertNotIn("pa_g3k_native_guard_negative_observe",C)
        self.assertNotIn("ci_v5f_g3k_native_guard_negative.c",MESON)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RT)
        self.assertIn("const bool physical_verified = false;",RT)
        self.assertIn("const bool native_scroll_lock_active = true;",RT)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        for token in ("OS-managed","Nurse 2","HARD OFF",
                      "target unmap","device removal",
                      "UNKNOWN/DENY","Company Map","TimeW",
                      "not Scroll Lock attestation"):
            self.assertIn(token.lower(),M.lower())

if __name__=="__main__":
    unittest.main()
