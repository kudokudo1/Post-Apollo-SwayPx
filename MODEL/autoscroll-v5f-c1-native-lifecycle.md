# V5F-C1 // Native XDG Target + Output Lifetime Receipts, CI-Only

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Mouse & Keyboard · Universal Autoscroll  
STAFF ROLE // External Hospital-compatible DOC, not verified live-registered  
ASSIGNMENT // Evidence for real mapped window and output-owner lifetime invalidation  
BASE // V5F-B PR #159 · exact `f7ac7f548c3639d7a733016665b48c403226ed2f`  
OWNER // Company Map PR #126 remains paused, separate and unmodified.

## IMPLEMENTATION

**No production C, headers, Meson, input handlers or Company Map files are changed.** This branch adds only a disposable CI source-overlay script, headless runtime harness, guarded temporary runner permission wrapper, source-safety test, workflow and this document.

On a disposable GitHub Actions runner, the overlay copies the exact checkout into `$RUNNER_TEMP/pa-fc1-native-src`, refuses non-CI execution, and modifies **only** the copied `sway/tree/container.c` and `sway/tree/output.c`. These two files are real SwayPX callback owners and are not edited in the checked-out PR. The copied binary logs object-lifetime receipt values:
- **TARGET_BIND** from a real mapped XDG view's newly created container after owner epoch bind but before node publication.
- **TARGET_REVOKE** from the real `container_begin_destroy()` callback, after invalidation and before close/destroy signals, recording that the accessor becomes zero.
- **OUTPUT_ENABLE** after actual output enable and token mint.
- **OUTPUT_CONFIG** after a successful compositor-owner reconfiguration revokes and reissues the output epoch.
- **OUTPUT_DISABLE** after actual output disable revokes and before workspace evacuation; the live epoch must be zero.

The Python harness boots the Nix-built, CI-instrumented compositor on a private Wayland and IPC socket with headless software rendering, no libinput physical devices, no operator configuration and no external credentials. It opens and destroys two genuine independent Foot XDG windows, verifies the Sway IPC tree identifies their mapped `xdg_shell` objects, reapplies the same output scale (no live monitor resize), then disables the headless output. It parses compositor-origin logs and requires actual nonzero/changed owner tokens and zero-after-revoke. All test resources are deleted, and a temporary runner-only `/dev/udmabuf` ACL, if necessary, is restored by a trap.

**The production guard is left in hard-deny mode.** No middle-click is consumed, no Wayland scroll axes are emitted, no positive physical witness fabricated, and no production app or local desktop modified.

## VERIFY

The dedicated workflow checks a strict six-file addition diff against F-B, confirms the real checkout contains no `PA_FC1_NATIVE` instrumentation, runs source-order/scope tests, builds and tests the isolated compositor, checks each real XDG target's bind-to-revoke identity, requires fresh tokens across two windows, and verifies output enable/config rotation/disable. This goes beyond portable C mocks: the callbacks are actually invoked by a real headless compositor.

**Known limits:** this package cannot verify a real physical input device because the headless backend sets `WLR_LIBINPUT_NO_DEVICES=1`. It also does not create/destroy transient `wlr_seat` objects or activate the V5C guard. These are explicit V5F-C2 follow-ups rather than fabricated receipts. A successful target/output lifecycle trace is evidence of two owner domains only, not full four-domain session authentication. Real live hardware, XDG focus/scene permission, native Scroll Lock and trusted layout generation remain separate requirements.

## HOW TO USE

Review CI run and exact-head GitHub diff only. The overlay script refuses non-GitHub runner environments. Never run the temporary ACL or compositor harness on the operator's machine.

## WATCH OUT FOR

A headless output reconfiguration must not resize a running virtual output: the prior Stage Director test identified Mesa llvmpipe crashes under mode changes. The harness reapplies `scale 1` at the same output mode to trigger a real successful configuration commit without changing resolution.

A passing Nix native build alone is not a dynamic token witness. The workflow must pass the Python runtime assertions over the real compositor's `PA_FC1_NATIVE` logs. If the runner's Wayland IPC hangs, report the exact failing step, not a fabricated test pass.

Numerical lifetime epochs are not credentials, and the CI receipts are not proof of a real physical device. Company Map's cursor, seat, Scene and Meson ownership is preserved without new file overlap.

## CHECKLIST

- [x] Latest canonical protocols, GitHub branch and Company owner inspected
- [x] Disposable CI-only exact-source overlay for real window/output lifecycle
- [x] Native headless real XDG map/unmap and output config/disable harness
- [x] No production C/Meson change and no Company #126 overlap
- [x] CI-only permission restore, static source guards and workflow
- [ ] Exact-head dedicated workflow PASS
- [ ] Independent review and durable Company owner discoverability
- [ ] F5C2 dynamic device and transient-seat evidence
- [ ] Positive physical provenance, full scene/layout witness and operator authorization

## NEXT

Verify CI. If the native XDG and output receipts pass, report C1 as **two-of-four native lifecycle domains** and post a zero-overlap notification to Company PR #126. Work on C2 separately, with its own approved shared-seat/device ownership review. Keep production PA_AUTOSCROLL_LIVE_ACTIVATION 0 throughout.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION OFF · DISPOSABLE CI ONLY · NO OPERATOR HOST CHANGES · COMPANY MAP OWNER PRESERVED.
