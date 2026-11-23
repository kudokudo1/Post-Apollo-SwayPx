"""G3F two-tier trust reference must remain test-only, fail-closed and inert."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
TEST=ROOT/"tests/autoscroll"
H=(TEST/"ci_v5f_g3f_trust_policy.h").read_text()
C=(TEST/"ci_v5f_g3f_trust_policy.c").read_text()
HARNESS=(TEST/"ci_v5f_g3f_trust_harness.c").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
SOURCE=(ROOT/"sway/input/post_apollo_autoscroll_source.c").read_text()
ORIGIN=(ROOT/"sway/input/post_apollo_autoscroll_origin_policy.c").read_text()
OWNERS=(ROOT/"sway/input/post_apollo_autoscroll_owner_snapshot.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
MODEL=(ROOT/"MODEL/autoscroll-v5f-g3f-two-tier-trust-policy.md").read_text()

class G3FTrustModel(unittest.TestCase):
    def test_not_linked_shipped_or_enabling_production(self):
        for deployed in (RUNTIME,BRIDGE,CURSOR,SOURCE,ORIGIN,OWNERS,MESON):
            self.assertNotIn("ci_v5f_g3f_",deployed)
            self.assertNotIn("pa_g3f_diagnose_source(",deployed)
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",BRIDGE)
        self.assertNotIn("pa_scroll_dispatch_enable(",RUNTIME)
        self.assertNotIn("PA_G3F_LANE_OS_MANAGED",RUNTIME)

    def test_two_lanes_are_diagnostic_only(self):
        for token in (
            "PA_G3F_LANE_OFF = 0",
            "PA_G3F_LANE_OS_MANAGED = 1",
            "PA_G3F_LANE_ENROLLED_DEVICE = 2",
            "PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY",
            "PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED",
            "pa_g3f_may_consume_middle(",
            "pa_g3f_may_emit_axis(",
            "claimed_device_enrollment",
            "claimed_matching_serial",
            "claimed_signed_witness",
        ):
            self.assertIn(token,H)
        self.assertIn("return false;",C)
        self.assertEqual(C.count("return false;"),3)
        self.assertNotIn("return true;",C)
        self.assertNotIn("PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",C)
        for forbidden in (
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "pa_scroll_dispatch_enable(",
            "pa_autoscroll_epoch_begin(",
            "pa_autoscroll_guard_bridge_begin(",
            "getenv(",
            "fopen(",
            "libinput_dispatch(",
        ):
            self.assertNotIn(forbidden,C)

    def test_native_source_candidacy_not_equated_to_hardware(self):
        self.assertIn("kernel uinput",SOURCE.lower())
        self.assertIn("kernel_candidate",SOURCE)
        self.assertIn("PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE",ORIGIN)
        self.assertIn("candidate->is_virtual ? NULL : candidate",OWNERS)
        self.assertIn("!f->protocol_virtual",C)
        self.assertIn("!f->known_kernel_synthetic",C)
        self.assertIn("f->stage_director_first_refusal_clear",C)
        self.assertIn("f->event_device_attached_to_seat",C)
        self.assertIn("f->native_scroll_method_known",C)
        self.assertIn("!f->button_down_scroll_conflict",C)
        self.assertIn("native_compositor",MODEL.lower())
        self.assertIn("uinput",MODEL.lower())

    def test_adversarial_model_documents_operator_and_issuer_gate(self):
        for s in (
            "524288UL",
            "PA_G3F_OS_LANE_IS_ONLY_SOURCE_CANDIDATE_PASS",
            "PA_G3F_ENROLLED_LANE_REQUIRES_INDEPENDENT_TRUST_BROKER_PASS",
            "PA_G3F_OFF_UNSELECTED_INVALID_OR_MISSING_SOURCE_FAIL_CLOSED_PASS",
            "PA_G3F_VIRTUAL_AND_KNOWN_UINPUT_DENIED_UNKNOWN_SPOOF_RISK_PASS",
            "PA_G3F_FORGED_ENROLLMENT_SERIAL_OR_SIGNATURE_CANNOT_PROMOTE_PASS",
            "PA_G3F_ALL_524288_INPUT_MODE_CASES_NONACTUATING_PASS",
        ):
            self.assertIn(s,HARNESS)
        for s in (
            "PROPOSED",
            "OS-managed",
            "enrolled-device",
            "not hardware attestation",
            "no trusted issuer",
            "operator",
            "no actual scrolling",
            "HARD OFF",
        ):
            self.assertIn(s.lower(),MODEL.lower())

if __name__=="__main__":
    unittest.main()
