#!/usr/bin/env python3
"""Overlay real per-instruction READY/TIMEOUT/CURRENT/SCENE probes in CI only.

Execute after the #172, #178 and #179 disposable overlays. Neither this
script nor its C instrumentation is part of production SwayPX.
Native transaction pointers match callbacks only while live; no credentials.
"""
import os
from pathlib import Path

if os.environ.get("GITHUB_ACTIONS") != "true" or os.environ.get("CI") != "true":
    raise SystemExit("BLOCKED_NOT_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NOT_EPHEMERAL_RUNNER")
root = Path(__file__).resolve().parents[2]
txn = root / "sway/desktop/transaction.c"
probe = root / "sway/timew_scope_a_ci_real_xdg.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"
actor = root / "tests/stage_director/ci_native_news_actor_smoke.sh"
meson = root / "sway/meson.build"

def one(source, anchor, replacement, name):
    count = source.count(anchor)
    if count != 1:
        raise SystemExit(f"BLOCKED_CI_OUTCOME_SOURCE_DRIFT_{name} count={count}")
    return source.replace(anchor, replacement, 1)

t, p, s, a, m = (file.read_text() for file in (txn, probe, smoke, actor, meson))
if "TIMEW_CI_OUTCOME_" in t or "TIMEW_CI_OUTCOME_" in p:
    raise SystemExit("BLOCKED_CI_OUTCOME_ALREADY_LINKED")
if "TIMEW_CI_REAL_INSTRUCTION_BIND" not in p:
    raise SystemExit("BLOCKED_NO_PARENT_INSTRUCTION_LINEAGE")
if "'timew_scope_a_observed_lineage.c'" not in m:
    raise SystemExit("BLOCKED_PARENT_PRIVATE_LINEAGE_MISSING")
if "timew_scope_a_ci_outcome_ready(" in m:
    raise SystemExit("BLOCKED_UNAPPROVED_PRODUCTION_SOURCE")

t = one(t,
    '#include "sway/timew_scope_a_ci_real_instruction.h"',
    '#include "sway/timew_scope_a_ci_real_instruction.h"\n'
    '#include "sway/timew_scope_a_ci_real_outcome.h"',
    "TXN_INCLUDE",
)
t = one(t,
    '\tif (instruction != NULL && instruction->serial == serial) {\n'
    '\t\tset_instruction_ready(instruction);',
    '\tif (instruction != NULL && instruction->serial == serial) {\n'
    '\t\ttimew_scope_a_ci_outcome_ready(view,\n'
    '\t\t\t(uintptr_t)instruction->transaction, serial);\n'
    '\t\tset_instruction_ready(instruction);',
    "REAL_SERIAL_READY",
)
t = one(t,
    '\ttransaction->num_waiting = 0;\n\ttransaction_progress();\n\treturn 0;\n}',
    '\ttimew_scope_a_ci_outcome_timeout((uintptr_t)transaction);\n'
    '\ttransaction->num_waiting = 0;\n\ttransaction_progress();\n\treturn 0;\n}',
    "REAL_TIMER_TIMEOUT",
)
t = one(t,
    '\ttransaction_apply(server.queued_transaction);\n\tarrange_root(root);',
    '\ttransaction_apply(server.queued_transaction);\n'
    '\ttimew_scope_a_ci_outcome_current((uintptr_t)server.queued_transaction);\n'
    '\tarrange_root(root);\n'
    '\ttimew_scope_a_ci_outcome_scene((uintptr_t)server.queued_transaction);',
    "REAL_APPLY_AND_SCENE",
)
p = one(p,
    '#include "sway/timew_scope_a_ci_real_instruction.h"',
    '#include "sway/timew_scope_a_ci_real_instruction.h"\n'
    '#include "sway/timew_scope_a_ci_real_outcome.h"',
    "PROBE_INCLUDE",
)

record_source = r"""
enum { CI_MAX_OUTCOME_RECORDS = 128 };
struct ci_outcome_record {
	uintptr_t transaction_ref;
	uint64_t lifetime;
	uint64_t commit_sequence;
	uint32_t configure_serial;
	bool ready, timed_out, current, scene;
};
static struct ci_outcome_record ci_outcomes[CI_MAX_OUTCOME_RECORDS];
static size_t ci_outcome_count;

static void ci_outcome_emit(const char *stage,
		const struct ci_outcome_record *record) {
	fprintf(stderr,
		"TIMEW_CI_OUTCOME_%s lifetime=%" PRIu64
		" sequence=%" PRIu64 " configured=%d"
		" real_callback=1 receipt=NONE authority=NONE\n",
		stage, record->lifetime, record->commit_sequence,
		record->configure_serial != 0 ? 1 : 0);
	fflush(stderr);
}

static void ci_outcome_commit(uintptr_t ref, uint32_t serial) {
	if (!ref || ci_outcome_count >= CI_MAX_OUTCOME_RECORDS) {
		fail("outcome_record_overflow_or_ref");
	}
	struct ci_outcome_record *record = &ci_outcomes[ci_outcome_count++];
	*record = (struct ci_outcome_record) {
		.transaction_ref = ref,
		.lifetime = ci_lifetime.lifetime_sequence,
		.commit_sequence = ci_real_commit_floor,
		.configure_serial = serial,
	};
	ci_outcome_emit("COMMIT", record);
}

static struct ci_outcome_record *ci_outcome_find(
		uintptr_t ref, uint32_t serial, bool require_serial) {
	if (!ref || !ci_view || !ci_issuer.live) {
		return NULL;
	}
	for (size_t i = ci_outcome_count; i > 0; --i) {
		struct ci_outcome_record *record = &ci_outcomes[i - 1];
		if (record->lifetime == ci_lifetime.lifetime_sequence &&
				record->transaction_ref == ref && !record->scene &&
				(!require_serial ||
					(record->configure_serial != 0 &&
					record->configure_serial == serial))) {
			return record;
		}
	}
	return NULL;
}

void timew_scope_a_ci_outcome_ready(struct sway_view *view,
		uintptr_t ref, uint32_t serial) {
	if (!ci_view || view != ci_view) {
		return;
	}
	struct ci_outcome_record *record = ci_outcome_find(ref, serial, true);
	if (!record || record->ready || record->current) {
		fail("real_ready_for_unknown_or_completed_instruction");
	}
	record->ready = true;
	ci_outcome_emit("READY", record);
}

void timew_scope_a_ci_outcome_timeout(uintptr_t ref) {
	for (size_t i = 0; i < ci_outcome_count; ++i) {
		struct ci_outcome_record *record = &ci_outcomes[i];
		if (record->lifetime == ci_lifetime.lifetime_sequence &&
				record->transaction_ref == ref && !record->scene) {
			if (record->timed_out || record->current) {
				fail("duplicate_or_late_native_timeout");
			}
			record->timed_out = true;
			ci_outcome_emit("TIMEOUT", record);
		}
	}
}

void timew_scope_a_ci_outcome_current(uintptr_t ref) {
	for (size_t i = 0; i < ci_outcome_count; ++i) {
		struct ci_outcome_record *record = &ci_outcomes[i];
		if (record->lifetime == ci_lifetime.lifetime_sequence &&
				record->transaction_ref == ref && !record->scene) {
			if (record->current) {
				fail("duplicate_current_copy");
			}
			record->current = true;
			ci_outcome_emit("CURRENT", record);
		}
	}
}

void timew_scope_a_ci_outcome_scene(uintptr_t ref) {
	for (size_t i = 0; i < ci_outcome_count; ++i) {
		struct ci_outcome_record *record = &ci_outcomes[i];
		if (record->lifetime == ci_lifetime.lifetime_sequence &&
				record->transaction_ref == ref && !record->scene) {
			if (!record->current) {
				fail("scene_arranged_without_current_copy");
			}
			record->scene = true;
			ci_outcome_emit("SCENE", record);
		}
	}
}
"""
p = one(p,
    'void timew_scope_a_ci_note_real_instruction(struct sway_view *view,',
    record_source + '\n'
    'void timew_scope_a_ci_note_real_instruction(struct sway_view *view,',
    "PROBE_HELPERS",
)
p = one(p,
    '\t++ci_real_commit_floor;\n\t++ci_real_matched_this_lifetime;',
    '\t++ci_real_commit_floor;\n\t++ci_real_matched_this_lifetime;\n'
    '\tci_outcome_commit(transaction_ref, configure_serial);',
    "COMMIT_SEQUENCE_AT_REAL_INSTRUCTION",
)
p = one(p,
    '\tif (ci_real_matched_this_lifetime == 0) {\n'
    '\t\tfail("no_real_instruction_for_selected_lifetime");\n\t}',
    '\tif (ci_real_matched_this_lifetime == 0) {\n'
    '\t\tfail("no_real_instruction_for_selected_lifetime");\n\t}\n'
    '\tfprintf(stderr, "TIMEW_CI_OUTCOME_SEAL lifetime=%" PRIu64\n'
    '\t\t" committed_instructions=%" PRIu64\n'
    '\t\t" real_unmap=1 settled=0 authority=NONE\\n",\n'
    '\t\tci_lifetime.lifetime_sequence, ci_real_matched_this_lifetime);\n'
    '\tfflush(stderr);',
    "UNMAP_SEAL",
)
a = one(a,
    '  echo "TIMEW_CI_REAL_INSTRUCTION_TRIGGER native_xdg=1 fixture_only=1"',
    '  echo "TIMEW_CI_REAL_INSTRUCTION_TRIGGER native_xdg=1 fixture_only=1"\n'
    '  if [[ "${TIMEW_CI_OUTCOME_MODE:-}" != "" ]]; then\n'
    '    sleep 0.55\n'
    '  fi',
    "ACTOR_NATIVE_CALLBACK_GAP",
)
s = one(s,
    '"$sway_binary" -d -c "$config" > "$log" 2>&1 &',
    '''if [[ "${TIMEW_CI_OUTCOME_MODE:-}" == forced_timeout ]]; then
  sway_argv=("$sway_binary" -d -D txn-wait -D txn-timeout=180 -c "$config")
elif [[ "${TIMEW_CI_OUTCOME_MODE:-}" == normal ]]; then
  sway_argv=("$sway_binary" -d -c "$config")
else
  echo "BLOCKED_UNSUPPORTED_CI_OUTCOME_MODE" >&2
  exit 2
fi
"${sway_argv[@]}" > "$log" 2>&1 &''',
    "DISPOSABLE_MODE_LAUNCH",
)
s = one(s,
    '  python3 -B tests/timew/ci_assert_real_instruction_lineage.py "$log"',
    '  python3 -B tests/timew/ci_assert_real_instruction_lineage.py "$log"\n'
    '  python3 -B tests/timew/ci_assert_real_instruction_outcomes.py '
    '"$log" "$TIMEW_CI_OUTCOME_MODE"',
    "ENFORCE_PER_INSTRUCTION_OUTCOMES",
)
# Validate ALL source anchors before touching any of the temporary files.
txn.write_text(t)
probe.write_text(p)
actor.write_text(a)
smoke.write_text(s)
print("TIMEW_CI_OUTCOME_OVERLAY_PASS: real instruction READY/timeout/"
      "current/scene callbacks correlated, receipt/authority NONE")
