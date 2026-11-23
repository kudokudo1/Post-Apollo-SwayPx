#!/usr/bin/env python3
"""Inject read-only TimeW runtime probes into a DISPOSABLE CI checkout ONLY.

The checked-in SwayPX source remains unmodified. In particular production
sway/meson.build, sway/tree/view.c, and Stage Director scripts are untouched
in the repository. Never invoke this on the operator's host/worktree.
"""
import os
from pathlib import Path

if os.environ.get("CI") != "true" or os.environ.get("GITHUB_ACTIONS") != "true":
    raise SystemExit("BLOCKED_NOT_GITHUB_ACTIONS")
if not os.environ.get("RUNNER_TEMP", "").startswith("/home/runner/"):
    raise SystemExit("BLOCKED_NOT_EPHEMERAL_RUNNER")

root = Path(__file__).resolve().parents[2]
meson = root / "sway/meson.build"
view = root / "sway/tree/view.c"
smoke = root / "tests/stage_director/headless_sway_smoke.sh"

def replace_once(text, anchor, replacement, label):
    if text.count(anchor) != 1:
        raise SystemExit(f"BLOCKED_CI_INJECTION_DRIFT_{label}: matches={text.count(anchor)}")
    return text.replace(anchor, replacement, 1)

m = meson.read_text()
v = view.read_text()
s = smoke.read_text()
units = (
    "timew_scope_a_lifetime_candidate.c",
    "timew_scope_a_native_observer.c",
    "timew_scope_a_lifecycle_signals.c",
    "timew_scope_a_native_lifecycle.c",
    "timew_scope_a_ci_real_xdg.c",
    "timew_scope_a_exact_geometry.c",
)
for unit in units:
    if f"'{unit}'" in m:
        raise SystemExit(f"BLOCKED_ALREADY_LINKED_PRODUCTION_{unit}")
m = replace_once(
    m,
    "\t'stage_director_social_native.c',",
    "\t'stage_director_social_native.c'," +
        "".join(f"\n\t'{unit}'," for unit in units),
    "MESON",
)
v = replace_once(
    v, '#include "sway/tree/view.h"',
    '#include "sway/tree/view.h"\n#include "sway/timew_scope_a_ci_real_xdg.h"',
    "VIEW_INCLUDE",
)
v = replace_once(
    v,
    "void view_update_size(struct sway_view *view) {\n"
    "\tstruct sway_container *con = view->container;\n"
    "\tcon->pending.content_width = view->geometry.width;\n"
    "\tcon->pending.content_height = view->geometry.height;\n"
    "\tcontainer_set_geometry_from_content(con);\n}",
    "void view_update_size(struct sway_view *view) {\n"
    "\tstruct sway_container *con = view->container;\n"
    "\tcon->pending.content_width = view->geometry.width;\n"
    "\tcon->pending.content_height = view->geometry.height;\n"
    "\tcontainer_set_geometry_from_content(con);\n"
    "\ttimew_scope_a_ci_real_xdg_after_map(view);\n}",
    "REAL_COMMITTED_XDG_GEOMETRY",
)
v = replace_once(
    v, "\twl_signal_emit_mutable(&view->events.unmap, view);",
    "\twl_signal_emit_mutable(&view->events.unmap, view);\n"
    "\ttimew_scope_a_ci_real_xdg_after_unmap_signal(view);",
    "REAL_UNMAP_SIGNAL",
)
actor_call = (
    "  bash tests/stage_director/ci_native_news_actor_smoke.sh \\\n"
    '    "$runtime" "${socket##*/}"'
)
if actor_call not in s:
    raise SystemExit("BLOCKED_CI_ACTOR_ANCHOR_MISSING")
replacement = """  for cycle in 1 2; do
    bash tests/stage_director/ci_native_news_actor_smoke.sh \\
      "$runtime" "${socket##*/}"
    proven=0
    for _ in {1..100}; do
      maps="$(grep -c 'TIMEW_CI_REAL_XDG_MAPPED gen=' "$log" || true)"
      unmaps="$(grep -c 'TIMEW_CI_REAL_XDG_UNMAPPED gen=' "$log" || true)"
      if [[ "$maps" == "$cycle" && "$unmaps" == "$cycle" ]]; then
        proven=1
        break
      fi
      sleep 0.1
    done
    if [[ "$proven" != 1 ]]; then
      echo "TIMEW_CI_REAL_XDG_FAIL: missing actual mapped/unmapped cycle $cycle" >&2
      tail -n 120 "$log" >&2
      exit 1
    fi
  done
  if grep -Fq 'TIMEW_CI_REAL_XDG_FAIL' "$log"; then
    echo "TIMEW_CI_REAL_XDG_FAIL: compositor observed a denied lifecycle" >&2
    tail -n 120 "$log" >&2
    exit 1
  fi
  exact_count="$(grep -c 'TIMEW_CI_REAL_XDG_EXACT_GEOMETRY pending=' "$log" || true)"
  if [[ "$exact_count" != 2 ]]; then
    echo "TIMEW_CI_REAL_XDG_FAIL: expected 2 real exact binary64 pending snapshots; got $exact_count" >&2
    exit 1
  fi
  grep -F 'TIMEW_CI_REAL_XDG_' "$log"
  echo "TIMEW_CI_REAL_XDG_RUNTIME PASS: 2 real mapped XDG lifetimes, synchronous unmap-before-destroy, exact pending binary64=2, zero native authority"
"""
s = replace_once(s, actor_call, replacement.rstrip("\n"), "REAL_ACTOR")
# Validate every target before writing even the temporary CI checkout.
meson.write_text(m)
view.write_text(v)
smoke.write_text(s)
print("TIMEW_CI_ONLY_INJECTION: temporary native hooks and 2 disposable real XDG lifetimes enabled")
