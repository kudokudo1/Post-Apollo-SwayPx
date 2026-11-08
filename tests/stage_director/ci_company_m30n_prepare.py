#!/usr/bin/env python3
"""M30n CI-only camera witness on top of M30l's exact Company source copy.

This never modifies the original Company repository or production renderer.
It reads MapLibre QtLocation camera state from the actual Qt QML window
after genuine Wayland gestures. Everything runs in disposable Actions CI.
"""
from __future__ import annotations
import os
from pathlib import Path
import ci_company_m30l_prepare as original


def exactly_once(text: str, anchor: str, replacement: str, label: str) -> str:
    if text.count(anchor) != 1:
        raise RuntimeError("M30N_SOURCE_INSTRUMENTATION_DRIFT_" + label)
    return text.replace(anchor, replacement, 1)


def main() -> None:
    env = os.environ
    if not (env.get("CI") == "true" and
            env.get("GITHUB_ACTIONS") == "true" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30N") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30L") == "1" and
            Path(env.get("RUNNER_TEMP", "/missing")) ==
                Path("/home/runner/work/_temp")):
        raise RuntimeError("BLOCKED_M30N_NOT_EXACT_DISPOSABLE_CI")
    # Original helper enforces exact clean Company SHA, non-root, output
    # beneath RUNNER_TEMP, and no original checkout modifications.
    original.main()
    src = Path(env["STAGE_DIRECTOR_M30L_CI_SOURCE"])
    qml = src / "main.qml"
    script = qml.read_text(encoding="utf-8")
    suffix = """    // M30n: read only the *actual* QtLocation MapView camera.
    // This source is a disposable CI COPY, never a production edit.
    readonly property real m30nCameraLat: mapView.map.center.latitude
    readonly property real m30nCameraLon: mapView.map.center.longitude
    readonly property real m30nCameraZoom: mapView.map.zoomLevel
"""
    if not script.endswith("}\n"):
        raise RuntimeError("M30N_QML_ROOT_END_DRIFT")
    if script.count("readonly property bool m30lMapReady:") != 1:
        raise RuntimeError("M30N_ORIGINAL_MAPLIBRE_MAP_READY_BINDING_MISSING")
    qml.write_text(script[:-2] + suffix + "}\n", encoding="utf-8")

    cpp = src / "main.cpp"
    code = cpp.read_text(encoding="utf-8")
    marker = "        m30lTimer->start();"
    camera = """        m30lTimer->start();
        // M30n observes Qt's real QML MapView camera after actual input.
        // Never synthesizes a camera value, repositions the map, reads
        // user records, or disables the offline/renderer guards above.
        if (qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30N") ==
                QStringLiteral("1")) {
            auto *m30nCameraPoll = new QTimer(&app);
            m30nCameraPoll->setInterval(125);
            auto prev = std::make_shared<QVector<double>>();
            QObject::connect(m30nCameraPoll, &QTimer::timeout, &app,
                [m30lRoot, prev]() {
                    const QVariant latitude =
                        m30lRoot->property("m30nCameraLat");
                    const QVariant longitude =
                        m30lRoot->property("m30nCameraLon");
                    const QVariant zoom =
                        m30lRoot->property("m30nCameraZoom");
                    bool latOk = false, lonOk = false, zoomOk = false;
                    const double lat = latitude.toDouble(&latOk);
                    const double lon = longitude.toDouble(&lonOk);
                    const double level = zoom.toDouble(&zoomOk);
                    if (!latOk || !lonOk || !zoomOk ||
                            !std::isfinite(lat) || !std::isfinite(lon) ||
                            !std::isfinite(level) || std::abs(lat) > 85.06 ||
                            std::abs(lon) > 180 || level < 0 || level > 24)
                        return;
                    if (prev->size() == 3 &&
                            std::abs(prev->at(0)-lat) < 0.0000001 &&
                            std::abs(prev->at(1)-lon) < 0.0000001 &&
                            std::abs(prev->at(2)-level) < 0.0001)
                        return;
                    prev->clear();
                    prev->append(lat);
                    prev->append(lon);
                    prev->append(level);
                    std::fprintf(stdout,
                        "SD_COMPANY_M30N_REAL_CAMERA "
                        "latitude=%.9f longitude=%.9f zoom=%.6f\\n",
                        lat, lon, level);
                    std::fflush(stdout);
                });
            m30nCameraPoll->start();
        }"""
    code = exactly_once(code, marker, camera, "QT_CAMERA_POLL")
    cpp.write_text(code, encoding="utf-8")
    print("SD_COMPANY_M30N_CI_SOURCE "
          "original_company_pin=" + original.PIN + " "
          "original_checkout_unchanged=1 "
          "m30l_offline_provenance=UNCHANGED "
          "m30n_real_qml_camera_readonly=1 "
          "no_camera_rewrite=1 no_owner_data=1 PASS", flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30N_BLOCKED:", str(exc), flush=True)
        raise SystemExit(1)
