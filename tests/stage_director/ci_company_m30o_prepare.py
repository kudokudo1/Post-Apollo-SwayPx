#!/usr/bin/env python3
"""M30o disposable original Company MapLibre Qt source resize observer.

Reuses the M30n camera witness and exact original Company source SHA.
This extends ONLY a CI private copy, does not set window size, camera,
location, network permissions, selected production host or main SwayPX.
"""
from __future__ import annotations
import os
from pathlib import Path
import ci_company_m30n_prepare as inherited


def exactly_once(s, original, replacement, field):
    if s.count(original) != 1:
        raise RuntimeError("M30O_EXACT_SOURCE_DRIFT_" + field)
    return s.replace(original, replacement, 1)


def main():
    env = os.environ
    if not (env.get("CI") == "true" and
            env.get("GITHUB_ACTIONS") == "true" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30O") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30N") == "1" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30L") == "1" and
            Path(env.get("RUNNER_TEMP", "/unavailable")) ==
                Path("/home/runner/work/_temp")):
        raise RuntimeError("BLOCKED_M30O_NOT_EXACT_DISPOSABLE_RUNNER")
    inherited.main()
    out = Path(env["STAGE_DIRECTOR_M30L_CI_SOURCE"])
    qml = out / "main.qml"
    q = qml.read_text(encoding="utf-8")
    if not q.endswith("}\n") or q.count("m30nCameraZoom") != 1:
        raise RuntimeError("M30O_CAMERA_READY_QML_DRIFT")
    qml.write_text(q[:-2] + """    // Temporary read-only observation of genuine MapLibre Qt geometry.
    readonly property int m30oWindowWidth: root.width
    readonly property int m30oWindowHeight: root.height
    readonly property int m30oMapWidth: mapView.width
    readonly property int m30oMapHeight: mapView.height
}
""", encoding="utf-8")

    cpp = out / "main.cpp"
    s = cpp.read_text(encoding="utf-8")
    target = """        m30lTimer->start();
        // M30n observes Qt's real QML MapView camera after actual input."""
    replacement = """        m30lTimer->start();
        // M30o: observe actual QML root and MapView sizes via Qt property
        // reads, never simulate resize success from a Sway IPC reply.
        if (qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30O") ==
                QStringLiteral("1")) {
            auto *sizePoll = new QTimer(&app);
            sizePoll->setInterval(150);
            auto last = std::make_shared<QVector<int>>();
            QObject::connect(sizePoll, &QTimer::timeout, &app,
                [m30lRoot,last]() {
                    const QVariant wv=m30lRoot->property("m30oWindowWidth");
                    const QVariant hv=m30lRoot->property("m30oWindowHeight");
                    const QVariant mwv=m30lRoot->property("m30oMapWidth");
                    const QVariant mhv=m30lRoot->property("m30oMapHeight");
                    bool okW=false,okH=false,okMW=false,okMH=false;
                    const int w=wv.toInt(&okW),h=hv.toInt(&okH);
                    const int mw=mwv.toInt(&okMW),mh=mhv.toInt(&okMH);
                    if (!okW || !okH || !okMW || !okMH ||
                            w < 100 || w > 2000 || h < 100 || h > 1200 ||
                            mw != w || mh != h) return;
                    if (last->size()==4 && last->at(0)==w &&
                            last->at(1)==h && last->at(2)==mw &&
                            last->at(3)==mh) return;
                    last->clear();
                    last->append(w);last->append(h);
                    last->append(mw);last->append(mh);
                    std::fprintf(stdout,
                        "SD_COMPANY_M30O_REAL_QML_SOURCE "
                        "window=%dx%d map=%dx%d\\n",w,h,mw,mh);
                    std::fflush(stdout);
                });
            sizePoll->start();
        }
        // M30n observes Qt's real QML MapView camera after actual input."""
    s=exactly_once(s,target,replacement,"SIZE_PROPERTY_POLL")
    cpp.write_text(s, encoding="utf-8")
    print("SD_COMPANY_M30O_SOURCE_COPY original_company_unmodified=1 "
          "original_camera_observer_kept=1 qt_mapview_size_observed=1 "
          "no_size_forcing=1 no_owner_data=1 no_network=1 PASS",
          flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30O_PREPARE_BLOCKED:",str(exc),flush=True)
        raise SystemExit(1)
