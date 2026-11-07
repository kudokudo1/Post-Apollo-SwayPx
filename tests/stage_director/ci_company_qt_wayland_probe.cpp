#include <QApplication>
#include <QGuiApplication>
#include <QWidget>
#include <QTimer>
#include <QString>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QObject>
#include <cstdlib>
#include <cstring>
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
/* M30h: observe ACTUAL Qt-delivered Wayland pointer events. This is not
 * generated evidence or IPC focus selection; only Qt dispatch writes a line.
 * The widget coordinates remain native to the actual 840x540 Qt source. */
class InputSpy final : public QObject {
public:
    explicit InputSpy(const char *name) : name_(name) {}

    bool eventFilter(QObject *object, QEvent *event) override {
        QWidget *widget = qobject_cast<QWidget *>(object);
        if (!widget) {
            return QObject::eventFilter(object, event);
        }
        const char *type = nullptr;
        switch (event->type()) {
        case QEvent::MouseButtonPress: type = "press"; break;
        case QEvent::MouseButtonRelease: type = "release"; break;
        case QEvent::MouseMove: type = "motion"; break;
        case QEvent::Wheel: type = "wheel"; break;
        default: return QObject::eventFilter(object, event);
        }
        int gx = 0, gy = 0, lx = 0, ly = 0;
        int held = 0;
        if (event->type() == QEvent::Wheel) {
            const auto *wheel = static_cast<QWheelEvent *>(event);
            gx = qRound(wheel->globalPosition().x());
            gy = qRound(wheel->globalPosition().y());
            lx = qRound(wheel->position().x());
            ly = qRound(wheel->position().y());
        } else {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            gx = qRound(mouse->globalPosition().x());
            gy = qRound(mouse->globalPosition().y());
            lx = qRound(mouse->position().x());
            ly = qRound(mouse->position().y());
            held = static_cast<int>(mouse->buttons());
        }
        /* QtWayland globalPosition() can be local on compositors because
         * Wayland clients do not know global window placement. Emit the
         * root-widget position; Sway IPC provides the compositor origin. */
        const QPoint root = widget->mapTo(widget->window(), QPoint(lx, ly));
        std::fprintf(stdout,
            "SD_CI_COMPANY_QT_INPUT target=%s event=%s "
            "global=%d,%d local=%d,%d rootlocal=%d,%d buttons=%d pid=%ld\n",
            name_, type, gx, gy, lx, ly, root.x(), root.y(),
            held, (long)getpid());
        std::fflush(stdout);
        return QObject::eventFilter(object, event);
    }

private:
    const char *name_;
};

int main(int argc, char **argv) {
	QApplication app(argc, argv);
    const char *gate = std::getenv("STAGE_DIRECTOR_CI_COMPANY_INPUT");
    const bool spy_input = gate && std::strcmp(gate, "1") == 0 &&
        std::getenv("GITHUB_ACTIONS") && std::getenv("CI");
    InputSpy map_spy("crop");
    InputSpy frame_spy("frame");
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
	if (spy_input && which == QStringLiteral("crop")) {
        app.installEventFilter(&map_spy);
    } else if (spy_input && which == QStringLiteral("frame")) {
        app.installEventFilter(&frame_spy);
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
