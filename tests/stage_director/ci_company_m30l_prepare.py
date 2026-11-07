#!/usr/bin/env python3
"""Prepare ONLY a disposable exact-pinned Company MapLibre native Qt CI copy.

No online style, private records, production QML or user-local writes.
Do not use a source checkout that is dirty or differs from the M30e pin.
"""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess
import sys

PIN = "e3595b0035bb3d89aa87c6ba2bd1aac04a975181"


def replace_exact(text: str, before: str, after: str, what: str):
    if text.count(before) != 1 or after in text:
        raise RuntimeError("M30L_SOURCE_CONTRACT_DRIFT_" + what)
    return text.replace(before, after, 1)


SPY = r'''
#include <QMouseEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QObject>
#include <cstdio>
#include <unistd.h>
class M30lQtInputWitness final : public QObject {
public:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (!qobject_cast<QQuickWindow *>(obj))
            return QObject::eventFilter(obj, event);
        const char *kind = nullptr;
        switch (event->type()) {
        case QEvent::MouseButtonPress: kind = "press"; break;
        case QEvent::MouseButtonRelease: kind = "release"; break;
        case QEvent::MouseMove: kind = "motion"; break;
        case QEvent::Wheel: kind = "wheel"; break;
        default: return QObject::eventFilter(obj, event);
        }
        int gx=0, gy=0, x=0, y=0, buttons=0;
        if (event->type() == QEvent::Wheel) {
            const auto *e = static_cast<QWheelEvent *>(event);
            gx = qRound(e->globalPosition().x());
            gy = qRound(e->globalPosition().y());
            x = qRound(e->position().x());
            y = qRound(e->position().y());
        } else {
            const auto *e = static_cast<QMouseEvent *>(event);
            gx = qRound(e->globalPosition().x());
            gy = qRound(e->globalPosition().y());
            x = qRound(e->position().x());
            y = qRound(e->position().y());
            buttons = int(e->buttons());
        }
        // Wayland global coords may be local; rootlocal is from the
        // genuine QQuickWindow dispatcher, not invented by the runner.
        std::fprintf(stdout, "SD_CI_COMPANY_QT_INPUT target=crop "
            "event=%s global=%d,%d local=%d,%d rootlocal=%d,%d "
            "buttons=%d pid=%ld\n",
            kind, gx, gy, x, y, x, y, buttons, (long)getpid());
        std::fflush(stdout);
        return QObject::eventFilter(obj, event);
    }
};
'''


def main():
    env = os.environ
    runner = Path(env.get("RUNNER_TEMP", "/not-runner"))
    src = Path(env.get("STAGE_DIRECTOR_COMPANY_PINNED_SOURCE", "/no-source"))
    out = Path(env.get("STAGE_DIRECTOR_M30L_CI_SOURCE", "/no-out"))
    if not (env.get("CI") == "true" and
            env.get("GITHUB_ACTIONS") == "true" and
            env.get("STAGE_DIRECTOR_CI_COMPANY_M30L") == "1" and
            runner == Path("/home/runner/work/_temp") and
            out.parent == runner and
            out.name == "sd-company-m30l-native-src" and
            src.is_dir() and (src / ".git").is_dir() and
            os.getuid() != 0 and not out.exists()):
        raise RuntimeError("BLOCKED_M30L_NOT_EXACT_DISPOSABLE_RUNNER")
    def git(*args):
        result = subprocess.run(
            ["git", "-C", str(src), *args], capture_output=True,
            text=True, check=True, timeout=10)
        return result.stdout.strip()
    if git("rev-parse", "HEAD") != PIN or git("status", "--porcelain"):
        raise RuntimeError("BLOCKED_M30L_COMPANY_NOT_EXACT_CLEAN_PIN")
    original = src / "experiments/company-vector-qt"
    if not all((original / p).is_file() and
               not (original / p).is_symlink()
               for p in ("main.cpp", "main.qml", "CMakeLists.txt",
                         "style-fixture.json", "style-post-apollo-online.json",
                         "fixture-hubs.json")):
        raise RuntimeError("M30L_ORIGINAL_MAPLIBRE_SOURCE_NOT_COMPLETE")
    shutil.copytree(original, out, symlinks=False)
    qml = (out / "main.qml").read_text()
    qml = replace_exact(qml, "    width: 1000\n    height: 700",
                        "    width: 840\n    height: 540",
                        "QT_SOURCE_SIZE")
    qml = replace_exact(
        qml, '    title: "POST-APOLLO // COMPANY VECTOR RENDERER PROBE"',
        '    title: "SD-CI-CompanyVectorProbe-crop"', "QT_TITLE")
    if "--online" in qml:
        # No behaviour change needed: the C++ process defaults offline.
        pass
    # Log genuine QtLocation/MapLibre map readiness and camera movement.
    # A loaded QQuickWindow alone is not MapLibre-rendering evidence.
    if not qml.rstrip().endswith("}"):
        raise RuntimeError("M30L_QML_ROOT_END_MISSING")
    qml = qml.rstrip()[:-1] + '''
    // M30l temporary CI-copy-only MapLibre/QtLocation observation.
    // Read the real QtLocation MapView.map.mapReady binding from C++.
    // Console logging may be disabled on a headless Nix Qt application.
    readonly property bool m30lMapReady:
        mapView.map !== null && mapView.map.mapReady === true
    Connections {
        target: mapView.map
        function onCenterChanged() {
            console.log("SD_COMPANY_M30L_MAP_CAMERA_EVENT")
        }
    }
}
'''
    (out / "main.qml").write_text(qml)

    cpp = (out / "main.cpp").read_text()
    cpp = replace_exact(cpp, "#include <QGuiApplication>",
                        "#include <QGuiApplication>\\n#include <QQmlError>",
                        "QML_ERROR_INCLUDE")
    cpp = replace_exact(cpp, "int main(int argc, char **argv) {",
                        SPY + "\nint main(int argc, char **argv) {",
                        "INPUT_WITNESS_DECLARATION")
    cpp = replace_exact(
        cpp, "    QGuiApplication app(argc, argv);",
        '    QGuiApplication app(argc, argv);\n'
        '    M30lQtInputWitness m30lSpy;\n'
        '    if (qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30L") == '
        'QStringLiteral("1")) app.installEventFilter(&m30lSpy);',
        "SPY_INSTALL")
    cpp = replace_exact(
        cpp,
        '    engine.load(QUrl(QStringLiteral("qrc:/CompanyVectorProbe/main.qml")));',
        '''    const bool m30lCiTrace =
        qEnvironmentVariable("STAGE_DIRECTOR_CI_COMPANY_M30L") ==
            QStringLiteral("1");
    if (m30lCiTrace) {
        QObject::connect(&engine, &QQmlEngine::warnings, &engine,
            [](const QList<QQmlError> &items) {
                for (const auto &item : items) {
                    const QByteArray message = item.toString().toUtf8();
                    std::fprintf(stderr, "SD_COMPANY_M30L_QML_WARNING %s\\n",
                                 message.constData());
                }
                std::fflush(stderr);
            });
        for (const QString &path : engine.importPathList()) {
            const QByteArray value = path.toUtf8();
            std::fprintf(stderr, "SD_COMPANY_M30L_IMPORT_PATH %s\\n",
                         value.constData());
        }
        std::fprintf(stderr, "SD_COMPANY_M30L_QML_LOAD_ATTEMPT\\n");
        std::fflush(stderr);
    }
    engine.load(QUrl(QStringLiteral("qrc:/CompanyVectorProbe/main.qml")));
    if (m30lCiTrace && engine.rootObjects().isEmpty()) {
        std::fprintf(stderr, "SD_COMPANY_M30L_QML_ROOT_EMPTY\\n");
        std::fflush(stderr);
    }''',
        "QML_STARTUP_DIAGNOSTICS")
    cpp = replace_exact(
        cpp, "    if (benchMotion) {",
        '''    if (m30lCiTrace) {
        // An actual QML property bound to the actual MapView.map.mapReady,
        // not a timer-created success string or screenshot-only proof.
        if (anyOnline || !styleUrl.isLocalFile()
                || QQuickWindow::graphicsApi() !=
                   QSGRendererInterface::OpenGLRhi) {
            std::fprintf(stderr, "SD_COMPANY_M30L_PROVENANCE_REFUSED\\n");
            std::fflush(stderr);
            return 15;
        }
        std::fprintf(stdout,
            "MAPLIBRE_COMPAT_PROBE mode offline renderer OpenGLRhi "
            "source_exact_pin=1\\n");
        std::fflush(stdout);
        QObject *m30lRoot = engine.rootObjects().first();
        auto *m30lTimer = new QTimer(&app);
        m30lTimer->setInterval(200);
        auto attempts = std::make_shared<int>(0);
        QObject::connect(m30lTimer, &QTimer::timeout, &app,
            [m30lRoot, m30lTimer, attempts]() {
                if (m30lRoot->property("m30lMapReady").toBool()) {
                    std::fprintf(stdout,
                        "SD_COMPANY_M30L_MAP_READY real_maplibre=1\\n");
                    std::fflush(stdout);
                    m30lTimer->stop();
                } else if (++(*attempts) >= 75) {
                    std::fprintf(stdout,
                        "SD_COMPANY_M30L_MAP_READY real_maplibre=0\\n");
                    std::fflush(stdout);
                    m30lTimer->stop();
                }
            });
        m30lTimer->start();
    }
    std::fprintf(stdout, "SD_CI_COMPANY_QT_QPA_READY case=crop "
        "pid=%ld wayland_display=1 maplibre_v3_source=1\\n",
        (long)getpid());
    std::fflush(stdout);
    if (benchMotion) {''',
        "NATIVE_QT_READY_MARKER")
    (out / "main.cpp").write_text(cpp)
    cmake_file = out / "CMakeLists.txt"
    cmake = cmake_file.read_text()
    old_root = "$" + "{CMAKE_CURRENT_SOURCE_DIR}"
    for suffix in ("style-fixture.json",
                   "style-post-apollo-online.json",
                   "fixture-hubs.json"):
        cmake = replace_exact(
            cmake,
            old_root + "/" + suffix,
            "M30L_NIX_OUTPUT_ROOT/share/post-apollo-company-m30l/" + suffix,
            "RELOCATE_" + suffix)
    cmake = replace_exact(
        cmake,
        old_root + "/../../services/company/CompanyHubMapReadOnly.py",
        "M30L_NIX_OUTPUT_ROOT/share/post-apollo-company-m30l/NO_OWNER_LOCAL_DATA",
        "OWNER_LOCAL_DISABLED")
    cmake_file.write_text(cmake)
    # This is a dedicated, non-network CI-only scene, not a production app
    # edit. Report precisely what was modified for a reviewer.
    print("SD_COMPANY_M30L_SOURCE_COPY "
          "original_company_pin=" + PIN +
          " original_checkout=UNCHANGED"
          " main_qml_size=840x540"
          " native_input_witness=CI_ONLY"
          " qml_startup_trace=CI_ONLY"
          " real_qml_property_poll=CI_ONLY"
          " default_offline=1"
          " no_owner_records=1 PASS", flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30L_BLOCKED:", exc, file=sys.stderr)
        raise SystemExit(1)
