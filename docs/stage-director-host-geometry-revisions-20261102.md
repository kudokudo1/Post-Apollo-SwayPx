# Stage Director SD1 // Native Weather host geometry revisions

STATE // Candidate-only implementation, pending GitHub CI. Not production NEWS playback or operator acceptance.

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Stage Director / Weather Station NEWS viewer
BASE // `master` `a74f82760814f83e3220a3ff2286e510b134be89`
WORKER // External Hospital-compatible ChatGPT doctor; no verified live Hospital attachment.

## IMPLEMENTATION

The compositor already listens to actual Weather layer-shell map/commit/unmap/destroy events. After the existing native scene-hit pairing selects the exact mapped Wayland client/layer/output/epoch, the compositor records validated **outer Weather Station layer** coordinates, plus output coordinates, in that same native session. Every changed rectangle increments a monotonically increasing geometry revision; identical commits do not. Snapshot reads require exact current native host/client/output/epoch. A prior revision is stale.

On CLOSE, host unmap, resource/client destruction or native peer/output identity loss, the geometry snapshot is invalidated with the native WATCH candidate. The revision floor survives revocation and no wraparound is allowed.

**Important scope:** this is NOT the inner NEWS viewport rectangle, and geometry evidence is not browser authorization. There are no public IPC/QML setters. Normal SwayPX still refuses production browser spawn, clip, focus, keyboard/pointer dispatch and popup authority; no local Fedora or Quickshell changes.

## HOW TO USE

Read the PR's changed files and GitHub native host-session CI. No desktop action is available or required.

## VERIFY

- Exact native client, host, output, epoch identity and bounded host/output geometry.
- Same geometry is idempotent; changed geometry advances revision.
- Wrong native peer/output/epoch, outside-output or invalid host bounds are rejected.
- Old revision cannot be reused after movement, CLOSE, unmap, new epoch or native client destruction.
- Native Wayland two-client lifecycle regression and independent full SwayPX build.
- No production launch or CI-only selector enabled.

## WATCH OUT FOR

Outer-layer geometry alone cannot tell Stage Director where NEWS is rendered **inside** Weather Station. That geometry must be coordinated with the Weather Station owner and independently validated. Even a current revision is candidate-only evidence, never a permission or protection boundary for browser pixels, popups or input.

## CHECKLIST

- [x] Inspect Syntagma, current master HEAD, active PRs and native observer
- [x] Reuse existing native host session and compositor map/commit events
- [x] Record revisioned bounded geometry with lifecycle revocation
- [x] Extend native host and two-client regression tests
- [ ] GitHub CI and full production build verified
- [ ] Coordinate authenticated interior NEWS viewport contract with Weather worker
- [ ] Repeatable isolated real-browser viewport updates, popup/input/focus guards
- [ ] Operator desktop testing and final acceptance

## NEXT

After CI, use this candidate geometry as one input to a separately authenticated NEWS interior viewport controller. Never derive the interior rectangle by guessing margins from the outer chassis. Continue with repeated real Chrome mask screenshots and abnormal exit/revocation tests before any production activation.

## DECISIONS

Keep the #53 consolidation as the base; preserve #51 excluded. No broad PR queue merge, no active browser launcher, no changes to another worker's Weather Station QML, and no claim of Hospital registration.
