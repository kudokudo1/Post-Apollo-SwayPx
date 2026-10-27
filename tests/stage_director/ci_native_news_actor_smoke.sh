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
echo "No video browser, keyboard/popup guard or production scene authority activated."
