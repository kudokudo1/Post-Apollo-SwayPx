# 🏥 Company M32q // Operator-authorized native launch and scene contract

PATIENT // `kudokudo1/Post-Apollo-SwayPx` (SwayPX compositor)
DEPENDENCY PATIENT // `kudokudo1/The-Post-Apollo-Project` (Company UI and sidecar)
ROOM // Company → COVERAGE → VECTOR VIEW, native MapLibre in-widget embedding
STAFF ROLE // External Hospital-compatible Nurse 2 continuity; no verified live Hospital assignment
PHASE // Authorization decision / production design review only
CHECKPOINT // M32p PR #132 `2415299bb7e261601a7a13505ee265eecd6f7bcb` (the preceding head)
DECISION AUTHORITY // Operator; no production activation, merge or local Fedora changes authorized

## IMPLEMENTATION // Verified source, not production readiness

The current Company UI already has an explicit `VECTOR VIEW` chooser with `OFFLINE TEST` and `ONLINE STREETS` actions in `widgets/CompanyCoveragePane.qml`. These actions invoke the attached `CompanyVectorSidecarController`, which launches `/usr/bin/python3 services/company/CompanyVectorSidecarLaunch.py`. This script checks the owner-local `~/.local/libexec/post-apollo/company-vector-probe` path and replaces its process with the native Qt binary through `os.execv`. Company sends only a launch *intent* and its attached child PID to the optional Wayland bridge; the bridge remains disabled by default (`CompanyNetworkW.vectorBridgeOptIn: false`).

In SwayPX, M24's physical `Ctrl+Super+Shift+F11` + left-click gesture selects an actual Company layer through libinput only, as distinct from public IPC. It does **not** approve a separate Qt actor. M28b's kernel-held pidfd and M29's inode evidence authenticate the candidate process generation, but not the operator's authorization of this particular execution. M30a's source-pinned expected executable SHA-256 remains **empty on purpose**. M32m's read-only preflight therefore cannot authorize. M32o's five-second candidate ledger and M32p's compositor-owned source bridge both return **zero** scene, input, keyboard or popup permissions. The only positive MapLibre scene experiments are private CI and remain production OFF.

## REQUIRED TRUST MODEL // Decisions and implementation boundary

1. **Operator gesture** must be distinct from M24 host pairing and verifiable by SwayPX from physical libinput, not from a QML boolean, Wayland message, IPC, automation, accessibility injection or a mutable status label. A Company-painted confirmation may *describe* the requested operation but cannot be the sole authentication. Do not silently pick a new hotkey or redefine the existing F11 ceremony; the operator must approve the final interaction.
2. **Explicit operation scope** must include OFFLINE versus ONLINE, exact selected Company native host, target output, exact in-widget map rectangle, and launch generation. Changing mode, workspace, output, scale, Company visibility or requested geometry invalidates the consent and scene.
3. **Executable trust** must be established by the operator reviewing a reproducible, installed native MapLibre build and its dependency/plugin load set. Pin the authorized source/build and *measured on-disk* executable digest in a reviewed compositor build or approved signed manifest. The current user-owned path is **not sufficient on its own**, since the owner can update that file between approval and execution. Avoid approving the digest from synthetic CI's disposable app.
4. **Actual custody** must bind the approved operator action to the *one process that runs*: preferably a compositor-controlled launch path that opens/verifies the intended executable and captures a kernel pidfd and process generation before admission. If retaining the Company attached QProcess, demonstrate an equally strong anti-swap launch and continuous custody proof; PID equality, `execv`, same parent and elapsed time are insufficient alone. Keep the current standalone Company launcher available as a separate safe fallback.
5. **Actual lease issuance** must be a separate compositor-owned operation after physical operator approval, trusted executable and launch custody, and a fresh M32m/M32p native preflight. Give it a single-use generation, short monotonic TTL, exact actor root and output, current viewport revision and explicit revoke on any loss. M32o's candidate receipt must not be promoted to a real lease merely by toggling an enum or boolean.
6. **Scene/seat coupling** must require a fresh live lease for every relevant render/seat operation, and proactively hide the quarantined Qt actor on CLOSE, host/actor death, output loss, mode change, expired authority, focus or popup attempt. There must be no fallback unmasked floating MapLibre after scene loss. The default normal SwayPX build remains fail-closed until all gates pass and the operator authorizes activation.

## OPERATOR UX DECISION // Pending

**Option A (recommended for Post-Apollo):** Reuse Company's existing `VECTOR VIEW` → `OFFLINE TEST` / `ONLINE STREETS` choice as intent, then add one explicit SwayPX-verified physical approval ceremony **for the exact selected Company session and exact native launch**. The Company widget displays the pending request/identity, expiration, cancellation and success/denial status; the actual approval remains compositor-owned. Keep operator-selected key/button details unresolved until accepted.

**Option B:** Require a trusted OS authorization dialog outside Company, then launch through a compositor-controlled process custodian. This could be easier to audit but adds a separate surface contrary to the in-widget experience. Do not treat an ordinary QML confirmation as a trusted dialog.

Neither option has been approved or implemented. The final choice also requires the Company UI worker's ownership agreement before editing Company controls.

## SEQUENCED IMPLEMENTATION // No hidden activation

1. **Authority decision:** Obtain operator approval of the physical approval flow, exact target screen rules, and whether compositor custody may replace the embedded launch path. Record named Company/SwayPX owners and fallback behavior.
2. **Reproducible native artifact:** Produce a deterministic Qt MapLibre executable/dependency manifest, verify its provenance, obtain operator approval, and pin the reviewed hash securely. No synthetic CI digest is permitted.
3. **Trusted process birth:** Implement a compositor-only launch transaction with exact pidfd and executable identity, bounded cancellation and anti-swap checks, or an independently reviewed equivalent. Run negative tests for replaced binary/dependency, wrong client/actor, stale PID, crash, replay and cancellation.
4. **Physical consent:** Bind a compositor-owned physical input producer to *that exact transaction*, not to a generic app, workspace or QML permission. Test wrong device, virtual input, wrong seat, lock screen, timeouts, double presses and stale sessions.
5. **Real lease and scene:** Implement an expiring issuer that is impossible to reach from public IPC or client-generated messages. Gate the current native scene and pointer/focus/popup guards against the active lease. Preserve the previous private CI tests as regression tests, not as authority.
6. **Company UX and rollout:** Wire status/cancellation and native-vs-standalone fallback in the Company-owned widget after its worker agrees. Run isolated normal/private CI, then operator-approved Fedora dual-monitor and real GPU acceptance with rollback. Only separately authorized merge/production deployment may follow.

## VERIFY // Acceptance gates

- A new QML `true` flag, direct Wayland request, fake pointer, virtual keyboard, window title, app ID, PID, or arbitrary command cannot issue the operator lease.
- A legitimate physical Company selection still does not authorize MapLibre. Consent must be distinct, current and consumed exactly once.
- Replacing the executable, dynamic dependencies, Qt plugins or process between approval and execution denies the launch.
- Moved/resized/hidden Company, output switch, host loss, stale viewport, expired lease and actor exit all revoke without native Qt leakage.
- Under actual Fedora monitors, map remains clipped and interactive after pan, wheel and resize; all Company buttons outside the map remain interactive; no focus theft or popup escape.
- All CI-private positive tests continue passing, normal production build retains OFF until operator-approved flip and acceptance. Operator decides final DONE.

## WATCH OUT FOR // Verified blockers

The current `CompanyVectorSidecarLaunch.py` deliberately uses an *owner-local* executable; ownership and no-group/world-write checks are useful but do not make this an immutable or operator-approved image. `execv` continuity and a held pidfd do not guarantee immutable dependencies or compositor-mediated launch approval. F11 selection is an existing host pairing ceremony, not permission for native map embedding. Avoid assuming that a green test or pinned digest alone solves these problems.

## DECISIONS

No new physical shortcut, production issuer, approved executable hash, privileged spawn path, Company edit, local Fedora change or production deployment is authorized by this document. This records the candidate UX and trust contract for an explicit operator/owner decision, not a completed implementation.
