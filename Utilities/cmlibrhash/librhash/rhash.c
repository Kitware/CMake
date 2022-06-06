/* rhash.c - implementation of LibRHash library calls
 *
 * Copyright (c) 2008, Aleksey Kravchenko <rhash.admin@gmail.com>
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

/* modifier for Windows DLL */
#if (defined(_WIN32) || defined(__CYGWIN__)) && defined(RHASH_EXPORTS)
# define RHASH_API __declspec(dllexport)
#endif

/* macros for large file support, must be defined before any include file */
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include "ustd.h"   /* Need this first within CMake.  */

#include "rhash.h"
#include "algorithms.h"
#include "byte_order.h"
#include "hex.h"
#include "util.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define STATE_ACTIVE  0xb01dbabe
#define STATE_STOPED  0xdeadbeef
#define STATE_DELETED 0xdecea5ed
#define RCTX_AUTO_FINAL 0x1
#define RCTX_FINALIZED  0x2
#define RCTX_FINALIZED_MASK (RCTX_AUTO_FINAL | RCTX_FINALIZED)
#define RHPR_FORMAT (RHPR_RAW | RHPR_HEX | RHPR_BASE32 | RHPR_BASE64)
#define RHPR_MODIFIER (RHPR_UPPERCASE | RHPR_URLENCODE | RHPR_REVERSE)

void rhash_library_init(void)
{
	rhash_init_algorithms(RHASH_ALL_HASHES);
#ifdef USE_OPENSSL
	rhash_plug_openssl();
#endif
}

int RHASH_API rhash_count(void)
{
	return rhash_info_size;
}

/* LOW-LEVEL LIBRHASH INTERFACE */

RHASH_API rhash rhash_init(unsigned hash_id)
{
	unsigned tail_bit_index; /* index of hash_id trailing bit */
	unsigned num = 0;        /* number of hashes to compute */
	rhash_context_ext* rctx = NULL; /* allocated rhash context */
	size_t hash_size_sum = 0;   /* size of hash contexts to store in rctx */

	unsigned i, bit_index, id;
	struct rhash_hash_info* info;
	size_t aligned_size;
	char* phash_ctx;

	hash_id &= RHASH_ALL_HASHES;
	if (hash_id == 0) {
		errno = EINVAL;
		return NULL;
	}

	tail_bit_index = rhash_ctz(hash_id); /* get trailing bit index */
	assert(tail_bit_index < RHASH_HASH_COUNT);

	id = 1 << tail_bit_index;

	if (hash_id == id) {
		/* handle the most common case of only one hash */
		num = 1;
		info = &rhash_info_table[tail_bit_index];
		hash_size_sum = info->context_size;
	} else {
		/* another case: hash_id contains several hashes */
		for (bit_index = tail_bit_index; id <= hash_id; bit_index++, id = id << 1) {
			assert(id != 0);
			assert(bit_index < RHASH_HASH_COUNT);
			info = &rhash_info_table[bit_index];
			if (hash_id & id) {
				/* align sizes by 8 bytes */
				aligned_size = (info->context_size + 7) & ~7;
				hash_size_sum += aligned_size;
				num++;
			}
		}
		assert(num > 1);
	}

	/* align the size of the rhash context common part */
	aligned_size = ((offsetof(rhash_context_ext, vector) + sizeof(rhash_vector_item) * num) + 7) & ~7;
	assert(aligned_size >= sizeof(rhash_context_ext));

	/* allocate rhash context with enough memory to store contexts of all used hashes */
	rctx = (rhash_context_ext*)malloc(aligned_size + hash_size_sum);
	if (rctx == NULL) return NULL;

	/* initialize common fields of the rhash context */
	memset(rctx, 0, sizeof(rhash_context_ext));
	rctx->rc.hash_id = hash_id;
	rctx->flags = RCTX_AUTO_FINAL; /* turn on auto-final by default */
	rctx->state = STATE_ACTIVE;
	rctx->hash_vector_size = num;

	/* aligned hash contexts follows rctx->vector[num] in the same memory block */
	phash_ctx = (char*)rctx + aligned_size;
	assert(phash_ctx >= (char*)&rctx->vector[num]);

	/* initialize context for every hash in a loop */
	for (bit_index = tail_bit_index, id = 1 << tail_bit_index, i = 0;
		id <= hash_id; bit_index++, id = id << 1)
	{
		/* check if a hash function with given id shall be included into rctx */
		if ((hash_id & id) != 0) {
			info = &rhash_info_table[bit_index];
			assert(info->context_size > 0);
			assert(((phash_ctx - (char*)0) & 7) == 0); /* hash context is aligned */
			assert(info->init != NULL);

			rctx->vector[i].hash_info = info;
			rctx->vector[i].context = phash_ctx;

#if 0
			/* BTIH initialization is complex, save pointer for later */
			if ((id & RHASH_BTIH) != 0) rctx->bt_ctx = phash_ctx;
#endif
			phash_ctx += (info->context_size + 7) & ~7;

			/* initialize the i-th hash context */
			info->init(rctx->vector[i].context);
			i++;
		}
	}

	return &rctx->rc; /* return allocated and initialized rhash context */
}

void rhash_free(rhash ctx)
{
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	unsigned i;

	if (ctx == 0) return;
	assert(ectx->hash_vector_size <= RHASH_HASH_COUNT);
	ectx->state = STATE_DELETED; /* mark memory block as being removed */

	/* clean the hash functions, which require additional clean up */
	for (i = 0; i < ectx->hash_vector_size; i++) {
		struct rhash_hash_info* info = ectx->vector[i].hash_info;
		if (info->cleanup != 0) {
			info->cleanup(ectx->vector[i].context);
		}
	}

	free(ectx);
}

RHASH_API void rhash_reset(rhash ctx)
{
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	unsigned i;

	assert(ectx->hash_vector_size > 0);
	assert(ectx->hash_vector_size <= RHASH_HASH_COUNT);
	ectx->state = STATE_ACTIVE; /* re-activate the structure */

	/* re-initialize every hash in a loop */
	for (i = 0; i < ectx->hash_vector_size; i++) {
		struct rhash_hash_info* info = ectx->vector[i].hash_info;
		if (info->cleanup != 0) {
			info->cleanup(ectx->vector[i].context);
		}

		assert(info->init != NULL);
		info->init(ectx->vector[i].context);
	}
	ectx->flags &= ~RCTX_FINALIZED; /* clear finalized state */
}

RHASH_API int rhash_update(rhash ctx, const void* message, size_t length)
{
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	unsigned i;

	assert(ectx->hash_vector_size <= RHASH_HASH_COUNT);
	if (ectx->state != STATE_ACTIVE) return 0; /* do nothing if canceled */

	ctx->msg_size += length;

	/* call update method for every algorithm */
	for (i = 0; i < ectx->hash_vector_size; i++) {
		struct rhash_hash_info* info = ectx->vector[i].hash_info;
		assert(info->update != 0);
		info->update(ectx->vector[i].context, message, length);
	}
	return 0; /* no error processing at the moment */
}

RHASH_API int rhash_final(rhash ctx, unsigned char* first_result)
{
	unsigned i = 0;
	unsigned char buffer[130];
	unsigned char* out = (first_result ? first_result : buffer);
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	assert(ectx->hash_vector_size <= RHASH_HASH_COUNT);

	/* skip final call if already finalized and auto-final is on */
	if ((ectx->flags & RCTX_FINALIZED_MASK) ==
		(RCTX_AUTO_FINAL | RCTX_FINALIZED)) return 0;

	/* call final method for every algorithm */
	for (i = 0; i < ectx->hash_vector_size; i++) {
		struct rhash_hash_info* info = ectx->vector[i].hash_info;
		assert(info->final != 0);
		assert(info->info->digest_size < sizeof(buffer));
		info->final(ectx->vector[i].context, out);
		out = buffer;
	}
	ectx->flags |= RCTX_FINALIZED;
	return 0; /* no error processing at the moment */
}

/**
 * Store digest for given hash_id.
 * If hash_id is zero, function stores digest for a hash with the lowest id found in the context.
 * For nonzero hash_id the context must contain it, otherwise function silently does nothing.
 *
 * @param ctx rhash context
 * @param hash_id id of hash to retrieve or zero for hash with the lowest available id
 * @param result buffer to put the hash into
 */
static void rhash_put_digest(rhash ctx, unsigned hash_id, unsigned char* result)
{
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	unsigned i;
	rhash_vector_item* item;
	struct rhash_hash_info* info;
	unsigned char* digest;

	assert(ectx);
	assert(ectx->hash_vector_size > 0 && ectx->hash_vector_size <= RHASH_HASH_COUNT);

	/* finalize context if not yet finalized and auto-final is on */
	if ((ectx->flags & RCTX_FINALIZED_MASK) == RCTX_AUTO_FINAL) {
		rhash_final(ctx, NULL);
	}

	if (hash_id == 0) {
		item = &ectx->vector[0]; /* get the first hash */
		info = item->hash_info;
	} else {
		for (i = 0;; i++) {
			if (i >= ectx->hash_vector_size) {
				return; /* hash_id not found, do nothing */
			}
			item = &ectx->vector[i];
			info = item->hash_info;
			if (info->info->hash_id == hash_id) break;
		}
	}
	digest = ((unsigned char*)item->context + info->digest_diff);
	if (info->info->flags & F_SWAP32) {
		assert((info->info->digest_size & 3) == 0);
		/* NB: the next call is correct only for multiple of 4 byte size */
		rhash_swap_copy_str_to_u32(result, 0, digest, info->info->digest_size);
	} else if (info->info->flags & F_SWAP64) {
		rhash_swap_copy_u64_to_str(result, digest, info->info->digest_size);
	} else {
		memcpy(result, digest, info->info->digest_size);
	}
}

RHASH_API void rhash_set_callback(rhash ctx, rhash_callback_t callback, void* callback_data)
{
	((rhash_context_ext*)ctx)->callback = (void*)callback;
	((rhash_context_ext*)ctx)->callback_data = callback_data;
}

/* HIGH-LEVEL LIBRHASH INTERFACE */

RHASH_API int rhash_msg(unsigned hash_id, const void* message, size_t length, unsigned char* result)
{
	rhash ctx;
	hash_id &= RHASH_ALL_HASHES;
	ctx = rhash_init(hash_id);
	if (ctx == NULL) return -1;
	rhash_update(ctx, message, length);
	rhash_final(ctx, result);
	rhash_free(ctx);
	return 0;
}

RHASH_API int rhash_file_update(rhash ctx, FILE* fd)
{
	rhash_context_ext* const ectx = (rhash_context_ext*)ctx;
	const size_t block_size = 8192;
	unsigned char* buffer;
	unsigned char* pmem;
	size_t length = 0, align8;
	int res = 0;
	if (ectx->state != STATE_ACTIVE) return 0; /* do nothing if canceled */

	if (ctx == NULL) {
		errno = EINVAL;
		return -1;
	}

	pmem = (unsigned char*)malloc(block_size + 8);
	if (!pmem) return -1; /* errno is set to ENOMEM according to UNIX 98 */

	align8 = ((unsigned char*)0 - pmem) & 7;
	buffer = pmem + align8;

	while (!feof(fd)) {
		/* stop if canceled */
		if (ectx->state != STATE_ACTIVE) break;

		length = fread(buffer, 1, block_size, fd);

		if (ferror(fd)) {
			res = -1; /* note: errno contains error code */
			break;
		} else if (length) {
			rhash_update(ctx, buffer, length);

			if (ectx->callback) {
				((rhash_callback_t)ectx->callback)(ectx->callback_data, ectx->rc.msg_size);
			}
		}
	}

	free(buffer);
	return res;
}

RHASH_API int rhash_file(unsigned hash_id, const char* filepath, unsigned char* result)
{
	FILE* fd;
	rhash ctx;
	int res;

	hash_id &= RHASH_ALL_HASHES;
	if (hash_id == 0) {
		errno = EINVAL;
		return -1;
	}

	if ((fd = fopen(filepath, "rb")) == NULL) return -1;

	if ((ctx = rhash_init(hash_id)) == NULL) {
		fclose(fd);
		return -1;
	}

	res = rhash_file_update(ctx, fd); /* hash the file */
	fclose(fd);

	rhash_final(ctx, result);
	rhash_free(ctx);
	return res;
}

#ifdef _WIN32 /* windows only function */
#include <share.h>

RHASH_API int rhash_wfile(unsigned hash_id, const wchar_t* filepath, unsigned char* result)
{
	FILE* fd;
	rhash ctx;
	int res;

	hash_id &= RHASH_ALL_HASHES;
	if (hash_id == 0) {
		errno = EINVAL;
		return -1;
	}

	if ((fd = _wfsopen(filepath, L"rb", _SH_DENYWR)) == NULL) return -1;

	if ((ctx = rhash_init(hash_id)) == NULL) {
		fclose(fd);
		return -1;
	}

	res = rhash_file_update(ctx, fd); /* hash the file */
	fclose(fd);

	rhash_final(ctx, result);
	rhash_free(ctx);
	return res;
}
#endif

/* RHash information functions */

#if 0
RHASH_API int rhash_is_base32(unsigned hash_id)
{
	/* fast method is just to test a bit-mask */
	return ((hash_id & (RHASH_TTH | RHASH_AICH)) != 0);
}
#endif

RHASH_API int rhash_get_digest_size(unsigned hash_id)
{
	hash_id &= RHASH_ALL_HASHES;
	if (hash_id == 0 || (hash_id & (hash_id - 1)) != 0) return -1;
	return (int)rhash_info_table[rhash_ctz(hash_id)].info->digest_size;
}

RHASH_API int rhash_get_hash_length(unsigned hash_id)
{
	const rhash_info* info = rhash_info_by_id(hash_id);
	return (int)(info ? (info->flags & F_BS32 ?
		BASE32_LENGTH(info->digest_size) : info->digest_size * 2) : 0);
}

RHASH_API const char* rhash_get_name(unsigned hash_id)
{
	const rhash_info* info = rhash_info_by_id(hash_id);
	return (info ? info->name : 0);
}

RHASH_API const char* rhash_get_magnet_name(unsigned hash_id)
{
	const rhash_info* info = rhash_info_by_id(hash_id);
	return (info ? info->magnet_name : 0);
}

#if 0
static size_t rhash_get_magnet_url_size(const char* filepath,
	rhash context, unsigned hash_mask, int flags)
{
	size_t size = 0; /* count terminating '\0' */
	unsigned bit, hash = context->hash_id & hash_mask;

	/* RHPR_NO_MAGNET, RHPR_FILESIZE */
	if ((flags & RHPR_NO_MAGNET) == 0) {
		size += 8;
	}

	if ((flags & RHPR_FILESIZE) != 0) {
		uint64_t num = context->msg_size;

		size += 4;
		if (num == 0) size++;
		else {
			for (; num; num /= 10, size++);
		}
	}

	if (filepath) {
		size += 4 + rhash_urlencode(NULL, filepath, strlen(filepath), 0);
	}

	/* loop through hash values */
	for (bit = hash & -(int)hash; bit <= hash; bit <<= 1) {
		const char* name;
		if ((bit & hash) == 0) continue;
		if ((name = rhash_get_magnet_name(bit)) == 0) continue;

		size += (7 + 2) + strlen(name);
		size += rhash_print(NULL, context, bit,
			(bit & RHASH_SHA1 ? RHPR_BASE32 : 0));
	}

	return size;
}

RHASH_API size_t rhash_print_magnet(char* output, const char* filepath,
	rhash context, unsigned hash_mask, int flags)
{
	int i;
	const char* begin = output;

	if (output == NULL)
		return rhash_get_magnet_url_size(filepath, context, hash_mask, flags);

	/* RHPR_NO_MAGNET, RHPR_FILESIZE */
	if ((flags & RHPR_NO_MAGNET) == 0) {
		strcpy(output, "magnet:?");
		output += 8;
	}

	if ((flags & RHPR_FILESIZE) != 0) {
		strcpy(output, "xl=");
		output += 3;
		output += rhash_sprintI64(output, context->msg_size);
		*(output++) = '&';
	}

	flags &= RHPR_UPPERCASE;
	if (filepath) {
		strcpy(output, "dn=");
		output += 3;
		output += rhash_urlencode(output, filepath, strlen(filepath), flags);
		*(output++) = '&';
	}

	for (i = 0; i < 2; i++) {
		unsigned bit;
		unsigned hash = context->hash_id & hash_mask;
		hash = (i == 0 ? hash & (RHASH_ED2K | RHASH_AICH)
			: hash & ~(RHASH_ED2K | RHASH_AICH));
		if (!hash) continue;

		/* loop through hash values */
		for (bit = hash & -(int)hash; bit <= hash; bit <<= 1) {
			const char* name;
			if ((bit & hash) == 0) continue;
			if (!(name = rhash_get_magnet_name(bit))) continue;

			strcpy(output, "xt=urn:");
			output += 7;
			strcpy(output, name);
			output += strlen(name);
			*(output++) = ':';
			output += rhash_print(output, context, bit,
				(bit & RHASH_SHA1 ? flags | RHPR_BASE32 : flags));
			*(output++) = '&';
		}
	}
	output[-1] = '\0'; /* terminate the line */

	return (output - begin);
}


/* HASH SUM OUTPUT INTERFACE */

size_t rhash_print_bytes(char* output, const unsigned char* bytes, size_t size, int flags)
{
	size_t result_length;
	int upper_case = (flags & RHPR_UPPERCASE);
	int format = (flags & ~RHPR_MODIFIER);

	switch (format) {
	case RHPR_HEX:
		result_length = size * 2;
		rhash_byte_to_hex(output, bytes, size, upper_case);
		break;
	case RHPR_BASE32:
		result_length = BASE32_LENGTH(size);
		rhash_byte_to_base32(output, bytes, size, upper_case);
		break;
	case RHPR_BASE64:
		result_length = rhash_base64_url_encoded_helper(output, bytes, size, (flags & RHPR_URLENCODE), upper_case);
		break;
	default:
		if (flags & RHPR_URLENCODE) {
			result_length = rhash_urlencode(output, (char*)bytes, size, upper_case);
		} else {
			memcpy(output, bytes, size);
			result_length = size;
		}
		break;
	}
	return result_length;
}

size_t RHASH_API rhash_print(char* output, rhash context, unsigned hash_id, int flags)
{
	const rhash_info* info;
	unsigned char digest[80];
	size_t digest_size;

	info = (hash_id != 0 ? rhash_info_by_id(hash_id) :
		((rhash_context_ext*)context)->vector[0].hash_info->info);

	if (info == NULL) return 0;
	digest_size = info->digest_size;
	assert(digest_size <= 64);

	flags &= (RHPR_FORMAT | RHPR_MODIFIER);
	if ((flags & RHPR_FORMAT) == 0) {
		/* use default format if not specified by flags */
		flags |= (info->flags & RHASH_INFO_BASE32 ? RHPR_BASE32 : RHPR_HEX);
	}

	if (output == NULL) {
		size_t multiplier = (flags & RHPR_URLENCODE ? 3 : 1);
		switch (flags & RHPR_FORMAT) {
		case RHPR_HEX:
			return (digest_size * 2);
		case RHPR_BASE32:
			return BASE32_LENGTH(digest_size);
		case RHPR_BASE64:
			return BASE64_LENGTH(digest_size) * multiplier;
		default:
			return digest_size * multiplier;
		}
	}

	/* note: use info->hash_id, cause hash_id can be 0 */
	rhash_put_digest(context, info->hash_id, digest);

	if ((flags & ~RHPR_UPPERCASE) == (RHPR_REVERSE | RHPR_HEX)) {
		/* reverse the digest */
		unsigned char* p = digest;
		unsigned char* r = digest + digest_size - 1;
		char tmp;
		for (; p < r; p++, r--) {
			tmp = *p;
			*p = *r;
			*r = tmp;
		}
	}

	return rhash_print_bytes(output, digest, digest_size, flags);
}

#if (defined(_WIN32) || defined(__CYGWIN__)) && defined(RHASH_EXPORTS)
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	(void)hModule;
	(void)reserved;
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		rhash_library_init();
		break;
	case DLL_PROCESS_DETACH:
		/*rhash_library_free();*/
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
#endif

#define PVOID2UPTR(p) ((rhash_uptr_t)(((char*)(p)) + 0))

RHASH_API rhash_uptr_t rhash_transmit(unsigned msg_id, void* dst, rhash_uptr_t ldata, rhash_uptr_t rdata)
{
	/* for messages working with rhash context */
	rhash_context_ext* const ctx = (rhash_context_ext*)dst;

	switch (msg_id) {
	case RMSG_GET_CONTEXT:
		{
			unsigned i;
			for (i = 0; i < ctx->hash_vector_size; i++) {
				struct rhash_hash_info* info = ctx->vector[i].hash_info;
				if (info->info->hash_id == (unsigned)ldata)
					return PVOID2UPTR(ctx->vector[i].context);
			}
			return (rhash_uptr_t)0;
		}

	case RMSG_CANCEL:
		/* mark rhash context as canceled, in a multithreaded program */
		atomic_compare_and_swap(&ctx->state, STATE_ACTIVE, STATE_STOPED);
		return 0;

	case RMSG_IS_CANCELED:
		return (ctx->state == STATE_STOPED);

	case RMSG_GET_FINALIZED:
		return ((ctx->flags & RCTX_FINALIZED) != 0);
	case RMSG_SET_AUTOFINAL:
		ctx->flags &= ~RCTX_AUTO_FINAL;
		if (ldata) ctx->flags |= RCTX_AUTO_FINAL;
		break;

	/* OpenSSL related messages */
#ifdef USE_OPENSSL
	case RMSG_SET_OPENSSL_MASK:
		rhash_openssl_hash_mask = (unsigned)ldata;
		break;
	case RMSG_GET_OPENSSL_MASK:
		return rhash_openssl_hash_mask;
#endif
	case RMSG_GET_OPENSSL_SUPPORTED_MASK:
		return rhash_get_openssl_supported_hash_mask();
	case RMSG_GET_OPENSSL_AVAILABLE_MASK:
		return rhash_get_openssl_available_hash_mask();

	default:
		return RHASH_ERROR; /* unknown message */
	}
	return 0;
}
#endif
