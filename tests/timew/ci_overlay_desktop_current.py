"""Attach a diagnostic to GET_TREE in disposable GitHub CI only."""
import os
from pathlib import Path

if (os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true"
        or not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/")):
    raise SystemExit("BLOCKED_NON_EPHEMERAL_CAPTURE")

root = Path(__file__).resolve().parents[2]
ipc = root / "sway/ipc-server.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
source = (root / "tests/timew/ci_desktop_current_probe.c").read_text()

def once(s, old, new):
    if s.count(old) != 1:
        raise SystemExit("BLOCKED_CAPTURE_ANCHOR_DRIFT: " + old)
    return s.replace(old,new,1)

i = once(ipc.read_text(), "static int ipc_socket = -1;",
         source + "\nstatic int ipc_socket = -1;")
i = once(i, "json_object *tree = ipc_json_describe_node_recursive(&root->node);",
         'json_object *tree = ipc_json_describe_node_recursive(&root->node);\n'
         '\t\tjson_object_object_add(tree, "timew_ci_current", ci_capture_current());')
s = once(smoke.read_text(),
         '  bash tests/stage_director/ci_native_news_actor_smoke.sh \\\n    "$runtime" "${socket##*/}"',
         '  bash tests/timew/ci_desktop_current_smoke.sh "$runtime" "${socket##*/}"')
s = once(s, 'for_window [app_id="post-apollo-weather-news"] floating enable',
         'for_window [app_id="timew-capture-fixture" title="TIMEW-FLOAT"] floating enable')
# Multi-output mode uses fixed headless outputs from boot, not live
# resolution changes. Configure placement before any actor maps.
s = once(s, 'export WLR_HEADLESS_OUTPUTS=1',
    'export WLR_HEADLESS_OUTPUTS=1\n'
    'if [[ "${TIMEW_CI_MULTI_CAPTURE:-0}" == 1 ]]; then\n'
    '  export WLR_HEADLESS_OUTPUTS=2\n'
    '  cat >> "$config" <<\'EOF\'\n'
    'output HEADLESS-1 position 0 0\n'
    'output HEADLESS-2 position 1920 0\n'
    'workspace 1 output HEADLESS-1\n'
    'workspace 2 output HEADLESS-2\n'
    'for_window [app_id="timew-capture-fixture" title="TIMEW-TILE"] move container to workspace 1\n'
    'for_window [app_id="timew-capture-fixture" title="TIMEW-FLOAT"] move container to workspace 2\n'
    'EOF\nfi')
# Both targets are verified before writing the temporary checkout.
ipc.write_text(i)
smoke.write_text(s)
print("TIMEW_CI_DESKTOP_CURRENT_OVERLAY PASS authority=NONE")
