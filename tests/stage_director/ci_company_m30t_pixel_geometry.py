#!/usr/bin/env python3
"""M30t: compare straight Qt fiducial bars in native and composited pixels.

Reads ONLY bounded, synthetic offline CI PPM files. No OCR, network,
Company records, widget edits, renderer permission or user display access.
The geometry check MUST FAIL when straight Qt Rectangles become slanted.
"""
from __future__ import annotations
import json
from pathlib import Path
import sys

EXPECTED_NATIVE = (1494,788)
EXPECTED_COMPOSITOR = (1920,1080)
CANVAS_ORIGIN=(166,203)
SAMPLE_Y=tuple(range(205,491,26))
SAMPLE_X=tuple(range(325,446,15))


def read_ppm(path: Path, dimensions):
    data=path.read_bytes()
    if len(data)>12*1024*1024 or not data.startswith(b"P6"):
        raise RuntimeError("M30T_NOT_BOUNDED_RAW_PPM")
    p=2
    def token():
        nonlocal p
        while p<len(data) and data[p] in b" \t\r\n":
            p+=1
        if p>=len(data):
            raise RuntimeError("M30T_PPM_TRUNCATED_HEADER")
        end=p
        while end<len(data) and data[end] not in b" \t\r\n":
            end+=1
        result=data[p:end]
        p=end
        return result
    w,h,maxval=(int(token()),int(token()),int(token()))
    if (w,h)!=dimensions or maxval!=255:
        raise RuntimeError(f"M30T_WRONG_PPM_DIMENSIONS {(w,h,maxval)}")
    if p>=len(data) or data[p] not in b" \t\r\n":
        raise RuntimeError("M30T_PPM_NO_BINARY_DELIMITER")
    p+=1
    if len(data)-p!=w*h*3:
        raise RuntimeError(f"M30T_INCORRECT_IMAGE_BYTES expected={w*h*3} "
                           f"actual={len(data)-p}")
    return w,h,memoryview(data)[p:]


def rgb(im,x,y):
    w,h,b=im
    if x<0 or y<0 or x>=w or y>=h:
        raise RuntimeError("M30T_PIXEL_OUTSIDE_SNAPSHOT")
    k=(y*w+x)*3
    return b[k],b[k+1],b[k+2]


def is_green(c):
    return c[0]<52 and c[1]>218 and 22<c[2]<130


def is_magenta(c):
    return c[0]>218 and c[1]<55 and c[2]>185


def runs(xs):
    found=[]
    first=prev=None
    for x in xs:
        if first is None: first=prev=x
        elif x==prev+1: prev=x
        else:
            found.append((first,prev))
            first=prev=x
    if first is not None:
        found.append((first,prev))
    return [(a,b) for a,b in found if b-a+1>=2]


def measure(image,offset,kind):
    ox,oy=offset
    line_centers=[]
    horizontal=[]
    for row in SAMPLE_Y:
        y=oy+row
        candidates=(x for x in range(ox+200,ox+630)
                    if is_green(rgb(image,x,y)))
        clusters=runs(candidates)
        if len(clusters)!=2:
            raise RuntimeError(f"M30T_{kind}_VERTICAL_FIDUCIAL_MISSING "
                               f"row={row} runs={clusters[:8]}")
        xs=[(a+b)/2.0-ox for a,b in clusters]
        if not (100<xs[1]-xs[0]<235):
            raise RuntimeError(f"M30T_{kind}_FIDUCIAL_PAIR_AMBIGUOUS {xs}")
        line_centers.append(xs)
    for column in SAMPLE_X:
        x=ox+column
        candidates=(y for y in range(oy+500,oy+555)
                    if is_magenta(rgb(image,x,y)))
        clusters=runs(candidates)
        if len(clusters)!=1:
            raise RuntimeError(f"M30T_{kind}_HORIZONTAL_FIDUCIAL_MISSING "
                               f"x={column} runs={clusters[:8]}")
        horizontal.append((clusters[0][0]+clusters[0][1])/2-oy)
    left=[p[0] for p in line_centers]
    right=[p[1] for p in line_centers]
    gap=[p[1]-p[0] for p in line_centers]
    return {
        "left_vertical_drift_px":round(max(left)-min(left),3),
        "right_vertical_drift_px":round(max(right)-min(right),3),
        "vertical_spacing_variation_px":round(max(gap)-min(gap),3),
        "horizontal_drift_px":round(max(horizontal)-min(horizontal),3),
        "left_at_top_px":round(left[0],3),
        "left_at_bottom_px":round(left[-1],3),
        "right_at_top_px":round(right[0],3),
        "right_at_bottom_px":round(right[-1],3),
        "bar_centers":line_centers,
        "horizontal_centers":horizontal,
    }


def straight(result):
    return (result["left_vertical_drift_px"]<=3.0
        and result["right_vertical_drift_px"]<=3.0
        and result["vertical_spacing_variation_px"]<=3.0
        and result["horizontal_drift_px"]<=3.0)


def diagnose(native_file,compositor_file,json_file):
    raw=measure(read_ppm(native_file,EXPECTED_NATIVE),(0,0),"NATIVE")
    # A compositor shear can shift a vertical reference bar beyond the
    # expected measurement window or crop it entirely. That is a known
    # visual failure and must be classified, not mistaken for missing Qt
    # source geometry. Still always require both actual captured images.
    final_image=read_ppm(compositor_file,EXPECTED_COMPOSITOR)
    try:
        composed=measure(final_image,CANVAS_ORIGIN,"COMPOSITED")
        compositor_missing=False
    except RuntimeError as exc:
        if "FIDUCIAL_MISSING" not in str(exc) and (
                "FIDUCIAL_PAIR_AMBIGUOUS" not in str(exc)):
            raise
        composed={"error":str(exc)}
        compositor_missing=True
    native_straight=straight(raw)
    composed_straight=not compositor_missing and straight(composed)
    placement=not compositor_missing and all(abs(x-y)<=3 for x,y in zip(
        (raw["left_at_top_px"],raw["right_at_top_px"],
         raw["left_at_bottom_px"],raw["right_at_bottom_px"]),
        (composed["left_at_top_px"],composed["right_at_top_px"],
         composed["left_at_bottom_px"],composed["right_at_bottom_px"])))
    if not native_straight:
        classification="NATIVE_QT_RENDERER_SKEW"
    elif not composed_straight or not placement:
        classification="COMPOSITOR_CROP_OR_TRANSFORM_SKEW"
    else:
        classification="CALIBRATION_AXIS_ALIGNED"
    result={
        "schema":"post-apollo.company-map.m30t-pixel-geometry-v1",
        "source":"same_frame_approximation_static_CI_Qt_fiducials",
        "native_qt_geometry":raw,
        "final_swaypx_geometry":composed,
        "native_straight":native_straight,
        "composited_straight":composed_straight,
        "compositor_reference_bars_missing":compositor_missing,
        "native_to_compositor_placement_within_3px":placement,
        "classification":classification,
        "production_authority":"OFF",
    }
    json_file.write_text(json.dumps(result,indent=2,sort_keys=True)+"\n")
    print("SD_COMPANY_M30T_PIXEL_GEOMETRY "
          f"source_vertical_drift={raw['left_vertical_drift_px']:.2f} "
          f"composited_vertical_drift={composed.get('left_vertical_drift_px','NOT_MEASURABLE')} "
          f"native_straight={int(native_straight)} "
          f"composited_straight={int(composed_straight)} "
          f"placement={int(placement)} "
          f"classification={classification}",flush=True)
    if classification!="CALIBRATION_AXIS_ALIGNED":
        raise RuntimeError("M30T_DISTORTION_PRESENT_DO_NOT_ACCEPT")
    print("SD_COMPANY_M30T_PIXEL_FIDELITY native_and_compositor_straight=1 "
          "no_shear=1 same_coordinates=1 PASS",flush=True)


def synthetic(skew=0):
    w,h=1493,788
    arr=bytearray(bytes((27,6,35))*(w*h))
    def paint(x,y,color):
        p=(y*w+x)*3
        arr[p:p+3]=bytes(color)
    for y in range(170,531):
        shift=round(skew*(y-170)/360)
        for x in (*range(305+shift,311+shift),
                  *range(462+shift,468+shift)):
            paint(x,y,(4,255,50))
    for x in range(305,469):
        for y in range(525,531):
            paint(x,y,(255,13,255))
    return w,h,memoryview(arr)


def selftest():
    base=measure(synthetic(),(0,0),"SELFTEST")
    skew=measure(synthetic(skew=35),(0,0),"SKEW_SELFTEST")
    if not straight(base) or straight(skew):
        raise RuntimeError("M30T_SELFTEST_FAILED_TO_DETECT_SHEAR")
    print("SD_COMPANY_M30T_SELFTEST straight_rectangles=PASS "
          "35px_shear_must_fail=PASS",flush=True)


if __name__=="__main__":
    try:
        if len(sys.argv)==2 and sys.argv[1]=="--selftest":
            selftest()
        elif len(sys.argv)==4:
            diagnose(Path(sys.argv[1]),Path(sys.argv[2]),Path(sys.argv[3]))
        else:
            raise RuntimeError("M30T_ARGUMENTS_EXPECT_SELFTEST_OR_TWO_PPMS_AND_JSON")
    except Exception as exc:
        print("SD_COMPANY_M30T_GEOMETRY_REFUSED:",exc,
              file=sys.stderr,flush=True)
        raise SystemExit(1)
