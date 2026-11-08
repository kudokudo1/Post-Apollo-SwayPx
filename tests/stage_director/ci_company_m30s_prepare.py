#!/usr/bin/env python3
"""Prepare M30o actual MapLibre Qt source from immutable MERGED Company.

This edits only a GitHub Actions disposable COPY. All native renderer bytes
match the already reviewed M30l source, while Company QML comes from
M30q's merged commit and is never patched back to old geometry.
"""
from __future__ import annotations
import os
from pathlib import Path
import subprocess
import sys

import ci_company_m30r_real_source as merged
import ci_company_m30l_prepare as source
import ci_company_m30o_prepare as observer

NATIVE_BLOBS = {
    "experiments/company-vector-qt/main.qml":
        "1276c045fdf1060b83c0333d1af38333ec17bcaa",
    "experiments/company-vector-qt/main.cpp":
        "fc2d41634dedb94dd0dcdadfdc8fc49e682b9be7",
    "experiments/company-vector-qt/CMakeLists.txt":
        "d8541cc2a3bf1e9a740a44b45dfc4392f0d7e81a",
    "experiments/company-vector-qt/style-fixture.json":
        "88630fa511b34f3c4249df158e67ffeb1674ce66",
    "experiments/company-vector-qt/fixture-hubs.json":
        "dd57c38d14c43eb7f2ba55a422203c8ee472abdf",
}


def main() -> None:
    env=os.environ
    if not (env.get("CI") == "true" and
            env.get("GITHUB_ACTIONS") == "true" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30S") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30O") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30N") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30L") == "1" and
            Path(env.get("RUNNER_TEMP","/missing")) ==
                Path("/home/runner/work/_temp")):
        raise RuntimeError("BLOCKED_M30S_SOURCE_PREP_OUTSIDE_PRIVATE_CI")
    checkout=Path(env.get("STAGE_DIRECTOR_COMPANY_PINNED_SOURCE","/missing"))
    if not checkout.is_dir() or not (checkout/".git").is_dir():
        raise RuntimeError("BLOCKED_M30S_COMPANY_GIT_CHECKOUT_MISSING")
    def git(*args: str) -> str:
        result=subprocess.run(["git","-C",str(checkout),*args],
            capture_output=True,text=True,check=False,timeout=10)
        if result.returncode:
            raise RuntimeError("M30S_GIT_SOURCE_PROBE_FAILED")
        return result.stdout.strip()
    if git("rev-parse","HEAD")!=merged.COMPANY_MERGED_SHA or (
            git("status","--porcelain")):
        raise RuntimeError("BLOCKED_M30S_NOT_EXACT_CLEAN_MERGED_COMPANY")
    for path, sha in NATIVE_BLOBS.items():
        if git("hash-object",path)!=sha:
            raise RuntimeError("M30S_NATIVE_MAPLIBRE_SOURCE_DRIFT_"+
                               path.replace("/","_"))
    for path,sha in merged.EXPECTED_QML_BLOBS.items():
        if git("hash-object",path)!=sha:
            raise RuntimeError("M30S_QML_GEOMETRY_SOURCE_DRIFT_"+
                               path.replace("/","_"))
    source.PIN=merged.COMPANY_MERGED_SHA
    # Existing M30n/M30o observers still check full private CI permissions
    # and only instrument real MapLibre QML/C++ in RUNNER_TEMP. Nothing in
    # this wrapper can set production executable manifest or scene rights.
    observer.main()
    print("SD_COMPANY_M30S_MERGED_NATIVE_SOURCE "
          "original_company_sha="+merged.COMPANY_MERGED_SHA+
          " merged_qml_blobs_verified=1 "
          "maplibre_native_blobs_identical_to_m30l=1 "
          "source_checkout_unchanged=1 no_geometry_copy_patch=1 PASS",
          flush=True)


if __name__=="__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30S_PREPARE_BLOCKED:",str(exc),
              file=sys.stderr,flush=True)
        raise SystemExit(1)
