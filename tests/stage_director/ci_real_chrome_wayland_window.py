#!/usr/bin/env python3
"""CI ONLY: map a REAL Chrome xdg_toplevel in isolated headless SwayPX.

Use the EXISTING host Chrome (never --no-sandbox), fixed about:blank,
transient systemd control-group. Compare the Sway-native mapped Wayland
client PID to the independently held Linux cgroup and kernel starttime.
No production admission, scene mutation, browser clip, focus or QML.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import time


def call(*args: str, check: bool = True, timeout: int = 20) -> str:
    out = subprocess.run(args, capture_output=True, text=True,
                         timeout=timeout, check=False)
    if check and out.returncode:
        raise RuntimeError(f"command {args[:3]} rc={out.returncode}\n"
                           + out.stderr[-2500:] + out.stdout[-1000:])
    return out.stdout.strip()


def proc_start(pid: int) -> int | None:
    try:
        return int(Path(f"/proc/{pid}/stat").read_text()
                   .rsplit(")", 1)[1].split()[19])
    except (OSError, ValueError, IndexError):
        return None


def members(root: Path) -> set[int]:
    pids: set[int] = set()
    if root.is_dir():
        for f in root.rglob("cgroup.procs"):
            try:
                pids.update(int(x) for x in f.read_text().split())
            except (OSError, ValueError):
                pass
    return pids


def containers(node):
    if isinstance(node, dict):
        if node.get("pid") and node.get("type") == "con":
            yield node
        for sub in node.get("nodes", []):
            yield from containers(sub)
        for sub in node.get("floating_nodes", []):
            yield from containers(sub)


def main() -> None:
    e = os.environ
    runtime = Path(e.get("XDG_RUNTIME_DIR", "/missing"))
    temp = Path(e.get("RUNNER_TEMP", "/not-runner"))
    swaymsg = e.get("STAGE_DIRECTOR_TEST_SWAYMSG", "")
    if (e.get("GITHUB_ACTIONS") != "true" or e.get("CI") != "true" or
            not str(runtime).startswith(str(temp) + "/") or
            not runtime.is_dir() or not swaymsg.startswith("/nix/store/") or
            not Path(swaymsg).is_file() or
            not Path("/run/systemd/system").is_dir() or
            not Path("/opt/google/chrome/chrome").is_file() or
            not Path("/sys/fs/cgroup/cgroup.controllers").is_file() or
            not e.get("WAYLAND_DISPLAY", "").startswith("wayland-")):
        raise RuntimeError("BLOCKED_OUTSIDE_ISOLATED_WAYLAND_GITHUB_CI")
    # Existing headless harness created the socket under this exact runtime.
    sock = runtime / e["WAYLAND_DISPLAY"]
    if not sock.is_socket():
        raise RuntimeError("BLOCKED_NO_ISOLATED_WAYLAND_SOCKET")
    if call("id", "-un") != "runner":
        raise RuntimeError("BLOCKED_NOT_RUNNER_USER")
    ipc_sockets = [p for p in runtime.glob("sway-ipc.*.sock")
                   if p.is_socket()]
    if len(ipc_sockets) != 1:
        raise RuntimeError("BLOCKED_NO_UNIQUE_PRIVATE_SWAY_IPC")
    ipc = str(ipc_sockets[0])
    run_id = e["GITHUB_RUN_ID"] + "-" + e.get("GITHUB_RUN_ATTEMPT", "1")
    unit = "sd-ci-chrome-wayland-" + run_id + ".service"
    rt_name = "sd-ci-chrome-wayland-" + run_id
    profile = Path("/run") / rt_name / "profile"
    uid = os.getuid()
    started = False
    root: Path | None = None
    snapshot: dict[int, int] = {}
    try:
        # The runner's Chrome has its own distro/AppArmor policy (whether
        # the internal sandbox is ACTIVE remains unverified). The service
        # never requests a browser-sandbox bypass or outside network.
        call("sudo", "-n", "systemd-run", "--unit=" + unit,
             "--property=User=runner", "--property=Group=runner",
             "--property=RuntimeDirectory=" + rt_name,
             "--property=RuntimeDirectoryMode=0700",
             "--property=PrivateNetwork=yes",
             "--property=ProtectSystem=strict",
             # The headless socket lives in runner HOME. Read-only allows
             # traversal/socket connection but not writing inside HOME.
             "--property=ProtectHome=read-only",
             "--property=PrivateTmp=yes",
             "--property=NoNewPrivileges=yes",
             "--property=IPAddressDeny=any",
             "--property=MemoryMax=1536M",
             "--property=TasksMax=128",
             "--property=KillMode=control-group",
             "--property=SendSIGKILL=yes",
             "--property=TimeoutStopSec=3s",
             "--setenv=HOME=" + str(profile.parent),
             "--setenv=XDG_CONFIG_HOME=" + str(profile.parent),
             "--setenv=XDG_CACHE_HOME=" + str(profile.parent),
             "--setenv=XDG_RUNTIME_DIR=" + str(runtime),
             "--setenv=WAYLAND_DISPLAY=" + e["WAYLAND_DISPLAY"],
             "--setenv=XDG_SESSION_TYPE=wayland",
             "/opt/google/chrome/chrome",
             "--ozone-platform=wayland",
             "--disable-gpu",
             "--disable-background-networking",
             "--disable-component-update",
             "--disable-extensions", "--disable-sync",
             "--no-first-run", "--no-default-browser-check",
             "--user-data-dir=" + str(profile),
             "--window-size=1120,720",
             "--app=about:blank", timeout=30)
        started = True
        relative = call("systemctl", "show", unit,
                        "--property=ControlGroup", "--value")
        if (not relative.startswith("/") or ".." in relative or
                rt_name not in relative or relative == "/"):
            raise RuntimeError("BLOCKED_MISSING_EXACT_SERVICE_CGROUP")
        root = Path("/sys/fs/cgroup" + relative)
        if not (root / "cgroup.procs").is_file():
            raise RuntimeError("BLOCKED_CGROUP_NOT_VISIBLE")

        actor = None
        count = 0
        for _ in range(80):
            if call("systemctl", "is-active", unit, check=False) != "active":
                logs = call("sudo", "-n", "journalctl", "-u", unit,
                            "-n", "75", "--no-pager", check=False)
                raise RuntimeError("BLOCKED_CHROME_SERVICE_EXITED\n"
                                   + logs[-3700:])
            pids = members(root)
            count = sum(1 for p in pids if (
                Path(f"/proc/{p}/comm").exists() and
                Path(f"/proc/{p}/comm").read_text().strip() == "chrome"))
            snapshot.update((p, st) for p in pids
                            if (st := proc_start(p)) is not None)
            tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            matches = []
            for node in containers(tree):
                pid = node["pid"]
                rect = node.get("rect") or {}
                if (pid in pids and proc_start(pid) == snapshot.get(pid) and
                        node.get("shell") == "xdg_shell" and
                        rect.get("width", 0) >= 300 and
                        rect.get("height", 0) >= 200):
                    matches.append(node)
            if count >= 3 and len(matches) == 1:
                actor = matches[0]
                break
            time.sleep(0.15)
        if actor is None:
            logs = call("sudo", "-n", "journalctl", "-u", unit,
                        "-n", "65", "--no-pager", check=False)
            raise RuntimeError("BLOCKED_NO_UNIQUE_LIVE_NATIVE_CHROME_TOPLEVEL "
                               f"chrome={count} group_pids={len(snapshot)}\n"
                               + logs[-3500:])
        client_pid = actor["pid"]
        # A mapped XDG root is actual native Sway provenance, not title or
        # advertised app_id; matching kernel starttime and exact service cgroup
        # gives a READ-ONLY candidate, not authorized browser/viewer custody.
        print(f"SD_CI_REAL_CHROME_WAYLAND_WINDOW: native_xdg=1 "
              f"mapped=1 actor_in_exact_service_cgroup=1 "
              f"kernel_starttime=1 browser_processes={count} "
              "authority=NONE PASS", flush=True)
        call("sudo", "-n", "systemctl", "stop", unit)
        for _ in range(100):
            survivors = [p for p, st in snapshot.items() if proc_start(p) == st]
            current = members(root)
            if not survivors and not current and not profile.parent.exists():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError(f"FAIL_BROWSER_WAYLAND_CGROUP_TEARDOWN "
                               f"survivors={survivors} members={current} "
                               f"profile_runtime_exists={profile.parent.exists()}")
        after = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        if any(n.get("pid") == client_pid for n in containers(after)):
            raise RuntimeError("FAIL_STALE_BROWSER_XDG_AFTER_STOP")
        print("SD_CI_REAL_CHROME_WAYLAND_TEARDOWN: original_pids_gone=1 "
              "native_xdg_unmapped=1 cgroup_members=0 private_profile_gone=1 "
              "watch=OFF scene_clip=OFF input=OFF PASS", flush=True)
    finally:
        if started:
            call("sudo", "-n", "systemctl", "stop", unit, check=False)
            call("sudo", "-n", "systemctl", "reset-failed",
                 unit, check=False)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"SD_CI_REAL_CHROME_WAYLAND_BLOCKED: {exc}",
              file=sys.stderr)
        raise SystemExit(1)
