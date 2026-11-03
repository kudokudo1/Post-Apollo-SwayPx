#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

/* Real wl_subsurface under a real xdg_toplevel, NOT a Chrome descendant.
 * Exact independent client is a POSITIVE control for root attribution.
 * This does not authorize launching any Weather Station browser. */
struct state {
	struct wl_display *display;
	struct wl_compositor *comp;
	struct wl_subcompositor *subcomp;
	struct wl_shm *shm;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_surface *last_pointer_surface;
	struct xdg_wm_base *wm;
	struct wl_surface *parent, *child;
	struct wl_subsurface *sub;
	struct xdg_surface *xdg;
	struct xdg_toplevel *top;
	struct wl_buffer *parent_buf, *child_buf;
	bool configured, ready, closed;
};
static volatile sig_atomic_t stopping;
static void on_term(int s) {(void)s; stopping = 1;}
static void global(void *data, struct wl_registry *reg,
		uint32_t name, const char *interface, uint32_t version) {
	struct state *s = data;
	if (!strcmp(interface, wl_compositor_interface.name)) {
		s->comp = wl_registry_bind(reg, name,
			&wl_compositor_interface, version < 4 ? version : 4);
	} else if (!strcmp(interface, wl_subcompositor_interface.name)) {
		s->subcomp = wl_registry_bind(reg, name, &wl_subcompositor_interface, 1);
	} else if (!strcmp(interface, wl_shm_interface.name)) {
		s->shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	} else if (!strcmp(interface, wl_seat_interface.name)) {
		s->seat = wl_registry_bind(reg, name, &wl_seat_interface,
			version < 4 ? version : 4);
	} else if (!strcmp(interface, xdg_wm_base_interface.name)) {
		s->wm = wl_registry_bind(reg, name,
			&xdg_wm_base_interface, version < 3 ? version : 3);
	}
}
static void removed(void *data, struct wl_registry *reg, uint32_t name) {
	(void)data; (void)reg; (void)name;
}
static const struct wl_registry_listener registry_listener = {
	.global = global, .global_remove = removed,
};
static void ping(void *data, struct xdg_wm_base *wm, uint32_t serial) {
	(void)data; xdg_wm_base_pong(wm, serial);
}
static const struct xdg_wm_base_listener wm_listener = {.ping=ping};
static void top_configure(void *data, struct xdg_toplevel *top,
		int32_t width, int32_t height, struct wl_array *states) {
	(void)data; (void)top; (void)width; (void)height; (void)states;
}
static void top_close(void *data, struct xdg_toplevel *top) {
	(void)top; ((struct state *)data)->closed = true;
}
static const struct xdg_toplevel_listener top_listener = {
	.configure=top_configure, .close=top_close,
};
static struct wl_buffer *solid(struct state *s, int32_t w, int32_t h,
		uint32_t color) {
	int fd = memfd_create("sd-ci-subsurface-native-shm", MFD_CLOEXEC);
	if (fd < 0) return NULL;
	size_t bytes = (size_t)w*h*4;
	if (ftruncate(fd, (off_t)bytes) != 0) {close(fd);return NULL;}
	uint32_t *pix = mmap(NULL, bytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (pix == MAP_FAILED) {close(fd);return NULL;}
	for (size_t i=0; i<bytes/4; ++i) pix[i]=color;
	munmap(pix,bytes);
	struct wl_shm_pool *pool=wl_shm_create_pool(s->shm,fd,(int32_t)bytes);
	struct wl_buffer *out=pool?wl_shm_pool_create_buffer(pool,0,w,h,w*4,
		WL_SHM_FORMAT_ARGB8888):NULL;
	if (pool) wl_shm_pool_destroy(pool);
	close(fd);
	return out;
}
static void configured(void *data, struct xdg_surface *xdg, uint32_t serial) {
	struct state *s=data;
	xdg_surface_ack_configure(xdg,serial);
	if (s->ready) return;
	s->parent_buf=solid(s,160,120,0xff202020);
	s->child_buf=solid(s,80,60,0xff33dd77);
	if (!s->parent_buf || !s->child_buf) return;
	s->child=wl_compositor_create_surface(s->comp);
	s->sub=wl_subcompositor_get_subsurface(s->subcomp,s->child,s->parent);
	wl_subsurface_set_position(s->sub,32,28);
	wl_surface_attach(s->child,s->child_buf,0,0);
	wl_surface_damage(s->child,0,0,80,60);
	wl_surface_commit(s->child);
	wl_surface_attach(s->parent,s->parent_buf,0,0);
	wl_surface_damage(s->parent,0,0,160,120);
	wl_surface_commit(s->parent);
	s->ready=true;
	printf("SD_CI_NATIVE_SUBSURFACE_MAPPED native_parent=1 child=1 "
		"parent_same_connection=1 pid=%ld result=PASS\n",(long)getpid());
	fflush(stdout);
}
static const struct xdg_surface_listener xdg_listener = {.configure=configured};
static void pointer_enter(void *data, struct wl_pointer *pointer,
		uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {
	(void)pointer; (void)serial; (void)x; (void)y;
	struct state *s=data;
	s->last_pointer_surface=surface;
	printf("SD_CI_SUBSURFACE_RECEIPT kind=ENTER owner=%s\n",
		surface==s->child?"CHILD":surface==s->parent?"PARENT":"OTHER");
	fflush(stdout);
}
static void pointer_leave(void *data, struct wl_pointer *ptr,
		uint32_t serial, struct wl_surface *surface) {
	(void)ptr; (void)serial;
	struct state *s=data;
	if (s->last_pointer_surface == surface) s->last_pointer_surface=NULL;
	printf("SD_CI_SUBSURFACE_RECEIPT kind=LEAVE owner=%s\n",
		surface==s->child?"CHILD":surface==s->parent?"PARENT":"OTHER");
	fflush(stdout);
}
static void pointer_motion(void *data, struct wl_pointer *ptr,
		uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	(void)data; (void)ptr; (void)time; (void)x; (void)y;
}
static void pointer_button(void *data, struct wl_pointer *ptr,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	(void)ptr; (void)serial; (void)time; (void)button;
	struct state *s=data;
	printf("SD_CI_SUBSURFACE_RECEIPT kind=BUTTON state=%s owner=%s\n",
		state==WL_POINTER_BUTTON_STATE_PRESSED?"PRESS":"RELEASE",
		s->last_pointer_surface==s->child?"CHILD":
		s->last_pointer_surface==s->parent?"PARENT":"NONE");
	fflush(stdout);
}
static void pointer_axis(void *data, struct wl_pointer *ptr,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	(void)data; (void)ptr; (void)time; (void)axis; (void)value;
}
static void pointer_frame(void *data, struct wl_pointer *ptr) {
	(void)data; (void)ptr;
}
static void pointer_axis_source(void *data, struct wl_pointer *ptr,
		uint32_t source) {(void)data;(void)ptr;(void)source;}
static void pointer_axis_stop(void *data, struct wl_pointer *ptr,
		uint32_t time, uint32_t axis) {
	(void)data;(void)ptr;(void)time;(void)axis;
}
static void pointer_axis_discrete(void *data, struct wl_pointer *ptr,
		uint32_t axis, int32_t discrete) {
	(void)data;(void)ptr;(void)axis;(void)discrete;
}
static const struct wl_pointer_listener pointer_listener = {
	.enter=pointer_enter, .leave=pointer_leave, .motion=pointer_motion,
	.button=pointer_button, .axis=pointer_axis, .frame=pointer_frame,
	.axis_source=pointer_axis_source, .axis_stop=pointer_axis_stop,
	.axis_discrete=pointer_axis_discrete,
};
static void seat_capabilities(void *data, struct wl_seat *seat, uint32_t caps) {
	struct state *s=data;
	/* The disposable event sender creates and destroys a virtual pointer for
	 * each command. wl_seat can revoke POINTER capability between them.
	 * An old wl_pointer resource is not an acceptable event-receipt witness:
	 * release it on loss and bind a new pointer after capability returns. */
	if (!(caps & WL_SEAT_CAPABILITY_POINTER) && s->pointer) {
		wl_pointer_release(s->pointer);
		s->pointer=NULL;
		s->last_pointer_surface=NULL;
		puts("SD_CI_SUBSURFACE_POINTER_CAPABILITY available=0 old_resource_released=1");
		fflush(stdout);
	}
	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !s->pointer) {
		s->pointer=wl_seat_get_pointer(seat);
		if (wl_pointer_add_listener(s->pointer,&pointer_listener,s) != 0) {
			fputs("FAIL_SUBSURFACE_POINTER_LISTENER_BIND\n",stderr);
			exit(9);
		}
		puts("SD_CI_SUBSURFACE_POINTER_CAPABILITY available=1 resource_bound=1");
		fflush(stdout);
	}
}
static void seat_name(void *data, struct wl_seat *seat, const char *name) {
	(void)data;(void)seat;(void)name;
}
static const struct wl_seat_listener seat_listener = {
	.capabilities=seat_capabilities,.name=seat_name,
};
int main(int argc, char **argv) {
	const char *rt=getenv("XDG_RUNTIME_DIR");
	const char *tmp=getenv("RUNNER_TEMP");
	if (argc!=2 || strcmp(argv[1],"run") || !rt || !tmp ||
			strncmp(rt,tmp,strlen(tmp)) || rt[strlen(tmp)]!='/' ||
			!getenv("CI") || strcmp(getenv("CI"),"true") ||
			!getenv("GITHUB_ACTIONS") ||
			strcmp(getenv("GITHUB_ACTIONS"),"true") ||
			!getenv("STAGE_DIRECTOR_CI_CHROME_SUBSURFACE") ||
			strcmp(getenv("STAGE_DIRECTOR_CI_CHROME_SUBSURFACE"),"1")) {
		fputs("BLOCKED_SUBSURFACE_WITNESS_NOT_ISOLATED\n",stderr);
		return 2;
	}
	alarm(35);
	struct state s={0};
	s.display=wl_display_connect(NULL);
	if (!s.display) return 3;
	struct wl_registry *reg=wl_display_get_registry(s.display);
	wl_registry_add_listener(reg,&registry_listener,&s);
	if (wl_display_roundtrip(s.display)<0 || !s.comp || !s.subcomp ||
			!s.wm || !s.shm || !s.seat) return 4;
	wl_seat_add_listener(s.seat,&seat_listener,&s);
	xdg_wm_base_add_listener(s.wm,&wm_listener,&s);
	s.parent=wl_compositor_create_surface(s.comp);
	s.xdg=xdg_wm_base_get_xdg_surface(s.wm,s.parent);
	s.top=xdg_surface_get_toplevel(s.xdg);
	xdg_surface_add_listener(s.xdg,&xdg_listener,&s);
	xdg_toplevel_add_listener(s.top,&top_listener,&s);
	xdg_toplevel_set_app_id(s.top,"sd-ci-subsurface-witness");
	xdg_toplevel_set_title(s.top,"SD-CI-SUBSURFACE-WITNESS");
	xdg_toplevel_set_min_size(s.top,160,120);
	xdg_toplevel_set_max_size(s.top,160,120);
	wl_surface_commit(s.parent);
	struct sigaction sa={0};
	sa.sa_handler=on_term;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM,&sa,NULL);
	for (int i=0;i<240 && !stopping && !s.closed;i++) {
		struct pollfd pollfd={.fd=wl_display_get_fd(s.display),.events=POLLIN};
		int result=poll(&pollfd,1,100);
		if (result>0 && (pollfd.revents&POLLIN)) {
			if (wl_display_dispatch(s.display)<0) break;
		} else if (wl_display_dispatch_pending(s.display)<0) break;
		if (wl_display_flush(s.display)<0) break;
	}
	wl_display_disconnect(s.display);
	return 0;
}
