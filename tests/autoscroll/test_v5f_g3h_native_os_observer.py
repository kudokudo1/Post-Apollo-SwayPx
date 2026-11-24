"""G3H native OS-managed real-callback diagnostic must not actuate input."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
T = ROOT / "tests/autoscroll"
OBSERVER = (T / "ci_v5f_g3h_native_os_observer.c").read_text()
OVERLAY = (T / "ci_v5f_g3h_native_overlay.py").read_text()
TRACE = (T / "ci_v5f_g3h_native_trace.py").read_text()
BASH = (T / "ci_v5f_g3h_udmabuf.sh").read_text()
MODEL = (ROOT / "MODEL/autoscroll-v5f-g3h-native-os-managed-observer.md").read_text()
RUNTIME = (ROOT / "sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE = (ROOT / "sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
MESON = (ROOT / "sway/meson.build").read_text()
CURSOR = (ROOT / "sway/input/cursor.c").read_text()

class G3HNativeOsManagedDiagnostic(unittest.TestCase):
    def test_real_event_provenance_wired_only_to_read_only_model(self):
        for needle in (
            "struct wlr_pointer_button_event *event",
            "struct wlr_input_device *device = &event->pointer->base;",
            "wl_list_for_each(it, &server.input->devices, link)",
            "native->is_virtual", "pa_autoscroll_source_probe(device)",
            "pa_autoscroll_origin_classify(&source)",
            "pa_autoscroll_owner_snapshot_read(cursor, device, &owners)",
            "input_manager_autoscroll_pointer_lifetime_epoch(native)",
            "seat_autoscroll_lifetime_epoch(cursor->seat)",
            "owners.device == device_epoch",
            "owners.seat == seat_epoch",
            "!weather_paired && !company_paired",
            "PA_G3F_LANE_OS_MANAGED, true, &facts",
            "pa_g3f_may_consume_middle(",
            "pa_g3f_may_emit_axis(",
            "production=HARD_OFF",
        ):
            self.assertIn(needle, OBSERVER)

    def test_native_observer_is_non_actuating_and_not_in_shipped_compositor(self):
        for text in (CURSOR, MESON):
            self.assertNotIn("ci_v5f_g3h", text)
            self.assertNotIn("pa_g3h_native_os_managed_diagnostic_observe",text)
        for forbidden in (
            "wlr_seat_pointer_notify_axis",
            "wlr_seat_pointer_notify_frame",
            "pa_autoscroll_guard_bridge_begin",
            "pa_scroll_dispatch_enable",
            "pa_scroll_dispatch_middle",
            "wl_event_loop_add_timer",
            "seatop_button(",
            "return true;",
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
        ):
            self.assertNotIn(forbidden,OBSERVER)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;", RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;", RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)

    def test_only_disposable_copy_adds_hook_after_stage_director(self):
        for needle in (
            "g3e.main()",
            "ci_v5f_g3e_native_probe.c",
            "ci_v5f_g3h_native_os_observer.c",
            "ci_v5f_g3f_trust_policy.c",
            "pa_g3e_native_middle_negative_observe(cursor, event",
            "pa_g3h_native_os_managed_diagnostic_observe(cursor, event",
            "PA_G3H_NATIVE_OS_DIAG",
            "g2.guard_ci()",
            "COPY_SOURCE_DRIFT",
        ):
            if needle == "COPY_SOURCE_DRIFT":
                self.assertIn("DISPOSABLE_COPY_SOURCE_DRIFT",OVERLAY)
            else:
                self.assertIn(needle,OVERLAY)
        self.assertIn("PA_G3E_REAL_MIDDLE_NEGATIVE",TRACE)
        self.assertIn("G3H_EVENT.findall(raw)",TRACE)
        self.assertIn("virtual==1 and source_candidate==0 and diag==0",TRACE)
        self.assertIn("os_mode=TEST_ONLY can_consume=",TRACE)
        self.assertIn("PA_G3H_REAL_NATIVE_OS_MANAGED_DIAGNOSTIC_REFUSED_PASS",TRACE)
        self.assertIn("PA_G2_NATIVE_FROM_SEATOP",TRACE)
        self.assertIn("PA_G3H_NATIVE_OS_DIAG",BASH)
        self.assertIn("setfacl --restore",BASH)
        self.assertIn("ci_v5f_g3h_native_trace.py",BASH)

    def test_document_accepted_development_decision_not_authorization(self):
        for phrase in (
            "OS-managed first",
            "enrolled-device optional",
            "HARD OFF",
            "not a user-approved runtime toggle",
            "G3E",
            "G3F",
            "G3G",
            "normal seatop",
            "Nurse 2",
            "Company Map",
        ):
            self.assertIn(phrase.lower(),MODEL.lower())

if __name__ == "__main__":
    unittest.main()
