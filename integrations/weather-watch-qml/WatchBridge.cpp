#include "WatchBridge.hpp"

#include <QGuiApplication>
#include <QtGlobal>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <limits>
#include <wayland-client.h>

#include "post-apollo-weather-watch-v1-client-protocol.h"

WatchBridge::WatchBridge(QObject *parent) : QObject(parent) {
    m_queuePump.setInterval(40);
    connect(&m_queuePump, &QTimer::timeout, this, &WatchBridge::pump);
    m_viewportDebounce.setSingleShot(true);
    m_viewportDebounce.setInterval(25);
    connect(&m_viewportDebounce, &QTimer::timeout,
            this, &WatchBridge::sendViewport);
}

WatchBridge::~WatchBridge() {
    stop();
}

void WatchBridge::setStatus(const QString &status) {
    if (status == m_status) return;
    m_status = status;
    emit statusChanged();
}
void WatchBridge::setWatchCandidate(bool candidate) {
    if (candidate == m_watchCandidate) return;
    m_watchCandidate = candidate;
    emit watchCandidateChanged();
}
void WatchBridge::setViewportCandidate(bool candidate) {
    if (candidate == m_viewportCandidate) return;
    m_viewportCandidate = candidate;
    emit viewportCandidateChanged();
}
void WatchBridge::setHostRevision(qulonglong revision) {
    if (revision == m_hostRevision) return;
    m_hostRevision = revision;
    emit hostRevisionChanged();
}

void WatchBridge::setEnabled(bool enabled) {
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    emit enabledChanged();
    if (enabled) start();
    else stop();
}

void WatchBridge::setViewport(const QRectF &localRect) {
    if (localRect == m_viewport) return;
    m_viewport = localRect;
    emit viewportChanged();
    if (m_watchCandidate) {
        // Coalesce the ordinary QtQuick layout wave. A hidden rectangle is
        // sent immediately to revoke any old player candidate.
        if (localRect.isEmpty()) {
            m_viewportDebounce.stop();
            sendViewport();
        } else {
            m_viewportDebounce.start();
        }
    }
}

void WatchBridge::start() {
    m_closeAcknowledged = false;
    // Qt QPA owns this display. Creating a second Wayland socket would fail
    // the compositor's exact native client pairing check by design.
    if (!qGuiApp) {
        setStatus(QStringLiteral("NO_QT_GUI_APPLICATION"));
        return;
    }
    auto *native = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!native || !native->display()) {
        setStatus(QStringLiteral("NOT_QT_WAYLAND"));
        return;
    }
    m_display = native->display();
    m_queue = wl_display_create_queue(m_display);
    if (!m_queue) {
        m_display = nullptr;
        setStatus(QStringLiteral("WAYLAND_QUEUE_UNAVAILABLE"));
        return;
    }
    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        wl_event_queue_destroy(m_queue);
        m_queue = nullptr;
        m_display = nullptr;
        setStatus(QStringLiteral("WAYLAND_REGISTRY_UNAVAILABLE"));
        return;
    }
    // Own only our proxies and event queue, NEVER Qt's display/fd. Do not
    // call wl_display_read_events or wl_display_roundtrip from QML callbacks.
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_registry), m_queue);
    static const wl_registry_listener listener = {
        &WatchBridge::registryGlobal, &WatchBridge::registryRemoved
    };
    wl_registry_add_listener(m_registry, &listener, this);
    setStatus(QStringLiteral("AWAITING_NATIVE_PAIRING"));
    m_queuePump.start();
    flush();
}

void WatchBridge::stop() {
    m_closeAcknowledged = false;
    m_viewportDebounce.stop();
    m_queuePump.stop();
    if (m_watch && m_watchCandidate) {
        // Stop using any prior player candidate before tearing down the
        // protocol object. The server independently revokes on destruction.
        post_apollo_weather_watch_v1_report_viewport(m_watch, 0, 0, 0, 0);
        post_apollo_weather_watch_v1_close(m_watch);
    }
    releaseWatch();
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    flush();
    if (m_queue) {
        wl_event_queue_destroy(m_queue);
        m_queue = nullptr;
    }
    m_display = nullptr; // borrowed, never disconnect
    m_watchGlobalName = 0;
    setStatus(QStringLiteral("DISABLED"));
}

void WatchBridge::flush() {
    if (!m_display) return;
    // Qt QPA continues to own the socket read/dispatch. We only flush
    // nonblocking generated requests and drain this private pending queue.
    const int result = wl_display_flush(m_display);
    if (result < 0 && errno != EAGAIN) {
        setStatus(QStringLiteral("WAYLAND_FLUSH_ERROR"));
    }
}

void WatchBridge::pump() {
    if (!m_display || !m_queue) return;
    if (wl_display_dispatch_queue_pending(m_display, m_queue) < 0) {
        setWatchCandidate(false);
        setViewportCandidate(false);
        setHostRevision(0);
        setStatus(QStringLiteral("WAYLAND_DISCONNECTED"));
        m_queuePump.stop();
        return;
    }
    flush();
}

void WatchBridge::registryGlobal(void *data, wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version) {
    auto *self = static_cast<WatchBridge *>(data);
    if (!self || !self->m_enabled || !interface ||
            strcmp(interface, post_apollo_weather_watch_v1_interface.name)) {
        return;
    }
    if (self->m_watch) return;
    if (version < 2) {
        self->setStatus(QStringLiteral("WATCH_V2_REQUIRED"));
        return;
    }
    self->m_watch = static_cast<post_apollo_weather_watch_v1 *>(
        wl_registry_bind(registry, name,
                         &post_apollo_weather_watch_v1_interface, 2));
    if (!self->m_watch) {
        self->setStatus(QStringLiteral("WATCH_BIND_FAILED"));
        return;
    }
    self->m_watchGlobalName = name;
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(self->m_watch),
                       self->m_queue);
    static const post_apollo_weather_watch_v1_listener watchListener = {
        &WatchBridge::watchResult, &WatchBridge::viewportResult
    };
    if (post_apollo_weather_watch_v1_add_listener(
            self->m_watch, &watchListener, self) != 0) {
        self->releaseWatch();
        self->setStatus(QStringLiteral("WATCH_LISTENER_FAILED"));
        return;
    }
    emit self->pairedChanged();
    self->setStatus(QStringLiteral("WATCH_V2_AVAILABLE"));
    self->flush();
}

void WatchBridge::registryRemoved(void *data, wl_registry *,
                                  uint32_t name) {
    auto *self = static_cast<WatchBridge *>(data);
    if (!self || name != self->m_watchGlobalName) return;
    const bool acknowledgedClose = self->m_closeAcknowledged;
    self->releaseWatch();
    self->m_closeAcknowledged = false;
    self->setStatus(acknowledgedClose
        ? QStringLiteral("CLOSED")
        : QStringLiteral("PAIRING_REVOKED"));
}

void WatchBridge::releaseWatch() {
    const bool hadWatch = m_watch != nullptr;
    if (m_watch) {
        post_apollo_weather_watch_v1_destroy(m_watch);
        m_watch = nullptr;
    }
    m_watchGlobalName = 0;
    setWatchCandidate(false);
    setViewportCandidate(false);
    setHostRevision(0);
    if (hadWatch) emit pairedChanged();
}

void WatchBridge::watchResult(void *data, post_apollo_weather_watch_v1 *,
                             uint32_t result) {
    auto *self = static_cast<WatchBridge *>(data);
    if (!self) return;
    if (result == POST_APOLLO_WEATHER_WATCH_V1_STATUS_CANDIDATE_ONLY) {
        self->m_closeAcknowledged = false;
        self->setWatchCandidate(true);
        self->setStatus(QStringLiteral("WATCH_CANDIDATE_ONLY"));
        self->sendViewport();
    } else if (result == POST_APOLLO_WEATHER_WATCH_V1_STATUS_CLOSED) {
        self->m_closeAcknowledged = true;
        self->setWatchCandidate(false);
        self->setViewportCandidate(false);
        self->setHostRevision(0);
        self->setStatus(QStringLiteral("CLOSED"));
    } else if (result == POST_APOLLO_WEATHER_WATCH_V1_STATUS_INVALID_SOURCE) {
        self->m_closeAcknowledged = false;
        self->setStatus(QStringLiteral("INVALID_CATALOG_SOURCE"));
    } else {
        self->m_closeAcknowledged = false;
        self->setWatchCandidate(false);
        self->setViewportCandidate(false);
        self->setHostRevision(0);
        self->setStatus(QStringLiteral("WATCH_DENIED"));
    }
}

void WatchBridge::viewportResult(void *data,
                                 post_apollo_weather_watch_v1 *,
                                 uint32_t result, uint32_t revisionHi,
                                 uint32_t revisionLo) {
    auto *self = static_cast<WatchBridge *>(data);
    if (!self) return;
    self->setHostRevision((static_cast<qulonglong>(revisionHi) << 32) |
                          static_cast<qulonglong>(revisionLo));
    if (result == POST_APOLLO_WEATHER_WATCH_V1_VIEWPORT_STATUS_CANDIDATE_ONLY) {
        self->setViewportCandidate(true);
        self->setStatus(QStringLiteral("VIEWPORT_CANDIDATE_ONLY"));
    } else if (result == POST_APOLLO_WEATHER_WATCH_V1_VIEWPORT_STATUS_HIDDEN) {
        self->setViewportCandidate(false);
        self->setStatus(QStringLiteral("VIEWPORT_HIDDEN"));
    } else {
        self->setViewportCandidate(false);
        self->setWatchCandidate(false);
        self->setStatus(QStringLiteral("VIEWPORT_DENIED"));
    }
}

bool WatchBridge::requestWatch(const QString &sourceId) {
    if (!m_enabled || !m_watch || m_watchCandidate ||
            (sourceId != QStringLiteral("bbc") &&
             sourceId != QStringLiteral("cspan"))) {
        return false;
    }
    const QByteArray source = sourceId.toUtf8();
    post_apollo_weather_watch_v1_watch(m_watch, source.constData());
    setStatus(QStringLiteral("WATCH_REQUEST_SENT"));
    flush();
    return true; // SENT, not accepted; only result() confirms candidate
}

void WatchBridge::sendViewport() {
    if (!m_enabled || !m_watch || !m_watchCandidate) return;
    const auto r = m_viewport;
    // Zero all four fields on close, unset or malformed QML input.
    // No user-provided rectangle is ever treated as a native wl_surface.
    const bool valid = std::isfinite(r.x()) && std::isfinite(r.y()) &&
        std::isfinite(r.width()) && std::isfinite(r.height()) &&
        r.x() >= 0 && r.y() >= 0 &&
        r.width() > 0 && r.height() > 0 &&
        r.x() <= static_cast<qreal>(std::numeric_limits<int32_t>::max() - 1) &&
        r.y() <= static_cast<qreal>(std::numeric_limits<int32_t>::max() - 1) &&
        r.width() <= static_cast<qreal>(std::numeric_limits<int32_t>::max() - 1) &&
        r.height() <= static_cast<qreal>(std::numeric_limits<int32_t>::max() - 1);
    if (!valid) {
        post_apollo_weather_watch_v1_report_viewport(m_watch, 0, 0, 0, 0);
    } else {
        const int32_t x = static_cast<int32_t>(qRound(r.x()));
        const int32_t y = static_cast<int32_t>(qRound(r.y()));
        const int32_t w = static_cast<int32_t>(qRound(r.width()));
        const int32_t h = static_cast<int32_t>(qRound(r.height()));
        if (w <= 0 || h <= 0) {
            post_apollo_weather_watch_v1_report_viewport(m_watch, 0, 0, 0, 0);
        } else {
            post_apollo_weather_watch_v1_report_viewport(m_watch, x, y, w, h);
        }
    }
    flush();
}

void WatchBridge::close() {
    if (m_watch && m_enabled) {
        if (m_watchCandidate) {
            post_apollo_weather_watch_v1_report_viewport(m_watch, 0, 0, 0, 0);
        }
        post_apollo_weather_watch_v1_close(m_watch);
        flush();
    }
    m_viewportDebounce.stop();
    setWatchCandidate(false);
    setViewportCandidate(false);
    setHostRevision(0);
    setStatus(QStringLiteral("CLOSE_SENT"));
}
