#!/usr/bin/env python3
"""Compare two real CI screencopies across a *virtual-output allocation change*.

This only classifies the overflow. The independent fail-closed screenshot
acceptance test must pass on BOTH frames before the NEWS clip is accepted.
"""
import json
import sys
from pathlib import Path

from assert_news_render_pixels import parse_ppm, patch_average


def actor_rect(path):
    tree = json.loads(Path(path).read_text(encoding="utf-8"))

    def nodes(node):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from nodes(child)

    actors = [
        node for node in nodes(tree)
        if node.get("app_id") == "post-apollo-weather-news"
        and node.get("name") == "SD1-CI-DISPOSABLE-NOT-BROWSER"
        and node.get("shell") == "xdg_shell"
    ]
    if len(actors) != 1:
        raise SystemExit("FAIL: output-reallocation comparison lost unique native actor")
    rect = actors[0].get("rect", {})
    values = [rect.get(k) for k in ("x", "y", "width", "height")]
    if not all(isinstance(x, int) for x in values) or values[2] < 600 or values[3] < 440:
        raise SystemExit("FAIL: output-reallocation comparison has invalid actor rectangle")
    return values


def screenshot(path, tree_path):
    width, height, pixels = parse_ppm(path)
    x, y, _, _ = actor_rect(tree_path)
    return {
        "size": (width, height),
        "inside": patch_average(pixels, width, height, x + 160, y + 160),
        "outside_left": patch_average(pixels, width, height, x + 24, y + 24),
        "outside_right": patch_average(pixels, width, height, x + 450, y + 150),
        "outside_bottom": patch_average(pixels, width, height, x + 160, y + 370),
        "desktop": patch_average(pixels, width, height, 10, 10),
    }


def main():
    if len(sys.argv) != 5:
        raise SystemExit("usage: compare_news_reallocated_pixels.py initial.ppm fresh.ppm initial-tree.json fresh-tree.json")
    original = screenshot(sys.argv[1], sys.argv[3])
    resized = screenshot(sys.argv[2], sys.argv[4])
    if original["size"] != (1920, 1080) or resized["size"] != (1918, 1078):
        raise SystemExit(
            "FAIL: CI virtual output allocation not independently proven: "
            f"original={original['size']} reallocated={resized['size']}"
        )
    if original["desktop"] > 45 or resized["desktop"] > 45:
        raise SystemExit("FAIL: unexpected nonblack reference desktop in CI screenshots")
    if original["inside"] < 180 or resized["inside"] < 180:
        raise SystemExit("FAIL: disposable native actor no longer visible inside crop")

    # This is evidence classification, not acceptance. The caller separately
    # rejects ANY visible outside pixels in either screenshot.
    originally_leaked = max(original["outside_right"], original["outside_bottom"]) > 45
    still_leaks = max(resized["outside_right"], resized["outside_bottom"]) > 45
    if still_leaks:
        state = "OVERFLOW_SURVIVES_FRESH_OUTPUT"
    elif originally_leaked:
        state = "OVERFLOW_CLEARED_ON_FRESH_OUTPUT"
    else:
        state = "NO_OVERFLOW_IN_EITHER_FRAME"

    print(
        "SD_CI_REALLOC_PIXEL "
        f"original={original['size'][0]}x{original['size'][1]} "
        f"fresh={resized['size'][0]}x{resized['size'][1]} "
        f"original_right={original['outside_right']:.1f} "
        f"fresh_right={resized['outside_right']:.1f} "
        f"original_bottom={original['outside_bottom']:.1f} "
        f"fresh_bottom={resized['outside_bottom']:.1f} "
        f"state={state}"
    )
    print("Reallocation classification does not establish actual scene paint origin or input containment.")


if __name__ == "__main__":
    main()
