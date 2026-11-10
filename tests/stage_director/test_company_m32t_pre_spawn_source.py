#!/usr/bin/env python3
"""M32t source-contract with negative mutations; no runtime issuer."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
p=(root/"sway/stage_director_company_pre_spawn_custody.c").read_text()
h=(root/"include/sway/stage_director_company_pre_spawn_custody.h").read_text()
m=(root/"sway/meson.build").read_text()
s=(root/"sway/stage_director_company_scene.c").read_text()
d=(root/"sway/stage_director_company_digest.c").read_text()
wire=(root/"protocols/post-apollo-company-vector-v1.xml").read_text()

def check(text):
    requirements=(
        "stage_director_company_launch_request_probe(",
        "stage_director_company_bundle_inventory_probe_fd(",
        "F_DUPFD_CLOEXEC",
        "O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC",
        "same_metadata(&exe_meta, &path_exe)",
        "same_metadata(&executable, &current_path)",
        "same_metadata(&c->executable_at_capture, &executable)",
        "listed_image_current(c)",
        "stage_director_company_pre_spawn_revoke(c, generation)",
        "snapshot.manifest[i].sha256_hex",
        "if (snapshot.expires_ns > request->expires_ns)",
        "SD_COMPANY_PRE_SPAWN_LISTED_BYTES_CANDIDATE_ONLY",
    )
    for t in requirements:
        assert t in text,t
    forb=("fexecve(", "execve(", "execveat(", "posix_spawn(",
          "system(", "fork(", "vfork(", ".can_spawn = true",
          ".can_apply_scene = true", ".operator_authenticated = true")
    for t in forb:
        assert t not in text,t
    assert "'stage_director_company_pre_spawn_custody.c'" in m
    assert "bool can_spawn;" in h
    assert 'company_m30_manifest_elf_sha256[] = ""' in d
    assert '#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION' in s
    assert wire.count('<request name="launch_intent">')==1

check(p)
for i,(before,after) in enumerate((
    ("F_DUPFD_CLOEXEC","F_DUPFD",),
    ("same_metadata(&exe_meta, &path_exe)","true"),
    ("same_metadata(&executable, &current_path)","true"),
    ("listed_image_current(c)","true"),
    ("if (snapshot.expires_ns > request->expires_ns)",
     "if (snapshot.expires_ns > UINT64_MAX)"),
)):
    bad=p.replace(before,after,1)
    assert bad!=p,i
    try: check(bad)
    except AssertionError: continue
    raise AssertionError("unsafe M32t mutation survived: "+str(i))
print("SD_COMPANY_M32T_SOURCE held_inode=RECHECKED "
      "request_generation=BOUND listed_image=REVALIDATED "
      "spawn=NONE authority=NONE PASS")
