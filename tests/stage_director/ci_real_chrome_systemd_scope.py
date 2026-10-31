#!/usr/bin/env python3
"""CI-only real Chrome sandbox + systemd cgroup v2 lifecycle probe.

Alternative to Docker's blocked Chromium user-namespace sandbox, NOT a
production browser launcher. Only on disposable GitHub-hosted Ubuntu.
Uses existing /opt/google/chrome/chrome + its installed AppArmor policy.
Fixed about:blank URL and ephemeral /run RuntimeDirectory. No browser
sandbox bypass, no workstation commands, no production WATCH connection.
"""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import time


def run(*argv: str, timeout: int = 30, check: bool = True) -> str:
    p = subprocess.run(argv, text=True, capture_output=True,
                       timeout=timeout, check=False)
    if check and p.returncode:
        raise RuntimeError(f"{argv[0:3]} rc={p.returncode}\n"
                           + p.stdout[-1200:] + p.stderr[-2800:])
    return p.stdout.strip()


def stat_start(pid: int) -> int | None:
    try:
        content = Path(f"/proc/{pid}/stat").read_text()
        return int(content.rsplit(")", 1)[1].split()[19])
    except (FileNotFoundError, ProcessLookupError, PermissionError):
        return None


def ids(root: Path) -> set[int]:
    if not root.is_dir():
        return set()
    result = set()
    for path in root.rglob("cgroup.procs"):
        try:
            result.update(int(p) for p in path.read_text().split())
        except FileNotFoundError:
            pass
    return result


def main() -> None:
    env = os.environ
    if (env.get("CI") != "true" or env.get("GITHUB_ACTIONS") != "true"
            or not env.get("GITHUB_RUN_ID")
            or not Path("/sys/fs/cgroup/cgroup.controllers").exists()
            or not Path("/opt/google/chrome/chrome").is_file()
            or not Path("/run/systemd/system").is_dir()):
        raise RuntimeError("BLOCKED_UNSUPPORTED_DISPOSABLE_SYSTEMD_CHROME")
    if run("id", "-un") != "runner":
        raise RuntimeError("BLOCKED_NONSTANDARD_GITHUB_RUNNER_IDENTITY")
    unit = "sd-ci-weather-chrome-" + env["GITHUB_RUN_ID"] + ".service"
    runtime_name = "sd-ci-weather-chrome-" + env["GITHUB_RUN_ID"]
    runtime = Path("/run") / runtime_name
    browser = "/opt/google/chrome/chrome"
    cgroup: Path | None = None
    started = False
    try:
        print("SD_CI_CHROME_SYSTEMD: real_browser=1 AppArmor=installed "
              "network=private root=read_only home=hidden "
              "sandbox_bypass=NONE", flush=True)
        run("sudo", "systemd-run",
            "--unit=" + unit,
            "--property=User=runner",
            "--property=Group=runner",
            "--property=RuntimeDirectory=" + runtime_name,
            "--property=RuntimeDirectoryMode=0700",
            "--property=PrivateNetwork=yes",
            "--property=ProtectSystem=strict",
            "--property=ProtectHome=yes",
            "--property=PrivateTmp=yes",
            "--property=NoNewPrivileges=yes",
            "--property=IPAddressDeny=any",
            "--property=MemoryMax=1536M",
            "--property=TasksMax=128",
            "--property=KillMode=control-group",
            "--property=SendSIGKILL=yes",
            "--property=TimeoutStopSec=3s",
            "--setenv=HOME=" + str(runtime),
            "--setenv=XDG_CONFIG_HOME=" + str(runtime),
            "--setenv=XDG_CACHE_HOME=" + str(runtime),
            browser,
            "--headless=new",
            "--disable-gpu",
            "--disable-background-networking",
            "--disable-component-update",
            "--disable-extensions",
            "--disable-sync",
            "--no-first-run",
            "--no-default-browser-check",
            "--user-data-dir=" + str(runtime / "profile"),
            "about:blank")
        started = True
        relative = run("systemctl", "show", unit,
                       "--property=ControlGroup", "--value")
        if (not relative.startswith("/") or ".." in relative or
                runtime_name not in relative or relative == "/"):
            raise RuntimeError("BLOCKED_NO_DISTINCT_SYSTEMD_CGROUP: "
                               + relative)
        cgroup = Path("/sys/fs/cgroup" + relative)
        if not (cgroup / "cgroup.procs").is_file():
            raise RuntimeError("BLOCKED_SYSTEMD_CGROUP_NOT_VISIBLE")
        sample: dict[int, int] = {}
        chrome_count = 0
        for _ in range(100):
            active = run("systemctl", "is-active", unit, check=False)
            if active != "active":
                diag = run("sudo", "journalctl", "-u", unit,
                           "--no-pager", "-n", "75", check=False)
                raise RuntimeError("BLOCKED_CHROME_SYSTEMD_EXITED "
                                   f"state={active}\n" + diag[-3500:])
            members = ids(cgroup)
            chrome_count = 0
            for pid in members:
                comm = Path(f"/proc/{pid}/comm")
                try:
                    if comm.read_text().strip() in ("chrome", "chromium"):
                        chrome_count += 1
                except (FileNotFoundError, ProcessLookupError):
                    pass
            sample = {p: s for p in members if (s := stat_start(p))}
            if chrome_count >= 3 and len(sample) >= 3:
                break
            time.sleep(0.25)
        else:
            raise RuntimeError("BLOCKED_NO_REAL_CHROME_MULTIPROCESS "
                               f"pids={len(sample)} chrome={chrome_count}")
        if not (runtime / "profile").is_dir():
            raise RuntimeError("BLOCKED_PRIVATE_PROFILE_UNAVAILABLE")
        if (runtime / "profile").stat().st_uid != os.getuid():
            raise RuntimeError("BLOCKED_PROFILE_NOT_RUNNER_OWNED")
        print(f"SD_CI_CHROME_SYSTEMD_LIVE: processes={len(sample)} "
              f"chrome={chrome_count} unique_cgroup=1 "
              "temp_profile=1 network=PRIVATE PASS", flush=True)
        run("sudo", "systemctl", "stop", unit)
        for _ in range(60):
            survivors = [p for p, st in sample.items()
                         if stat_start(p) == st]
            current = ids(cgroup)
            if not survivors and not current and not runtime.exists():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("FAIL_SYSTEMD_CHROME_CGROUP_TEARDOWN "
                               f"survivors={survivors} members={current} "
                               f"runtime_directory_exists={runtime.exists()}")
        print(f"SD_CI_CHROME_SYSTEMD_TEARDOWN: captured_pids={len(sample)} "
              "matching_survivors=0 cgroup_members=0 private_profile_removed=1 "
              "real_browser=1 production_browser=OFF PASS", flush=True)
    finally:
        if started:
            # systemctl scopes are exact-run-id only. Never kill by name.
            run("sudo", "systemctl", "stop", unit, check=False)
            run("sudo", "systemctl", "reset-failed", unit, check=False)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_CI_CHROME_SYSTEMD_BLOCKED: " + str(exc),
              file=sys.stderr)
        raise SystemExit(1)
