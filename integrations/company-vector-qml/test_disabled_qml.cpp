/* M25 imported Company client MUST be inert on offscreen and require Qt QPA. */
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QMetaObject>
#include <QRectF>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <memory>

int main(int argc, char **argv) {
	QGuiApplication app(argc, argv);
	if (argc != 2) return 2;
	QQmlEngine engine;
	engine.addImportPath(QString::fromLocal8Bit(argv[1]));
	QQmlComponent component(&engine);
	component.setData(R"QML(
		import QtQml
		import PostApollo.CompanyVector 1.0
		CompanyVectorBridge {
			enabled: false
			frameRect: Qt.rect(12, 9, 976, 679)
			mapRect: Qt.rect(200, 110, 650, 450)
		}
	)QML", QUrl());
	if (component.isError()) {
		qCritical() << component.errors();
		return 3;
	}
	std::unique_ptr<QObject> bridge(component.create());
	if (!bridge) {
		qCritical() << component.errors();
		return 4;
	}
	for (const auto *key : {"enabled", "wireBound",
			"launchCandidate", "viewportCandidate"}) {
		if (bridge->property(key).toBool()) {
			qCritical() << "Unexpected active M25 property" << key;
			return 5;
		}
	}
	if (bridge->property("status").toString() !=
			QStringLiteral("DISABLED") ||
			bridge->property("epoch").toULongLong() != 0 ||
			bridge->property("hostRevision").toULongLong() != 0) {
		return 6;
	}
	const QRectF frame = bridge->property("frameRect").toRectF();
	const QRectF map = bridge->property("mapRect").toRectF();
	if (frame.x() != 12 || frame.y() != 9 ||
			frame.width() != 976 || frame.height() != 679 ||
			map.x() != 200 || map.y() != 110 ||
			map.width() != 650 || map.height() != 450) {
		return 7;
	}
	bool sent = true;
	if (!QMetaObject::invokeMethod(
			bridge.get(), "requestLaunch", Q_RETURN_ARG(bool, sent),
			Q_ARG(bool, false)) || sent) return 8;
	/* Offscreen has no native Wayland QPA display; explicit enable must
	 * fail closed and must never create a second wl_display connection. */
	if (!bridge->setProperty("enabled", true) ||
			bridge->property("status").toString() !=
			QStringLiteral("NOT_QT_WAYLAND") ||
			bridge->property("wireBound").toBool()) return 9;
	if (!bridge->setProperty("enabled", false) ||
			bridge->property("status").toString() !=
			QStringLiteral("DISABLED")) return 10;
	qInfo() << "STAGE_DIRECTOR_COMPANY_M25: QML import and offscreen fail-closed PASS";
	return 0;
}
