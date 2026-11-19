# V5E // Explicit Split Between Autoscroll and Paused Company Map

**Operator authorization:** November 19, 2026. The operator confirmed that the Company Map worker is paused and authorized a *narrow* shared-cursor change, requiring an explicit split and a way for the original owner to discover the change.

**Patient:** kudokudo1/Post-Apollo-SwayPx
**Autoscroll Room:** Mouse & Keyboard / Universal Autoscroll
**Other Room:** Company native MapLibre in-widget scene, draft PR #126
**Staff:** External Hospital-compatible DOC. Live Hospital attachment and full Company Map ownership are NOT asserted.

## V5E-A (this PR): policy only, zero shared files

- Distinguish unsupported/virtual/unknown input from a **libinput backend candidate**.
- Reject unknown native Scroll Lock method and any libinput button-scroll method.
- Do **not** equate libinput backend with physical authentication. Kernel uinput can also enter libinput; no certificate/udev/device-lifetime verification exists here.
- Tests compile the pure policy with strict warnings.
- No compositor source/handler integration, no Meson changes and no production activation.

## V5E-B: separately reviewable shared-input integration

- Its own branch/PR will add a read-only native wlroots/libinput probe, pass the result via an explicit bounded internal runtime argument, and modify only the existing autoscroll dispatch line in cursor.c AFTER Weather and Company pairing.
- The probe runs only if an autoscroll runtime exists. Currently it is NULL because production compile-time activation is hard OFF.
- Native Scroll Lock state must not be guessed. A missing or button-scroll libinput method is denial, not permission.
- Source classification is **non-authorizing** until separate genuine hardware, lifecycle, target, output and operator-consent gates are verified.
- It must not modify Company scene policy, widget code, seat.c, or the Map branch itself.

## Company owner notification / recovery

When V5E-B is open, post an explicit cross-reference and current diff/ordering on **Company Map PR #126**. Include its exact head, changed files, conflict expectations, hard-OFF guarantee and rollback. The Company owner may then inspect the change before rebasing/resuming. A GitHub PR comment is a durable handoff, **not proof of live worker delivery or acceptance**.

## Decisions

- Shared handler exception is restricted to the autoscroll source probe at the existing cursor seam. No general takeover.
- Both PRs remain draft/unmerged pending reviews and user approval.
- No local or production desktop changes.
- Close either draft to abandon; no running state to roll back.
