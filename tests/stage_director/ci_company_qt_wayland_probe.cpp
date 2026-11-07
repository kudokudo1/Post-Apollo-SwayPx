#include <QApplication>
#include <QGuiApplication>
#include <QWidget>
#include <QTimer>
#include <QString>
#include <cstdio>
#include <unistd.h>

/*
 * Disposable native Qt/QPA XDG toplevel. Real wl_client and QtWayland,
 * not offscreen Qt, an imitated screenshot, MapLibre, or Quickshell.
 * QWidget deliberately submits an actual painted backing-store buffer.
 * Its spoofable application ID deliberately resembles CompanyVectorProbe.
 * No private Company protocol, selection request, native fd transport,
 * network access, map rendering or scene/input authority is enabled.
 */
int main(int argc, char **argv) {
	QApplication app(argc, argv);
	const QString which = app.arguments().size() > 1
		? app.arguments().at(1) : QStringLiteral("one");
	if (which != QStringLiteral("one") &&
			which != QStringLiteral("two") &&
			which != QStringLiteral("frame") &&
			which != QStringLiteral("map") &&
			which != QStringLiteral("crop")) return 64;
	QGuiApplication::setDesktopFileName(
		QStringLiteral("CompanyVectorProbe"));
	auto *wayland =
		app.nativeInterface<QNativeInterface::QWaylandApplication>();
	if (!wayland || !wayland->display()) {
		std::fprintf(stderr, "SD_CI_COMPANY_QT_ERROR not_real_wayland_qpa\n");
		return 65;
	}
	QWidget window;
	window.setStyleSheet(which == QStringLiteral("frame")
		? QStringLiteral("background-color: #1b0623;")
		: QStringLiteral("background-color: #55cfca;"));
	const QString title = which == QStringLiteral("frame")
		? QStringLiteral("SD-CI-CompanyWidgetFrame")
		: QStringLiteral("SD-CI-CompanyVectorProbe-") + which;
	window.setWindowTitle(title);
	window.resize(which == QStringLiteral("frame") ? 1100 : 840,
		which == QStringLiteral("frame") ? 740 : 540);
	/* M30g true-source-crop witness: source is STILL 840x540. Orange
	 * occupies all pixels beyond the 600x380 intended viewport. If the
	 * compositor merely rescales the surface or fails to clip it, the
	 * real screenshot will expose orange immediately. This is a normal
	 * Qt QWidget paint, not a synthetic pixel array or fake frame. */
	QWidget overflowRight(&window);
	QWidget overflowBottom(&window);
	if (which == QStringLiteral("crop")) {
		overflowRight.setStyleSheet(
			QStringLiteral("background-color: #ed981a;"));
		overflowBottom.setStyleSheet(
			QStringLiteral("background-color: #ed981a;"));
		overflowRight.setGeometry(600, 0, 240, 540);
		overflowBottom.setGeometry(0, 380, 840, 160);
		overflowRight.show();
		overflowBottom.show();
	}
	window.show();
	QTimer::singleShot((which == QStringLiteral("frame") ||
		which == QStringLiteral("map") ||
		which == QStringLiteral("crop")) ? 90000 : 30000,
		&app, &QCoreApplication::quit);
	std::fprintf(stdout,
		"SD_CI_COMPANY_QT_QPA_READY case=%s pid=%ld wayland_display=1\n",
		which.toUtf8().constData(), (long)getpid());
	std::fflush(stdout);
	return app.exec();
}
