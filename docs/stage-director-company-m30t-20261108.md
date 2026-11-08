# 🏥💜 Stage Director / Company M30t // Native pixel geometry fidelity

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Company Native MapLibre / Pixel Geometry  
STAFF ROLE // Nurse 2, external Hospital-compatible doctor  
PROVIDER // OPEN AI // CHATGPT  
ASSIGNMENT // Identify and refuse skewed straight edges from actual Company MapLibre CI screenshot  
BASE // SwayPX Company M30s merged into M30l at `89f2456b630dd9b364342252da684d4db6eecf79`  
SOURCE // M30q original Company QML `877eb00d0f260f9fdda52712f35abec59b2f83b3`  
CONNECTION // EXTERNAL; no verified live Hospital session or runtime ownership  
PERMISSIONS // CI-only test instrumentation + GitHub branch. No Social Stage Director production edits, user-local files, SwayPX host actor grants, merges or deployment.

## IMPLEMENTATION

The operator inspected the [M30s green fullscreen screenshot](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42494435717)
and correctly reported diagonals where straight verticals are expected.
We inspected the exact downloaded compositor image: *the native Qt QML
camera rail's rectangular buttons are parallelograms*, and some footer
edges are oblique. The original Company frame outside the map has
straight edges. In the original MapLibre Qt QML source, the camera rail
uses ordinary untransformed `Rectangle` items, so perspective/skew
in the displayed image is a **real visual defect**, not evidence of
slanted real streets.

M30s proved genuine pixels/drag/zoom/fullscreen/crop/identity and ran
against the original merged Company QML **without** temporary geometry
repair. It never tested whether native Qt's axis-aligned primitive edges
remained axis-aligned *after rendering and compositing*.

M30t adds a diagnostic, **not a guessed compositor or Qt bug fix**:

- `ci_company_m30t_prepare.py` reuses exact source-pinned M30s and
  M30o native preparation, adds two luminous **true vertical Qt
  Rectangles** and one horizontal Rectangle to a disposable Qt QML
  source **only when fullscreen source size is 1493×788**.
- The exact same test Qt process calls the genuine
  `QQuickWindow::grabWindow()` to save its *pre-compositor* native
  fullscreen source pixels as a bounded PPM file under RUNNER_TEMP.
- M30s's already verified actual Wayland/MapLibre test continues to
  screenshot the resulting *composited* output with `grim`. An M30t
  flag gated in `ci_company_m30o_real_resize.py` compares the two.
  Both share the real Qt source and compositor; the extra fiducials
  don't modify the original Company QML or synthetic GeoJSON.
- `ci_company_m30t_pixel_geometry.py` independently measures left
  and right vertical-bar x drift and their spacing at multiple rows,
  horizontal-bar y drift at multiple columns, and Qt→composited
  registration. Drift over 3 pixels is FAIL. A skewed synthetic bar
  selftest must be detected as a failure.
- The pixel report classifies
  `NATIVE_QT_RENDERER_SKEW` (already skewed in Qt grabWindow),
  `COMPOSITOR_CROP_OR_TRANSFORM_SKEW` (native straight, final
  output skewed), or `CALIBRATION_AXIS_ALIGNED` (both straight).
  Any distortion **fails the CI**, rather than reporting success.
- M30s and M30o unchanged when the M30t-only environment variable
  is absent. No production source or compiled compositor is changed.

## HOW TO USE

Review `.github/workflows/stage-director-company-m30t.yml`
and its retained artifact
`sd-company-m30t-native-compositor-geometry`: the raw Qt window
PPM, actual full Company compositor PPM, and JSON diagnosis.

A red M30t CI is **valuable evidence of a real visual defect**,
not an excuse to disable the pixel fidelity requirement.
No operator action or desktop restart needed.

## VERIFY

PENDING exact-head CI. Required:
1. Source/normal compositor M30s guards intact.
2. Selftest passes for straight reference bars and rejects 35px shear.
3. Qt fullscreen source is genuinely 1493×788.
4. Qt `grabWindow` returns full-sized nonempty PPM of the actual
   test window including the three calibration rectangles.
5. Wayland/Grim compositor returns actual fullscreen 1920×1080
   image; source registration expected at (166,203).
6. Both independent images contain recognizable reference bars;
   measured x/y drift, shape and placement are compared.
7. If warped: test FAILS and JSON identifies where distortion begins.
8. Existing real MapLibre/fullscreen/camera/input checks not weakened.

## WATCH OUT FOR

Qt `grabWindow` may itself be unavailable under software GL/Wayland;
an absent or malformed capture fails the diagnostic and cannot be
interpreted as a renderer skew classification. The permanent root-cause
fix requires corroborating the native/compositor comparison and checking
the exact renderer state/texture path at the responsible boundary.

Do NOT conclude that rendering is visually accepted merely because
M30s's geometry and input tests were green; those tests were insensitive
to shear. Do NOT patch production stage-director shader logic or the
operator's Qt native map from this CI diagnosis alone.

The source map is finite synthetic Flint GeoJSON, not real business
coverage. Single CI 1920×1080 output and Mesa software OpenGL
rendering are not proof of multi-monitor or hardware GPU behavior.
Other workers own Social Stage Director modules, which remain untouched.

## CHECKLIST

- [x] Inspect operator-rejected screenshot: camera rail rectangular edges skewed
- [x] Inspect actual original Qt QML and CI SwayPX crop/destination mapping
- [x] Recheck AGENTS/Hospital protocols and current M30s development merge
- [x] Prepare isolated M30t branch with no ownership collision
- [x] Add exact native Qt capture, physical reference rectangles and dual-image geometry detector
- [x] Add explicit CI-only runtime flag and anti-shear selftest
- [ ] Run exact-head CI and archive source + final image evidence
- [ ] Diagnose true Qt or compositor origin
- [ ] Make reviewed fix in the correct owner's area, avoiding Social
- [ ] Re-run actual native fullscreen and operator screenshot acceptance

## NEXT

Inspect M30t CI result. If the source already contains skew, investigate
MapLibre Qt v3/OpenGLRhi software rendering or QtQuick state restore in
a separate Company-owned renderer task. If the native source is straight
but the final compositor output is skewed, investigate the test-only
SwayPX crop destination texture mapping without touching the Social
production compositor until cross-owner approval.

Keep the original M30s green functional baseline and this separate
visual-fidelity rejection as distinct checkpoint evidence.

## DECISIONS

Visual fidelity is not accepted. Correct dimensions, map presence,
colorful pixels, and working pointer are not sufficient proof that Qt
rectangles stayed rectangular. M30t is intentionally a **fail-closed
source-vs-compositor diagnostic**, not a production patch or a new map.
