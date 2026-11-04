/*
 * Isolated Qt 6 -> SwayPX WATCH native connection feasibility test.
 *
 * QGuiApplication creates the ONLY Wayland display connection. Its public
 * QNativeInterface::QWaylandApplication::display() provides the existing
 * wl_display. The test creates a real mapped layer-shell host and binds the
 * production private WATCH protocol ON THAT SAME DISPLAY. The generated
 * layer-shell and WATCH proxies use a separate Wayland event queue, so this
 * CI test never steals Qt's own QPA event stream or opens a new socket.
 *
 * This is NOT a Quickshell plugin or a Weather UI edit. No browser, renderer,
 * focus, input or popup authority is granted by a successful candidate.
 */
#include <QGuiApplication>
#include <QByteArray>
#ifdef SD_CI_QML_WATCH_BRIDGE
#include <QElapsedTimer>
#include <QEventLoop>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QRectF>
#include <QThread>
#include <QVariant>
#include <functional>
#endif
#include <QtGlobal>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "post-apollo-weather-watch-v1-client-protocol.h"

struct State {
	wl_display *display = nullptr;  // borrowed from QPA; do not disconnect
	wl_event_queue *queue = nullptr;
	wl_registry *registry = nullptr;
	wl_compositor *compositor = nullptr; // borrowed from QPA; do not destroy
	wl_shm *shm = nullptr;
	zwlr_layer_shell_v1 *layer_shell = nullptr;
	wl_surface *surface = nullptr;
	zwlr_layer_surface_v1 *layer = nullptr;
	wl_buffer *buffer = nullptr;
	post_apollo_weather_watch_v1 *watch = nullptr;
	bool mapped = false;
	bool closed = false;
	unsigned status[4] = {0, 0, 0, 0};
	unsigned viewport_status[3] = {0, 0, 0};
	uint32_t watch_protocol_version = 0;
	uint64_t last_host_revision = 0;
	bool watch_advertised = false;
	bool has_layer_protocol = false;
	bool has_compositor = false;
};

static bool ci_only() {
	const char *ci = getenv("CI");
	const char *gha = getenv("GITHUB_ACTIONS");
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	const char *temp = getenv("RUNNER_TEMP");
	return ci && !strcmp(ci,"true") &&
		gha && !strcmp(gha,"true") &&
		temp && *temp && runtime &&
		!strncmp(runtime,temp,strlen(temp)) &&
		runtime[strlen(temp)] == '/';
}

#ifndef SD_CI_QML_WATCH_BRIDGE
static void watch_result(void *data, post_apollo_weather_watch_v1 *,
		uint32_t value) {
	State *s = static_cast<State *>(data);
	if (value < 4) ++s->status[value];
}
/* This fixture deliberately binds the backwards-compatible v1 proxy.
 * The v2 listener slot must still be initialized under -Werror. */
static void watch_viewport_result(void *data,
		post_apollo_weather_watch_v1 *, uint32_t status,
		uint32_t host_revision_hi, uint32_t host_revision_lo) {
	State *s = static_cast<State *>(data);
	if (status < 3) ++s->viewport_status[status];
	s->last_host_revision =
		(static_cast<uint64_t>(host_revision_hi) << 32) | host_revision_lo;
}
/* This fixture deliberately binds v2 on Qt's real Wayland connection. A
 * separate C wire fixture retains v1 WATCH/CLOSE/opcode coverage. */
static const post_apollo_weather_watch_v1_listener watch_listener = {
	watch_result,
	watch_viewport_result
};
#endif

static void registry_global(void *data, wl_registry *registry,
		uint32_t name, const char *iface, uint32_t version) {
	State *s = static_cast<State *>(data);
	if (!strcmp(iface,wl_compositor_interface.name)) {
		s->has_compositor = true;
	} else if (!strcmp(iface,wl_shm_interface.name)) {
		s->shm = static_cast<wl_shm *>(wl_registry_bind(
			registry,name,&wl_shm_interface,1));
	} else if (!strcmp(iface,zwlr_layer_shell_v1_interface.name)) {
		s->has_layer_protocol = true;
		s->layer_shell = static_cast<zwlr_layer_shell_v1 *>(
			wl_registry_bind(registry,name,
				&zwlr_layer_shell_v1_interface,
				version < 1 ? version : 1));
	} else if (!strcmp(iface,post_apollo_weather_watch_v1_interface.name)) {
		s->watch_advertised = true;
#ifndef SD_CI_QML_WATCH_BRIDGE
		if (!s->watch && version >= 2) {
			s->watch_protocol_version = 2;
			s->watch = static_cast<post_apollo_weather_watch_v1 *>(
				wl_registry_bind(registry,name,
					&post_apollo_weather_watch_v1_interface,2));
			if (s->watch) {
				post_apollo_weather_watch_v1_add_listener(
					s->watch, &watch_listener, s);
			}
		}
#else
		if (version >= 2) s->watch_protocol_version = 2;
#endif
	}
}
static void registry_remove(void *, wl_registry *, uint32_t) {}
static const wl_registry_listener registry_listener = {
	registry_global, registry_remove
};

static wl_buffer *make_buffer(State *s, uint32_t width, uint32_t height) {
	if (!s->shm || width == 0 || height == 0 || width > 600 || height > 600)
		return nullptr;
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!runtime || strlen(runtime) > PATH_MAX - 64) return nullptr;
	char file[PATH_MAX];
	if (snprintf(file,sizeof(file),"%s/qt-watch-XXXXXX",runtime) <= 0)
		return nullptr;
	int fd = mkstemp(file);
	if (fd < 0) return nullptr;
	(void)unlink(file);
	const uint32_t stride = width * 4;
	const size_t length = static_cast<size_t>(stride) * height;
	if (ftruncate(fd,static_cast<off_t>(length)) != 0) {
		close(fd); return nullptr;
	}
	uint32_t *pixels = static_cast<uint32_t *>(mmap(
		nullptr,length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
	if (pixels == MAP_FAILED) {
		close(fd); return nullptr;
	}
	for (size_t i=0;i<length/4;++i) pixels[i]=0xff55cfca;
	(void)munmap(pixels,length);
	wl_shm_pool *pool = wl_shm_create_pool(s->shm,fd,static_cast<int>(length));
	wl_buffer *buf = pool ? wl_shm_pool_create_buffer(pool,0,
		static_cast<int32_t>(width),static_cast<int32_t>(height),
		static_cast<int32_t>(stride),WL_SHM_FORMAT_ARGB8888) : nullptr;
	if (pool) wl_shm_pool_destroy(pool);
	close(fd);
	return buf;
}
static void layer_configure(void *data, zwlr_layer_surface_v1 *layer,
		uint32_t serial, uint32_t width, uint32_t height) {
	State *s = static_cast<State *>(data);
	zwlr_layer_surface_v1_ack_configure(layer,serial);
	if (s->mapped) return;
	s->buffer = make_buffer(s,width,height);
	if (!s->buffer) { s->closed = true; return; }
	wl_surface_attach(s->surface,s->buffer,0,0);
	wl_surface_damage(s->surface,0,0,
		static_cast<int32_t>(width),static_cast<int32_t>(height));
	wl_surface_commit(s->surface);
	s->mapped = true;
}
static void layer_closed(void *data, zwlr_layer_surface_v1 *) {
	static_cast<State *>(data)->closed = true;
}
static const zwlr_layer_surface_v1_listener layer_listener = {
	layer_configure,layer_closed
};
static bool dispatch(State *s) {
	return wl_display_roundtrip_queue(s->display,s->queue) >= 0;
}
#ifdef SD_CI_QML_WATCH_BRIDGE
/* After the real Weather layer is mapped, the bridge must receive messages
 * through Qt's OWN socket reader and its QTimer event pump. No native
 * wl_display_roundtrip_queue/dispatch call is permitted in this phase. */
static bool qpa_events_until(QGuiApplication &app,
		const std::function<bool()> &predicate, int limitMs = 6500) {
	QElapsedTimer elapsed;
	elapsed.start();
	while (elapsed.elapsed() < limitMs) {
		app.processEvents(QEventLoop::AllEvents, 25);
		if (predicate()) return true;
		QThread::msleep(10);
	}
	return predicate();
}
static bool invoke_watch(QObject *obj, const QString &source, bool expected) {
	bool sent = !expected;
	return QMetaObject::invokeMethod(obj, "requestWatch",
		Q_RETURN_ARG(bool, sent), Q_ARG(QString, source)) &&
		sent == expected;
}
static bool invoke_close(QObject *obj) {
	return QMetaObject::invokeMethod(obj, "close", Qt::DirectConnection);
}
static bool qml_bridge_test(QGuiApplication &app, State *s) {
	const char *module = getenv("SD_CI_WATCH_QML_IMPORT_PATH");
	if (!module || strncmp(module, "/nix/store/", 11)) {
		fputs("BLOCKED_NONISOLATED_QML_IMPORT\n", stderr);
		return false;
	}
	QQmlEngine engine;
	engine.addImportPath(QString::fromLocal8Bit(module));
	QQmlComponent qml(&engine);
	qml.setData(R"QML(
import QtQml
import PostApollo.StageDirector 1.0
WatchBridge { enabled: true; viewport: Qt.rect(20, 30, 120, 100) }
)QML", QUrl());
	if (qml.isError()) {
		for (const auto &error : qml.errors())
			fprintf(stderr, "QML_BRIDGE_IMPORT_FAIL: %s\n", qPrintable(error.toString()));
		return false;
	}
	QObject *obj = qml.create();
	if (!obj) {
		fputs("FAIL: no live WatchBridge QObject\n", stderr);
		return false;
	}
	const auto prop = [obj](const char *name) {return obj->property(name);};
	const auto is = [&prop](const char *name, bool value) {
		return prop(name).toBool() == value;
	};
	if (!s->mapped ||
		!qpa_events_until(app, [&]() {return is("paired",true);}, 7000) ||
		prop("status").toString() != QStringLiteral("WATCH_V2_AVAILABLE")) {
		fprintf(stderr,"FAIL: Qt QPA event loop did not discover privileged v2: %s\n",
			qPrintable(prop("status").toString()));
		delete obj;
		return false;
	}
	if (!invoke_watch(obj, QStringLiteral("https://invalid.example/watch"),false) ||
		!invoke_watch(obj,QStringLiteral("bbc"),true) ||
		!qpa_events_until(app,[&]() {
			return is("watchCandidate",true) &&
				is("viewportCandidate",true) &&
				prop("hostRevision").toULongLong() != 0;
		},7000)) {
		fprintf(stderr,"FAIL: real QML WATCH/viewport acknowledgements: %s\n",
			qPrintable(prop("status").toString()));
		delete obj;
		return false;
	}
	const auto initialRevision = prop("hostRevision").toULongLong();
	if (!obj->setProperty("viewport",QRectF()) ||
		!qpa_events_until(app,[&]() {
			return is("viewportCandidate",false) &&
				prop("status").toString() == QStringLiteral("VIEWPORT_HIDDEN");
		},6000)) {
		fprintf(stderr,"FAIL: QML hidden viewport not acknowledged: %s\n",
			qPrintable(prop("status").toString()));
		delete obj;
		return false;
	}
	if (!obj->setProperty("viewport",QRectF(25,35,120,100)) ||
		!qpa_events_until(app,[&]() {
			return is("viewportCandidate",true) &&
				prop("hostRevision").toULongLong() == initialRevision;
		},6000)) {
		fputs("FAIL: QML viewport reconnect after hidden\n",stderr);
		delete obj;
		return false;
	}
	if (!invoke_close(obj) ||
		!qpa_events_until(app,[&]() {
			return is("paired",false) &&
				is("watchCandidate",false) &&
				is("viewportCandidate",false) &&
				prop("status").toString() == QStringLiteral("CLOSED");
		},6000)) {
		fprintf(stderr,"FAIL: QML CLOSE lost or unreconciled: %s\n",
			qPrintable(prop("status").toString()));
		delete obj;
		return false;
	}
	if (!obj->setProperty("enabled",false) ||
		!is("paired",false) ||
		prop("status").toString() != QStringLiteral("DISABLED") ||
		!invoke_watch(obj,QStringLiteral("bbc"),false)) {
		fputs("FAIL: QML disabled but retained WATCH authority\n",stderr);
		delete obj;
		return false;
	}
	delete obj;
	printf("SD_CI_QML_WATCH_BRIDGE_REAL_QPA QtQml_type=1 qt_owned_display=1 "
		"real_native_mapped_layer=1 v2_dynamic_global=1 watch_candidate=1 "
		"viewport_candidate=1 hidden=1 resize=1 close_revoked=1 "
		"disabled=1 host_revision=%llu browser=OFF input=NONE PASS\n",
		static_cast<unsigned long long>(initialRevision));
	fflush(stdout);
	return true;
}
#endif

int main(int argc, char **argv) {
	alarm(25);
	if (!ci_only() || argc != 1) {
		fputs("BLOCKED_NOT_DISPOSABLE_QT_WAYLAND_BRIDGE\n",stderr);
		return 2;
	}
	QGuiApplication app(argc,argv);
	auto *qt = app.nativeInterface<QNativeInterface::QWaylandApplication>();
	if (!qt || !qt->display() || !qt->compositor()) {
		fputs("FAIL: Qt QPA Wayland native interface unavailable\n",stderr);
		return 1;
	}
	State s;
	s.display = qt->display();
	s.compositor = qt->compositor();
	s.queue = wl_display_create_queue(s.display);
	s.registry = wl_display_get_registry(s.display);
	if (!s.queue || !s.registry) return 1;
	/* The new registry and its children must belong to an isolated event
	 * queue; Qt retains ownership of its existing display/dispatch thread. */
	wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(s.registry),s.queue);
	if (wl_registry_add_listener(s.registry,&registry_listener,&s) ||
			!dispatch(&s) || !s.has_compositor || !s.has_layer_protocol ||
			!s.layer_shell || !s.shm) {
		fputs("FAIL: Qt-owned Wayland display lacks layer-shell/SHM registry\n",
			stderr);
		return 1;
	}
	/* Creates native layer on Qt's *existing* compositor connection. */
	s.surface = wl_compositor_create_surface(s.compositor);
	if (!s.surface) return 1;
	s.layer = zwlr_layer_shell_v1_get_layer_surface(s.layer_shell,
		s.surface,nullptr,ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
		"post-apollo-weather-station");
	if (!s.layer) return 1;
	zwlr_layer_surface_v1_add_listener(s.layer,&layer_listener,&s);
	zwlr_layer_surface_v1_set_size(s.layer,320,240);
	zwlr_layer_surface_v1_set_anchor(s.layer,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
	zwlr_layer_surface_v1_set_exclusive_zone(s.layer,0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(s.layer,0);
	wl_surface_commit(s.surface);
	for (int i=0;i<35 && !s.mapped && !s.closed;++i) {
		if (!dispatch(&s)) return 1;
	}
	if (!s.mapped || s.closed) {
		fputs("FAIL: Qt-owned Wayland display failed to map Weather layer\n",
			stderr);
		return 1;
	}
	puts("SD_CI_QT_WAYLAND_LAYER_MAPPED qt_native_display=1 "
		"same_connection=1 mapped=1 authority=NONE");
	fflush(stdout);
#ifdef SD_CI_QML_WATCH_BRIDGE
	/* No test-native WATCH proxy may consume the candidate. The production
	 * WatchBridge QML object must discover the dynamic global on this same
	 * Qt native display via Qt's event loop. */
	if (!qml_bridge_test(app, &s)) return 1;
	puts("SD_CI_QT_NATIVE_WATCH_BRIDGE Qt6_QGuiApplication=1 "
		"QWaylandApplication_display=1 same_wl_display=1 "
		"actual_mapped_layer=1 candidate=1 v2_viewport=1 "
		"invalid_url_refused=1 close_revoked=1 browser=OFF "
		"input_auth=UNTESTED authority=NONE PASS");
	fflush(stdout);
#else
	/* Compositor publishes WATCH only after native mapped host selection.
	 * A registry created before pairing must receive this dynamic global. */
	for (int i=0;i<60 && !s.watch;++i) {
		if (!dispatch(&s)) return 1;
	}
	if (!s.watch || !s.watch_advertised || s.watch_protocol_version != 2) {
		fputs("FAIL: Qt-owned Weather Wayland display did not receive WATCH v2\n",
			stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_watch(s.watch,"bbc");
	if (!dispatch(&s) || s.status[1] != 1) {
		fputs("FAIL: Qt-owned display WATCH candidate request\n",stderr);
		return 1;
	}
	/* Real production host: QML-like rectangle is local to the mapped
	 * 320x240 layer, rebased to the compositor's native output geometry.
	 * It remains candidate-only; it does not create an inner wl_surface. */
	post_apollo_weather_watch_v1_report_viewport(s.watch, 20, 30, 120, 100);
	if (!dispatch(&s) || s.viewport_status[1] != 1 ||
			s.last_host_revision == 0) {
		fputs("FAIL: Qt-owned display WATCH v2 viewport candidate\n",stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_report_viewport(s.watch, -1, 30, 120, 100);
	if (!dispatch(&s) || s.viewport_status[0] != 1) {
		fputs("FAIL: Qt-owned display accepted invalid local viewport\n",
			stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_report_viewport(s.watch, 0, 0, 0, 0);
	if (!dispatch(&s) || s.viewport_status[2] != 1) {
		fputs("FAIL: Qt-owned display failed to clear hidden viewer\n",stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_report_viewport(s.watch, 20, 30, 120, 100);
	if (!dispatch(&s) || s.viewport_status[1] != 2) {
		fputs("FAIL: Qt-owned display refused renewed viewport candidate\n",
			stderr);
		return 1;
	}
	puts("SD_CI_QT_NATIVE_VIEWPORT_V2 same_wl_display=1 "
		"real_paired_weather_layer=1 positive=1 invalid_denied=1 "
		"hidden=1 renewed=1 host_revision=1 browser=OFF authority=NONE PASS");
	fflush(stdout);
	/* A second valid WATCH on the same paired resource must be denied:
	 * only one compositor-owned candidate can be reserved per epoch. */
	post_apollo_weather_watch_v1_watch(s.watch,"cspan");
	if (!dispatch(&s) || s.status[0] != 1 || s.status[1] != 1) {
		fputs("FAIL: duplicate live WATCH candidate was accepted\n",stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_watch(s.watch,"https://invalid.example/");
	if (!dispatch(&s) || s.status[2] != 1) {
		fputs("FAIL: Qt-owned display accepted URL instead of catalog ID\n",stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_close(s.watch);
	if (!dispatch(&s) || s.status[3] != 1) {
		fputs("FAIL: Qt-owned display CLOSE request\n",stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_report_viewport(s.watch, 20, 30, 120, 100);
	if (!dispatch(&s) || s.viewport_status[0] != 2 ||
			s.last_host_revision != 0) {
		fputs("FAIL: Qt-owned display accepted stale viewport after CLOSE\n",
			stderr);
		return 1;
	}
	post_apollo_weather_watch_v1_watch(s.watch,"bbc");
	if (!dispatch(&s) || s.status[0] != 2 || s.status[1] != 1) {
		fputs("FAIL: Qt-owned display accepted stale WATCH after CLOSE\n",stderr);
		return 1;
	}
	puts("SD_CI_QT_NATIVE_WATCH_BRIDGE Qt6_QGuiApplication=1 "
		"QWaylandApplication_display=1 same_wl_display=1 "
		"actual_mapped_layer=1 candidate=1 v2_viewport=1 invalid_url_refused=1 "
		"duplicate_watch_denied=1 close_revoked=1 stale_watch_denied=1 browser=OFF "
		"input_auth=UNTESTED authority=NONE PASS");
	fflush(stdout);
	/* Display and Qt platform compositor proxy remain Qt-owned.
	 * Other local protocol objects/queue belong to the test. */
#endif
	if (s.watch) post_apollo_weather_watch_v1_destroy(s.watch);
	zwlr_layer_surface_v1_destroy(s.layer);
	wl_surface_destroy(s.surface);
	if (s.buffer) wl_buffer_destroy(s.buffer);
	zwlr_layer_shell_v1_destroy(s.layer_shell);
	wl_shm_destroy(s.shm);
	wl_registry_destroy(s.registry);
	wl_event_queue_destroy(s.queue);
	return 0;
}
