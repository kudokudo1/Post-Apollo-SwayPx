#!/usr/bin/env python3
"""M32p: native-source candidate bridge contract, with denial mutations.

Static contract checks real compositor callback wiring. No synthetic
operator approval, physical F11 spoofing, or positive scene operation.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
protocol = (root / "sway/stage_director_company_vector_protocol.c").read_text()
header = (root / "include/sway/stage_director_company_vector_protocol.h").read_text()
ledger = (root / "sway/stage_director_company_scene_lease_candidate.c").read_text()
digest = (root / "sway/stage_director_company_digest.c").read_text()
scene = (root / "sway/stage_director_company_scene.c").read_text()

def function(source, signature):
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    brace = source.index("{", start + len(signature))
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if not depth:
                return source[brace + 1:pos]
    raise AssertionError("unclosed function " + signature)

def in_order(source, *fragments):
    positions = []
    for fragment in fragments:
        assert fragment in source, fragment
        positions.append(source.index(fragment))
    assert positions == sorted(positions), (fragments, positions)

def contract(source):
    observe = function(source, "static bool current_native_lease_observation(")
    reserve = function(source,
        "bool stage_director_company_vector_protocol_candidate_reserve_current(")
    probe = function(source,
        "stage_director_company_vector_protocol_candidate_probe_current(")
    close = function(source, "static void handle_close(")
    viewport = function(source, "static void handle_report_viewport(")
    revoke = function(source,
        "void stage_director_company_vector_protocol_revoke(")
    init = function(source, "void stage_director_company_vector_protocol_init(")

    # Source is a compositor-owned mapped sway_view, re-observed as native
    # XDG Wayland resource. No client-provided app_id, PID, title or pointer.
    in_order(observe,
        "stage_director_company_native_observe_actor(view)",
        "stage_director_company_vector_protocol_admission_preflight(",
        "preflight.status != SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT",
        "stage_director_company_vector_protocol_intent_snapshot(",
        "stage_director_company_vector_protocol_viewport_snapshot(",
        "birth_native_host_probe(birth_native_host_context, &host)")
    assert "actor.native_view_ref != (uintptr_t)view" in observe
    for gate in (
        "!preflight.native_pair_candidate",
        "!preflight.current_executable_candidate",
        "!preflight.exact_elf_digest_candidate",
        "preflight.operator_authorized",
        "preflight.authenticated_launch",
        "preflight.scene_lease_issued",
        "preflight.can_apply_scene",
        "preflight.can_accept_input",
        "preflight.can_focus_keyboard",
        "preflight.can_show_popup",
        "intent.selection_epoch != viewport.selection_epoch",
        "intent.sequence != viewport.intent_sequence",
        "intent.host_revision != viewport.host_revision",
        "host.native_client_ref != intent.native_host_client_ref",
        ".actor_start_ticks = actor.start_ticks",
        ".native_actor_root_surface_ref = actor.native_root_surface_ref",
        ".native_output_ref = host.native_output_ref",
        ".exact_digest_candidate = preflight.exact_elf_digest_candidate",
    ):
        assert gate in observe, gate

    # Production digest remains empty, so normal native observation
    # cannot reach the preflight's final candidate-only status today.
    assert 'company_m30_manifest_elf_sha256[] = ""' in digest
    assert "SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT" in source

    in_order(reserve,
        "current_native_lease_observation(",
        "candidate_boot_ns()",
        "stage_director_company_scene_lease_candidate_reserve(")
    assert "SD_COMPANY_LEASE_CANDIDATE_MAX_NS" in reserve
    assert "current_native_lease_observation(" in probe
    assert "current ? &identity : NULL" in probe
    assert "current ? &conditions : NULL" in probe
    assert "candidate_boot_ns()" in probe
    assert "stage_director_company_scene_lease_candidate_probe(" in probe
    assert "clock_gettime(CLOCK_BOOTTIME" in source
    assert "stage_director_company_scene_lease_candidate_init(" in init

    # The current owner alone can invalidate on viewport change. That
    # includes an invalid report, BEFORE any geometry reset. CLOSE and
    # protocol death revoke before releasing role/pidfd references.
    in_order(viewport,
        "if (bound_is_current(client, resource))",
        "stage_director_company_scene_lease_candidate_revoke(",
        "struct company_binding *b = wl_resource_get_user_data(resource)")
    in_order(close, "if (!bound_is_current(client, resource))",
        "stage_director_company_scene_lease_candidate_revoke(",
        "stage_director_company_scene_revoke(NULL);",
        "b->launch_intent = false;")
    in_order(revoke,
        "stage_director_company_scene_lease_candidate_revoke(",
        "stage_director_company_scene_revoke(NULL);",
        "if (bound_resource)",
        "stage_director_company_birth_pidfd_revoke(&birth_held);")
    assert "void stage_director_company_vector_protocol_revoke(void)" in header
    assert "stage_director_company_vector_protocol_candidate_reserve_current(" in header
    assert "stage_director_company_vector_protocol_candidate_probe_current(" in header

    # Neither the bridge nor the ledger may issue any privileges or call
    # the private CI scene setter. Positive scene stays compile-time OFF.
    for forbidden in (
        "wlr_scene_node_set_enabled(", "wl_global_create(",
        "post_apollo_company_vector_v1_send_result(",
        "stage_director_company_scene_apply_ci_only(",
        ".can_apply_scene = true", ".scene_lease_issued = true",
        ".operator_authenticated = true", ".can_accept_input = true"):
        assert forbidden not in observe + reserve + probe + ledger, forbidden
    assert "#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION" in scene

contract(protocol)

# Negative controls must fail this same contract, not a separate mock.
mutations = [
    protocol.replace(
        "preflight.status != SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT",
        "preflight.status == SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT", 1),
    protocol.replace(
        ".exact_digest_candidate = preflight.exact_elf_digest_candidate",
        ".exact_digest_candidate = true", 1),
    protocol.replace(
        "current ? &conditions : NULL, candidate_boot_ns()",
        "&conditions, candidate_boot_ns()", 1),
    protocol.replace(
        "if (bound_is_current(client, resource))",
        "if (!bound_is_current(client, resource))", 1),
    protocol.replace(
        "stage_director_company_scene_lease_candidate_revoke(\n        &native_lease_candidate);\n\t/* Wipe",
        "/* missing M32p revoke */\n\t/* Wipe", 1),
]
for index, variant in enumerate(mutations):
    assert variant != protocol, ("mutation missing", index)
    try:
        contract(variant)
    except (AssertionError, ValueError):
        continue
    raise AssertionError("unsafe native bridge mutation accepted: " + str(index))

print("SD_COMPANY_M32P_NATIVE_SOURCE real_native_reobservation=PASS "
      "pidfd_digest=RECHECKED viewport_and_close=REVOKE "
      "unsafe_mutations=REJECTED operator_issuer=NONE scene=OFF PASS")
