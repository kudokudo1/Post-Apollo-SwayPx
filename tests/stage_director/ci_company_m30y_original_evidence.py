#!/usr/bin/env python3
"""M30y: validate the *original* unpadded native wlroots buffer in CI.

Do not assume a stride value for 1493 pixels. Require a real wlroots
scene-root observation from the selected native actor, then compare with
M30w's separately measured padded buffer. The two samples come from
separate disposable sessions using identical pinned sources/renderers.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

MARKER = "SD_CI_COMPANY_M30Y_ORIGINAL_NATIVE_BUFFER "
SCHEMA = "post-apollo.swaypx.company-original-native-buffer-ci-m30y.v1"
REQUIRED = {
    "status", "kind", "root_surface", "physical", "canvas", "format",
    "plane0_stride", "planes", "modifier", "surface_scale",
    "surface_transform", "epoch", "actor", "production_actor",
    "scene_authority", "input_authority"
}
RGB565 = int.from_bytes(b"RG16", "little")


def original_evidence(log: str) -> dict:
    reports = [row.split(MARKER, 1)[1] for row in log.splitlines()
               if MARKER in row]
    if len(reports) != 1:
        raise RuntimeError("M30Y_MISSING_OR_DUPLICATE_ROOT_OBSERVATION")
    pairs = re.findall(r"([a-z0-9_]+)=([^\s]+)", reports[0])
    if len(pairs) != len(REQUIRED):
        raise RuntimeError("M30Y_MALFORMED_NATIVE_EVIDENCE")
    fields = dict(pairs)
    if set(fields) != REQUIRED:
        raise RuntimeError("M30Y_UNEXPECTED_NATIVE_EVIDENCE_FIELDS")
    if (fields["status"] != "OBSERVED" or
            fields["root_surface"] != "1" or
            fields["physical"] != "1493x788" or
            fields["canvas"] != "1493x788"):
        raise RuntimeError("M30Y_ORIGINAL_ROOT_GEOMETRY_NOT_OBSERVED")
    if (fields["actor"] != "KERNEL_CI_ONLY" or
            fields["production_actor"] != "UNVERIFIED" or
            fields["scene_authority"] != "NONE" or
            fields["input_authority"] != "NONE"):
        raise RuntimeError("M30Y_AUTHORITY_SCOPE_BROKEN")
    if not re.fullmatch(r"0x[0-9a-fA-F]{8}", fields["format"]):
        raise RuntimeError("M30Y_BAD_NATIVE_FOURCC")
    fmt = int(fields["format"], 16)
    if fields["kind"] != "SHM" or fmt != RGB565 or (
            fields["modifier"] != "UNKNOWN"):
        raise RuntimeError("M30Y_UNREVIEWED_NATIVE_FORMAT")
    def number(key, low, high):
        value = fields[key]
        if not re.fullmatch(r"[0-9]+", value):
            raise RuntimeError("M30Y_INVALID_NUMERIC_FIELD_" + key)
        n = int(value)
        if not low <= n <= high:
            raise RuntimeError("M30Y_OUT_OF_RANGE_" + key)
        return n
    planes = number("planes", 1, 1)
    pitch = number("plane0_stride", 1493 * 2, 131072)
    scale = number("surface_scale", 1, 1)
    transform = number("surface_transform", 0, 0)
    epoch = number("epoch", 1, 10000000)
    return {
        "schema": SCHEMA,
        "status": "OBSERVED_ORIGINAL_IMPORT_CI_ONLY",
        "kind": fields["kind"],
        "physicalWidth": 1493,
        "physicalHeight": 788,
        "visibleWidth": 1493,
        "visibleHeight": 788,
        "fourccHex": fields["format"],
        "formatName": "DRM_FORMAT_RGB565",
        "planeCount": planes,
        "actualPlane0PitchBytes": pitch,
        "packedMinimumRowBytes": 2986,
        "observedRowPaddingBytes": pitch - 2986,
        "nativeSurfaceScale": scale,
        "nativeSurfaceTransform": transform,
        "mappedCiGeneration": epoch,
        "source": "wlr_buffer_get_shm",
        "rootActorCheckedByCiKernelSelector": True,
        "actualOriginalPitchMeasuredNotCalculated": True,
        "productionActorAuthorized": False,
        "productionSceneAuthority": False,
        "productionInputAuthority": False,
        "profileScope": "ONE_HEADLESS_MESA_UNPADDED_CI_RUN",
        "universalDriverAlignmentVerified": False,
    }


def compare(original: dict, corrected: dict) -> dict:
    if original.get("schema") != SCHEMA or (
            original.get("status") != "OBSERVED_ORIGINAL_IMPORT_CI_ONLY") or (
            original.get("productionActorAuthorized") is not False) or (
            original.get("productionSceneAuthority") is not False) or (
            original.get("productionInputAuthority") is not False):
        raise RuntimeError("M30Y_ORIGINAL_EVIDENCE_UNTRUSTED_OR_MISSING")
    if (corrected.get("schema") !=
            "post-apollo.swaypx.company-native-buffer-ci-m30w.v1" or
            corrected.get("status") !=
            "OBSERVED_NATIVE_BUFFER_METADATA_CI_ONLY" or
            corrected.get("physicalWidth") != 1494 or
            corrected.get("physicalHeight") != 788 or
            corrected.get("actualPlane0PitchBytes") != 2988 or
            corrected.get("productionActorAttested") is not False or
            corrected.get("productionSceneAuthority") is not False or
            corrected.get("productionInputAuthority") is not False):
        raise RuntimeError("M30Y_CORRECTED_M30W_EVIDENCE_CHANGED")
    for field in ("kind", "fourccHex", "nativeSurfaceScale",
                  "nativeSurfaceTransform", "planeCount"):
        if original.get(field) != corrected.get(field):
            raise RuntimeError("M30Y_SAMPLES_NOT_COMPARABLE_" + field)
    pitch = original["actualPlane0PitchBytes"]
    if type(pitch) is not int or pitch < 2986 or pitch > 131072:
        raise RuntimeError("M30Y_ORIGINAL_PITCH_INVALID")
    # This is a comparison of two measured wlroots-imported buffers,
    # not proof of why the earlier compositor screenshot sheared.
    return {
        "schema": "post-apollo.swaypx.company-paired-imported-buffers-m30y.v1",
        "status": "PAIRED_CI_MEASUREMENTS_ONLY",
        "nativeFormat": "SHM_RGB565",
        "originalWidth": 1493,
        "originalMeasuredPitchBytes": pitch,
        "originalMeasuredPaddingBytes": pitch - 2986,
        "correctedWidth": 1494,
        "correctedMeasuredPitchBytes": 2988,
        "correctedMeasuredPaddingBytes": 0,
        "observedPitchDeltaBytes": 2988 - pitch,
        "sameSoftwareStackRequired": True,
        "sameLiveNativeActorClaimed": False,
        "exactCauseOfPixelShearCertified": False,
        "productionBufferPolicyAuthorized": False,
        "productionSceneAuthority": False,
        "productionInputAuthority": False,
        "hardwareAndScalingValidated": False
    }


def test_self() -> None:
    def line(**changes):
        fields = {
            "status": "OBSERVED", "kind": "SHM", "root_surface": "1",
            "physical": "1493x788", "canvas": "1493x788",
            "format": "0x36314752", "plane0_stride": "2988",
            "planes": "1", "modifier": "UNKNOWN",
            "surface_scale": "1", "surface_transform": "0",
            "epoch": "1", "actor": "KERNEL_CI_ONLY",
            "production_actor": "UNVERIFIED",
            "scene_authority": "NONE", "input_authority": "NONE"
        }
        fields.update(changes)
        return MARKER + " ".join(f"{k}={v}" for k, v in fields.items())
    for pitch in ("2986", "2988", "2992"):
        d = original_evidence(line(plane0_stride=pitch))
        assert d["actualPlane0PitchBytes"] == int(pitch)
        assert d["observedRowPaddingBytes"] == int(pitch) - 2986
    bad = [
        "", line() + "\n" + line(), line(physical="1494x788"),
        line(canvas="1494x788"), line(status="REFUSED"),
        line(root_surface="0"), line(format="0x34325241"),
        line(kind="DMABUF"), line(modifier="DMA_PRESENT"),
        line(plane0_stride="0"), line(plane0_stride="2985"),
        line(plane0_stride="5976-not-measured"),
        line(planes="2"), line(surface_scale="2"),
        line(surface_transform="1"), line(epoch="0"),
        line(actor="TITLE_ONLY"), line(production_actor="VERIFIED"),
        line(scene_authority="GRANTED"), line(input_authority="GRANTED"),
        line(other_field="x")
    ]
    for b in bad:
        try:
            original_evidence(b)
        except (RuntimeError, ValueError):
            continue
        raise RuntimeError("M30Y_NEGATIVE_SELFTEST_FALSE_ACCEPT")
    good = original_evidence(line())
    w = {
        "schema": "post-apollo.swaypx.company-native-buffer-ci-m30w.v1",
        "status": "OBSERVED_NATIVE_BUFFER_METADATA_CI_ONLY",
        "kind": "SHM", "fourccHex": "0x36314752",
        "physicalWidth": 1494, "physicalHeight": 788,
        "actualPlane0PitchBytes": 2988, "nativeSurfaceScale": 1,
        "nativeSurfaceTransform": 0, "planeCount": 1,
        "productionActorAttested": False,
        "productionSceneAuthority": False,
        "productionInputAuthority": False
    }
    assert compare(good, w)["observedPitchDeltaBytes"] == 0
    for mutation in ({"kind": "DMABUF"}, {"actualPlane0PitchBytes": 5976},
                     {"nativeSurfaceScale": 2},
                     {"productionSceneAuthority": True}):
        try:
            compare(good, {**w, **mutation})
        except RuntimeError:
            continue
        raise RuntimeError("M30Y_PAIRED_FALSE_ACCEPT")
    print("SD_COMPANY_M30Y_PAIRED_SELFTEST original_pitch_not_guessed=1 "
          "production_authority=OFF PASS", flush=True)


def main(args: list[str]) -> int:
    if len(args) == 1 and args[0] == "--selftest":
        test_self()
        return 0
    if len(args) == 3 and args[0] == "--compare":
        original = json.loads(Path(args[1]).read_text())
        corrected = json.loads(Path(args[2]).read_text())
        result = compare(original, corrected)
        print(json.dumps(result, sort_keys=True, indent=2))
        print("SD_COMPANY_M30Y_PAIRED_NATIVE_BUFFER "
              "native_format=RGB565 original_pitch=MEASURED "
              "corrected_pitch=MEASURED production_authority=OFF PASS",
              flush=True)
        return 0
    raise RuntimeError("M30Y_CI_ONLY_SELFTEST_OR_PAIR_ARGUMENTS_REQUIRED")


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except (RuntimeError, OSError, ValueError, AssertionError) as ex:
        print("SD_COMPANY_M30Y_REFUSED", str(ex), file=sys.stderr)
        sys.exit(1)
