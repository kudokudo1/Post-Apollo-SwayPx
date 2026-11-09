#!/usr/bin/env python3
"""M32r source-contract test: native selected host can request, never approve."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
source = (root / "sway/stage_director_company_vector_protocol.c").read_text()
header = (root / "include/sway/stage_director_company_vector_protocol.h").read_text()
module = (root / "sway/stage_director_company_launch_request.c").read_text()
meson = (root / "sway/meson.build").read_text()
scene = (root / "sway/stage_director_company_scene.c").read_text()

def function(text, signature):
    assert text.count(signature) == 1, signature
    start = text.index("{", text.index(signature) + len(signature))
    depth = 0
    for end in range(start, len(text)):
        if text[end] == "{":
            depth += 1
        if text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start + 1:end]
    raise AssertionError(signature)

def in_order(text, *needles):
    positions = [text.index(x) for x in needles]
    assert positions == sorted(positions), needles

def contract(p):
    native = function(p, "static bool current_launch_request_identity(")
    probe = function(p,
        "stage_director_company_vector_protocol_launch_request_probe_current(")
    viewport = function(p, "static void handle_report_viewport(")
    close = function(p, "static void handle_close(")
    revoke = function(p, "void stage_director_company_vector_protocol_revoke(")
    init = function(p, "void stage_director_company_vector_protocol_init(")
    in_order(native,
        "bound_is_current(selected_client, bound_resource)",
        "stage_director_company_vector_protocol_viewport_snapshot(",
        "birth_native_host_probe(",
        "native.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY",
        "viewport.intent_sequence != binding->intent_sequence",
        ".map_in_host = viewport.map_in_host")
    in_order(viewport,
        "if (bound_is_current(client, resource))",
        "stage_director_company_scene_lease_candidate_revoke(",
        "stage_director_company_launch_request_revoke(",
        "struct company_binding *b")
    in_order(viewport,
        "if (b->viewport_candidate)",
        "current_launch_request_identity(&id)",
        "stage_director_company_launch_request_reserve(",
        "send_status(resource, (uint32_t)candidate.state")
    in_order(close,
        "if (!bound_is_current(client, resource))",
        "stage_director_company_launch_request_revoke(",
        "stage_director_company_scene_revoke(NULL);",
        "b->launch_intent = false;")
    in_order(revoke,
        "stage_director_company_scene_lease_candidate_revoke(",
        "stage_director_company_launch_request_revoke(",
        "stage_director_company_scene_revoke(NULL);",
        "if (bound_resource)")
    assert "stage_director_company_launch_request_init(&native_launch_request)" in init
    assert "current_launch_request_identity(&observed)" in probe
    assert "current ? &observed : NULL, candidate_boot_ns()" in probe
    assert "uint64_t compositor_generation" in header
    assert "'stage_director_company_launch_request.c'" in meson
    assert "#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION" in scene
    # The context bridge may never set a scene or approval right.
    for forbidden in ("stage_director_company_scene_apply_ci_only(",
                      "wlr_scene_node_set_enabled(", ".can_apply_scene = true",
                      ".operator_authenticated = true", ".launch_authenticated = true"):
        assert forbidden not in native + probe + viewport + module, forbidden

contract(source)

# Mutation tests must fail the SAME contract on the real implementation.
bad = [
    source.replace("stage_director_company_launch_request_revoke(\n            &native_launch_request);",
                   "/* missing viewport request revocation */", 1),
    source.replace(".map_in_host = viewport.map_in_host",
                   ".map_in_host = (struct stage_director_rect){0}", 1),
    source.replace("current ? &observed : NULL, candidate_boot_ns()",
                   "&observed, candidate_boot_ns()", 1),
]
for i, text in enumerate(bad):
    assert text != source, i
    try:
        contract(text)
    except (AssertionError, ValueError):
        continue
    raise AssertionError("unsafe request mutation survived: " + str(i))

print("SD_COMPANY_M32R_PROTOCOL native_context=REOBSERVED "
      "viewport_change=REVOKED stale_owner=DENIED "
      "authorization=NONE scene=OFF PASS")
