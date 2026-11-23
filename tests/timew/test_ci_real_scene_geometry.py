"""Adversarial tests for untrusted native post-scene snapshot log parsing."""
import math
import struct
import unittest

from ci_assert_real_scene_geometry import validate_scene_geometry
from test_ci_outcome_checker import sample

def bits(value):
    return struct.pack(">d", float(value)).hex()

def row(generation):
    return (
        f"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime={generation} "
        f"sequence={generation} "
        f"current_x={bits(259)} current_y={bits(92.5)} "
        f"current_w={bits(1402)} current_h={bits(872)} "
        f"pending_x={bits(259)} pending_y={bits(92.5)} "
        f"pending_w={bits(1402)} pending_h={bits(872)} "
        f"scale={bits(1.25)} transform=0 "
        "output_lx=0 output_ly=0 output_w=1920 output_h=1080 "
        "native_source=REAL_XDG same_context=1 "
        "ack=0 frame=0 epoch_proof=0 source=UNENROLLED "
        "selected=0 spent=0 receipt=NONE action=NONE"
    )

def fixture(mode="normal"):
    lines = sample(mode)
    for g in (1, 2):
        i = next(i for i, line in enumerate(lines)
                 if f"TIMEW_CI_OUTCOME_SCENE lifetime={g}" in line)
        lines.insert(i + 1, row(g))
    return lines

class NativeSceneGeometryTests(unittest.TestCase):
    def refuses(self, lines, mode="normal"):
        with self.assertRaises(ValueError):
            validate_scene_geometry(lines, mode)

    def test_realistic_fractional_native_values_do_not_authorize(self):
        result = validate_scene_geometry(fixture(), "normal")
        self.assertEqual(result["snapshots"], 2)
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_real_timeout_scene_still_denies(self):
        result = validate_scene_geometry(fixture("forced_timeout"), "forced_timeout")
        self.assertEqual(result["snapshots"], 2)
        self.assertEqual(result["safe_actions"], [])
        self.assertFalse(result["settled"])

    def test_absent_lifetime_snapshot(self):
        self.refuses([s for s in fixture() if "NATIVE_SCENE_SNAPSHOT lifetime=2" not in s])

    def test_duplicate_snapshot(self):
        lines = fixture()
        i = next(i for i,s in enumerate(lines) if "NATIVE_SCENE_SNAPSHOT lifetime=1" in s)
        lines.insert(i+1, lines[i])
        self.refuses(lines)

    def test_snapshot_before_scene(self):
        lines = fixture()
        i = next(i for i,s in enumerate(lines) if "NATIVE_SCENE_SNAPSHOT lifetime=1" in s)
        lines[i-1],lines[i] = lines[i],lines[i-1]
        self.refuses(lines)

    def test_snapshot_after_seal(self):
        lines = fixture()
        i = next(i for i,s in enumerate(lines) if "NATIVE_SCENE_SNAPSHOT lifetime=1" in s)
        item = lines.pop(i)
        seal = next(i for i,s in enumerate(lines) if "OUTCOME_SEAL lifetime=1" in s)
        lines.insert(seal+1,item)
        self.refuses(lines)

    def test_native_snapshot_cannot_claim_ack(self):
        lines=fixture()
        lines=[s.replace("ack=0", "ack=1") for s in lines]
        self.refuses(lines)

    def test_native_snapshot_cannot_claim_frame(self):
        lines=fixture()
        lines=[s.replace("frame=0", "frame=1") for s in lines]
        self.refuses(lines)

    def test_native_snapshot_cannot_claim_enrolled_source(self):
        lines=fixture()
        lines=[s.replace("source=UNENROLLED", "source=ENROLLED")
               for s in lines]
        self.refuses(lines)

    def test_native_snapshot_cannot_claim_c5_spend(self):
        lines=fixture()
        lines=[s.replace("spent=0", "spent=1") for s in lines]
        self.refuses(lines)

    def test_nonfinite_fraction_rejected(self):
        lines=fixture()
        lines=[s.replace("current_y="+bits(92.5), "current_y="+bits(math.nan))
               for s in lines]
        self.refuses(lines)

    def test_negative_zero_rejected(self):
        lines=fixture()
        lines=[s.replace("current_x="+bits(259), "current_x="+bits(-0.0))
               for s in lines]
        self.refuses(lines)

    def test_negative_width_rejected(self):
        lines=fixture()
        lines=[s.replace("current_w="+bits(1402), "current_w="+bits(-5.0))
               for s in lines]
        self.refuses(lines)

    def test_scale_zero_rejected(self):
        lines=fixture()
        lines=[s.replace("scale="+bits(1.25), "scale="+bits(0.0))
               for s in lines]
        self.refuses(lines)

    def test_invalid_output_transform_rejected(self):
        lines=fixture()
        lines=[s.replace("transform=0 ", "transform=8 ") for s in lines]
        self.refuses(lines)

    def test_invalid_output_dimensions_rejected(self):
        lines=fixture()
        lines=[s.replace("output_w=1920", "output_w=0") for s in lines]
        self.refuses(lines)

    def test_stale_new_lifetime_rejected(self):
        lines=fixture()
        lines=[s.replace("NATIVE_SCENE_SNAPSHOT lifetime=2",
                         "NATIVE_SCENE_SNAPSHOT lifetime=1") for s in lines]
        self.refuses(lines)

    def test_forged_orphan_snapshot_rejected(self):
        lines=fixture()
        lines.insert(0, row(9))
        self.refuses(lines)

    def test_missing_parent_bind_rejected(self):
        lines=fixture()
        lines=[s for s in lines if "REAL_INSTRUCTION_BIND lifetime=1" not in s]
        self.refuses(lines)

    def test_deferred_native_read_cannot_substitute_snapshot(self):
        lines=fixture()
        lines=[s for s in lines if "NATIVE_SCENE_SNAPSHOT lifetime=1" not in s]
        lines.insert(1,
            "TIMEW_CI_NATIVE_SCENE_DEFER lifetime=1 sequence=1 "
            "native_snapshot_unavailable=1 authority=NONE")
        self.refuses(lines)

if __name__ == "__main__":
    unittest.main(verbosity=2)
