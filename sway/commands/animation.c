#include <strings.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/animation_manager.h"

static bool parse_animation_style(const char *name, enum animation_style *style) {
	if (strcasecmp(name, "default") == 0) {
		*style = ANIMATION_STYLE_DEFAULT;
		return true;
	}

	if (strcasecmp(name, "crt") == 0) {
		*style = ANIMATION_STYLE_CRT;
		return true;
	}

	if (strcasecmp(name, "inherit") == 0) {
		*style = ANIMATION_STYLE_INHERIT;
		return true;
	}
	
	return false;
}

static bool parse_animation_event(const char *name, enum animation_event *event) {
	if (strcasecmp(name, "open") == 0) {
		*event = ANIMATION_EVENT_OPEN;
		return true;
	}

	if (strcasecmp(name, "close") == 0) {
		*event = ANIMATION_EVENT_CLOSE;
		return true;
	}

	if (strcasecmp(name, "move") == 0) {
		*event = ANIMATION_EVENT_MOVE;
		return true;
	}

	if (strcasecmp(name, "resize") == 0) {
		*event = ANIMATION_EVENT_RESIZE;
		return true;
	}

	if (strcasecmp(name, "workspace") == 0) {
		*event = ANIMATION_EVENT_WORKSPACE;
		return true;
	}

	return false;
}

struct cmd_results *cmd_animation(int argc, char **argv) {
	if (argc < 1 || argc > 2) {
		return cmd_results_new(CMD_INVALID,
			"Invalid animation command (expected 1 or 2 arguments, got %d)",
			argc);
	}

	enum animation_style style;

	// Global form: animation CRT
	if (argc == 1) {
		if (!parse_animation_style(argv[0], &style) ||
				style == ANIMATION_STYLE_INHERIT) {
			return cmd_results_new(CMD_INVALID,
				"Unknown global animation style '%s'", argv[0]);
		}

		config->animation_style = style;
		return cmd_results_new(CMD_SUCCESS, NULL);
	}

	// Event-specific form: animation open CRT
	enum animation_event event;

	if (!parse_animation_event(argv[0], &event)) {
		return cmd_results_new(CMD_INVALID,
			"Unknown animation event '%s'", argv[0]);
	}

	if (!parse_animation_style(argv[1], &style)) {
		return cmd_results_new(CMD_INVALID,
			"Unknown animation style '%s'", argv[1]);
	}

	config->animation_styles[event] = style;

	return cmd_results_new(CMD_SUCCESS, NULL);
}
