# Stage Director SD1 // Active NEWS shadow observation

**STATE //** Isolated PR development. Native observations connected to SD0 event hooks; all authority still **NONE**.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`; base `stage-director/weather-news-host-candidate-v0-20261025`.  
**RELATED //** Weather Station PR #804 is merged into `The-Post-Apollo-Project/main`. It declares `post-apollo-weather-station` layer-shell namespace and computes local NEWS player-bay coordinates.  
**AUTHORITY //** Operator controls merging, live activation and Hospital acceptance. No Hospital Room/Assignment is assumed.

## IMPLEMENTATION

A concrete connection between existing observers, not a separate controller:

- `stage_director_weather_host_observe_layer()` now runs through the existing SD0 layer map/commit event observer, against actual mapped native layer-shell state, namespace, output and scene position. It does **not** run during destroy callbacks, which can reference torn-down scene nodes.
- When a view is mapped or its identity changes, the existing Stage Director classifier marks only `scene=weather-news,role=browser` as a NEWS actor candidate. A lost identity, unmap or destroy removes the candidate.
- `stage_director_news_shadow_evaluate()` is a zero-dependency count policy. It distinguishes EMPTY, HOST_ONLY, ACTOR_ONLY, UNIQUE_CANDIDATES, DUPLICATE_HOSTS, DUPLICATE_ACTORS, AMBIGUOUS_BOTH and INVALID. Even one host + one actor yields only **UNIQUE_CANDIDATES** and never process ownership.
- The observer emits one `[stage-director][SD1][NEWS]` line when candidate **counts change**, reusing the current SD0 shadow log. Every emitted line explicitly says `owner_verified=no can_launch=no can_clip=no can_accept_input=no authority=NONE`. No additional poller, browser process, compositor clipping or focus action.
- Layer and actor teardown clear only these candidate flags and refresh shadow status. Existing SD0 scene registry and other profiles are preserved.

## HOW TO USE

After the parent SD0/SD1 development chain is integrated, build SwayPX in **an isolated compositor environment**, not the operator's live session. Map Weather Station and an independently created candidate NEWS browser to observe the host-only, actor-only, unique and duplicate transitions in `/tmp/swayfx-stage-director-shadow.log`. This file path is an existing SD0 facility, not a new writer. These logs are diagnostic, not a playback authorization signal.

For offline source verification, run `tests/stage_director/test_news_shadow.c` under the dedicated CI workflow and the no-authority source test `tests/stage_director/test_news_shadow_boundary.py`. Both have no browser, compositor or graphical dependency.

## VERIFY

The standalone tests cover absent candidates, one-sided discovery, unique candidate pairs, duplicate hosts and actors, large counts, unexpected negative counts and the hard `authority=NONE` invariant. Static code checks ensure SD0 observers invoke the native host adapter but cannot reach `stage_director_viewport_clip_view`, session reserve/attach, scene mutations or launch APIs.

**Not yet verified:** full SwayPX build/link, actual Wayland actor process lineage, a user WATCH session handshake, browser output containment, popup/input restrictions and the operator's running desktop. Those remain explicit gates.

## WATCH OUT FOR

1. Candidate classification is **not authentication**. A hostile browser can spoof `app_id` and a layer client can claim the namespace. A unique pair is not proof of common ownership.
2. This work is stacked on a development branch, not SwayPX `master`. Even when its PR is merged, the running compositor will not contain the change unless the complete SD0 chain is separately integrated, deployed and verified.
3. The existing Weather QML viewport is **chassis-local**. It is not a compositor IPC registration, and SwayPX cannot yet transform it into trusted global pixel geometry just from a host namespace.
4. The SD0 shadow file currently contains actor title information for other profiles. Preserve existing logging policy and do not treat it as permission to collect web URLs/credentials.
5. Do not bypass the missing launcher/session owner by setting all `stage_director_viewport_authority` booleans to true.
6. Host candidate count only changes when map/commit observation changes host eligibility, not when the UI swaps NEWS pages. It does not imply a viewer is open.

## CHECKLIST

- [x] Inspect current repository heads, current Weather UI and Stage Director dependency branches
- [x] Hook exact native host candidate into current layer observer
- [x] Hook exact NEWS actor classification/unmap into current actor observer
- [x] Handle duplicate/missing candidates and identity loss without activation
- [x] Add C tests and non-actuation static guard
- [ ] GitHub CI green
- [ ] Full SwayPX isolated build and nested compositor state validation
- [ ] Trusted launcher ↔ host/actor binding and NEWS local-to-global viewport protocol
- [ ] Popup/input/clip reapplication, rollback and real video acceptance
- [ ] Operator authorization before merging to master and live desktop deployment

## NEXT

The next substantial dependency is the trusted compositor-side launch/association contract. It must originate from an authenticated operator WATCH intent, select exactly the intended mapped Weather layer, spawn and verify one external browser process, associate its native Wayland/Xwayland view by proven process identity, and gate the clip with correctly enforced input/popup/teardown requirements. A diagnostic UNIQUE_CANDIDATES state must never be used as that permission.

## DECISIONS

Preserve SD0 observations and existing stage ownership. Make the first real observer connection observable through the current shadow log, without granting any compositor or playback authority. Keep Weather NEWS safe-mode until native containment passes isolated runtime acceptance.
