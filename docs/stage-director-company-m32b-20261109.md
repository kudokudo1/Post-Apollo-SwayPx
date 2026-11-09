# 🏥💜 M32b // Native Company actor + actual widget viewport pairing

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company MapLibre native in-widget embedding
DOCTOR // Nurse 2, external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE (no verified live Hospital registration)
ASSIGNMENT // Join M32a's private actual Company viewport to the observed native XDG MapLibre process + held kernel pidfd + actual installed ELF evidence for a compositor-owned target rectangle
PHASE // M32b production-source implementation, CI verification
STATE // DRAFT; scene/seat/clip still prohibited; operator visual acceptance pending
REPORT KIND // IMPLEMENTATION CHECKPOINT
BASE // SwayPX `master` `af421aaaa9a232e1879806794a2437dfd3544d4c`
STAFF ROLE // Nurse 2, external Hospital-compatible
SCOPE // Native Company actor/viewport pairing in SwayPX only; no Company QML, Social, installation or global compositor policy changes
PERMISSIONS // GitHub branch and draft PR, source implementation plus CI, NO production deployment/merge/host writes

## IMPLEMENTATION

Operator target: **Company → COVERAGE → VECTOR VIEW** must display the interactive native MapLibre window INSIDE Company's actual map rectangle, without a second floating window. The current M15 sidecar creates a separate Qt XDG window, and M30o CI had already proved overlay placement and pointer events inside a disposable real Company panel. Rendering correctness is separately proved by M31a/b/c. The user already had a standalone viewer and does not want another.

The preceding [M32a PR #123](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/123) **merged** to normal SwayPX master. It preserves the exact Company's private `report_viewport` frame/map rectangles only for the selected native Company Wayland client and current launch sequence/selection/host revision, wiping stale geometry.

**M32b moves the other half of the production connection into SwayPX:**

- `include/sway/stage_director_company_actor_pair.h` and `sway/stage_director_company_actor_pair.c` join compositor-observed native Company overlay role/surface/client/output, M32a's actual selected-client intent and geometry, native XDG toplevel/root/Wayland client, same-UID direct-child lineage, matching **live kernel pidfd**, installed native Qt binary inode, and the independently pinned executable digest result.
- The code computes **the intended global map rectangle** from the real host's current compositor coordinates and the Company's nested frame/map geometry, including negative-origin secondary monitors. Unlike the old geometry policy, it does not require the currently separate XDG window to already overlap the Company frame. Its native actor root is retained as diagnostic data, not moved or cropped.
- A mismatched client, stale selection epoch/launch generation/revision, missing pidfd actor match, invalid geometry/overflow, wrong process UID/parent, changed installed executable inode, missing or different compiled ELF digest all have explicit refusal reasons. Absent executable pin is `DIGEST_UNPINNED`; a matching synthetic digest would still be `EXACT_ELF_CANDIDATE_ONLY`.
- `sway/stage_director.c` normal SwayPX native actor observer **actually calls this source function** using the existing M24 selected host, M27 intent, M28 pidfd match, M29 executable, M30 digest, and M32a private viewport evidence. It emits a read-only diagnostic `COMPANY_M32B ACTOR_TARGET` with target and refusal status. This is not an untrusted app-id-based admission: app-id remains a debug filter, not an identity issuer.
- `sway/meson.build` compiles M32b in the **normal compositor binary**, not as an isolated CI-only instrument.
- `tests/stage_director/test_company_actor_pair_m32b.c` checks an existing off-widget native top-level, exact intended 650×450 target, stale generations, foreign clients, faked process, missing pidfd, malformed and overflow geometry, mismatched executable/digest, negative-monitor coordinates, and **no scene/seat permissions even if the digest matches**.
- New `.github/workflows/stage-director-company-m32b.yml` builds the actual production source C module under strict `-Werror -pedantic` and full normal SwayPX Nix.

**This is still not a render lease.** It makes the correct native actor and target rectangle available together in normal SwayPX. It does not reparent the actor, set a scene crop, send seat events, open popups or override focus.

## HOW TO USE

No new visible button or finished embed in this PR. The actual user workflow stays Company → COVERAGE → VECTOR VIEW, currently separate-window behavior. The M32b log is an internal diagnostic; it should not be presented as user-visible success.

Review the M32b draft PR and CI. Do not run a separate preview as a substitute for the in-Company feature. Do not install this branch into the live compositor until end-to-end scene and input admission, physical dual-monitor verification and operator approval are complete.

## VERIFY

Required:
- [ ] Build and run the actual M32b C policy against existing production viewport/wire and multiple reject cases
- [ ] Build normal SwayPX with M32b source linked
- [ ] Check current master and independent PR ownership before future edits
- [ ] Confirm compiled trusted ELF digest remains intentionally empty and normal scene/input are OFF
- [ ] Validate separate original Qt viewer remains distinct from in-panel behavior

A green CI **does not** mean the user's live embedded Company map exists. It means the normal compositor now has the source-derived actor + target pair needed by the upcoming controlled scene controller.

## WATCH OUT FOR

- `sway/stage_director_company_digest.c` has a deliberately empty compositor-approved binary SHA-256 pin. This PR **does not fill it**, approve a binary, install a Qt program or enable production rendering.
- The M32b C result is instantaneous and non-authorizing; caches or client-supplied identity/digest values could be spoofed. Later scene operations must reacquire fresh compositor evidence and bound a selected native actor root to its kernel process generation on every lifecycle change.
- A matching on-disk ELF alone is not sufficient proof of loaded Qt plugins, intended Company click or user approval. Untrusted Wayland client metadata cannot be used to mint a lease.
- The map may not yet cover Company, even when the intended target coordinates are correct. **Actual compositor scene placement/cropping and bounded input** remain unimplemented in production.
- Cross-Patient work in Company QML and Social Stage Director's output/input scene owner must be coordinated rather than silently edited by this one SwayPX assignment.
- The browser doctor is Hospital-compatible only; no registered live Hospital session or chart write is claimed.

## CHECKLIST

- [x] Verify current Syntagma, M32a merge, current SwayPX master, Company main, and concurrent open PRs
- [x] Implement actual compositor actor-to-viewport target pairing from native/custody evidence
- [x] Compile new module into default SwayPX and call it from actual Company XDG observation
- [x] Add negative lifecycle, spoofing, output bounds, actor-offset, and unpinned digest tests
- [ ] Exact-head M32b CI passes and owner review
- [ ] Authenticated compositor-owned actor/scene admission and crop/resize/clip, with immediate revoke
- [ ] Confine pointer/scroll/keyboard/focus/popup and click-through
- [ ] Replace Company VECTOR VIEW separate launch with authenticated embedded activation
- [ ] Operator physical Fedora two-output visual acceptance and authorized deployment

## NEXT

After M32b CI, take a separate **M32c guarded scene** change owned by SwayPX/Stage Director: (1) obtain operator-approved, source-pinned native executable identity, (2) convert fresh selected host+actor+viewport into a revocable compositor-authenticated lease, (3) apply scene placement/crop and seat/keyboard focus boundaries only under that lease, (4) tear down on any host/process/epoch/mode/output changes. Then wire the actual Company VECTOR VIEW control to the admitted native surface rather than another floating desktop window. Test against the real Qt app and Company Quickshell panel; operator gets final visual acceptance.

## DECISIONS

GitHub draft only; no production actuation, no app bundle install, no output configuration, no global Stage Director change, no unapproved hash pin or Fedora write. M32b remains **candidate evidence**, not a scene/focus policy. Preserve this report and the exact PR head for handoff; final DONE belongs to the operator.
