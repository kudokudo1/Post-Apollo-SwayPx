#!/usr/bin/env bash
# Map a deliberately oversized disposable *native Wayland* actor inside the
# CI-only SwayPX headless compositor. This is NOT browser playback, clipping,
# popup hiding or trusted process authentication.
set -euo pipefail
runtime="${1:?private XDG_RUNTIME_DIR}"
wayland_socket="${2:?Wayland display socket basename}"
foot_binary="${STAGE_DIRECTOR_TEST_FOOT:-}"
swaymsg_binary="${STAGE_DIRECTOR_TEST_SWAYMSG:-}"
actor=""
pidfd_witness=""
lineage_root=""
lineage_child=""
actor_log="$runtime/news-actor.log"
tree_json="$runtime/news-tree.json"

cleanup_actor() {
  if [[ -n "$actor" ]]; then
    if kill -0 "$actor" 2>/dev/null; then
      kill -TERM "$actor" 2>/dev/null || true
      for _ in {1..30}; do
        if ! kill -0 "$actor" 2>/dev/null; then break; fi
        sleep 0.1
      done
      if kill -0 "$actor" 2>/dev/null; then
        kill -KILL "$actor" 2>/dev/null || true
      fi
    fi
    wait "$actor" 2>/dev/null || true
  fi
  if [[ -n "$pidfd_witness" ]]; then
    if kill -0 "$pidfd_witness" 2>/dev/null; then
      kill -TERM "$pidfd_witness" 2>/dev/null || true
    fi
    wait "$pidfd_witness" 2>/dev/null || true
  fi
  if [[ -n "$lineage_child" ]] && kill -0 "$lineage_child" 2>/dev/null; then
    kill -TERM "$lineage_child" 2>/dev/null || true
  fi
  if [[ -n "$lineage_root" ]]; then
    if kill -0 "$lineage_root" 2>/dev/null; then
      kill -TERM "$lineage_root" 2>/dev/null || true
    fi
    wait "$lineage_root" 2>/dev/null || true
  fi
}
trap cleanup_actor EXIT

if [[ "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
      -z "${RUNNER_TEMP:-}" || "$runtime" != "$RUNNER_TEMP"/* ||
      ! -d "$runtime" || ! -x "$foot_binary" ||
      ! -x "$swaymsg_binary" || "$wayland_socket" != wayland-* ]]; then
  echo "BLOCKED_UNSAFE_ACTOR_ENV: require disposable CI session and fixed binaries" >&2
  exit 2
fi

ipc_socket=""
for _ in {1..60}; do
  for candidate in "$runtime"/sway-ipc.*.sock; do
    if [[ -S "$candidate" ]]; then
      ipc_socket="$candidate"
      break
    fi
  done
  if [[ -n "$ipc_socket" ]]; then break; fi
  sleep 0.1
done
if [[ -z "$ipc_socket" ]]; then
  echo "BLOCKED_NO_SWAY_IPC: compositor Wayland socket exists but IPC was not registered" >&2
  exit 2
fi

# Keep the test actor's environment private from the GitHub runner account.
mkdir -m 700 -p "$runtime/home" "$runtime/config-home"
export HOME="$runtime/home"
export XDG_CONFIG_HOME="$runtime/config-home"
export XDG_RUNTIME_DIR="$runtime"
export WAYLAND_DISPLAY="$wayland_socket"
export SWAYSOCK="$ipc_socket"
unset DISPLAY

if [[ "${STAGE_DIRECTOR_CI_PIXEL_TEST:-}" == 1 ]]; then
  if [[ -z "${STAGE_DIRECTOR_TEST_GRIM:-}" ||
        ! -x "${STAGE_DIRECTOR_TEST_GRIM:-}" ]]; then
    echo "FAIL: missing pinned grim screenshot executable in CI pixel mode" >&2
    exit 2
  fi
  mkdir -p "$XDG_CONFIG_HOME/foot"
  cat > "$XDG_CONFIG_HOME/foot/foot.ini" <<'EOF'
[colors-dark]
background=ffffff
foreground=000000
alpha=1.0
[colors-light]
background=ffffff
foreground=000000
alpha=1.0
EOF
fi

# App-id imitation intentionally tests classification as CANDIDATE ONLY.
# It is a native xdg_toplevel terminal running a fixed sleep test command,
# NOT a browser, not a URL, and not a trusted WEATHER WATCH child.
"$foot_binary" \
  --app-id=post-apollo-weather-news \
  --title=SD1-CI-DISPOSABLE-NOT-BROWSER \
  --window-size-pixels=1400x850 \
  /usr/bin/sleep 30 >"$actor_log" 2>&1 &
actor=$!

mapped=0
for _ in {1..90}; do
  if ! kill -0 "$actor" 2>/dev/null; then
    echo "Wayland test actor exited before mapping:" >&2
    tail -n 70 "$actor_log" >&2
    exit 1
  fi
  if "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree >"$tree_json" 2>/dev/null &&
      python3 tests/stage_director/assert_native_news_actor.py "$tree_json" >/dev/null 2>&1; then
    mapped=1
    break
  fi
  sleep 0.1
done
if [[ "$mapped" -ne 1 ]]; then
  echo "Native Wayland NEWS test actor did not map with an oversized rect" >&2
  tail -n 70 "$actor_log" >&2
  python3 tests/stage_director/assert_native_news_actor.py "$tree_json" >&2 || true
  exit 1
fi

python3 tests/stage_director/assert_native_news_actor.py "$tree_json"
echo "Stage Director real mapped actor smoke: PASS"

if [[ "${STAGE_DIRECTOR_CI_SCENE_PROBE:-}" == 1 ]]; then
  probe_seen=0
  for _ in {1..70}; do
    if grep -Fq 'SD_CI_SCENE_CLIP measured=1' "$runtime/sway.log"; then
      probe_seen=1
      break
    fi
    sleep 0.1
  done
  if [[ "$probe_seen" != 1 ]]; then
    echo "FAIL: native scene test buffer never reached CI clip/hit-test probe" >&2
    tail -n 100 "$runtime/sway.log" >&2
    exit 1
  fi
  grep -F 'SD_CI_SCENE_CLIP measured=1' "$runtime/sway.log"
  # The existing native SwayPX view_map grabs wl_client credentials.
  # Validate the SAME disposable foot PID (not a title/app-id trust claim)
  # against a fresh procfs identity in the CI compositor. No launching,
  # IPC authorization, focus or browser binding is performed here.
  grep -F 'SD_CI_WL_PEER ' "$runtime/sway.log" || true
  python3 - "$actor" "$runtime/sway.log" "$runtime/ci-native-peer.txt" <<'PY'
import pathlib, re, sys
expected = int(sys.argv[1])
log = pathlib.Path(sys.argv[2]).read_text(encoding="utf-8", errors="replace")
rows = re.findall(
    r"SD_CI_WL_PEER pid=(\d+) view_pid=(\d+) uid=(\d+) "
    r"proc_uid=(\d+) proc_start=(\d+) native_xdg=(\d+) evidence=(PASS|FAIL)",
    log,
)
if len(rows) != 1:
    raise SystemExit(f"FAIL: expected one real Wayland client credential record; got {len(rows)}")
pid, view_pid, uid, proc_uid, start, native_xdg, result = rows[0]
if not (int(pid) == expected == int(view_pid) and
        int(uid) == int(proc_uid) and int(start) > 0 and
        native_xdg == "1" and result == "PASS"):
    raise SystemExit("FAIL: native Wayland peer does not match disposable actor/process")
pathlib.Path(sys.argv[3]).write_text(f"{pid} {uid} {start}\n", encoding="ascii")
print("SD_CI_WL_PEER_CHECK source=actual_wl_client actor_pid_matched=1 "
      "uid_starttime_verified=1 authority=NONE result=PASS")
PY
  if [[ "${STAGE_DIRECTOR_CI_PIDFD_WITNESS:-}" == 1 ]]; then
    if [[ -z "${STAGE_DIRECTOR_TEST_PIDFD_WITNESS:-}" ||
          ! -x "${STAGE_DIRECTOR_TEST_PIDFD_WITNESS:-}" ]]; then
      echo "FAIL: no independently compiled CI-only pidfd witness" >&2
      exit 1
    fi
    read -r peer_pid peer_uid peer_start < "$runtime/ci-native-peer.txt"
    "$STAGE_DIRECTOR_TEST_PIDFD_WITNESS" \
      "$peer_pid" "$peer_uid" "$peer_start" \
      "$runtime/sd1-pidfd.ready" >"$runtime/sd1-pidfd.log" 2>&1 &
    pidfd_witness=$!
    ready=0
    for _ in {1..100}; do
      if [[ -f "$runtime/sd1-pidfd.ready" ]] &&
          grep -Fxq READY "$runtime/sd1-pidfd.ready"; then
        ready=1
        break
      fi
      if ! kill -0 "$pidfd_witness" 2>/dev/null; then break; fi
      sleep 0.1
    done
    if [[ "$ready" != 1 ]]; then
      echo "FAIL: pidfd never held the mapped native peer" >&2
      cat "$runtime/sd1-pidfd.log" >&2 || true
      exit 1
    fi
    grep -F 'SD_CI_PIDFD_HELD ' "$runtime/sd1-pidfd.log"
  fi
  if [[ "${STAGE_DIRECTOR_CI_PIXEL_TEST:-}" == 1 ]]; then
    expected="after_inside=1 after_outside=0 normal_clip_restored=0 result=PASS"
  else
    expected="after_inside=1 after_outside=0 normal_clip_restored=1 result=PASS"
  fi
  if ! grep -Fq "$expected" "$runtime/sway.log"; then
    echo "FAIL: actual native clip/hit-test behavior did not match the active CI mode" >&2
    exit 1
  fi
  echo "Stage Director actual wlroots scene hit-test clip probe: PASS"

  if [[ "${STAGE_DIRECTOR_CI_PIXEL_TEST:-}" == 1 ]]; then
    if ! grep -Fq 'SD_CI_RENDER_PIXEL inside=' "$runtime/sway.log"; then
      echo "FAIL: real compositor never reported held pixel-test clip" >&2
      exit 1
    fi
    # Complete compositor screenshot uses real wlr-screencopy protocol; no
    # simulation, offscreen PNG generation or source-level guess.
    sleep 0.35
    "$STAGE_DIRECTOR_TEST_GRIM" -t ppm "$runtime/news-clip.ppm"
    "$STAGE_DIRECTOR_TEST_GRIM" -t png "$runtime/news-clip.png"
    test -s "$runtime/news-clip.png"
    cp -- "$runtime/news-clip.png" "$RUNNER_TEMP/stage-director-render-evidence.png"
    echo "Stage Director CI: preserved actual compositor screenshot for auditing"
    if [[ "${STAGE_DIRECTOR_CI_RENDER_NORMALIZE:-}" == 1 ]]; then
      if ! grep -Eq 'SD_CI_BUFFER_DEST normalized=[1-9][0-9]*' "$runtime/sway.log"; then
        echo "FAIL: no actual native wlr_scene_surface destination was normalized" >&2
        exit 1
      fi
      grep -F 'SD_CI_BUFFER_DEST normalized=' "$runtime/sway.log" | tail -n 3
    fi
    grep -F 'SD_CI_POST_LAYOUT clip_reapplied=1' "$runtime/sway.log" || true
    grep -F 'SD_CI_BUFFER_METRICS ' "$runtime/sway.log" || true
    grep -F 'SD_CI_PAINT_BUFFER ' "$runtime/sway.log" || true
    grep -F 'SD_CI_PAINT_AUDIT ' "$runtime/sway.log" || true
    grep -F 'SD_CI_DRAW_NODE ' "$runtime/sway.log" || true
    grep -F 'SD_CI_DRAW_AUDIT ' "$runtime/sway.log" || true
    if [[ "${STAGE_DIRECTOR_CI_FX_PASS_TRACE:-}" == 1 ]]; then
      grep -F 'SD_CI_FX_SCENE_TEX ' "$runtime/sway.log" || true
      grep -F 'SD_CI_FX_GL_QUADS ' "$runtime/sway.log" || true
      grep -F 'SD_CI_FRAME_DEST_PRESERVED ' "$runtime/sway.log" || true
      if [[ "${STAGE_DIRECTOR_CI_FRAME_DEST_OVERRIDE:-}" == 1 ]] &&
          ! grep -Fq 'SD_CI_FRAME_DEST_PRESERVED dst=320x240' "$runtime/sway.log"; then
        echo "FAIL: output repaint overwrote CI-held native destination" >&2
        exit 1
      fi
      if ! grep -Eq 'SD_CI_FX_SCENE_TEX src=96,96 320x240 dst=[0-9]+,[0-9]+ 320x240 ' "$runtime/sway.log"; then
        echo "FAIL: pinned SceneFX actual scene draw region not measured" >&2
        exit 1
      fi
      if ! grep -Eq 'SD_CI_FX_GL_QUADS dst=[0-9]+,[0-9]+ 320x240 ' "$runtime/sway.log"; then
        echo "FAIL: pinned SceneFX submitted GLES texture region not measured" >&2
        exit 1
      fi
    fi
    if ! grep -Eq 'SD_CI_DRAW_AUDIT nodes=[1-9][0-9]* trees=[1-9][0-9]* buffers=[1-9][0-9]* drawable_buffers=[1-9][0-9]*' "$runtime/sway.log"; then
      echo "FAIL: CI drawable-scene node provenance absent" >&2
      exit 1
    fi
    if ! grep -Eq 'SD_CI_PAINT_AUDIT content=[1-9][0-9]* content_surfaces=[1-9][0-9]* container=[1-9][0-9]*' "$runtime/sway.log"; then
      echo "FAIL: CI renderer scene buffer provenance not measured" >&2
      exit 1
    fi
    if [[ "${STAGE_DIRECTOR_CI_RENDER_NORMALIZE:-}" == 1 ]] &&
        ! grep -Eq 'SD_CI_BUFFER_METRICS .*after_dst=320x240 ' "$runtime/sway.log"; then
      echo "FAIL: CI-only buffer destination metrics absent or not normalized" >&2
      exit 1
    fi
    if ! grep -Eq 'SD_CI_POST_LAYOUT clip_reapplied=1 origin=[0-9]+,[0-9]+ inside=1 outside=0 result=PASS' "$runtime/sway.log"; then
      echo "FAIL: native clip not verified AFTER Sway arrange_root transaction" >&2
      exit 1
    fi
    if [[ -n "${STAGE_DIRECTOR_CI_COLD_STAGE:-}" ]]; then
      stage="$STAGE_DIRECTOR_CI_COLD_STAGE"
      if [[ "$stage" != initial && "$stage" != fresh ||
            "${CI:-}" != true || "${GITHUB_ACTIONS:-}" != true ||
            -z "${RUNNER_TEMP:-}" ]]; then
        echo "BLOCKED_INVALID_COLD_PIXEL_STAGE" >&2
        exit 2
      fi
      # Preserve both independent native screenshots BEFORE acceptance.
      # The workflow rejects either leaking frame after both boots exist.
      cp -- "$runtime/news-clip.ppm" "$RUNNER_TEMP/stage-director-cold-$stage.ppm"
      cp -- "$runtime/sway.log" "$RUNNER_TEMP/stage-director-cold-$stage.log"
      cp -- "$tree_json" "$RUNNER_TEMP/stage-director-cold-$stage-tree.json"
      cp -- "$runtime/news-clip.png" "$RUNNER_TEMP/stage-director-render-evidence-cold-$stage.png"
      echo "SD_CI_COLD_CAPTURE stage=$stage screenshot_saved=1"
      if [[ "$stage" == initial &&
            "${STAGE_DIRECTOR_CI_MOVE_PIXEL:-}" == 1 ]]; then
        # Move the SAME still-mapped native actor on the SAME fixed output.
        # This is only performed on the disposable GitHub-hosted compositor.
        # No production scene authority or output mode change is involved.
        echo "SD_CI_MOVE_START target=520,180"
        "$swaymsg_binary" -s "$ipc_socket" -r \
          '[app_id="post-apollo-weather-news" title="SD1-CI-DISPOSABLE-NOT-BROWSER"] move position 520 180' \
          >"$runtime/news-move-response.json"
        python3 - "$runtime/news-move-response.json" <<'PY'
import json
import sys
data = json.load(open(sys.argv[1], encoding="utf-8"))
if not isinstance(data, list) or len(data) != 1 or data[0].get("success") is not True:
    raise SystemExit("FAIL: CI native actor move command was not accepted")
PY
        sleep 0.45
        "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
          >"$runtime/news-tree-moved.json"
        "$STAGE_DIRECTOR_TEST_GRIM" -t ppm "$runtime/news-clip-moved.ppm"
        "$STAGE_DIRECTOR_TEST_GRIM" -t png \
          "$RUNNER_TEMP/stage-director-render-evidence-moved.png"
        cp -- "$runtime/news-clip-moved.ppm" \
          "$RUNNER_TEMP/stage-director-moved.ppm"
        cp -- "$runtime/news-tree-moved.json" \
          "$RUNNER_TEMP/stage-director-moved-tree.json"
        echo "SD_CI_MOVE_CAPTURE actor_still_mapped=1 screenshot_saved=1"
        if [[ "${STAGE_DIRECTOR_CI_OPACITY_PIXEL:-}" == 1 ]]; then
          # The same XDG native actor remains mapped. Only its Sway-native
          # opacity is toggled to test drawing versus residual pixels.
          echo "SD_CI_OPACITY_START alpha=0 actor_still_mapped=1"
          "$swaymsg_binary" -s "$ipc_socket" -r \
            '[app_id="post-apollo-weather-news" title="SD1-CI-DISPOSABLE-NOT-BROWSER"] opacity set 0' \
            >"$runtime/news-opacity-zero-response.json"
          python3 - "$runtime/news-opacity-zero-response.json" <<'PY'
import json, sys
responses=json.load(open(sys.argv[1], encoding="utf-8"))
if len(responses) != 1 or responses[0].get("success") is not True:
    raise SystemExit("FAIL: CI Sway opacity-zero command rejected")
PY
          sleep 0.4
          "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
            >"$runtime/news-tree-opacity-zero.json"
          "$STAGE_DIRECTOR_TEST_GRIM" -t ppm \
            "$RUNNER_TEMP/stage-director-opacity-zero.ppm"
          "$STAGE_DIRECTOR_TEST_GRIM" -t png \
            "$RUNNER_TEMP/stage-director-render-evidence-opacity-zero.png"
          echo "SD_CI_OPACITY_CAPTURE alpha=0 actor_still_mapped=1"
          "$swaymsg_binary" -s "$ipc_socket" -r \
            '[app_id="post-apollo-weather-news" title="SD1-CI-DISPOSABLE-NOT-BROWSER"] opacity set 1' \
            >"$runtime/news-opacity-one-response.json"
          python3 - "$runtime/news-opacity-one-response.json" <<'PY'
import json, sys
responses=json.load(open(sys.argv[1], encoding="utf-8"))
if len(responses) != 1 or responses[0].get("success") is not True:
    raise SystemExit("FAIL: CI Sway opacity-restoration command rejected")
PY
          sleep 0.4
          "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
            >"$runtime/news-tree-opacity-one.json"
          "$STAGE_DIRECTOR_TEST_GRIM" -t ppm \
            "$RUNNER_TEMP/stage-director-opacity-one.ppm"
          "$STAGE_DIRECTOR_TEST_GRIM" -t png \
            "$RUNNER_TEMP/stage-director-render-evidence-opacity-one.png"
          cp -- "$runtime/news-tree-opacity-zero.json" \
            "$RUNNER_TEMP/stage-director-opacity-zero-tree.json"
          cp -- "$runtime/news-tree-opacity-one.json" \
            "$RUNNER_TEMP/stage-director-opacity-one-tree.json"
          echo "SD_CI_OPACITY_CAPTURE alpha=1 actor_still_mapped=1"
          if [[ "${STAGE_DIRECTOR_CI_BACKGROUND_REFRESH:-}" == 1 ]]; then
            # Replace only the headless background surface twice, leaving
            # output mode, actor geometry and actor opacity unchanged.
            # This is a redraw/damage discriminator, NOT paint acceptance.
            for bg in '#010101' '#000000'; do
              "$swaymsg_binary" -s "$ipc_socket" -r \
                "output * bg $bg solid_color" >"$runtime/news-bg-response.json"
              python3 - "$runtime/news-bg-response.json" <<'PY'
import json, sys
responses = json.load(open(sys.argv[1], encoding="utf-8"))
if not isinstance(responses, list) or not responses or not all(
    isinstance(i, dict) and i.get("success") is True for i in responses
):
    raise SystemExit("FAIL: CI-only background refresh rejected")
PY
              sleep 0.35
            done
            "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
              >"$runtime/news-tree-background-refreshed.json"
            "$STAGE_DIRECTOR_TEST_GRIM" -t ppm \
              "$RUNNER_TEMP/stage-director-background-refreshed.ppm"
            "$STAGE_DIRECTOR_TEST_GRIM" -t png \
              "$RUNNER_TEMP/stage-director-render-evidence-background-refreshed.png"
            cp -- "$runtime/news-tree-background-refreshed.json" \
              "$RUNNER_TEMP/stage-director-background-refreshed-tree.json"
            echo "SD_CI_BACKGROUND_REFRESH mapped_actor=1 desktop_restored=1"
          fi
        fi
      fi
    elif [[ "${STAGE_DIRECTOR_CI_OUTPUT_REALLOC:-}" == 1 ]]; then
      # Changing the isolated headless output size requires a newly sized
      # render buffer. This is a diagnostic, NEVER a production resize.
      # Keep the original (possibly failing) screenshot as an independent
      # acceptance gate so a fresh frame cannot hide an initial leak.
      echo "Stage Director CI: reallocate disposable output 1920x1080 -> 1918x1078"
      "$swaymsg_binary" -s "$ipc_socket" -r \
        'output * resolution 1918x1078' >"$runtime/news-output-change.json"
      python3 - "$runtime/news-output-change.json" <<'PY'
import json
import sys
responses = json.load(open(sys.argv[1], encoding="utf-8"))
if not isinstance(responses, list) or not responses or not all(
    isinstance(item, dict) and item.get("success") is True for item in responses
):
    raise SystemExit("FAIL: isolated output resize rejected by SwayPX")
PY
      sleep 0.5
      "$STAGE_DIRECTOR_TEST_GRIM" -t ppm "$runtime/news-clip-reallocated.ppm"
      "$STAGE_DIRECTOR_TEST_GRIM" -t png \
        "$RUNNER_TEMP/stage-director-render-evidence-reallocated.png"
      "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
        >"$runtime/news-tree-reallocated.json"
      python3 tests/stage_director/compare_news_reallocated_pixels.py \
        "$runtime/news-clip.ppm" "$runtime/news-clip-reallocated.ppm" \
        "$tree_json" "$runtime/news-tree-reallocated.json"

      # Preserve both failure results and the actual second screenshot.
      original_result=0
      fresh_result=0
      python3 tests/stage_director/assert_news_render_pixels.py \
        "$runtime/sway.log" "$runtime/news-clip.ppm" "$tree_json" || original_result=$?
      python3 tests/stage_director/assert_news_render_pixels.py \
        "$runtime/sway.log" "$runtime/news-clip-reallocated.ppm" \
        "$runtime/news-tree-reallocated.json" || fresh_result=$?
      if [[ "$original_result" != 0 || "$fresh_result" != 0 ]]; then
        echo "FAIL: original=$original_result fresh_output=$fresh_result; CI render gate still closed" >&2
        exit 1
      fi
    else
      python3 tests/stage_director/assert_news_render_pixels.py \
        "$runtime/sway.log" "$runtime/news-clip.ppm" "$tree_json"
    fi
  fi
fi
if [[ "${STAGE_DIRECTOR_CI_PIDFD_WITNESS:-}" == 1 ]]; then
  # All compositor screen captures finished. Close only our disposable foot.
  # The separate witness has held its pidfd since the mapped state and must
  # observe a kernel process exit before we accept release and replay gates.
  if [[ -z "$pidfd_witness" || -z "$actor" ]]; then
    echo "FAIL: missing actor or pidfd lifetime witness" >&2
    exit 1
  fi
  kill -TERM "$actor"
  wait "$actor" 2>/dev/null || true
  actor=""
  if ! wait "$pidfd_witness"; then
    cat "$runtime/sd1-pidfd.log" >&2 || true
    echo "FAIL: native pidfd did not observe actor exit" >&2
    exit 1
  fi
  pidfd_witness=""
  cat "$runtime/sd1-pidfd.log"
  if ! grep -Fq 'SD_CI_PIDFD_RELEASE process_exit_observed=1 pidfd_closed=1 epoch_replay_rejected=1 authority=NONE result=PASS' \
      "$runtime/sd1-pidfd.log"; then
    echo "FAIL: native pidfd release and replay gate missing" >&2
    exit 1
  fi
fi
if [[ "${STAGE_DIRECTOR_CI_CHILD_LINEAGE:-0}" == 1 ]]; then
  if [[ -z "${STAGE_DIRECTOR_TEST_CHILD_LINEAGE_WITNESS:-}" ||
        ! -x "$STAGE_DIRECTOR_TEST_CHILD_LINEAGE_WITNESS" ||
        "${GITHUB_ACTIONS:-}" != true ||
        "${CI:-}" != true ]]; then
    echo "BLOCKED_CI_CHILD_LINEAGE_ENV" >&2
    exit 2
  fi
  # The FIRST fixed actor's 320x240 render, move, opacity, cold
  # compositor and pidfd-exit checks are already complete. Launch a
  # DIFFERENT disposable native window through an independent bash
  # parent; it cannot acquire the first actor's CI source/dest crop.
  # The parent's real kernel PID+starttime identifies our launch root.
  CI_CHILD_PID_FILE="$runtime/sd1-lineage-child.pid" \
  CI_CHILD_ACTOR_LOG="$runtime/sd1-lineage-child.log" \
    bash -c '
      "$@" >"$CI_CHILD_ACTOR_LOG" 2>&1 &
      child=$!
      printf "%d\\n" "$child" >"$CI_CHILD_PID_FILE"
      wait "$child"
    ' _ "$foot_binary" \
        --app-id=post-apollo-weather-news-child \
        --title=SD1-CI-LINEAGE-CHILD-NOT-BROWSER \
        --window-size-pixels=850x600 \
        /usr/bin/sleep 30 >"$runtime/sd1-lineage-launcher.log" 2>&1 &
  lineage_root=$!

  lineage_mapped=0
  for _ in {1..90}; do
    if [[ -s "$runtime/sd1-lineage-child.pid" ]]; then
      read -r lineage_child < "$runtime/sd1-lineage-child.pid"
      if [[ "$lineage_child" =~ ^[1-9][0-9]*$ ]] &&
          "$swaymsg_binary" -s "$ipc_socket" -r -t get_tree \
            >"$runtime/sd1-lineage-tree.json" 2>/dev/null &&
          python3 - "$runtime/sd1-lineage-tree.json" "$lineage_child" <<'PY'
import json, sys
tree=json.load(open(sys.argv[1],encoding="utf-8"))
pid=int(sys.argv[2])
matches=[]
def visit(node):
    if (node.get("pid")==pid and
            node.get("app_id")=="post-apollo-weather-news-child" and
            node.get("name")=="SD1-CI-LINEAGE-CHILD-NOT-BROWSER"):
        matches.append(node)
    for key in ("nodes","floating_nodes"):
        for child in node.get(key,[]):
            visit(child)
visit(tree)
if len(matches)!=1:
    raise SystemExit("FAIL: no exact mapped native lineage child")
PY
      then
        lineage_mapped=1
        break
      fi
    fi
    if ! kill -0 "$lineage_root" 2>/dev/null; then break; fi
    sleep 0.1
  done
  if [[ "$lineage_mapped" != 1 ]]; then
    echo "FAIL: trusted CI launch root did not map exact child actor" >&2
    cat "$runtime/sd1-lineage-launcher.log" >&2 || true
    cat "$runtime/sd1-lineage-child.log" >&2 || true
    exit 1
  fi
  native_seen=0
  for _ in {1..50}; do
    if grep -Fq 'SD_CI_CHILD_WL_PEER ' "$runtime/sway.log"; then
      native_seen=1
      break
    fi
    sleep 0.1
  done
  if [[ "$native_seen" != 1 ]]; then
    echo "FAIL: no native wl_client credentials for separate fixed child" >&2
    exit 1
  fi
  grep -F 'SD_CI_CHILD_WL_PEER ' "$runtime/sway.log"
  python3 - "$runtime/sway.log" "$lineage_child" "$runtime/sd1-lineage-peer" <<'PY'
import pathlib, re, sys
log=pathlib.Path(sys.argv[1]).read_text(encoding="utf-8",errors="replace")
expected=int(sys.argv[2])
rows=re.findall(
    r"SD_CI_CHILD_WL_PEER pid=(\d+) view_pid=(\d+) uid=(\d+) "
    r"proc_uid=(\d+) proc_start=(\d+) evidence=(PASS|FAIL) authority=NONE",
    log)
if len(rows)!=1:
    raise SystemExit(f"FAIL: expected single distinct Wayland child credential, got {len(rows)}")
pid, view_pid, uid, proc_uid, start, result=rows[0]
if not (int(pid)==expected==int(view_pid) and
        int(uid)==int(proc_uid) and int(start)>0 and result=="PASS"):
    raise SystemExit("FAIL: Wayland child is not the fixed launcher child")
pathlib.Path(sys.argv[3]).write_text(f"{pid} {uid} {start}\n",encoding="ascii")
PY
  read -r peer_pid peer_uid peer_start < "$runtime/sd1-lineage-peer"
  "$STAGE_DIRECTOR_TEST_CHILD_LINEAGE_WITNESS" \
    "$lineage_root" "$peer_pid" "$peer_uid" "$peer_start"

  # The launcher is the parent of exactly this test actor. Terminate only
  # our own child, reap the bash parent; no browser/session actor exists.
  kill -TERM "$lineage_child"
  wait "$lineage_root" 2>/dev/null || true
  lineage_root=""
  lineage_child=""
  echo "SD_CI_CHILD_LIFETIME owned_launch_root_reaped=1 authority=NONE result=PASS"
fi
echo "No video browser, keyboard/popup guard or production scene authority activated."
