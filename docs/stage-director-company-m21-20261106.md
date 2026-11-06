# Stage Director M21 // Revocable Company native candidate ledger

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre containment
STAFF ROLE // Nurse 2, external Hospital-compatible continuation, no live Hospital registration
ASSIGNMENT // Build an exact native Wayland resource lifetime ledger and bounded one-use map-viewport report
BASE // M20 draft PR #75 at 5136158d749dbfe0fe2d2efab856aab5f5c29714
DEPENDENCY // M19 PR #74, Company M18 #1365 merged, M19b #1369 merged
PHASE // Isolated branch and CI; not installed

## IMPLEMENTATION

M20 provides read-only observations of mapped Company layer and native Qt XDG map root, with real wl_client credentials and stable Linux process generation. M19 provides fail-closed geometry-only policy. M21 adds `stage_director_company_candidate_session.c/.h`, a **non-authoritative** ledger linking the exact observed native Company layer role, host wl_surface and separate Qt map root via actual Wayland resource identities.

- `record()` accepts only native observation candidates matching the actual resources and separate clients, with distinct process identities and matching UID. It issues a monotonically increasing **candidate epoch**, not an authenticated owner lease or production launch permission.
- The three real native `wl_resource` destruction listeners revoke the entire candidate on layer-role, layer-surface, or actor-root destruction. Explicit close removes listeners. Replacing a native process generation also revokes.
- `refresh()` demands matching fresh host/actor identity and geometry, invalidates the previous crop on host/output/actor movement, and advances the candidate revision.
- `report()` accepts at most one geometry hint per native refresh, only from a *separate* reporter resource belonging to the selected Company host Wayland client. It rejects stale epoch/revision, hidden/closed map, malformed frame/canvas geometry, wrong output, and an actor that doesn't cover the viewport. An invalid same-client replacement clears any previous viewport.
- `receipt_current()` confirms the exact current epoch/revision, viewport and crop, with all scene, clipping, focus, popup, keyboard, input, launch and owner flags false.

The caller is responsible for verifying a future Company-specific private protocol resource interface before invoking report. This milestone does not create that interface or a Wayland global. It does NOT select the real operator's Company or M15-launched Qt map: native snapshots are necessary evidence, not authenticated operator consent or launch custody. The Company M18 rectangle is measured relative to `companyFrame`; the **separate** `frame_in_host` must eventually be reported from the verified same-Qt-QPA-client Company host connection.

`sway/meson.build` compiles M21 into ordinary SwayPX but no production code calls record/report and no window/pixel/input action is wired. Existing Weather WATCH behavior and all old Chrome experimental branches remain untouched.

## HOW TO USE

No new UI action for this source milestone. The operator's installed Company → Coverage → VECTOR VIEW continues to launch the original separate MapLibre Qt window. GitHub tests are the only active M21 usage. Do not restart Quickshell/SwayPX, install software or replace local files without new operator approval.

## VERIFY

- M19/M20 exact GitHub heads and Syntagma rules checked.
- Native server-side tests use actual Wayland `wl_resource` objects, two distinct wl_clients, a foreign reporter, and destroy callbacks. Synthetic observation records in the test are **fixtures**, not live process attestation.
- Tests exercise wrong resource, reused actor start-time, stale epoch/revision and geometry, hidden map, and real host-role/host-surface/actor-root teardown.
- CI compiles native unit tests with `-Wall -Wextra -Werror -pedantic`, preserves M20, checks no scene/seat/launch authority, and builds actual standard SwayPX in pinned Nix.
- A successful build will NOT prove a real Company pairing or clipped window.

## WATCH OUT FOR

- `record()` is a candidate observation API, not a production pairing issuer. There is intentionally no call site in `stage_director.c`, no special keyboard shortcut, no private Company protocol and no on-disk credential.
- A foreign user process sharing a spoofable namespace or app ID cannot be trusted. UID plus distinct Linux PID does not establish Qt map custody. A later approved physical-input selection and M15 launch attestation are still necessary.
- The `report()` C function checks the Wayland client; a future private interface handler must check the exact `wl_resource` interface and epoch too.
- The M21 revision is an internal candidate geometry generation (host or actor movement); M19's eventual compositor host revision must be reconciled by the authorized producer before wire activation. No untrusted client can claim a native reference.
- Actor viewport containment alone does not prove first-frame clipping, popup/seat/focus/sibling safety or pixel cropping.

## CHECKLIST

- [x] Confirm current M19/M20 dependencies and Company merged sources
- [x] Create separate M21 branch on M20 head
- [x] Implement real native resource lifetime listeners and candidate epoch
- [x] Implement native geometry refresh, bounded one-use reports and revocation
- [x] Add native test matrix and full compositor CI
- [ ] Verify exact-head tests and complete compositor build
- [ ] Draft PR code review, authorized merge in stack order
- [ ] Future M22 trusted physical selection / M15 actor launch custody
- [ ] Future M23 private Wayland geometry report and live QML bridge
- [ ] Future M24 isolated actual native scene crop, first-frame/input/popup safety
- [ ] Operator final acceptance

## NEXT

Check the exact PR's native and full compositor CI. If successful, keep the stack reviewable. The next work cannot safely grant scene authority until (1) SwayPX owns an authenticated Company selection, (2) a held process identity proves the exact M15 map actor, (3) the Company private Wayland reporter is authenticated, and (4) separate pixel, input and popup tests pass.

## DECISIONS

User delegated bounded Stage Director Company implementation. M21 remains a candidate-only repository draft, never an approval to merge, modify local files, restart the compositor, or self-register in Hospital. No final acceptance assumed.
