#ifndef _SWAY_STAGE_DIRECTOR_NEWS_SESSION_LEASE_H
#define _SWAY_STAGE_DIRECTOR_NEWS_SESSION_LEASE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_process_evidence.h"

/*
 * SD1 // compositor-local NEWS session candidate ledger.
 *
 * This is a PRODUCTION-BUILT, INERT CORE for a future authorized broker.
 * No live SwayPX callback currently creates or consumes this ledger.
 * The words "candidate" and "record" are deliberate: caller-supplied
 * native references are NOT a trusted WATCH gesture, browser launch proof
 * or an unforgeable Wayland resource binding.
 *
 * Only a future separately authorized compositor owner may feed this API,
 * using in-process live native objects and an independently authenticated
 * operator WATCH origin. NEVER expose these functions as public IPC or let
 * QML supply native pointers, process identities or an epoch credential.
 */
enum stage_director_news_lease_state {
	SD_NEWS_LEASE_EMPTY,
	SD_NEWS_LEASE_HOST_CANDIDATE,
	SD_NEWS_LEASE_PROCESS_CANDIDATE,
	SD_NEWS_LEASE_ACTOR_CANDIDATE,
};

struct stage_director_news_session_lease {
	enum stage_director_news_lease_state state;
	uint64_t last_epoch; /* Retained after revoke/close to reject reuse. */
	uint64_t epoch;
	uintptr_t native_host_ref;
	uintptr_t native_host_client_ref;
	uintptr_t native_output_ref;
	uintptr_t native_actor_ref;
	uintptr_t native_actor_client_ref;
	uintptr_t native_root_surface_ref;
	struct stage_director_process_launch_root launch_root;
	int64_t actor_pid;
	uint64_t actor_start;
	char source_id[41]; /* copied allowlisted ID, not caller URL */
};

struct stage_director_news_lease_observation {
	uint64_t epoch;
	uintptr_t native_host_ref;
	uintptr_t native_host_client_ref;
	uintptr_t native_output_ref;
	uintptr_t native_actor_ref;
	uintptr_t native_actor_client_ref;
	uintptr_t native_root_surface_ref;
	int64_t native_wayland_client_pid;
	uint64_t native_wayland_client_start;
	bool exact_xdg_toplevel_root;
};

enum stage_director_news_lease_probe_state {
	SD_NEWS_LEASE_PROBE_NO_SESSION,
	SD_NEWS_LEASE_PROBE_NOT_BOUND,
	SD_NEWS_LEASE_PROBE_MISMATCH,
	SD_NEWS_LEASE_PROBE_PROCESS_UNSTABLE,
	SD_NEWS_LEASE_PROBE_MATCHED_CANDIDATE_ONLY,
};

struct stage_director_news_lease_probe {
	enum stage_director_news_lease_probe_state state;
	bool candidate_correlated;
	bool operator_watch_authenticated;
	bool browser_launch_authorized;
	bool owner_verified;
	bool can_launch;
	bool can_bind_viewer;
	bool can_clip;
	bool can_render;
	bool can_accept_input;
	bool can_show_popup;
};

/* Initialize once for a compositor-owner lifetime; not a persistent ticket. */
void stage_director_news_session_lease_init(
		struct stage_director_news_session_lease *lease);

/*
 * Reserve exactly one prospective host/source/epoch.
 * This does NOT attest WATCH origin. Actual authenticated admission must
 * precede a future real call from a private compositor callback.
 */
bool stage_director_news_session_lease_reserve_candidate(
		struct stage_director_news_session_lease *lease,
		uint64_t epoch, uintptr_t native_host_ref,
		uintptr_t native_host_client_ref, uintptr_t native_output_ref,
		const char *registered_source_id);

/* Record an untrusted process candidate, NOT a production spawn credential. */
bool stage_director_news_session_lease_record_process_candidate(
		struct stage_director_news_session_lease *lease,
		uint64_t epoch, const struct stage_director_process_launch_root *root);

/*
 * Correlate the root to the native actor client's LIVE Linux ancestry
 * and exact XDG root-surface candidate. Does not grant scene privileges.
 */
bool stage_director_news_session_lease_record_actor_candidate(
		struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *observation,
		stage_director_process_reader reader, void *context);

/* Require exact fresh native refs and process evidence on every probe. */
struct stage_director_news_lease_probe stage_director_news_session_lease_probe(
		const struct stage_director_news_session_lease *lease,
		const struct stage_director_news_lease_observation *observation,
		stage_director_process_reader reader, void *context);

/*
 * All closing paths must leave old epochs non-reusable. No process killing,
 * focus change, rendering, IPC or browser profile removal occurs here.
 */
bool stage_director_news_session_lease_close(
		struct stage_director_news_session_lease *lease, uint64_t epoch);
bool stage_director_news_session_lease_host_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_host_ref);
bool stage_director_news_session_lease_actor_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_actor_ref);
bool stage_director_news_session_lease_output_lost(
		struct stage_director_news_session_lease *lease,
		uintptr_t native_output_ref);
bool stage_director_news_session_lease_process_lost(
		struct stage_director_news_session_lease *lease, int64_t pid);

#endif
