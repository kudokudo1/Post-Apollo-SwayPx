#!/usr/bin/env python3
"""Adversarial offline parser tests, no compositor privilege or signed receipt."""
import unittest
from ci_assert_real_instruction_outcomes import validate

def sample(mode="normal"):
    lines = []
    for lifetime in (1, 2):
        sequence = lifetime
        lines.append(f"TIMEW_CI_LINEAGE_OPEN lifetime={lifetime} transaction={lifetime}")
        def event(name, configured=1):
            return (
                f"TIMEW_CI_OUTCOME_{name} lifetime={lifetime} "
                f"sequence={sequence} configured={configured} "
                "real_callback=1 receipt=NONE authority=NONE"
            )
        lines.append(event("COMMIT"))
        lines.append(
            f"TIMEW_CI_REAL_INSTRUCTION_BIND lifetime={lifetime} "
            f"commit_sequence={sequence} native_transaction_instruction=1"
        )
        if mode == "normal":
            lines.append(event("READY"))
        else:
            lines.append(event("TIMEOUT"))
        lines.append(event("CURRENT"))
        lines.append(event("SCENE"))
        lines.append(
            f"TIMEW_CI_OUTCOME_SEAL lifetime={lifetime} "
            "committed_instructions=1 real_unmap=1 settled=0 authority=NONE"
        )
        lines.append(
            f"TIMEW_CI_LINEAGE_REVOKED lifetime={lifetime} transaction={lifetime}"
        )
        lines.append(f"TIMEW_CI_REAL_XDG_UNMAPPED gen={lifetime}")
    return lines

class DenyOnlyNativeOutcomeTests(unittest.TestCase):
    def refuses(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate(lines, mode)

    def test_normal_never_authorizes(self):
        r = validate(sample(), "normal")
        self.assertEqual(r["qualified"], 2)
        self.assertEqual(r["safe_actions"], [])
        self.assertFalse(r["settled"])

    def test_timeout_never_authorizes(self):
        r = validate(sample("forced_timeout"), "forced_timeout")
        self.assertEqual(r["qualified"], 2)
        self.assertEqual(r["safe_actions"], [])
        self.assertFalse(r["settled"])

    def test_early_unbound_ready_is_diagnostic_only(self):
        x = sample()
        x.insert(1, "TIMEW_CI_OUTCOME_ORPHAN_READY unbound=1 "
                 "real_callback=1 trusted=0 authority=NONE")
        result = validate(x, "normal")
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_forged_unbound_ready_is_rejected(self):
        x = sample()
        x.insert(1, "TIMEW_CI_OUTCOME_ORPHAN_READY unbound=1 "
                 "real_callback=1 trusted=1 authority=TRUE")
        self.refuses(x)

    def test_normal_missing_ready(self):
        self.refuses([x for x in sample() if not x.startswith(
            "TIMEW_CI_OUTCOME_READY lifetime=1")])

    def test_timeout_not_ready(self):
        x = [s.replace("OUTCOME_TIMEOUT", "OUTCOME_READY")
             for s in sample("forced_timeout")]
        self.refuses(x, "forced_timeout")

    def test_timeout_cannot_be_promoted_to_normal(self):
        self.refuses(sample("forced_timeout"), "normal")

    def test_duplicate_current(self):
        x = sample()
        row = next(y for y in x if "OUTCOME_CURRENT lifetime=1" in y)
        x.insert(x.index(row) + 1, row)
        self.refuses(x)

    def test_scene_before_current(self):
        x = sample()
        i = next(i for i, s in enumerate(x) if "OUTCOME_CURRENT lifetime=1" in s)
        x[i], x[i+1] = x[i+1], x[i]
        self.refuses(x)

    def test_forged_positive_authority(self):
        x = sample()
        x[1] = x[1].replace("authority=NONE", "authority=TRUE")
        self.refuses(x)

    def test_forged_settled_receipt(self):
        x = sample()
        x[6] = x[6].replace("settled=0", "settled=1")
        self.refuses(x)

    def test_missing_commit(self):
        self.refuses([x for x in sample() if not
                      x.startswith("TIMEW_CI_OUTCOME_COMMIT lifetime=1")])

    def test_missing_real_parent_instruction(self):
        self.refuses([x for x in sample() if not
                      x.startswith("TIMEW_CI_REAL_INSTRUCTION_BIND lifetime=1")])

    def test_configured_fork(self):
        x = sample()
        i = next(i for i, s in enumerate(x) if "OUTCOME_CURRENT lifetime=1" in s)
        x[i] = x[i].replace("configured=1", "configured=0")
        self.refuses(x)

    def test_missing_unmap(self):
        self.refuses([x for x in sample() if "UNMAPPED gen=2" not in x])

    def test_duplicate_open(self):
        x = sample()
        x.insert(1, x[0])
        self.refuses(x)

    def test_old_lifetime_fork(self):
        x = sample()
        i = next(i for i, s in enumerate(x) if "LINEAGE_REVOKED lifetime=2" in s)
        x[i] = x[i].replace("transaction=2", "transaction=1")
        self.refuses(x)

    def test_stage_after_seal(self):
        x = sample()
        i = next(i for i, s in enumerate(x) if "OUTCOME_SCENE lifetime=1" in s)
        row = x.pop(i)
        j = next(i for i, s in enumerate(x) if "OUTCOME_SEAL lifetime=1" in s)
        x.insert(j+1, row)
        self.refuses(x)

    def test_duplicate_reused_global_commit_sequence(self):
        x = sample()
        x = [s.replace("sequence=2", "sequence=1").replace(
            "commit_sequence=2", "commit_sequence=1") for s in x]
        self.refuses(x)

    def test_bad_mode(self):
        self.refuses(sample(), "production")

    def test_missing_seal(self):
        self.refuses([x for x in sample() if "OUTCOME_SEAL lifetime=1" not in x])

    def test_upstream_failure(self):
        x = sample()
        x.append("TIMEW_CI_REAL_XDG_FAIL reason=stale_root")
        self.refuses(x)

if __name__ == "__main__":
    unittest.main(verbosity=2)
