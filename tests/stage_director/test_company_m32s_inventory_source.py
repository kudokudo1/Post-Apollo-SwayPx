#!/usr/bin/env python3
"""M32s: enforce inventory is real read-only source, never an issuer."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
source = (root / "sway/stage_director_company_bundle_inventory.c").read_text()
header = (root / "include/sway/stage_director_company_bundle_inventory.h").read_text()
meson = (root / "sway/meson.build").read_text()
digest = (root / "sway/stage_director_company_digest.c").read_text()
scene = (root / "sway/stage_director_company_scene.c").read_text()
wire = (root / "protocols/post-apollo-company-vector-v1.xml").read_text()

def contract(s):
    required = (
        "stage_director_company_sha256_fd(fd, actual)",
        "O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC",
        "AT_SYMLINK_NOFOLLOW",
        "same_file_metadata(&before, &after)",
        "same_file_metadata(&after, &current)",
        "S_IWGRP | S_IWOTH",
        "st->st_nlink == 1",
        "return executables == 1 && libraries >= 1 && plugins >= 1",
        "SD_COMPANY_BUNDLE_LISTED_BYTES_CANDIDATE_ONLY",
        "listed_bytes_match_candidate =",
    )
    for line in required:
        assert line in s, line
    forbidden = ("execve(", "posix_spawn(", "fork(", "system(",
                 "wlr_scene_node_set_enabled(", ".can_spawn = true",
                 ".can_apply_scene = true", ".operator_authorized = true",
                 ".dependency_closure_verified = true")
    for line in forbidden:
        assert line not in s, line
    assert "#define SD_COMPANY_BUNDLE_MAX_ENTRIES 24" in header
    assert "'stage_director_company_bundle_inventory.c'" in meson
    assert 'company_m30_manifest_elf_sha256[] = ""' in digest
    assert "#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION" in scene
    assert '<request name="launch_intent">' in wire
    assert "stage_director_company_bundle_inventory_probe_fd(" in header

contract(source)
mutations = (
    source.replace("O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC",
                   "O_NONBLOCK | O_CLOEXEC", 1),
    source.replace("return executables == 1 && libraries >= 1 && plugins >= 1",
                   "return executables == 1", 1),
    source.replace("same_file_metadata(&after, &current)",
                   "true", 1),
)
for i, bad in enumerate(mutations):
    assert bad != source, i
    try:
        contract(bad)
    except AssertionError:
        continue
    raise AssertionError("M32s unsafe source mutation passed: " + str(i))
print("SD_COMPANY_M32S_SOURCE reviewed_file_inventory_only=PASS "
      "sha256_reused=YES no_launch_no_issuer=PASS")
