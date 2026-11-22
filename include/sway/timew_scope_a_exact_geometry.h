#ifndef _SWAY_TIMEW_SCOPE_A_EXACT_GEOMETRY_H
#define _SWAY_TIMEW_SCOPE_A_EXACT_GEOMETRY_H

#include <stdbool.h>
#include <stdint.h>

/*
 * TimeW Scope A // CI-only exact logical-coordinate value contract.
 *
 * Not linked into the ordinary SwayPX compositor. These are DATA VALUES,
 * not trusted surface identities, geometry observations, consent, protected
 * transport, mutation permits, or signed receipts. A caller-supplied phase
 * is NOT independent evidence of transaction completion.
 *
 * Each coordinate is IEEE-754 binary64, preserved as a 64-bit bit pattern.
 * This avoids silently rounding/truncating SwayPX floating pending geometry
 * (observed in a real headless XDG test: y=92.5).
 *
 * Values are compositor LOGICAL layout coordinates, not physical output
 * pixels. Frame scale/transform and the target/seat/object identity must be
 * separately bound by the eventual approved native contract.
 */
enum timew_scope_a_exact_geometry_phase {
	TIMEW_SCOPE_A_GEOMETRY_INVALID = 0,
	TIMEW_SCOPE_A_GEOMETRY_PENDING = 1,
	TIMEW_SCOPE_A_GEOMETRY_CURRENT = 2,
};

enum { TIMEW_SCOPE_A_EXACT_GEOMETRY_VERSION = 1 };

struct timew_scope_a_exact_geometry {
	uint32_t version;
	uint32_t phase;
	uint64_t x_bits;
	uint64_t y_bits;
	uint64_t width_bits;
	uint64_t height_bits;
};

/*
 * Fail-closed value capture. Rejects NaN, +/-Infinity, coordinates outside
 * signed 32-bit layout bounds and nonpositive/oversized width/height.
 * Accepts fractional coordinates/sizes >= 1 layout unit for dimensions.
 * Normalizes negative zero to +0 for a unique representation.
 *
 * On failure, *out remains byte-for-byte unchanged. No authority flags.
 */
bool timew_scope_a_exact_geometry_capture(
	struct timew_scope_a_exact_geometry *out,
	enum timew_scope_a_exact_geometry_phase phase,
	double x, double y, double width, double height);

/*
 * Validate raw bits and return the EXACT original finite binary64 values.
 * Rejects invalid phase/version/bits and noncanonical -0.
 * On failure, none of the four output doubles is modified.
 */
bool timew_scope_a_exact_geometry_read(
	const struct timew_scope_a_exact_geometry *geometry,
	double *x, double *y, double *width, double *height);

/* Exact phase-and-bit equality only, never a surface identity/authority. */
bool timew_scope_a_exact_geometry_equal(
	const struct timew_scope_a_exact_geometry *a,
	const struct timew_scope_a_exact_geometry *b);

#endif
