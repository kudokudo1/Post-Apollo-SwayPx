# Stage Director SD1 // Real Chrome sibling-XDG quarantine in isolated CI

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // Fail closed on second real Chrome native XDG top-level scene
BASE // `master` `5fe21666028cc3d8486bc18512984cd20e24b628`
WORKER // External Hospital-compatible ChatGPT Doctor, no live attachment
STATE // Source candidate; GitHub CI and operator final acceptance pending

## IMPLEMENTATION

PR #61 independently proved a genuine Chrome-generated second native XDG top-level escaped the original single-view SceneFX mask: 3460 new bright outside-mask sample points from 0, plus a moved/reflowed original XDG window. It is a verified **negative** containment finding, not success.

This narrow opt-in CI-only patch adds two safeguards for **the exact second Chrome native view** whose Wayland peer, Linux process generation and final cgroup component match the disposable Chrome systemd service:

1. At the **earliest workspace insertion** in `view_map()`, the exact native cgroup-matched sibling is directly placed in the floating workspace instead of first being inserted into the tiled tree. Sway's normal tiled Chrome actor is never rearranged just to make room for a second window. The ordinary production branch is untouched.
2. The CI map scene probe disables the **entire sibling container scene tree**, including content and decorations. The existing after-arrange CI hook reapplies that disabled state; the existing view-unmap lifecycle revokes the held sibling pointer. A third same-service view is fail-closed on its scene, but full multi-window lifecycle coverage is still outstanding.

The whole-scene quarantine still applies at the native view's map observation rather than at initial Wayland surface construction. **Absence of a transient rendered frame before that hook remains unproven**; the current screenshot checker demonstrates only the captured steady-state frame.

In the earlier draft checks, Chrome hit `pthread_create: Resource temporarily unavailable (11)` with many cgroup processes under the original `TasksMax=128` cap; another run changed native rectangle before PHASE2 and failed closed as `CHROME_MASK_OUT_OF_BOUNDS`. The isolated **test service only** now uses a still-bounded `TasksMax=256`, and records actual kernel `pids.current`, `pids.max` and `pids.events` to distinguish real task pressure from a guess. The post-startup observer also requires 16 unchanged native-ID/PID/rectangle samples before sending masks or second-window requests. That is an additional safety assertion, not a workaround for mismatched geometry. A changed rectangle is captured in `sd-ci-chrome-geometry-changed-tree.json` and fails the run.

The test's fourth Chrome matrix lane switches from discovery to **strict fail-on-leak**. It retains the original first-mask screenshot, requires two distinct real Chrome native XDG IDs and an actual second Chrome executable request, asserts the quarantine log, and compares actual grim output. The new checker fails on *any* additional sampled bright pixels outside the viewport or any original-XDG geometry change. Teardown must still remove both XDG roots, cgroup processes and private profile.

There is no production compositor change: all new native scene behavior is built only with `-Dsd-ci-news-scene-probe=true`, protected by exact CI environment and cgroup verification. Production `can_spawn=false` remains unchanged. No Weather QML or workstation activity.

## HOW TO USE

Review the PR's `Stage Director real Chrome stationary XDG moving mask (CI)` / `chrome-second-toplevel` job. Inspect screenshot artifacts `sd-ci-chrome-second-toplevel.ppm/png` and Sway IPC trees, and demand `SD_CI_CHROME_SECOND_TOPLEVEL_CONTAINMENT: result=PASS` in the job log. Production Chrome remains disabled.

## VERIFY

- [ ] Normal production SwayPX binary excludes `SD_CI_CHROME_SIBLING_QUARANTINED` and compiles
- [ ] Actual Chrome second native XDG IDs and process/cgroup proof
- [ ] CI-only whole-sibling scene disabled before screenshot; post-layout reapply observed
- [ ] Original main Chrome XDG geometry remains unchanged after the floating/layout transaction
- [ ] Post-sibling screenshot outside-mask bright delta = 0 and no stale pixels
- [ ] Unrelated app titlebar, real native XDG popup and Chrome kill/regression scenarios pass
- [ ] Exact cgroup/native/profile teardown and all workflows pass before merge

## WATCH OUT FOR

A green test covers **one additional Chrome-created XDG top-level in a disposable CI compositor**. Sibling windows remain independent native windows, not an authenticated inside-NEWS popup broker. Focus/seat input may still route to a hidden window, popup/grandchild scenes may have independent roots, and multiple sibling windows/mapping races require separate tests. This patch MUST NOT be taken as a production enablement or browser security boundary; it is a targeted visual containment experiment.

## CHECKLIST

- [x] Reconstruct #61 leakage, current master and Syntagma rules
- [x] Add CI-only exact-cgroup sibling prelayout floating and whole-scene quarantine
- [x] Revoke sibling scene pointer during unmap, reapply disable after scene arrangement
- [x] Upgrade actual Chrome second-XDG screenshot validator to strict zero-leak policy
- [x] Keep all prior CI Chrome/popup/cleanup/regression checks
- [ ] Current-head GitHub real pixel proof, CI full build and merge
- [ ] Verify separate keyboard/pointer/focus and multiple-sibling lifecycle
- [ ] Authenticate true Weather Station NEWS inner surface and popup geometry
- [ ] Operator final acceptance

## NEXT

If the strict screenshot gate fails, diagnose **why** (late scene enabling, workspace reflow, border/decoration, output reallocation), without weakening the real screenshot criterion. When green, repeat with 2+ extra XDG top-levels and actual browser child subsurfaces; separately verify focus/keyboard/pointer cannot target hidden siblings. Coordinate with Weather Station QML owner before production integration.

## DECISIONS

No local Fedora write, Quickshell restart, production browser spawn, QML ownership transfer or historic PR mass merge. External Hospital-compatible worker only.
