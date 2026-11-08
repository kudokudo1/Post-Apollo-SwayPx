# 🏥💜 SwayPX Company M30w // Real selected native wlroots buffer metadata, CI ONLY

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Company MapLibre / native Wayland buffer evidence  
DOCTOR // Nurse 2 · external Hospital-compatible successor  
PROVIDER // OPEN AI // CHATGPT  
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, not live registered  
ASSIGNMENT // Observe actual selected native buffer pixel format/pitch from compositor in disposable CI  
PHASE // Source-gated read-only witness, no trusted production actor admission  
BASE // SwayPX M30t draft head `a965305bea8a3aae7d0c9efda2e498377286e456`  
OWNERSHIP // SwayPX Company CI-only native crop test path. Stage Director Social production pairing untouched.

## IMPLEMENTATION

Company M30u [PR #1544](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1544)
and M30v [PR #1549](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1549)
are merged. M30u records a candidate 1493×788 logical Company viewport
and 1494×788 test-only physical source; M30v measures *Qt CPU QImage
readback* bytes per line, but deliberately keeps actual Wayland buffer
format/stride/modifier unknown. The Stage Director worker recorded the
[ownership boundary](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1544#issuecomment-6724674448)
before any runtime linking.

M30w adds read-only inspection at a **different, more authoritative**
location: the actual imported native scene buffer held by wlroots for
the same kernel-checked Qt MapLibre root surface inside the isolated
Company CI compositor.

- `sway/stage_director_ci_company_crop.c` is compiled **only by the
  disposable `-Dsd-ci-company-crop-probe=true` test compositor**, never
  the normal/prod SwayPX binary. The new M30w observer is additionally
  gated by `STAGE_DIRECTOR_CI_COMPANY_M30W=1` and ALL M30t CI flags.
- The observer is invoked from the existing scene-buffer iteration only
  **after** the current Company-native actor has passed the existing
  kernel-selected process/executable and *real mapped host* checks.
  It additionally verifies the scene's root
  `wlr_scene_surface->surface == sway_view->surface`, the exact
  `wl_client` PID/UID of the mapped actor, the 1494×788 physical buffer
  and 1493×788 logical Canvas, and a per-map CI epoch.
- It asks wlroots `wlr_buffer_get_dmabuf()` or
  `wlr_buffer_get_shm()` for the imported buffer's actual allocation
  kind, FourCC pixel format, dimensions, plane-0 pitch, number of
  planes, DMA modifier when available, and actual surface scale /
  transform. **Neither source uses Qt QImage::bytesPerLine()**.
- The log contains **no file descriptors, buffer pointers, image
  content, Company or Social data, user location, personal credentials,
  or arbitrary IPC**. If the imported buffer cannot expose format and
  pitch, it emits `REFUSED kind=UNKNOWN` instead of fabricating values.
  Every report says `production_actor=UNVERIFIED`,
  `scene_authority=NONE` and `input_authority=NONE`.
- `tests/stage_director/ci_company_m30w_buffer_evidence.py`
  requires exactly one selected-root observation, checks actual
  pitch/format/width/height, metadata consistency, scale/transform,
  CI-only identity evidence, and DMA modifier schema when relevant.
  It refuses unknown, missing, malformed, duplicated, wrong-sized,
  title-only, and production-authorized evidence.
- An M30w-flag branch in the already proven M30o native test runner
  writes a bounded JSON report under GitHub `RUNNER_TEMP` only.
  It retains the prior M30t actual Qt-vs-SwayPX pixel-fidelity,
  Wayland pointer/camera, wrong-size hide, PIDFD/XDG and native-close
  checks. Existing M30m/M30n/M30r/M30s/M30t behavior is unchanged
  without M30w flag.
- The standalone `.github/workflows/stage-director-company-m30w.yml`
  compiles and runs the same native MapLibre + Quickshell + SwayPX
  private CI stack, with an independent selftest of all refusals.

## HOW TO USE

Inspect the M30w Actions run. The artifact
`sd-company-m30w-real-native-buffer-metadata` contains the JSON
evidence, native QML/MapLibre screenshots and CI-only runtime log.
This is **not** a production installation command; no local
Fedora/Quickshell/sway restart, account access or online map action
is necessary.

## VERIFY

EXACT-HEAD CI PENDING. It must establish:
1. Existing M30t native MapLibre, original Company QML, and 0-pixel
   Qt/compositor visual geometry fidelity remain green.
2. New C code compiles with the pinned wlroots API in SwayPX's isolated
   Nix test derivation.
3. One real native root buffer is observed and tied to the existing
   kernel-selected mapped native actor (not solely title/app-id).
4. A real imported buffer exposes actual `SHM` or `DMABUF`
   FourCC and plane-0 pitch, not a CPU QImage estimate.
5. Unknown buffer format/stride or mismatched root geometry is
   **REFUSED**, not declared observed.
6. All production/selected-host scene and input authority stays OFF;
   no Social/Stage Director production files change.

## WATCH OUT FOR

Even if this CI-only test records an actual wlroots buffer format/pitch,
it is **not production selected-host admission**. The CI actor identity
uses an exact, pinned test derivation and kernel credentials; the
production ELF digest/custody gate remains empty, and the authority
state is OFF. This does not authorize a global renderer scene,
outside-widget mouse capture, focus redirection or production clipping.

Some GPU/Wayland backends might expose no native SHM/DMABUF metadata at
the specific scene-buffer abstraction. That is an honest **refusal**.
Do not replace missing values with 5976 from Company M30v's CPU image
readback or M30u's calculated stride.

This test covers one headless Mesa software renderer at 1920×1080
with a finite synthetic offline map and one selected native actor.
Multi-monitor scale, HDR/color space, modifiers and actual Fedora
hardware must be separately verified before a production policy exists.

## CHECKLIST

- [x] Verify Company M30v native test/merge and M30u merge
- [x] Reconstruct Stage Director Company/Social ownership and M30t baseline
- [x] Create independent M30w branch based on M30t exact head
- [x] Add CI-only imported native buffer inspector and strict negative parser
- [x] Preserve original native pointer/pixel geometry/close verification
- [ ] Exact-head CI including actual buffer observation
- [ ] Compare with Company M30v Qt CPU readback without conflating sources
- [ ] Coordinate SwayPX production selected-host actor trust work
- [ ] Operator review, merge, Fedora hardware and multi-display acceptance

## NEXT

Run real M30w CI. If wlroots exposes the imported SHM or DMA-BUF,
inspect the actual pixel format and pitch and preserve the metadata
without licensing it as a universal GPU/desktop rule. If not available,
report the exact unsupported boundary and consider a separate,
reviewed **buffer-attach lifecycle** observation point rather than
guessing.

With genuine host-authorized production evidence, a future
SwayPX Company owner can match selected `wl_client`, XDG root,
PIDFD/executable, buffer lifetime/generation, physical backing and
logical Company viewport before evaluating M30u policy. That future
work requires separate owner/operator approval. Social remains under
its independent native pairing owner.

## DECISIONS

M30w is deliberately test-only and strictly **read-only** on native
buffer metadata. It adds no `wlr_scene`/focus/input authorization and
changes no user desktop, production compositor or Social pairing.
The operator retains final acceptance authority.
