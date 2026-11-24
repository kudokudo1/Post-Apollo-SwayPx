# G4D1 // Per-seat Scroll Lock and Escape control contract

PATIENT // Post-Apollo-SwayPx
ROOM // Universal Autoscroll
ROLE // successor DOC; live Hospital registration unverified
BASE // G4C2 #207 2e1aba8ca30997bd16ab1deefddc03a66c23d582; 3/3 CI PASS
PRODUCTION // HARD OFF

## IMPLEMENTATION

Add a detached private seat-local control state machine using G4C1 session cancellation. Feature mode is distinct from arming. Default OFF/disarmed; selecting OS-managed leaves disarmed. A fresh managed key press toggles armed/disarmed; release only clears its keyboard-local held state, repeat never toggles, duplicate held press never toggles. Escape cancels the current session without changing mode or armed preference; any unknown or unmatched key input can request disable/cancel but cannot arm. Keyboard retirement and topology invalidation disarm and cancel. Mode changes cancel and disarm. Policy generation advances on changes and cancellation; sequence or generation exhaustion terminally denies. Exact seat identity required; no global state, no trusted physical claim. Never consumes native key or writes XKB LEDs.

## VERIFY

Portable C11 fixture links the actual G4C1 contract and checks mode, two keyboard slots, repeat, held press, unknown source, cross-seat, retirement/replacement, six positive fixture sessions followed by cancellation, replay and exhaustion. CI pending. No existing production files changed. No native keyboard epoch issuer, trusted configuration producer, real debounce callbacks, key delivery, or actual scrolling yet. G4D1 deliberately separates per-keyboard held state from per-seat controls; native owner must allocate and retire slots with actual lifetime, and provide monotonic event sequence; cannot take fixture structs as credentials. Independent G4A through G4C2 reviews and #126 coordination remain outstanding.

## DECISIONS / NEXT

Proposed keyboard semantics for initial implementation: OS-managed selection is explicit; arm via fresh managed Scroll Lock press, untrusted sources disable-only, Escape preserves arm state. No persistence beyond live seat lifetime until approved. Test-only positive fixtures are not live input grants. Next native key observation and control bridge in disposable CI, then actual native scene continuity and target delivery. No merge, workstation modification or enablement.
