/* Run without a Wayland compositor. QML must load, default to OFF,
 * refuse WATCH and fail safely when explicitly enabled on non-Wayland Qt. */
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QMetaObject>
#include <QRectF>
#include <QString>
#include <QVariant>
/* The Nix package is Release; keep these runtime safety assertions active. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdio>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    if (argc != 2) return 2;
    QQmlEngine engine;
    engine.addImportPath(QString::fromLocal8Bit(argv[1]));
    QQmlComponent component(&engine);
    component.setData(R"QML(
        import QtQml
        import PostApollo.StageDirector 1.0
        WatchBridge {
            enabled: false
            viewport: Qt.rect(25, 35, 120, 90)
        }
    )QML", QUrl());
    if (component.isError()) {
        for (const auto &error : component.errors())
            std::fprintf(stderr, "QML_ERROR: %s\n",
                         qPrintable(error.toString()));
        return 1;
    }
    QObject *bridge = component.create();
    if (!bridge) return 1;
    assert(!bridge->property("enabled").toBool());
    assert(!bridge->property("paired").toBool());
    assert(!bridge->property("watchCandidate").toBool());
    assert(!bridge->property("viewportCandidate").toBool());
    assert(bridge->property("hostRevision").toULongLong() == 0);
    assert(bridge->property("status").toString() ==
           QStringLiteral("DISABLED"));
    const QRectF initial = bridge->property("viewport").toRectF();
    assert(initial.x() == 25 && initial.y() == 35);
    bool sent = true;
    assert(QMetaObject::invokeMethod(bridge, "requestWatch",
           Q_RETURN_ARG(bool, sent),
           Q_ARG(QString, QStringLiteral("bbc"))));
    assert(!sent);
    assert(bridge->setProperty("enabled", true));
    assert(!bridge->property("paired").toBool());
    assert(bridge->property("status").toString() ==
           QStringLiteral("NOT_QT_WAYLAND"));
    assert(bridge->setProperty("enabled", false));
    assert(bridge->property("status").toString() ==
           QStringLiteral("DISABLED"));
    delete bridge;
    std::puts("SD_WEATHER_QML_WATCH_OFFSCREEN: qml_import=1 "
              "default_disabled=1 nonwayland_denied=1 "
              "second_display=NONE browser=OFF authority=NONE PASS");
    return 0;
}
