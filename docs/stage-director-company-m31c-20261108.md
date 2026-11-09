# 🏥💜 Hospital // Company M31c native corrected renderer candidate

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre native pixel fidelity and visually pullable preview
DOCTOR // Nurse 2 external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, no confirmed live Hospital registration
ASSIGNMENT // Prove generalized, stride-aware SceneFX source patch against the actual 1493px native Company app and preserve screenshots
PHASE // Isolated CI source candidate, not production adoption
STATE // CI PENDING
REPORT KIND // IMPLEMENTATION
PERMISSIONS // Dedicated GitHub branch, draft PR, CI only. No local Fedora write, Social source change, production runtime switch or merge.

## IMPLEMENTATION

M31a [PR #120](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/120) showed the original 1493px MapLibre/SHM RGB565 native surface sheared **-1 px per row** with unmodified pinned SceneFX and rendered **0 px** drift when a separately built SceneFX saved/set/restored `GL_UNPACK_ALIGNMENT=1`. M31b [PR #121](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/121) established that a *universal* alignment=1 is unsafe for RGB888 padded sources and passed a real 90-case Mesa GLES3 texture create/update, stride-layout and state-restoration matrix.

M31c patches a **private, pinned SceneFX Nix build only**, using the existing M31a strict-source-copy patcher plus a checked layout selector based on `stride`, `width` and the DRM format's bytes-per-block. It validates the actual measured byte pitch can be expressed exactly using a pixel row length and GL unpack alignment from 1,2,4,8. It refuses unrepresentable layouts before texture creation/context mutation; update failures end buffer CPU access. Actual creation and update uploads save and restore prior GL alignment. The DMABUF path and default `flake.lock` remain unchanged.

The CI retains real negative-control original 1493px MapLibre/QQuickWindow/Wayland screenshots and patched 1493px positive images, plus the corrected 1494px zero-drift reference. The native runtime must observe the existing M31a patch witness and the new `M31C_SCENEFX_STRIDE_AWARE_CI_ONLY` marker for the exact tested upload.

## HOW TO USE

Review the draft M31c GitHub Actions workflow and its `sd-company-m31c-scenefx-stride-aware-maplibre-preview-evidence` image artifact. The artifacts are visually inspectable in GitHub Actions, not executable Fedora desktop software. No changes to the operator machine or live Company menu.

## VERIFY

Required: strict patcher selftest; build against pinned SceneFX; actual native renderer/map and real Wayland selection; reproduce original -1px/row control; obtain 0px/row patched original image; preserve reference 1494px and native input/camera/resize/close; enforce test-only ownership and source. Previous M31b 90-case GLES matrix already passed and remains separate.

## WATCH OUT FOR

No production/host authorization follows from a passing CI test. The standalone source candidate intentionally **refuses** strides not expressible in GL pixel-store settings; no generic staging/repacking behavior is authorized. This is not automatically a native binary compatible with Fedora's ABI, nor a Company panel embed. Social Stage Director ownership and production SceneFX remain untouched.

## CHECKLIST

- [x] Verify M31a/M31b current draft status and exact verified heads
- [x] Preserve branch and owners; use private pinned SceneFX source only
- [x] Implement checked, exact measured-stride plan for both CPU texture routes
- [x] Add separate native CI workflow with actual MapLibre screenshot evidence
- [ ] Pass exact-head native M31c CI and runtime patch witness
- [ ] Package a safe user-pullable standalone preview with local preflight
- [ ] Operator-authorized Fedora Toolbox visual test
- [ ] SceneFX upstream owner approval, production integration and operator final acceptance

## NEXT

Inspect M31c native CI output, then make a Company-owned independent-preview path that uses the already existing Fedora Toolbox MapLibre lab without modifying live Quickshell. Do not mistake downloadable CI screenshots for a live embedded Company map.

## DECISIONS

Unmerged, CI-only candidate; no production SceneFX change, no local machine changes, no merge or approval to alter Social. Operator final authority retained. Rollback is branch deletion or abandoning the draft.
