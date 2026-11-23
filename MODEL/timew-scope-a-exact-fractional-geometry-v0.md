# SwayPX / TimeW Scope A // exact fractional logical geometry candidate v0

**STATE //** SwayPX owner **REVISE** proposal with CI-only value reference implementation; **not** an accepted source/consent/wire protocol or production geometry command.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx` · **ROOM //** TimeW Scope A native window-lifetime and geometry  
**STAFF ROLE //** External Hospital-compatible SwayPX DOC owner, live Hospital registration unverified  
**SOURCE BASE //** Real mapped XDG runtime [draft #172](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/172), head `88ac1615af19c12d66d9fbe7951a5f77fdb1f20f`, stacked on Nurse-2-reviewed #170/#169/#168.  
**TIMEW OWNER //** [Merged TimeW consumer proposal #2117](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2117); owner reply requested on [cross-owner issue #2067](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067)  
**DELIVERY //** Isolated reference C data/value API and tests. Not linked into production SwayPX; no actual operator selected-surface authority.

## 1. Why the integer-only coordinate premise fails

The disposable **real mapped XDG** test in #172 observed a **floating logical pending rectangle**:

```text
x=259, y=92.5, width=1402, height=872
```

Earlier #170 bridges the double-precision compositor state into `int32_t` **only if all four values are exact integers**. This correctly **refuses** y=92.5; the runtime test uses an explicitly synthetic 1×1 rectangle for **native event-listener correlation only**, not as TimeW geometry evidence. Silently truncating y to 92 would move/compare the wrong geometry.

SwayPX currently keeps floating container `pending` and `current` geometry in C `double`, and centered floating resize/mapping can introduce half-unit positions. Even before a Restore, an actual window may have fractional `x/y`. The correct next step is a lossless candidate data model, **not enabling the old int32 ledger**.

## 2. Proposed SwayPX value boundary (reference C v0, NOT approved wire schema)

The new `include/sway/timew_scope_a_exact_geometry.h` and `sway/timew_scope_a_exact_geometry.c` implement:

- Exact **IEEE-754 binary64 values** for x/y/w/h, copied with `memcpy` to/from four 64-bit integer bit-pattern fields. No cast via `float` or `int32_t`, no `printf` decimal roundtrip. A future cross-process encoding would need an **explicit versioned, canonical lossless representation**. Neither byte order nor JSON hexadecimal field names have been approved yet.
- An explicit source *phase label* `PENDING` vs `CURRENT`, which is a diagnostic type discriminator **not evidence** that a frame/configure transaction settled. Before/after/final states require separately attested compositor transaction and native live-object proof.
- Fail-closed finite and logical-layout coordinate constraints: x/y bounded by signed-32-bit layout coordinate limits (fractions allowed); width/height at least 1 and at most `INT32_MAX` logical unit; reject NaN, ±infinity, negative/zero/subunit dimensions, out-of-range values, invalid phase/version/bits.
- Canonical **+0** for coordinate zero (negative zero normalized on input and rejected if raw forged). No buffer writes if a capture/read fails. Equality requires matching version, phase and **all four exact bit patterns**, never just rounded values.
- An explicit **no-authority boundary**: these are value snapshots only. They do not authenticate compositor release, operator gesture, seat/session, surface lifetime, C3e/C4b, C5 one-use spend, native command, target, physical pixel position or protected receipt.

**Important:** Native `pending` is what the compositor proposes to arrange, not proof of current committed rectangle or actual settled rendered geometry. `CURRENT` only denotes that a trusted future caller read the native current field, not that any independent source/consent check succeeded. The protected operation must bind actual object lifetime/issuer release/transaction epoch/seat/output/workspace, plus separately authenticated C3e/C4b and spent C5 operation.

A future signed/checkpoint-bound consumer needs a lossless *four-coordinate* before/target and an explicitly observed actual **post-resize** rectangle. Resize can modify x/y while centering. Move must use that fresh protected post-resize observation, not the saved pre-resize x/y. If native target cannot be represented or the expected exact state cannot be proven, **UNKNOWN/HOLD** with no automatic retry or undo.

## 3. Source-only test coverage

`tests/timew/test_timew_scope_a_exact_geometry.c` directly exercises:

- Real observed `259,92.5 1402×872`, all four binary64 bit patterns unchanged after roundtrip.
- 200 deterministic fractional samples and single-ULP mismatch (rounding is not tolerated).
- `PENDING` and `CURRENT` snapshots with identical numbers are **not** identical stage evidence.
- Finite range edges, nonfinite values, fractional dimensions, zero/subunit/negative sizes, stale/forged version/phase/NaN/infinite/negative-zero bits, atomic failure without changing outputs.
- Strict optimized and UBSan C builds on an isolated GitHub runner. Production `sway/meson.build` is untouched.

On top of #172, the **disposable only** real-XDG probe now captures/roundtrips the actual native `pending` rectangle through the binary64 reference, emits `TIMEW_CI_REAL_XDG_EXACT_GEOMETRY` (pending and `protected_geometry=0`), then still uses the synthetic 1×1 **lifetime-only** ledger when old int32 representation refuses the half-unit. Its CI enforces **two real exact snapshots and two actual native map/unmap revocations**. No active View/Seat/Move/Resize code is changed.

## 4. Unresolved cross-owner acceptance (must remain separate)

| Owner | Required decision | Current position |
| --- | --- | --- |
| **SwayPX** | Which exact native geometry state and numeric value is eligible, when an operation's pre/post rectangle is observed; future protected emitter and receipt need a versioned state/coordinate representation | **REVISE**. Binary64 value reference only; event+data CI may advance, authority stays off |
| **TimeW** | ACCEPT/REVISE/HOLD comparison and checkpoint binding for fractional logical `x/y/w/h`; explicit pending/current/final transaction distinctions and zero-rounding rules | **PENDING**, no unilateral changes to TimeW consumer |
| **C3e/C4b** | Independently enrolled SwayPX producer/release/host/session/seat proof, protected challenge/revocation and authenticated receipt | **PENDING** |
| **C5** | Show reviewed **exact** before/target in user-visible form, bind canonical unrounded values to one-use grant, durably SPEND before effect, crash-gap UNKNOWN/HOLD | **PENDING** |
| **Nurse 2** | Independently review new numeric reference and real pending-value CI at its exact head | **PENDING**, parent reviews do not transfer |
| **Operator** | Separate repository merge, installed-host and production authorization | **NOT GRANTED** |

### NEXT

1. Validate exact-head CI: standalone fractional reference and actual disposable real-XDG fractional preservation. Correct only the isolated stacked branch if failure.
2. Request Nurse 2 scoped numeric/CI review of this **new increment**, preserving earlier accepted #168/#169/#170 checkpoints. No premature acceptance of unreviewed #172.
3. Record the SwayPX **REVISE** proposal on TimeW coordination #2067 and obtain the TimeW owner choice of exact canonical representation and state/commit semantics before a protected wire schema or any live compositor integration.
4. Only after cross-owner acceptance, separately implement approved *compositor-origin selected-object epoch/seat/input* and protected per-command mutation/receipts, preserving Company Map #126 and Autoscroll #158 shared seams.

**DECISIONS //** Production TimeW Restore **HOLD**. Scope A first; Scope B Brave identity separate. No launch, move, resize, C5 spend, new producer keys, Fedora filesystem change, GDM action, or merge.
