# 🏥💜 G4C1 // OS-managed admission and stationary continuity

PATIENT // Post-Apollo-SwayPx  
ROOM // Universal Autoscroll  
ROLE // successor DOC; live Hospital registration unverified  
BASE // G4B #205 1a1c6823fc522666133db3fc08d5e23e7149ba98  
PRODUCTION // HARD OFF

## DECISIONS

Operator accepted OS-managed initial mode in conversation on November 24, 2026, responding “That is fine. That works” to the explicit proposal and residual non-attestation warning. This approves the OS trust model for development, NOT enabling production, installing, merging, or selecting final keyboard semantics. Enrolled-device mode remains unavailable.

## IMPLEMENTATION

Introduce a distinct, detached policy-scoped eligibility session, without reusing PHYSICAL_LIBINPUT or a trusted_physical_pointer flag. Require explicit OS-managed selection, armed per-seat lock, real-owner source/scene facts, monotonic non-replayed event and scene sequences, exact seat/target/device/output/layout/policy binding. Preserve sequence high-water marks on cancel; fresh event required to restart. Separate stationary tick scene reads from original event timestamp. Bound tick gaps to 100 ms (consistent with existing engine), scene age 48 ms and total session lifetime 180 seconds. A second start while active cancels instead of rearming. No native input changes, no actual axes, no Meson linking, no UI/IPC, no claims of physical attestation.

## VERIFY

Compile strict C11 fixture. Positive fixture eligibility and stationary continuity with 100 fresh scene samples; denied native-source/mode/lock/ownership/freshness facts and replay; revocation and no automatic restart. All values in the fixture are invented. The module is not a credential issuer; real compositor custody and distinct non-reused sequence generation remain unimplemented, as do actual per-seat toggle, source recheck on ticks, lifetime-bound release pairing and target delivery. Approval of G4A and shared Company owner integration remain outstanding. G4C1 source paths are new private files only.

## NEXT

Native read-only producer and review of custody, seat-local sequence and scene epochs. Actual positive delivery and input consumption gated on owner reconciliation, native client receipt and separately authorized desktop trial. No merge or workstation modification.
