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


# M30z second pass: follow the two green fiducials around the *full*
# visible 1493px row, including wrap at the left/right edge. The original
# narrow M30t scan assumed both bars would stay in the original x window.
# This does NOT assume the amount or direction of shear before measuring.
SCAN_ROWS = tuple(range(170, 496, 25))
VISIBLE_WIDTH = 1493
BAR_CENTERS = (307.5, 464.5)
EXPECTED_SEPARATION = BAR_CENTERS[1] - BAR_CENTERS[0]


def wrapped_bar_trace(image, offset: tuple[int, int], label: str) -> dict:
    ox, oy = offset
    observations = []
    for y in SCAN_ROWS:
        spans = geometry.runs(
            x for x in range(VISIBLE_WIDTH)
            if geometry.is_green(geometry.rgb(image, ox + x, oy + y)))
        spans = [(a, b) for a, b in spans if b-a+1 <= 12]
        if len(spans) != 2:
            raise RuntimeError(f"M30Z_{label}_WRAPPED_BARS_MISSING "
                               f"row={y} spans={spans[:10]}")
        c0, c1 = [(a+b)/2.0 for a, b in spans]
        if abs(((c1 - c0) % VISIBLE_WIDTH) -
               EXPECTED_SEPARATION) <= 1.0:
            left, right = c0, c1
        elif abs(((c0 - c1) % VISIBLE_WIDTH) -
                 EXPECTED_SEPARATION) <= 1.0:
            left, right = c1, c0
        else:
            raise RuntimeError(f"M30Z_{label}_WRAPPED_BAR_SPACING_WRONG "
                               f"row={y} centers={c0},{c1}")
        shift = (left - BAR_CENTERS[0]) % VISIBLE_WIDTH
        if shift > VISIBLE_WIDTH / 2:
            shift -= VISIBLE_WIDTH
        observations.append({
            "row": y,
            "leftCenterPx": left,
            "rightCenterPx": right,
            "leftShiftFromQtPx": shift,
            "barCrossesWrap": left > 1000 or right > 1000
        })
    if len(observations) != len(SCAN_ROWS):
        raise RuntimeError("M30Z_INCOMPLETE_WRAPPED_BAR_MEASUREMENT")
    n = len(observations)
    mean_x = sum(d["row"] for d in observations) / n
    mean_y = sum(d["leftShiftFromQtPx"] for d in observations) / n
    xx = sum((d["row"]-mean_x)**2 for d in observations)
    if xx <= 0:
        raise RuntimeError("M30Z_INSUFFICIENT_VERTICAL_ROW_VARIATION")
    slope = sum(
        (d["row"]-mean_x)*(d["leftShiftFromQtPx"]-mean_y)
        for d in observations) / xx
    intercept = mean_y - slope*mean_x
    residual = max(abs(d["leftShiftFromQtPx"] -
                       (slope*d["row"]+intercept))
                   for d in observations)
    return {
        "measuredRows": n,
        "visibleWidthPx": VISIBLE_WIDTH,
        "signedHorizontalShiftPerSourceRowPx": round(slope, 6),
        "shiftInterceptPx": round(intercept, 6),
        "maximumFitResidualPx": round(residual, 6),
        "rowsWithWrappedBars": sum(d["barCrossesWrap"] for d in observations),
        "rowSamples": observations,
        "reference": "actual compositor and native PPM pixels",
        "causeCertified": False
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
    # The narrow M30t scan misses bars that wrap around a 1493px row.
    # Re-scan the *actual* PPMs across the entire visible width, never
    # extrapolating a shift from buffer pitch or invented pixel positions.
    source_trace = wrapped_bar_trace(original_source, (0, 0), "NATIVE_ORIGINAL")
    corrected_source_trace = wrapped_bar_trace(
        corrected_source, (0, 0), "NATIVE_CORRECTED")
    original_final_trace = wrapped_bar_trace(
        original_screen, ORIGIN, "COMPOSITOR_ORIGINAL")
    corrected_final_trace = wrapped_bar_trace(
        corrected_screen, ORIGIN, "COMPOSITOR_CORRECTED")
    for label, trace in (("SOURCE_ORIGINAL", source_trace),
                         ("SOURCE_CORRECTED", corrected_source_trace),
                         ("COMPOSITOR_CORRECTED", corrected_final_trace)):
        if (abs(trace["signedHorizontalShiftPerSourceRowPx"]) > .02 or
                abs(trace["shiftInterceptPx"]) > 1 or
                trace["maximumFitResidualPx"] > .5):
            raise RuntimeError("M30Z_"+label+"_REFERENCE_DRIFT_INVALID")
    shifted = (abs(original_final_trace[
        "signedHorizontalShiftPerSourceRowPx"] + 1.0) <= .02 and
        abs(original_final_trace["shiftInterceptPx"]) <= 1 and
        original_final_trace["maximumFitResidualPx"] <= .5 and
        original_final_trace["rowsWithWrappedBars"] >= 1)
    report["originalCompositorWrappedBarTrace"] = original_final_trace
    report["originalQtSourceWrappedBarTrace"] = source_trace
    report["correctedCompositorWrappedBarTrace"] = corrected_final_trace
    report["correctedQtSourceWrappedBarTrace"] = corrected_source_trace
    report["originalFullWidthBarTrackingSucceeded"] = True
    report["pixelRowShiftOfExactlyOneLeftObserved"] = shifted
    report["narrowM30tScanMissedWrappedBars"] = composited is None
    report["classification"] = (
        "ORIGINAL_COMPOSITOR_ONE_PIXEL_LEFT_PER_ROW_WITH_MODULO_WRAP"
        if shifted else "ORIGINAL_COMPOSITOR_DIFFERENT_MEASURED_ROW_PATTERN")
    report["originalCompositorDistortionObserved"] = shifted
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
          "measured_pixel_shift_per_row="+str(report[
              "originalCompositorWrappedBarTrace"][
              "signedHorizontalShiftPerSourceRowPx"])+" "
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
    # Test full-width scan with true modulo wrap, not a fixture of
    # invented real PPM evidence. Contrast a straight 1493px reference
    # with precisely one pixel shifted left per successive source row.
    def synthetic_wrap(per_row_shift):
        w, h = VISIBLE_WIDTH, 788
        pixels = bytearray(bytes((27, 6, 35)) * (w * h))
        for y in SCAN_ROWS:
            offset = per_row_shift * y
            for base in (305, 462):
                for dx in range(6):
                    x = (base + offset + dx) % w
                    p = (y*w + x)*3
                    pixels[p:p+3] = bytes((4, 255, 50))
        return w, h, memoryview(pixels)
    straight_trace=wrapped_bar_trace(synthetic_wrap(0), (0,0),"TEST_STRAIGHT")
    wrapped_trace=wrapped_bar_trace(synthetic_wrap(-1),(0,0),"TEST_WRAPPED")
    if (straight_trace["signedHorizontalShiftPerSourceRowPx"] != 0 or
            wrapped_trace["signedHorizontalShiftPerSourceRowPx"] != -1 or
            wrapped_trace["maximumFitResidualPx"] != 0 or
            wrapped_trace["rowsWithWrappedBars"] < 1):
        raise RuntimeError("M30Z_MODULO_ROW_SHIFT_SELFTEST_FAILED")
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
