# Stage Director SD1 // Native pointer button and implicit grab custody (CI)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
WORKER // External Hospital-compatible ChatGPT Doctor, no verified live registration
BASE // `master` `56d0f8ed57200c3de4cd0e949d1bc58a7ca120f5`
STATE // Candidate, automated verification and merge pending

## IMPLEMENTATION

PR #65 proved real Wayland virtual keyboard/pointer-motion delivery targeting the visible Chrome XDG root rather than a visually hidden Chrome sibling. It did not press a pointer button or exercise implicit grabs. This bounded CI-only increment adds a **BTN_LEFT press/release** and same-native-pointer **press → move across hidden sibling → release** protocol sequence using the existing pinned Wayland virtual-pointer C client. No local Fedora input devices or production compositor controls are created.

The opt-in `SD_STAGE_DIRECTOR_CI_SCENE_PROBE` compositor variant observes the actual wlroots button notification in `seat_pointer_notify_button()`: inspect the chosen `pointer_state.focused_surface` **before** `wlr_seat_pointer_notify_button` and classify its exact native root identity as `MAIN`, `HIDDEN_SIBLING`, `OTHER` or `NONE`. The observer is logging only; it does not grant native input authority or modify stock production button routing.

An independent GitHub matrix lane `chrome-button-grab` retains the existing real-Chrome native XDG/cgroup/private-profile identity, focus denial and rendering checks. With explicit runner-owned native input it must prove:
1. The clipped visible Chrome window accepts both native BTN_LEFT PRESS and RELEASE, with the compositor observing `target=MAIN` twice.
2. A BTN_LEFT click at the hidden sibling's mapped-but-quarantined rectangle generates real PRESS and RELEASE, neither targeted to `HIDDEN_SIBLING`.
3. After explicitly restoring visible focus, the **same virtual pointer** sends PRESS at MAIN, MOTION at hidden coordinates while held, RELEASE there, and another MOTION at hidden coordinates after release. Native button and pointer delivery logs must never show `HIDDEN_SIBLING`; native PRESS must target MAIN.
4. Hidden sibling must never become Sway's native focused window, original geometry must remain stable, the real screenshot must show zero additional sampled bright pixels outside the 320×240 mask, and exact Chrome service/native/profile teardown must pass.

Clicking an empty workspace can legitimately change focus to the workspace. The test observes that the hidden Chrome sibling did **not** gain focus before restoring original Chrome focus between independent phases, rather than treating normal background focus as a security failure.

## HOW TO USE

Review the PR checks under `Stage Director real Chrome stationary XDG moving mask (CI)`, job `chrome-button-grab`. Look for `SD_CI_CHROME_BUTTON_GRAB: result=PASS`, actual emitter `SD_CI_VIRTUAL_INPUT_SENT kind=BUTTON/GRAB` and compositor `SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS/RELEASE target=...` markers. The existing real PPM/PNG screenshot artifact and Sway native tree remain available. No local desktop commands.

## VERIFY

- [ ] Real Wayland virtual-pointer client compiles with BTN_LEFT event and native grab sequence
- [ ] Production SwayPX binary excludes button observer; opt-in CI compositor contains it
- [ ] Positive visible Chrome BTN_LEFT press/release recipient is MAIN
- [ ] Both hidden click and active pointer grab never target hidden native root
- [ ] Main XDG geometry, real screenshot, native focus and owned Chrome teardown all pass
- [ ] All older Chrome popup/multiwindow/focus/pointer-motion/headless/production regressions pass on same head
- [ ] Merge only after current-head CI green

## WATCH OUT FOR

The event observer classifies **root** wl_surfaces. A nested Chrome subsurface or popup can be classified `OTHER`, which is unverified, not safe. The native pointer grab is one fixed press/motion/release trace, not proof of arbitrary pointer trajectories, XDG drag-and-drop/data device permission, touch, keyboard grabs, or continuous first-frame containment. A green CI test does not authorize production NEWS browser/input access or establish actual Weather Station inner viewport provenance.

## CHECKLIST

- [x] Inspect Syntagma, exact prior master/PR #65 and Sway actual input routing
- [x] Reuse native pinned virtual-pointer emitter and CI-only compositor observer
- [x] Add button, hidden click and held-button crossing test phases
- [x] Keep existing source, native and visual regression gates
- [ ] CI full verification, report, merge
- [ ] Nested/popup input origin, pointer grabs outside chosen trace, transient first-frame study
- [ ] Operator final acceptance and production broker

## NEXT

If verified, inspect nested Chrome Wayland subsurface and real popup input routing with actual native client events, and separately test full-frame early-paint exposure. Coordinate with independent Weather Station QML owner before any authenticated browser integration.

## DECISIONS

No live Hospital registration, local Fedora/Quickshell mutation, production browser activation, QML scope change or historic draft PR bulk merge.
