#!/usr/bin/env python3
"""M30z // compare real original Qt fiducials against compositor pixels.

Reads ONLY bounded, offline PPMs from two already isolated CI sessions.
Records pixel geometry and the independently observed M30y native buffer
pitch. Does not infer that pixel pitch is necessarily the cause of shear.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path
import ci_company_m30t_pixel_geometry as geometry

NATIVE_UNPADDED = (1493, 788)
CORRECTED = (1494, 788)
OUTPUT = (1920, 1080)
ORIGIN = (166, 203)
SCHEMA = "post-apollo.swaypx.company-native-pixel-origin-m30z.v1"


def _interpret(original_native: dict, original_composed: dict | None,
               corrected_native: dict, corrected_composed: dict,
               missing_reason: str | None) -> dict:
    source_straight = geometry.straight(original_native)
    corrected_straight = (geometry.straight(corrected_native) and
                          geometry.straight(corrected_composed))
    if not source_straight:
        raise RuntimeError("M30Z_ORIGINAL_QT_SOURCE_NOT_STRAIGHT")
    if not corrected_straight:
        raise RuntimeError("M30Z_CORRECTED_REFERENCE_BARS_NOT_STRAIGHT")
    if original_composed is None:
        classification = "ORIGINAL_COMPOSITOR_FIDUCIALS_UNMEASURABLE"
        distortion = None
        placement = None
    else:
        placement = all(
            abs(original_native[key] - original_composed[key]) <= 3
            for key in ("left_at_top_px", "left_at_bottom_px",
                        "right_at_top_px", "right_at_bottom_px"))
        distortion = not geometry.straight(original_composed) or not placement
        classification = ("ORIGINAL_COMPOSITOR_DISTORTION_OBSERVED" if distortion
                          else "ORIGINAL_COMPOSITOR_STRAIGHT_IN_THIS_RUN")
    return {
        "classification": classification,
        "originalQtSourceStraight": source_straight,
        "originalCompositorDistortionObserved": distortion,
        "originalCompositorFiducialsMeasurable": original_composed is not None,
        "originalQtToCompositorPlacementWithin3px": placement,
        "correctedReferenceBarsStraight": corrected_straight,
        "originalCompositorMissingReason": missing_reason,
        "originalQtSourceGeometry": original_native,
        "originalCompositorGeometry": original_composed,
        "correctedQtSourceGeometry": corrected_native,
        "correctedCompositorGeometry": corrected_composed,
        "pixelShearRootCauseCertified": False,
        "productionRendererOrInputAuthorized": False,
    }


def analyze(original_qt: Path, original_final: Path,
            corrected_qt: Path, corrected_final: Path,
            original_meta_path: Path, corrected_meta_path: Path,
            result_path: Path) -> dict:
    original_meta = json.loads(original_meta_path.read_text())
    corrected_meta = json.loads(corrected_meta_path.read_text())
    if (original_meta.get("status") != "OBSERVED_ORIGINAL_IMPORT_CI_ONLY" or
            original_meta.get("physicalWidth") != 1493 or
            original_meta.get("physicalHeight") != 788 or
            original_meta.get("kind") != "SHM" or
            original_meta.get("fourccHex") != "0x36314752" or
            original_meta.get("actualPlane0PitchBytes") != 2986 or
            original_meta.get("observedRowPaddingBytes") != 0 or
            original_meta.get("productionSceneAuthority") is not False):
        raise RuntimeError("M30Z_ORIGINAL_REAL_WLROOTS_IMPORT_NOT_ATTESTED_BY_CI")
    if (corrected_meta.get("status") !=
            "OBSERVED_NATIVE_BUFFER_METADATA_CI_ONLY" or
            corrected_meta.get("physicalWidth") != 1494 or
            corrected_meta.get("physicalHeight") != 788 or
            corrected_meta.get("kind") != "SHM" or
            corrected_meta.get("fourccHex") != "0x36314752" or
            corrected_meta.get("actualPlane0PitchBytes") != 2988 or
            corrected_meta.get("observedRowPaddingBytes") != 0 or
            corrected_meta.get("productionSceneAuthority") is not False):
        raise RuntimeError("M30Z_CORRECTED_REAL_WLROOTS_IMPORT_CHANGED")
    original_source = geometry.read_ppm(original_qt, NATIVE_UNPADDED)
    original_screen = geometry.read_ppm(original_final, OUTPUT)
    corrected_source = geometry.read_ppm(corrected_qt, CORRECTED)
    corrected_screen = geometry.read_ppm(corrected_final, OUTPUT)
    native = geometry.measure(original_source, (0, 0), "M30Z_ORIGINAL_QT")
    try:
        composited = geometry.measure(
            original_screen, ORIGIN, "M30Z_ORIGINAL_COMPOSITED")
        missing = None
    except RuntimeError as exc:
        if ("FIDUCIAL_MISSING" not in str(exc) and
                "FIDUCIAL_PAIR_AMBIGUOUS" not in str(exc)):
            raise
        composited = None
        missing = str(exc)
    corrected_native = geometry.measure(
        corrected_source, (0, 0), "M30Z_CORRECTED_QT")
    corrected_composed = geometry.measure(
        corrected_screen, ORIGIN, "M30Z_CORRECTED_COMPOSITED")
    report = _interpret(native, composited, corrected_native,
                        corrected_composed, missing)
    report.update({
        "schema": SCHEMA,
        "status": "OBSERVED_TEST_ONLY_PIXEL_COMPARISON",
        "source": "real_QQuickWindow_grabWindow_and_swaypx_grim_PPM",
        "actualOriginalNativeFormat": "SHM_RGB565",
        "actualOriginalWidth": 1493,
        "actualOriginalImportedStrideBytes": 2986,
        "actualOriginalRowPaddingBytes": 0,
        "actualCorrectedWidth": 1494,
        "actualCorrectedImportedStrideBytes": 2988,
        "actualCorrectedRowPaddingBytes": 0,
        "testOutputSize": "1920x1080",
        "sameLiveActor": False,
        "originalCompanySourceModified": False,
        "hardwareMultiOutputVerified": False,
        "universalAlignmentRuleEstablished": False,
    })
    result_path.write_text(json.dumps(report, sort_keys=True, indent=2)+"\n")
    print("SD_COMPANY_M30Z_ORIGINAL_PIXEL_GEOMETRY "
          "qt_straight=1 corrected_straight=1 "
          "original_classification="+report["classification"]+" "
          "actual_native_original_pitch=2986 corrected_pitch=2988 "
          "pixel_shear_cause_certified=NO production_authority=OFF PASS",
          flush=True)
    return report


def selftest() -> None:
    source = geometry.measure(geometry.synthetic(), (0, 0),
                              "M30Z_SYNTH_NATIVE")
    good = geometry.measure(geometry.synthetic(), (0, 0),
                            "M30Z_SYNTH_COMPOSITED")
    skewed = geometry.measure(geometry.synthetic(skew=35), (0, 0),
                              "M30Z_SYNTH_SKEW")
    ok = _interpret(source, good, source, good, None)
    if (ok["classification"] != "ORIGINAL_COMPOSITOR_STRAIGHT_IN_THIS_RUN" or
            ok["originalCompositorDistortionObserved"] is not False):
        raise RuntimeError("M30Z_BASELINE_SYNTHETIC_CLASSIFICATION_WRONG")
    bad = _interpret(source, skewed, source, good, None)
    if (bad["classification"] != "ORIGINAL_COMPOSITOR_DISTORTION_OBSERVED" or
            bad["originalCompositorDistortionObserved"] is not True):
        raise RuntimeError("M30Z_FAILED_TO_DETECT_SYNTHETIC_35PX_SHEAR")
    absent = _interpret(source, None, source, good, "missing")
    if (absent["classification"] !=
            "ORIGINAL_COMPOSITOR_FIDUCIALS_UNMEASURABLE" or
            absent["originalCompositorDistortionObserved"] is not None):
        raise RuntimeError("M30Z_MISSING_FIDUCIALS_FALSE_CERTIFICATION")
    try:
        _interpret(skewed, good, source, good, None)
    except RuntimeError as ex:
        if "ORIGINAL_QT_SOURCE_NOT_STRAIGHT" not in str(ex):
            raise
    else:
        raise RuntimeError("M30Z_ACCEPTED_BAD_NATIVE_QT_SOURCE")
    print("SD_COMPANY_M30Z_SELFTEST 35px_shear=DETECTED "
          "missing_unmeasurable=REFUSED_SOURCE_OR_DECLARED "
          "production_authority=OFF PASS", flush=True)


def main(args: list[str]) -> int:
    if args == ["--selftest"]:
        selftest()
        return 0
    if len(args) == 8 and args[0] == "--analyze":
        analyze(*map(Path, args[1:]))
        return 0
    raise RuntimeError("M30Z_EXPECT_SELFTEST_OR_SEVEN_INPUT_AND_OUTPUT_PATHS")


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except (OSError, ValueError, AssertionError, RuntimeError) as ex:
        print("SD_COMPANY_M30Z_REFUSED", str(ex), file=sys.stderr)
        sys.exit(1)
