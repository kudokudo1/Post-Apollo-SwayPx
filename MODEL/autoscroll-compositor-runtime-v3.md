# SwayPX // Autoscroll V3 · Continuous Axis Dispatch

> **SOURCE ONLY / DEFAULT OFF / NOT COMPILED INTO THE RUNNING COMPOSITOR.**
>
> This is a dependent SwayPX autoscroll draft after PR #139 (native rate core) and PR #140 (physical button/motion/ESC adapter). It does not modify input event handlers, the Meson build, a user desktop, or the active Wayland seat.

## Hospital continuity

- PATIENT // `kudokudo1/Post-Apollo-SwayPx`
- ROOM // Native Universal Autoscroll, source-only event-dispatch V3
- STAFF ROLE // External Hospital-compatible DOC; no live registration verified
- ASSIGNMENT // Operator requested continuation toward real middle-click autoscroll. Preserve shared pointer input handlers until their owning Room accepts an integration plan
- BASE // PR #140 exact head `698d4258a4febaf22f3ad46bc38f947fda5909c1`

## Verified ownership collision

Open [SwayPX PR #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) is a Company native MapLibre Stage Director workstream that modifies **actual** cursor motion, wheel, button, popup and scene focus paths. This autoscroll Room cannot quietly absorb or overwrite those changes. The new V3 code therefore touches only namespaced autoscroll source, tests, workflow and model notes. No modification to `sway/input/cursor.c`, `sway/input/seatop_default.c`, `sway/input/seat.c`, or `sway/meson.build`.

## Implementation

The standalone `post_apollo_autoscroll_axis_dispatch.{h,c}` wraps the V2 middle-click adapter and the V1 continuous-speed engine. It accepts hypothetical **compositor-authenticated** physical input events and emits **wheel event intentions** through two injected callbacks, `emit_axis` and `emit_frame`.

- After initialization the mode is **OFF**. Until a trusted compositor owner adds an explicit safe activation path, normal mouse middle click and movement are untouched.
- For the active mode, middle click sets the anchor, and subsequent relative physical motion adjusts speed and direction without moving the anchor.
- A periodic tick produces up to two packets, vertical first and then horizontal, with pixel displacement, discrete steps zero, and the exact target node identity.
- An accepted nonzero tick gets **one frame**, even when it emits both axes. A dead-zone tick emits no axes and no frame.
- If an output callback is missing, rejects delivery, or detects nonfinite data, the session cancels. A rejected emission does **not** claim a successful frame. A partially delivered pair cannot be undone; the session fails closed and does not send further ticks.
- A second middle-click, same-seat Escape, a different physical button, session lock, Sway Scroll Lock collision, missing target, changed layout generation, foreign mouse/seat or expired tick cancels or refuses.

The data structure deliberately does **not** claim real wlroots `wlr_seat_pointer_notify_axis` usage. The future SwayPX integration owner must determine the correct wheel source, discrete vs pixel axis semantics, event timestamps and target delivery rules on a headless compositor. This V3 code cannot receive actual physical input, inject events or display the Windows-style anchor indicator.

## Tests

The dedicated GitHub workflow compiles three isolated C units with `-Wall -Wextra -Werror -pedantic` and runs tests for:
- Default-off behavior
- Middle-click → motion → repeated vertical scrolling while stationary
- Direction reversal, diagonal scrolling, and one frame per tick
- Dead zone, no empty frames, exact target
- Failing emission and missing sink abort
- Wrong target, changed layout, wrong device, lock collision
- Escape, middle-click, other button, and paired releases

The workflow diff guard forbids touching the shared SwayPX runtime input paths. A passing synthetic test is **not evidence** of live Brave/Kitty/Zellij behavior.

## NEXT // Shared compositor hook ownership and actual runtime verification

After V3 passes CI, the compositor owning Room should accept or reject a tightly bounded default-off integration, then:
1. Wire the true compositor physical middle-click and scene target at event time, without changing Stage Director's input pairing.
2. Hold the pointer position at its anchor, accumulate compositor relative physical movement and dispatch timed axis events through the legitimate seat operation.
3. Bind Escape/other clicks, session lock, monitor changes and device disappearance to cancellation.
4. Provide an input-transparent anchor indicator matching the compositor output and verify on real headless SwayPX with a physical-mouse simulation.
5. Only after independent security review, operator-approved merge and local deployment, expose a usable CPU++ Universal Autoscroll toggle.

**Rollback:** Close V3 and parent draft PRs. No running compositor/host state changes exist to undo.
