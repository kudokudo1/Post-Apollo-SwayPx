# 🏥 M32m // Company protocol revocation and read-only admission preflight

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre in-widget embedding
STAFF ROLE // External Hospital-compatible Nurse 2 (live registration unverified)
ASSIGNMENT // Extend real M24/M28b/M29/M30a evidence toward a safe scene lease; close protocol-revocation escape
BASE // M32d PR #126 head `2b315b699ced8d45601f84bcf3a8bf6689fce3d0`, which contains merged M32e–M32l CI
PHASE // Reviewable production-disabled security preparation
STATE // DRAFT, NOT OPERATOR ACCEPTED

## IMPLEMENTATION

The current Company protocol already has genuine physically selected native Company host resources (M24), private request intent/viewport (M27/M32a), kernel-held child pidfd (M28b), current installed ELF inode evidence (M29), and compiled operator-source ELF digest comparison (M30a). **None authenticates the operator-approved launch or creates a scene lease.** The compiled digest manifest remains empty.

M32m adds a compositor-internal `stage_director_company_vector_protocol_admission_preflight()` which *reobserves* the current selected native Company host and checks the current protocol intent, viewport, kernel-held mapped actor, executable inode, digest and M32b pair policy. Its result is an explicit blocker/status, not an authority grant. Every outcome, **including an eventual exact ELF digest match**, returns `operator_authorized=false`, `authenticated_launch=false`, `scene_lease_issued=false` and all render/input/focus/popup permissions false. No new Wayland message, public IPC, executable approval, or production scene caller is introduced.

M32m also closes a native lifecycle boundary: `stage_director_company_vector_protocol_revoke()` now calls `stage_director_company_scene_revoke(NULL)` **before** clearing any selected-client, intent/viewport or pidfd identity. This covers explicit protocol CLOSE and native role/host/binding teardown, including cases where the Company layer is still mapped.

## HOW TO USE

No desktop action. Existing Company → COVERAGE → VECTOR VIEW behavior is unchanged. Production embedding is OFF. Internal trusted compositor callers can inspect the read-only result in a later separately reviewed integration.

## VERIFY

The new `stage-director-company-m32m.yml` CI gates require the real pidfd/executable/digest checks, an explicit `OPERATOR_LEASE_ABSENT` result, no possible positive authority flags, and scene revocation ordered before protocol state cleanup. It also compiles normal and private SwayPX binaries, checking the CI-only positive scene marker stays out of the ordinary binary.

**Limit:** This is source/build evidence, **not** a live selected-Company protocol CLOSE runtime test and not proof of an actual production lease issuer. Previous M32l real Wayland one/two-output tests passed on parent head, but do not exercise this new protocol revoke seam.

## WATCH OUT FOR

Production digest remains unpinned by design. The read-only probe has no production caller and must not be wired to a public Wayland request or QML boolean. Repeating PID/inode/digest probes after approval may have bounded synchronous cost. A future authenticated lease must bind physical operator intent, exact launch generation, mapped actor, approved binaries/dependencies, native output/viewport epoch, TTL and revocation; this change issues none.

## CHECKLIST

- [x] Reuse existing native/Kernel identity and compiled manifest evidence
- [x] Make candidate-only admission statuses explicit, with zero authority bits
- [x] Revoke active native scene before protocol identity teardown
- [ ] Verify exact-head M32m CI
- [ ] Add real selected-client CLOSE/destroy negative test with live private Company scene
- [ ] Implement independently authorized operator issuer, executable approval and bounded lease
- [ ] Wire actual Company VECTOR VIEW and Fedora dual-display acceptance

## NEXT

Run exact-head CI, then add a genuine protocol CLOSE/selected resource destruction runtime test. Later design/implement a compositor-owned operator authorization issuer after operator approval, not before.

## DECISIONS

This is a **new draft stacked branch**, separate from still-draft M32d PR #126. No merge, production activation, Fedora changes, digest pin, live Hospital registration, operator approval or final DONE.
