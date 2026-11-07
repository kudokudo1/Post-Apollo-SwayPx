# 🏥💜 Stage Director // M30f put-the-window-there-first prototype

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company native MapLibre integration
STAFF ROLE // Nurse 2
DOCTOR // External browser ChatGPT worker
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE (not registered in live Hospital)
ASSIGNMENT // Prototype visible real Qt window placement before production embedding
PHASE // Disposable CI overlay viewport experiment
BASE // M30e PR #89 (14c19119aec239259c161efea798e9b54b097325)
SCOPE // Company-owned Qt CI harness, disposable simulated frame, screenshot verification, no production compositor or live desktop writes

## IMPLEMENTATION

Operator explicitly chose a **prototype-first integration path** to get actual visual feedback before finishing every native actor admission step. M30f tests the fastest separable hypothesis: can a real Qt Wayland window be placed and resized over a Company-shaped frame without corrupting it?

- Extend the existing, REAL Qt/QPA client from M30d with two test-only flavors: `frame` (dark purple 1100x740 QWidget) and `map` (cyan QWidget). They have distinct native Wayland XDG roots and the same spoofable Company app-ID; they are NOT the actual MapLibre ELF or Quickshell.
- Boot the existing unprivileged disposable headless SwayPX under a private `RUNNER_TEMP` Wayland socket.
- An exclusively CI-guarded script starts the frame, obtains its exact Sway XDG container and real Linux pidfd, floats and moves it to `(360,180)`, removes its own test decoration and measures actual compositor pixels.
- Start the separate real Qt map-shaped witness, verify its exact PID and XDG root, float it inside the simulated frame at `(520,300)`, resize it to 600x380, use Sway IPC to select focus, and capture real compositor pixels.
- Grow the same native XDG map window to 780x460; verify the new part fills with its real cyan Qt buffer rather than retained purple frame pixels, with no cyan elsewhere on the output.
- Terminate the map, require its XDG root disappears, screenshot the unchanged host frame, confirm no stale cyan pixels, then terminate and reap all disposable windows and the compositor.
- `ci_company_overlay_prototype.py` verifies four actual headless `grim -t ppm` screenshots. It decodes RGB bytes independently using stdlib, tests inside pixels, outside-black, frame-purple, newly expanded map region, global cyan leakage (outside expected window rectangle) and post-close restoration.
- The CI job uploads its PPM screenshots for independent examination and invokes the existing reversible temporary `/dev/udmabuf` ACL helper. No Qt/MapLibre or Sway code is installed on the operator workstation.

This is an **overlay**, NOT real compositor-side cropping or embedded QML. The first screenshot proves only that a native Qt XDG window can be moved and drawn inside another window's rectangle. It does NOT prove map pixels clipped when the native buffer is bigger than the viewport. That is the next task, based on the evidence.

## HOW TO USE

No desktop action. Review the M30f GitHub draft, its `real-qt-overlay-screenshots` job, and the four attached disposable pixel artifacts. The normal Company `COVERAGE -> VECTOR VIEW` action still opens the original standalone map; the optional Company QPA bridge remains default-OFF.

## VERIFY

- CI-only launch gate: requires `GITHUB_ACTIONS=true`, `CI=true`, a dedicated private Wayland runtime under `RUNNER_TEMP`, and exact Nix store Qt/Sway/grim binaries.
- Native process: QWidget really uses Qt6 Wayland QPA, actual Sway XDG-root PID and Linux kernel pidfd; app ID and window title never grant the Company production broker any scene permission.
- Screenshot phases must all independently pass; IPC geometry and scene hit tests without real pixels do not satisfy acceptance.
- First and expanded rectangle require distinct native Qt process, exact same XDG container ID on resize, correct Sway focus *selection* (not actual pointer injection), and no cyan outside target rectangle.
- Map-close screenshot must show the original purple frame and no stale cyan leakage.
- **Not yet verified:** actual MapLibre Native Qt 3.0 6.11.2 binary, Quickshell frame, compositor-owned clipping when surfaces overflow, direct mouse dragging/scrolling/keyboard, popup containment, reproducible Qt plugin bundle, actual M24-selected host and M28 v2 fd transfer.

## WATCH OUT FOR

- The browser Worker cannot inspect or install into the operator's Fedora Silverblue. This test runs ONLY in the temporary GitHub runner.
- M30f uses existing Sway IPC commands to position a Qt top-level; this is sufficient for a fast visual placement prototype but it is not a production window broker or authenticated native scene lease.
- Sway can refuse to move or float a native window, or a Qt decoration can change measured geometry. The test must remain red on those failures, not mask them with relaxed assertions.
- Passing the global cyan-no-leak test only proves the window **fits the frame**. The test window itself is sized to the viewport. True clipped overflow requires a larger source surface and compositor-side crop.
- The rendering output uses two fixed fake widgets and exact synthetic coordinates. It is not an actual Company QML layer or evidence for a real interactive geographic map. Do not call it a usable embedded MapLibre prototype.
- No production M30a digest pin, no scene/focus/input authority and no installation or merges.

## CHECKLIST

- [x] Confirm M30e head, current SwayPX master, live Company M28c launch/bridge QML and other Weather owner branches
- [x] Re-read Syntagma AGENTS and Hospital/authority/report protocols
- [x] Build distinct Qt frame and map-shaped witness modes in existing Company-owned test client
- [x] Write guarded overlay/resize/focus-selection/close experiment with real compositor screenshots
- [x] Add standalone M30f GitHub workflow and artifact retention
- [ ] Pass M30f exact-head CI with no pixel leaks and clean teardown
- [ ] Extend to original MapLibre native executable, not generic Qt QWidget
- [ ] True render-surface crop and input using owned Company viewport geometry, not fixed screenshot positions
- [ ] Operator-authorized integration in live Quickshell Company and runtime acceptance
- [ ] Operator acceptance, merge order and production activation decisions

## NEXT

Inspect exact-head screenshot CI; correct real native geometry/render issues without weakening screenshot evidence. Once the first visible step passes, advance to **actual source crop and pointer delivery with native Qt QPA**, then replace the synthetic map widget with actual locally built MapLibre Qt after obtaining compatible dependencies. Keep source-level trust and production actor/input authority unchanged until those checks pass.

## DECISIONS

The operator approved testing the visible minimal prototype before optional infrastructure polish. This authorizes bounded GitHub changes in the existing Stage Director Company integration room; it does **not** authorize local Fedora writes, other-room Weather source changes, bypassing Company physical host selection, or automatic production enablement. The earlier M30a–M30e draft PRs remain dependencies and unmerged. This checkpoint is not final DONE.
