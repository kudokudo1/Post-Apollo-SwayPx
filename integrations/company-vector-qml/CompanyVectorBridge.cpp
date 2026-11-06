#include "CompanyVectorBridge.hpp"

#include <QGuiApplication>
#include <QtGlobal>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <limits>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client.h>
#include "post-apollo-company-vector-v1-client-protocol.h"

CompanyVectorBridge::CompanyVectorBridge(QObject *parent) : QObject(parent) {
	m_queuePump.setInterval(40);
	connect(&m_queuePump, &QTimer::timeout,
		this, &CompanyVectorBridge::pump);
	m_geometryDebounce.setSingleShot(true);
	m_geometryDebounce.setInterval(25);
	connect(&m_geometryDebounce, &QTimer::timeout,
		this, &CompanyVectorBridge::sendViewport);
}

CompanyVectorBridge::~CompanyVectorBridge() {
	stop();
}

void CompanyVectorBridge::setStatus(const QString &value) {
	if (m_status == value) return;
	m_status = value;
	emit statusChanged();
}

void CompanyVectorBridge::setLaunchCandidate(bool value) {
	if (m_launchCandidate == value) return;
	m_launchCandidate = value;
	emit launchCandidateChanged();
}

void CompanyVectorBridge::setViewportCandidate(bool value) {
	if (m_viewportCandidate == value) return;
	m_viewportCandidate = value;
	emit viewportCandidateChanged();
}

void CompanyVectorBridge::setKernelBirthCandidate(bool value) {
	if (m_kernelBirthCandidate == value) return;
	m_kernelBirthCandidate = value;
	emit kernelBirthCandidateChanged();
}

void CompanyVectorBridge::setEpoch(qulonglong value) {
	if (m_epoch == value) return;
	m_epoch = value;
	emit epochChanged();
}

void CompanyVectorBridge::setHostRevision(qulonglong value) {
	if (m_hostRevision == value) return;
	m_hostRevision = value;
	emit hostRevisionChanged();
}

void CompanyVectorBridge::setEnabled(bool value) {
	if (m_enabled == value) return;
	m_enabled = value;
	emit enabledChanged();
	if (value) start();
	else stop();
}

void CompanyVectorBridge::setFrameRect(const QRectF &rect) {
	if (m_frameRect == rect) return;
	m_frameRect = rect;
	emit frameRectChanged();
	if (m_launchCandidate) {
		m_geometryDebounce.start();
	}
}

void CompanyVectorBridge::setMapRect(const QRectF &rect) {
	if (m_mapRect == rect) return;
	m_mapRect = rect;
	emit mapRectChanged();
	if (m_launchCandidate) {
		/* Hidden viewport revokes promptly, not after a debounce. */
		if (rect.isEmpty()) {
			m_geometryDebounce.stop();
			sendViewport();
		} else {
			m_geometryDebounce.start();
		}
	}
}

void CompanyVectorBridge::start() {
	/* MUST be the SAME wl_client as Quickshell's Company layer. Do not
	 * open a second connection or obtain Wayland FD through an IPC helper. */
	if (!qGuiApp) {
		setStatus(QStringLiteral("NO_QT_GUI_APPLICATION"));
		return;
	}
	auto *native =
		qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
	if (!native || !native->display()) {
		setStatus(QStringLiteral("NOT_QT_WAYLAND"));
		return;
	}
	m_display = native->display(); // BORROWED, never disconnect
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
	/* Only this registry/protocol proxy uses our dispatch queue.
	 * Qt QPA continues to own socket polling and event reads. */
	wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_registry), m_queue);
	static const wl_registry_listener listener = {
		&CompanyVectorBridge::registryGlobal,
		&CompanyVectorBridge::registryRemoved
	};
	wl_registry_add_listener(m_registry, &listener, this);
	setStatus(QStringLiteral("AWAITING_NATIVE_COMPANY_SELECTION"));
	m_queuePump.start();
	flush();
}

void CompanyVectorBridge::stop() {
	m_queuePump.stop();
	m_geometryDebounce.stop();
	if (m_vector) {
		/* The server independently invalidates geometry when its binding
		 * is destroyed, host role disappears, or selection epoch changes. */
		if (m_launchCandidate) {
			post_apollo_company_vector_v1_report_viewport(
				m_vector, 0, 0, 0, 0, 0, 0, 0, 0);
		}
		post_apollo_company_vector_v1_close(m_vector);
	}
	flush();
	releaseVector();
	if (m_registry) {
		wl_registry_destroy(m_registry);
		m_registry = nullptr;
	}
	flush();
	if (m_queue) {
		wl_event_queue_destroy(m_queue);
		m_queue = nullptr;
	}
	m_display = nullptr;
	m_globalName = 0;
	setStatus(QStringLiteral("DISABLED"));
}

void CompanyVectorBridge::flush() {
	if (!m_display) return;
	const int rc = wl_display_flush(m_display);
	if (rc < 0 && errno != EAGAIN) {
		setStatus(QStringLiteral("WAYLAND_FLUSH_ERROR"));
	}
}

void CompanyVectorBridge::pump() {
	if (!m_display || !m_queue) return;
	if (wl_display_dispatch_queue_pending(m_display, m_queue) < 0) {
		setLaunchCandidate(false);
		setViewportCandidate(false);
		setEpoch(0);
		setHostRevision(0);
		setStatus(QStringLiteral("WAYLAND_DISCONNECTED"));
		m_queuePump.stop();
		return;
	}
	flush();
}

void CompanyVectorBridge::registryGlobal(
		void *data, wl_registry *registry, uint32_t name,
		const char *interface, uint32_t version) {
	auto *self = static_cast<CompanyVectorBridge *>(data);
	if (!self || !self->m_enabled || self->m_vector ||
			!interface ||
			std::strcmp(interface,
				post_apollo_company_vector_v1_interface.name) != 0) {
		return;
	}
	if (version < 1) {
		self->setStatus(QStringLiteral("COMPANY_WIRE_V1_REQUIRED"));
		return;
	}
	self->m_vector = static_cast<post_apollo_company_vector_v1 *>(
		wl_registry_bind(registry, name,
			&post_apollo_company_vector_v1_interface, version >= 2 ? 2 : 1));
	if (!self->m_vector) {
		self->setStatus(QStringLiteral("COMPANY_WIRE_BIND_FAILED"));
		return;
	}
	self->m_globalName = name;
	wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(self->m_vector),
		self->m_queue);
	static const post_apollo_company_vector_v1_listener listener = {
		&CompanyVectorBridge::vectorResult,
		&CompanyVectorBridge::vectorBirthResult
	};
	if (post_apollo_company_vector_v1_add_listener(
			self->m_vector, &listener, self) != 0) {
		self->releaseVector();
		self->setStatus(QStringLiteral("COMPANY_WIRE_LISTENER_FAILED"));
		return;
	}
	emit self->wireBoundChanged();
	self->setStatus(QStringLiteral("COMPANY_WIRE_BOUND_CANDIDATE_ONLY"));
	self->flush();
}

void CompanyVectorBridge::registryRemoved(
		void *data, wl_registry *, uint32_t name) {
	auto *self = static_cast<CompanyVectorBridge *>(data);
	if (!self || name != self->m_globalName) return;
	self->releaseVector();
	self->setStatus(QStringLiteral("COMPANY_PAIRING_REVOKED"));
}

void CompanyVectorBridge::releaseVector() {
	const bool wasBound = m_vector != nullptr;
	if (m_vector) {
		post_apollo_company_vector_v1_destroy(m_vector);
		m_vector = nullptr;
	}
	m_globalName = 0;
	setLaunchCandidate(false);
	setViewportCandidate(false);
	setKernelBirthCandidate(false);
	m_birthSent = false;
	m_pendingChildPid = 0;
	setEpoch(0);
	setHostRevision(0);
	if (wasBound) emit wireBoundChanged();
}

void CompanyVectorBridge::vectorResult(void *data,
		post_apollo_company_vector_v1 *,
		uint32_t status, uint32_t epochHi, uint32_t epochLo,
		uint32_t revisionHi, uint32_t revisionLo) {
	auto *self = static_cast<CompanyVectorBridge *>(data);
	if (!self) return;
	self->setEpoch((static_cast<qulonglong>(epochHi) << 32) |
		static_cast<qulonglong>(epochLo));
	self->setHostRevision((static_cast<qulonglong>(revisionHi) << 32) |
		static_cast<qulonglong>(revisionLo));
	switch (status) {
	case POST_APOLLO_COMPANY_VECTOR_V1_STATUS_LAUNCH_CANDIDATE:
		self->setLaunchCandidate(true);
		self->setViewportCandidate(false);
		self->setStatus(QStringLiteral("LAUNCH_INTENT_CANDIDATE_ONLY"));
		self->tryReportBirth();
		self->sendViewport();
		break;
	case POST_APOLLO_COMPANY_VECTOR_V1_STATUS_VIEWPORT_CANDIDATE:
		self->setViewportCandidate(true);
		self->setStatus(QStringLiteral("VIEWPORT_CANDIDATE_ONLY"));
		break;
	case POST_APOLLO_COMPANY_VECTOR_V1_STATUS_HIDDEN:
		self->setViewportCandidate(false);
		self->setStatus(QStringLiteral("VIEWPORT_HIDDEN"));
		break;
	case POST_APOLLO_COMPANY_VECTOR_V1_STATUS_CLOSED:
		self->setLaunchCandidate(false);
		self->setViewportCandidate(false);
		self->setStatus(QStringLiteral("COMPANY_WIRE_CLOSED"));
		break;
	case POST_APOLLO_COMPANY_VECTOR_V1_STATUS_INVALID_GEOMETRY:
		self->setViewportCandidate(false);
		self->setStatus(QStringLiteral("INVALID_COMPANY_GEOMETRY"));
		break;
	default:
		self->setLaunchCandidate(false);
		self->setViewportCandidate(false);
		self->setStatus(QStringLiteral("COMPANY_WIRE_DENIED"));
	}
}

void CompanyVectorBridge::vectorBirthResult(void *data,
		post_apollo_company_vector_v1 *, uint32_t status) {
	auto *self = static_cast<CompanyVectorBridge *>(data);
	if (!self) return;
	const bool held =
		status == POST_APOLLO_COMPANY_VECTOR_V1_BIRTH_STATUS_KERNEL_CANDIDATE;
	self->setKernelBirthCandidate(held);
	self->setStatus(held
		? QStringLiteral("KERNEL_PIDFD_CANDIDATE_ONLY")
		: QStringLiteral("KERNEL_PIDFD_DENIED"));
}

/* This PID must originate from Quickshell's attached QProcess.processId,
 * not a user-entered ID. C++ asks Linux for an actual pidfd, and transmits
 * it via the SAME QPA wl_client that owns Company's physical layer.
 * No secret in environment, argv, disk or a separate Wayland connection. */
bool CompanyVectorBridge::reportNativeChildPid(qlonglong processId) {
	if (!m_enabled || !m_vector || processId <= 0 ||
			processId > std::numeric_limits<int32_t>::max() ||
			m_birthSent || m_pendingChildPid != 0 ||
			wl_proxy_get_version(reinterpret_cast<wl_proxy *>(m_vector)) < 2) {
		return false;
	}
	m_pendingChildPid = processId;
	tryReportBirth();
	return true;
}

void CompanyVectorBridge::tryReportBirth() {
	if (!m_enabled || !m_vector || !m_launchCandidate ||
			m_birthSent || m_pendingChildPid <= 0 ||
			wl_proxy_get_version(reinterpret_cast<wl_proxy *>(m_vector)) < 2) {
		return;
	}
#if defined(SYS_pidfd_open)
	const int fd = static_cast<int>(syscall(SYS_pidfd_open,
		static_cast<pid_t>(m_pendingChildPid), 0));
	if (fd < 0) {
		setStatus(QStringLiteral("CHILD_PIDFD_UNAVAILABLE"));
		return;
	}
	post_apollo_company_vector_v1_report_child_pidfd(
		m_vector, static_cast<uint32_t>(m_pendingChildPid), fd);
	/* Wayland marshals and transfers the fd; this client-owned fd is
	 * closed immediately. The compositor will own a separate CLOEXEC dup
	 * only after rechecking native host, intent and kernel identity. */
	::close(fd);
	m_birthSent = true;
	setStatus(QStringLiteral("CHILD_PIDFD_SENT_UNCONFIRMED"));
	flush();
#else
	setStatus(QStringLiteral("CHILD_PIDFD_NOT_SUPPORTED"));
#endif
}

bool CompanyVectorBridge::requestLaunch(bool online) {
	if (!m_enabled || !m_vector || m_launchCandidate) return false;
	const uint32_t mode = online
		? POST_APOLLO_COMPANY_VECTOR_V1_MODE_ONLINE
		: POST_APOLLO_COMPANY_VECTOR_V1_MODE_OFFLINE;
	post_apollo_company_vector_v1_launch_intent(m_vector, mode);
	setStatus(QStringLiteral("COMPANY_INTENT_SENT_UNCONFIRMED"));
	flush();
	return true;
}

static bool integer_rect(const QRectF &r, int32_t &x, int32_t &y,
		int32_t &w, int32_t &h) {
	const qreal max = static_cast<qreal>(
		std::numeric_limits<int32_t>::max() - 1);
	if (!std::isfinite(r.x()) || !std::isfinite(r.y()) ||
			!std::isfinite(r.width()) ||
			!std::isfinite(r.height()) ||
			r.x() < 0 || r.y() < 0 ||
			r.width() <= 0 || r.height() <= 0 ||
			r.x() > max || r.y() > max ||
			r.width() > max || r.height() > max) {
		return false;
	}
	x = qRound(r.x());
	y = qRound(r.y());
	w = qRound(r.width());
	h = qRound(r.height());
	return w > 0 && h > 0;
}

void CompanyVectorBridge::sendViewport() {
	if (!m_enabled || !m_vector || !m_launchCandidate) return;
	int32_t fx=0, fy=0, fw=0, fh=0, mx=0, my=0, mw=0, mh=0;
	if (integer_rect(m_frameRect, fx, fy, fw, fh) &&
			integer_rect(m_mapRect, mx, my, mw, mh) &&
			static_cast<int64_t>(mx) + mw <= fw &&
			static_cast<int64_t>(my) + mh <= fh) {
		post_apollo_company_vector_v1_report_viewport(m_vector,
			fx, fy, fw, fh, mx, my, mw, mh);
	} else {
		/* Fail closed: zero ALL eight ints. Do not send malformed QML
		 * geometry as a meaningful candidate or retain an old crop. */
		post_apollo_company_vector_v1_report_viewport(m_vector,
			0, 0, 0, 0, 0, 0, 0, 0);
	}
	setViewportCandidate(false);
	flush();
}

void CompanyVectorBridge::close() {
	m_geometryDebounce.stop();
	m_pendingChildPid = 0;
	m_birthSent = false;
	setKernelBirthCandidate(false);
	if (m_enabled && m_vector) {
		if (m_launchCandidate) {
			post_apollo_company_vector_v1_report_viewport(
				m_vector, 0, 0, 0, 0, 0, 0, 0, 0);
		}
		post_apollo_company_vector_v1_close(m_vector);
		flush();
	}
	setLaunchCandidate(false);
	setViewportCandidate(false);
	setStatus(QStringLiteral("COMPANY_CLOSE_SENT"));
}
