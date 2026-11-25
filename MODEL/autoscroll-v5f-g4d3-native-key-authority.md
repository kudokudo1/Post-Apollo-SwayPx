# G4D3 // Native keyboard source eligibility and lifetime custody

PATIENT // Post-Apollo-SwayPx
ROOM // Universal Autoscroll
ROLE // successor implementation DOC; live registration unverified
BASE // G4D2 #209 a5c4ac46bb1e76e0e4af67b2c4525c66ec1aa7a5; 3/3 CI PASS
PRODUCTION // HARD OFF

## IMPLEMENTATION

Add an unlinked native keyboard observation producer. Bind each actual keyboard object during configure to a seat-local non-reused keyboard epoch; seed held bits from wlroots pressed keycodes; do not clear held bits on reconfigure. Require live seat membership, exact registered device, actual keyboard attachment, nonaggregate native callback and real libinput handle; read udev topology, reject obvious synthetic paths, protocol virtual and locked session for managed-source eligibility. Freshness uses actual bounded monotonic event timestamp and per-key high-water mark. Stale releases must not clear a held state; unknown press is disable-only. Retire binding on destruction. No physical attestation. Native receipt is source eligibility, not a permission to consume or arm across Company/shortcut/grab boundaries. No production configuration or hook changes.

## VERIFY

Disposable overlay binds the actual producer at real device configure and handle_keyboard_key, not aggregate group dispatch; independent existing G4D2 shadow test unchanged. Test eight native callback receipts against separate G4D2 receipt data, distinct keyboard epochs, retirement, synthetic refusal, default OFF, no axes. New source compiled only in disposable compositor. CI pending. A positive native nonvirtual keyboard cannot be verified by the synthetic test; ordinary OS trust assumption approved by operator. Actual selected-mode producer, session lock transitions, key binding/shortcut/grab/Company clearance, topology invalidation and live control integration remain outstanding. Caller must ignore stale release, cancel on producer failure and retire controls before destruction. Native positive arming remains gated.

## NEXT

Real stationary scene/layout revision producer, native control integration, positive native client delivery in disposable explicit test lane; independent review and Company #126 ownership coordination remain unresolved. Preserve #204–209 exact heads and HARD OFF.
