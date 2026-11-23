# 🏥💜 V5F-G3H // Native OS-managed pointer event observer, diagnostic only

PATIENT // kudokudo1/Post-Apollo-SwayPx  
ROOM // Universal Autoscroll · OS-managed input integration  
STAFF ROLE // DOC, external Hospital-compatible, live registration unverified  
BASE // G3G #180, head dc8f0c5482925f5ced8083bba2baafe2359a3c1a  
DECISION // Operator-approved development direction: OS-managed native mouse first; enrolled-device optional later, [Nurse 2 decision](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/177#issuecomment-7099016932)  
PRODUCTION // HARD OFF, no compositor production file changed, no workstation write

## IMPLEMENTATION

G3E observed a virtual Wayland middle-button event from a real disposable compositor, including real owner generations and a direct mapped XDG target. G3F defined a test-only OS-managed source diagnostic that never grants permission. G3G verified a real kernel **uinput** mouse that falsely advertised BUS_USB was seen by libinput but refused by the existing source topology filter.

G3H makes the next bounded bridge: a **test-only observer inserted in the real compositor cursor callback** of a disposable Nix SwayPX copy. Unlike a caller-forged unit-test structure, its inputs come from the running compositor event:

- Actual middle press/release and event timestamp from the native wlr_pointer_button_event.
- Actual wlr_input_device pointer matched against the compositor input manager's current device list; explicit virtual flag.
- The same OS/libinput source probe and origin classifier from checked-in production source.
- Native device/seat lifetime generations, current real four-owner snapshot (device, seat, directly hit XDG target, enabled layout-matched output).
- Stage Director Weather/Company first-refusal result from the existing native callback, not a proposed external IPC input.
- G3F OS-managed mode selected only in the **disposable test**. It is NOT a live operator setting, active session credential, positive attestation, or production grant.

The test-only G3F model and G3H observer are linked **only in the copied CI compositor**. No production source or normal Meson file links them. Since the test uses a virtual Wayland pointer, the expected native observation is virtual=1, source_candidate=0, owner_snapshot=0, and G3F OS-managed diagnostic DENIED on both press and release. Other real scene epochs may be nonzero, but they cannot override virtual input denial. The original G3E/G2 trace must independently prove both real middle-button transitions pass through **normal seatop handling**, with zero scrolling, no swallowed events, and device/seat revocation afterward.

No C code in G3H invokes wlr_seat_pointer_notify_axis, frame, guard begin, or runtime enable. `pa_g3f_may_consume_middle` and `pa_g3f_may_emit_axis` remain unconditional false even if source is an OS candidate. This is read-only event observability for OS-managed development, NOT operational mouse scrolling.

## HOW TO USE

Review the draft PR's exact-head GitHub CI logs. There is no UI toggle, service, new input device on the user's workstation, or local command. CI runs a copied SwayPX compositor under a temporary headless Wayland environment, launches a mapped Foot XDG window, and sends exactly one virtual middle press and release with a disposable Wayland client. Normal mouse dispatch remains intact. GitHub CI's temporary /dev/udmabuf ACL restoration is inherited from G3E and must succeed. The production checkout is not altered.

## VERIFY

- Seven new paths only: C observer, Python overlay, Python actual-scene trace, CI bash harness, Python source guards, MODEL and workflow. No existing runtime, cursor, input-manager, seat, Meson or Company/TimeW file modified.
- Original G3E native compositor proof still checks real four owner epochs, direct mapped XDG hit/layout, original virtual denial, real normal seatop path and source revocation.
- New actual compositor G3H logs must show two correct OS-managed model denials, not only static or synthetic model fields.
- Current source source-only regressions G3H/G3G/G3F/G3E and earlier guard checks. Compile real disposable Nix SwayPX with injected C observer.
- Verify original production `PA_AUTOSCROLL_LIVE_ACTIVATION 0`, `physical_verified=false`, `native_scroll_lock_active=true`, bridge origin UNKNOWN/layout unverified.
- Exact-head 2/2 GitHub CI + independent Nurse 2 scoped review before integration.

## WATCH OUT FOR

This is native compositor observation, but the test input is intentionally **virtual**, not a positive physical mouse or a real libinput device in the compositor. G3G's separate kernel libinput negative test is not a substitute for a compositor libinput callback. The OS-managed decision allows development under the normal OS trust model, it does not imply physical hardware authenticity. Local privileged synthetic HID risk remains. The G3F model's source fields are not sufficient for production authorization, and G3H's `true` test-only lane selection is not a user-approved runtime toggle.

Positive native OS-managed admission still needs an operator-approved trusted configuration switch, native per-event source and scene continuity, fresh Scroll Lock policy, output-layout generation, cancellation, check-before-axis and an independent signed acceptance boundary as appropriate. No event consumption or scroll frames can occur from this milestone. No real XDG app event-receipt proof is claimed. Company Map #126 owns shared cursor/seat production seams, and TimeW #178/#179 is separately owned.

## CHECKLIST

- [x] Operator's OS-managed-first decision verified and retained.
- [x] Native test-only callback observer and G3F fail-closed model bridge authored.
- [ ] Exact-head CI confirms real native callback press/release and deny-only trace.
- [ ] Independent Nurse 2 review of G3H exact source head.
- [ ] OS-managed positive native source candidate proof in a real wlroots input backend, without hardware-attestation claims.
- [ ] Live guard integration and atomic layout/Scroll Lock/cancellation verified on disposable compositor.
- [ ] Company Map coordination and explicit operator production/merge approval.

## NEXT

Verify the native CI, request scoped Nurse 2 review, and then focus on a real OS-managed libinput callback as a **candidate-only** independent native milestone. Keep production HARD OFF.

## DECISIONS

Accepted development priority: **OS-managed first**, enrolled-device optional later. **Not accepted:** a source candidate as hardware proof, enabled scrolling, compositor production edits, merge, local installation or live Hospital registration. Autoscroll HARD OFF; TimeW Restore HOLD.
