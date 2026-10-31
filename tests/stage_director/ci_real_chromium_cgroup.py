#!/usr/bin/env python3
"""CI-only real Chromium cgroup v2 custody experiment. No operator desktop.

Uses one disposable Docker image, non-root Chromium, no network, read-only
root, private tmpfs profile, bounded resources, default container seccomp.
No --no-sandbox, --privileged, host PID/network, or unconfined seccomp.
Production WATCH never invokes this probe.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import time


def command(*args: str, timeout: int = 60) -> str:
    result = subprocess.run(args, text=True, capture_output=True,
                            timeout=timeout, check=False)
    if result.returncode:
        raise RuntimeError(
            f"command failed ({result.returncode}): {args[0:3]}\n"
            + result.stderr[-3500:] + "\n" + result.stdout[-1500:])
    return result.stdout.strip()


def starttime(pid: int) -> int | None:
    try:
        stat = Path(f"/proc/{pid}/stat").read_text()
        return int(stat.rsplit(")", 1)[1].split()[19])  # proc stat field 22
    except (FileNotFoundError, ProcessLookupError, PermissionError):
        return None


def members(group: Path) -> set[int]:
    if not group.is_dir():
        return set()
    ids: set[int] = set()
    for file in group.rglob("cgroup.procs"):
        try:
            ids.update(int(n) for n in file.read_text().split())
        except FileNotFoundError:
            pass
    return ids


def main() -> None:
    env = os.environ
    if env.get("CI") != "true" or env.get("GITHUB_ACTIONS") != "true":
        raise RuntimeError("BLOCKED_NON_CI_BROWSER_EXECUTION")
    temp = Path(env.get("RUNNER_TEMP", "/nonexistent"))
    if (not temp.is_dir() or not env.get("GITHUB_SHA") or
            not env.get("GITHUB_RUN_ID")):
        raise RuntimeError("BLOCKED_NO_DISPOSABLE_RUNNER_SCOPE")
    if not Path("/sys/fs/cgroup/cgroup.controllers").is_file():
        raise RuntimeError("BLOCKED_CGROUP_V2_UNAVAILABLE")
    if command("docker", "info", "--format", "{{.CgroupVersion}}") != "2":
        raise RuntimeError("BLOCKED_DOCKER_CGROUP_V2_UNAVAILABLE")

    image = "sd-ci-chromium:" + env["GITHUB_SHA"][:12]
    name = ("sd-ci-chromium-" + env["GITHUB_RUN_ID"] + "-" +
            env.get("GITHUB_RUN_ATTEMPT", "1"))
    cid: str | None = None
    group: Path | None = None
    try:
        print("SD_REAL_CHROMIUM_BUILD: nonroot=1 network=OFF "
              "sandbox_not_explicitly_disabled=1", flush=True)
        command("docker", "build", "--quiet", "--file",
                "tests/stage_director/ci_chromium_cgroup.Dockerfile",
                "--tag", image, ".", timeout=540)
        cid = command(
            "docker", "run", "--detach", "--init", "--name", name,
            "--network", "none", "--read-only", "--user", "10001:10001",
            "--cap-drop", "ALL", "--security-opt", "no-new-privileges",
            "--pids-limit", "128", "--memory", "1536m", "--cpus", "2",
            "--shm-size", "256m",
            "--tmpfs", "/tmp:rw,nosuid,nodev,mode=1777,size=512m",
            "--tmpfs", "/home/stage-watch:rw,nosuid,nodev,uid=10001,"
                       "gid=10001,mode=700,size=32m",
            image, timeout=80)
        if len(cid) != 64 or not all(c in "0123456789abcdef" for c in cid):
            raise RuntimeError("BLOCKED_UNEXPECTED_CONTAINER_ID")
        inspect = json.loads(command("docker", "inspect", cid))[0]
        host = inspect["HostConfig"]
        pid = int(inspect["State"]["Pid"])
        if (pid <= 1 or host["NetworkMode"] != "none" or
                not host["ReadonlyRootfs"] or host["Privileged"] or
                host["PidsLimit"] != 128 or
                host["CapDrop"] != ["ALL"]):
            raise RuntimeError("BLOCKED_CONTAINER_POLICY_MISMATCH")
        cgroup_line = [line for line in
                       Path(f"/proc/{pid}/cgroup").read_text().splitlines()
                       if line.startswith("0::")]
        if len(cgroup_line) != 1:
            raise RuntimeError("BLOCKED_NO_UNIQUE_HOST_CGROUP")
        relative = cgroup_line[0][3:]
        if (relative == "/" or not relative.startswith("/") or
                ".." in relative or cid[:12] not in relative):
            raise RuntimeError("BLOCKED_CONTAINER_CGROUP_NOT_DISTINCT: "
                               + relative)
        group = Path("/sys/fs/cgroup" + relative)
        if not (group / "cgroup.procs").is_file():
            raise RuntimeError("BLOCKED_CGROUP_NOT_OBSERVABLE")
        sample: dict[int, int] = {}
        browser_count = 0
        for _ in range(120):
            inspect = json.loads(command("docker", "inspect", cid))[0]
            if not inspect["State"]["Running"]:
                # Docker logs may be exclusively on stderr. Do not mask
                # Chromium namespace/sandbox errors with stdout-only capture.
                diagnostic = subprocess.run(
                    ["docker", "logs", "--tail", "100", cid],
                    text=True, capture_output=True, timeout=20, check=False)
                state = inspect["State"]
                raise RuntimeError(
                    "BLOCKED_CHROMIUM_EXITED_EARLY "
                    f"exit={state.get('ExitCode')} "
                    f"oom={state.get('OOMKilled')} "
                    f"docker_error={state.get('Error')} "
                    "stderr=" + diagnostic.stderr[-3500:] +
                    " stdout=" + diagnostic.stdout[-1000:])
            pids = members(group)
            browser_count = sum(
                1 for p in pids if Path(f"/proc/{p}/comm").exists() and
                Path(f"/proc/{p}/comm").read_text().strip() in
                ("chromium", "chrome"))
            sample = {p: st for p in pids if (st := starttime(p))}
            if browser_count >= 3 and len(sample) >= 4:
                break
            time.sleep(0.25)
        else:
            raise RuntimeError("BLOCKED_NO_MULTIPROCESS_CHROMIUM "
                               f"members={len(sample)} chromium={browser_count}")
        command("docker", "exec", cid, "sh", "-c",
                "test \"$(stat -c %u /tmp/stage-director-browser-profile)\""
                " = 10001 && test -d /tmp/stage-director-browser-profile")
        print(f"SD_REAL_CHROMIUM_LIVE_CGROUP: pids={len(sample)} "
              f"chromium_processes={browser_count} private_profile=1 "
              "network=OFF authority=NONE PASS", flush=True)
        command("docker", "stop", "--time", "3", cid, timeout=30)
        after = json.loads(command("docker", "inspect", cid))[0]
        if after["State"]["Running"]:
            raise RuntimeError("FAIL_STOP_CONTAINER_STILL_RUNNING")
        survivors = [p for p, started in sample.items()
                     if starttime(p) == started]
        leftover = members(group)
        if survivors or leftover:
            raise RuntimeError("FAIL_CGROUP_DESCENDANTS_SURVIVED: "
                               f"survivors={survivors} members={leftover}")
        print(f"SD_REAL_CHROMIUM_CGROUP_TEARDOWN: "
              f"original_pids={len(sample)} matching_survivors=0 "
              "cgroup_members=0 isolated_profile_discarded=1 PASS",
              flush=True)
        command("docker", "rm", cid)
        cid = None
        print("SD_REAL_CHROMIUM_CUSTODY: real_browser=1 cgroup_v2=1 "
              "subprocesses=observed browser_sandbox_not_disabled=1 "
              "session_cleaned=1 production_watch=OFF scene=OFF PASS",
              flush=True)
    finally:
        if cid:
            try:
                print("SD_REAL_CHROMIUM_TEARDOWN: forced exact cleanup",
                      flush=True)
                command("docker", "rm", "-f", cid, timeout=60)
            except Exception as exc:
                print(f"TEARDOWN_ERROR: {exc}", file=sys.stderr)
        try:
            command("docker", "image", "rm", "-f", image, timeout=30)
        except Exception:
            pass


if __name__ == "__main__":
    try:
        main()
    except Exception as err:
        print(f"SD_REAL_CHROMIUM_BLOCKED: {err}", file=sys.stderr)
        raise SystemExit(1)
