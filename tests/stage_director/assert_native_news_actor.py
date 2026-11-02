#!/usr/bin/env python3
"""CI-only evidence: SwayPX mapped one real Wayland actor bigger than NEWS bay.

Do NOT mistake exact app_id for authenticated WATCH process ownership.
This does not assert pixels were cropped or pointer/popup events contained.
"""
import json
import sys
from pathlib import Path

EXPECTED_ID = "post-apollo-weather-news"
# Only a reference size to demonstrate the actor extends beyond the planned
# NEWS bay. Real accepted viewport must later come from mapped Weather host.
REFERENCE_BAY_W = 824
REFERENCE_BAY_H = 494


def walk(node):
    if isinstance(node, dict):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from walk(child)


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: assert_native_news_actor.py SWAY_GET_TREE_JSON")
    tree = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
    candidates = [n for n in walk(tree) if n.get("app_id") == EXPECTED_ID]
    if len(candidates) != 1:
        raise SystemExit(
            f"expected one mapped native Wayland actor, observed {len(candidates)}"
        )

    node = candidates[0]
    rect = node.get("rect") or {}
    w, h = rect.get("width", 0), rect.get("height", 0)
    # Sway may resize floating windows. Accept only an ACTUALLY mapped
    # oversized rectangle observed via compositor IPC, never requested sizes.
    if not isinstance(w, int) or not isinstance(h, int) or (
        w <= REFERENCE_BAY_W or h <= REFERENCE_BAY_H
    ):
        raise SystemExit(f"mapped actor not oversized: {w}x{h}")
    if not isinstance(node.get("pid"), int) or node["pid"] <= 0:
        raise SystemExit("missing compositor-observed Wayland client PID")
    if node.get("shell") != "xdg_shell":
        raise SystemExit(f"unexpected shell protocol: {node.get('shell')!r}")
    if node.get("window") is not None:
        raise SystemExit("XWayland actor unexpectedly mapped")
    if node.get("name") != "SD1-CI-DISPOSABLE-NOT-BROWSER":
        raise SystemExit("unexpected actor title: no test actor identity")
    print(
        f"Stage Director native NEWS candidate mapped: PASS "
        f"pid={node['pid']} rect={rect.get('x')},{rect.get('y')} "
        f"{w}x{h} shell={node['shell']}"
    )
    print("No trusted WATCH session, paint clip or input containment asserted.")


if __name__ == "__main__":
    main()
