"""G3N: test-only event epochs never confer production scrolling authority."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
C=(ROOT/"tests/autoscroll/ci_v5f_g3n_keyboard_layout_aba_oracle.c").read_text()
RUNTIME=(ROOT/"sway/input/post_apollo_autoscroll_runtime.c").read_text()
BRIDGE=(ROOT/"sway/input/post_apollo_autoscroll_guard_bridge.c").read_text()
CURSOR=(ROOT/"sway/input/cursor.c").read_text()
MESON=(ROOT/"sway/meson.build").read_text()
MODEL=(ROOT/"MODEL/autoscroll-v5f-g3n-keyboard-layout-aba-negative.md").read_text()

class G3NUntrustedEpochModel(unittest.TestCase):
    def test_original_native_deny_guards_untouched(self):
        self.assertIn("#define PA_AUTOSCROLL_LIVE_ACTIVATION 0",RUNTIME)
        self.assertIn("const bool physical_verified = false;",RUNTIME)
        self.assertIn("const bool native_scroll_lock_active = true;",RUNTIME)
        self.assertIn(".origin = PA_POINTER_ORIGIN_UNKNOWN",BRIDGE)
        self.assertIn(".layout_generation_verified = false",BRIDGE)
        self.assertNotIn("ci_v5f_g3n_keyboard_layout_aba_oracle.c",MESON)
        self.assertNotIn("g3n_test_session",CURSOR)

    def test_untrusted_local_epochs_are_not_authentication(self):
        for word in (
            "CI-ONLY","TEST-SIMULATED",
            "native_issuer=ABSENT","native_authorization=NONE",
            "static bool may_consume_middle_or_axis(",
            "return false; /* HARD OFF.",
            "same_untrusted_snapshot",
            "hypothetical_test_session",
            "test_only_revalidate",
            "UINT64_MAX",
            "now->poisoned=true",
            "now->owner_present=false",
        ):
            self.assertIn(word,C)
        for forbidden in (
            "wlr_seat_pointer_notify_axis(",
            "wlr_seat_pointer_notify_frame(",
            "pa_scroll_dispatch_enable(",
            "PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT",
            "static bool may_consume_middle_or_axis(\n"
            "        const struct g3n_observation *now,\n"
            "        const struct g3n_test_session *s) {\n"
            "    (void)now;(void)s;\n    return true;",
        ):
            self.assertNotIn(forbidden,C)

    def test_aba_group_hotplug_owner_and_overflow_cases_exist(self):
        for token in (
            "KEYBOARD_HOTPLUG_ABA_4_5_4",
            "OUTPUT_POSITION_ABA_0_137_0",
            "SAME_COUNT_DIFFERENT_KEYBOARD_MEMBERSHIP",
            "SAME_COUNT_GROUP_CHANGED",
            "SEAT_CHANGE",
            "TARGET_UNMAP",
            "EPOCH_OVERFLOW_FAIL_CLOSED",
            "INVALID_LAYOUT_FAIL_CLOSED",
            "PA_G3N_ALL_EIGHT_ADVERSARIAL_FRESHNESS_TESTS_PASS",
        ):
            self.assertIn(token,C)
        self.assertIn("keyboard_topology_event(&now,5,3,1)",C)
        self.assertIn("keyboard_topology_event(&now,4,3,1)",C)
        self.assertIn("layout_event(&now,137,0,1920,1080)",C)
        self.assertIn("layout_event(&now,0,0,1920,1080)",C)

    def test_authority_and_owner_boundaries_are_truthful(self):
        for token in (
            "G3M","Nurse 2","UNKNOWN/DENY","Company Map",
            "HARD OFF","not authenticated","not production",
            "TimeW","protected","ABA","read-only",
        ):
            self.assertIn(token.lower(),MODEL.lower())
        self.assertNotIn("G3N authorized",MODEL)

if __name__=="__main__":
    unittest.main()
