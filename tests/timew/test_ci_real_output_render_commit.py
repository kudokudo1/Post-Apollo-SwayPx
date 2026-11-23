"""Adversarial tests: accepted output render state != presented selected XDG."""
import unittest

from ci_assert_real_output_render_commit import validate_output_render_commit
from test_ci_real_view_buffer_scan import sample as parent_sample

def accepted(g):
    return (
        f"TIMEW_CI_RENDER_COMMIT lifetime={g} sequence={g} "
        "scene_state_built=1 output_commit_accepted=1 real_callback=1 "
        "same_mapped_output=1 "
        "xdg_rendered=UNVERIFIED xdg_contributed=UNVERIFIED "
        "xdg_final_presented=UNVERIFIED "
        "client_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
        "receipt=NONE move=OFF retry=OFF authority=NONE"
    )

def seal(g):
    return (
        f"TIMEW_CI_RENDER_COMMIT_SEAL lifetime={g} "
        "accepted_output_commits=1 real_unmap=1 "
        "xdg_final_presented=UNVERIFIED authority=NONE"
    )

def sample(mode="normal"):
    lines = parent_sample(mode)
    for g in (1, 2):
        snapshot = next(i for i, line in enumerate(lines)
                        if f"NATIVE_SCENE_SNAPSHOT lifetime={g}" in line)
        lines.insert(snapshot + 1, accepted(g))
        seal_idx = next(i for i, line in enumerate(lines)
                        if f"VIEW_BUFFER_SCAN_SEAL lifetime={g}" in line)
        lines.insert(seal_idx, seal(g))
    return lines

class RenderCommitNegativeTests(unittest.TestCase):
    def refuses(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate_output_render_commit(lines, mode)

    def test_normal_render_commit_not_selected_buffer(self):
        result = validate_output_render_commit(sample(), "normal")
        self.assertEqual(result["accepted_output_state_commits"], 2)
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["selected_xdg_rendered"])
        self.assertFalse(result["xdg_contributed_to_present"])
        self.assertFalse(result["selected_xdg_final_frame"])
        self.assertFalse(result["settled"])

    def test_forced_timer_commit_and_present_still_hold(self):
        result = validate_output_render_commit(
            sample("forced_timeout"), "forced_timeout")
        self.assertFalse(result["settled"])
        self.assertEqual(result["safe_actions"], [])

    def test_missing_first_commit(self):
        self.refuses([x for x in sample() if "RENDER_COMMIT lifetime=1" not in x])

    def test_missing_second_commit(self):
        self.refuses([x for x in sample() if "RENDER_COMMIT lifetime=2" not in x])

    def test_duplicate_render_commit(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "RENDER_COMMIT lifetime=1" in x)
        lines.insert(i, lines[i])
        self.refuses(lines)

    def test_render_commit_before_native_snapshot(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "RENDER_COMMIT lifetime=1" in x)
        lines[i-1], lines[i] = lines[i], lines[i-1]
        self.refuses(lines)

    def test_render_commit_after_lifetime_seal(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "RENDER_COMMIT lifetime=1" in x)
        item = lines.pop(i)
        j = next(i for i,x in enumerate(lines) if "RENDER_COMMIT_SEAL lifetime=1" in x)
        lines.insert(j+1, item)
        self.refuses(lines)

    def test_render_commit_after_unmap(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines) if "RENDER_COMMIT lifetime=1" in x)
        item = lines.pop(i)
        j = next(i for i,x in enumerate(lines) if "REAL_XDG_UNMAPPED gen=1" in x)
        lines.insert(j+1, item)
        self.refuses(lines)

    def test_render_commit_wrong_xdg_lifetime(self):
        lines = [
            x.replace("RENDER_COMMIT lifetime=2", "RENDER_COMMIT lifetime=1")
            for x in sample()
        ]
        self.refuses(lines)

    def test_replayed_or_unknown_instruction_sequence(self):
        lines = [
            x.replace("RENDER_COMMIT lifetime=1 sequence=1",
                      "RENDER_COMMIT lifetime=1 sequence=999")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_rendered_xdg(self):
        lines = [
            x.replace("xdg_rendered=UNVERIFIED", "xdg_rendered=VERIFIED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_view_contribution(self):
        lines = [
            x.replace("xdg_contributed=UNVERIFIED", "xdg_contributed=VERIFIED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_final_presented_xdg(self):
        lines = [
            x.replace("xdg_final_presented=UNVERIFIED",
                      "xdg_final_presented=VERIFIED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_output_commit_failure(self):
        lines = [
            x.replace("output_commit_accepted=1", "output_commit_accepted=0")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_scene_build_failure(self):
        lines = [
            x.replace("scene_state_built=1", "scene_state_built=0")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_source_enrollment(self):
        lines = [
            x.replace("source=UNENROLLED", "source=ENROLLED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_client_ack(self):
        lines = [
            x.replace("client_ack=UNVERIFIED", "client_ack=VERIFIED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_c5_spent(self):
        lines = [
            x.replace("c5_spent=NONE", "c5_spent=SPENT")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_signed_receipt(self):
        lines = [
            x.replace("receipt=NONE", "receipt=SIGNED")
            for x in sample()
        ]
        self.refuses(lines)

    def test_forged_native_move(self):
        lines = [
            x.replace("move=OFF", "move=ON")
            for x in sample()
        ]
        self.refuses(lines)

    def test_missing_commit_seal(self):
        self.refuses([
            x for x in sample() if "RENDER_COMMIT_SEAL lifetime=2" not in x
        ])

    def test_commit_seal_bad_count(self):
        lines = [
            x.replace("accepted_output_commits=1", "accepted_output_commits=7")
            for x in sample()
        ]
        self.refuses(lines)

    def test_commit_seal_after_unmap(self):
        lines = sample()
        i = next(i for i,x in enumerate(lines)
                 if "RENDER_COMMIT_SEAL lifetime=1" in x)
        item = lines.pop(i)
        j = next(i for i,x in enumerate(lines)
                 if "REAL_XDG_UNMAPPED gen=1" in x)
        lines.insert(j+1, item)
        self.refuses(lines)

    def test_output_context_drift_fails_closed(self):
        lines = sample()
        lines.append(
            "TIMEW_CI_RENDER_COMMIT_DRIFT source_or_output=CHANGED "
            "native_authority=NONE"
        )
        self.refuses(lines)

    def test_wrong_native_mode_rejected(self):
        self.refuses(sample("forced_timeout"), "normal")

if __name__ == "__main__":
    unittest.main(verbosity=2)
