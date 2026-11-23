#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sway/timew_scope_a_exact_geometry.h"

static unsigned checks;

static uint64_t bits_of(double d) {
	uint64_t bits;
	memcpy(&bits, &d, sizeof(bits));
	return bits;
}

static void preserved_failure(
		struct timew_scope_a_exact_geometry *out,
		enum timew_scope_a_exact_geometry_phase phase,
		double x, double y, double w, double h) {
	struct timew_scope_a_exact_geometry original = *out;
	assert(!timew_scope_a_exact_geometry_capture(out, phase, x, y, w, h));
	assert(memcmp(out, &original, sizeof(*out)) == 0);
	++checks;
}

static void test_real_fractional_fixture(void) {
	const double x = 259.0, y = 92.5, w = 1402.0, h = 872.0;
	struct timew_scope_a_exact_geometry pending = {0};
	assert(timew_scope_a_exact_geometry_capture(&pending,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, x, y, w, h));
	assert(pending.version == TIMEW_SCOPE_A_EXACT_GEOMETRY_VERSION);
	assert(pending.phase == TIMEW_SCOPE_A_GEOMETRY_PENDING);
	assert(pending.x_bits == bits_of(x));
	assert(pending.y_bits == bits_of(y));
	assert(pending.width_bits == bits_of(w));
	assert(pending.height_bits == bits_of(h));
	double rx = -1, ry = -1, rw = -1, rh = -1;
	assert(timew_scope_a_exact_geometry_read(
		&pending, &rx, &ry, &rw, &rh));
	assert(bits_of(rx) == bits_of(x));
	assert(bits_of(ry) == bits_of(y));
	assert(bits_of(rw) == bits_of(w));
	assert(bits_of(rh) == bits_of(h));
	assert(timew_scope_a_exact_geometry_equal(&pending, &pending));
	++checks;

	/* Numeric equality does NOT establish a committed/current snapshot. */
	struct timew_scope_a_exact_geometry current = {0};
	assert(timew_scope_a_exact_geometry_capture(&current,
		TIMEW_SCOPE_A_GEOMETRY_CURRENT, x, y, w, h));
	assert(!timew_scope_a_exact_geometry_equal(&pending, &current));
	++checks;

	/* A single representable binary64 step must not be lost to float. */
	struct timew_scope_a_exact_geometry changed = {0};
	const double next = nextafter(y, INFINITY);
	assert(next != y);
	assert(timew_scope_a_exact_geometry_capture(&changed,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, x, next, w, h));
	assert(!timew_scope_a_exact_geometry_equal(&pending, &changed));
	assert(changed.y_bits != pending.y_bits);
	++checks;
}

static void test_no_silent_roundtrip_loss(void) {
	for (int i = 0; i < 200; ++i) {
		const double x = -900.0 + (double)i / 8.0;
		const double y = 0.25 + (double)i / 16.0;
		const double w = 100.0 + (double)i / 4.0;
		const double h = 300.0 + (double)i / 2.0;
		struct timew_scope_a_exact_geometry v = {0};
		assert(timew_scope_a_exact_geometry_capture(&v,
			TIMEW_SCOPE_A_GEOMETRY_PENDING, x, y, w, h));
		double out_x, out_y, out_w, out_h;
		assert(timew_scope_a_exact_geometry_read(
			&v, &out_x, &out_y, &out_w, &out_h));
		assert(bits_of(out_x) == bits_of(x));
		assert(bits_of(out_y) == bits_of(y));
		assert(bits_of(out_w) == bits_of(w));
		assert(bits_of(out_h) == bits_of(h));
		++checks;
	}
}

static void test_canonical_zero_and_edge_bounds(void) {
	struct timew_scope_a_exact_geometry v = {0};
	assert(timew_scope_a_exact_geometry_capture(&v,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, -0.0, 0.0, 1.0, 1.0));
	assert(v.x_bits == 0 && v.y_bits == 0);
	double x, y, w, h;
	assert(timew_scope_a_exact_geometry_read(&v, &x, &y, &w, &h));
	assert(!signbit(x) && !signbit(y));
	++checks;

	assert(timew_scope_a_exact_geometry_capture(&v,
		TIMEW_SCOPE_A_GEOMETRY_CURRENT, (double)INT32_MIN,
		(double)INT32_MAX, (double)INT32_MAX, 1.0));
	assert(timew_scope_a_exact_geometry_read(&v, &x, &y, &w, &h));
	assert(x == (double)INT32_MIN && y == (double)INT32_MAX);
	assert(w == (double)INT32_MAX && h == 1.0);
	++checks;

	assert(timew_scope_a_exact_geometry_capture(&v,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, -0.5, 0.125, 1.5, 2.5));
	assert(timew_scope_a_exact_geometry_read(&v, &x, &y, &w, &h));
	assert(bits_of(x) == bits_of(-0.5));
	assert(bits_of(y) == bits_of(0.125));
	assert(bits_of(w) == bits_of(1.5));
	assert(bits_of(h) == bits_of(2.5));
	++checks;
}

static void test_fail_closed(void) {
	struct timew_scope_a_exact_geometry v = {0};
	assert(timew_scope_a_exact_geometry_capture(&v,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, 1, 2, 3, 4));
	assert(!timew_scope_a_exact_geometry_capture(NULL,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, 1, 2, 3, 4));
	++checks;
#define BAD(phase,x,y,w,h) preserved_failure(&v,phase,x,y,w,h)
	BAD(TIMEW_SCOPE_A_GEOMETRY_INVALID,1,2,3,4);
	BAD((enum timew_scope_a_exact_geometry_phase)3,1,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,NAN,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,NAN,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,NAN,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,NAN);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,INFINITY,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,-INFINITY,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,INFINITY,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,-INFINITY,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,INFINITY);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,(double)INT32_MIN - 1,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,(double)INT32_MAX + 1,2,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,(double)INT32_MIN - 1,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,(double)INT32_MAX + 1,3,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,0,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,-1,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,0.5,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,0);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,-1);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,0.5);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,(double)INT32_MAX + 1,4);
	BAD(TIMEW_SCOPE_A_GEOMETRY_PENDING,1,2,3,(double)INT32_MAX + 1);
#undef BAD
	/* A forged snapshot with untrusted raw bits must not be accepted. */
	double x = -11, y = -12, w = -13, h = -14;
	struct timew_scope_a_exact_geometry tampered = v;
	tampered.version++;
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	assert(x == -11 && y == -12 && w == -13 && h == -14);
	++checks;
	tampered = v;
	tampered.phase = 0;
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	++checks;
	tampered = v;
	tampered.x_bits = UINT64_C(0x7ff8000000000001); /* NaN */
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	++checks;
	tampered = v;
	tampered.y_bits = UINT64_C(0x8000000000000000); /* noncanonical -0 */
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	++checks;
	tampered = v;
	tampered.height_bits = UINT64_C(0x7ff0000000000000); /* infinity */
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	++checks;
	tampered = v;
	tampered.width_bits = bits_of(0.5);
	assert(!timew_scope_a_exact_geometry_read(&tampered, &x, &y, &w, &h));
	++checks;
	assert(!timew_scope_a_exact_geometry_equal(&v, &tampered));
	assert(!timew_scope_a_exact_geometry_equal(NULL, &v));
	++checks;
}

int main(void) {
	test_real_fractional_fixture();
	test_no_silent_roundtrip_loss();
	test_canonical_zero_and_edge_bounds();
	test_fail_closed();
	printf("TIMEW_EXACT_FRACTIONAL_GEOMETRY PASS: %u value/phase/"
		"invalid-bit groups; 92.5 preserved, no truncation, "
		"no committed/authority proof\n", checks);
	return 0;
}
