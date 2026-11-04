#pragma once

#include <QObject>
#include <QRectF>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>
#include <cstdint>

struct wl_display;
struct wl_event_queue;
struct wl_registry;
struct post_apollo_weather_watch_v1;

/*
 * Opt-in QML client adapter to SwayPX's paired WATCH v2 protocol.
 *
 * This QObject runs inside the SAME Qt/Quickshell process as the real Weather
 * layer-shell wl_surface. It borrows the QPA Wayland display; never calls
 * wl_display_connect/disconnect, never spawns a browser, and cannot grant
 * scene, popup, focus or input authority.
 *
 * Do NOT instantiate it in a separate helper process: the server requires
 * identity of the exact Wayland client that owns the paired Weather layer.
 */
class WatchBridge : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(WatchBridge)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QRectF viewport READ viewport WRITE setViewport NOTIFY viewportChanged)
    Q_PROPERTY(bool paired READ paired NOTIFY pairedChanged)
    Q_PROPERTY(bool watchCandidate READ watchCandidate NOTIFY watchCandidateChanged)
    Q_PROPERTY(bool viewportCandidate READ viewportCandidate NOTIFY viewportCandidateChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(qulonglong hostRevision READ hostRevision NOTIFY hostRevisionChanged)

public:
    explicit WatchBridge(QObject *parent = nullptr);
    ~WatchBridge() override;

    bool enabled() const { return m_enabled; }
    QRectF viewport() const { return m_viewport; }
    bool paired() const { return m_watch != nullptr; }
    bool watchCandidate() const { return m_watchCandidate; }
    bool viewportCandidate() const { return m_viewportCandidate; }
    QString status() const { return m_status; }
    qulonglong hostRevision() const { return m_hostRevision; }

    void setEnabled(bool enabled);
    void setViewport(const QRectF &localRect);

    // Returns only whether a request was SENT, not whether it was accepted.
    // Only fixed catalog tokens accepted. No URL, PID or shell command.
    Q_INVOKABLE bool requestWatch(const QString &sourceId);
    Q_INVOKABLE void close();

signals:
    void enabledChanged();
    void viewportChanged();
    void pairedChanged();
    void watchCandidateChanged();
    void viewportCandidateChanged();
    void statusChanged();
    void hostRevisionChanged();

private:
    static void registryGlobal(void *, wl_registry *, uint32_t,
                               const char *, uint32_t);
    static void registryRemoved(void *, wl_registry *, uint32_t);
    static void watchResult(void *, post_apollo_weather_watch_v1 *, uint32_t);
    static void viewportResult(void *, post_apollo_weather_watch_v1 *,
                               uint32_t, uint32_t, uint32_t);

    void start();
    void stop();
    void pump();
    void flush();
    void sendViewport();
    void setStatus(const QString &status);
    void setWatchCandidate(bool candidate);
    void setViewportCandidate(bool candidate);
    void setHostRevision(qulonglong revision);
    void releaseWatch();

    bool m_enabled = false;
    QRectF m_viewport;
    QString m_status = QStringLiteral("DISABLED");
    bool m_watchCandidate = false;
    bool m_viewportCandidate = false;
    qulonglong m_hostRevision = 0;
    uint32_t m_watchGlobalName = 0;
    wl_display *m_display = nullptr;  // BORROWED from Qt QPA
    wl_event_queue *m_queue = nullptr;
    wl_registry *m_registry = nullptr;
    post_apollo_weather_watch_v1 *m_watch = nullptr;
    QTimer m_queuePump;
    QTimer m_viewportDebounce;
};
