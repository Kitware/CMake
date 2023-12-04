/* algorithms.c - the algorithms supported by the rhash library
 *
 * Copyright (c) 2011, Aleksey Kravchenko <rhash.admin@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE  INCLUDING ALL IMPLIED WARRANTIES OF  MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT,  OR CONSEQUENTIAL DAMAGES  OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT,  NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF  OR IN CONNECTION  WITH THE USE  OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "algorithms.h"
#include "byte_order.h"
#include "rhash.h"

/* header files of all supported hash functions */
#if 0
#include "aich.h"
#include "blake2b.h"
#include "blake2s.h"
#include "crc32.h"
#include "ed2k.h"
#include "edonr.h"
#include "gost12.h"
#include "gost94.h"
#include "has160.h"
#include "md4.h"
#endif
#include "md5.h"
#if 0
#include "ripemd-160.h"
#include "snefru.h"
#endif
#include "sha1.h"
#include "sha256.h"
#include "sha512.h"
#include "sha3.h"
#if 0
#include "tiger.h"
#include "tth.h"
#include "whirlpool.h"
#endif

#ifdef USE_OPENSSL
# include "plug_openssl.h"
#endif /* USE_OPENSSL */
#include <assert.h>

#ifdef USE_OPENSSL
/* note: BTIH and AICH depends on the used SHA1 algorithm */
# define NEED_OPENSSL_INIT (RHASH_MD4 | RHASH_MD5 | \
	RHASH_SHA1 | RHASH_SHA224 | RHASH_SHA256 | RHASH_SHA384 | RHASH_SHA512 | \
	RHASH_BTIH | RHASH_AICH | RHASH_RIPEMD160 | RHASH_WHIRLPOOL)
#else
# define NEED_OPENSSL_INIT 0
#endif /* USE_OPENSSL */

#ifdef GENERATE_GOST94_LOOKUP_TABLE
# define NEED_GOST94_INIT (RHASH_GOST94 | RHASH_GOST94_CRYPTOPRO)
#else
# define NEED_GOST94_INIT 0
#endif /* GENERATE_GOST94_LOOKUP_TABLE */

#define RHASH_NEED_INIT_ALG (NEED_GOST94_INIT | NEED_OPENSSL_INIT)
unsigned rhash_uninitialized_algorithms = RHASH_NEED_INIT_ALG;

rhash_hash_info* rhash_info_table = rhash_hash_info_default;
int rhash_info_size = RHASH_HASH_COUNT;

#if 0
static void rhash_crc32_init(uint32_t* crc32);
static void rhash_crc32_update(uint32_t* crc32, const unsigned char* msg, size_t size);
static void rhash_crc32_final(uint32_t* crc32, unsigned char* result);
static void rhash_crc32c_init(uint32_t* crc32);
static void rhash_crc32c_update(uint32_t* crc32, const unsigned char* msg, size_t size);
static void rhash_crc32c_final(uint32_t* crc32, unsigned char* result);
#endif

#if 0
rhash_info info_crc32  = { RHASH_CRC32,  F_BE32, 4, "CRC32", "crc32" };
rhash_info info_crc32c = { RHASH_CRC32C, F_BE32, 4, "CRC32C", "crc32c" };
rhash_info info_md4 = { RHASH_MD4, F_LE32, 16, "MD4", "md4" };
#endif
rhash_info info_md5 = { RHASH_MD5, F_LE32, 16, "MD5", "md5" };
rhash_info info_sha1 = { RHASH_SHA1,      F_BE32, 20, "SHA1", "sha1" };
#if 0
rhash_info info_tiger = { RHASH_TIGER,    F_LE64, 24, "TIGER", "tiger" };
rhash_info info_tth  = { RHASH_TTH,       F_BS32 | F_SPCEXP, 24, "TTH", "tree:tiger" };
rhash_info info_btih = { RHASH_BTIH,      F_SPCEXP, 20, "BTIH", "btih" };
rhash_info info_ed2k = { RHASH_ED2K,      F_LE32, 16, "ED2K", "ed2k" };
rhash_info info_aich = { RHASH_AICH,      F_BS32 | F_SPCEXP, 20, "AICH", "aich" };
rhash_info info_whirlpool = { RHASH_WHIRLPOOL, F_BE64, 64, "WHIRLPOOL", "whirlpool" };
rhash_info info_rmd160 = { RHASH_RIPEMD160,  F_LE32, 20, "RIPEMD-160", "ripemd160" };
rhash_info info_gost12_256 = { RHASH_GOST12_256, F_LE64, 32, "GOST12-256", "gost12-256" };
rhash_info info_gost12_512 = { RHASH_GOST12_512, F_LE64, 64, "GOST12-512", "gost12-512" };
rhash_info info_gost94 = { RHASH_GOST94,       F_LE32, 32, "GOST94", "gost94" };
rhash_info info_gost94pro = { RHASH_GOST94_CRYPTOPRO, F_LE32, 32, "GOST94-CRYPTOPRO", "gost94-cryptopro" };
rhash_info info_has160 = { RHASH_HAS160,     F_LE32, 20, "HAS-160", "has160" };
rhash_info info_snf128 = { RHASH_SNEFRU128,  F_BE32, 16, "SNEFRU-128", "snefru128" };
rhash_info info_snf256 = { RHASH_SNEFRU256,  F_BE32, 32, "SNEFRU-256", "snefru256" };
#endif
rhash_info info_sha224 = { RHASH_SHA224,     F_BE32, 28, "SHA-224", "sha224" };
rhash_info info_sha256 = { RHASH_SHA256,     F_BE32, 32, "SHA-256", "sha256" };
rhash_info info_sha384 = { RHASH_SHA384,     F_BE64, 48, "SHA-384", "sha384" };
rhash_info info_sha512 = { RHASH_SHA512,     F_BE64, 64, "SHA-512", "sha512" };
#if 0
rhash_info info_edr256 = { RHASH_EDONR256,   F_LE32, 32, "EDON-R256", "edon-r256" };
rhash_info info_edr512 = { RHASH_EDONR512,   F_LE64, 64, "EDON-R512", "edon-r512" };
rhash_info info_blake2s = { RHASH_BLAKE2S,   F_LE32, 32, "BLAKE2S", "blake2s" };
rhash_info info_blake2b = { RHASH_BLAKE2B,   F_LE64, 64, "BLAKE2B", "blake2b" };
#endif
rhash_info info_sha3_224 = { RHASH_SHA3_224, F_LE64, 28, "SHA3-224", "sha3-224" };
rhash_info info_sha3_256 = { RHASH_SHA3_256, F_LE64, 32, "SHA3-256", "sha3-256" };
rhash_info info_sha3_384 = { RHASH_SHA3_384, F_LE64, 48, "SHA3-384", "sha3-384" };
rhash_info info_sha3_512 = { RHASH_SHA3_512, F_LE64, 64, "SHA3-512", "sha3-512" };

/* some helper macros */
#define dgshft(name) ((uintptr_t)((char*)&((name##_ctx*)0)->hash))
#define dgshft2(name, field) ((uintptr_t)((char*)&((name##_ctx*)0)->field))
#define ini(name) ((pinit_t)(name##_init))
#define upd(name) ((pupdate_t)(name##_update))
#define fin(name) ((pfinal_t)(name##_final))
#define iuf(name) ini(name), upd(name), fin(name)
#define iuf2(name1, name2) ini(name1), upd(name2), fin(name2)

/* information about all supported hash functions */
rhash_hash_info rhash_hash_info_default[RHASH_HASH_COUNT] =
{
#if 0
	{ &info_crc32, sizeof(uint32_t), 0, iuf(rhash_crc32), 0 }, /* 32 bit */
	{ &info_md4, sizeof(md4_ctx), dgshft(md4), iuf(rhash_md4), 0 }, /* 128 bit */
#endif
	{ &info_md5, sizeof(md5_ctx), dgshft(md5), iuf(rhash_md5), 0 }, /* 128 bit */
	{ &info_sha1, sizeof(sha1_ctx), dgshft(sha1), iuf(rhash_sha1), 0 }, /* 160 bit */
#if 0
	{ &info_tiger, sizeof(tiger_ctx), dgshft(tiger), iuf(rhash_tiger), 0 }, /* 192 bit */
	{ &info_tth, sizeof(tth_ctx), dgshft2(tth, tiger.hash), iuf(rhash_tth), 0 }, /* 192 bit */
	{ &info_ed2k, sizeof(ed2k_ctx), dgshft2(ed2k, md4_context_inner.hash), iuf(rhash_ed2k), 0 }, /* 128 bit */
	{ &info_aich, sizeof(aich_ctx), dgshft2(aich, sha1_context.hash), iuf(rhash_aich), (pcleanup_t)rhash_aich_cleanup }, /* 160 bit */
	{ &info_whirlpool, sizeof(whirlpool_ctx), dgshft(whirlpool), iuf(rhash_whirlpool), 0 }, /* 512 bit */
	{ &info_rmd160, sizeof(ripemd160_ctx), dgshft(ripemd160), iuf(rhash_ripemd160), 0 }, /* 160 bit */
	{ &info_gost94, sizeof(gost94_ctx), dgshft(gost94), iuf(rhash_gost94), 0 }, /* 256 bit */
	{ &info_gost94pro, sizeof(gost94_ctx), dgshft(gost94), iuf2(rhash_gost94_cryptopro, rhash_gost94), 0 }, /* 256 bit */
	{ &info_has160, sizeof(has160_ctx), dgshft(has160), iuf(rhash_has160), 0 }, /* 160 bit */
	{ &info_gost12_256, sizeof(gost12_ctx), dgshft2(gost12, h) + 32, iuf2(rhash_gost12_256, rhash_gost12), 0 }, /* 256 bit */
	{ &info_gost12_512, sizeof(gost12_ctx), dgshft2(gost12, h), iuf2(rhash_gost12_512, rhash_gost12), 0 }, /* 512 bit */
#endif
	{ &info_sha224, sizeof(sha256_ctx), dgshft(sha256), iuf2(rhash_sha224, rhash_sha256), 0 }, /* 224 bit */
	{ &info_sha256, sizeof(sha256_ctx), dgshft(sha256), iuf(rhash_sha256), 0 },  /* 256 bit */
	{ &info_sha384, sizeof(sha512_ctx), dgshft(sha512), iuf2(rhash_sha384, rhash_sha512), 0 }, /* 384 bit */
	{ &info_sha512, sizeof(sha512_ctx), dgshft(sha512), iuf(rhash_sha512), 0 },  /* 512 bit */
#if 0
	{ &info_edr256, sizeof(edonr_ctx),  dgshft2(edonr, u.data256.hash) + 32, iuf(rhash_edonr256), 0 },  /* 256 bit */
	{ &info_edr512, sizeof(edonr_ctx),  dgshft2(edonr, u.data512.hash) + 64, iuf(rhash_edonr512), 0 },  /* 512 bit */
#endif
	{ &info_sha3_224, sizeof(sha3_ctx), dgshft(sha3), iuf2(rhash_sha3_224, rhash_sha3), 0 }, /* 224 bit */
	{ &info_sha3_256, sizeof(sha3_ctx), dgshft(sha3), iuf2(rhash_sha3_256, rhash_sha3), 0 }, /* 256 bit */
	{ &info_sha3_384, sizeof(sha3_ctx), dgshft(sha3), iuf2(rhash_sha3_384, rhash_sha3), 0 }, /* 384 bit */
	{ &info_sha3_512, sizeof(sha3_ctx), dgshft(sha3), iuf2(rhash_sha3_512, rhash_sha3), 0 }, /* 512 bit */
#if 0
	{ &info_crc32c, sizeof(uint32_t), 0, iuf(rhash_crc32c), 0 }, /* 32 bit */
	{ &info_snf128, sizeof(snefru_ctx), dgshft(snefru), iuf2(rhash_snefru128, rhash_snefru), 0 }, /* 128 bit */
	{ &info_snf256, sizeof(snefru_ctx), dgshft(snefru), iuf2(rhash_snefru256, rhash_snefru), 0 }, /* 256 bit */
	{ &info_blake2s, sizeof(blake2s_ctx),  dgshft(blake2s), iuf(rhash_blake2s), 0 },  /* 256 bit */
	{ &info_blake2b, sizeof(blake2b_ctx),  dgshft(blake2b), iuf(rhash_blake2b), 0 },  /* 512 bit */
#endif
};

/**
 * Initialize requested algorithms.
 *
 * @param mask ids of hash sums to initialize
 */
void rhash_init_algorithms(unsigned mask)
{
	(void)mask; /* unused now */

	/* verify that RHASH_HASH_COUNT is the index of the major bit of RHASH_ALL_HASHES */
	assert(1 == (RHASH_ALL_HASHES >> (RHASH_HASH_COUNT - 1)));

#ifdef GENERATE_GOST94_LOOKUP_TABLE
	rhash_gost94_init_table();
#endif
	rhash_uninitialized_algorithms = 0;
}

/**
 * Returns information about a hash function by its hash_id.
 *
 * @param hash_id the id of hash algorithm
 * @return pointer to the rhash_info structure containing the information
 */
const rhash_info* rhash_info_by_id(unsigned hash_id)
{
	hash_id &= RHASH_ALL_HASHES;
	/* check that one and only one bit is set */
	if (!hash_id || (hash_id & (hash_id - 1)) != 0) return NULL;
	return rhash_info_table[rhash_ctz(hash_id)].info;
}

#if 0
/* CRC32 helper functions */

/**
 * Initialize crc32 hash.
 *
 * @param crc32 pointer to the hash to initialize
 */
static void rhash_crc32_init(uint32_t* crc32)
{
	*crc32 = 0; /* note: context size is sizeof(uint32_t) */
}

/**
 * Calculate message CRC32 hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param crc32 pointer to the hash
 * @param msg message chunk
 * @param size length of the message chunk
 */
static void rhash_crc32_update(uint32_t* crc32, const unsigned char* msg, size_t size)
{
	*crc32 = rhash_get_crc32(*crc32, msg, size);
}

/**
 * Store calculated hash into the given array.
 *
 * @param crc32 pointer to the current hash value
 * @param result calculated hash in binary form
 */
static void rhash_crc32_final(uint32_t* crc32, unsigned char* result)
{
#if defined(CPU_IA32) || defined(CPU_X64)
	/* intel CPUs support assigment with non 32-bit aligned pointers */
	*(unsigned*)result = be2me_32(*crc32);
#else
	/* correct saving BigEndian integer on all archs */
	result[0] = (unsigned char)(*crc32 >> 24), result[1] = (unsigned char)(*crc32 >> 16);
	result[2] = (unsigned char)(*crc32 >> 8), result[3] = (unsigned char)(*crc32);
#endif
}

/**
 * Initialize crc32c hash.
 *
 * @param crc32c pointer to the hash to initialize
 */
static void rhash_crc32c_init(uint32_t* crc32c)
{
	*crc32c = 0; /* note: context size is sizeof(uint32_t) */
}

/**
 * Calculate message CRC32C hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param crc32c pointer to the hash
 * @param msg message chunk
 * @param size length of the message chunk
 */
static void rhash_crc32c_update(uint32_t* crc32c, const unsigned char* msg, size_t size)
{
	*crc32c = rhash_get_crc32c(*crc32c, msg, size);
}

/**
 * Store calculated hash into the given array.
 *
 * @param crc32c pointer to the current hash value
 * @param result calculated hash in binary form
 */
static void rhash_crc32c_final(uint32_t* crc32c, unsigned char* result)
{
#if defined(CPU_IA32) || defined(CPU_X64)
	/* intel CPUs support assigment with non 32-bit aligned pointers */
	*(unsigned*)result = be2me_32(*crc32c);
#else
	/* correct saving BigEndian integer on all archs */
	result[0] = (unsigned char)(*crc32c >> 24), result[1] = (unsigned char)(*crc32c >> 16);
	result[2] = (unsigned char)(*crc32c >> 8), result[3] = (unsigned char)(*crc32c);
#endif
}
#endif

#if !defined(NO_IMPORT_EXPORT)
/**
 * Export a hash function context to a memory region,
 * or calculate the size required for context export.
 *
 * @param hash_id identifier of the hash function
 * @param ctx the algorithm context containing current hashing state
 * @param out pointer to the memory region or NULL
 * @param size size of memory region
 * @return the size of the exported data on success, 0 on fail.
 */
size_t rhash_export_alg(unsigned hash_id, const void* ctx, void* out, size_t size)
{
	switch (hash_id)
	{
		case RHASH_TTH:
			return rhash_tth_export((const tth_ctx*)ctx, out, size);
		case RHASH_AICH:
			return rhash_aich_export((const aich_ctx*)ctx, out, size);
	}
	return 0;
}

/**
 * Import a hash function context from a memory region.
 *
 * @param hash_id identifier of the hash function
 * @param ctx pointer to the algorithm context
 * @param in pointer to the data to import
 * @param size size of data to import
 * @return the size of the imported data on success, 0 on fail.
 */
size_t rhash_import_alg(unsigned hash_id, void* ctx, const void* in, size_t size)
{
	switch (hash_id)
	{
		case RHASH_TTH:
			return rhash_tth_import((tth_ctx*)ctx, in, size);
		case RHASH_AICH:
			return rhash_aich_import((aich_ctx*)ctx, in, size);
	}
	return 0;
}
#endif /* !defined(NO_IMPORT_EXPORT) */

#ifdef USE_OPENSSL
void rhash_load_sha1_methods(rhash_hashing_methods* methods, int methods_type)
{
	int use_openssl;
	switch (methods_type) {
		case METHODS_OPENSSL:
			use_openssl = 1;
			break;
		case METHODS_SELECTED:
			assert(rhash_info_table[3].info->hash_id == RHASH_SHA1);
			use_openssl = ARE_OPENSSL_METHODS(rhash_info_table[3]);
			break;
		default:
			use_openssl = 0;
			break;
	}
	if (use_openssl) {
		methods->init = rhash_ossl_sha1_init();
		methods->update = rhash_ossl_sha1_update();
		methods->final = rhash_ossl_sha1_final();
	} else {
		methods->init = (pinit_t)&rhash_sha1_init;
		methods->update = (pupdate_t)&rhash_sha1_update;
		methods->final = (pfinal_t)&rhash_sha1_final;
	}
}
#endif
