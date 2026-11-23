"""Hostile wlroots exact XDG root surface committed-state generation witness."""
import unittest
from ci_assert_real_surface_generation import validate_surface_generation
from test_ci_real_root_sample import sample as parent_sample

def root_gen_sample(gen):
    return (
        f"TIMEW_CI_XDG_GEN_SAMPLE lifetime={gen} sequence={gen} "
        f"surface_state_seq={100+gen} exact_native_root=1 "
        "real_scenefx_sample=1 immutable_buffer=UNVERIFIED "
        "final_frame=UNVERIFIED ack=UNVERIFIED source=UNENROLLED "
        "c5_spent=NONE receipt=NONE move=OFF authority=NONE"
    )

def root_gen_commit(gen):
    return (
        f"TIMEW_CI_XDG_GEN_COMMIT lifetime={gen} sequence={gen} "
        f"output_commit_seq={100+gen} sampled_this_scene=1 "
        f"sampled_surface_seq={100+gen} current_surface_seq={100+gen} "
        "accepted_output_commit=1 immutable_buffer=UNVERIFIED "
        "final_frame=UNVERIFIED ack=UNVERIFIED source=UNENROLLED "
        "c5_spent=NONE receipt=NONE move=OFF authority=NONE"
    )

def root_gen_present(gen):
    return (
        f"TIMEW_CI_XDG_GEN_PRESENT lifetime={gen} sequence={gen} "
        f"output_commit_seq={100+gen} current_surface_seq={100+gen} "
        "real_output_present=1 sampled_buffer_in_presented_frame=UNVERIFIED "
        "immutable_buffer=UNVERIFIED final_frame=UNVERIFIED "
        "ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
        "receipt=NONE move=OFF authority=NONE"
    )

def root_gen_seal(gen):
    return (
        f"TIMEW_CI_XDG_GEN_SEAL lifetime={gen} "
        "samples=1 output_commits=1 output_presents=1 "
        "real_unmap=1 final_frame=UNVERIFIED authority=NONE"
    )

def sample(mode="normal"):
    lines=parent_sample(mode)
    for gen in (1, 2):
        i=next(i for i,l in enumerate(lines)
               if f"ROOT_SAMPLE lifetime={gen}" in l)
        lines.insert(i+1,root_gen_sample(gen))
        i=next(i for i,l in enumerate(lines)
               if f"OUTPUT_SEQ_COMMIT lifetime={gen}" in l)
        lines.insert(i+1,root_gen_commit(gen))
        i=next(i for i,l in enumerate(lines)
               if f"OUTPUT_SEQ_PRESENT lifetime={gen}" in l)
        lines.insert(i+1,root_gen_present(gen))
        i=next(i for i,l in enumerate(lines)
               if f"ROOT_SAMPLE_SEAL lifetime={gen}" in l)
        lines.insert(i,root_gen_seal(gen))
    return lines

def replace_gen(rows, gen, tag, before, after):
    prefix=f"TIMEW_CI_XDG_GEN_{tag} lifetime={gen}"
    return [x.replace(before,after) if x.startswith(prefix) else x for x in rows]

class ActualSurfaceGenerationTests(unittest.TestCase):
    def denies(self, rows, mode="normal"):
        with self.assertRaises(ValueError):
            validate_surface_generation(rows,mode)

    def test_normal_sample_commit_present_still_no_authority(self):
        r=validate_surface_generation(sample(),"normal")
        self.assertEqual(r["actual_root_state_samples"],2)
        self.assertEqual(r["unchanged_at_output_commit_diagnostic"],2)
        self.assertEqual(r["unchanged_at_output_present_diagnostic"],2)
        self.assertFalse(r["immutable_buffer_identified"])
        self.assertFalse(r["selected_xdg_final_frame"])
        self.assertEqual(r["safe_actions"],[])
        self.assertFalse(r["settled"])

    def test_forced_timeout_stable_surface_does_not_settle(self):
        r=validate_surface_generation(sample("forced_timeout"),"forced_timeout")
        self.assertEqual(r["unchanged_at_output_present_diagnostic"],2)
        self.assertFalse(r["client_configure_ack"])
        self.assertFalse(r["settled"])

    def test_real_surface_generation_changed_after_sample_before_present(self):
        rows=replace_gen(sample(),1,"PRESENT",
                         "current_surface_seq=101","current_surface_seq=102")
        r=validate_surface_generation(rows,"normal")
        self.assertEqual(r["superseded_or_wrapped_at_present_hold"],1)
        self.assertEqual(r["safe_actions"],[])
        self.assertFalse(r["selected_xdg_final_frame"])

    def test_sampled_generation_zero_never_authorizes(self):
        rows=sample()
        for tag in ("SAMPLE","COMMIT","PRESENT"):
            rows=replace_gen(rows,1,tag,"=101","=0")
        r=validate_surface_generation(rows,"normal")
        self.assertEqual(r["superseded_or_wrapped_at_commit_hold"],1)
        self.assertFalse(r["settled"])

    def test_output_present_unbound_to_commit_keeps_hold(self):
        rows=sample()
        rows=[x.replace("OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=101",
                        "OUTPUT_SEQ_PRESENT lifetime=1 sequence=1 output_commit_seq=999")
              if "TIMEW_CI_XDG_GEN_PRESENT" not in x else x for x in rows]
        rows=replace_gen(rows,1,"PRESENT","output_commit_seq=101","output_commit_seq=999")
        r=validate_surface_generation(rows,"normal")
        self.assertEqual(r["unbound_output_present_hold"],1)
        self.assertFalse(r["selected_xdg_final_frame"])

    def test_missing_root_generation_sample(self):
        self.denies([x for x in sample() if "XDG_GEN_SAMPLE lifetime=1" not in x])

    def test_missing_root_generation_commit(self):
        self.denies([x for x in sample() if "XDG_GEN_COMMIT lifetime=1" not in x])

    def test_missing_root_generation_present(self):
        self.denies([x for x in sample() if "XDG_GEN_PRESENT lifetime=1" not in x])

    def test_duplicate_root_generation_sample(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_SAMPLE lifetime=1" in x)
        rows.insert(i,rows[i])
        self.denies(rows)

    def test_duplicate_root_generation_commit(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_COMMIT lifetime=1" in x)
        rows.insert(i+1,rows[i])
        self.denies(rows)

    def test_duplicate_root_generation_present(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_PRESENT lifetime=1" in x)
        rows.insert(i,rows[i])
        self.denies(rows)

    def test_sample_precedes_actual_scenefx_sample(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_SAMPLE lifetime=1" in x)
        rows[i],rows[i-1]=rows[i-1],rows[i]
        self.denies(rows)

    def test_commit_observed_before_parent_output_commit(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_COMMIT lifetime=1" in x)
        rows[i],rows[i-1]=rows[i-1],rows[i]
        self.denies(rows)

    def test_present_before_parent_output_present(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_PRESENT lifetime=1" in x)
        rows[i],rows[i-1]=rows[i-1],rows[i]
        self.denies(rows)

    def test_wrong_real_output_commit_serial(self):
        self.denies(replace_gen(sample(),1,"COMMIT",
            "output_commit_seq=101","output_commit_seq=999"))

    def test_wrong_real_output_present_serial(self):
        self.denies(replace_gen(sample(),1,"PRESENT",
            "output_commit_seq=101","output_commit_seq=999"))

    def test_sampled_seq_claim_does_not_match_scenefx_sample(self):
        self.denies(replace_gen(sample(),1,"COMMIT",
            "sampled_surface_seq=101","sampled_surface_seq=102"))

    def test_sampled_flag_false_with_real_scene_sample(self):
        self.denies(replace_gen(sample(),1,"COMMIT",
            "sampled_this_scene=1","sampled_this_scene=0"))

    def test_sampled_flag_malformed(self):
        self.denies(replace_gen(sample(),1,"COMMIT",
            "sampled_this_scene=1","sampled_this_scene=2"))

    def test_missing_native_lifetime(self):
        self.denies([x.replace("XDG_GEN_SAMPLE lifetime=2",
                              "XDG_GEN_SAMPLE lifetime=3") for x in sample()])

    def test_replayed_wrong_scene_instruction(self):
        self.denies(replace_gen(sample(),1,"SAMPLE",
            "sequence=1","sequence=999"))

    def test_negative_surface_generation(self):
        self.denies(replace_gen(sample(),1,"SAMPLE",
            "surface_state_seq=101","surface_state_seq=-1"))

    def test_overflow_surface_generation(self):
        self.denies(replace_gen(sample(),1,"SAMPLE",
            "surface_state_seq=101","surface_state_seq=4294967296"))

    def test_out_of_range_output_serial(self):
        self.denies(replace_gen(sample(),1,"COMMIT",
            "output_commit_seq=101","output_commit_seq=4294967296"))

    def test_forged_immutable_buffer(self):
        self.denies([x.replace("immutable_buffer=UNVERIFIED",
                              "immutable_buffer=VERIFIED") for x in sample()])

    def test_forged_final_frame(self):
        self.denies([x.replace("final_frame=UNVERIFIED",
                              "final_frame=VERIFIED") for x in sample()])

    def test_forged_client_ack(self):
        self.denies([x.replace("ack=UNVERIFIED","ack=ACKED") for x in sample()])

    def test_forged_enrolled_native_source(self):
        self.denies([x.replace("source=UNENROLLED","source=ENROLLED")
                     for x in sample()])

    def test_forged_c5_pre_action_spend(self):
        self.denies([x.replace("c5_spent=NONE","c5_spent=SPENT")
                     for x in sample()])

    def test_forged_protected_native_receipt(self):
        self.denies([x.replace("receipt=NONE","receipt=SIGNED")
                     for x in sample()])

    def test_forged_native_move(self):
        self.denies([x.replace("move=OFF","move=ON") for x in sample()])

    def test_missing_lifetime_seal(self):
        self.denies([x for x in sample() if "XDG_GEN_SEAL lifetime=1" not in x])

    def test_duplicate_lifetime_seal(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_SEAL lifetime=1" in x)
        rows.insert(i,rows[i])
        self.denies(rows)

    def test_seal_sample_count_wrong(self):
        self.denies([x.replace("samples=1","samples=4") for x in sample()])

    def test_seal_commit_count_wrong(self):
        self.denies([x.replace("output_commits=1","output_commits=4")
                     for x in sample()])

    def test_seal_present_count_wrong(self):
        self.denies([x.replace("output_presents=1","output_presents=4")
                     for x in sample()])

    def test_gen_sample_after_seal(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_SAMPLE lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "XDG_GEN_SEAL lifetime=1" in x)
        rows.insert(j+1,item)
        self.denies(rows)

    def test_gen_commit_after_seal(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_COMMIT lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "XDG_GEN_SEAL lifetime=1" in x)
        rows.insert(j+1,item)
        self.denies(rows)

    def test_gen_present_after_seal(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_PRESENT lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "XDG_GEN_SEAL lifetime=1" in x)
        rows.insert(j+1,item)
        self.denies(rows)

    def test_gen_seal_after_native_unmap(self):
        rows=sample()
        i=next(i for i,x in enumerate(rows) if "XDG_GEN_SEAL lifetime=1" in x)
        item=rows.pop(i)
        j=next(i for i,x in enumerate(rows) if "REAL_XDG_UNMAPPED gen=1" in x)
        rows.insert(j+1,item)
        self.denies(rows)

    def test_forged_sampled_frame_presented(self):
        self.denies([x.replace("sampled_buffer_in_presented_frame=UNVERIFIED",
                              "sampled_buffer_in_presented_frame=VERIFIED")
                     for x in sample()])

    def test_wrong_native_outcome_mode(self):
        self.denies(sample("forced_timeout"),"normal")

if __name__ == "__main__":
    unittest.main(verbosity=2)
