#!/usr/bin/env python3
"""Guard against pidfd custody being mistaken for process/scene authority."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
read = lambda p: (root / p).read_text(encoding="utf-8")

custody = read("sway/stage_director_news_custody.c")
header = read("include/sway/stage_director_news_custody.h")
meson = read("sway/meson.build")
test = read("tests/stage_director/test_news_custody.c")
core = read("sway/stage_director.c")
session = read("sway/stage_director_viewer_session.c")
clip = read("sway/stage_director_viewport_clip.c")

assert "'stage_director_news_custody.c'" in meson
assert "SYS_pidfd_open" in custody
assert "stage_director_read_linux_process(" in custody
assert "second.start_time_ticks == first.start_time_ticks" in custody
assert "second.uid == first.uid" in custody
assert "pidfd_alive(descriptor)" in custody
assert "poll(&fd, 1, 0)" in custody
assert "close(lease->pidfd)" in custody
assert "lease->last_epoch" in custody
assert "epoch <= lease->last_epoch" in custody
assert "unique_native_host_candidate" in custody
assert "fork()" in test and "waitpid(" in test
assert "stage_director_news_custody_capture(" in header
for flag in (".owner_verified = false", ".can_launch = false",
             ".can_bind_viewer = false", ".can_clip = false",
             ".can_accept_input = false"):
    assert flag in custody, flag

# Actual compositor hooks remain SD0 shadow-only. The new native holder
# cannot be used by QML, IPC, session/clip callbacks or arbitrary launchers.
for code in (core, session, clip):
    assert "stage_director_news_custody_capture(" not in code
    assert "stage_director_news_custody_reserve(" not in code
for forbidden in ("fork(", "execv(", "execve(", "posix_spawn(",
                  "system(", "kill(", "pidfd_send_signal",
                  "stage_director_viewport_clip_view(",
                  "wlr_scene_subsurface_tree_set_clip(",
                  "sway_seat_set_focus("):
    assert forbidden not in custody, forbidden
print("Stage Director pidfd custody remains disconnected and nonactuating: PASS")
