"""Adversarial proof that output PRESENT != selected XDG final frame."""
import unittest
from ci_assert_real_output_present import validate_output_present
from test_ci_real_scene_geometry import fixture

def present(g):
    return (
        f"TIMEW_CI_OUTPUT_PRESENT lifetime={g} sequence={g} "
        "output_presented=1 real_callback=1 same_output_context=1 "
        "selected_xdg_frame=UNVERIFIED final_view_frame=UNVERIFIED "
        "source=UNENROLLED ack=UNVERIFIED c5_spent=NONE "
        "receipt=NONE move=OFF retry=OFF authority=NONE"
    )

def seal(g):
    return (
        f"TIMEW_CI_OUTPUT_PRESENT_SEAL lifetime={g} "
        "observed_output_presentations=1 real_unmap=1 "
        "selected_xdg_frame=UNVERIFIED authority=NONE"
    )

def sample(mode="normal"):
    lines = fixture(mode)
    for g in (1, 2):
        snapshot = next(i for i, s in enumerate(lines)
                        if f"NATIVE_SCENE_SNAPSHOT lifetime={g}" in s)
        lines.insert(snapshot + 1, present(g))
        outcome_seal = next(i for i, s in enumerate(lines)
                            if f"OUTCOME_SEAL lifetime={g}" in s)
        lines.insert(outcome_seal, seal(g))
    return lines

class OutputPresentDenialTests(unittest.TestCase):
    def refuses(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate_output_present(lines, mode)

    def test_normal_output_present_still_no_frame(self):
        result = validate_output_present(sample(), "normal")
        self.assertEqual(result["output_presented_observations"], 2)
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["selected_xdg_frame_verified"])
        self.assertFalse(result["settled"])

    def test_forced_timeout_present_still_hold(self):
        result = validate_output_present(sample("forced_timeout"), "forced_timeout")
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_absent_present_denied(self):
        self.refuses([s for s in sample() if "OUTPUT_PRESENT lifetime=2" not in s])

    def test_duplicate_present_denied(self):
        lines = sample()
        i = next(i for i,s in enumerate(lines) if "OUTPUT_PRESENT lifetime=1" in s)
        lines.insert(i, lines[i])
        self.refuses(lines)

    def test_present_before_scene_snapshot_denied(self):
        lines = sample()
        i = next(i for i,s in enumerate(lines) if "OUTPUT_PRESENT lifetime=1" in s)
        lines[i], lines[i-1] = lines[i-1], lines[i]
        self.refuses(lines)

    def test_present_after_seal_denied(self):
        lines = sample()
        i = next(i for i,s in enumerate(lines) if "OUTPUT_PRESENT lifetime=1" in s)
        item = lines.pop(i)
        j = next(i for i,s in enumerate(lines) if "OUTPUT_PRESENT_SEAL lifetime=1" in s)
        lines.insert(j+1, item)
        self.refuses(lines)

    def test_incorrect_seal_count_denied(self):
        lines = [s.replace("observed_output_presentations=1",
                           "observed_output_presentations=9") for s in sample()]
        self.refuses(lines)

    def test_fake_selected_xdg_frame_denied(self):
        lines = [s.replace("selected_xdg_frame=UNVERIFIED",
                           "selected_xdg_frame=VERIFIED") for s in sample()]
        self.refuses(lines)

    def test_fake_final_view_frame_denied(self):
        lines = [s.replace("final_view_frame=UNVERIFIED",
                           "final_view_frame=VERIFIED") for s in sample()]
        self.refuses(lines)

    def test_fake_c5_spent_denied(self):
        lines = [s.replace("c5_spent=NONE", "c5_spent=SPENT") for s in sample()]
        self.refuses(lines)

    def test_fake_source_denied(self):
        lines = [s.replace("source=UNENROLLED", "source=ENROLLED") for s in sample()]
        self.refuses(lines)

    def test_fake_ack_denied(self):
        lines = [s.replace("ack=UNVERIFIED", "ack=VERIFIED") for s in sample()]
        self.refuses(lines)

    def test_fake_protected_receipt_denied(self):
        lines = [s.replace("receipt=NONE", "receipt=SIGNED") for s in sample()]
        self.refuses(lines)

    def test_fake_move_denied(self):
        lines = [s.replace("move=OFF", "move=ON") for s in sample()]
        self.refuses(lines)

    def test_orphan_lifetime_denied(self):
        lines = sample()
        lines.append(present(9))
        self.refuses(lines)

    def test_missing_seal_denied(self):
        self.refuses([s for s in sample() if "OUTPUT_PRESENT_SEAL lifetime=2" not in s])

    def test_no_scene_snapshot_denied(self):
        self.refuses([s for s in sample() if "NATIVE_SCENE_SNAPSHOT lifetime=1" not in s])

    def test_lifetime_replay_denied(self):
        lines = sample()
        lines = [s.replace("OUTPUT_PRESENT lifetime=2", "OUTPUT_PRESENT lifetime=1")
                 for s in lines]
        self.refuses(lines)

    def test_wrong_native_outcome_mode_denied(self):
        self.refuses(sample("forced_timeout"), "normal")

    def test_forged_present_status_denied(self):
        lines = [s.replace("output_presented=1", "output_presented=0") for s in sample()]
        self.refuses(lines)

if __name__ == "__main__":
    unittest.main(verbosity=2)
