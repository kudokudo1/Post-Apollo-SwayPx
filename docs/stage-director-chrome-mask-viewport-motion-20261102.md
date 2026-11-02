# Stage Director SD1 // Stationary Chrome / movable viewport mask

STATUS // Draft, GitHub CI-only. No production launch, local Fedora modification, Weather QML edit, merge, or operator final acceptance.

PATIENT // kudokudo1/Post-Apollo-SwayPx

ROOM // Stage Director NEWS protected browser viewport

ASSIGNMENT // Prove real Chrome's unchanged native XDG window can remain mapped while a fixed SceneFX mask moves and expands, with two actual screenshots and old-viewport cleanup.

WORKER // External Hospital-compatible ChatGPT Doctor, not verified as registered with live Hospital.

BASE // Verified PR #50 `fd1f35535aac5699e4e2633ff30ede80aa6a25c1`. PR #51 remains a separate draft **blocked** after CI showed moving a tiled Chrome into floating mode unmapped the XDG view, and initial floating mapping had no stable XDG view. Do not merge PR #51 as a successful dynamic geometry fix.

## IMPLEMENTATION

The opt-in `-Dsd-ci-news-scene-probe=true` SwayPX source already verifies an actual Chrome XDG `wl_client` and Linux UID/PID/starttime against an exact transient systemd test cgroup, applies a 320×240 SceneFX content crop and normalizes source/destination buffer sizes. The CI browser remains tiled, unmoved and full output size (1920×1080), using fixed `about:blank`, private network/cgroup/profile, and no explicit sandbox bypass. Another pinned Foot native control verifies normal titlebars persist on unrelated windows.

This experiment adds a **CI-only host mask request** after the first actual screenshot. The runner writes one exact `PHASE2` request with mode 0600 in its own `RUNNER_TEMP` using `O_CREAT|O_EXCL`; the ordinary compositor has no such request handler. A test-only timer in Sway's Wayland event loop polls only the exact run-scoped request name, opens with `O_NOFOLLOW`, validates ownership, regular file, link count, permissions and exact contents, then **revalidates the same mapped Chrome wl_client and exact Linux cgroup** before changing a scene mask. The one-shot request moves the mask from local (96,96) 320×240 to (520,240) 400×300. Crucially, it does **not** move/resize the Chrome container, create a browser, change server borders, or grant pointer/keyboard/focus/popups. It removes the timer on unmap or after one accepted update and restores ordinary clipping if actor custody is lost.

The independent screenshot checker compares the black 1920×1080 baseline, actual initial Chrome 320×240 crop, and actual second compositor screenshot. It asserts same native container ID, PID, border and original rectangle, old viewport pixels cleared, new inside samples white including expanded width and height, four margins and desktop black, and no renderer leaks. Browser cgroup/process identity/window/profile and temporary /dev/udmabuf ACL teardown remain mandatory.

## HOW TO USE

Review the draft PR and `Stage Director real Chrome stationary XDG moving mask (CI)` workflow. GitHub-hosted Ubuntu runner only; no workstation steps or runtime activation.

## VERIFY

CI source gate requires the production compositor build exclude the CI selector, request only under exact private `/home/runner/work/_temp`, no-follow file opening, independent native/cgroup verification, and before/after screenshot analyzer. The separate opt-in compositor launches real Chrome and unrelated Foot, verifies native XDG process, original screenshot, new mask and relocated pixels with unchanged XDG window, and complete teardown. **Pending actual GitHub Actions result.**

## WATCH OUT FOR

- A SceneFX subsurface source clip and its destination buffer size may not reposition pixels together. This is why both actual screenshots, not hit tests or logs, are mandatory. If they fail, record the blocker.
- Fixed test-only coordinates are **not** actual Weather host-derived geometry. A real authenticated host/session contract and independent popup, focus, input and dynamically changing content boundaries remain.
- This is not equivalent to changing the Chrome outer window. The failed tiled-to-floating transitions remain an unresolved SwayPX/Chrome problem, isolated in blocked PR #51.
- Chrome's internal sandbox has not been independently attested; no production browser broker is implemented. No untrusted URL, app_id or title is used as ownership.
- PR #43 private Wayland-global discovery fix is required before any ancestor WATCH protocol activation. Do not deploy #42 alone.
- If the timer cannot safely verify request/actor, it fails closed. A canceled/unmapped browser must never be retained by a pending test timer.

## CHECKLIST

- [x] Verify Syntagma, PR #50 and blocked PR #51 current heads and ownership
- [x] Derive mask update from test-owned fixed runner request without changing XDG geometry
- [x] Revalidate exact native Chrome and cgroup; apply bounded second crop and destination normalization
- [x] Require unchanged native actor/window and two actual screenshot pixel gates
- [x] Preserve unrelated window border, full process/window/profile teardown and normal compositor isolation
- [ ] All CI jobs green, actual old/new pixel values verified
- [ ] Authenticate real Weather-host viewport coordinates and leases
- [ ] Popups, focus/keyboard/pointer/multi-surface confinement
- [ ] Production broker and operator live acceptance

## NEXT

If actual pixels move, test multiple successive host viewport updates and abnormal browser exits, then popup/input/focus confinement. If white content fails to move or old pixels persist, inspect the CI screenshot artifacts and SceneFX source/destination coordinates, and keep the PR draft and unmerged.

## DECISIONS

Operator gave permission to merge when necessary. No merge is necessary for this isolated CI experiment. No local system, Weather QML, production compositor or Hospital registration changes.
