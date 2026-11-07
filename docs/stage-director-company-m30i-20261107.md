# 🏥💜 Stage Director // M30i real Quickshell Company layout and viewport geometry

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director · Company MapLibre native integration
STAFF ROLE // Nurse 2 · externally attached Hospital-compatible worker
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL, not verified live Hospital registered
ASSIGNMENT // Replace synthetic Company frame with an actual Quickshell CompanyNetworkW geometry witness
PHASE // M30i source-pinned real Company UI smoke in disposable headless SwayPX
BASE // M30h PR #92 at 44b0711cfe340b694c5c7e2d78ff23be63e18dcd
DEPENDENCIES // M30a–h stacked unmerged draft PRs
COMPANY SOURCE PIN // e3595b0035bb3d89aa87c6ba2bd1aac04a975181 (M30e-reviewed)
AUTHORITY // GitHub-only development, no production/local changes or merges

## IMPLEMENTATION

M30h passed native Qt press, held drag, release, wheel and hidden-overflow input containment. However, M30f–h used a synthetic purple QWidget as the Company frame. M30i tests the **actual source-pinned `CompanyNetworkW.qml` Quickshell PanelWindow** against the unchanged normal SwayPX compositor, using a disposable Nix Quickshell and temporary HOME, with real compositor screenshots.

The real Company source already owns both coordinate transforms:
- `CompanyNetworkW.activeVectorFrameInHostRect` is the actual frame's PanelWindow-relative QQuickItem rectangle.
- `CompanyNetworkW.activeVectorViewportRect` is Company-frame-relative and gated by real Coverage/native process ownership.
- `CompanyCoveragePane.activeVectorViewportRect` derives from the actual `mapCanvas.mapToItem(root,...)` but **must be zero unless a genuine tracked native vector child is running**.
- `CompanyVectorBridgeLink` remains optional and `vectorBridgeOptIn: false` unless separately authorized.

M30i checks out that exact Company commit independently in CI. A Python test requires a clean Git checkout and copies it into a disposable `RUNNER_TEMP` directory. Only the **temporary copy** receives two additive, read-only QML introspection properties: actual mapCanvas rectangle in Coverage-local coordinates, and translated Canvas rectangle in the Company PanelWindow. The original tracked Company files, control logic, `vectorController.running`, OFFLINE/ONLINE consent, launcher, and bridge conditions remain untouched.

The test launches a **real Quickshell ShellRoot** containing the **original CompanyNetworkW** component under actual headless SwayPX, using a separately pinned Nixpkgs Quickshell binary and software Mesa renderer. It opens Company, measures `standard`, `compact`, and `fullscreen` chassis geometry from QML itself, then closes Company and checks revocation. It captures four actual compositor PPM screenshots, requires independent Company-color pixels and correct frame/canvas bounds and zero active MapLibre viewport candidate without an accepted native child. A real MapLibre actor is NOT started or embedded.

This is more faithful than M30f's synthetic Company frame but NOT production embedded MapLibre or the physically selected Wayland layer. It only establishes live real-QML geometry and appearance in an isolated session.

## HOW TO USE

No local machine action. Review the M30i draft and GitHub workflow `stage-director-company-m30i.yml` for actual Quickshell logs and screenshots. The user's existing Company menu and standalone VECTOR VIEW behavior remain unchanged.

## VERIFY

- Exact original Company source from M30e, clean and pinned; original M30e executable/launch contract run before any temporary copies.
- Normal production SwayPX built (no M30g opt-in compositor source-crop selector), no live workstation writes.
- Original CompanyNetworkW on real Quickshell PanelWindow with its real frame geometry and painted UI.
- Candidate native viewport MUST remain zero when the tracked MapLibre child is absent; bridge remains disabled.
- Real Canvas coordinates come from live mapCanvas QQuickItem transforms in the temporary read-only instrumented copy, not hardcoded screenshot positions.
- Three sizes and close must be observed in order; screenshots must show Company painted while open and clear after close.
- Exact-head CI must pass before claiming runtime success.

## WATCH OUT FOR

- The pinned M30e Company snapshot is not the continuously changing `main` branch. Deliberate repinning and source review are needed for any future production handoff.
- The only QML modifications are two read-only diagnostic properties in the *disposable test copy*. This is not proof an unmodified production Company UI published a nonzero authorized MapLibre viewport.
- The genuine Company frontend may depend on optional Qt QML modules or fonts not bundled in pinned Nixpkgs. Any CI module/load failure is a blocker, not accepted QML geometry evidence.
- M30g's compositor clipping and M30h's pointer tests are separately proven with synthetic Qt; this M30i test does not connect their temporary compositor hooks to the selected real Company layer.
- Do not enable `vectorBridgeOptIn`, invent the user's locally built Qt/MapLibre ELF digest, change client-data storage, or trigger an online map.
- No operator desktop deployment, PR merge, or final acceptance.

## CHECKLIST

- [x] Read Syntagma AGENTS and relevant authority / Hospital protocols
- [x] Recheck active SwayPX PR #92 and both source heads
- [x] Inspect real CompanyNetworkW, CompanyCoveragePane, viewport ownership policy, optional bridge
- [x] Add exact pinned Company Quickshell CI copy with read-only Canvas geometry observability
- [x] Add all three Company chassis states and close revocation assertions
- [x] Add actual screenshot and default-off actor/bridge refusal checks
- [ ] Exact-head successful real Quickshell + headless Wayland CI
- [ ] Use host-validated real geometry to drive compositor crop and native input
- [ ] Launch actual MapLibre 3.0 with compatible dependency patch in isolated CI
- [ ] Native selected Company layer + authentic M28v2 fd receipt and controlled actor authority
- [ ] Independent review, merge decisions and user desktop acceptance

## NEXT

Fix any genuine Quickshell/Nix/QML dependency or startup failures without bypassing menu, actor, or viewport gates. Once real Company UI geometry is reproducible in CI, make Stage Director consume those native-layer-relative rectangles under an explicitly admitted test actor; later replace the synthetic Qt map client with actual MapLibre. Verify source runtime boundaries first; do not describe an unauthorized standalone overlay as production embedding.

## DECISIONS

The operator approved the prototype-first route and GitHub development. This PR stays stacked on M30h and unmerged. Company remains a separate patient and owner: only a source-pinned, read-only CI checkout is used, with temporary disposable instrumentation, not a Company repository commit or live install.
