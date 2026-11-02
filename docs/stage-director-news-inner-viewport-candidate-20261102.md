# Stage Director SD1 // Inner NEWS viewport candidate contract

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
BASE // `master` `a5ddcb69924911be1286dbe136b183f572fca2fe`
CONNECTION // External Hospital-compatible; live Hospital registration not verified
STATUS // Candidate-only native policy implementation; GitHub verification pending

## IMPLEMENTATION

After PR #54's versioned native Weather layer geometry, introduce a small **read-only interior NEWS viewport preflight**. It checks that an asserted inner rectangle matches the current host/client/output/epoch and **exact host geometry revision**. It then reuses `stage_director_viewport_plan()` to require the inner rectangle inside the current mapped host and output, and covered by an observed browser actor.

The request includes a distinct inner native surface reference and a native-observation precondition. These **do not authenticate interior geometry**. The caller must prove actual interior provenance through a future compositor-owned path; current QML/IPC may not supply native references or authority. A successful result is `GEOMETRY_CANDIDATE_ONLY` and proposes only an actor-local crop rectangle. It is not a renderer transaction, trusted session lease or pointer/focus/popup gate.

A positive preflight always returns `can_launch=false`, `can_render=false`, `can_clip=false`, `can_accept_input=false`, `can_show_popup=false`. Source is linked into ordinary SwayPX but has **no production caller**. No Weather Station QML, host compositor input, or operator workstation files are modified.

## HOW TO USE

Review the draft PR's native host-session workflow and C regression. There is no live NEWS playback action.

## VERIFY

- Native two-client session: matching current revision and valid inner bounds yield **candidate-only**, with an actor-local crop proposal and all authority false.
- Old revision after host movement, wrong client, lost session, no inner native observation, inner surface aliasing the host, rectangle outside chassis or actor, unverified actor all fail.
- Source CI prohibits launch, scene mutation, input, focus and public Wayland-global operations in this module.
- Production SwayPX full build links the module; no CI-only scene selector and no live browser issuer.

## WATCH OUT FOR

**The actual NEWS pane's compositor-authenticated geometry is still unknown.** This API accepts a caller's *preflight data*, not a credential. Never use a QML-provided rectangle, namespace, title, or boolean as the sole basis for browser rendering. Do not turn `GEOMETRY_CANDIDATE_ONLY` into paint/input/spawn permission.

The Chrome moved-mask #52 evidence remains a single fixed CI transition. Multiple movement/reallocation tests, popups, input, keyboard, focus, production process sandbox, Weather Qt native bridge and operator acceptance remain open. #51's failed Chrome outer-window move stays excluded.

## CHECKLIST

- [x] Recover merged #54 and its 9/9 passing workflows
- [x] Introduce minimal inner candidate preflight by reusing existing viewport policy
- [x] Cover stale revision, foreign peer, invalid bounds and zero authority in native session tests
- [ ] GitHub native-session and ordinary SwayPX build checks green
- [ ] Coordinate the independently owned NEWS inner surface geometry provenance
- [ ] Repeated actual Chrome mask transitions and abnormal exit pixel proof
- [ ] Popup, focus, pointer, keyboard containment and production broker
- [ ] Operator desktop acceptance

## NEXT

On successful checks, continue with isolated repeatable Chrome SceneFX mask tests and authentic inner-surface provenance. Preserve the no-activation boundary until all containment guards are actually proven.

## DECISIONS

Do not merge historical #23–52 in bulk. Work from master, preserve #51 exclusion, keep QML ownership separate, and do not claim that geometry checks authorize rendering. No local Fedora modifications or live Hospital registration.
