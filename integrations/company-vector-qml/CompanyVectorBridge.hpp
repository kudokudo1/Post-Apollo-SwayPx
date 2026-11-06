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
struct post_apollo_company_vector_v1;

/*
 * M25 Company-only private Wayland client. Borrow Qt/Quickshell QPA display.
 *
 * Disabled by default. No direct wl_display_connect/disconnect, no launch,
 * no compositor scene, no focus/seat authorization and no Weather WATCH use.
 *
 * After the compositor physically pairs the *real Company layer* (M24),
 * the M23 private global appears to THIS exact wl_client. Binding reports
 * only candidates; not native Qt MapLibre actor custody or embedded pixels.
 */
class CompanyVectorBridge : public QObject {
	Q_OBJECT
	QML_NAMED_ELEMENT(CompanyVectorBridge)
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(QRectF frameRect READ frameRect WRITE setFrameRect NOTIFY frameRectChanged)
	Q_PROPERTY(QRectF mapRect READ mapRect WRITE setMapRect NOTIFY mapRectChanged)
	Q_PROPERTY(bool wireBound READ wireBound NOTIFY wireBoundChanged)
	Q_PROPERTY(bool launchCandidate READ launchCandidate NOTIFY launchCandidateChanged)
	Q_PROPERTY(bool viewportCandidate READ viewportCandidate NOTIFY viewportCandidateChanged)
	Q_PROPERTY(bool kernelBirthCandidate READ kernelBirthCandidate NOTIFY kernelBirthCandidateChanged)
	Q_PROPERTY(QString status READ status NOTIFY statusChanged)
	Q_PROPERTY(qulonglong epoch READ epoch NOTIFY epochChanged)
	Q_PROPERTY(qulonglong hostRevision READ hostRevision NOTIFY hostRevisionChanged)

public:
	explicit CompanyVectorBridge(QObject *parent = nullptr);
	~CompanyVectorBridge() override;

	bool enabled() const { return m_enabled; }
	QRectF frameRect() const { return m_frameRect; }
	QRectF mapRect() const { return m_mapRect; }
	bool wireBound() const { return m_vector != nullptr; }
	bool launchCandidate() const { return m_launchCandidate; }
	bool viewportCandidate() const { return m_viewportCandidate; }
	bool kernelBirthCandidate() const { return m_kernelBirthCandidate; }
	QString status() const { return m_status; }
	qulonglong epoch() const { return m_epoch; }
	qulonglong hostRevision() const { return m_hostRevision; }

	void setEnabled(bool enabled);
	void setFrameRect(const QRectF &rect);
	void setMapRect(const QRectF &rect);

	/* Only SENDs a fixed offline/online intent to paired native Company.
	 * No shell command, process, map open, or online consent is created here. */
	Q_INVOKABLE bool requestLaunch(bool online);
	/* Only Quickshell's attached Process.processId belongs here. C++ opens
	 * a real pidfd, transmits fd on existing Qt QPA Wayland connection.
	 * This does not authenticate the M15 executable or grant scene rights. */
	Q_INVOKABLE bool reportNativeChildPid(qlonglong processId);
	Q_INVOKABLE void close();

signals:
	void enabledChanged();
	void frameRectChanged();
	void mapRectChanged();
	void wireBoundChanged();
	void launchCandidateChanged();
	void viewportCandidateChanged();
	void kernelBirthCandidateChanged();
	void statusChanged();
	void epochChanged();
	void hostRevisionChanged();

private:
	static void registryGlobal(void *data, wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version);
	static void registryRemoved(void *data, wl_registry *registry,
		uint32_t name);
	static void vectorBirthResult(void *data,
		post_apollo_company_vector_v1 *, uint32_t status);
	static void vectorResult(void *data, post_apollo_company_vector_v1 *,
		uint32_t status, uint32_t epochHi, uint32_t epochLo,
		uint32_t revisionHi, uint32_t revisionLo);

	void start();
	void stop();
	void pump();
	void flush();
	void sendViewport();
	void releaseVector();
	void setStatus(const QString &status);
	void setLaunchCandidate(bool candidate);
	void setViewportCandidate(bool candidate);
	void setKernelBirthCandidate(bool candidate);
	void tryReportBirth();
	void setEpoch(qulonglong epoch);
	void setHostRevision(qulonglong revision);

	bool m_enabled = false;
	bool m_launchCandidate = false;
	bool m_kernelBirthCandidate = false;
	bool m_birthSent = false;
	qlonglong m_pendingChildPid = 0;
	bool m_viewportCandidate = false;
	QRectF m_frameRect;
	QRectF m_mapRect;
	QString m_status = QStringLiteral("DISABLED");
	qulonglong m_epoch = 0;
	qulonglong m_hostRevision = 0;
	uint32_t m_globalName = 0;

	/* Owned Qt QPA display, borrowed: do not close its Wayland socket. */
	wl_display *m_display = nullptr;
	wl_event_queue *m_queue = nullptr;
	wl_registry *m_registry = nullptr;
	post_apollo_company_vector_v1 *m_vector = nullptr;
	QTimer m_queuePump;
	QTimer m_geometryDebounce;
};
