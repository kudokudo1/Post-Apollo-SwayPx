# Stage Director // Social W grouped geometry preflight

**STATE // CI-only geometry proof; no compositor integration, frame synchronization or live desktop change**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`. Dependency: `kudokudo1/The-Post-Apollo-Project`, merged Social external-activation PR #1427.  
**ROOM //** Social W compositor group seam, external Hospital-compatible browser assignment only; no registered live Room/role verified.  
**SCOPE //** New test-only files; no edits to production SwayPX C, Meson, existing Company/Weather Stage Director tests, Quickshell, or local Sway.

## IMPLEMENTATION
The operator requires Social W's chassis and Vesktop (then other supported apps) to drag/resize as one perceived window with no visible trailing. Existing Social W `DiscordGeometry.py` polls Sway at 8 ms when open, follows on subsequent IPC commands, and does not produce an atomic compositor frame.

`ci_social_group_geometry` is a narrow arithmetic/ownership **candidate** for both directions of pointer moves. It requires a preexisting, current epoch with exact verified host/guest/output references, mapped main client, confirmed initial dock, healthy output and one explicitly selected leader. It projects the chassis+guest rectangles using the rail width/gap/inset, checks integer bounds, minimum shell size and output containment, and returns *one geometry-only plan* or a fail-closed state. The plan cannot mutate focus, input, launch state, scene graph or windows. Identity values in synthetic tests are **not** real native authority and must never be wired directly from app ID or title.

## VERIFY
An isolated C test uses the existing `stage_director_viewport.c` rectangle predicates and verifies chassis-led move/resize, guest-led move/resize, negative-origin monitor, stale epoch/refs, missing permission/mapping, splash/undocked refusal, minimum size, integer overflow and output violation. The dedicated workflow compiles with `-Werror` and does not build or activate SwayPX.

**Not verified:** Any runtime frame atomicity, renderer behavior, cursor input, real Vesktop process or native ownership binding. Passing this test cannot demonstrate lag removal.

## WATCH OUT FOR
Active Company Stage Director M30 and Weather/Chrome work own shared compositor rendering paths. This source does not take over those paths. A future native compositor owner must provide **authenticated** per-session host/guest binding, scene-frame update, correct hit testing/clipping/popups, abort/revoke and native pixel-frame tests before declaring the user requirement met. Do not merge this pure plan into production integration automatically.

## NEXT
The authorized SwayPX Stage Director owner uses this disposable geometry policy as a reference when implementing a compositor **same-frame host+guest transform** behind an explicit CI-only feature gate. Capture real screenshots/scene-frame evidence for two-way drag and resize before proposing desktop activation. The older bridge remains rollback.

## DECISIONS
No global component changes; no false claim of compositor actuation. Preserve Social W canonical shell size, rail, Settings++ dimensions and existing Dock initiation. Stage Director ownership must be coordinated before modifying shared transaction/scene paths.
