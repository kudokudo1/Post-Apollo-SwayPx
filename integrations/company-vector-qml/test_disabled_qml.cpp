#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QObject>
#include <QDebug>
#include <QMetaObject>
#include <QString>
#include <QUrl>
#include <memory>

int main(int argc, char **argv) {
	QGuiApplication app(argc, argv);
	if (argc != 2) {
		qCritical() << "Expected private QML import root";
		return 2;
	}
	QQmlEngine engine;
	engine.addImportPath(QString::fromLocal8Bit(argv[1]));
	QQmlComponent component(&engine);
	component.setData(R"QML(
		import QtQml
		import PostApollo.CompanyVector 1.0
		QtObject {
			id: root
			property CompanyVectorBridge company: CompanyVectorBridge {
				objectName: "companyBridge"
			}
		}
	)QML", QUrl(QStringLiteral("file:///m25-company-disabled.qml")));
	if (component.isError()) {
		qCritical() << component.errors();
		return 3;
	}
	std::unique_ptr<QObject> root(component.create());
	if (!root) {
		qCritical() << component.errors();
		return 4;
	}
	QObject *bridge = root->findChild<QObject *>(
		QStringLiteral("companyBridge"));
	if (!bridge) {
		qCritical() << "M25 QML type missing";
		return 5;
	}
	for (const auto *key : {"enabled", "wireBound",
			"launchCandidate", "viewportCandidate"}) {
		if (bridge->property(key).toBool()) {
			qCritical() << "Unexpected active M25 property" << key;
			return 6;
		}
	}
	if (bridge->property("status").toString() !=
			QStringLiteral("DISABLED") ||
			bridge->property("epoch").toULongLong() != 0 ||
			bridge->property("hostRevision").toULongLong() != 0) {
		qCritical() << "M25 creates state or Wayland connection on load";
		return 7;
	}
	bool sent = true;
	if (!QMetaObject::invokeMethod(
			bridge, "requestLaunch", Q_RETURN_ARG(bool, sent),
			Q_ARG(bool, false)) || sent) {
		qCritical() << "Disabled M25 must refuse an offline launch intent";
		return 8;
	}
	qInfo() << "STAGE_DIRECTOR_COMPANY_M25: disabled QML module PASS";
	return 0;
}
