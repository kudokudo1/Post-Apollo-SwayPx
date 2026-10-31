# SD1 // Native-input Weather host selection

STATE // Draft feature, disabled by default; **not** authenticated WATCH; no desktop activation.
PATIENT // kudokudo1/Post-Apollo-SwayPx.
ROOM // Stage Director / Weather integration, external Hospital-compatible only.
ASSIGNMENT // Operator-approved compositor-controlled pairing, NEWS viewer first.
BASE // PR #39 updated with operator-merged PR #40, commit 92512bcd6b72494cce646e5b6d441761121b97af.
NEIGHBOR // Weather Station worker owns The-Post-Apollo-Project/QML; untouched.
DOCTOR // OPEN AI // CHATGPT; no live Hospital session registration.

## IMPLEMENTATION

Add an opt-in compositor-native two-step selection ceremony using existing SwayPX keyboard and pointer event handlers, not command/IPC dispatch. Normal operation is unchanged unless SwayPX starts with `POST_APOLLO_STAGE_DIRECTOR_PAIRING=1`. When enabled, an ungrouped libinput keyboard's exact Ctrl+Super+Shift+F12 gesture arms a 3-second one-shot selection. A libinput pointer button press from the same seat must hit the real mapped root Weather layer surface; the compositor checks its exact role, overlay layer, namespace, output, actual Wayland client and stable native Linux process observation. A click through the QML transparent cutout sees another surface and is refused. The selection click is consumed along with its release. Absent a release, the swallowing state expires; a screen lock, keyboard-shortcut inhibitor, foreign seat, wrong button, stale timestamp or virtual input cannot produce a selection.

On arming, any previously selected native candidate is revoked. Successful selection records only a revocable native candidate in the real PR #40 session registry; its client/role/surface/output/epoch generation are tracked. Existing layer unmap/commit/destroy and resource disconnect code invalidates it. No WATCH global, lease reservation, browser, focus, clipping, popup or input-routing permissions are enabled by selection.

## HOW TO USE

Review draft and GitHub CI first. There is no live user action yet and this milestone must not be enabled on the operator desktop without separate explicit approval. The documented gesture is a proposed limited testing control, not a permanent UI binding decision.

## VERIFY

CI source boundary checks that the mechanism stays opt-in, is reached solely by native input event paths, requires ungrouped libinput keyboard and pointer, validates exact role via compositor scene hit, refuses lock/inhibitor/foreign-seat/expired input, and remains free of Sway IPC/Wayland control protocol and browser/scene/seat actuation. A separate CI job compiles the complete normal SwayPX with the new code. Physical desktop input, real Weather alpha-hole selection, uinput attack tests and same-client Qt bridge remain unverified until operator-approved isolated runtime acceptance.

## WATCH OUT FOR

Libinput provenance excludes ordinary Wayland virtual keyboard and virtual pointer, but may include Linux uinput-generated devices. A compromised same-UID process with uinput access or ptrace may synthesize a gesture, so this mechanism by itself does NOT provide strong cryptographic hardware consent. Additional OS device trust restrictions are required if such attacks are in scope. Do not label the resulting session as WATCH-authenticated. The operator's existing Sway keybind may conflict with the provisional F12 chord; no desktop settings have been changed.

The layer's namespace is a classification filter, not a credential. A spoof layer can still be selected if the user deliberately clicks that spoof, just as a fraudulent window can deceive a person. The exact scene-hit selection prevents automatic namespace-only admission, while the remaining UI authenticity and explicit user feedback belong to later review.

## CHECKLIST

- [x] Confirm operator merge was into PR #39's development branch, not master
- [x] Branch from exact updated PR #39 commit
- [x] Add opt-in native two-stage input selection into real keyboard and cursor paths
- [x] Bind exact pointer hit to native Weather role/output/client and session epoch
- [x] Keep all action and WATCH authority denied; preserve existing revocation
- [x] Add CI source boundary and full Nix compositor compilation
- [ ] Validate GitHub CI results; correct any failures
- [ ] Isolated native input/live layer map spoof and alpha-hole test
- [ ] Verify a trusted input provenance model against the configured uinput threat
- [ ] Private WATCH global + same-connection Weather bridge, separate review
- [ ] Browser supervisor, clip, input/popup/focus guard and operator acceptance

## NEXT

After CI, design and implement the private exact-paired-client Wayland WATCH protocol in a separate bounded PR, while the Weather worker independently verifies a Qt native bridge sharing the layer's Wayland client connection. Keep safe-mode NEWS until browser/scene/input/popup guards pass. No live SwayPX installation authorized by this draft.

## DECISIONS

Option A is approved. PR #40 merge is preserved, not reverted. This narrow implementation chooses only a provisional, disabled-by-default native input gesture; the final operator interaction and stronger uinput hardening remain open. No merge, local machine changes, Weather QML writes or live Hospital registration by this worker.
