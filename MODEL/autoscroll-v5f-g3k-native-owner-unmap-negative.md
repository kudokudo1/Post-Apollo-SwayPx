# 🏥💜 V5F-G3K // Actual native target unmap and input guard refusal

PATIENT // Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed native safety guards  
STAFF ROLE // DOC · External Hospital-compatible; no verified live Hospital registration  
BASE // G3J #184 exact head `1fae6d0e747e82530f6ba72851b7867a2315a074`, independently Nurse 2 scoped native PASS #6024169310  
PRODUCTION // Autoscroll HARD OFF · TimeW Restore HOLD

## IMPLEMENTATION

The G3J native test proved a real kernel uinput middle-click reaches the **actual** SwayPX compositor wlroots/libinput backend while the source remains synthetic and denied. G3K extends the same real disposable compositor scene through a **target unmap** and **device removal** without authorizing any scroll.

In an isolated GitHub-hosted test, a known kernel-software mouse with deliberately false BUS_USB presents one native middle press/release to the actual SwayPX cursor callback while a directly mapped Foot XDG target exists. The G3K callback calls the **unchanged compositor-owned `pa_autoscroll_owner_snapshot_read`** and the **unchanged production `pa_autoscroll_guard_bridge_begin/tick/cancel`** for those real events. The read-only diagnostic records actual target/output owner epochs, source candidacy and bridge result.

Then the CI runner **terminates the real Foot process**, and waits until the old XDG window ID and PID are absent from native `get_tree`. The same still-registered Linux device generates a second middle press/release. The real native callback must now report `owner_snapshot_read == false`, zero target/output epochs and a denied guard begin/tick; normal middle-button dispatch must still occur on all four real events. This is not a caller-fabricated owner-revoked boolean.

Finally the kernel actor issues `UI_DEV_DESTROY`. CI queries SwayPX `get_inputs` and requires the synthetic device to have disappeared from its actual native input manager. Missing callbacks, stale owner acceptance, retained native input registration or skipped teardown FAIL the native test. No scroll axis/frame can be emitted.

**This is specifically a fail-closed target-unmap and device-removal test. It is not Scroll Lock attestation, complete layout-generation verification, keyboard-led acquisition or a positive genuine-mouse witness.** The bridge still holds origin UNKNOWN, native Scroll Lock UNKNOWN/DENY and layout-generation UNKNOWN/DENY even when real direct XDG owner epochs are present. Since source is synthetic the OS candidate is also denied. Native guard calls must never authorize or mutate user input.

## HOW TO USE

Review the exact-head PR/CI logs. Nothing is installed or run on the user's workstation. The only compositor code edits are applied to an ephemeral SwayPX **COPY** inside a GitHub-hosted runner, with a private disposable seatd, isolated XDG runtime, real software kernel input and temporary /dev/udmabuf ACL restoration. The actor runs as root ONLY in that disposable VM, gates its environment, emits exactly two middle press/release pairs with no axis or motion, and destroys its own test device. Private test files are cleaned.

## VERIFY

- Exactly eight additive MODEL/test/workflow paths relative to G3J, with zero modifications to checked-in production runtime/cursor/seat/Meson and no direct changed-file overlap with Company Map #126.
- First real G3K event pair has nonzero owner target/output snapshot for mapped XDG; second pair after actual window unmap has owner false and zero target/output epochs.
- Original compositor libinput callback and normal seatop each receive all 4 real events. No synthetic alternative logs, mock device, virtual Wayland replacement, scroll or input swallowing.
- Guard begin and tick are always refused on all 4 events; cancel clears the temporary stack-local guard. No native physical/protected positive evidence or UI activation.
- Kernel device destroyed and disappeared from native get_inputs, temporary runner ACL restored. Fail if unavailable or if any condition contradicts expected native source behavior.
- Exact-head GitHub CI source regressions and native build are separate; only both passing support a scoped technical conclusion. Independent Nurse 2 review remains required.

## WATCH OUT FOR

This test uses a real Linux OS-managed input path but its `uinput` origin is software-synthetic. A fake BUS_USB device is not a hardware-authenticated mouse. A real owner snapshot is a *lifetime observation*, not consent or physical hardware. A timer has no new authentic pointer event and cannot silently refresh a witness. Native Scroll Lock and output-layout generation are **UNKNOWN/DENY** because the project has not yet established independently validated current-state acquisition for them. This is **not Scroll Lock attestation**, nor a positive native safety authorization. G3K does NOT prove how a real hardware mouse should pass all final gates.

G3J, G3I, G3H, G3G remain open, stacked, independently scoped-reviewed drafts. Company Map #126 retains shared compositor input ownership and requires coordination before any parent merge. TimeW #182 and protected Restore/consent work remain separate.

## CHECKLIST

- [x] Verify Nurse 2 G3J #184 reviewed exact head and native scoped PASS.
- [x] Implement native read-only guard diagnostic and two-phase synthetic kernel actor.
- [ ] Exact-head real G3K compositor CI proves mapped target then unmap and source denial.
- [ ] Independent Nurse 2 exact-head G3K review.
- [ ] Native multi-keyboard Scroll Lock status, complete layout generation and Esc/cancel producer verification.
- [ ] Actual trusted OS-managed mouse admission, app scroll receipt, Company Map seam reconciliation and operator activation approval.

## NEXT

Run exact-head native CI. Repair any isolated disposable fixture issues without weakening source, owner, Scroll Lock or layout denial. After G3K scoped acceptance, target a separately reviewed **current native keyboard Scroll Lock and multi-output layout witness** milestone, still non-actuating.

## DECISIONS

Operator-selected OS-managed-first development remains. Enrolled-device identity optional later. G3K is a **read-only native negative-guard experiment** with actual target unmap and device removal, not a production mouse feature or positive approval. Autoscroll HARD OFF. TimeW Restore HOLD. No merge or workstation change.
