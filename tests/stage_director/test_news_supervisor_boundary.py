#!/usr/bin/env python3
"""Keep test-child execution isolated from deployed Stage Director."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda p: (root / p).read_text(encoding="utf-8")
supervisor = read("sway/stage_director_news_supervisor.c")
header = read("include/sway/stage_director_news_supervisor.h")
test = read("tests/stage_director/test_news_supervisor.c")
meson = read("sway/meson.build")
workflow = read(".github/workflows/stage-director-viewport-contracts.yml")
core = read("sway/stage_director.c")
clip = read("sway/stage_director_viewport_clip.c")
custody = read("sway/stage_director_news_custody.c")
plan = read("sway/stage_director_news_launch_plan.c")

assert "'stage_director_news_supervisor.c'" in meson
assert "SD_STAGE_DIRECTOR_TEST_HELPER" in supervisor
assert "SD_STAGE_DIRECTOR_TEST_HELPER" in header
assert "/usr/bin/sleep" in supervisor
assert "posix_spawn(" in supervisor
assert "pidfd_open" in supervisor
assert "pidfd_send_signal" in supervisor
assert "waitpid(s->child_pid, &status, WNOHANG)" in supervisor
assert "stage_director_news_supervisor_finish(" in supervisor
assert "epoch <= s->last_epoch" in supervisor
assert "stage_director_news_supervisor_poll(" in supervisor
assert "stage_director_news_supervisor_start_test_child" in test
assert "SD_STAGE_DIRECTOR_TEST_HELPER" in workflow
assert "nm -g" in workflow

# Never allow the upstream observer or a candidate browser argv to execute.
for source in (core, clip, custody, plan):
    for method in ("stage_director_news_supervisor_start_test_child(",
                   "stage_director_news_supervisor_request_stop(",
                   "stage_director_news_supervisor_poll("):
        assert method not in source
assert "stage_director_news_launch_plan" not in supervisor
for forbidden in (
    "wlr_scene_subsurface_tree_set_clip(",
    "wlr_scene_node_set_position(",
    "sway_seat_set_focus(", "QtWebEngine",
    "system(", "popen(", "execvp(",
):
    assert forbidden not in supervisor
for permission in ("owner_verified", "can_launch_browser",
                   "can_clip", "can_accept_input"):
    assert "." + permission + " = false" in supervisor

print("Stage Director NEWS supervisor test-only execution boundary: PASS")
