# Stage Director // SD1 real Chromium cgroup teardown readiness

STATE // Draft, CI-only real browser execution; no production WATCH activation.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director / Weather Station NEWS integration.

ASSIGNMENT // Verify real Chromium subprocess custody and private browser profile cleanup on isolated GitHub runner, retaining prior Option A fail-closed admission.

WORKER // OPEN AI // CHATGPT, Hospital-compatible external Doctor, no verified live Hospital registration.

BASE // PR #46 head `69ebc16dcb70b4d6e97863a3aa3889aba905b2f7`; PR #43 discovery-leak fix required upstream.

NEIGHBOR // Weather Station worker owns Quickshell Weather QML and native bridge; nothing modified there.

## IMPLEMENTATION

Run **actual Chromium**, not the previous two-process `sleep`/fork fixture, in a disposable GitHub runner Docker container with a distinct Linux cgroup v2. The browser comes from Debian's Chromium package and opens only local `about:blank`. Docker run uses `--network none`, a read-only root filesystem, a non-root fixed UID, dropped Linux capabilities, `no-new-privileges`, default seccomp, bounded memory/processes and a temporary in-memory private Chromium profile. Docker's container runtime is the test's cgroup owner, **not production SwayPX or WATCH**.

The Python test checks runtime policy via Docker inspection, identifies the exact host cgroup from `/proc/<container_init_pid>/cgroup`, requires that it belongs to the container ID, recursively samples its `cgroup.procs` files and observes at least three real Chromium processes plus an init process. It records kernel process starttime markers (not just PIDs) for all observed members, then stops the exact Docker container ID. It refuses PASS if any saved process identity survives or if any member remains in that cgroup after stop. The temporary profile is in an unmounted tmpfs and the container/image are removed even on failure.

This is a **CI-only test**. No new browser broker, production launch method, process authority, scene mutation, focus/input/popup permission or Weather QML change is added. The original single-child pidfd and opt-in group stop from PR #46 remain intact.

## HOW TO USE

Review GitHub draft PR and `Stage Director real Chromium cgroup custody (no activation)` Actions. No operator desktop command or local browser installation. Read both the source guard and actual Docker cgroup test results. If it fails because of Docker / Chromium sandbox restrictions, treat it as a blocker, not proof of readiness.

## VERIFY

A pass requires actual running Chromium with multiple recorded subprocesses, visible unique Docker cgroup v2, private writable profile inside a non-root read-only container, no container network, no Chromium `--no-sandbox` flag, successful runtime stop, empty/deleted cgroup, zero observed process identities surviving by their original starttime, and explicit cleanup. The separate source gate ensures production WATCH/IPC does not invoke the CI fixture. Verify workflow outcome before declaring completed.

## WATCH OUT FOR

- A Docker container cgroup is **not yet** the design for an installed production browser supervisor. The test explores real Chromium ownership and cleanup, while SwayPX still cannot start a browser.
- An isolated process with `--no-sandbox` would not be valid evidence for safe web viewing. Chromium and Ubuntu/Docker namespace restrictions may prevent this test from passing; do not bypass Chrome sandbox via `--no-sandbox`, privileged container, seccomp=unconfined, host network or PID modes.
- Lack of `--no-sandbox` is **not proof** Chromium's internal sandbox is active. Production rollout must verify Chromium child security state, sandbox flags, and site isolation separately.
- Even a passing Docker cgroup test says nothing about SwayPX's real actor/window attribution, oversized Wayland viewport clipping, keyboard/pointer transforms, popups or focus, embedded browser connection or Quickshell native bridge.
- Docker image package provenance is the runner's apt repository, not an immutable pinned image digest. Tighten image/version pinning before using the test as release evidence.
- The user did not approve live Fedora or SwayPX modifications; this runs on GitHub's disposable CI runner only.

## CHECKLIST

- [x] Recheck Syntagma, PR #46 state, existing supervisor/candidate custody and source ownership
- [x] Design fail-closed, bounded real Chromium Docker cgroup v2 test
- [x] Add read-only, non-root, network-off, ephemeral-profile Docker fixture
- [x] Add host cgroup/pid/starttime lifecycle verifier and exact-ID cleanup
- [x] Add GitHub source safety/real execution workflow
- [ ] CI passes and first actual browser process-tree evidence is verified
- [ ] Real production cgroup owner/broker design and Chromium profile lifetime
- [ ] Browser's native wl_client actor association and scene/pixel boundary
- [ ] Popup, focus, input, geometry, crash recovery, and all-descendant custody
- [ ] Actual Weather Quickshell bridge and operator live acceptance

## NEXT

Verify the CI. If Chrome cannot use its sandbox in the isolated runner, preserve the exact error and design a supported sandboxed runner instead of loosening isolation. If green, use the evidence to design a compositor-owned browser cgroup with explicitly verified process/renderer identity, without connecting live WATCH until clipping and input/popup containment are finished.

## DECISIONS

Draft only, unmerged, no local machine mutation, no Weather QML edit, no production browser launch, no Hospital registration. Technical CI is not user final acceptance.
