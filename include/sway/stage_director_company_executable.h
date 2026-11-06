#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_EXECUTABLE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_EXECUTABLE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_birth_pidfd.h"

/*
 * M29 // READ-ONLY runtime executable inode evidence for the M15 map.
 *
 * The approved M15 chain ends with execve(owner home/
 * postapollo-vector-lab/company-vector-build/company-vector-probe).
 * A matching inode proves the observed process is executing that currently
 * installed file, not that its content was pinned, independently approved,
 * signed, or the *intended* Company click's legitimate spawn.
 *
 * No scene, seat, actor lease, or executable-approval authority derives from
 * a successful match.
 */
enum stage_director_company_executable_status {
	SD_COMPANY_EXE_NO_PIDFD,
	SD_COMPANY_EXE_PROCESS_CHANGED,
	SD_COMPANY_EXE_HOME_UNAVAILABLE,
	SD_COMPANY_EXE_UNSAFE_INSTALL,
	SD_COMPANY_EXE_RUNNING_EXE_UNAVAILABLE,
	SD_COMPANY_EXE_DIFFERENT_INODE,
	SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE,
};

struct stage_director_company_executable_result {
	enum stage_director_company_executable_status status;
	bool running_current_install_inode_candidate;
	bool trusted_content_digest_verified;
	bool executable_approved;
	bool launch_authenticated;
	bool owner_verified;
	bool can_pair_actor;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_focus;
	bool can_show_popup;
};

/* The directory argument is compositor-private, NEVER supplied by Wayland,
 * QML, app_id or command line. For tests only, supply a synthetic owner home.
 * The system wrapper obtains its path from getpwuid_r(held uid).
 */
struct stage_director_company_executable_result
stage_director_company_executable_check_owner_home(
	const struct stage_director_company_birth_pidfd *held,
	const char *owner_home);

struct stage_director_company_executable_result
stage_director_company_executable_check_system_home(
	const struct stage_director_company_birth_pidfd *held);

const char *stage_director_company_executable_status_name(
	enum stage_director_company_executable_status state);

#endif
