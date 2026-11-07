# 🏥💜 Stage Director // M30d real Qt QPA baseline and MapLibre acceptance gap

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre Integration
STAFF ROLE // Nurse 2 · External Hospital-compatible continuation
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL, Hospital-compatible, not live registered
ASSIGNMENT // M30d isolate a real QtWayland/XDG MapLibre-adjacent runtime and preserve a strict evidence boundary
BASE // M30c branch 5cb8c02b28fcfa15e153316deef942a4af944a36 · draft PR #87
DEPENDENCIES // M30a #85 and M30b #86 stacked drafts; SwayPX master remains at M29
PHASE // Headless native Qt/XDG regression; NOT a genuine owner-local MapLibre runtime acceptance

## IMPLEMENTATION

- Added **real Qt 6 QPA Wayland** disposable XDG toplevel (`tests/stage_director/ci_company_qt_wayland_probe.cpp`) built with SwayPX's **locked Nixpkgs Qt6 and QtWayland**. This is a Qt native peer, not an offscreen widget, synthetic XDG mock, screenshot simulation, Quickshell Company or native MapLibre actor.
- Use the existing SwayPX CI-only headless launcher: one unprivileged SwayPX instance, a private `XDG_RUNTIME_DIR`, `WLR_BACKENDS=headless`, locked Mesa EGL software rendering and narrowly reversible `/dev/udmabuf` ACL when required. The path refuses execution outside a GitHub Actions disposable runner.
- Verify that **two distinct real Qt Wayland processes** map native XDG toplevel windows concurrently. Read actual PIDs and titles from Sway IPC **for observation only**, compare against the launched Qt processes, and obtain independent Linux `pidfd_open` handles. Parse each kernel fdinfo `Pid` and nonzero `/proc/PID/stat` process start ticks.
- Deliberately imitate the `CompanyVectorProbe` app identity in a Qt XDG window, verify it **cannot** cause a private Company host selection or M30c actor approval without the physical M24 ceremony and M28v2 child receipt.
- Terminate the first Qt app, show that the other remains mapped, terminate the second, verify both XDG roots disappear. The existing SwayPX smoke also closes and reaps the headless compositor.
- No production scene, input, clip, focus, popup, private global or automatic launcher activation. No local Fedora installation/Quickshell restart.
- The test **does not pretend to test the original MapLibre ELF**. Source for the operator's `~/postapollo-vector-lab/company-vector-build/company-vector-probe` executable was not found in the checked public GitHub repositories; the existing `The-Post-Apollo-Project` code only provides a launcher and attached process callback. Reproducing its real QtLocation/MapLibre bundle, Quickshell's native layer host and authenticated v2 bridge is still an independent acceptance requirement.

## HOW TO USE

No desktop actions. CI automatically builds the disposable Qt client and compositor when this M30d branch is proposed for review. The user's Company → Coverage → VECTOR VIEW remains standalone; the private native bridge stays disabled.

## VERIFY

- Source-only shell safety and artifact-type checks.
- Real Qt/QPA Wayland app+XDG process PID matching in a headless SwayPX **running compositor**, not a mock server.
- Real `pidfd_open`/fdinfo kernel PID matching and process-start-tick verification for two concurrent Qt processes.
- Root disappearance after each app exits, including a sibling that remains running; no admission on a spoofable app ID without physical selection.
- Device ACL restoration by existing CI helper on success and failure.
- Precise test labels: `REAL_QT_QPA` does not imply MapLibre, bridge, consent or actor custody.

## WATCH OUT FOR

- Headless CI has no physical libinput gesture and cannot authenticate the M24 native Company host. There is **no** positive selected-host/Wayland v2 child fd/QtMapLibre end-to-end proof in this milestone. Previous M30b tests cover real pidfd + synthetic Wayland peer resources, not actual Qt process paired to selected Company.
- The true MapLibre binary comes from an owner-local build not included in this repository. Do not substitute a green Qt QPA check for trusted ELF manifest, native Qt plugin dependency integrity, map pixels, seat or popup containment.
- Sway IPC titles/app IDs are used strictly to locate **test actors**, never to authorize pairing; the native PID is checked independently through the kernel. This fixture deliberately does not bind any private Company protocol.
- The GitHub runner's software EGL/device setup can block without indicating a production MapLibre error; distinguish CI environment from actual Fedora system.
- Production activation, merge order and final acceptance still require operator authorization.

## CHECKLIST

- [x] Review M30a–M30c stacked branches and original headless native Wayland tests
- [x] Identify missing owner-local MapLibre ELF/artifact source
- [x] Write pinned Nix Qt/QPA Wayland fixture, two-root pidfd smoke and bounded teardown
- [x] Preserve CI-only device/renderer sandbox, clear evidence limits
- [ ] Verify exact M30d PR-head CI
- [ ] Obtain a reviewable MapLibre native build artifact/reproducible recipe under appropriate owner authority
- [ ] Exercise real selected Company layer + Qt QPA bridge + M28 v2 pidfd birth receipt together
- [ ] Add real Qt MapLibre pixel, popup, keyboard and pointer isolation checks
- [ ] Operator approval for digest, merge sequence, desktop activation and final acceptance

## NEXT

Check exact-head CI and repair any observed native runtime issues in the disposable runner. For an **actual MapLibre M30d acceptance**, obtain an independently reproducible native Qt+MapLibre build and a CI-only physical selection test path (never automatic production admission), or use an explicitly authorized operator-local isolated test with clear rollback. Continue without assuming the locally installed binary is approved.

## DECISIONS

Nurse 2 retains only the Company MapLibre integration lane. No merges, local machine changes, installed compositor restart, default-on native bridge, or scope expansion into Weather/Chrome. This is evidence for a real Qt Wayland transport baseline **only**, not completed MapLibre embedding.
