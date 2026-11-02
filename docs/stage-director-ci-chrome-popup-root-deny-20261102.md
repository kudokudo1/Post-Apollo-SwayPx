# Stage Director SD1 // CI-only Chrome XDG popup root-scene denial

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
ASSIGNMENT // Fail-closed native XDG popup creation while CI Chrome is selected
BASE // `master` `6211bbb80804f45869ded72451091608fb51206c`
STATE // CI-only source implementation, GitHub verification pending
CONNECTION // External Hospital-compatible Doctor, no live registration verified

## IMPLEMENTATION

Source inspection identified a concrete compositor layering problem. Sway's `handle_new_popup()` attaches a real XDG popup to `root->layers.popup`, while nested `popup_handle_new_popup()` creates further scene descendants of that independent popup tree. The existing fixed browser SceneFX clip only operates on `view->content_tree`. Consequently neither strict popup *coordinates* nor an otherwise correct browser clip proves pixel containment.

This narrow experiment adds a CI-only refusal hook to **both native XDG popup event handlers**, before any `popup_create` call. The hook blocks popup creation for the currently selected Chrome actor in the isolated renderer test. It uses the native mapped view identity already bound to the exact browser Wayland client and service cgroup. If the actor was selected but its process credentials have gone stale, the guard **still denies** rather than treating identity loss as permission. Native `wlr_xdg_popup_destroy` is used on the untrusted popup; ordinary applications continue their stock popup logic.

The hook is compiled only in the opt-in `SD_STAGE_DIRECTOR_CI_SCENE_PROBE` variant. The normal production SwayPX binary does not contain the guard. This is a **deny-all policy** for the isolated browser; not an interior popup renderer or an approval to place popup surfaces elsewhere.

## HOW TO USE

Review the pull request and the `Stage Director real Chrome stationary XDG moving mask (CI)` workflow. A source-boundary check confirms both popup event hooks are guarded. The normal full production binary is checked for absence of the CI-only marker; the test binary is checked for its presence.

There is no local Fedora configuration or Weather Station NEWS control to enable.

## VERIFY

- [ ] Source gate verifies both **root** and **nested** popup paths invoke the deny hook before `popup_create`.
- [ ] Source checks verify selected Chrome identity is tied to exact native process/cgroup and that identity loss cannot open the popup path.
- [ ] Normal production SwayPX build excludes all CI popup-deny symbols; opt-in test compositor links them.
- [ ] Previous real Chrome pixel/mask/abrupt SIGKILL tests remain green.
- [ ] Future real XDG popup *event* test must generate a mapped popup and observe the event denial; this PR **does not claim** that evidence.

## WATCH OUT FOR

This intentionally **does not demonstrate a real browser-generated XDG popup** or its post-denial pixels. The existing about:blank test does not cause a popup. A separate native xdg_popup witness (and an actual Chrome popup generation path) is still needed for end-to-end rendered-pixel proof. Chromium can also create separate XDG toplevel windows or other surfaces; those are not the same as XDG popups and remain untested. Focus, pointer, keyboard, popup broker, production browser custody and authenticated inner NEWS viewport remain disabled or open.

Do not treat a browser title, URL, client assertion or geometry alone as authority. Sway's global popup tree cannot be safely confined by only cropping the browser content tree.

## CHECKLIST

- [x] Recover PR #58 and exact master; re-read current Syntagma protocols
- [x] Inspect stock Sway top-level/nested native XDG popup creation and global root attachment
- [x] Add guarded CI-only deny hook on both creation handlers
- [x] Fail closed for selected actor even if native credentials disappear
- [x] Add source and binary build exclusion checks
- [ ] GitHub compile/test CI complete
- [ ] Real native xdg_popup event and post-blocking screenshot proof
- [ ] Separate toplevel/subsurface, popup/input/focus/seat containment
- [ ] Operator live integration and acceptance

## NEXT

After CI, create a bounded **real Wayland XDG popup emitter** against the isolated SwayPX compositor, observe actual compositor event and rendered pixels, and demonstrate the denial gate rather than relying on source assertions. Coordinate the real Chrome popup trigger and actual Weather Station inner-NEWS geometry with their respective owners. A guarded rendering broker should only be designed after native popup origin/scene ancestry is authenticated.

## DECISIONS

Preserve source-only master-based integration, keep old stacked PR #23–52 unmerged (especially blocked #51), and do not activate browser, keyboard/focus, popup or local workstation behavior. No live Hospital registration or operator final DONE claimed.
