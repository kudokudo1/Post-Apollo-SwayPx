#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "sway/timew_scope_a_exact_geometry.h"

#if FLT_RADIX != 2 || DBL_MANT_DIG != 53 || DBL_MAX_EXP != 1024
#error "TimeW exact geometry requires IEEE-754 binary64"
#endif
_Static_assert(sizeof(double) == sizeof(uint64_t),
	"TimeW exact geometry requires 64-bit double");

static uint64_t as_bits(double value) {
	uint64_t bits = 0;
	/* memcpy prevents aliasing UB, even under optimizations. */
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static double from_bits(uint64_t bits) {
	double value = 0;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

static bool phase_ok(uint32_t phase) {
	return phase == TIMEW_SCOPE_A_GEOMETRY_PENDING ||
		phase == TIMEW_SCOPE_A_GEOMETRY_CURRENT;
}

static bool valid_values(
		double x, double y, double width, double height) {
	return isfinite(x) && isfinite(y) &&
		isfinite(width) && isfinite(height) &&
		x >= (double)INT32_MIN && x <= (double)INT32_MAX &&
		y >= (double)INT32_MIN && y <= (double)INT32_MAX &&
		width >= 1.0 && width <= (double)INT32_MAX &&
		height >= 1.0 && height <= (double)INT32_MAX;
}

bool timew_scope_a_exact_geometry_capture(
		struct timew_scope_a_exact_geometry *out,
		enum timew_scope_a_exact_geometry_phase phase,
		double x, double y, double width, double height) {
	if (!out || !phase_ok(phase) ||
			!valid_values(x, y, width, height)) {
		return false;
	}
	/* Both -0.0 and +0.0 represent the same logical coordinate. Expose
	 * one canonical binary64 representation, never a negative-zero alias. */
	if (x == 0.0) {
		x = 0.0;
	}
	if (y == 0.0) {
		y = 0.0;
	}
	struct timew_scope_a_exact_geometry next = {
		.version = TIMEW_SCOPE_A_EXACT_GEOMETRY_VERSION,
		.phase = (uint32_t)phase,
		.x_bits = as_bits(x),
		.y_bits = as_bits(y),
		.width_bits = as_bits(width),
		.height_bits = as_bits(height),
	};
	*out = next;
	return true;
}

bool timew_scope_a_exact_geometry_read(
		const struct timew_scope_a_exact_geometry *geometry,
		double *x, double *y, double *width, double *height) {
	if (!geometry || !x || !y || !width || !height ||
			geometry->version != TIMEW_SCOPE_A_EXACT_GEOMETRY_VERSION ||
			!phase_ok(geometry->phase)) {
		return false;
	}
	const double dx = from_bits(geometry->x_bits);
	const double dy = from_bits(geometry->y_bits);
	const double dw = from_bits(geometry->width_bits);
	const double dh = from_bits(geometry->height_bits);
	if (!valid_values(dx, dy, dw, dh) ||
			(dx == 0.0 && signbit(dx)) ||
			(dy == 0.0 && signbit(dy))) {
		return false;
	}
	*x = dx;
	*y = dy;
	*width = dw;
	*height = dh;
	return true;
}

bool timew_scope_a_exact_geometry_equal(
		const struct timew_scope_a_exact_geometry *a,
		const struct timew_scope_a_exact_geometry *b) {
	double ax, ay, aw, ah, bx, by, bw, bh;
	return timew_scope_a_exact_geometry_read(a, &ax, &ay, &aw, &ah) &&
		timew_scope_a_exact_geometry_read(b, &bx, &by, &bw, &bh) &&
		a->version == b->version && a->phase == b->phase &&
		a->x_bits == b->x_bits && a->y_bits == b->y_bits &&
		a->width_bits == b->width_bits &&
		a->height_bits == b->height_bits;
}
