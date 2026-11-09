# 🏥💜 SwayPX Company M30y // Direct original-versus-corrected native imported-buffer measurement

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre imported-buffer pixel fidelity
STAFF ROLE // Nurse 2, external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, no verified live registration
ASSIGNMENT // Measure the previously distorted 1493px imported native buffer instead of inferring its pitch from the corrected 1494px buffer
PHASE // Isolated two-session native evidence measured; production integration pending
BASE // M30w exact head `bf3be2236ebedb620bd16ef8a98c10aaa3f94f30`
OWNER // SwayPX Company isolated CI harness only; Social production host/pairing untouched
PERMISSIONS // GitHub branch, draft PR, tests; no production scene changes, local machine write or merge.

## IMPLEMENTATION

M30t [PR #116](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/116) measured no drift when a 1494×788 physical Qt buffer was clipped into the original 1493×788 visible Company Canvas. M30w [draft PR #117](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/117) independently inspected the real imported root `wlroots` buffer of that corrected source: SHM RGB565 (`0x36314752`), 1494×788, measured row pitch 2988, scale 1. Company M30x [PR #1561](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1561) reconciled this with its distinct Qt CPU RGBA readback and has merged.

**Historical uncertainty resolved by M30y:** M30w had measured only the corrected 1494px buffer; M30y now directly measured the original unpadded 1493px buffer at **2986 actual bytes/row** in a second private compositor session. The corrected 1494px buffer remains **2988 actual bytes/row**. Both are SHM RGB565 (16-bit), and both measured pitches equal packed width × 2 with **zero extra row-padding bytes**. The measured 2-byte pitch increase is explained by the one extra pixel per row. The original pitch is not divisible by 4 and the corrected pitch is, which is *compatible* with an alignment-sensitive bug, but it does not establish the actual source of the diagonal pixels or certify a universal pitch rule.

This M30y source-only extension retains M30w's unchanged corrected run and introduces a **separate isolated compositor boot** with `STAGE_DIRECTOR_CI_COMPANY_M30T=0`, `M30W=0`, `M30Y=1`. It runs the same pinned MapLibre Qt, original merged Company QML, SwayPX headless Mesa, Quickshell, exact kernel-matched Wayland root and 1920×1080 virtual output. Unlike M30t, it preserves the **original 1493×788 native source** without a padding column.

- `sway/stage_director_ci_company_crop.c`: strictly test-only M30y observer, using the selected root surface and current wlroots SHM/DMABUF imported buffer metadata. It logs actual width/height, FourCC, plane pitch, planes, scale and transform, while deliberately not outputting memory addresses, FDs or pixels.
- `tests/stage_director/ci_company_m30o_real_resize.py`: captures the original compositor log, validates the exactly selected 1493px native root and archives one JSON report. Existing camera, pointer, hide, resize and close checks still run.
- `tests/stage_director/ci_company_m30y_original_evidence.py`: validates a real 1493px imported SHM RGB565 root without **preselecting its row pitch**, checks packed minimum 2986 and records observed padding. It then compares independent original and corrected reports, refusing mismatched formats, scale, selection and any production authority.
- `tests/stage_director/headless_sway_smoke.sh`: CI-only baseline gate explicitly refuses combining M30y with padded M30t/M30w.
- `.github/workflows/stage-director-company-m30y.yml`: separate draft-target CI workflow; runs corrected M30w first, preserves its JSON and screenshot, then executes a new unpadded compositor session and compares the measured two buffer reports.

This does not change production `sway/stage_director.c`, Social, Company original QML, its native app, current native child, display config, or any local Fedora environment. Neither session receives production scene/input permission.

## HOW TO USE

Run the M30y GitHub Actions workflow on the draft branch. Review artifact `sd-company-m30y-original-vs-corrected-wayland`, particularly:

- `sd-company-m30w-real-buffer-evidence.json` (actual **padded** imported buffer)
- `sd-company-m30y-original-native-evidence.json` (actual **unpadded** imported buffer)
- `sd-company-m30y-paired-native-evidence.json` (observed pitch difference, without claiming root-cause certainty)
- corrected and original compositor screenshots and bounded synthetic logs.

Nothing to install or activate on the operator's machine.

## VERIFY

**M30y exact-head CI succeeded at commit `a5854346e265e8cae1ee7a43e3f4bf6d0705dedc`**: [Actions run #42519372425](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42519372425), four of four checks passed. The first attempt [#42514943733](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42514943733) had failed at the **second session's reused QML copy directory**, before any original buffer report. M30y isolated the original session's QML copy and HOME/cache under new private paths, leaving prior tests' paths unchanged; the successful rerun passed both native sessions.

Actual observed compositor log records in successful run:

| Test-only native root | Format | Real imported stride | Row padding | Canvas |
|---|---|---:|---:|---|
| Original unpadded `1493×788` | SHM RGB565 (FourCC `0x36314752`) | **2986 bytes** | 0 bytes | `1493×788` |
| Corrected `1494×788` | SHM RGB565 (same FourCC) | **2988 bytes** | 0 bytes | `1493×788` |

The original observation reports `SD_COMPANY_M30Y_ORIGINAL_BUFFER ... physical=1493x788 pitch=2986 ... PASS`. The corrected observation reports `SD_COMPANY_M30W_REAL_BUFFER ... source=1494x788 pitch=2988 ... PASS`. Paired parser records `original_measured_pitch=2986 corrected_measured_pitch=2988 cause_certified=NO`. The corrected M30t pixel-fidelity diagnostic measured **0.0-pixel vertical drift**, real Qt/Wayland pan and zoom, root PIDFD/XDG continuity, invalid-size refusal, clean close, and unmodified original Company QML. **The first/original visual shear is historical M30t evidence; this M30y comparison deliberately did not implement a new pixel-shear quantifier for the original unpadded capture.**

All acceptance checks passed on the reviewed code:

1. Compile only the CI probe into a disposable SwayPX compositor; production binary must not contain M30y markers.
2. Preserve the padded 1494×788 M30w real native buffer witness, MapLibre input and M30t zero-skew tests.
3. In a separately booted, identically pinned CI session, obtain exactly one imported root `wlroots` buffer at 1493×788 linked to the kernel-matched CI native actor and same Company Canvas.
4. Record actual native 1493px SHM RGB565 pitch without assigning a target pitch before observing it.
5. Independently compare both real imported buffers and refuse unreviewed formats, scales, authority, missing/ambiguous evidence, or invalid pitches.
6. Preserve original source and CI-only authority restrictions.

**Failure to observe** would have been a failed experiment, never permission to fill a missing pitch with Qt QImage bytesPerLine or arithmetic. Here both rows were actually observed.

## WATCH OUT FOR

These are two separate ephemeral native processes, not one identical live actor. Their sameness is limited to pinned source, renderer, compositor build and output profile. One headless Mesa/Wayland test is not a universal rule for Intel/AMD/NVIDIA, Wayland DMA-BUF, fractional scaling or the operator's multi-monitor Fedora desktop.

The measured row strides are **different by exactly two bytes**, but each matches its own tightly packed RGB565 width, and neither has physical row padding. A 4-byte alignment-sensitive upload or crop error remains a possible explanation because 2986 is 2 modulo 4 while 2988 is divisible by 4. The data do **not** demonstrate that adding extra row-padding bytes fixed the skew. Geometry, actual upload stride, texture interpretation, clipping and screenshot fiducials need controlled, independent follow-up before production surgery.

## CHECKLIST

- [x] Verify current M30w, Company M30x, Syntagma, and Social ownership
- [x] Keep M30w branch unchanged and make an independent M30y branch
- [x] Add CI-only original imported-buffer observer, root/identity validation and timeout
- [x] Add parser, negative tests, paired report, separate session and artifact workflow
- [x] Exact-head M30y CI passed; real original imported stride = 2986 bytes
- [x] Original 1493 vs corrected 1494 imported buffers measured; paired evidence and screenshots archived in GitHub Actions
- [ ] SwayPX Company selected-host production trust/custody contract (separate)
- [ ] Fedora GPU, multi-output and scale verification under explicit local approval
- [ ] Operator review and merge authorization

## NEXT

**Native evidence milestone complete.** Hand the measured RGB565 native 1493/2986 and 1494/2988 profiles plus the 0px corrected reference-fiducial proof to the SwayPX Company selected-host integration owner. The next bounded engineering experiment should correlate the **original source's pixel shear** with import pitch / shader texture upload / clipping, without confusing observed width-dependent packed pitch with padding; after that design authenticated production selected-host crop/input. Keep Social Stage Director production pairing, the operator's desktop and local Fedora untouched.

## DECISIONS

No merge, runtime install, Shell reload, compositor restart, live Company bridge opt-in, online tiles or production selected actor granted. The actual native pitches are **observations**, not authorization for a 4-byte alignment rule or permanent one-column padding. Operator final DONE remains separate.
