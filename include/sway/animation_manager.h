#ifndef _SWAY_ANIMATION_MANAGER_H
#define _SWAY_ANIMATION_MANAGER_H

#include <stdbool.h>
#include <wayland-util.h>

struct sway_container;
struct sway_server;

enum animation_style {
        ANIMATION_STYLE_DEFAULT,
        ANIMATION_STYLE_CRT,
};

enum animation_event {
	    ANIMATION_EVENT_NONE,
        ANIMATION_EVENT_OPEN,
        ANIMATION_EVENT_CLOSE,
        ANIMATION_EVENT_MOVE,
        ANIMATION_EVENT_RESIZE,
        ANIMATION_EVENT_WORKSPACE,
};

// TODO: make animation just a pointer to progress, make multiplier and callback private
struct animation {
	struct wl_list link;
	float progress;
	enum animation_event event;
	void *data;
	float multiplier;
	bool initialized;
	void (*update)(void *);
	void (*complete)(void *);
	
};

void animation_manager_init(struct sway_server *server);

struct animation init_animation(void *data);

void refresh_animation_manager_timing();

void add_animation(struct animation *animation, void (*update_callback)(void *),
	void (*complete_callback)(void *));

void finish_animation(struct animation *animation);

void start_animations();

float get_animated_value(float from, float to, const struct animation *animation);

#endif

