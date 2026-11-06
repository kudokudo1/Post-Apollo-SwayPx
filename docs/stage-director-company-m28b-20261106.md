# 🏥💜 Stage Director Company M28b // Private Wayland v2 kernel child fd receipt

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native integration
STAFF ROLE // Nurse 2 (externally delegated Company implementation scope)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // Hospital-compatible, not live registered
ASSIGNMENT // Transfer actual attached Quickshell child pidfd through selected Company private Wayland client and hold kernel birth candidate
BASE // master 168b5c5f220e2597b7b7d65c9ec86ce639123596 (M28a #82 merged)
DEPENDENCIES // M23–M28a merged; Company M28c companion PR in The-Post-Apollo-Project
PHASE // Source development / exact-head CI; no local activation

## IMPLEMENTATION

M28a owns a Linux pidfd while validating kernel parent, UID and process generation. M27 separately records selected Company client launch intent and maps a native Qt XDG root candidate. M28b connects those two facts across the *existing* Quickshell Qt/QPA Wayland connection, without new sockets or tokens.

1. Extend canonical `post_apollo_company_vector_v1` XML interface additively from version 1 to version **2**. The old v1 `launch_intent`, `report_viewport`, `close`, `destroy` and `result` remain unchanged. New `report_child_pidfd(uint pid, fd pidfd)` and `birth_result` have `since="2"`; old v1 clients keep operating but cannot submit a birth receipt.
2. M24 physically selected native Company host registers a **compositor-private** fresh host snapshot callback. The Wayland server requires current native selected wl_client, same live M24 layer role/host wl_surface, an exact active M27 private launch-intent generation, callback identity/geometry, a genuine fd received by Wayland and a kernel/Quickshell parent check via M28a. It closes every incoming fd, and M28a holds only a separate CLOEXEC duplicate on success.
3. Native `stage_director_company_vector_protocol_birth_actor_candidate` compares held kernel PID/start-time to a live observed Wayland XDG root with M22 direct parentage and current Company selection/intent/host revision. A diagnostic-only XDG map event logs yes/no. No process is started, window moved, clipped or focused.
4. Update optional `PostApollo.CompanyVector 1.0` QML plugin: `Q_INVOKABLE reportNativeChildPid(qint64)` accepts only the attached Quickshell Process PID passed through Company M28c. It stores at most one pending PID, waits for M23's `LAUNCH_CANDIDATE` response, opens a real kernel pidfd using `pidfd_open` in C++, sends the fd over the **borrowed QNativeInterface::QWaylandApplication display** only when protocol v2 is bound, and closes the local fd after marshalling. A separate `birth_result` event exposes a candidate-only status. Old M23/v1, missing QML module, pidfd failure or child exit fail closed. No second `wl_display_connect`, no parent/Qt process spawn.
5. The Company M28c companion in `The-Post-Apollo-Project` forwards the **actual** `Quickshell.Io.Process.processId` from the existing attached M15 `Process.onStarted`, not a user-supplied number. M15 Python execv/execve preserves that PID into the Qt binary. Company remains `vectorBridgeOptIn:false` until separately authorized matching runtime installation/rollback.
6. Native generated protocol compatibility checks, M28a kernel pidfd tests, M27 intent tests, Qt plugin Nix package build, full ordinary SwayPX build and existing Stage Director regressions apply. No new permissions are granted.

## HOW TO USE

No new user action now. Current Company → Coverage → VECTOR VIEW retains existing **standalone** OFFLINE TEST and separately consented ONLINE STREETS. On ordinary SwayPX startup, the private Company global is still unpublished unless M24's opt-in physical selection gesture is enabled and completed; M26 Qt plugin remains disabled by default. This source is not a live, installed embedded map.

In a future controlled native acceptance session, the operator's Company UI event will emit the genuine child PID to the optional matching Qt module, which sends `report_child_pidfd` only after selected Company `launch_intent` ACK. A status `KERNEL_PIDFD_CANDIDATE_ONLY` means kernel lifetime correlation was observed, **not** that map pixels have been embedded.

## VERIFY

- Canonical Syntagma authority and Hospital protocols, actual M15 single PID exec chain, official Quickshell Process.processId API and M25 Qt module inspected.
- M28a #82 merged exact master and 26/26 CI checks passed before M28b branch.
- Server validates native host and selected wl_client before using kernel pidfd, and always closes the received descriptor. Native selection/path lifecycle revokes held pidfd, including host role/surface destruction and explicit close.
- M28b CI compiles actual version 2 Wayland server interface with `wayland-scanner`, reruns M28a real Linux pidfd lifecycle + M27 tests, rebuilds matching Qt QPA plugin and full SwayPX, and enforces no scene/input authority.
- Not proven: actual user-local Wayland v2 fd round-trip, stable Qt QPA plugin import on Fedora Silverblue 44, M15 executable inode/hash, or native embedded pixel/input acceptance.

## WATCH OUT FOR

- **Security boundary:** A kernel pidfd attests process existence, generation and parentage, but a compromised selected Quickshell client could submit a pidfd of another direct child. M28b still returns `exact_executable_attested=false`, `launch_authenticated=false`, `owner_verified=false`; no M21 lease or clip/seat permissions.
- The v2 server callback must be attached only after a **physical M24 Company layer hit test**. A guessed namespace/app ID is insufficient.
- A v1 plugin binds v1 safely; a matching v2 bridge cannot submit fd until actual M23 launch-intent acknowledgment and its own `reportNativeChildPid` event.
- The M25 client may receive the child PID before launch ACK, so it queues exactly one pending PID and does not replay stale ones after close/revoke.
- An actor may map before the fd reaches the compositor. The current map-time diagnostic is opportunistic; a future M28d must handle late-arriving receipts and match against a stable live XDG inventory without granting authority.
- No local install, startup environment change, system package, Quickshell/SwayPX restart or change to Weather WATCH/Chrome room.

## CHECKLIST

- [x] M28a real kernel pidfd foundation merged
- [x] Private Company Wayland ABI versioned additively to v2
- [x] Selected wl_client/native Company host check for FD receipt
- [x] Kernel-held child matching and revocation
- [x] Optional Qt same-QPA pidfd sender and ACK
- [x] Source/ABI/kernel/Qt/full SwayPX CI authored
- [ ] M28b exact-head CI green and code review
- [ ] M28b merge after authorization gate
- [ ] Company M28c companion PR green/merged
- [ ] M28d exact M15 executable identity and secure per-click launch provenance
- [ ] Connect M21 verified actor candidate/geometry receipt and real popup/input/pixel isolation
- [ ] Controlled local activation and final operator acceptance

## NEXT

Verify M28b and Company M28c on exact PR heads. If all green, merge under the operator-delegated Company integration scope. Then implement exact M15 native Qt executable identity using an owner-checked fixed path/inode and per-launch provenance before elevating any compositor-selected actor privileges. Reconcile asynchronous actor mapping/Wayland fd receipt timing in isolated compositor CI.

## DECISIONS

Keep Weather WATCH and historic Chrome SceneFX branches untouched. Never equate `KERNEL_PIDFD_CANDIDATE_ONLY` with authenticated MapLibre ownership or completion. No local-machine write or production activation under this assignment.
