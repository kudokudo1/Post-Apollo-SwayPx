"""Hostile tests: actual view-buffer iterator is not a presented final frame."""
import unittest

from ci_assert_real_view_buffer_scan import validate_view_buffer_scan
from test_ci_real_output_present import sample as parent_sample

def scan(g):
    return (
        f"TIMEW_CI_VIEW_BUFFER_SCAN lifetime={g} sequence={g} "
        "ancestor_view=REAL_XDG scene_surface=1 primary_output=1 "
        "frame_done_iterator=1 "
        "contributed_to_present=UNVERIFIED rendered=UNVERIFIED "
        "selected_final_frame=UNVERIFIED "
        "client_ack=UNVERIFIED c5_spent=NONE source=UNENROLLED "
        "receipt=NONE move=OFF retry=OFF authority=NONE"
    )

def seal(g):
    return (
        f"TIMEW_CI_VIEW_BUFFER_SCAN_SEAL lifetime={g} "
        "native_scans=1 real_unmap=1 "
        "selected_final_frame=UNVERIFIED authority=NONE"
    )

def sample(mode="normal"):
    lines = parent_sample(mode)
    for g in (1, 2):
        snapshot = next(i for i, text in enumerate(lines)
                        if f"NATIVE_SCENE_SNAPSHOT lifetime={g}" in text)
        lines.insert(snapshot + 1, scan(g))
        end = next(i for i, text in enumerate(lines)
                   if f"OUTPUT_PRESENT_SEAL lifetime={g}" in text)
        lines.insert(end, seal(g))
    return lines

class ViewBufferTraversalTests(unittest.TestCase):
    def denies(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate_view_buffer_scan(lines, mode)

    def test_normal_ancestor_not_final_presented_frame(self):
        result = validate_view_buffer_scan(sample(), "normal")
        self.assertEqual(result["matched_view_buffer_scans"], 2)
        self.assertFalse(result["view_buffer_presented"])
        self.assertFalse(result["selected_final_frame"])
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_forced_timer_and_output_present_still_no_settlement(self):
        result = validate_view_buffer_scan(sample("forced_timeout"), "forced_timeout")
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_missing_first_view_scan(self):
        self.denies([x for x in sample() if "VIEW_BUFFER_SCAN lifetime=1" not in x])

    def test_missing_second_view_scan(self):
        self.denies([x for x in sample() if "VIEW_BUFFER_SCAN lifetime=2" not in x])

    def test_duplicate_same_native_scene_scan(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "VIEW_BUFFER_SCAN lifetime=1" in x)
        lines.insert(i+1, lines[i])
        self.denies(lines)

    def test_scan_before_scene_snapshot(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "VIEW_BUFFER_SCAN lifetime=1" in x)
        lines[i-1], lines[i] = lines[i], lines[i-1]
        self.denies(lines)

    def test_scan_after_lifetime_seal(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "VIEW_BUFFER_SCAN lifetime=1" in x)
        item = lines.pop(i)
        j = next(i for i,x in enumerate(lines) if "VIEW_BUFFER_SCAN_SEAL lifetime=1" in x)
        lines.insert(j+1, item)
        self.denies(lines)

    def test_wrong_lifetime_replay(self):
        lines = [x.replace("VIEW_BUFFER_SCAN lifetime=2",
                           "VIEW_BUFFER_SCAN lifetime=1")
                 for x in sample()]
        self.denies(lines)

    def test_orphan_instruction_sequence(self):
        lines = [x.replace("VIEW_BUFFER_SCAN lifetime=1 sequence=1",
                           "VIEW_BUFFER_SCAN lifetime=1 sequence=999")
                 for x in sample()]
        self.denies(lines)

    def test_forged_xdg_ancestor(self):
        lines = [x.replace("ancestor_view=REAL_XDG", "ancestor_view=UNVERIFIED")
                 for x in sample()]
        self.denies(lines)

    def test_forged_primary_output(self):
        lines = [x.replace("primary_output=1", "primary_output=0")
                 for x in sample()]
        self.denies(lines)

    def test_forged_scene_surface(self):
        lines = [x.replace("scene_surface=1", "scene_surface=0")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_rendering_is_not_authority(self):
        lines = [x.replace("rendered=UNVERIFIED", "rendered=1")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_contribution_is_not_proven(self):
        lines = [x.replace("contributed_to_present=UNVERIFIED",
                           "contributed_to_present=VERIFIED")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_final_selected_frame(self):
        lines = [x.replace("selected_final_frame=UNVERIFIED",
                           "selected_final_frame=VERIFIED")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_protected_ack(self):
        lines = [x.replace("client_ack=UNVERIFIED", "client_ack=VERIFIED")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_c5_spend(self):
        lines = [x.replace("c5_spent=NONE", "c5_spent=SPENT")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_enrolled_source(self):
        lines = [x.replace("source=UNENROLLED", "source=ENROLLED")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_signed_receipt(self):
        lines = [x.replace("receipt=NONE", "receipt=SIGNED")
                 for x in sample()]
        self.denies(lines)

    def test_claimed_native_move(self):
        lines = [x.replace("move=OFF", "move=ON") for x in sample()]
        self.denies(lines)

    def test_missing_scan_seal(self):
        lines = [x for x in sample()
                 if "VIEW_BUFFER_SCAN_SEAL lifetime=2" not in x]
        self.denies(lines)

    def test_scan_seal_bad_count(self):
        lines = [x.replace("native_scans=1", "native_scans=7") for x in sample()]
        self.denies(lines)

    def test_scan_seal_after_unmap(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "VIEW_BUFFER_SCAN_SEAL lifetime=1" in x)
        item = lines.pop(i)
        j = next(i for i,x in enumerate(lines) if "REAL_XDG_UNMAPPED gen=1" in x)
        lines.insert(j+1, item)
        self.denies(lines)

    def test_native_context_drift_fails(self):
        lines = sample()
        lines.append("TIMEW_CI_VIEW_BUFFER_DRIFT same_view_diagnostic=1 "
                     "native_source_or_output_drift=1 authority=NONE")
        self.denies(lines)

    def test_wrong_native_mode_fails(self):
        self.denies(sample("forced_timeout"), "normal")

if __name__ == "__main__":
    unittest.main(verbosity=2)
