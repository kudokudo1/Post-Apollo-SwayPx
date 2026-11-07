#include <QGuiApplication>
#include <QTimer>
#include <QWindow>
#include <QString>
#include <cstdio>
#include <unistd.h>

/*
 * Disposable native Qt/QPA XDG toplevel. Real wl_client and QtWayland,
 * not offscreen Qt, an imitated screenshot, MapLibre, or Quickshell.
 * Its spoofable application ID deliberately resembles CompanyVectorProbe.
 * No private Company protocol, selection request, native fd transport,
 * network access, map rendering or scene/input authority is enabled.
 */
int main(int argc, char **argv) {
	QGuiApplication app(argc, argv);
	const QString which = app.arguments().size() > 1
		? app.arguments().at(1) : QStringLiteral("one");
	if (which != QStringLiteral("one") &&
			which != QStringLiteral("two")) return 64;
	QGuiApplication::setDesktopFileName(
		QStringLiteral("CompanyVectorProbe"));
	auto *wayland =
		app.nativeInterface<QNativeInterface::QWaylandApplication>();
	if (!wayland || !wayland->display()) {
		std::fprintf(stderr, "SD_CI_COMPANY_QT_ERROR not_real_wayland_qpa\n");
		return 65;
	}
	QWindow window;
	const QString title = QStringLiteral("SD-CI-CompanyVectorProbe-") +
		which;
	window.setTitle(title);
	window.resize(840, 540);
	window.show();
	QTimer::singleShot(30000, &app, &QCoreApplication::quit);
	std::fprintf(stdout,
		"SD_CI_COMPANY_QT_QPA_READY case=%s pid=%ld wayland_display=1\n",
		which.toUtf8().constData(), (long)getpid());
	std::fflush(stdout);
	return app.exec();
}
