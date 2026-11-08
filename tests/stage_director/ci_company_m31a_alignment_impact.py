#!/usr/bin/env python3
"""Company M31a: compare pinned SceneFX baseline against a CI-only fix.

All source/final images come from real independent headless Wayland sessions.
A corrected final image alone cannot pass: baseline MUST reproduce the
negative -1px/row displacement, both Qt sources and the 1494px reference
MUST be straight, and the patched 1493px image MUST have zero row shear.
Does not certify production or physical-GPU behavior.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

from ci_company_m30t_pixel_geometry import read_ppm
from ci_company_m30z_pixel_origin import wrapped_bar_trace

SCHEMA = "post-apollo.swaypx.company-scenefx-unpack-ci-m31a.v1"
ORIGINAL = (1493, 788)
OUTPUT = (1920, 1080)
ORIGIN = (166, 203)


def _trace(path: Path, dims: tuple[int, int], offset, label: str):
    return wrapped_bar_trace(read_ppm(path, dims), offset, label)


def _verify_meta(path: Path, label: str) -> dict:
    x = json.loads(path.read_text())
    if (x.get("status") != "OBSERVED_ORIGINAL_IMPORT_CI_ONLY" or
            x.get("kind") != "SHM" or
            x.get("fourccHex") != "0x36314752" or
            x.get("physicalWidth") != 1493 or
            x.get("physicalHeight") != 788 or
            x.get("actualPlane0PitchBytes") != 2986 or
            x.get("observedRowPaddingBytes") != 0 or
            x.get("nativeSurfaceScale") != 1 or
            x.get("nativeSurfaceTransform") != 0 or
            x.get("productionSceneAuthority") is not False or
            x.get("productionInputAuthority") is not False):
        raise RuntimeError("M31A_"+label+"_NATIVE_METADATA_INVALID")
    return x


def assess(control_source: dict, control_final: dict,
           patched_source: dict, patched_final: dict) -> dict:
    for label, data in (("CONTROL_SOURCE", control_source),
                        ("PATCHED_SOURCE", patched_source)):
        if (abs(data.get("signedHorizontalShiftPerSourceRowPx", 999)) > .02 or
                abs(data.get("shiftInterceptPx", 999)) > 1 or
                data.get("maximumFitResidualPx", 999) > .5):
            raise RuntimeError("M31A_"+label+"_NOT_STRAIGHT")
    control_slope = control_final.get("signedHorizontalShiftPerSourceRowPx")
    if (type(control_slope) not in (float, int) or
            abs(control_slope + 1.0) > .02 or
            control_final.get("maximumFitResidualPx", 999) > .5 or
            control_final.get("rowsWithWrappedBars", 0) < 1):
        raise RuntimeError("M31A_NEGATIVE_CONTROL_NOT_REPRODUCED")
    fixed_slope = patched_final.get("signedHorizontalShiftPerSourceRowPx")
    if (type(fixed_slope) not in (float, int) or
            abs(fixed_slope) > .02 or
            abs(patched_final.get("shiftInterceptPx", 999)) > 1 or
            patched_final.get("maximumFitResidualPx", 999) > .5 or
            patched_final.get("measuredRows") != 14):
        raise RuntimeError("M31A_PATCHED_ORIGINAL_STILL_HAS_ROW_SHEAR")

    return {
        "schema": SCHEMA,
        "status": "ISOLATED_CI_PATCH_CORRECTED_ORIGINAL_ROW_SHEAR",
        "controlQtSourceStraight": True,
        "controlCompositorShiftPxPerRow": control_slope,
        "patchedQtSourceStraight": True,
        "patchedCompositorShiftPxPerRow": fixed_slope,
        "measuredRows": patched_final["measuredRows"],
        "nativeFormat": "SHM_RGB565",
        "nativeOriginalWidth": 1493,
        "nativeOriginalStrideBytes": 2986,
        "actualNativeRowPaddingBytes": 0,
        "alignmentHypothesisSupportedByControlledExperiment": True,
        "patchVariant": "CI_ONLY_NIX_PINNED_SCENEFX_SAVE_SET1_RESTORE",
        "patchedProductionRenderer": False,
        "userFedoraHardwareVerified": False,
        "selectedHostProductionAuthorized": False,
        "sceneProductionAuthorized": False,
        "inputProductionAuthorized": False,
        "upstreamOwnerReviewComplete": False,
    }


def analyze(args: list[str]) -> int:
    if len(args) != 7:
        raise RuntimeError("M31A_REQUIRES_FOUR_PPMS_TWO_METADATA_ONE_REPORT")
    bsrc, bfinal, psrc, pfinal, bmeta, pmeta, report = map(Path,args)
    _verify_meta(bmeta, "CONTROL")
    _verify_meta(pmeta, "PATCHED")
    original_source = _trace(bsrc, ORIGINAL, (0, 0), "CONTROL_QT")
    original_final = _trace(bfinal, OUTPUT, ORIGIN, "CONTROL_COMPOSITOR")
    patched_source = _trace(psrc, ORIGINAL, (0, 0), "PATCHED_QT")
    patched_final = _trace(pfinal, OUTPUT, ORIGIN, "PATCHED_COMPOSITOR")
    comparison = assess(original_source, original_final,
                        patched_source, patched_final)
    comparison["source"] = "distinct_pinned_native_CI_Qt_and_SwayPX_PPM"
    comparison["negativeControl"] = {
        "qt": original_source, "compositor": original_final
    }
    comparison["patched"] = {
        "qt": patched_source, "compositor": patched_final
    }
    report.write_text(json.dumps(comparison,sort_keys=True,indent=2)+"\n")
    print("SD_COMPANY_M31A_ISOLATED_GL_UNPACK_CORRECTION "
          "original_1493_control_shift=-1 "
          "patched_1493_shift="+str(comparison["patchedCompositorShiftPxPerRow"])+" "
          "real_Qt_and_compositor_ppms=1 "
          "original_RBG565_pitch=2986 native_padding=0 "
          "production_renderer=OFF PASS",flush=True)
    return 0


def selftest():
    good={"signedHorizontalShiftPerSourceRowPx":0.0,
          "shiftInterceptPx":0.0, "maximumFitResidualPx":0.0,
          "rowsWithWrappedBars":0,"measuredRows":14}
    distorted={**good,
        "signedHorizontalShiftPerSourceRowPx":-1.0,
        "rowsWithWrappedBars":3}
    result=assess(good,distorted,good,good)
    if (result["patchedCompositorShiftPxPerRow"]!=0 or
            result["patchedProductionRenderer"] is not False):
        raise RuntimeError("M31A_SELFTEST_FALSE_PASS")
    for badcase in (
        (good,good,good,good),
        (good,distorted,good,distorted),
        (distorted,distorted,good,good),
        (good,distorted,distorted,good),
        (good,distorted,good,{**good,"maximumFitResidualPx":9}),
        (good,distorted,good,{**good,"measuredRows":3}),
    ):
        try:
            assess(*badcase)
        except RuntimeError:
            continue
        raise RuntimeError("M31A_NEGATIVE_CONTROL_OR_BAD_PATCH_FALSE_ACCEPT")
    print("SD_COMPANY_M31A_IMPACT_SELFTEST "
          "bad_baseline=REFUSED lingering_shear=REFUSED "
          "same_native_Qt_not_enough=1 production=OFF PASS",flush=True)


def main(argv: list[str]):
    if argv == ["--selftest"]:
        selftest()
        return 0
    if argv and argv[0] == "--compare":
        return analyze(argv[1:])
    raise RuntimeError("M31A_ARGUMENTS_REQUIRE_SELFTEST_OR_COMPARE")


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except (RuntimeError, AssertionError, OSError, ValueError) as exc:
        print("SD_COMPANY_M31A_REFUSED",str(exc),file=sys.stderr)
        sys.exit(1)
