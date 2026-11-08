#!/usr/bin/env python3
"""M30s: M30o real MapLibre native camera/fullscreen with merged Company M30q.

Reuses the passed M30o *genuine* compositor/input/screenshot proof. Replaces
only the original Company read-only CI source-copy adapter and exact PIN;
never replays M30i's two coordinate fixes. No production scene authority.
"""
from __future__ import annotations

import os
from pathlib import Path
import sys

import ci_company_m30r_real_source as merged
import ci_company_m30o_real_resize as native


def main() -> None:
    env=os.environ
    if not (env.get("CI") == "true" and
            env.get("GITHUB_ACTIONS") == "true" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30S") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30O") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30R") is None and
            Path(env.get("RUNNER_TEMP", "/missing")) ==
                Path("/home/runner/work/_temp")):
        raise RuntimeError("BLOCKED_M30S_NOT_EXACT_DISPOSABLE_CI")
    # M30r's existing Company source verifier is allowed only in private CI.
    # The M30r flag is set INSIDE the Python process, after the shell's
    # exclusive M30s dispatch, to reuse its exact immutable blob/commit
    # checks. No separate M30r runner is launched or altered on disk.
    native.real.PIN=merged.COMPANY_MERGED_SHA
    native.real.prepare=merged.instrument_without_geometry_repair
    env["STAGE_DIRECTOR_CI_COMPANY_M30R"]="1"
    try:
        native.main()
    finally:
        env.pop("STAGE_DIRECTOR_CI_COMPANY_M30R",None)

    source_full_width = (1494 if
        env.get("STAGE_DIRECTOR_CI_COMPANY_M30T")=="1" else 1493)
    print("SD_COMPANY_M30S_MERGED_SOURCE_AND_NATIVE_MAPLIBRE "
          "company_original_merged_m30q=1 geometry_copy_patch=OFF "
          "real_Quickshell_and_MapLibre_in_same_Wayland=1 "
          f"source_840_to_{source_full_width}_to_840=PASS "
          "genuine_geography_and_fullscreen_input=PASS "
          "camera_and_native_pidfd_xdg_preserved=PASS "
          "production_scene_input=OFF PASS",flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30S_BLOCKED:",str(exc),file=sys.stderr,flush=True)
        raise SystemExit(1)
