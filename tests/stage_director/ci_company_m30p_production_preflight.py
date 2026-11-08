#!/usr/bin/env python3
"""M30p: source-grounded Company/Stage Director production admission preflight.

This is a read-only *source* contract, NOT a production authorization.
No active compositor/Qt/Quickshell process is launched or inspected, no
user credentials/locations are read, and no file or system is modified.
Run with --company pointing at the reviewed immutable Company checkout.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

COMPANY_SOURCE_COMMIT = "e3595b0035bb3d89aa87c6ba2bd1aac04a975181"
EXPECTED_BLOBS = {
    "widgets/CompanyNetworkW.qml":
        "6405119aaf24bf67c1a6ce6d800de70a97ca10ea",
    "widgets/CompanyCoveragePane.qml":
        "87ba60e13c6ab7461cc693b32451cd24923e6767",
    "widgets/CompanyVectorBridgeLink.qml":
        "977f14eae33acef5a3088d31a6bb4b7857f9be68",
    "services/company/CompanyVectorSidecarController.qml":
        "dcc97046f1f8fdda9ea740f8641d2ed51360ef30",
}


class SourceDrift(Exception):
    pass


def require(test: bool, token: str) -> None:
    if not test:
        raise SourceDrift("M30P_SOURCE_CONTRACT_DRIFT_" + token)


def read(root: Path, path: str) -> str:
    file = root / path
    require(file.is_file() and not file.is_symlink(), "MISSING_" +
            path.replace("/", "_"))
    require(file.resolve().is_relative_to(root.resolve()),
            "OUTSIDE_" + path.replace("/", "_"))
    return file.read_text(encoding="utf-8")


def git(repo: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(repo), *args],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        check=False, timeout=8)
    require(result.returncode == 0, "GIT_" + "_".join(args).replace("/", "_"))
    return result.stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--company", type=Path, required=True)
    parser.add_argument("--sway", type=Path, default=Path("."))
    parser.add_argument("--require-production-ready", action="store_true",
                        help="Explicitly fail closed unless fully authorized; "
                             "this version intentionally cannot approve production")
    args = parser.parse_args()

    company, sway = args.company.resolve(), args.sway.resolve()
    require(company.is_dir() and sway.is_dir(), "INPUT_DIRECTORY")
    sha = git(company, "rev-parse", "HEAD")
    require(sha == COMPANY_SOURCE_COMMIT, "PINNED_COMPANY_COMMIT")

    # Immutable source attestations, not operational authorizations. A
    # changed original Company tree needs a new owner review, not guessing
    # whether a formerly successful disposable-CI patch still applies.
    for file, expected in EXPECTED_BLOBS.items():
        require(git(company, "hash-object", file) == expected,
                "COMPANY_BLOB_" + file.replace("/", "_"))
    require(not git(company, "status", "--porcelain", "--untracked-files=no"),
            "COMPANY_DIRTY")

    network = read(company, "widgets/CompanyNetworkW.qml")
    coverage = read(company, "widgets/CompanyCoveragePane.qml")
    bridge = read(company, "widgets/CompanyVectorBridgeLink.qml")
    controller = read(
        company, "services/company/CompanyVectorSidecarController.qml")
    policy = read(sway, "sway/stage_director.c")
    pairing = read(sway, "sway/stage_director_company_pairing.c")
    protocol = read(sway, "sway/stage_director_company_vector_protocol.c")
    digest = read(sway, "sway/stage_director_company_digest.c")
    ci_actor = read(sway, "sway/stage_director_ci_company_crop.c")
    ci_geometry = read(sway, "tests/stage_director/ci_company_real_quickshell.py")
    ci_resize = read(sway, "tests/stage_director/ci_company_m30o_real_resize.py")

    # These checks assert only that previously reviewed source is STILL
    # fail-closed. The user/operator must approve a future design that
    # replaces each hard-blocking condition.
    require('property bool vectorBridgeOptIn: false' in network,
            "COMPANY_BRIDGE_NO_LONGER_DEFAULT_OFF")
    require('bridgeEnabled: root.vectorBridgeOptIn' in network,
            "COMPANY_BRIDGE_NOT_BOUND_TO_EXPLICIT_OPTIN")
    require('import PostApollo.CompanyVector 1.0' in bridge and
            "Qt.createQmlObject" in bridge and
            "Component.onCompleted: if (bridgeEnabled) attachBridge()" in bridge,
            "LAZY_OPTIONAL_COMPANY_BRIDGE")
    require('vectorRunner.command = ["/usr/bin/python3"' in controller and
            'Quickshell.shellPath("services/company/CompanyVectorSidecarLaunch.py")'
            in controller and
            'signal nativeChildStarted(int pid)' in controller,
            "ATTACHED_ORIGINAL_COMPANY_LAUNCHER")
    require('onNativeVectorChildStarted(pid)' in network and
            'vectorBridgeLink.reportChildPid(pid)' in network and
            'nativeVectorLaunchRequested(bool online)' in coverage,
            "CHILD_PID_BRIDGE_PIPELINE")
    require('WlrLayershell.namespace: "post-apollo-company-network"' in network,
            "COMPANY_NATIVE_LAYER_NAMESPACE")
    require('const origin = mapCanvas.mapToItem(root, 0, 0);' in coverage,
            "COVERAGE_SOURCE_ALREADY_PATCHED_REVIEW_REQUIRED")
    require('const origin = companyFrame.mapToItem(root, 0, 0);' in network,
            "NETWORK_SOURCE_ALREADY_PATCHED_REVIEW_REQUIRED")
    require('const origin = companyFrame.mapToItem(null, 0, 0);' in ci_geometry and
            'column.x + mapRow.x + mapSurface.x + mapCanvas.x' in ci_geometry and
            'original_company_checkout=UNCHANGED' in ci_geometry,
            "CI_ONLY_QML_GEOMETRY_WITNESS")

    require('const char *flag = getenv("POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING");'
            in pairing and
            "physical_libinput(device, WLR_INPUT_DEVICE_POINTER)" in pairing and
            "geometry=ONLY scene=OFF input=OFF" in pairing,
            "PHYSICAL_HOST_SELECTION_CANDIDATE_ONLY")
    require('post_apollo_company_vector_v1_interface, 2, NULL, bind_company'
            in protocol and
            "stage_director_company_birth_pidfd_match_actor" in protocol and
            "stage_director_company_digest_check_system_home" in protocol,
            "V2_BIRTH_AND_DIGEST_PIPELINE")
    require('static const char company_m30_manifest_elf_sha256[] = "";'
            in digest and "SD_COMPANY_DIGEST_UNPINNED" in digest,
            "UNAPPROVED_DIGEST_CANDIDATE")
    require("owner_verified=no scene=OFF input=OFF" in policy and
            "scene=OFF input=OFF" in policy,
            "PRODUCTION_ACTOR_MUST_REMAIN_OFF")
    require("STAGE_DIRECTOR_CI_COMPANY_M30O" not in policy and
            'getenv("STAGE_DIRECTOR_CI_COMPANY_M30O")' in ci_actor and
            "ci_company_m30o_enabled" in ci_actor,
            "CI_ONLY_COMPANY_RENDERER_ISOLATION")
    require('SD_COMPANY_M30O_REAL_FULLSCREEN_RESIZE' in ci_resize and
            'wait_real_size(qt_log,(1493,788)' in ci_resize,
            "EXACT_FULL_RESIZE_TEST_PROVENANCE")

    checks = [
        {"id": "company-source-pinned", "status": "SOURCE_VERIFIED",
         "evidence": COMPANY_SOURCE_COMMIT + " and exact reviewed QML blobs"},
        {"id": "attached-company-sidecar", "status": "SOURCE_VERIFIED",
         "evidence": "CompanyVectorSidecarController Quickshell Process; nativeChildStarted PID"},
        {"id": "native-host-selection", "status": "CANDIDATE_ONLY",
         "evidence": "F11 physical libinput selection, explicit opt-in, no scene/input"},
        {"id": "v2-pidfd-inode-digest-observation", "status": "CANDIDATE_ONLY",
         "evidence": "kernel PIDFD + actual executable inode + digest reader; no render authority"},
        {"id": "m30o-fullscreen", "status": "CI_VERIFIED_ONLY",
         "evidence": "genuine full-size MapLibre in private CI compositor, not production"},
    ]
    blockers = [
        {"id": "company-coverage-coordinate-repair", "owner": "Company QML",
         "state": "NOT_LANDED",
         "source": "CompanyCoveragePane mapCanvas.mapToItem(root); CI-only guarded ancestor sum"},
        {"id": "company-panel-coordinate-repair", "owner": "Company QML",
         "state": "NOT_LANDED",
         "source": "CompanyNetworkW companyFrame.mapToItem(PanelWindow); CI-only null"},
        {"id": "approved-native-executable-hash", "owner": "operator + SwayPX",
         "state": "UNPINNED",
         "source": "stage_director_company_digest.c empty approved ELF manifest"},
        {"id": "selected-native-actor-render-input-authority",
         "owner": "Stage Director/SwayPX",
         "state": "NOT_IMPLEMENTED",
         "source": "production scene=OFF input=OFF; M30o actor scene compiled CI-only"},
        {"id": "desktop-operator-acceptance", "owner": "operator",
         "state": "NOT_VERIFIED",
         "source": "no live Fedora installation/multidisplay/focus/keyboard acceptance"},
    ]
    status = {
        "schema": "post-apollo.company-map.production-readiness-source-v1",
        "patient": "kudokudo1/Post-Apollo-SwayPx",
        "company_source_sha": sha,
        "scope": "SOURCE_ONLY_NO_RUNTIME_OR_OPERATOR_AUTHORITY",
        "source_contract_verified": True,
        "production_ready": False,
        "production_activation": "REFUSED",
        "checks": checks,
        "blockers": blockers,
    }
    print(json.dumps(status, sort_keys=True, indent=2))
    return 3 if args.require_production_ready else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (SourceDrift, OSError, UnicodeError, subprocess.TimeoutExpired) as exc:
        print("M30P_SOURCE_CONTRACT_REFUSED: " + str(exc), file=sys.stderr)
        raise SystemExit(2)
