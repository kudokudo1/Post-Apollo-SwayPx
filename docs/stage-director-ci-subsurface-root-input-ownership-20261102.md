# Stage Director SD1 // Real Wayland subsurface root input ownership (CI)

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
WORKER // External Hospital-compatible ChatGPT Doctor (no verified live registration)
BASE // `master` `a5670505bce883c25663d52994d883732c549c6d` (PR #66)
STATE // Draft CI candidate, not yet verified, not production
ASSIGNMENT // Resolve native wl_subsurface parentage for input and focus

## IMPLEMENTATION

PR #66 tested actual Chrome BTN_LEFT and implicit pointer grab against the native **root** Chrome windows. Its observer classified every unfamiliar wl_surface as `OTHER`. This creates a specific blind spot: a `wl_subsurface` has its own surface pointer, but inherits a root in the same **native wl_subsurface tree**. Comparing only direct pointer equality cannot attribute input to the root's owner.

This CI-only change calls the real wlroots `wlr_surface_get_root_surface(surface)` before classifying keyboard, pointer-motion and pointer-button delivery. A `wl_subsurface` under an attested MAIN or HIDDEN_SIBLING root is classified with that root, accompanied by `descendant=1`; unknown roots remain `OTHER`, not safe. A destroyed/unknown subsurface tree remains `UNKNOWN_DESTROYED_TREE`. The preexisting direct surface-focus guard likewise resolves the native subsurface root before checking its exact hidden Chrome registry, so a child cannot bypass a same-root native focus refusal merely because its pointer differs. All code is compiled **only** under `SD_STAGE_DIRECTOR_CI_SCENE_PROBE`; production behavior is unchanged.

A real disposable Wayland client `ci_subsurface_input_witness.c` creates a genuine XDG toplevel and a green SHM-buffered `wl_subsurface` child on the same Wayland connection, not a Chrome-generated child. It receives actual `wl_pointer.enter` and button presses/releases and logs them. The separate `chrome-subsurface-owner` job:
1. Keeps real Chrome MAIN and hidden sibling native identities, cgroup, profile and earlier focus/pixel safety checks.
2. Starts the unrelated native parent/child witness with an exact CI-only app ID and floating placement. It sends a real Wayland virtual pointer and BTN_LEFT to the child coordinates.
3. Requires `SD_CI_NATIVE_INPUT_DELIVERED kind=POINTER target=OTHER root_only=0 descendant=1` and `SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS/RELEASE target=OTHER root_only=0 descendant=1`, proving the compositor traversed a real child surface to an unrelated root.
4. Requires client-side `SD_CI_SUBSURFACE_RECEIPT kind=ENTER owner=CHILD` plus both button events, so compositor-side logs alone cannot pass.
5. Fails if any tested event targets HIDDEN_SIBLING, disposes of the unrelated witness and checks its XDG unmap, restores exact Chrome native focus, rechecks real output screenshot and Chrome cgroup/profile teardown.

## HOW TO USE

Review PR checks > `Stage Director real Chrome stationary XDG moving mask (CI)` > `chrome-subsurface-owner`. Inspect `sd-ci-native-subsurface-witness.log`, `sd-ci-subsurface-positive.png`, the separate actual Chrome pixel screenshot and `SD_CI_NATIVE_SUBSURFACE_INPUT_OWNER: result=PASS`.

## VERIFY

- [ ] CI native Wayland parent/child XDG client compiles with pinned Wayland scanner.
- [ ] Client confirms actual child input receipt; compositor confirms descendant=1 and derived unrelated root.
- [ ] Same native hidden-Chrome root is not an input recipient and original focus/geometry remain correct.
- [ ] Production binary excludes every test input observer, old popup/mask/multiwindow/button/headless regression green.
- [ ] Merge only after all current-head GitHub checks pass.

## WATCH OUT FOR

**The input-positive child belongs to an unrelated disposable Wayland client, not the Chrome executable.** This test proves the native root-resolution mechanism and a real positive child receipt, plus the previous hidden Chrome root negative case. It does not prove Chrome emitted a `wl_subsurface` or that hidden Chrome descendants were actively targeted and blocked. The Wayland `xdg_popup` relationship is **not** a `wl_subsurface` tree; popup input, global root scene and grabs still require a separate test. This remains sampled, steady-state visual evidence, not continuous first-paint/seat authority. The client is CI-only and never starts on operator Fedora.

## CHECKLIST

- [x] Read Syntagma, current PR #66 master and real seat/subsurface source.
- [x] Extend CI-only classifier and direct focus guard to resolve wl_subsurface root.
- [x] Add actual native parent/child witness with client-side input receipts.
- [x] Add independent matrix lane and audit production/test source seams.
- [ ] Green CI and exact merge readback.
- [ ] Real Chrome-generated child fixture and native popup input/grab boundary.
- [ ] Early-frame paint and authenticated actual Weather NEWS host/production authority.

## NEXT

On green, investigate real XDG popup (separate root) and Chrome-generated descendants with direct native client events; do not conflate a root-descendant witness with browser-created subsurfaces. Continue transient early-paint and inner Weather NEWS geometry checks as independent work. Recheck ownership and GitHub head before edits.

## DECISIONS

No local Fedora/Quickshell changes, no Weather Station QML edits, no live Hospital registration, no historical PR bulk merge, no production browser/input activation; operator final acceptance reserved.
