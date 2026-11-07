#!/usr/bin/env python3
"""Concurrent real-pixel seam sampling while a CI-only native Sway seatop runs.

A separate worker repeatedly requests real compositor screenshots while the
main test thread dispatches bounded, test-only native pointer steps. Before
and after each screenshot, it independently observes exact live Sway member
rectangles. A changed pair is labeled an unscorable transition, never a
false success. A stable pair must have correctly rendered rail/guest edges.

This is a screenshot stream, NOT every compositor presentation frame and NOT
an authenticated production window-group API. Nothing leaves RUNNER_TEMP.
"""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import threading
import time

import ci_social_real_wayland_pair as b


class StreamingSeamWitness:
    def __init__(self, *, name, swaymsg, ipc, grim, env, root, pids, ids,
                 unrelated_rect, min_frames=4, min_positions=3):
        if (env.get("CI") != "true" or env.get("GITHUB_ACTIONS") != "true" or
                env.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE") != "1" or
                env.get("RUNNER_TEMP") != "/home/runner/work/_temp" or
                os.getuid() == 0 or not name.replace("-", "").isalnum()):
            raise RuntimeError("BLOCKED_SOCIAL_STREAM_NOT_PRIVATE_CI")
        root = Path(root).resolve()
        if (not root.is_dir() or not root.is_relative_to(
                Path(env["RUNNER_TEMP"]).resolve())):
            raise RuntimeError("BLOCKED_SOCIAL_STREAM_RUNTIME_PATH")
        self.name = name
        self.swaymsg, self.ipc, self.grim = swaymsg, ipc, grim
        self.env, self.root = dict(env), root
        self.pids, self.ids = dict(pids), dict(ids)
        self.unrelated_rect = tuple(unrelated_rect)
        self.min_frames, self.min_positions = min_frames, min_positions
        self.stop_flag = threading.Event()
        self.thread = None
        self.total = 0
        self.stable = 0
        self.transitions = 0
        self.positions = set()
        self.error = None

    @staticmethod
    def _rect(node):
        if node is None:
            return None
        r = node.get("rect") or {}
        v = tuple(r.get(k) for k in ("x", "y", "width", "height"))
        return v if all(type(x) is int for x in v) else None

    def _read_pair(self):
        tree = b.read_tree(self.swaymsg, self.ipc, self.env)
        out = []
        for app in (b.APP_HOST, b.APP_GUEST, b.APP_OTHER):
            node = b.select(tree, app, self.pids[app])
            if (node is None or node.get("id") != self.ids[app] or
                    node.get("visible") is not True or
                    node.get("floating") not in ("user_on", "auto_on") or
                    node.get("border") != "none"):
                raise RuntimeError(
                    "SD_CI_SOCIAL_STREAM_IDENTITY_LOST role="+app)
            rect = self._rect(node)
            if rect is None:
                raise RuntimeError(
                    "SD_CI_SOCIAL_STREAM_INVALID_RECT role="+app)
            out.append(rect)
        return tuple(out)

    @staticmethod
    def _assert_pixels(data, host, guest, unrelated, name, index):
        if guest != b.guest_from_chassis(host):
            raise RuntimeError(
                "SD_CI_SOCIAL_STREAM_GEOMETRY_SPLIT "
                f"stage={name} capture={index} host={host} guest={guest}")
        if not (unrelated[0] == 1520 and unrelated[1] == 660 and
                unrelated[2:] == (310, 240)):
            raise RuntimeError(
                "SD_CI_SOCIAL_STREAM_UNRELATED_MOVED "
                f"stage={name} capture={index} rect={unrelated}")
        x, y, w, h = host
        gx, gy, gw, gh = guest
        if (x < 0 or y < 0 or gx < x or gy < y or
                x+w > b.WIDTH or y+h > b.HEIGHT or
                min(w, h, gw, gh) < 200):
            raise RuntimeError("SD_CI_SOCIAL_STREAM_OUTSIDE_OUTPUT "+name)

        points = []
        def expect(area, px, py, rgb):
            points.append((area, px, py, rgb))

        # Same exact 38 meaningful samples as the preceding paused witness.
        for row in (y + 40, y + h//2, y + h - 41):
            for off in (4, 55, 109, 110):
                expect("rail", x+off, row, b.HOST_COLOR)
            for px in (gx+2, gx+gw//2, gx+gw-2):
                expect("guest", px, row, b.GUEST_COLOR)
            expect("right-border", x+w-1, row, b.HOST_COLOR)
        for px in (gx+30, gx+gw//2, gx+gw-30):
            expect("top-border", px, y, b.HOST_COLOR)
            expect("top-guest", px, gy+2, b.GUEST_COLOR)
            expect("bottom-guest", px, gy+gh-3, b.GUEST_COLOR)
            expect("bottom-border", px, y+h-1, b.HOST_COLOR)
        expect("unrelated", 1650, 770, b.OTHER_COLOR)
        expect("background", 25, 25, (0,0,0))

        for area, px, py, expected in points:
            observed = b.ppm_pixel(data, px, py)
            if max(abs(a-c) for a,c in zip(observed, expected)) > 25:
                raise RuntimeError(
                    "SD_CI_SOCIAL_STREAM_PIXEL_SPLIT "
                    f"stage={name} capture={index} area={area} "
                    f"point={px},{py} observed={observed} "
                    f"expected={expected} host={host} guest={guest}")
        if len(points) != 38:
            raise RuntimeError("SD_CI_SOCIAL_STREAM_SAMPLE_COUNT_DRIFT")
        return len(points)

    def _run(self):
        try:
            while not self.stop_flag.is_set():
                before = self._read_pair()
                path = self.root / (
                    f"stream-{self.name}-{self.total:04d}.ppm")
                started = time.monotonic_ns()
                try:
                    proc = subprocess.run(
                        [str(self.grim), "-t", "ppm", str(path)],
                        capture_output=True, timeout=8, check=False,
                        env=self.env)
                    if proc.returncode:
                        raise RuntimeError(
                            "SD_CI_SOCIAL_STREAM_SCREENSHOT_FAILED " +
                            proc.stderr.decode(errors="replace")[-600:])
                    data = path.read_bytes()
                finally:
                    path.unlink(missing_ok=True)
                after = self._read_pair()
                self.total += 1
                if before != after:
                    # No geometry epoch can be bound reliably to this PPM.
                    # Count the transition but never call it a visual pass.
                    self.transitions += 1
                    continue
                host, guest, other = after
                count = self._assert_pixels(
                    data, host, guest, other, self.name, self.total)
                self.stable += 1
                self.positions.add(host)
                elapsed_ms = (time.monotonic_ns()-started)/1_000_000
                print("SD_CI_SOCIAL_STREAM_FRAME "
                      f"stage={self.name} capture={self.total} "
                      f"stable=1 elapsed_ms={elapsed_ms:.1f} "
                      f"pixels={count} result=PASS",
                      flush=True)
        except Exception as exc:
            self.error = f"{type(exc).__name__}: {exc}"
            self.stop_flag.set()

    def start(self):
        if self.thread is not None:
            raise RuntimeError("SD_CI_SOCIAL_STREAM_ALREADY_STARTED")
        self.thread = threading.Thread(
            target=self._run, name="ci-social-pixel-stream-"+self.name,
            daemon=True)
        self.thread.start()

    def stop(self):
        if self.thread is None:
            raise RuntimeError("SD_CI_SOCIAL_STREAM_NOT_STARTED")
        self.stop_flag.set()
        self.thread.join(timeout=12)
        if self.thread.is_alive():
            raise RuntimeError(
                "SD_CI_SOCIAL_STREAM_UNJOINED_THREAD "+self.name)
        if self.error:
            raise RuntimeError(self.error)
        if (self.stable < self.min_frames or
                len(self.positions) < self.min_positions):
            raise RuntimeError(
                "SD_CI_SOCIAL_STREAM_UNDER_SAMPLED "
                f"stage={self.name} frames={self.total} "
                f"stable={self.stable} transitions={self.transitions} "
                f"positions={len(self.positions)} "
                f"required={self.min_frames}/{self.min_positions}")
        print("SD_CI_SOCIAL_STREAM_RESULT "
              f"stage={self.name} frames={self.total} "
              f"stable={self.stable} transitions={self.transitions} "
              f"distinct_poses={len(self.positions)} "
              "all_presentation_frames=NOT_OBSERVED result=PASS",
              flush=True)
