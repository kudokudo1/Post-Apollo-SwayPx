# 🏥💜 Stage Director M27 // Company private launch intent vs Qt XDG process origin

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native containment
STAFF ROLE // Nurse 2 (operator-authorized external Company integration owner)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL HOSPITAL-COMPATIBLE; NOT LIVE REGISTERED
ASSIGNMENT // Correlate a private, physically paired Company launch request with a real observed XDG Qt child by kernel birth time and direct lineage, without granting actor authority
BASE // master 6be21511de611e70d81959be5dd3be859bc4f714 (M25 merged)
DEPENDENCIES // Company M26 #1385 merged; SwayPX M19–M25 merged
PHASE // Isolated GitHub source implementation, CI and review required

## IMPLEMENTATION

The actual installed map launch chain is Quickshell `CompanyVectorSidecarController.Process` → `CompanyVectorSidecarLaunch.py` → owner-local M15 fixed-path `CompanyVectorHostBridge.py` → native Qt MapLibre `company-vector-probe`, with two `os.execv/execve` replacements preserving Linux child PID and start-time ticks. The Company M26 client sends `requestLaunch(offline|online)` to the M23 private selected-wl_client protocol only when the existing controller accepts the explicitly chosen launch. **The Qt actor itself does not yet carry an authenticated birth token**, and Quickshell launches unrelated children too.

M27 improves the evidence, without claiming that a near-time child is necessarily the exact authorized map:

1. M23 `launch_intent` now uses compositor-owned Linux `clock_gettime(CLOCK_BOOTTIME)` and `_SC_CLK_TCK` to record boot-relative intent ticks and a monotonically increasing private request sequence after native selected-wl_client, geometry, epoch, and offline/online mode validation. Missing kernel time or counter exhaustion rejects the intent. Neither QML nor the map supplies the timestamp or sequence.
2. `stage_director_company_vector_protocol_intent_snapshot` is a native compositor-only function. It returns intent fields **only** for the exact currently selected, bound Wayland host client and a fresh native host revision. Closing/revoking the host invalidates the snapshot.
3. `stage_director_company_origin_check` accepts a candidate only when the private intent is live, its native host-client identity matches the real M20 host candidate, the Qt XDG candidate is a different native client of the same UID, M22's kernel-proven direct-child lineage passes, and the observed Qt process birth lies within a conservative window: up to 2 seconds before the intent (the Quickshell Process may spawn before the request reaches Wayland) or 5 seconds afterward. Old/manual/reused processes and malformed modes or clocks fail.
4. The existing M22 native map observer logs `[stage-director][COMPANY_M27][ORIGIN]` with this evidence, but does not register an owner, reserve M21's candidate session, mint an M19 compositor-selected lease, move the map, change any scene node or route input.
5. Pure native C adversarial tests and exact-source boundary checks accompany a full pinned-Nix SwayPX compositor compile.

## HOW TO USE

No additional UI button or local install. The user continues with **Company → Coverage → VECTOR VIEW → OFFLINE TEST**, or explicitly consented **ONLINE STREETS**. Source-only M27 cannot affect the local compositor until separately installed and enabled through a verified, recoverable activation procedure.

A future approved SwayPX install with M24 Company host physically selected and M25/M26 bridge loaded can emit M27 diagnostic events; `candidate=yes` is **not** authorization or proof of embedded pixels.

## VERIFY

- Syntagma AGENTS / Hospital Agent / Authority rechecked.
- SwayPX master 6be21511 and Company main 576a76bc current as of preparation; no new overlapping active Company Stage Director PRs.
- Real M15 chain, M23 source protocol, M24 physical selection, M22 Linux process lineage, M20 native Wayland adapter and M26 QML control flow inspected.
- Dedicated M27 C cases cover near-time valid intent, offline/online mode, timing bounds (2s before, 5s after), manual old/late Qt actor, missing/replayed epoch/sequence, foreign native client, wrong process UID, unmapped view and rejected parentage. Existing M22 regression rerun.
- Complete ordinary SwayPX Nix build and repository full regressions must pass on exact PR head.
- No live Qt host pairing or per-launch process identity has been operator-tested.

## WATCH OUT FOR

- **Critical limitation:** Process parentage, timing and matching Wayland client are *necessary evidence*, not sufficient to tell a Company-launched map from another nearby Quickshell child. The existing M26 QML intent can be queued after a child has already spawned. This is why a two-second retrospective window is permitted, and why M27 must never grant scene/seat authority.
- For real custody, a later milestone needs a compositor-owned launch challenge and a protected receipt from the actual M15 child process; verify executable identity, PID/start-time, process-generation continuity, and association to exactly one consumed intent. The Qt child should not gain a raw, reusable secret from public process command-line or environment.
- M25's same-QPA Qt module must be installed separately in matching Qt and authorized; M26 `vectorBridgeOptIn` is false by default. M27 is inert on normal desktops.
- App ID, namespace, process name and window title remain spoofable. Do not promote M27 evidence or Weather WATCH authority to native scene clipping, popups, pointer or focus.
- No local host files, package installation, Quickshell/SwayPX restart or worktree reset occurred.

## CHECKLIST

- [x] Recheck current GitHub and Syntagma/ownership
- [x] Inspect exact M15 exec launcher and M26 user-click flow
- [x] Implement private M23 boot-relative intent timestamp/sequence
- [x] Add selected-client and fresh-host native intent snapshot API
- [x] Apply M20/M22 actor/native lineage plus time-bound policy
- [x] Add read-only compositor diagnostic
- [x] Add unit tests, source boundary and full SwayPX build CI
- [ ] Verify all exact-head GitHub checks
- [ ] Review/merge if green and unchanged under operator-delegated scope
- [ ] Authenticated M15 child birth receipt tied to one private intent
- [ ] Connect M21 revocable native actor ledger and geometry receipt
- [ ] Controlled compositor scene/size/crop, mouse/keyboard/popup, cleanup tests
- [ ] Approved local deployment and operator final visual acceptance

## NEXT

After exact-head CI passes, merge the isolated read-only M27 source. The next Company implementation must produce an **authenticated per-launch child birth witness**, not just another window-title or PID filter. Design that as a separate broker or compositor-owned spawn/FD handshake. Only after it proves the map's actual process generation can M21 candidate actor and M23 viewport be used to test controlled pixel/seat containment in isolated CI.

## DECISIONS

Scope stays bounded to Company Stage Director; Weather WATCH and unrelated Chrome experimental rooms remain untouched. No scene/seat authorization is created by this milestone. Hospital-compatible continuity is maintained without claiming active live Hospital registration. No local-machine modifications or production activation.
