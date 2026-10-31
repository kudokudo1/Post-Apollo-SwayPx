# Stage Director // Native Weather host session, revocation and pairing boundary

STATE // Bounded production-built infrastructure. No trusted operator pairing, browser, scene, input or live desktop activation.
PATIENT // kudokudo1/Post-Apollo-SwayPx.
ROOM // Stage Director / Weather Station integration (external Hospital-compatible, not live registered).
ASSIGNMENT // Operator-approved Option A, compositor-controlled pairing path, prioritized to first usable NEWS viewer.
BASE // PR #39 at a831cb669bda0a34e49d3c797d17fac51f021a0d.
NEIGHBOR // kudokudo1/The-Post-Apollo-Project / Weather Station; separate worker owns QML.
PROVIDER // OPEN AI // CHATGPT.

## IMPLEMENTATION

Add a single native host-session candidate registry and its peer-matching code to the normal SwayPX Meson source set. The existing Stage Director layer lifecycle invalidates a session when the exact host unmaps or destroys and rechecks live native host/output/client identity on commits. The registry independently listens for destruction of its actual client, layer-role resource and underlying wl_surface resource, and refuses stale generation reuse. Request probes compare against exactly the registered Wayland connection and current host/output/epoch, but return CANDIDATE_ONLY with all authority false.

The selector `stage_director_news_host_session_record_native_candidate` is **deliberately uncalled by all production sources**. It does not accept an arbitrary namespace as proof, and this PR implements no public IPC command, compositor keybinding, custom global, browser process, rendering mutation or input injection. It is intentionally a production-linked state component awaiting the separately reviewed actual physical-input pairing issuer. This is a concrete step beyond CI-only PRs #38 and #39, but it is not yet a user-facing viewer.

## HOW TO USE

Review this draft, check the production SwayPX build and native Wayland lifetime CI. There is no user command or Weather Station UI change. Existing safe NEWS viewer behavior remains.

## VERIFY

GitHub Actions compiles and runs a native test with two actual Wayland client connections; verifies that same-process foreign WATCH resources are refused; tests mismatched output, host and generation, wrong-host invalidation, one-at-a-time session reservation, role resource destroy, wl_surface destroy, whole client disconnect and monotonic generation replay refusal. CI also ensures there is no production candidate issuer or scene/seat actuation, and builds the actual ordinary SwayPX compositor through locked Nix dependencies. GitHub CI is pending until this draft is opened and checked.

## WATCH OUT FOR

**Authorization is not granted by building this state module.** No API accepts a normal keyboard shortcut, Sway IPC message, title, namespace or self-declared trusted bit as a selection ceremony. Sway binding execution can be invoked through public IPC, so pairing must be issued from a compositor-native input event with explicit virtual-input exclusion; any pointer-based host selection must prove exact scene/layer identity and reject Weather alpha-hole ambiguity. Under unrestricted same-UID uinput or ptrace compromise, physical-input provenance is not guaranteed; an explicit stronger OS trust boundary is required if that attacker is in scope.

**Runtime semantics:** output disable/disconnect and layer recreation must revoke before any active WATCH admission. Current commit-time revalidation protects native candidates, while no candidate issuer is present. The actual native Qt/Quickshell bridge may open a separate wl_client, in which case Option A must be reassessed rather than forging identity. Renderer/seat/popup and broker remain independently denied. No local desktop verification.

## CHECKLIST

- [x] Operator approved Option A and first usable NEWS viewer before Kitty extension
- [x] Recheck latest stacked SwayPX PRs and Syntagma/Hospital rules
- [x] Add production-linked native host-session candidate with exact client/role/surface/output/generation
- [x] Wire existing layer callbacks to automatic host-loss invalidation
- [x] Add native Wayland resource/client destruction listener revocation and replay refusal
- [x] Add standalone real-client CI and full ordinary SwayPX build check
- [ ] Verify both CI jobs and grammar on draft
- [ ] Input/security review: devise operator-controlled, non-IPC trusted selection with virtual input exclusion
- [ ] Add disabled-by-default private Wayland WATCH protocol global with paired-client disclosure/bind/request checks
- [ ] Weather owner: native Qt/Quickshell same-wayland-connection request bridge
- [ ] End-to-end broker custody, clip/destination sizing, popup/focus/input/teardown acceptance
- [ ] Operator-controlled desktop installation, rollback and final acceptance

## NEXT

Once actual compositor compilation passes, implement the physical-input selection ceremony and real mapped host hit-test in a separate draft. Do not connect production WATCH protocol until the selected client can be trusted despite multiple identical Weather namespaces. Coordinate that native protocol shape with Weather owner before cross-repository edits.

## DECISIONS

Compositor-owned pairing is operator-approved as the target architecture. The specific physical-input gesture and cross-owner bridge remain to be designed and verified, rather than silently guessed. No repository merge, live local writes, production activation, Weather QML edits, or Hospital registration performed.
