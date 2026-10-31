#ifndef _SWAY_STAGE_DIRECTOR_WEATHER_WATCH_PROTOCOL_H
#define _SWAY_STAGE_DIRECTOR_WEATHER_WATCH_PROTOCOL_H
#include <stdbool.h>
#include <wayland-server-core.h>

/*
 * A private Wayland global is published only after native Weather pairing.
 * Existing clients may have a registry when the global appears. The global
 * filter and bind/request handlers independently validate the same wl_client.
 * Does not itself authorize launch, capture, rendering, input, or popup.
 */
void stage_director_weather_watch_protocol_init(struct wl_display *display);
void stage_director_weather_watch_protocol_revoke(void);
bool stage_director_weather_watch_protocol_publish(void);
void stage_director_weather_watch_protocol_finish(void);
bool stage_director_weather_watch_protocol_is_global(const struct wl_global *global);

#endif
