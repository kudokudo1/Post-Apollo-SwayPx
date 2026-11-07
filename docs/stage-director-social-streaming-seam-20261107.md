# Stage Director // Social W concurrent compositor seam sampling

**STATE // EXPERIMENTAL CI-ONLY REAL SCENE CAPTURE. NOT every presented frame and NOT production-activated.**

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`, Social W integration depends on `kudokudo1/The-Post-Apollo-Project`.  
**ROOM //** Social W external-guest grouping and no-visible-drag/resize-separation task. Hospital-compatible browser session only; no live Room, assigned Staff Role, or production permissions verified.  
**BASE //** SwayPX draft PR #101, the eight-snapshot in-flight visual seam test, stacked on draft #99 native move+resize geometry.  
**OPERATOR GOAL //** The actual chassis and external app appear visually joined during movement and resize initiated on either side.

## IMPLEMENTATION

PR #101 demonstrated why checking only final rectangle geometry was insufficient: when chassis-led native move began, Sway raised the opaque chassis above the guest, hiding it even though their computed rectangles were aligned. The corrected **disposable CI source overlay** now raises the verified guest above the host before the same compositor transaction. Eight *paused* screenshots then passed 38 seam samples each.

This milestone adds one separate `StreamingSeamWitness` class in `tests/stage_director/`. While a disposable native SwayPX session runs its actual move/resize seatop, the main test dispatches 10-12 bounded synthetic cursor steps in each of four cases: chassis drag, guest drag, chassis resize, guest resize. **A separate concurrent thread continuously requests real grim PPM screenshots during the seatop.** It independently reads the native Sway tree just before and after each screenshot; a frame whose window rectangle changes across the screenshot capture is counted as an *unscorable transition*, not a pass.

When both reads agree, it demands exact current mapped ID/PID and floating identity, full chassis/guest geometry relationship, unchanged unrelated window, and the same 38 rendered seam, border, rail, guest, unrelated and background RGB checks as the previous test. A mismatch fails with actual RGB, point and stage.

Each of the four runs demands at least four stable screenshots across at least three distinct observed window positions and prints frame counts, discarded transitions and screenshot durations. The CI screenshot stream is concurrent with pointer updates, unlike the previous synchronous per-step screenshot proof.

## VERIFY

Run the existing disposable **Stage Director Social W native seatop move overlay** GitHub Actions workflow on this draft. It builds a temporary source-overlay SwayPX compositor; no checked-in production compositor code, input drivers, Quickshell processes or workstation files are changed.

**Exact-head CI MUST pass** the original native bidirectional move, paced resize/ACK, eight in-flight screenshots, stale guest rejection, temporary `/dev/udmabuf` ACL rollback **and** all four new concurrent screenshot streams. The new test does not weaken original exact geometry or pixel assertions.

**Limits:** Grim screenshot sampling is not an exhaustive compositor presentation-frame callback trace. Stable reads before/after a screenshot reduce but do not abolish temporal attribution ambiguity; a frame can repaint between those reads. The CI uses software rendering, synthetic native Wayland terminal actors and a test-only seat cursor command rather than physical mouse input or actual Vesktop+Quickshell. Passing is stronger streaming evidence, **not proof of zero perceptible lag under every physical drag/resize frame**.

## WATCH OUT FOR

The test-only native pairing uses exact CI fixture IDs and PIDs with a private lease file. It is NOT production authority. Active Company/Weather Stage Director owners retain shared compositor code. No merge to a production branch, no installed compositor build, and no local configuration change are authorized by this proof.

If concurrent screenshots expose compositor/render timing defects, preserve the failing frame metadata and exact reproduction. Do not silently add tolerances or turn unstable frames into successes; report a blocker if source-level staging cannot guarantee the seam.

## NEXT

After green CI, require a compositor-owner-authored **presentation-frame callback / scene-trace witness**, authenticated pair leases, physical input path, popup/input clipping, multi-output scaling, and actual Quickshell/Vesktop rendering under a reviewable opt-in feature gate. Operator final acceptance remains separate.

## DECISIONS

The existing Social W fast-follow bridge and all Settings++/rail dimensions remain untouched. PR #99 and #101 are independent reviewable drafts. This is only a new stacked CI observation milestone with reversible rollback by dropping this draft.
