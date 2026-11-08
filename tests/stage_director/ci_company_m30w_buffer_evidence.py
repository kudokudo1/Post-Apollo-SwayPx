#!/usr/bin/env python3
"""M30w // Validate *real* wlroots-imported native buffer metadata.

An independent check on the genuine compositor log after it has
kernel-matched the same native MapLibre XDG root and a real Company host.
Does not read Qt QImage bytesPerLine(), fetch frame pixels, create a
Wayland connection, or grant production scene/input permissions.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

MARKER = "SD_CI_COMPANY_M30W_NATIVE_BUFFER "
DMA_MARKER = "SD_CI_COMPANY_M30W_DMABUF_METADATA "

def evidence(log: str) -> dict:
    rows = [line.split(MARKER,1)[1] for line in log.splitlines()
            if MARKER in line]
    if len(rows) != 1:
        raise RuntimeError("M30W_MISSING_OR_MULTIPLE_ROOT_BUFFER_REPORTS")
    fields = dict(re.findall(r"([a-z_]+)=([^\s]+)",rows[0]))
    required = (
        "status","kind","root_surface","physical","canvas",
        "format","plane0_stride","planes","modifier",
        "surface_scale","surface_transform","epoch","actor",
        "production_actor","scene_authority","input_authority")
    if set(fields) != set(required):
        raise RuntimeError("M30W_NATIVE_BUFFER_SCHEMA_CHANGED "+repr(fields))
    if (fields["status"]!="OBSERVED" or
            fields["kind"] not in ("DMABUF","SHM")):
        raise RuntimeError("M30W_NO_REAL_BUFFER_FORMAT_AND_PITCH")
    if (fields["root_surface"]!="1" or
            fields["physical"]!="1494x788" or
            fields["canvas"]!="1493x788"):
        raise RuntimeError("M30W_WRONG_SELECTED_ROOT_OR_DIMENSIONS")
    if (fields["actor"]!="KERNEL_CI_ONLY" or
            fields["production_actor"]!="UNVERIFIED" or
            fields["scene_authority"]!="NONE" or
            fields["input_authority"]!="NONE"):
        raise RuntimeError("M30W_TEST_ONLY_AUTHORITY_BOUNDARY_CHANGED")
    if not re.fullmatch(r"0x[0-9a-fA-F]{8}",fields["format"]):
        raise RuntimeError("M30W_NATIVE_FORMAT_MALFORMED")
    fmt = int(fields["format"],16)
    pitch=int(fields["plane0_stride"])
    planes=int(fields["planes"])
    scale=int(fields["surface_scale"])
    transform=int(fields["surface_transform"])
    epoch=int(fields["epoch"])
    if (fmt==0 or pitch<=0 or planes<1 or planes>4 or
            scale<1 or scale>16 or transform<0 or transform>7 or epoch<1):
        raise RuntimeError("M30W_NATIVE_PROFILE_INVALID")
    if pitch>131072:
        raise RuntimeError("M30W_NATIVE_PITCH_UNBOUNDED")
    if fields["kind"]=="SHM" and (planes!=1 or
            fields["modifier"]!="UNKNOWN"):
        raise RuntimeError("M30W_SHM_MUST_NOT_CLAIM_DMA_MODIFIER")
    if fields["kind"]=="DMABUF" and fields["modifier"]!="DMA_PRESENT":
        raise RuntimeError("M30W_DMA_MODIFIER_EVIDENCE_MISSING")
    dma_rows = [line.split(DMA_MARKER,1)[1] for line in log.splitlines()
                if DMA_MARKER in line]
    modifier=None
    if fields["kind"]=="DMABUF":
        if len(dma_rows)!=1:
            raise RuntimeError("M30W_MISSING_EXACT_DMA_METADATA")
        dma=dict(re.findall(r"([a-z_]+)=([^\s]+)",dma_rows[0]))
        if set(dma)!=set(("modifier","plane0_stride","format",
                           "planes","epoch","authority")) or (
                dma["authority"]!="NONE" or
                not re.fullmatch(r"0x[0-9a-fA-F]{16}",dma["modifier"]) or
                int(dma["plane0_stride"])!=pitch or
                int(dma["format"],16)!=fmt or
                int(dma["planes"])!=planes or
                int(dma["epoch"])!=epoch):
            raise RuntimeError("M30W_DMA_METADATA_INCONSISTENT")
        modifier=dma["modifier"]
    elif dma_rows:
        raise RuntimeError("M30W_UNEXPECTED_DMA_METADATA_ON_SHM")
    return {
        "schema":"post-apollo.swaypx.company-native-buffer-ci-m30w.v1",
        "status":"OBSERVED_NATIVE_BUFFER_METADATA_CI_ONLY",
        "kind":fields["kind"],
        "physicalWidth":1494,"physicalHeight":788,
        "visibleCanvasWidth":1493,"visibleCanvasHeight":788,
        "fourccHex":fields["format"],
        "actualPlane0PitchBytes":pitch,
        "planeCount":planes,
        "modifierHex":modifier,
        "nativeSurfaceScale":scale,
        "nativeSurfaceTransform":transform,
        "selectedRootSurface":True,
        "mappedCiGeneration":epoch,
        "kernelIdentityCheckedByExistingCiSelector":True,
        "productionActorAttested":False,
        "productionSceneAuthority":False,
        "productionInputAuthority":False,
        "source":"wlr_buffer_get_shm_or_dmabuf",
        "cpuQtReadbackPitchNotSubstituted":True,
    }


def main(args: list[str]) -> int:
    if len(args)==2 and args[0]=="--check":
        log=Path(args[1]).read_text(errors="replace")
        data=evidence(log)
        print("SD_COMPANY_M30W_REAL_NATIVE_BUFFER "
              "kind="+data["kind"]+
              " actual_pitch="+str(data["actualPlane0PitchBytes"])+
              " fourcc="+data["fourccHex"]+
              " source=WLROOTS_IMPORTED_NATIVE_SCENE_BUFFER "
              "root_identity=CI_KERNEL_MATCHED "
              "production_actor=UNVERIFIED scene_input=OFF PASS",
              flush=True)
        print(json.dumps(data,sort_keys=True,indent=2))
        return 0
    if len(args)==1 and args[0]=="--selftest":
        def line(**kw):
            fields={"status":"OBSERVED","kind":"SHM","root_surface":"1",
                "physical":"1494x788","canvas":"1493x788",
                "format":"0x34325241","plane0_stride":"5976",
                "planes":"1","modifier":"UNKNOWN",
                "surface_scale":"1","surface_transform":"0",
                "epoch":"1","actor":"KERNEL_CI_ONLY",
                "production_actor":"UNVERIFIED",
                "scene_authority":"NONE","input_authority":"NONE"}
            fields.update(kw)
            return MARKER+" ".join(k+"="+v for k,v in fields.items())
        r=evidence(line())
        assert r["actualPlane0PitchBytes"]==5976
        for b in [
            "",
            line(status="REFUSED",kind="UNKNOWN"),
            line(physical="1493x788"),
            line(canvas="1494x788"),
            line(format="0x00000000"),
            line(plane0_stride="0"),
            line(scene_authority="YES"),
            line(actor="TITLE_ONLY"),
            line(production_actor="VERIFIED"),
            line(root_surface="0"),
            line(epoch="0"),
            line()+ "\n"+line()
        ]:
            try:
                evidence(b)
            except (RuntimeError,ValueError):
                pass
            else:
                raise RuntimeError("M30W_SELFTEST_ACCEPTED_INVALID_EVIDENCE "+b)
        print("SD_COMPANY_M30W_SELFTEST "
              "real_buffer_evidence=STRICT "
              "unknown_bad_identity_wrong_geometry=REFUSED "
              "production_authority=OFF PASS",flush=True)
        return 0
    raise RuntimeError("M30W_ARGUMENTS_ONLY_CHECK_OR_SELFTEST")

if __name__=="__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except (RuntimeError,ValueError) as exc:
        print("SD_COMPANY_M30W_EVIDENCE_REFUSED",str(exc),
              file=sys.stderr,flush=True)
        raise SystemExit(1)
