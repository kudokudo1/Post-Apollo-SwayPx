#!/usr/bin/env python3
"""M32n: static fail-closed Wayland protocol teardown contract.

This checks the real registered request/resource callbacks, not a surrogate
mock protocol or a fake physical M24 libinput selection. Native Wayland
CLOSE/destroy acceptance still requires a genuinely paired Company client.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (ROOT / "sway/stage_director_company_vector_protocol.c").read_text()
SCENE = (ROOT / "sway/stage_director_company_scene.c").read_text()


def function(source: str, signature: str) -> str:
    """Return the braces-balanced C function, with unique anchor."""
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    brace = source.index("{", start + len(signature))
    level = 0
    for i in range(brace, len(source)):
        if source[i] == "{":
            level += 1
        elif source[i] == "}":
            level -= 1
            if level == 0:
                return source[brace + 1 : i]
    raise AssertionError("unterminated function: " + signature)


def ordered(source: str, *parts: str):
    offsets = []
    for part in parts:
        assert part in source, "missing " + part
        offsets.append(source.index(part))
    assert offsets == sorted(offsets), ("order", offsets, parts)


def contract(proto: str):
    close = function(proto, "static void handle_close(")
    destroy = function(proto, "static void handle_destroy(")
    release = function(proto, "static void destroy_binding(")
    selected_role_loss = function(proto, "static void native_resource_destroyed(")
    revoke = function(proto, "void stage_director_company_vector_protocol_revoke(")
    finish = function(proto, "void stage_director_company_vector_protocol_finish(")
    bind = function(proto, "static void bind_company(")
    impl = proto.split("static const struct post_apollo_company_vector_v1_interface company_impl = {", 1)[1].split("};", 1)[0]

    # A foreign/stale CLOSE cannot revoke the selected client. A valid
    # CLOSE must hide the real mapped scene before any intent changes.
    ordered(close, "if (!bound_is_current(client, resource))",
            "stage_director_company_scene_revoke(NULL);",
            "b->launch_intent = false;",
            "send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_CLOSED, 0);",
            "stage_director_company_vector_protocol_revoke();")
    assert "return;" in close.split("stage_director_company_scene_revoke(NULL);", 1)[0]

    # Explicit DESTROY and client disconnect reach this SAME registered
    # Wayland resource destructor. Never free the binding first; never
    # let a stale binding revoke a different current client.
    assert ".close = handle_close" in impl
    assert ".destroy = handle_destroy" in impl
    assert "wl_resource_destroy(resource);" in destroy
    assert "wl_resource_set_implementation(resource, &company_impl, b, destroy_binding);" in bind
    ordered(release, "if (bound_resource == resource) {",
            "stage_director_company_vector_protocol_revoke();",
            "free(wl_resource_get_user_data(resource));")
    assert release.index("stage_director_company_vector_protocol_revoke();") < release.index("}\n")
    assert "stage_director_company_vector_protocol_revoke();" in selected_role_loss

    # Clearing wl_resource identity and the kernel-held actor pidfd before
    # hiding its scene is forbidden. This is the actual protocol revoke.
    ordered(revoke, "stage_director_company_scene_revoke(NULL);",
            "if (bound_resource)",
            "wl_global_destroy(company_global);",
            "native_role = NULL;",
            "selected_client = NULL;",
            "stage_director_company_birth_pidfd_revoke(&birth_held);",
            "bound_resource = NULL;")
    assert "stage_director_company_vector_protocol_revoke();" in finish

    # Native scene revoke is one-way and quarantines any still-mapped
    # Qt actor; this source assertion does not fake a positive scene.
    scene_revoke = function(SCENE, "void stage_director_company_scene_revoke(")
    ordered(scene_revoke, "ci_actor_quarantine(actor);",
            "wlr_scene_node_set_enabled(node, false);",
            "ci_scene.view = NULL;")


contract(PROTOCOL)

# Negative controls demonstrate that this test actually rejects the
# dangerous regressions, rather than merely detecting function names.
mutations = [
    PROTOCOL.replace(
        "stage_director_company_scene_revoke(NULL);\n\tstruct company_binding *b",
        "/* M32n regression: scene revoke missing */\n\tstruct company_binding *b", 1),
    PROTOCOL.replace(
        "if (bound_resource == resource) {\n\t\tstage_director_company_vector_protocol_revoke();",
        "if (bound_resource != resource) {\n\t\tstage_director_company_vector_protocol_revoke();", 1),
    PROTOCOL.replace(
        "stage_director_company_vector_protocol_revoke();\n\t}\n\tfree(wl_resource_get_user_data(resource));",
        "/* missing revocation */\n\t}\n\tfree(wl_resource_get_user_data(resource));", 1),
]
for index, mutated in enumerate(mutations):
    assert mutated != PROTOCOL, ("mutation did not apply", index)
    try:
        contract(mutated)
    except (AssertionError, ValueError):
        continue
    raise AssertionError("unsafe teardown mutation accepted: " + str(index))

print("SD_COMPANY_M32N_TEARDOWN explicit_close=SCENE_FIRST "
      "resource_destroy=CURRENT_BINDING_ONLY "
      "native_role_destroy=REVOKE pidfd_order=SCENE_FIRST "
      "unsafe_mutations=REJECTED authority=NONE PASS")
