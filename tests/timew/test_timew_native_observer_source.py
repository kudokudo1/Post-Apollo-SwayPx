#!/usr/bin/env python3
"""Source-contract smoke for the dormant SwayPX TimeW native observer.

This is not a physical input provenance, surface lifetime, or authorization
test. The separate CI job compiles the observer against actual pinned SwayPX
and wlroots headers inside a disposable Nix build without linking it in the
production repository.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "include/sway/timew_scope_a_native_observer.h"
SOURCE = ROOT / "sway/timew_scope_a_native_observer.c"
MESON = ROOT / "sway/meson.build"


def main() -> None:
    header = HEADER.read_text()
    source = SOURCE.read_text()
    meson = MESON.read_text()
    assert "timew_scope_a_native_observer.c" not in meson
    assert "timew_scope_a_native_observer.h" not in meson

    required = {
        "real compositor hit-test": "node_at_coords(seat,",
        "session lock": "server.session_lock.lock",
        "live XDG view": "view->type != SWAY_VIEW_XDG_SHELL",
        "strict XDG root": "root != view->surface",
        "native surface mapping": "!view->surface->mapped",
        "pointer target": "wlr_surface_get_root_surface(hit)",
        "floating policy": "!container_is_floating(con)",
        "not fullscreen": "con->pending.fullscreen_mode != FULLSCREEN_NONE",
        "not scratchpad": "con->scratchpad",
        "not nested group": "con->pending.parent != NULL",
        "no destroyed node": "node->destroying",
        "client from kernel-owned resource": "wl_resource_get_client(root->resource)",
        "same live view container": "view->container != con",
        "real output": "!ws->output->enabled",
        "current x": "result.x = con->pending.x",
        "current y": "result.y = con->pending.y",
        "current width": "result.width = con->pending.width",
        "current height": "result.height = con->pending.height",
        "no fill-in epoch": "A protected future caller",
    }
    for label, text in required.items():
        assert text in source, f"Missing source gate: {label}"

    assert "struct timew_scope_a_native_candidate result = {0};" in source
    assert "result.candidate_observed = true;" in source
    # No positive privileged state can be created by this observer.
    positive = (
        "operator_selection_authenticated", "c3e_producer_authenticated",
        "c5_spend_verified", "native_resize_allowed", "native_move_allowed",
        "protected_receipt_verified",
    )
    for field in positive:
        assert f"bool {field};" in header
        assert not re.search(rf"result\.{field}\s*=", source), field

    side_effect_calls = (
        "container_floating_move_to(", "resize_set_floating(",
        "transaction_commit_dirty(", "arrange_container(",
        "ipc_send_reply(", "wl_signal_add(",
        "seat_set_focus(", "sway_cursor_warp(",
    )
    for call in side_effect_calls:
        assert call not in source, f"Unexpected side-effect call: {call}"

    print(
        "TIMEW_SCOPE_A_NATIVE_OBSERVER_SOURCE_ONLY PASS: live XDG root, "
        "seat/lock/pointer/floating/output/rectangle guards present; "
        "native authority NONE, runtime binding NONE"
    )


if __name__ == "__main__":
    main()
