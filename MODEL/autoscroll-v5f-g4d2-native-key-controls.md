# G4D2 // Native keyboard callback control integration

PATIENT // Post-Apollo-SwayPx
ROOM // Universal Autoscroll
STAFF // successor DOC; live Hospital registration unverified
BASE // G4D1 #208 deb7e4325290d98656858078c0f330aeca5b5139; 3/3 exact-head CI PASS
PRODUCTION // HARD OFF

## DECISIONS

Operator confirmed in conversation November 24 that Scroll Lock manages enable/disable; Escape stops current scrolling without changing mode or enable preference. Continue development authorized, no merge or live activation.

## IMPLEMENTATION

Bind actual G4D1 control logic and G4C1 session cancellation to device-specific native keyboard callbacks in the disposable compositor copy only. Fixture keyboard slots live inside real keyboard objects, seat-local control and shadow slots inside real seat objects, with retirement before destruction. No aggregate callback may toggle; no native key is consumed. Two private positive fixture sessions use invented owner IDs and explicit fabricated setup, never actual native synthetic keys as positive permission. Actual native synthetic Scroll Lock disables/cancels first shadow, two more native Scroll Lock presses cannot arm, native Escape cancels second shadow while preserving armed preference and selected OS-managed mode. Confirm 8 key callbacks, two keyboard owners, retirement, normal inherited 24 middle callbacks, no real axes. The test uses per-seat keyboard identity minted on first relevant callback, not a final production epoch issuer. Positive fixture arming is distinct from actual native source trust.

## VERIFY / WATCH OUT FOR

CI pending. Native keyboard eligibility for positive OS-managed arming, real debounce/grab policy, complete layout/scene issuer, key receipt at focused client, real axis delivery and Company #126 combined containment remain outstanding. No existing production source changes, no workstation changes. G4A through G4D1 independent reviews remain pending. Do not infer live activation or actual native positive arming from shadow fixtures.

## NEXT

Verify exact-head CI and request independent review; develop real keyboard ownership/positive admission and stationary scene revalidation under agreed owner boundaries before actual delivery. Preserve default OFF. 
