"""Validate CI-only same-response topology coverage, not M12a wire format."""
import json
import math
import re
import struct
import sys

HEX = re.compile(r"[0-9a-f]{16}\Z")
def validate(tree):
    capture = tree.get("timew_ci_current")
    if not isinstance(capture,dict) or set(capture) != {"status","rows","authority"} or capture["status"] != "CURRENT_DIAGNOSTIC" or capture["authority"] != "NONE":
        raise ValueError("capture_not_qualified")
    expected = {}
    def visit(node, out=None, ws=None, floating=False):
        typ = node.get("type")
        if typ == "output":
            if node.get("name") == "__i3":
                # Scratchpad must be empty; source refuses all scratchpad.
                if any(child.get("nodes") or child.get("floating_nodes") for child in node.get("nodes", [])):
                    raise ValueError("scratchpad_not_empty")
                return
            out = node["id"]
        elif typ == "workspace":
            ws = node["id"]
        if node.get("app_id") is not None or node.get("window") is not None:
            if node.get("app_id") != "timew-capture-fixture" or out is None or ws is None or node["id"] in expected:
                raise ValueError("unexpected_or_duplicate_view")
            expected[node["id"]] = (out,ws,floating)
        for child in node.get("nodes",[]):
            visit(child,out,ws,floating)
        for child in node.get("floating_nodes",[]):
            visit(child,out,ws,True)
    visit(tree)
    if len(expected) != 2 or {x[2] for x in expected.values()} != {False,True}:
        raise ValueError("missing_tiled_or_floating_fixture")
    seen = set()
    fractional = 0
    for row in capture["rows"]:
        if set(row) != {"con_id","workspace_node","output_node","floating","current_bits"}:
            raise ValueError("wrong_fields")
        cid = row["con_id"]
        if type(cid) is not int or cid in seen or cid not in expected or type(row["floating"]) is not bool or (row["output_node"],row["workspace_node"],row["floating"]) != expected[cid]:
            raise ValueError("wrong_coverage_or_context")
        seen.add(cid)
        bits = row["current_bits"]
        if len(bits) != 4 or any(type(x) is not str or not HEX.fullmatch(x) for x in bits):
            raise ValueError("bad_bits")
        values = [struct.unpack(">d",bytes.fromhex(x))[0] for x in bits]
        if any(not math.isfinite(x) for x in values) or min(values[2:]) <= 0:
            raise ValueError("bad_geometry")
        fractional += sum(not x.is_integer() for x in values)
    if seen != set(expected):
        raise ValueError("missing_current_rows")
    return fractional

if __name__ == "__main__":
    try:
        with open(sys.argv[1],encoding="utf-8") as f:
            n = validate(json.load(f))
        print(f"TIMEW_CI_DESKTOP_CURRENT_PASS views=2 tiled=1 floating=1 same_app=1 fractional_components={n} epoch=UNVERIFIED revision=UNVERIFIED authority=NONE")
    except (ValueError,TypeError,KeyError,AttributeError,IndexError) as e:
        print(str(e),file=sys.stderr)
        sys.exit(1)
