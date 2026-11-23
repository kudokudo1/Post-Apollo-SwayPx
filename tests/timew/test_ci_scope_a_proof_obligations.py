"""Adversarial acceptance tests for SwayPX's *negative* proof inventory.

The input consists entirely of synthetic/native diagnostic log TEXT,
never source-authenticated receipt bytes. Every result must remain HOLD.
"""
import unittest

from ci_scope_a_proof_obligations import (
    GAPS, DENIED, inspect_native_obligations,
)
from test_ci_outcome_checker import sample


def assert_held(case, result):
    case.assertTrue(result["status"].endswith("_HOLD"))
    case.assertEqual(result["safe_actions"], [])
    case.assertEqual(result["needs"], list(GAPS))
    case.assertIsNone(result["protected_receipt"])
    case.assertFalse(result["resize_to_move_continuation_allowed"])
    for k in DENIED:
        case.assertIs(result[k], False, k)


class ScopeAProofGapTests(unittest.TestCase):
    def test_clean_ready_is_still_untrusted(self):
        out = inspect_native_obligations(lines=sample(), mode="normal")
        self.assertEqual(out["status"], "MATCHED_NATIVE_DIAGNOSTIC_UNTRUSTED_HOLD")
        self.assertEqual(out["qualified_diagnostic_count"], 2)
        assert_held(self, out)

    def test_clean_timeout_is_never_authentic_settlement(self):
        out = inspect_native_obligations(lines=sample("forced_timeout"), mode="forced_timeout")
        self.assertEqual(out["status"], "MATCHED_NATIVE_DIAGNOSTIC_UNTRUSTED_HOLD")
        assert_held(self, out)

    def test_ready_timeout_conflict_is_explicit_hold(self):
        lines = sample("forced_timeout")
        for generation in (1, 2):
            i = next(i for i,s in enumerate(lines)
                     if f"OUTCOME_TIMEOUT lifetime={generation}" in s)
            lines.insert(i, lines[i].replace("OUTCOME_TIMEOUT", "OUTCOME_READY"))
        out = inspect_native_obligations(lines=lines, mode="forced_timeout")
        self.assertEqual(out["status"], "CONTRADICTORY_NATIVE_READY_TIMEOUT_HOLD")
        self.assertEqual(out["qualified_diagnostic_count"], 0)
        self.assertEqual(out["ambiguous_ready_timeout_count"], 2)
        assert_held(self, out)

    def test_forged_late_bind_is_not_authenticated(self):
        lines = sample()
        i = next(i for i,s in enumerate(lines)
                 if "REAL_INSTRUCTION_BIND lifetime=1" in s)
        bind = lines.pop(i)
        j = next(i for i,s in enumerate(lines) if "OUTCOME_SCENE lifetime=1" in s)
        lines.insert(j+1, bind)
        out = inspect_native_obligations(lines=lines, mode="normal")
        self.assertEqual(out["status"], "MISSING_OR_INVALID_NATIVE_EVIDENCE_HOLD")
        assert_held(self, out)

    def test_resize_diagnostics_cannot_unlock_followon_move(self):
        lines = sample()
        for fake in (None, True, {"trusted": True, "spent": True},
                     {"signed_receipt": "fake"}, [], "COMPLETE_VERIFIED"):
            with self.subTest(fake=fake):
                out = inspect_native_obligations(
                    lines=lines, mode="normal", step="move",
                    claimed_prior_resize_receipt=fake)
                self.assertEqual(out["status"],
                                 "MOVE_BLOCKED_WITHOUT_PROTECTED_RESIZE_RECEIPT_HOLD")
                assert_held(self, out)

    def test_timeout_diagnostics_cannot_unlock_followon_move(self):
        out = inspect_native_obligations(
            lines=sample("forced_timeout"), mode="forced_timeout",
            step="move", claimed_prior_resize_receipt={"ok": True})
        assert_held(self, out)

    def test_orphan_unbound_ready_only_adds_diagnostics(self):
        lines = sample()
        lines.insert(1, "TIMEW_CI_OUTCOME_ORPHAN_READY unbound=1 "
                        "real_callback=1 trusted=0 authority=NONE")
        out = inspect_native_obligations(lines=lines, mode="normal")
        self.assertEqual(out["status"], "MATCHED_NATIVE_DIAGNOSTIC_UNTRUSTED_HOLD")
        assert_held(self, out)

    def test_local_spent_or_geometry_fields_are_not_authority(self):
        lines = sample()
        lines.append("TIMEW_LOCAL_SQLITE_SPENT status=SPENT exact_y=92.5 "
                     "claimed_source=ENROLLED")
        out = inspect_native_obligations(lines=lines, mode="normal")
        assert_held(self, out)

    def test_legacy_missing_data_is_hold(self):
        for lines in ([], ["totally fabricated"], sample()[:-1]):
            with self.subTest(lines=len(lines)):
                out = inspect_native_obligations(lines=lines, mode="normal")
                self.assertEqual(out["status"], "MISSING_OR_INVALID_NATIVE_EVIDENCE_HOLD")
                assert_held(self, out)

    def test_malformed_step_fails_closed(self):
        for bad in ("restore", "undo", "retry", "", None, 5, False):
            with self.subTest(bad=bad):
                out = inspect_native_obligations(lines=sample(), mode="normal", step=bad)
                self.assertEqual(out["status"], "INVALID_DIAGNOSTIC_REQUEST_HOLD")
                assert_held(self, out)

    def test_malformed_native_line_types_fail_closed(self):
        for bad in (None, (), "text", 3, [None], [1], True):
            with self.subTest(bad=bad):
                out = inspect_native_obligations(lines=bad, mode="normal")
                self.assertEqual(out["status"], "INVALID_DIAGNOSTIC_REQUEST_HOLD")
                assert_held(self, out)

    def test_wrong_mode_cannot_relabel_native_timeout(self):
        out = inspect_native_obligations(
            lines=sample("forced_timeout"), mode="normal")
        self.assertEqual(out["status"], "MISSING_OR_INVALID_NATIVE_EVIDENCE_HOLD")
        assert_held(self, out)


if __name__ == "__main__":
    unittest.main(verbosity=2)
