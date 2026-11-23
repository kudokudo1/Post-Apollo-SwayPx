"""SwayPX TimeW Scope A: source-side proof-obligation inventory (CI only).

This is NOT an authenticated receipt parser or TimeW runtime dependency.
It reuses #182's strict native diagnostic log checker and can only emit HOLD.
Its inputs are ordinary, caller-controlled diagnostic test strings; they can
never confer selected-window, enrolled-source, C5 or movement authority.
"""

from __future__ import annotations

from ci_assert_real_instruction_outcomes import validate

KIND = "timew-scope-a-swaypx-proof-obligations-untrusted"
REVISION = 1  # test schema revision, NOT a signed wire version

# Owners of these positive obligations are deliberately NOT implemented here.
GAPS = (
    "OPERATOR_SELECTED_LIVE_XDG_UNVERIFIED",
    "ACTIVE_SEAT_AND_INPUT_ORIGIN_UNVERIFIED",
    "COMPOSITOR_BOOT_AND_LIFETIME_UNENROLLED",
    "OUTPUT_WORKSPACE_SCALE_TRANSFORM_EPOCH_UNPROTECTED",
    "EXACT_BEFORE_TARGET_AFTER_GEOMETRY_UNBOUND",
    "NATIVE_INSTRUCTION_ACK_VS_TIMEOUT_UNATTESTED",
    "SELECTED_VIEW_FINAL_FRAME_UNVERIFIED",
    "C3E_C4B_INSTALLED_SOURCE_UNENROLLED",
    "C5_DURABLE_ONE_USE_SPENT_UNVERIFIED",
    "SIGNED_PER_STEP_RECEIPT_ABSENT",
    "CRASH_AND_ROLLBACK_RECONCILIATION_UNVERIFIED",
)

DENIED = (
    "source_authenticated",
    "operator_selection_authenticated",
    "client_ack_authenticated",
    "frame_settled_authenticated",
    "c5_spent_authenticated",
    "receipt_authenticated",
    "native_resize_allowed",
    "native_move_allowed",
    "retry_allowed",
    "undo_allowed",
)


def _hold(status: str, *, step: str, native: dict | None = None) -> dict:
    result = {
        "kind": KIND,
        "revision": REVISION,
        "status": status,
        "step": step,
        "diagnostic_only": True,
        "needs": list(GAPS),
        "safe_actions": [],
        "protected_receipt": None,
        "pending_or_current_does_not_mean_settled": True,
        "resize_to_move_continuation_allowed": False,
        "qualified_diagnostic_count": (
            native["qualified"] if native is not None else 0
        ),
        "ambiguous_ready_timeout_count": (
            native["ambiguous_ready_timeout_hold"] if native is not None else 0
        ),
    }
    result.update({name: False for name in DENIED})
    return result


def inspect_native_obligations(
    *,
    lines: list[str],
    mode: str,
    step: str = "resize",
    claimed_prior_resize_receipt=None,
) -> dict:
    """Read the previous source-only XDG native trace; never authorize effects.

    claimed_prior_resize_receipt is untrusted, regardless of shape or value,
    and is NEVER parsed or promoted. A future signed transport+receipt must
    be independently designed and accepted by TimeW/C3e/C4b/C5.
    """
    if (type(lines) is not list
            or any(type(line) is not str for line in lines)
            or type(mode) is not str
            or type(step) is not str
            or step not in ("resize", "move")
            or mode not in ("normal", "forced_timeout")):
        return _hold("INVALID_DIAGNOSTIC_REQUEST_HOLD", step="unknown")
    try:
        native = validate(lines, mode)
    except (ValueError, TypeError, KeyError, OverflowError):
        return _hold("MISSING_OR_INVALID_NATIVE_EVIDENCE_HOLD", step=step)

    if step == "move":
        # A caller-created dict, pointer, serial or local SQLite row cannot
        # attest a protected and settled resize, even with matching geometry.
        # Never use claimed_prior_resize_receipt as a source of authority.
        return _hold("MOVE_BLOCKED_WITHOUT_PROTECTED_RESIZE_RECEIPT_HOLD",
                     step=step, native=native)
    if native["ambiguous_ready_timeout_hold"]:
        return _hold("CONTRADICTORY_NATIVE_READY_TIMEOUT_HOLD",
                     step=step, native=native)
    if native["qualified"]:
        return _hold("MATCHED_NATIVE_DIAGNOSTIC_UNTRUSTED_HOLD",
                     step=step, native=native)
    return _hold("NO_CLEAN_NATIVE_OUTCOME_HOLD", step=step, native=native)
