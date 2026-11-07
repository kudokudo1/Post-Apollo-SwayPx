#!/usr/bin/env python3
"""GitHub-only disposable compositor source overlay for Social native move proof.

This never edits the checked-out repository. A fresh copy under RUNNER_TEMP
is the only location where seatop/cursor C files are instrumented, and the
CI build uses that copy. No native group API or production paths are added.
"""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess
import sys

MOVE = "sway/input/seatop_move_floating.c"
CURSOR = "sway/commands/seat/cursor.c"
RESIZE = "sway/input/seatop_resize_floating.c"

def only(src: str, before: str, after: str, reason: str) -> str:
    if src.count(before) != 1 or after in src:
        raise RuntimeError("CI_SOCIAL_NATIVE_ANCHOR_DRIFT " + reason)
    return src.replace(before, after, 1)


C_HELPERS = r'''
/* DISPOSABLE CI OVERLAY ONLY. This does not belong to production SwayPX. */
static bool sd_ci_social_active(void) {
    const char *ci = getenv("CI");
    const char *actions = getenv("GITHUB_ACTIONS");
    const char *mode = getenv("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE");
    const char *temp = getenv("RUNNER_TEMP");
    return ci && strcmp(ci, "true") == 0 &&
        actions && strcmp(actions, "true") == 0 &&
        mode && strcmp(mode, "1") == 0 &&
        temp && strcmp(temp, "/home/runner/work/_temp") == 0;
}

struct sd_ci_social_find { size_t id; };
static bool sd_ci_social_find_id(struct sway_container *con, void *data) {
    struct sd_ci_social_find *query = data;
    return con && !con->node.destroying && con->node.id == query->id;
}

static struct sway_container *sd_ci_social_exact(size_t id) {
    struct sd_ci_social_find query = {.id = id};
    return root_find_container(sd_ci_social_find_id, &query);
}

static void sd_ci_social_attach(struct seatop_move_floating_event *e) {
    if (!sd_ci_social_active()) return;
    const char *temp = getenv("RUNNER_TEMP");
    const char *path = getenv("SD_SOCIAL_CI_LEASE_FILE");
    if (!path || strncmp(path, temp, strlen(temp)) != 0 ||
            path[strlen(temp)] != '/' || strlen(path) > 400) return;
    struct stat st;
    if (lstat(path, &st) != 0 || !S_ISREG(st.st_mode) ||
            st.st_uid != getuid() || (st.st_mode & 0777) != 0600)
        return;
    FILE *file = fopen(path, "r");
    if (!file) return;
    unsigned long long host_id=0, guest_id=0;
    long host_pid=0, guest_pid=0;
    int got = fscanf(file, "%llu %llu %ld %ld",
        &host_id, &guest_id, &host_pid, &guest_pid);
    fclose(file);
    if (got != 4 || host_id == 0 || guest_id == 0 ||
            host_id == guest_id || host_pid <= 0 || guest_pid <= 0)
        return;
    struct sway_container *host = sd_ci_social_exact((size_t)host_id);
    struct sway_container *guest = sd_ci_social_exact((size_t)guest_id);
    if (!host || !guest || host == guest ||
            !host->view || !guest->view ||
            host->view->pid != host_pid || guest->view->pid != guest_pid ||
            !host->view->surface || !guest->view->surface ||
            !container_is_floating(host) || !container_is_floating(guest) ||
            !host->pending.workspace ||
            host->pending.workspace != guest->pending.workspace ||
            (e->con != host && e->con != guest)) {
        fprintf(stderr, "SD_CI_SOCIAL_NATIVE_ATTACH denied=1 stale_or_unverified=1\n");
        return;
    }
    struct sway_container *peer = e->con == host ? guest : host;
    e->sd_ci_peer = peer;
    e->sd_ci_peer_dx = peer->pending.x - e->con->pending.x;
    e->sd_ci_peer_dy = peer->pending.y - e->con->pending.y;
    fprintf(stderr, "SD_CI_SOCIAL_NATIVE_ATTACH accepted=1 leader_id=%zu peer_id=%zu\n",
        e->con->node.id, peer->node.id);
}
'''

def patch_move(s: str) -> str:
    s = only(s, '#include "sway/input/seat.h"\n',
        '#include "sway/input/seat.h"\n'
        '#include "sway/tree/root.h"\n'
        '#include "sway/tree/view.h"\n'
        '#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n'
        '#include <sys/stat.h>\n#include <unistd.h>\n',
        "move_includes")
    struct_old = '''struct seatop_move_floating_event {
	struct sway_container *con;
	double dx, dy; // cursor offset in container
};
'''
    struct_new = '''struct seatop_move_floating_event {
	struct sway_container *con;
	double dx, dy; // cursor offset in container
	struct sway_container *sd_ci_peer;
	double sd_ci_peer_dx, sd_ci_peer_dy;
};
''' + C_HELPERS
    s = only(s, struct_old, struct_new, "move_struct")
    follow_old = '''	container_floating_move_to(e->con, cursor->x - e->dx, cursor->y - e->dy);
	transaction_commit_dirty();
'''
    follow_new = '''	container_floating_move_to(e->con, cursor->x - e->dx, cursor->y - e->dy);
	/* Tested native seatop boundary: BOTH pending rectangles before commit. */
	if (e->sd_ci_peer && !e->sd_ci_peer->node.destroying &&
			e->sd_ci_peer->view && e->sd_ci_peer->view->surface &&
			e->con->pending.workspace &&
			e->sd_ci_peer->pending.workspace == e->con->pending.workspace) {
		container_floating_move_to(e->sd_ci_peer,
			e->con->pending.x + e->sd_ci_peer_dx,
			e->con->pending.y + e->sd_ci_peer_dy);
		fprintf(stderr,
			"SD_CI_SOCIAL_NATIVE_FOLLOW before_commit=1 leader=%zu peer=%zu\\n",
			e->con->node.id, e->sd_ci_peer->node.id);
	}
	transaction_commit_dirty();
'''
    s = only(s, follow_old, follow_new, "seatop_precommit")
    unref_old = '''	if (e->con == con) {
		seatop_begin_default(seat);
	}
'''
    unref_new = '''	if (e->sd_ci_peer == con) {
		e->sd_ci_peer = NULL;
		fprintf(stderr, "SD_CI_SOCIAL_NATIVE_REVOKE peer_unmapped=1\\n");
	}
	if (e->con == con) {
		seatop_begin_default(seat);
	}
'''
    s = only(s, unref_old, unref_new, "seatop_unref")
    attach_old = '''	e->dy = cursor->cursor->y - con->pending.y;

	con->animation_state.seat_is_moving_float = true;
'''
    attach_new = '''	e->dy = cursor->cursor->y - con->pending.y;
	sd_ci_social_attach(e);

	con->animation_state.seat_is_moving_float = true;
'''
    return only(s, attach_old, attach_new, "seatop_attach")



def patch_resize(s: str) -> str:
    """Disposable native resize seatop: pair before its existing commit."""
    s=only(s, '#include "sway/tree/container.h"\n',
        '#include "sway/tree/container.h"\n'
        '#include "sway/tree/root.h"\n'
        '#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n'
        '#include <sys/stat.h>\n#include <unistd.h>\n',
        "resize_includes")
    old='''struct seatop_resize_floating_event {
	struct sway_container *con;
	enum wlr_edges edge;
	bool preserve_ratio;
	double ref_lx, ref_ly;         // cursor's x/y at start of op
	double ref_width, ref_height;  // container's size at start of op
	double ref_con_lx, ref_con_ly; // container's x/y at start of op
};
'''
    helpers=(C_HELPERS.replace(
        "struct seatop_move_floating_event",
        "struct seatop_resize_floating_event")
        .replace(
            '    e->sd_ci_peer_dx = peer->pending.x - e->con->pending.x;\n','')
        .replace(
            '    e->sd_ci_peer_dy = peer->pending.y - e->con->pending.y;\n','')
        .replace(
            '    e->sd_ci_peer = peer;\n',
            '    e->sd_ci_peer = peer;\n'
            '    e->sd_ci_host_leader = e->con == host;\n'))
    new=old.replace('};\n','\tstruct sway_container *sd_ci_peer;\n'
                    '\tbool sd_ci_host_leader;\n};\n',1)+helpers
    s=only(s,old,new,"resize_event_and_lease")

    follow_old='''	arrange_container(con);
	transaction_commit_dirty();
'''
    follow_new='''	arrange_container(con);
	/* The verified companion gets pending geometry in this SAME seatop
	 * transaction. This is CI policy, not authorized production identity. */
	if (e->sd_ci_peer && !e->sd_ci_peer->node.destroying &&
			e->sd_ci_peer->view && e->sd_ci_peer->view->surface &&
			e->con->pending.workspace &&
			e->sd_ci_peer->pending.workspace == e->con->pending.workspace) {
		double x, y, w, h;
		if (e->sd_ci_host_leader) {
			x = con->pending.x + 111;
			y = con->pending.y + 1;
			w = con->pending.width - 112;
			h = con->pending.height - 2;
		} else {
			x = con->pending.x - 111;
			y = con->pending.y - 1;
			w = con->pending.width + 112;
			h = con->pending.height + 2;
		}
		if (w >= 200 && h >= 200 && x >= 0 && y >= 0 &&
				x + w <= 1920 && y + h <= 1080 &&
				con->pending.width >= 200 && con->pending.height >= 200) {
			struct sway_container *peer = e->sd_ci_peer;
			double dx = x - peer->pending.x;
			double dy = y - peer->pending.y;
			double dw = w - peer->pending.width;
			double dh = h - peer->pending.height;
			peer->pending.x += dx;
			peer->pending.y += dy;
			peer->pending.width += dw;
			peer->pending.height += dh;
			peer->pending.content_x += dx;
			peer->pending.content_y += dy;
			peer->pending.content_width += dw;
			peer->pending.content_height += dh;
			arrange_container(peer);
			fprintf(stderr, "SD_CI_SOCIAL_NATIVE_RESIZE_BEFORE_COMMIT leader=%zu peer=%zu\\n",
				e->con->node.id,peer->node.id);
		}
	}
	transaction_commit_dirty();
'''
    s=only(s,follow_old,follow_new,"resize_precommit")
    unref_old='''	if (e->con == con) {
		seatop_begin_default(seat);
	}
'''
    unref_new='''	if (e->sd_ci_peer == con) {
		e->sd_ci_peer = NULL;
		fprintf(stderr, "SD_CI_SOCIAL_NATIVE_RESIZE_REVOKED peer_unmapped=1\\n");
	}
	if (e->con == con) {
		seatop_begin_default(seat);
	}
'''
    s=only(s,unref_old,unref_new,"resize_unref")
    attach_old='''	e->ref_width = con->pending.width;
	e->ref_height = con->pending.height;

	seat->seatop_impl = &seatop_impl;
'''
    attach_new='''	e->ref_width = con->pending.width;
	e->ref_height = con->pending.height;
	sd_ci_social_attach(e);

	seat->seatop_impl = &seatop_impl;
'''
    return only(s,attach_old,attach_new,"resize_attach")


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

def main() -> None:
    e=os.environ
    tmp=Path(e.get("RUNNER_TEMP","/absent"))
    ws=Path(e.get("GITHUB_WORKSPACE","/absent")).resolve()
    if not(e.get("CI")=="true" and e.get("GITHUB_ACTIONS")=="true"
           and e.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE")=="1"
           and str(tmp)=="/home/runner/work/_temp" and tmp.is_dir()
           and ws.is_dir() and (ws/"sway/input/seatop_move_floating.c").is_file()
           and os.getuid()!=0):
        raise RuntimeError("BLOCKED_SOCIAL_NATIVE_PATCH_NOT_CI")
    dest=tmp/"sd-ci-social-native-move-src"
    if dest.exists():
        raise RuntimeError("BLOCKED_SOCIAL_NATIVE_PATCH_DEST_EXISTS")
    shutil.copytree(ws,dest,symlinks=False,
                    ignore=shutil.ignore_patterns("__pycache__","result"),
                    dirs_exist_ok=False)
    # Source modifications are made ONLY under private disposable CI temp.
    for path,fn in ((MOVE,patch_move),(CURSOR,patch_cursor),(RESIZE,patch_resize)):
        f=dest/path
        if f.is_symlink() or not f.is_file() or f.stat().st_size>250000:
            raise RuntimeError("CI_SOCIAL_NATIVE_PATCH_DISALLOWED_PATH "+path)
        original=f.read_text(encoding="utf-8")
        patched=fn(original)
        if patched==original:
            raise RuntimeError("CI_SOCIAL_NATIVE_PATCH_NOOP "+path)
        f.write_text(patched,encoding="utf-8")
    check=subprocess.run(["git","-C",str(dest),"status","--short"],
                         capture_output=True,text=True,check=True)
    lines=[x for x in check.stdout.splitlines() if x.strip()]
    expected={" M "+MOVE," M "+CURSOR," M "+RESIZE}
    if set(lines)!=expected:
        raise RuntimeError("CI_SOCIAL_NATIVE_PATCH_UNEXPECTED_FILES "+repr(lines))
    print("SD_CI_SOCIAL_NATIVE_SOURCE_COPY isolated=1 shared_repo_untouched=1 "
          "modified_only=move_seatop+resize_seatop+seat_cursor result=PASS",flush=True)
    print(dest,flush=True)

if __name__=="__main__":
    main()
