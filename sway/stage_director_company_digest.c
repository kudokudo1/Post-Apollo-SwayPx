#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_digest.h"
#include "sway/stage_director_company_executable.h"

/*
 * Compositor-owned, source-pinned manifest v1.
 * EMPTY ON PURPOSE: no approved hash was provided by the operator.
 * Do not fill this from the running user's installed binary, client
 * messages, runtime environment, or CI fixtures. Approval is a separate
 * operator-controlled step. The M30 match is still evidence-only.
 */
static const char company_m30_manifest_elf_sha256[] = "";

struct sha256_ctx {
	uint32_t h[8];
	uint8_t block[64];
	uint64_t bytes;
	size_t used;
};
static uint32_t rotr(uint32_t x, unsigned n) {
	return (x >> n) | (x << (32 - n));
}
static const uint32_t sha256_k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
	0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
static void sha256_compress(struct sha256_ctx *ctx) {
	uint32_t w[64];
	for (unsigned i = 0; i < 16; ++i) {
		const uint8_t *p = &ctx->block[4 * i];
		w[i] = ((uint32_t)p[0] << 24) |
			((uint32_t)p[1] << 16) |
			((uint32_t)p[2] << 8) | p[3];
	}
	for (unsigned i = 16; i < 64; ++i) {
		uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^
			(w[i - 15] >> 3);
		uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^
			(w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	uint32_t a = ctx->h[0], b = ctx->h[1], c = ctx->h[2],
		d = ctx->h[3], e = ctx->h[4], f = ctx->h[5],
		g = ctx->h[6], h = ctx->h[7];
	for (unsigned i = 0; i < 64; ++i) {
		uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
		uint32_t ch = (e & f) ^ ((~e) & g);
		uint32_t t1 = h + s1 + ch + sha256_k[i] + w[i];
		uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t t2 = s0 + maj;
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}
	ctx->h[0] += a; ctx->h[1] += b;
	ctx->h[2] += c; ctx->h[3] += d;
	ctx->h[4] += e; ctx->h[5] += f;
	ctx->h[6] += g; ctx->h[7] += h;
}
static void sha256_init(struct sha256_ctx *ctx) {
	*ctx = (struct sha256_ctx) {
		.h = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
			0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}
	};
}
static void sha256_update(struct sha256_ctx *ctx,
		const uint8_t *input, size_t count) {
	ctx->bytes += count;
	while (count) {
		size_t take = sizeof(ctx->block) - ctx->used;
		if (take > count) take = count;
		memcpy(ctx->block + ctx->used, input, take);
		ctx->used += take;
		input += take;
		count -= take;
		if (ctx->used == sizeof(ctx->block)) {
			sha256_compress(ctx);
			ctx->used = 0;
		}
	}
}
static void sha256_final(struct sha256_ctx *ctx, char hex[65]) {
	const uint64_t bits = ctx->bytes * 8;
	uint8_t padding[128] = {0x80};
	const size_t amount = ctx->used < 56 ?
		56 - ctx->used : 120 - ctx->used;
	sha256_update(ctx, padding, amount);
	uint8_t length[8];
	for (unsigned i = 0; i < 8; ++i) {
		length[7 - i] = (uint8_t)(bits >> (8 * i));
	}
	sha256_update(ctx, length, sizeof(length));
	static const char digits[] = "0123456789abcdef";
	for (unsigned i = 0; i < 8; ++i) {
		for (unsigned j = 0; j < 4; ++j) {
			uint8_t byte = (uint8_t)(ctx->h[i] >> (24 - 8 * j));
			hex[8 * i + 2 * j] = digits[byte >> 4];
			hex[8 * i + 2 * j + 1] = digits[byte & 15];
		}
	}
	hex[64] = '\0';
}

/* 512 MiB guards the compositor against pathological sparse/huge files.
 * Bounded hashing still runs synchronously; NO scheduled/automatic hashing
 * occurs while the manifest is unpinned. */
#define SD_COMPANY_M30_MAX_ELF_BYTES (512ULL * 1024 * 1024)
bool stage_director_company_sha256_fd(int fd, char hex[65]) {
	if (fd < 0 || !hex) return false;
	struct stat before = {0};
	if (fstat(fd, &before) != 0 || !S_ISREG(before.st_mode) ||
			before.st_size < 0 ||
			(uint64_t)before.st_size > SD_COMPANY_M30_MAX_ELF_BYTES) {
		return false;
	}
	struct sha256_ctx sha;
	sha256_init(&sha);
	uint8_t buf[65536];
	off_t offset = 0;
	while (offset < before.st_size) {
		size_t count = sizeof(buf);
		if ((off_t)count > before.st_size - offset) {
			count = (size_t)(before.st_size - offset);
		}
		ssize_t got = pread(fd, buf, count, offset);
		if (got <= 0) return false;
		sha256_update(&sha, buf, (size_t)got);
		offset += got;
	}
	struct stat after = {0};
	if (fstat(fd, &after) != 0 ||
			before.st_dev != after.st_dev ||
			before.st_ino != after.st_ino ||
			before.st_size != after.st_size ||
			before.st_mode != after.st_mode ||
			before.st_ctim.tv_sec != after.st_ctim.tv_sec ||
			before.st_ctim.tv_nsec != after.st_ctim.tv_nsec ||
			before.st_mtim.tv_sec != after.st_mtim.tv_sec ||
			before.st_mtim.tv_nsec != after.st_mtim.tv_nsec) {
		return false;
	}
	sha256_final(&sha, hex);
	return true;
}
static struct stage_director_company_digest_result result(
		enum stage_director_company_digest_status status) {
	return (struct stage_director_company_digest_result) {
		.status = status,
		.pinned_elf_hash_candidate =
			status == SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE,
	};
}
static bool valid_pin(const char *hex) {
	if (!hex || strlen(hex) != 64) return false;
	for (size_t i = 0; i < 64; ++i) {
		if (!((hex[i] >= '0' && hex[i] <= '9') ||
				(hex[i] >= 'a' && hex[i] <= 'f'))) {
			return false;
		}
	}
	return true;
}
struct stage_director_company_digest_result
stage_director_company_digest_check_owner_home(
		const struct stage_director_company_birth_pidfd *held,
		const char *owner_home, const char *trusted_sha256_hex) {
	if (!trusted_sha256_hex || !*trusted_sha256_hex) {
		return result(SD_COMPANY_DIGEST_UNPINNED);
	}
	if (!valid_pin(trusted_sha256_hex)) {
		return result(SD_COMPANY_DIGEST_INVALID_PIN);
	}
	struct stage_director_company_executable_result before =
		stage_director_company_executable_check_owner_home(held, owner_home);
	if (!before.running_current_install_inode_candidate) {
		return result(SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	}
	/* Kernel-backed /proc/PID/exe; never hash a user-provided path. */
	char proc[64];
	int n = held && held->pid > 0 ?
		snprintf(proc, sizeof(proc), "/proc/%" PRId64 "/exe", held->pid) : -1;
	if (n <= 0 || (size_t)n >= sizeof(proc)) {
		return result(SD_COMPANY_DIGEST_HASH_UNAVAILABLE);
	}
	const int fd = open(proc, O_RDONLY | O_CLOEXEC);
	if (fd < 0) return result(SD_COMPANY_DIGEST_HASH_UNAVAILABLE);
	char actual[65] = {0};
	bool hashed = stage_director_company_sha256_fd(fd, actual);
	close(fd);
	struct stage_director_company_executable_result after =
		stage_director_company_executable_check_owner_home(held, owner_home);
	if (!after.running_current_install_inode_candidate) {
		return result(SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	}
	if (!hashed) return result(SD_COMPANY_DIGEST_HASH_UNAVAILABLE);
	if (strcmp(actual, trusted_sha256_hex)) {
		return result(SD_COMPANY_DIGEST_MISMATCH);
	}
	/* A matching on-disk ELF does not verify loaded Qt libraries, memory
	 * pages, the legitimate Company click, or operator actor permission. */
	return result(SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE);
}
struct stage_director_company_digest_result
stage_director_company_digest_check_system_home(
		const struct stage_director_company_birth_pidfd *held) {
	if (!company_m30_manifest_elf_sha256[0]) {
		return result(SD_COMPANY_DIGEST_UNPINNED);
	}
	/* Passwd/NSS lookup remains inside M29's system-home verifier.
	 * Never substitute HOME, argv, a Wayland path, or a client string. */
	char buffer[16384];
	struct passwd *found = NULL;
	struct passwd entry = {0};
	if (!held || !held->held || !held->uid ||
			getpwuid_r((uid_t)held->uid, &entry, buffer,
				sizeof(buffer), &found) != 0 ||
			!found || !found->pw_dir || !found->pw_dir[0]) {
		return result(SD_COMPANY_DIGEST_INODE_UNVERIFIED);
	}
	return stage_director_company_digest_check_owner_home(
		held, found->pw_dir, company_m30_manifest_elf_sha256);
}
const char *stage_director_company_digest_status_name(
		enum stage_director_company_digest_status status) {
	switch (status) {
	case SD_COMPANY_DIGEST_UNPINNED: return "UNPINNED";
	case SD_COMPANY_DIGEST_INVALID_PIN: return "INVALID_PIN";
	case SD_COMPANY_DIGEST_INODE_UNVERIFIED: return "INODE_UNVERIFIED";
	case SD_COMPANY_DIGEST_HASH_UNAVAILABLE: return "HASH_UNAVAILABLE";
	case SD_COMPANY_DIGEST_MISMATCH: return "MISMATCH";
	case SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE:
		return "PIN_MATCH_CANDIDATE";
	}
	return "UNKNOWN";
}
