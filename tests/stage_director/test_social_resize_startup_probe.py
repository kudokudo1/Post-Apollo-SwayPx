#!/usr/bin/env python3
"""Offline smoke test for the isolated production SwayPX startup probe."""
import os
from pathlib import Path
import subprocess
import tempfile

PROBE = Path(__file__).resolve().parents[2] / "OPERATE/social-resize-startup-probe.sh"


def run_case(label: str, old_log: bool, outcome: int) -> None:
    with tempfile.TemporaryDirectory(prefix="social-resize-probe-test-") as temp:
        root = Path(temp)
        stage = root / "stage"
        bindir = stage / "bin"
        bindir.mkdir(parents=True)
        probe = bindir / "swaypx-social-resize-probe"
        probe.write_bytes(PROBE.read_bytes())
        probe.chmod(0o755)
        binary = bindir / "sway"
        binary.write_text(
            "#!/bin/sh\n"
            "if [ \"${1:-}\" = --version ]; then echo 'sway 0.6-1cbca16'; exit 0; fi\n"
            "[ \"${1:-}\" = -d ] || exit 90\n"
            "printf 'mock compositor launched\\n'\n"
            "if [ \"${MOCK_WRITE_LOG:-0}\" = 1 ]; then\n"
            "  printf 'INFO: Launching config: shell.qml\\n' > \"$SOCIAL_RESIZE_PROBE_QUICKSHELL_LOG\"\n"
            "fi\n"
            "sleep 2\n"
            "exit \"$MOCK_RESULT\"\n"
        )
        binary.chmod(0o755)
        state = root / "state"
        qs = root / "quickshell.log"
        if old_log:
            qs.write_text("STALE PREVIOUS SESSION\n")
            os.utime(qs, (946684800, 946684800))
        env = os.environ.copy()
        env.update({
            "XDG_STATE_HOME": str(state),
            "SOCIAL_RESIZE_PROBE_QUICKSHELL_LOG": str(qs),
            "MOCK_WRITE_LOG": "0" if old_log else "1",
            "MOCK_RESULT": str(outcome),
        })
        process = subprocess.run([str(probe)], env=env, timeout=12, capture_output=True, text=True)
        assert process.returncode == outcome, (label, process.returncode, process.stderr)
        latest_file = state / "swaypx/social-resize-startup/LATEST"
        assert "social-resize-probe: entered launcher" in process.stderr, label
        assert latest_file.is_file(), label
        run = Path(latest_file.read_text().strip())
        assert run.is_dir(), label
        assert oct(run.stat().st_mode & 0o777) == "0o700"
        launch = (run / "launch.log").read_text()
        for phase in ("invoked", "preflight_ok", "compositor_spawned", "compositor_exited"):
            assert "phase=" + phase in launch, (label, phase, launch)
        assert "sway 0.6-1cbca16" in (run / "metadata.log").read_text()
        assert "mock compositor launched" in (run / "sway.log").read_text()
        assert "exit=" + str(outcome) in (run / "exit.log").read_text()
        health = (run / "health.log").read_text()
        assert "quickshell" in health, (label, health)
        if old_log:
            assert not (run / "quickshell.log").exists(), label
            assert "quickshell_log=older_than_probe" in health, health
        else:
            assert "Launching config: shell.qml" in (run / "quickshell.log").read_text(), label
    print(f"PASS {label}")


def run_missing_compositor_case() -> None:
    with tempfile.TemporaryDirectory(prefix="social-resize-probe-missing-stage-") as temp:
        root = Path(temp)
        bindir = root / "stage/bin"
        bindir.mkdir(parents=True)
        probe = bindir / "swaypx-social-resize-probe"
        probe.write_bytes(PROBE.read_bytes())
        probe.chmod(0o755)
        state = root / "state"
        env = os.environ.copy()
        env["XDG_STATE_HOME"] = str(state)
        process = subprocess.run([str(probe)], env=env, timeout=8, capture_output=True, text=True)
        assert process.returncode == 127, process.stderr
        assert "social-resize-probe: entered launcher" in process.stderr
        assert "compositor not executable" in process.stderr
        latest = state / "swaypx/social-resize-startup/LATEST"
        assert latest.is_file(), process.stderr
        run = Path(latest.read_text().strip())
        assert oct(run.stat().st_mode & 0o777) == "0o700"
        launch = (run / "launch.log").read_text()
        assert "phase=invoked" in launch
        assert "phase=compositor_not_executable" in launch
        assert "phase=compositor_spawned" not in launch
        assert not (run / "sway.log").exists()
    print("PASS missing-compositor-preserves-preflight-evidence")


if __name__ == "__main__":
    run_case("successful-compositor-and-current-quickshell", old_log=False, outcome=0)
    run_case("nonzero-exit-and-stale-log-rejected", old_log=True, outcome=42)
    run_missing_compositor_case()
