#!/usr/bin/env python3
"""M30t: CI-only visual fidelity reference overlay + Qt-native pixel capture.

Use immutable merged M30q Company source and M30o source-pinned MapLibre.
No production Qt QML source, compositor code, network, desktop or owner
record is modified. A real QQuickWindow::grabWindow() is compared against
a simultaneous compositor grim screenshot; both must retain straight
vertical and horizontal reference bars at the same coordinates.
"""
from __future__ import annotations
import os
from pathlib import Path
import sys

import ci_company_m30s_prepare as inherited


def once(text,before,after,why):
    if text.count(before)!=1 or after in text:
        raise RuntimeError("M30T_SOURCE_COPY_DRIFT_"+why)
    return text.replace(before,after,1)


def main():
    env=os.environ
    if not (env.get("GITHUB_ACTIONS")=="true"
            and env.get("CI")=="true"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M30T")=="1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M30S")=="1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M30O")=="1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M30N")=="1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M30L")=="1"
            and Path(env.get("RUNNER_TEMP","/missing")) ==
                Path("/home/runner/work/_temp")):
        raise RuntimeError("BLOCKED_M30T_SOURCE_OUTSIDE_DISPOSABLE_ACTIONS")
    inherited.main()
    out=Path(env["STAGE_DIRECTOR_M30L_CI_SOURCE"])
    qml=out/"main.qml"
    text=qml.read_text(encoding="utf-8")
    if not text.endswith("}\n") or text.count("m30oMapWidth")!=1:
        raise RuntimeError("M30T_NATIVE_QML_SOURCE_NOT_PINNED")
    # Source-local bars: two pure vertical rectangles + one horizontal.
    # They appear only at EXACT fullscreen Qt window dimensions, never in
    # standard/compact and never on the original operator's Qt source.
    # Bright green/magenta do not occur in the synthetic map fixture.
    probe="""    // M30t independent Qt Quick pixel-geometry fiducials, CI ONLY.
    Item {
        id: m30tPixelFiducials
        z: 10000
        width: root.width
        height: root.height
        visible: root.width === 1494 && root.height === 788
        Rectangle {
            x: 305; y: 170; width: 6; height: 361
            color: "#04FF32"
        }
        Rectangle {
            x: 462; y: 170; width: 6; height: 361
            color: "#04FF32"
        }
        Rectangle {
            x: 305; y: 525; width: 164; height: 6
            color: "#FF0DFF"
            z: 2
        }
    }
"""
    # M30z tests the *same pure Qt Rectangle bars* on the original 1493px
    # baseline as well as the corrected 1494px source. The prepared source
    # is confined to RUNNER_TEMP; the original Company Qt files stay intact.
    # Without M30z, M30t remains byte-for-byte equivalent to its prior CI.
    if env.get("STAGE_DIRECTOR_CI_COMPANY_M30Z") == "1":
        probe=once(probe,
            "visible: root.width === 1494 && root.height === 788",
            "visible: (root.width === 1494 || root.width === 1493) "
            "&& root.height === 788", "M30Z_TEST_ONLY_FIDUCIAL_WIDTH")
    qml.write_text(text[:-2]+probe+"}\n",encoding="utf-8")

    cpp=out/"main.cpp"
    s=cpp.read_text(encoding="utf-8")
    anchor="""            sizePoll->start();
        }
        // M30n observes Qt's real QML MapView camera after actual input."""
    injection="""            sizePoll->start();
        }
        // M30t: one actual QQuickWindow::grabWindow() at exact fullscreen
        // (not an IPC-faked successful resize). Captures Qt's pixels BEFORE
        // SwayPX crops or composites them; no readback from user desktop.
        if (qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30T") ==
                QStringLiteral("1")) {
            auto *fidelityPoll=new QTimer(&app);
            fidelityPoll->setInterval(150);
            auto settled=std::make_shared<int>(0);
            QObject::connect(fidelityPoll,&QTimer::timeout,&app,
                [window,m30lRoot,fidelityPoll,settled]() {
                    if (!m30lRoot->property("m30lMapReady").toBool()
                            || m30lRoot->property("m30oWindowWidth").toInt()!=1494
                            || m30lRoot->property("m30oWindowHeight").toInt()!=788
                            || m30lRoot->property("m30oMapWidth").toInt()!=1494
                            || m30lRoot->property("m30oMapHeight").toInt()!=788) {
                        *settled=0;
                        return;
                    }
                    if (++*settled<5) return;
                    fidelityPoll->stop();
                    const QImage pixels=window->grabWindow().convertToFormat(
                        QImage::Format_RGB888);
                    if (pixels.width()!=1494 || pixels.height()!=788) {
                        std::fprintf(stderr,
                            "SD_COMPANY_M30T_NATIVE_QT_GRAB_FAILED dimensions\\n");
                        std::fflush(stderr);
                        return;
                    }
                    const QString path=QStringLiteral(
                        "/home/runner/work/_temp/sd-company-m30t-native-fullscreen.ppm");
                    if (!pixels.save(path, "PPM")) {
                        std::fprintf(stderr,
                            "SD_COMPANY_M30T_NATIVE_QT_GRAB_FAILED write\\n");
                        std::fflush(stderr);
                        return;
                    }
                    std::fprintf(stdout,
                        "SD_COMPANY_M30T_NATIVE_QT_GRAB real_grabWindow=1 "
                        "size=1494x788 qml_fiducials=1 offline=1\\n");
                    std::fflush(stdout);
                });
            fidelityPoll->start();
        }
        // M30n observes Qt's real QML MapView camera after actual input."""
    s=once(s,anchor,injection,"CXX_WINDOW_PIXEL_READBACK")
    if env.get("STAGE_DIRECTOR_CI_COMPANY_M30Z") == "1":
        # M30z adds a *separate* original-width Qt CPU pixel witness,
        # gated by M30Z + M30Y only at runtime. The existing M30t 1494px
        # path remains unchanged; never call this outside disposable CI.
        original_grab="""        // M30z: capture unpadded original QQuickWindow source pixels BEFORE
        // the compositor, with the same fiducials used by M30t.
        if (qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30Z") ==
                QStringLiteral("1") &&
            qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30Y") ==
                QStringLiteral("1") &&
            qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30T") !=
                QStringLiteral("1")) {
            auto *originalPoll = new QTimer(&app);
            originalPoll->setInterval(150);
            auto originalSettled = std::make_shared<int>(0);
            QObject::connect(originalPoll, &QTimer::timeout, &app,
                [window,m30lRoot,originalPoll,originalSettled]() {
                    if (!m30lRoot->property("m30lMapReady").toBool()
                            || m30lRoot->property("m30oWindowWidth").toInt()!=1493
                            || m30lRoot->property("m30oWindowHeight").toInt()!=788
                            || m30lRoot->property("m30oMapWidth").toInt()!=1493
                            || m30lRoot->property("m30oMapHeight").toInt()!=788) {
                        *originalSettled=0;
                        return;
                    }
                    if (++*originalSettled<5) return;
                    originalPoll->stop();
                    const QImage pixels=window->grabWindow().convertToFormat(
                        QImage::Format_RGB888);
                    if (pixels.width()!=1493 || pixels.height()!=788) {
                        std::fprintf(stderr,
                            "M30Z_ORIGINAL_QT_SOURCE_DIMENSIONS_REFUSED\\n");
                        std::fflush(stderr);
                        return;
                    }
                    const QString path=QStringLiteral(
                        "/home/runner/work/_temp/sd-company-m30z-original-native-fullscreen.ppm");
                    if (!pixels.save(path,"PPM")) {
                        std::fprintf(stderr,
                            "M30Z_ORIGINAL_QT_SOURCE_WRITE_REFUSED\\n");
                        std::fflush(stderr);
                        return;
                    }
                    std::fprintf(stdout,
                        "SD_COMPANY_M30Z_ORIGINAL_QT_GRAB "
                        "real_grabWindow=1 size=1493x788 "
                        "qml_fiducials=1 offline=1 production=OFF\\n");
                    std::fflush(stdout);
                });
            originalPoll->start();
        }
"""
        s=once(s,
            "        // M30n observes Qt's real QML MapView camera after actual input.",
            original_grab+
            "        // M30n observes Qt's real QML MapView camera after actual input.",
            "M30Z_ORIGINAL_QT_GRAB")

    cpp.write_text(s,encoding="utf-8")
    print("SD_COMPANY_M30T_SOURCE_COPY original_merged_company_unchanged=1 "
          "real_maplibre_with_pure_Qt_Rectangle_fiducials=1 "
          "native_QQuickWindow_grabWindow_vs_sway_compositor=1 "
          "NO_OWNER_DATA=1 CI_ONLY=1 PASS",flush=True)


if __name__=="__main__":
    try:
        main()
    except Exception as ex:
        print("SD_COMPANY_M30T_SOURCE_PREP_REFUSED:",str(ex),
              file=sys.stderr,flush=True)
        raise SystemExit(1)
