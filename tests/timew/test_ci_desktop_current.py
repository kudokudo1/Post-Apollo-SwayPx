import copy
import struct
import unittest
from ci_assert_desktop_current import validate

def fixture():
    def view(i):
        return {"type":"con","id":i,"app_id":"timew-capture-fixture","nodes":[]}
    def row(i,floating):
        return {"con_id":i,"workspace_node":4,"output_node":3,"floating":floating,
                "current_bits":[struct.pack(">d",x).hex() for x in (0.5,0.,640.,480.)]}
    return {"type":"root","nodes":[{"type":"output","id":3,"name":"HEADLESS-1","nodes":[
        {"type":"workspace","id":4,"nodes":[view(5)],"floating_nodes":[view(6)]}]}],
        "timew_ci_current":{"status":"CURRENT_DIAGNOSTIC","authority":"NONE","rows":[row(5,False),row(6,True)]}}

class CoverageTests(unittest.TestCase):
    def test_two_same_app_views(self):
        self.assertEqual(validate(fixture()),2)
    def test_missing_row(self):
        x=fixture(); x["timew_ci_current"]["rows"].pop()
        with self.assertRaises(ValueError): validate(x)
    def test_duplicate_row(self):
        x=fixture(); x["timew_ci_current"]["rows"][1]=copy.deepcopy(x["timew_ci_current"]["rows"][0])
        with self.assertRaises(ValueError): validate(x)
    def test_wrong_context(self):
        x=fixture(); x["timew_ci_current"]["rows"][0]["workspace_node"]=99
        with self.assertRaises(ValueError): validate(x)
    def test_nonfinite(self):
        x=fixture(); x["timew_ci_current"]["rows"][0]["current_bits"][0]="7ff0000000000000"
        with self.assertRaises(ValueError): validate(x)
    def test_partial_refusal(self):
        x=fixture(); x["timew_ci_current"]["status"]="REFUSED"
        with self.assertRaises(ValueError): validate(x)
    def test_authority_forgery(self):
        x=fixture(); x["timew_ci_current"]["authority"]="RESTORE"
        with self.assertRaises(ValueError): validate(x)

if __name__ == "__main__":
    unittest.main(verbosity=2)
