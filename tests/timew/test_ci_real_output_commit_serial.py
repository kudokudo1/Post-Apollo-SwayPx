"""Adversarial wlroots OUTPUT commit_seq and event-present serial binding tests."""
import unittest

from ci_assert_real_output_commit_serial import validate_output_commit_serial
from test_ci_real_output_render_commit import sample as parent_sample

def commit(g, serial=None):
    serial = serial if serial is not None else 100 + g
    return (
        f"TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime={g} sequence={g} "
        f"output_commit_seq={serial} accepted_output_state=1 real_wlroots=1 "
        "same_mapped_xdg_output=1 xdg_buffer=UNVERIFIED "
        "source=UNENROLLED c5_spent=NONE receipt=NONE "
        "move=OFF authority=NONE"
    )

def presented(g, serial=None):
    serial = serial if serial is not None else 100 + g
    return (
        f"TIMEW_CI_OUTPUT_SEQ_PRESENT lifetime={g} sequence={g} "
        f"output_commit_seq={serial} output_presented=1 real_wlroots_event=1 "
        "same_mapped_xdg_output=1 "
        "xdg_buffer=UNVERIFIED xdg_final_frame=UNVERIFIED "
        "ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
        "receipt=NONE move=OFF authority=NONE"
    )

def seal(g, commits=1, presents=1):
    return (
        f"TIMEW_CI_OUTPUT_SEQ_SEAL lifetime={g} "
        f"observed_output_commits={commits} "
        f"observed_output_presents={presents} "
        "real_unmap=1 xdg_final_frame=UNVERIFIED authority=NONE"
    )

def sample(mode="normal"):
    lines = parent_sample(mode)
    for g in (1, 2):
        i = next(i for i, x in enumerate(lines)
                 if f"RENDER_COMMIT lifetime={g}" in x)
        lines.insert(i + 1, commit(g))
        j = next(i for i, x in enumerate(lines)
                 if f"OUTPUT_PRESENT lifetime={g}" in x)
        lines.insert(j + 1, presented(g))
        k = next(i for i, x in enumerate(lines)
                 if f"RENDER_COMMIT_SEAL lifetime={g}" in x)
        lines.insert(k, seal(g))
    return lines

class OutputSerialTests(unittest.TestCase):
    def refuses(self, rows, mode="normal"):
        with self.assertRaises(ValueError):
            validate_output_commit_serial(rows, mode)

    def test_normal_actual_serial_match_is_only_output_level(self):
        data = validate_output_commit_serial(sample(), "normal")
        self.assertEqual(data["same_output_serials_proven_in_log"], 2)
        self.assertEqual(data["unbound_or_early_presentation_hold"], 0)
        self.assertFalse(data["selected_xdg_buffer_rendered"])
        self.assertFalse(data["selected_xdg_final_frame"])
        self.assertFalse(data["settled"])
        self.assertEqual(data["safe_actions"], [])

    def test_forced_timeout_match_still_no_frame_authority(self):
        data = validate_output_commit_serial(
            sample("forced_timeout"), "forced_timeout")
        self.assertEqual(data["same_output_serials_proven_in_log"], 2)
        self.assertEqual(data["safe_actions"], [])
        self.assertFalse(data["settled"])

    def test_valid_present_before_matching_commit_is_unbound_not_adopted(self):
        rows = sample()
        i = next(i for i, s in enumerate(rows)
                 if "OUTPUT_SEQ_COMMIT lifetime=1" in s)
        record = rows.pop(i)
        j = next(i for i, s in enumerate(rows)
                 if "OUTPUT_SEQ_PRESENT lifetime=1" in s)
        rows.insert(j + 1, record)
        data = validate_output_commit_serial(rows, "normal")
        self.assertEqual(data["same_output_serials_proven_in_log"], 1)
        self.assertEqual(data["unbound_or_early_presentation_hold"], 1)
        self.assertEqual(data["safe_actions"], [])

    def test_unknown_present_serial_unbound_hold(self):
        rows = [s.replace("OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=101",
                          "OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=999")
                for s in sample()]
        data = validate_output_commit_serial(rows, "normal")
        self.assertEqual(data["unbound_or_early_presentation_hold"], 1)
        self.assertFalse(data["settled"])

    def test_missing_real_commit_serial(self):
        self.refuses([x for x in sample() if "OUTPUT_SEQ_COMMIT lifetime=1" not in x])

    def test_missing_real_present_serial(self):
        self.refuses([x for x in sample() if "OUTPUT_SEQ_PRESENT lifetime=1" not in x])

    def test_duplicate_commit_line(self):
        rows = sample()
        i = next(i for i, x in enumerate(rows)
                 if "OUTPUT_SEQ_COMMIT lifetime=1" in x)
        rows.insert(i + 1, rows[i])
        self.refuses(rows)

    def test_duplicate_present_line(self):
        rows = sample()
        i = next(i for i, x in enumerate(rows)
                 if "OUTPUT_SEQ_PRESENT lifetime=1" in x)
        rows.insert(i + 1, rows[i])
        self.refuses(rows)

    def test_reused_commit_serial_across_two_xdg_lifetimes(self):
        rows = [s.replace("OUTPUT_SEQ_COMMIT lifetime=2 sequence=2 output_commit_seq=102",
                          "OUTPUT_SEQ_COMMIT lifetime=2 sequence=2 output_commit_seq=101")
                for s in sample()]
        self.refuses(rows)

    def test_replayed_present_serial_across_lifetimes(self):
        rows = [s.replace("OUTPUT_SEQ_PRESENT lifetime=2 sequence=2 output_commit_seq=102",
                          "OUTPUT_SEQ_PRESENT lifetime=2 sequence=2 output_commit_seq=101")
                for s in sample()]
        self.refuses(rows)

    def test_serial_zero_is_unknown_not_proven(self):
        rows = [s.replace("output_commit_seq=101", "output_commit_seq=0")
                for s in sample()]
        self.refuses(rows)

    def test_serial_overflow_rejected(self):
        rows = [s.replace("output_commit_seq=101", "output_commit_seq=4294967296")
                for s in sample()]
        self.refuses(rows)

    def test_negative_serial_rejected(self):
        rows = [s.replace("output_commit_seq=101", "output_commit_seq=-1")
                for s in sample()]
        self.refuses(rows)

    def test_commit_before_parent_commit_is_refused(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_COMMIT lifetime=1" in x)
        rows[i],rows[i-1] = rows[i-1],rows[i]
        self.refuses(rows)

    def test_present_before_parent_output_present_refused(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_PRESENT lifetime=1" in x)
        rows[i],rows[i-1] = rows[i-1],rows[i]
        self.refuses(rows)

    def test_commit_after_serial_seal_rejected(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_COMMIT lifetime=1" in x)
        item = rows.pop(i)
        j = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_SEAL lifetime=1" in x)
        rows.insert(j + 1, item)
        self.refuses(rows)

    def test_present_after_serial_seal_rejected(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_PRESENT lifetime=1" in x)
        item = rows.pop(i)
        j = next(i for i,x in enumerate(rows)
                 if "OUTPUT_SEQ_SEAL lifetime=1" in x)
        rows.insert(j + 1, item)
        self.refuses(rows)

    def test_commit_wrong_scene_sequence_rejected(self):
        rows = [s.replace("OUTPUT_SEQ_COMMIT lifetime=1 sequence=1",
                          "OUTPUT_SEQ_COMMIT lifetime=1 sequence=999")
                for s in sample()]
        self.refuses(rows)

    def test_present_wrong_lifetime_rejected(self):
        rows = [s.replace("OUTPUT_SEQ_PRESENT lifetime=1 sequence=1",
                          "OUTPUT_SEQ_PRESENT lifetime=9 sequence=1")
                for s in sample()]
        self.refuses(rows)

    def test_seal_missing(self):
        self.refuses([x for x in sample() if "OUTPUT_SEQ_SEAL lifetime=2" not in x])

    def test_seal_commit_count_forged(self):
        rows = [s.replace("observed_output_commits=1",
                          "observed_output_commits=5") for s in sample()]
        self.refuses(rows)

    def test_seal_present_count_forged(self):
        rows = [s.replace("observed_output_presents=1",
                          "observed_output_presents=5") for s in sample()]
        self.refuses(rows)

    def test_forged_xdg_buffer_authority(self):
        rows = [s.replace("xdg_buffer=UNVERIFIED",
                          "xdg_buffer=VERIFIED") for s in sample()]
        self.refuses(rows)

    def test_forged_enrolled_source(self):
        rows = [s.replace("source=UNENROLLED",
                          "source=ENROLLED") for s in sample()]
        self.refuses(rows)

    def test_forged_c5_and_protected_receipt(self):
        rows = [s.replace("c5_spent=NONE receipt=NONE",
                          "c5_spent=SPENT receipt=SIGNED") for s in sample()]
        self.refuses(rows)

    def test_forged_final_frame(self):
        rows = [s.replace("xdg_final_frame=UNVERIFIED",
                          "xdg_final_frame=VERIFIED") for s in sample()]
        self.refuses(rows)

    def test_forged_native_move(self):
        rows = [s.replace("move=OFF", "move=ON") for s in sample()]
        self.refuses(rows)

    def test_source_context_drift_blocks_all_correlations(self):
        rows = sample()
        rows.append("TIMEW_CI_OUTPUT_SEQ_DRIFT phase=PRESENT "
                    "candidate_or_output_changed=1 authority=NONE")
        self.refuses(rows)

    def test_wrong_native_outcome_mode(self):
        self.refuses(sample("forced_timeout"), "normal")

if __name__ == "__main__":
    unittest.main(verbosity=2)
