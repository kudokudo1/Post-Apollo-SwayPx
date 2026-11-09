# 🏥💜 Company M32a // Production viewport state for in-widget native MapLibre

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company MapLibre native embedded viewport, production actor admission
DOCTOR // Nurse 2, external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE; no verified live Hospital registration
STAFF ROLE // Nurse 2 (operator-authorized role; development scope only as explicitly assigned)
ASSIGNMENT // First bounded production implementation step: retain/revalidate actual Company Wayland map rectangle for subsequent native actor render/clip/input admission
PHASE // M32a implementation, draft CI
STATE // IMPLEMENTED IN BRANCH; FULL EMBEDDING NOT YET AVAILABLE
REPORT KIND // IMPLEMENTATION CHECKPOINT
BASE // SwayPX `master` exact starting commit `c3b2f6db13e13d1475cc841f47cf3e8173bd647d`
OWNERSHIP // SwayPX Company private Wayland viewport wire and read-only observation. No Social renderer or Company QML edits.
PERMISSIONS // GitHub branch, draft PR, unit and normal CI build; no local Fedora machine mutation, merge, production activation or digest pin.

## IMPLEMENTATION

The operator's actual goal is **Company → COVERAGE → VECTOR VIEW → live interactive MapLibre inside Company's own map rectangle**. A separate native Qt window is not acceptance. Existing [M30o](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/110) private compositor CI proved placing/panning/resizing the map in the actual Quickshell Company panel; M31c [PR #122](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/122) proved the generalized SceneFX pixel-alignment correction. Their test-only scene handler was **not** production scene/input admission.

The specific missing production-data seam discovered in source: `stage_director_company_vector_protocol.c` receives Company's **real selected-client** private Wayland `report_viewport(frame,map)` call. It validated the rectangle with `stage_director_company_wire_check()`, but discarded both submitted rectangles, retaining only a `bool viewport_candidate`. Thus a later native compositor actor placement operation had no owned, revocable viewport geometry to use.

M32a adds actual production source, NOT just new CI code:

- `include/sway/stage_director_company_viewport_state.h` and `sway/stage_director_company_viewport_state.c`: a compositor-owned viewport candidate keyed to the **currently selected host Wayland client**, selection epoch, launch-intent sequence, and fresh native-host geometry revision. Each valid report replaces the old map rectangle. A hidden/malformed report clears it. Each snapshot rechecks exact native host bounds and generation and returns a copy marked `candidate_only`, with **zero rendering, clipping, focus or seat authority**.
- `sway/stage_director_company_vector_protocol.c`: the production private Wayland handler now **stores the actual report**, resets on new launch, close, invalid/hidden report and native selection revocation, and exposes a narrowly scoped compositor-private `..._viewport_snapshot()`. No client controls native identities or state generations.
- `include/sway/stage_director_company_vector_protocol.h`: new internal snapshot API. The protocol XML and public Wayland request surface are unchanged.
- `sway/stage_director.c`: normal SwayPX Company actor observation now **consumes the selected native Company viewport snapshot** and emits a read-only, source-grounded debug diagnostic. It cannot position/reparent a window or redirect input. This demonstrates a production data-path use, not a claim of visual integration.
- `sway/meson.build`: compile the stateful module into the **normal** SwayPX package, unlike the old CI-only crop experiment.
- `tests/stage_director/test_company_viewport_state_m32a.c`: genuine production C module tests for valid map coordinates, close/reset, stale epoch/intent/host revision, wrong native client, hidden and invalid map, malformed geometry, compact resize and old-view refusal.
- `.github/workflows/stage-director-company-m32a.yml`: compile actual production C modules and independently build full normal SwayPX with pinned Nix dependencies. No CI actor may deploy.

This is the first implementation stage of a **production** Company in-widget map seam. The code is deliberately conservative: only validated geometry state becomes available to SwayPX's future authenticated native scene controller.

## HOW TO USE

There is **no new finished in-widget VECTOR VIEW control yet**. Existing `Company → COVERAGE → VECTOR VIEW` still opens a separate Qt window, and `Company → COVERAGE → STREETS` still uses the existing raster embedded map. M32a by itself should not be pulled into the user's live compositor as a finished feature.

Inspect the M32a PR and its two CI jobs. When SwayPX is eventually installed after the complete authenticated pipeline is approved, the private Company report's current map rectangle will be observable internally by SwayPX without any new public protocol surface. This branch never changes `~/.config/quickshell`, restarts SwayPX, installs native code or enables Company opt-in.

## VERIFY

Required before technical acceptance:
1. The new production `stage_director_company_viewport_state.c` compiles and passes the C runtime test.
2. The real private Wayland handler records only valid selected-client reports, wipes them on negative reports and selection loss, and has no scene/seat call.
3. The normal SwayPX Nix build compiles and links the production state implementation, not a CI-only actor.
4. Existing operator-approved native executable digest pin stays **empty**; `scene=OFF input=OFF` remains operative.
5. No Company or Social production source changes, no Fedora host writes.

This stage does **not** prove the map visible inside normal Company. It proves the first missing production state needed by the actual embedding controller.

## WATCH OUT FOR

The Company client's rectangle is a **candidate**, not authorization. A compromised app could lie about coordinates inside its own frame, and a previously valid report becomes stale after host movement. Compositor consumers must revalidate on each use and refuse old host/actor/process generations.

Current production blockers remain:
- `stage_director_company_digest.c` trusted native executable SHA-256 manifest is intentionally empty. It cannot be populated from a CI binary or arbitrary local process without operator approval.
- No authenticated native actor **scene placement/crop/input** is connected in default SwayPX. The old M30o code is test-only.
- Company `vectorBridgeOptIn` still defaults false and VECTOR VIEW still operates the existing separate launcher.
- The full cross-Patient integration requires coordination with the Company widget and Social/Stage Director owning workers, plus physical Fedora two-output/focus/keyboard validation and visual acceptance.
- No live Hospital registration/ownership transfer is claimed.

## CHECKLIST

- [x] Re-read current Syntagma and Hospital protocols, current SwayPX/Company main and overlapping PRs
- [x] Identify exact missing live Company viewport persistence after existing private Wayland report
- [x] Implement compositor-owned production viewport storage, generation checks and revocation
- [x] Wire the real private protocol to the stored viewport and normal SwayPX actor observation
- [x] Add production module to normal SwayPX build and negative C tests
- [ ] Verify exact-head M32a CI (unit + normal SwayPX Nix compile)
- [ ] Design/implement operator-approved native executable custody and actor lease
- [ ] Implement compositor-owned crop/scene placement + mouse/keyboard containment and immediate revocation
- [ ] Wire Company VECTOR VIEW and the actual map canvas to the proven scene lease
- [ ] Verify on the operator's Fedora machine, on both outputs, and obtain final visual acceptance

## NEXT

Finish M32a native/normal SwayPX CI. Continue into M32b on a non-colliding branch: consume the stored viewport together with an actual selected native Company host, kernel pidfd+ELF digest-approved actor lease, and source-driven close/revoke. Do **not** enable rendering until identity, geometry, and revocation can all be checked in the compositor from real native objects.

After M32b, complete guarded scene crop, input/focus routing and Company QML button activation. The precise acceptance test must be **one embedded native MapLibre panel, no external window, with pan/zoom/resize/close working**.

## DECISIONS

M32a is an **unmerged production-source infrastructure candidate**, not a deployed/activated embedding. It adds no production scene/input authority and does not change the empty digest pin or Company opt-in. The operator retains all merge, local-change, and final DONE decisions. Recovery branch `stage-director/company-m32a-embedded-viewport-state-20261109`, base `c3b2f6db`, CI and PR evidence attached after verification.
