#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_MAPPED_INVENTORY_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_MAPPED_INVENTORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_native.h"

/*
 * M30c: bounded, compositor-owned XDG map inventory. Stores an opaque
 * sway_view pointer ONLY until explicit unmap or wl_resource destruction.
 * The caller must REOBSERVE the real view before every use; the snapshot
 * and a map-time pointer are NEVER sufficient for custody.
 * No client-supplied app_id, title, namespace, PID or public IPC key.
 */
#define SD_COMPANY_M30C_MAX_MAPPED 12

struct stage_director_company_mapped_slot {
	bool occupied;
	struct wl_resource *root;
	void *view_context;
	uint64_t selection_epoch;
	uint64_t intent_sequence;
	struct stage_director_company_native_actor first_actor;
	struct wl_listener root_destroy;
};

struct stage_director_company_mapped_inventory {
	bool initialized;
	struct stage_director_company_mapped_slot
		slots[SD_COMPANY_M30C_MAX_MAPPED];
};

void stage_director_company_mapped_inventory_init(
	struct stage_director_company_mapped_inventory *inv);
void stage_director_company_mapped_inventory_clear(
	struct stage_director_company_mapped_inventory *inv);
void stage_director_company_mapped_inventory_forget_view(
	struct stage_director_company_mapped_inventory *inv, const void *view_context);
void stage_director_company_mapped_inventory_forget_root(
	struct stage_director_company_mapped_inventory *inv, struct wl_resource *root);

bool stage_director_company_mapped_inventory_remember(
	struct stage_director_company_mapped_inventory *inv,
	struct wl_resource *root, void *view_context,
	uint64_t selection_epoch, uint64_t intent_sequence,
	const struct stage_director_company_native_actor *observed);

const struct stage_director_company_mapped_slot *
stage_director_company_mapped_inventory_at(
	const struct stage_director_company_mapped_inventory *inv, size_t index);

bool stage_director_company_mapped_slot_same_actor(
	const struct stage_director_company_mapped_slot *slot,
	const struct stage_director_company_native_actor *fresh);

#endif
