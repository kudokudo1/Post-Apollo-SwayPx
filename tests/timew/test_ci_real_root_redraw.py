import unittest
from ci_assert_real_root_redraw import checkpoint, verify_after

BASE = [
    "TIMEW_CI_REAL_XDG_MAPPED gen=1 fixture=1",
    "TIMEW_CI_XDG_GEN_SAMPLE lifetime=1 sequence=2 surface_state_seq=30 root_client_object_match=1 watch_epoch=7 rest",
    "TIMEW_CI_XDG_GEN_PRESENT lifetime=1 sequence=2 output_commit_seq=90 current_surface_seq=30 watch_epoch=7 rest",
]
def revoke(reason="surface_commit",life=1,epoch=7):
    return f"TIMEW_CI_XDG_WATCH_REVOKE lifetime={life} epoch={epoch} reason={reason} authority=NONE"
def root(seq=31,prior=7,life=1,wire=9):
    return f"TIMEW_CI_XDG_REDRAW_COMMIT lifetime={life} surface_state_seq={seq} prior_watch_epoch={prior} wire_request_epoch={wire} authority=NONE"
def wire(epoch=9,nonnull=1,life=1):
    return [f"TIMEW_CI_XDG_WIRE_ATTACH lifetime={life} request_epoch={epoch} nonnull={nonnull} authority=NONE",f"TIMEW_CI_XDG_WIRE_COMMIT lifetime={life} request_epoch={epoch} nonnull={nonnull} authority=NONE"]

class RedrawTest(unittest.TestCase):
    def test_surface_commit(self):
        self.assertEqual(verify_after(BASE+wire()+[revoke(),root()],3),(1,7,"surface_commit"))
    def test_destroy_then_commit(self):
        self.assertEqual(verify_after(BASE+wire()+[revoke("buffer_destroy"),root(prior=0)],3),(1,7,"buffer_destroy"))
    def test_stale_before_checkpoint(self):
        with self.assertRaises(ValueError): checkpoint(BASE+[revoke()])
    def test_wrong_lifetime(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke(life=2),root(life=2)],3)
    def test_no_changed_root_state(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()+[revoke(),root(seq=30)],3)
    def test_unmap_is_not_redraw(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()+[revoke("unmap"),root()],3)
    def test_wrong_callback_epoch(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()+[revoke(),root(prior=8)],3)
    def test_next_lifetime_is_not_same_actor(self):
        with self.assertRaises(ValueError): verify_after(BASE+["TIMEW_CI_REAL_XDG_UNMAPPED gen=1",revoke(),root()],3)

    def test_damage_only_commit_not_attachment_request(self):
        with self.assertRaises(ValueError): verify_after(BASE+[revoke(),root(wire=0)],3)
    def test_null_attach_not_qualifying(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire(nonnull=0)+[revoke(),root()],3)
    def test_wrong_wire_epoch(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()+[revoke(),root(wire=8)],3)
    def test_commit_without_attach(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()[1:]+[revoke(),root()],3)
    def test_replayed_attach_request(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire()+wire()+[revoke(),root()],3)
    def test_foreign_attach(self):
        with self.assertRaises(ValueError): verify_after(BASE+wire(life=2)+[revoke(),root()],3)

if __name__ == "__main__":
    unittest.main(verbosity=2)
