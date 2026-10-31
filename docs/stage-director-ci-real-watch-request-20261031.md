# Stage Director SD1 // Real WATCH request provenance CI checkpoint

STATUS // Disposable CI-only proof, not operator authentication and not production runtime.
PATIENT // kudokudo1/Post-Apollo-SwayPx.
ASSIGNMENT // Continue Stage Director WATCH native request proof within SwayPX, without entering the Weather Station worker's scope.
BASE // PR #38 at 36edd99eba6f7f51973cc883c2ebb8df3ae35175.
PROVIDER // OPEN AI // CHATGPT (external Hospital-compatible; no live registration).

## IMPLEMENTATION

Introduce a **CI-only generated Wayland wire protocol** with WATCH(source_id) and CLOSE() requests. An isolated Wayland server owns two different real wl_client connections, both labeled with the exact same Weather namespace for the test fixture. Both clients are allowed to discover and bind the dummy global intentionally; the handler must enforce the exact selected wl_client again on EVERY WATCH/CLOSE request using the non-authorizing PR #38 peer probe.

The spoofed peer has real native resources with a matching self-owned host. That native match is still untrusted: comparing against the fixture-selected host instead rejects the spoofed request. A valid request from the selected connection produces only candidate evidence. Even the accepted candidate cannot reserve a lease, launch, render, clip, accept input or show popups. Test-only validation rejects a raw URL and a simulated host-unmap case.

The server and protocol remain under tests/stage_director only, not in SwayPX's normal Meson sources, server init, global filter, IPC, or keyboard/seat dispatch. This does not select the RFC #37 compositor-controlled pairing versus managed launcher architecture.

## HOW TO USE

Review the separate draft PR and its `Stage Director real WATCH wire request isolation (CI only)` GitHub workflow. No local Fedora commands or Weather UI changes are required.

## VERIFY

GitHub Actions generates client and server C protocol bindings with wayland-scanner, compiles native libwayland server and client code, and sends actual WATCH and CLOSE requests over two independent Wayland sockets. Required assertions cover: same namespace labels; two binds; accepted native peer candidate; foreign native connection rejected despite matching its own host resources; invalid source; simulated unmapped host; CLOSE refusal; and zero authority. All tests remain isolated from production SwayPX. CI status must be checked on the new PR.

## WATCH OUT FOR

The selected client is fixture-selected by C code, **not paired by an authenticated operator gesture**. The test host resources are native Wayland objects but not actually mapped layer-shell surfaces; PR #36 separately establishes real mapped duplicate-namespace layer-shell behavior. Simulated mapping/source checks live only in the disposable test, not in compositor admission. Global publication is deliberately permissive in the fixture so request-time enforcement cannot be skipped. The actual native Qt/Quickshell same-connection bridge and live browser/input/popup guards remain absent.

## CHECKLIST

- [x] Read Syntagma AGENTS and Hospital/working protocols
- [x] Recheck PR #38 HEAD and Weather worker ownership boundaries
- [x] Add disposable protocol XML and real wire request fixture
- [x] Prove request-time foreign-client denial with two real clients
- [x] Add CI build and no-production-registration guard
- [ ] Verify CI on GitHub and correct isolated test errors
- [ ] Operator decision on RFC #37 trust bootstrap before real SwayPX pairing/global work
- [ ] Independent Weather Qt/Quickshell same-client bridge and acceptance
- [ ] Host map/output/generation revocation, session admission and broker integration
- [ ] Scene, popup, input and teardown guards, isolated playback and operator acceptance

## NEXT

After green CI, the first production-facing protocol/paired-client milestone is blocked on the RFC #37 trust-root decision and cross-owner coordination. Until then, do not introduce an executable SwayPX WATCH global or treat a namespace or native pointer as consent.

## DECISIONS

No production source changes, no merge, no local Fedora mutation, no Weather Station files edited, no trust architecture chosen, no live Hospital registration, and no browser/scene/input authority activated.
