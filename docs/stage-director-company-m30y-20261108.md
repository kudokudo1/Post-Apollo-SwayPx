# 🏥💜 SwayPX Company M30y // Direct original-versus-corrected native imported-buffer measurement

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre imported-buffer pixel fidelity
STAFF ROLE // Nurse 2, external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, no verified live registration
ASSIGNMENT // Measure the previously distorted 1493px imported native buffer instead of inferring its pitch from the corrected 1494px buffer
PHASE // Disposable CI-only two-session experiment
BASE // M30w exact head `bf3be2236ebedb620bd16ef8a98c10aaa3f94f30`
OWNER // SwayPX Company isolated CI harness only; Social production host/pairing untouched
PERMISSIONS // GitHub branch, draft PR, tests; no production scene changes, local machine write or merge.

## IMPLEMENTATION

M30t [PR #116](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/116) measured no drift when a 1494×788 physical Qt buffer was clipped into the original 1493×788 visible Company Canvas. M30w [draft PR #117](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/117) independently inspected the real imported root `wlroots` buffer of that corrected source: SHM RGB565 (`0x36314752`), 1494×788, measured row pitch 2988, scale 1. Company M30x [PR #1561](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1561) reconciled this with its distinct Qt CPU RGBA readback and has merged.

**Remaining uncertainty:** M30w never measured the physical imported native buffer of the **original 1493px distorted source**. Do not assume its stride was 2986, 2988 or anything else. Nor can a single corrected CI sample certify a universal driver/GPU alignment defect.

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

The baseline M30w exact-head native test previously passed [run #42513247422](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42513247422). M30y CI is pending and **must**:

1. Compile only the CI probe into a disposable SwayPX compositor; production binary must not contain M30y markers.
2. Preserve the padded 1494×788 M30w real native buffer witness, MapLibre input and M30t zero-skew tests.
3. In a separately booted, identically pinned CI session, obtain exactly one imported root `wlroots` buffer at 1493×788 linked to the kernel-matched CI native actor and same Company Canvas.
4. Record actual native 1493px SHM RGB565 pitch without assigning a target pitch before observing it.
5. Independently compare both real imported buffers and refuse unreviewed formats, scales, authority, missing/ambiguous evidence, or invalid pitches.
6. Preserve original source and CI-only authority restrictions.

**Failure to observe** is a failed experiment and never permission to fill the missing pitch with Qt QImage bytesPerLine or arithmetic.

## WATCH OUT FOR

These are two separate ephemeral native processes, not one identical live actor. Their sameness is limited to pinned source, renderer, compositor build and output profile. One headless Mesa/Wayland test is not a universal rule for Intel/AMD/NVIDIA, Wayland DMA-BUF, fractional scaling or the operator's multi-monitor Fedora desktop.

Even observing identical strides does not prove the compositor's exact pixel-shear root cause. Geometry, pitch, source cropping and compositor texture upload must be correlated with the reference-line screenshots in a later authorized design; do not rewrite the production compositor based on one comparison.

## CHECKLIST

- [x] Verify current M30w, Company M30x, Syntagma, and Social ownership
- [x] Keep M30w branch unchanged and make an independent M30y branch
- [x] Add CI-only original imported-buffer observer, root/identity validation and timeout
- [x] Add parser, negative tests, paired report, separate session and artifact workflow
- [ ] Exact-head M30y CI and real 1493px imported stride observed
- [ ] Compare actual 1493 vs 1494 imported buffers and preserve screenshots
- [ ] SwayPX Company selected-host production trust/custody contract (separate)
- [ ] Fedora GPU, multi-output and scale verification under explicit local approval
- [ ] Operator review and merge authorization

## NEXT

Run M30y exact-head CI; classify the actual original stride/format and pixel evidence. If this succeeds, hand both native observations to the Company/SwayPX integration owners to design an authenticated production selected-host crop/input implementation. Keep Social Stage Director production pairing and user hardware untouched.

## DECISIONS

No merge, runtime install, Shell reload, compositor restart, live Company bridge opt-in, online tiles or production selected actor granted. Operator final DONE remains separate.
