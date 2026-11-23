"""Adversarial SceneFX root-surface output_sample identity and timing tests."""
import unittest

from ci_assert_real_root_sample import validate_root_sample
from test_ci_real_output_commit_serial import sample as parent_sample

def bind(g):
    return (
        f"TIMEW_CI_ROOT_SAMPLE_BIND lifetime={g} "
        "exact_native_xdg_root=1 primary_output=1 "
        "receipt=NONE authority=NONE"
    )

def sample_line(g, scanout=0):
    return (
        f"TIMEW_CI_ROOT_SAMPLE lifetime={g} sequence={g} "
        f"direct_scanout={scanout} exact_root_surface=1 sampled_scene_buffer=1 "
        "real_scenefx_event=1 same_primary_output=1 "
        "render_pass_or_scanout=1 accepted_output_commit=UNVERIFIED "
        "selected_xdg_final_presented=UNVERIFIED "
        "configure_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
        "receipt=NONE move=OFF retry=OFF authority=NONE"
    )

def seal(g, n=1, subscriptions=1):
    return (
        f"TIMEW_CI_ROOT_SAMPLE_SEAL lifetime={g} "
        f"root_samples={n} subscriptions={subscriptions} "
        "real_unmap=1 settled=0 authority=NONE"
    )

def sample(mode="normal"):
    lines = parent_sample(mode)
    for g in (1, 2):
        i = next(i for i,x in enumerate(lines)
                 if f"NATIVE_SCENE_SNAPSHOT lifetime={g}" in x)
        lines.insert(i+1, bind(g))
        lines.insert(i+2, sample_line(g))
        j = next(i for i,x in enumerate(lines)
                 if f"OUTPUT_SEQ_SEAL lifetime={g}" in x)
        lines.insert(j, seal(g))
    return lines

class RealRootSampleTests(unittest.TestCase):
    def refuses(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate_root_sample(lines, mode)

    def test_actual_composited_sample_is_not_final_frame(self):
        r = validate_root_sample(sample(), "normal")
        self.assertEqual(r["native_root_samples"], 2)
        self.assertEqual(r["composited_root_samples"], 2)
        self.assertEqual(r["native_output_serial_matches"], 2)
        self.assertFalse(r["selected_xdg_final_presented"])
        self.assertFalse(r["selected_xdg_render_completed"])
        self.assertEqual(r["safe_actions"], [])

    def test_direct_scanout_sample_is_not_frame_settlement(self):
        rows = sample()
        rows = [x.replace("direct_scanout=0", "direct_scanout=1") for x in rows]
        r = validate_root_sample(rows, "normal")
        self.assertEqual(r["direct_scanout_root_samples"], 2)
        self.assertFalse(r["settled"])

    def test_forced_native_timeout_still_no_authority(self):
        r = validate_root_sample(sample("forced_timeout"), "forced_timeout")
        self.assertFalse(r["settled"])
        self.assertEqual(r["safe_actions"], [])

    def test_missing_first_sample(self):
        self.refuses([x for x in sample() if "ROOT_SAMPLE lifetime=1" not in x])

    def test_missing_second_sample(self):
        self.refuses([x for x in sample() if "ROOT_SAMPLE lifetime=2" not in x])

    def test_duplicate_sample(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows) if "ROOT_SAMPLE lifetime=1" in x)
        rows.insert(i+1, rows[i])
        self.refuses(rows)

    def test_sample_before_bind(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows) if "ROOT_SAMPLE lifetime=1" in x)
        rows[i],rows[i-1] = rows[i-1],rows[i]
        self.refuses(rows)

    def test_sample_before_real_scene_snapshot(self):
        rows = sample()
        i = next(i for i,x in enumerate(rows) if "ROOT_SAMPLE lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "NATIVE_SCENE_SNAPSHOT lifetime=1" in x)
        rows.insert(j,item)
        self.refuses(rows)

    def test_sample_after_real_output_commit_serial(self):
        rows = sample()
        i=next(i for i,x in enumerate(rows) if "ROOT_SAMPLE lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "OUTPUT_SEQ_COMMIT lifetime=1" in x)
        rows.insert(j+1,item)
        self.refuses(rows)

    def test_sample_after_lifetime_seal(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "ROOT_SAMPLE lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "ROOT_SAMPLE_SEAL lifetime=1" in x)
        rows.insert(j+1,item)
        self.refuses(rows)

    def test_duplicate_root_bind(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "ROOT_SAMPLE_BIND lifetime=1" in x)
        rows.insert(i+1, rows[i])
        self.refuses(rows)

    def test_missing_root_bind(self):
        self.refuses([x for x in sample() if "ROOT_SAMPLE_BIND lifetime=1" not in x])

    def test_forged_foreign_root(self):
        self.refuses([x.replace("exact_root_surface=1", "exact_root_surface=0")
                      for x in sample()])

    def test_forged_primary_output(self):
        self.refuses([x.replace("same_primary_output=1", "same_primary_output=0")
                      for x in sample()])

    def test_forged_non_scenefx_event(self):
        self.refuses([x.replace("real_scenefx_event=1", "real_scenefx_event=0")
                      for x in sample()])

    def test_forged_render_completion(self):
        self.refuses([x.replace("accepted_output_commit=UNVERIFIED",
                                "accepted_output_commit=VERIFIED")
                      for x in sample()])

    def test_forged_selected_final_view_frame(self):
        self.refuses([x.replace("selected_xdg_final_presented=UNVERIFIED",
                                "selected_xdg_final_presented=VERIFIED")
                      for x in sample()])

    def test_forged_client_configure_ack(self):
        self.refuses([x.replace("configure_ack=UNVERIFIED",
                                "configure_ack=ACKED")
                      for x in sample()])

    def test_forged_enrolled_source(self):
        self.refuses([x.replace("source=UNENROLLED", "source=ENROLLED")
                      for x in sample()])

    def test_forged_protected_c5_spent(self):
        self.refuses([x.replace("c5_spent=NONE", "c5_spent=SPENT")
                      for x in sample()])

    def test_forged_signed_receipt(self):
        self.refuses([x.replace("receipt=NONE", "receipt=SIGNED")
                      for x in sample()])

    def test_forged_native_move(self):
        self.refuses([x.replace("move=OFF", "move=ON") for x in sample()])

    def test_wrong_output_sampling_mode(self):
        self.refuses([x.replace("direct_scanout=0", "direct_scanout=2")
                      for x in sample()])

    def test_replayed_root_sample_lifetime(self):
        self.refuses([x.replace("ROOT_SAMPLE lifetime=2 sequence=2",
                                "ROOT_SAMPLE lifetime=1 sequence=2")
                      for x in sample()])

    def test_forged_future_instruction(self):
        self.refuses([x.replace("ROOT_SAMPLE lifetime=1 sequence=1",
                                "ROOT_SAMPLE lifetime=1 sequence=999")
                      for x in sample()])

    def test_lost_unmap_or_seal(self):
        self.refuses([x for x in sample()
                      if "ROOT_SAMPLE_SEAL lifetime=1" not in x])

    def test_root_sample_seal_counter_mismatch(self):
        self.refuses([x.replace("root_samples=1", "root_samples=9")
                      for x in sample()])

    def test_subscription_counter_mismatch(self):
        self.refuses([x.replace("subscriptions=1", "subscriptions=9")
                      for x in sample()])

    def test_root_destroyed_before_unmap_is_hold(self):
        rows=sample()
        rows.append(
            "TIMEW_CI_ROOT_SAMPLE_REVOKED lifetime=1 "
            "root_buffer_destroyed=1 no_stale_reuse=1 authority=NONE")
        self.refuses(rows)

    def test_source_context_drift_is_hold(self):
        rows=sample()
        rows.append("TIMEW_CI_ROOT_SAMPLE_DRIFT source_or_output=CHANGED "
                    "authority=NONE")
        self.refuses(rows)

    def test_unknown_or_mismatched_present_sequence_is_not_grant(self):
        rows = sample()
        rows=[x.replace("OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=101",
                        "OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=999")
              for x in rows]
        r=validate_root_sample(rows,"normal")
        self.assertEqual(r["safe_actions"],[])
        self.assertFalse(r["settled"])
        self.assertFalse(r["selected_xdg_final_presented"])

    def test_wrong_native_transaction_mode(self):
        self.refuses(sample("forced_timeout"),"normal")

if __name__ == "__main__":
    unittest.main(verbosity=2)
