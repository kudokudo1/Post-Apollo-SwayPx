#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_company_vector_protocol.h"
#include "sway/stage_director_company_wire_policy.h"

struct fixture {
	struct wl_display *display;
	struct wl_client *host, *foreign;
	struct wl_resource *role, *surface, *foreign_surface;
	int host_fd, foreign_fd;
	bool host_current;
	uint64_t revision;
};

static struct wl_client *client_create(struct wl_display *display, int *fd) {
	int sv[2];
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	struct wl_client *c = wl_client_create(display, sv[0]);
	assert(c);
	*fd = sv[1];
	return c;
}

static struct wl_resource *resource_create(struct wl_client *client) {
	struct wl_resource *r =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(r);
	return r;
}

static bool observe_native_host(void *context,
		struct stage_director_rect *host, uint64_t *revision) {
	struct fixture *f = context;
	if (!f || !f->host_current) return false;
	*host = (struct stage_director_rect){0, 0, 1000, 700};
	*revision = f->revision;
	return true;
}

static void setup(struct fixture *f) {
	*f = (struct fixture){0};
	f->display = wl_display_create();
	assert(f->display);
	f->host = client_create(f->display, &f->host_fd);
	f->foreign = client_create(f->display, &f->foreign_fd);
	f->role = resource_create(f->host);
	f->surface = resource_create(f->host);
	f->foreign_surface = resource_create(f->foreign);
	f->host_current = true;
	f->revision = 1;
	stage_director_company_vector_protocol_init(f->display);
}

static void finish(struct fixture *f) {
	stage_director_company_vector_protocol_finish();
	wl_display_destroy_clients(f->display);
	close(f->host_fd);
	close(f->foreign_fd);
	wl_display_destroy(f->display);
}

static void check_no_authority(struct stage_director_company_wire_result r) {
	assert(!r.can_launch && !r.can_render && !r.can_clip &&
		!r.can_focus && !r.can_accept_input && !r.can_show_popup);
}

static void test_bounds_and_hidden(void) {
	const struct stage_director_rect host = {0, 0, 1000, 700};
	struct stage_director_company_wire_result r =
		stage_director_company_wire_check(true,true,host,
			(struct stage_director_rect){12,9,976,679},
			(struct stage_director_rect){200,110,650,450});
	assert(r.state == SD_COMPANY_WIRE_VIEWPORT_CANDIDATE);
	assert(r.geometry_candidate);
	assert(r.map_in_host.x == 212 && r.map_in_host.y == 119);
	assert(r.map_in_host.width == 650 && r.map_in_host.height == 450);
	check_no_authority(r);
	r = stage_director_company_wire_check(false,true,host,
		(struct stage_director_rect){12,9,976,679},
		(struct stage_director_rect){200,110,650,450});
	assert(r.state == SD_COMPANY_WIRE_DENIED);
	check_no_authority(r);
	r = stage_director_company_wire_check(true,false,host,
		(struct stage_director_rect){12,9,976,679},
		(struct stage_director_rect){200,110,650,450});
	assert(r.state == SD_COMPANY_WIRE_DENIED);
	check_no_authority(r);
	r = stage_director_company_wire_check(true,true,host,
		(struct stage_director_rect){0},
		(struct stage_director_rect){0});
	assert(r.state == SD_COMPANY_WIRE_HIDDEN);
	check_no_authority(r);
	r = stage_director_company_wire_check(true,true,host,
		(struct stage_director_rect){12,9,976,679},
		(struct stage_director_rect){0});
	assert(r.state == SD_COMPANY_WIRE_INVALID_GEOMETRY);
	check_no_authority(r);
	for(int i=0;i<4;i++) {
		struct stage_director_rect frame={12,9,976,679};
		struct stage_director_rect map={200,110,650,450};
		if(i==0)frame.x=-1;
		if(i==1)map.y=-1;
		if(i==2)frame.width=INT32_MAX;
		if(i==3)map.width=INT32_MAX;
		r=stage_director_company_wire_check(true,true,host,frame,map);
		assert(r.state == SD_COMPANY_WIRE_INVALID_GEOMETRY);
		check_no_authority(r);
	}
}

static void test_protected_publishing_and_lifetime(void) {
	struct fixture f;
	setup(&f);
	assert(!stage_director_company_vector_protocol_is_global(NULL));
	assert(!stage_director_company_vector_protocol_selected_client(f.host));
	assert(!stage_director_company_vector_protocol_publish(
		f.role, f.foreign_surface, observe_native_host, &f));
	assert(!stage_director_company_vector_protocol_publish(
		f.role, f.role, observe_native_host, &f));
	f.host_current=false;
	assert(!stage_director_company_vector_protocol_publish(
		f.role, f.surface, observe_native_host, &f));
	assert(!stage_director_company_vector_protocol_selected_client(f.host));
	f.host_current=true;
	assert(stage_director_company_vector_protocol_publish(
		f.role, f.surface, observe_native_host, &f));
	assert(stage_director_company_vector_protocol_selected_client(f.host));
	assert(!stage_director_company_vector_protocol_selected_client(f.foreign));
	assert(!stage_director_company_vector_protocol_publish(
		f.role, f.surface, observe_native_host, &f));
	/* Destroy exact host wl_surface. Revocation must occur before reused
	 * Wayland client and new resource can act as the old selection. */
	wl_resource_destroy(f.surface);
	assert(!stage_director_company_vector_protocol_selected_client(f.host));
	f.surface=resource_create(f.host);
	assert(stage_director_company_vector_protocol_publish(
		f.role, f.surface, observe_native_host, &f));
	wl_resource_destroy(f.role);
	assert(!stage_director_company_vector_protocol_selected_client(f.host));
	f.role=resource_create(f.host);
	assert(stage_director_company_vector_protocol_publish(
		f.role, f.surface, observe_native_host, &f));
	stage_director_company_vector_protocol_revoke();
	assert(!stage_director_company_vector_protocol_selected_client(f.host));
	stage_director_company_vector_protocol_revoke();
	finish(&f);
}

int main(void) {
	test_bounds_and_hidden();
	test_protected_publishing_and_lifetime();
	puts("STAGE_DIRECTOR_COMPANY_M23: PASS native selected client, resource revocation, no authority");
	return 0;
}
