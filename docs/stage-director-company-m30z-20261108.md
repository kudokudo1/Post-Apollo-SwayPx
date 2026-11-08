# 🏥💜 SwayPX Company M30z // Original vs corrected MapLibre pixel origin

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre pixel fidelity / rendered-frame localization
DOCTOR // Nurse 2, external Hospital-compatible successor
STAFF ROLE // NRS-2
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE (no live Hospital registration verified)
ASSIGNMENT // Compare actual original and corrected Qt source/compositor fiducials and locate visual distortion without authorizing production compositor surgery
PHASE // Disposable CI-only investigation
STATE // IMPLEMENTED; EXACT-HEAD CI PENDING
REPORT KIND // IMPLEMENTATION CHECKPOINT
BASE // M30y draft PR #118 exact head `5def7be1270e8e8966862cf53082c617e6f5a988`
OWNERSHIP // SwayPX Company test-only fidelity and source-copy runner; no Social or production compositor actor ownership
PERMISSIONS // GitHub branch, draft PR, automated tests, no merge, no Fedora or live Quickshell changes

## IMPLEMENTATION

[Company M30x #1561](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1561) merged its correct separation of Qt CPU readback versus real wlroots native pixel format. [SwayPX M30w #117](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/117) remains draft; M30y [draft #118](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/118) directly measured both source sizes with 4/4 checks green at `a5854346e2`: original SHM RGB565 1493×788 at **2986 measured bytes/row**, corrected SHM RGB565 1494×788 at **2988 measured bytes/row**. Both have zero physical row padding. The corrected 1494px compositor image has zero vertical drift. The original source's pixel shear was previously reported but *not quantified with identical fiducials in that run*.

M30z adds a **disposable test-copy** fiducial source at the same exact QML coordinates for both the original 1493px and corrected 1494px source. It retains M30t's original corrected Qt readback and adds an independently guarded QQuickWindow::grabWindow() readback for the original width in the second, already isolated M30y compositor session. Each real Wayland screenshot is captured after its Qt source evidence is present.

- `tests/stage_director/ci_company_m30t_prepare.py` extends only the prepared test copy (under RUNNER_TEMP), not the owner-managed native MapLibre app, and keeps M30t's original code path unchanged when M30z is disabled.
- `tests/stage_director/ci_company_m30o_real_resize.py` archives an original native Qt PPM and compositor PPM after genuine selected actor and root-surface checks. M30y process, separate QML-copy path, and native resize/input/close gates remain.
- `tests/stage_director/ci_company_m30z_pixel_origin.py` measures green vertical/magenta horizontal fiducials in both sources and final frames using the earlier M30t geometry algorithm. It reports source drift, compositor drift, placement, missing markers, and whether the corrected reference remains straight; explicitly records no certified shear cause or production authority.
- `.github/workflows/stage-director-company-m30z.yml` runs source/grammar/selftest gates and separately booted corrected and original genuine Wayland/MapLibre sessions, paired real imported-pitch proof, and the final pixel-origin comparison. Artifacts retain both source/compositor PPMs, actual wlroots metadata, summaries, and diagnostics.

The original Company QML checkout remains immutable; the normal SwayPX binary must not contain the CI-only observer; Social's GDM/Vesktop pairing is not modified. No user photo, email, location or online tiles are touched.

## HOW TO USE

Review the draft PR's CI and `sd-company-m30z-original-vs-corrected-pixel-fidelity` artifact:

- `sd-company-m30z-original-native-fullscreen.ppm`: Qt source pixels at 1493×788.
- `sd-company-m30o-m30z-original-composited-fullscreen.ppm`: actual headless SwayPX output of the same test markers, 1920×1080.
- `sd-company-m30t-native-fullscreen.ppm`: previously passing corrected 1494×788 Qt source pixels, freshly captured in same workflow's first session.
- `sd-company-m30y-corrected-1494-fullscreen.ppm`: 1920×1080 corrected Wayland compositor pixels, saved before second session.
- `sd-company-m30z-real-pixel-origin.json`: computed geometry classifications and explicit limitations.
- M30w/M30y JSON imported native buffer details: original **2986**, corrected **2988** actual bytes/row.

Nothing to install or enable on the operator desktop.

## VERIFY

Prior [M30y native CI #42519372425](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42519372425) has passed. M30z must independently prove both sources contain **straight fiducial bars** before comparing with actual composited pixels; corrected final screenshot must also be straight. If original final bars are measurable and distorted, classify them as compositor-origin distortion; if unmeasurable, explicitly report unmeasurable, **not** a diagnosed shear. The experiment must not silently accept missing original Qt bars or unknown native pixel format/stride.

Run the existing native resize, real pointer pan/zoom, clip, fullscreen, restore, and clean-close gates in both independent sessions. SwayPX production compositor must have no M30z flag and no touched release files.

## WATCH OUT FOR

M30z can **localize** an original-source vs compositor-side difference but does not, alone, certify which function is at fault (upload pitch, shader sampling, source cropping, transform). Earlier M30t output showed the corrected view aligned; this is still a CI-only software Mesa reference, not Fedora physical GPU/multi-output validation.

Because both real imported buffers were tightly packed RGB565, a "padding fixed it" claim is unsupported. There is a 4-byte divisibility difference (`2986 mod 4 = 2`, `2988 mod 4 = 0`); its role in any renderer-upload path needs controlled testing before production design.

If the original composited fiducials cannot be measured, the report must state that explicitly rather than manufacturing a zero drift.

## CHECKLIST

- [x] Recheck active M30w/M30y Company state, Syntagma and ownership
- [x] Preserve M30y previous branch and base a fresh M30z branch
- [x] Prepare the same test-only original and corrected source reference bars
- [x] Add original Qt source capture and actual compositor screenshot
- [x] Add bounded source/compositor geometry classifier and negative selftests
- [x] Add independent GitHub native CI workflow and artifacts
- [ ] Verify exact-head M30z workflow, including actual native MapLibre build, image measurements and regressions
- [ ] Classify where original shearing appears and correlate independent imported buffer pitch
- [ ] Review SwayPX Company selected-host production identity/scene/input with owning team
- [ ] Operator-authorized local Fedora GPU/multi-output verification
- [ ] Explicit operator merge and final visual acceptance

## NEXT

Inspect exact-head M30z CI and artifact JSON; if the original input bars are measurably straight in Qt and sheared in compositor, run an even narrower experiment isolating the offending crop/destination/texture stage. If the pixels cannot be measured, refine the **CI-only** fiducials or screenshot timing without changing source ownership or production.

## DECISIONS

Source-only CI experiment and draft PR; no production authorization, no live desktop or Social system mutation, no merge, no final DONE. Original data remains from pinned Company M30q source.
