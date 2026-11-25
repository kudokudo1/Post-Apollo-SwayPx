import unittest
from ci_assert_real_root_redraw import checkpoint, verify_after

BASE = [
    "TIMEW_CI_REAL_XDG_MAPPED gen=1 fixture=1",
    "TIMEW_CI_XDG_GEN_SAMPLE lifetime=1 sequence=2 surface_state_seq=30 root_client_object_match=1 watch_epoch=7 rest",
    "TIMEW_CI_XDG_GEN_PRESENT lifetime=1 sequence=2 output_commit_seq=90 current_surface_seq=30 watch_epoch=7 rest",
]
def revoke(reason="surface_commit",life=1,epoch=7):
    return f"TIMEW_CI_XDG_WATCH_REVOKE lifetime={life} epoch={epoch} reason={reason} authority=NONE"
def root(seq=31,prior=7,life=1):
    return f"TIMEW_CI_XDG_REDRAW_COMMIT lifetime={life} surface_state_seq={seq} prior_watch_epoch={prior} authority=NONE"

class RedrawTest(unittest.TestCase):
    def test_surface_commit(self):
        self.assertEqual(verify_after(BASE+[revoke(),root()],3),(1,7,"surface_commit"))
    def test_destroy_then_commit(self):
        self.assertEqual(verify_after(BASE+[revoke("buffer_destroy"),root(prior=0)],3),(1,7,"buffer_destroy"))
    def test_stale_before_checkpoint(self):
        with self.assertRaises(ValueError): checkpoint(BASE+[revoke()])
    def test_wrong_lifetime(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke(life=2),root(life=2)],3)
    def test_no_changed_root_state(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke(),root(seq=30)],3)
    def test_unmap_is_not_redraw(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke("unmap"),root()],3)
    def test_wrong_callback_epoch(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke(),root(prior=8)],3)
    def test_next_lifetime_is_not_same_actor(self):
        with self.assertRaises(ValueError): verify_after(BASE+["TIMEW_CI_REAL_XDG_UNMAPPED gen=1",revoke(),root()],3)

if __name__ == "__main__":
    unittest.main(verbosity=2)
