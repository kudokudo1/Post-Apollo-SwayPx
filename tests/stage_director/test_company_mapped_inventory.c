#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_company_mapped_inventory.h"

struct fixture {
	struct wl_display *display;
	struct wl_client *client;
	int fd;
	struct stage_director_company_mapped_inventory inv;
};
static struct fixture fixture(void) {
	struct fixture f = {0};
	f.display = wl_display_create();
	assert(f.display);
	int fds[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
	f.client = wl_client_create(f.display, fds[0]);
	assert(f.client);
	f.fd = fds[1];
	stage_director_company_mapped_inventory_init(&f.inv);
	return f;
}
static struct wl_resource *resource(struct fixture *f) {
	struct wl_resource *r =
		wl_resource_create(f->client, &wl_surface_interface, 1, 0);
	assert(r);
	return r;
}
static struct stage_director_company_native_actor actor(
		struct fixture *f, struct wl_resource *root, int64_t pid) {
	return (struct stage_director_company_native_actor){
		.candidate = true,
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_view_ref = (uintptr_t)(pid * 16),
		.native_root_surface_ref = (uintptr_t)root,
		.native_client_ref = (uintptr_t)f->client,
		.pid = pid, .uid = 1000, .start_ticks = 100,
		.root_global = {100,100,800,500},
	};
}
static size_t count(const struct fixture *f) {
	size_t n = 0;
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED; ++i)
		if (stage_director_company_mapped_inventory_at(&f->inv, i)) ++n;
	return n;
}
static void finish(struct fixture *f) {
	stage_director_company_mapped_inventory_clear(&f->inv);
	wl_display_destroy_clients(f->display);
	assert(close(f->fd) == 0);
	wl_display_destroy(f->display);
}
static void lifecycle_and_stale_identity(void) {
	struct fixture f = fixture();
	int view = 1;
	struct wl_resource *r = resource(&f);
	struct stage_director_company_native_actor a = actor(&f, r, 2301);
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r, &view, 41, 77, &a));
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r, &view, 41, 77, &a));
	assert(count(&f) == 1);
	const struct stage_director_company_mapped_slot *slot =
		stage_director_company_mapped_inventory_at(&f.inv, 0);
	assert(slot && slot->root == r && slot->view_context == &view);
	assert(stage_director_company_mapped_slot_same_actor(slot, &a));
	a.start_ticks++;
	assert(!stage_director_company_mapped_slot_same_actor(slot, &a));
	/* An old root cannot be relabeled for a new process or intent. */
	assert(!stage_director_company_mapped_inventory_remember(
		&f.inv, r, &view, 41, 77, &a));
	assert(count(&f) == 0);
	a.start_ticks--;
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r, &view, 41, 77, &a));
	/* Explicit unmap forgets the view while its wl_resource survives. */
	stage_director_company_mapped_inventory_forget_view(&f.inv, &view);
	assert(count(&f) == 0);
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r, &view, 41, 77, &a));
	/* Real native root destruction clears any remembered sway_view pointer. */
	wl_resource_destroy(r);
	assert(count(&f) == 0);
	finish(&f);
}
static void duplicate_and_foreign_roots(void) {
	struct fixture f = fixture();
	int first = 1, second = 2;
	struct wl_resource *r1 = resource(&f), *r2 = resource(&f);
	struct stage_director_company_native_actor a1 = actor(&f, r1, 2301);
	struct stage_director_company_native_actor a2 = actor(&f, r2, 2302);
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r1, &first, 7, 8, &a1));
	assert(!stage_director_company_mapped_inventory_remember(
		&f.inv, r2, &first, 7, 8, &a2));
	/* A conflicting root for the same view MUST revoke the previous
	 * candidate, not silently retain a possibly stale pointer. */
	assert(count(&f) == 0);
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r1, &first, 7, 8, &a1));
	a2.native_client_ref = (uintptr_t)r1; // Not the owning wl_client
	assert(!stage_director_company_mapped_inventory_remember(
		&f.inv, r2, &second, 7, 8, &a2));
	a2.native_client_ref = (uintptr_t)f.client;
	assert(stage_director_company_mapped_inventory_remember(
		&f.inv, r2, &second, 7, 8, &a2));
	assert(count(&f) == 2);
	stage_director_company_mapped_inventory_forget_root(&f.inv, r1);
	assert(count(&f) == 1);
	stage_director_company_mapped_inventory_clear(&f.inv);
	assert(count(&f) == 0);
	finish(&f);
}
static void bounded_and_destroy_client(void) {
	struct fixture f = fixture();
	struct wl_resource *roots[SD_COMPANY_M30C_MAX_MAPPED + 1];
	int views[SD_COMPANY_M30C_MAX_MAPPED + 1] = {0};
	for (size_t i = 0; i < SD_COMPANY_M30C_MAX_MAPPED + 1; ++i) {
		roots[i] = resource(&f);
		struct stage_director_company_native_actor a =
			actor(&f, roots[i], 3000 + (int)i);
		assert(stage_director_company_mapped_inventory_remember(
			&f.inv, roots[i], &views[i], 10, 20, &a) ==
			(i < SD_COMPANY_M30C_MAX_MAPPED));
	}
	assert(count(&f) == SD_COMPANY_M30C_MAX_MAPPED);
	wl_client_destroy(f.client);
	assert(count(&f) == 0);
	finish(&f);
}
int main(void) {
	lifecycle_and_stale_identity();
	duplicate_and_foreign_roots();
	bounded_and_destroy_client();
	puts("STAGE_DIRECTOR_COMPANY_M30C: PASS bounded native map inventory, client/root destruction, unmap and generation refusal; authority=NONE");
	return 0;
}
