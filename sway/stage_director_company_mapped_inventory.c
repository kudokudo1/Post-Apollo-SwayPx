#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_mapped_inventory.h"

/* Only the compositor stores view_context. A destroyed wl_resource always
 * evicts it; normal XDG unmap also evicts before the sway_view can die. */
static void forget(struct stage_director_company_mapped_slot *slot) {
	if (!slot || !slot->occupied) return;
	wl_list_remove(&slot->root_destroy.link);
	wl_list_init(&slot->root_destroy.link);
	slot->occupied = false;
	slot->root = NULL;
	slot->view_context = NULL;
	slot->selection_epoch = 0;
	slot->intent_sequence = 0;
	slot->first_actor = (struct stage_director_company_native_actor){0};
}
static void root_destroyed(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_company_mapped_slot *slot =
		wl_container_of(listener, slot, root_destroy);
	forget(slot);
}
void stage_director_company_mapped_inventory_init(
		struct stage_director_company_mapped_inventory *inv) {
	if (!inv || inv->initialized) return;
	memset(inv, 0, sizeof(*inv));
	inv->initialized = true;
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		wl_list_init(&inv->slots[i].root_destroy.link);
	}
}
void stage_director_company_mapped_inventory_clear(
		struct stage_director_company_mapped_inventory *inv) {
	if (!inv || !inv->initialized) return;
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		forget(&inv->slots[i]);
	}
}
void stage_director_company_mapped_inventory_forget_view(
		struct stage_director_company_mapped_inventory *inv,
		const void *view_context) {
	if (!inv || !inv->initialized || !view_context) return;
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		if (inv->slots[i].occupied &&
				inv->slots[i].view_context == view_context) {
			forget(&inv->slots[i]);
		}
	}
}
void stage_director_company_mapped_inventory_forget_root(
		struct stage_director_company_mapped_inventory *inv,
		struct wl_resource *root) {
	if (!inv || !inv->initialized || !root) return;
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		if (inv->slots[i].occupied && inv->slots[i].root == root) {
			forget(&inv->slots[i]);
		}
	}
}
bool stage_director_company_mapped_slot_same_actor(
		const struct stage_director_company_mapped_slot *slot,
		const struct stage_director_company_native_actor *fresh) {
	if (!slot || !slot->occupied || !slot->root || !fresh ||
			!fresh->candidate ||
			fresh->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			fresh->native_root_surface_ref != (uintptr_t)slot->root ||
			fresh->native_client_ref !=
				(uintptr_t)wl_resource_get_client(slot->root)) {
		return false;
	}
	const struct stage_director_company_native_actor *first =
		&slot->first_actor;
	return fresh->native_view_ref == first->native_view_ref &&
		fresh->native_root_surface_ref == first->native_root_surface_ref &&
		fresh->native_client_ref == first->native_client_ref &&
		fresh->pid == first->pid && fresh->uid == first->uid &&
		fresh->start_ticks == first->start_ticks;
}
bool stage_director_company_mapped_inventory_remember(
		struct stage_director_company_mapped_inventory *inv,
		struct wl_resource *root, void *view_context,
		uint64_t epoch, uint64_t sequence,
		const struct stage_director_company_native_actor *observed) {
	if (!inv || !inv->initialized || !root || !view_context ||
			!epoch || !sequence || !observed || !observed->candidate ||
			observed->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!observed->native_view_ref || !observed->pid ||
			!observed->uid || !observed->start_ticks ||
			observed->native_root_surface_ref != (uintptr_t)root ||
			observed->native_client_ref !=
				(uintptr_t)wl_resource_get_client(root)) {
		return false;
	}
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		struct stage_director_company_mapped_slot *slot =
			&inv->slots[i];
		if (!slot->occupied) continue;
		if (slot->root == root || slot->view_context == view_context) {
			if (slot->root == root &&
					slot->view_context == view_context &&
					slot->selection_epoch == epoch &&
					slot->intent_sequence == sequence &&
					stage_director_company_mapped_slot_same_actor(
						slot, observed)) {
				return true;
			}
			/* Same root/view recycled with stale intent or identity:
			 * reject, never silently relabel an old window. */
			forget(slot);
			return false;
		}
	}
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i) {
		struct stage_director_company_mapped_slot *slot =
			&inv->slots[i];
		if (slot->occupied) continue;
		slot->root = root;
		slot->view_context = view_context;
		slot->selection_epoch = epoch;
		slot->intent_sequence = sequence;
		slot->first_actor = *observed;
		slot->occupied = true;
		slot->root_destroy.notify = root_destroyed;
		wl_resource_add_destroy_listener(root, &slot->root_destroy);
		return true;
	}
	/* Full inventory refuses extra candidates. Never evict an unrelated
	 * live root or silently promote one matching app_id. */
	return false;
}
const struct stage_director_company_mapped_slot *
stage_director_company_mapped_inventory_at(
		const struct stage_director_company_mapped_inventory *inv,
		size_t index) {
	if (!inv || !inv->initialized ||
			index >= SD_COMPANY_M30C_MAX_MAPPED ||
			!inv->slots[index].occupied) return NULL;
	return &inv->slots[index];
}
