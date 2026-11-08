#!/usr/bin/env python3
"""Isolated CI cursor event adapter only. Uses production seatop code as-is."""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess

def only(src: str, before: str, after: str, reason: str) -> str:
    if src.count(before)!=1 or after in src:
        raise RuntimeError("SD_SOCIAL_PRODUCTION_CURSOR_ANCHOR_DRIFT "+reason)
    return src.replace(before,after,1)

CURSOR = "sway/commands/seat/cursor.c"
C_CURSOR = r'''
/* Private CI-only operation starter / motion driver. Not a public Sway API.
 * This exercises the actual seatop_move_floating handle_pointer_motion,
 * whereas ordinary 'seat cursor move' does only a cursor_rebase.
 */
static bool sd_ci_social_cursor_ready(void) {
    const char *ci=getenv("CI"), *actions=getenv("GITHUB_ACTIONS");
    const char *mode=getenv("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE");
    const char *temp=getenv("RUNNER_TEMP");
    return ci && !strcmp(ci,"true") && actions && !strcmp(actions,"true") &&
        mode && !strcmp(mode,"1") && temp &&
        !strcmp(temp,"/home/runner/work/_temp");
}

static struct cmd_results *sd_ci_social_cursor_command(
        struct sway_cursor *cursor, int argc, char **argv) {
    if (!sd_ci_social_cursor_ready() || argc < 2)
        return cmd_results_new(CMD_FAILURE, "CI-only Social move disabled");
    struct sway_seat *seat = cursor->seat;
    if (!strcmp(argv[0], "ci-social-start-move")) {
        struct sway_container *con = seat_get_focused_container(seat);
        if (!con || !con->view || !container_is_floating(con) ||
                con->node.destroying || !con->view->surface)
            return cmd_results_new(CMD_FAILURE, "No verified floating view");
        seatop_begin_move_floating(seat, con);
        return cmd_results_new(CMD_SUCCESS, NULL);
    }
    if (!strcmp(argv[0], "ci-social-start-resize")) {
        struct sway_container *con = seat_get_focused_container(seat);
        if (!con || !con->view || !container_is_floating(con) ||
                con->node.destroying || !con->view->surface)
            return cmd_results_new(CMD_FAILURE, "No verified resize view");
        seatop_begin_resize_floating(seat, con,
                                    WLR_EDGE_BOTTOM | WLR_EDGE_RIGHT);
        return cmd_results_new(CMD_SUCCESS, NULL);
    }
    if (!strcmp(argv[0], "ci-social-step") && argc >= 3) {
        char *end_x = NULL, *end_y = NULL;
        long dx = strtol(argv[1], &end_x, 10);
        long dy = strtol(argv[2], &end_y, 10);
        if (!end_x || !end_y || *end_x || *end_y ||
                dx < -160 || dx > 160 || dy < -160 || dy > 160)
            return cmd_results_new(CMD_INVALID, "Bad CI-only pointer delta");
        wlr_cursor_move(cursor->cursor, NULL, dx, dy);
        seatop_pointer_motion(seat, 0);
        return cmd_results_new(CMD_SUCCESS, NULL);
    }
    if (!strcmp(argv[0], "ci-social-end")) {
        seatop_button(seat, 0, NULL, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
        return cmd_results_new(CMD_SUCCESS, NULL);
    }
    return cmd_results_new(CMD_INVALID, "Unknown CI-only pointer command");
}
'''
def patch_cursor(s: str) -> str:
    s = only(s, '#include "sway/server.h"\n',
        '#include "sway/server.h"\n'
        '#include "sway/input/seat.h"\n'
        '#include "sway/tree/container.h"\n'
        '#include "sway/tree/view.h"\n'
        '#include <wlr/util/edges.h>\n'
        '#include <stdlib.h>\n#include <string.h>\n', "cursor_includes")
    anchor = 'static const char expected_syntax[] = "Expected '
    if s.count(anchor) != 1:
        raise RuntimeError("CI_SOCIAL_NATIVE_ANCHOR_DRIFT cursor_syntax")
    s = s.replace(anchor, C_CURSOR + '\n' + anchor, 1)
    before = '''static struct cmd_results *handle_command(struct sway_cursor *cursor,
		int argc, char **argv) {
	if (strcasecmp(argv[0], "move") == 0) {
'''
    after = '''static struct cmd_results *handle_command(struct sway_cursor *cursor,
		int argc, char **argv) {
	if (strncmp(argv[0], "ci-social-", 10) == 0)
		return sd_ci_social_cursor_command(cursor, argc, argv);
	if (strcasecmp(argv[0], "move") == 0) {
'''
    return only(s,before,after,"cursor_routing")



def main():
    e=os.environ
    tmp=Path(e.get("RUNNER_TEMP","/missing"))
    ws=Path(e.get("GITHUB_WORKSPACE","/missing")).resolve()
    if not(e.get("CI")=="true" and e.get("GITHUB_ACTIONS")=="true"
           and e.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE")=="1"
           and str(tmp)=="/home/runner/work/_temp" and tmp.is_dir()
           and ws.is_dir() and os.getuid()!=0):
        raise RuntimeError("BLOCKED_SD_SOCIAL_PRODUCTION_CURSOR_NOT_CI")
    dst=tmp/"sd-ci-social-production-src"
    if dst.exists(): raise RuntimeError("SD_SOCIAL_PRODUCTION_COPY_ALREADY_EXISTS")
    shutil.copytree(ws,dst,symlinks=False,ignore=shutil.ignore_patterns(
        "__pycache__","result"))
    cursor=dst/CURSOR
    cursor.write_text(patch_cursor(cursor.read_text(encoding="utf-8")),
                      encoding="utf-8")
    changed=subprocess.run(["git","-C",str(dst),"status","--short"],
        capture_output=True,text=True,check=True).stdout.splitlines()
    if changed!=[" M "+CURSOR]:
        raise RuntimeError("SD_SOCIAL_PRODUCTION_UNEXPECTED_DIFF "+repr(changed))
    original=subprocess.run(["git","-C",str(ws),"status","--porcelain"],
        capture_output=True,text=True,check=True).stdout
    if original:
        raise RuntimeError("SD_SOCIAL_PRODUCTION_MAIN_CHECKOUT_DIRTY")
    print("SD_SOCIAL_PRODUCTION_CURSOR_OVERLAY "
          "only_cursor_adapter_modified=1 production_seatop_source=UNMODIFIED",
          flush=True)

if __name__=="__main__":
    main()
