# Stage Director // Social W in-flight compositor seam witness

**STATE // CI-only, actual pixel-sampling between native seatop steps. NOT continuous frame proof or production grouping.**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`, with Social W chassis/application lifecycle owned by `kudokudo1/The-Post-Apollo-Project`.  
**DEPENDENCY //** Stacked after SwayPX Social W native resize proof PR #99; native move proof #98 and real Wayland baseline #95 have merged into their branch chain.  
**ROOM //** Social W chassis + guest no-visible-lag task, external Hospital-compatible session; no verified live Room/Staff role.  
**OPERATOR GOAL //** Moving or resizing from either side must look like a single unified window, not two independently catching up.

## IMPLEMENTATION

The preceding disposable headless SwayPX proof verified exact final window rectangles and screenshot pixels **after** native move/resize completed. That leaves a compositing gap: the native seatop can raise the leader above its guest, expose a one-pixel seam, or emit a transient split while subsequent client configures are pending.

This milestone does **not** change the native compositor overlay or production C. It extends only the test harness with a deliberately stricter witness that pauses **inside** the test-driven native move or resize operation, after each controlled pointer step and current real Wayland client geometry ACK, to capture the composited output with `grim -t ppm`.

For each of the two intermediate steps in each leader direction (host move, guest move, host resize, guest resize), the witness asserts exact mapped Sway rectangles and checks 38+ pixel samples across:

- Social rail, including the last two pixels immediately before the guest begins
- guest left interior and right interior
- the chassis's one-pixel right, top and bottom margins
- the unrelated third XDG client and separate black output background

Every mismatch fails with the actual RGB value, exact pixel location, intended color, scenario and step. No dimension tolerances are relaxed.

## VERIFY

The dedicated [native move/resize disposable CI workflow](../.github/workflows/stage-director-social-native-move.yml) executes on the current stacked PR and must pass on its **exact head**. This is a real software SwayPX scene and real XDG clients, with the compositor operation still active at screenshot time. It is NOT a capture of all frame presentation callbacks, not physical human pointer input, not Quickshell/Vesktop or content redraw timing. Even a passing run cannot certify perceptual zero-lag during continuous interactive motion.

## WATCH OUT FOR

The first in-flight CI run **failed at host-drag step 1**. Expected guest green RGB `(18, 170, 135)` at the application boundary, but the compositor actually rendered the opaque chassis purple `(48, 20, 102)`. This verifies a real layering defect in the CI fixture: native `container_raise_floating(host)` can cover the guest despite correct paired geometry. The disposable compositor overlay now puts the verified guest above the opaque chassis **when a host-led move or resize starts**, before the existing native transaction commit; guest-led operations must not repeat that correction. The same in-flight screenshot checks remain strict and the CI logs assert precisely one host-led stacking correction per operation. Production SwayPX is untouched.

The current native pair lease uses CI fixture IDs/PIDs and is not production authority. The real Stage Director owner must validate group surface identities/generation, input, focus, popup clipping, cross-output bounds and native frame scheduling before shared compositor edits.

## NEXT

Treat all source/CI failures as useful evidence. If the seam is exposed during native moves, add a **test-only paired stacking correction at seatop start before commit** with a dedicated negative control, then rerun the same in-flight pixel checks. Once reliable, use the evidence for a separate owner-reviewed, feature-gated production candidate. Keep Social W's current geometry bridge as fallback.

## DECISIONS

No PR merge, real machine mutation, global SwayPX configuration, or owner transfer. Existing Social menu layout/colors/Settings++ are unchanged. Frame-perfect drag/resize remains a target, not a claimed result.
