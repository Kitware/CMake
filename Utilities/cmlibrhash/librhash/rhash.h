/** @file rhash.h LibRHash interface */
#ifndef RHASH_H
#define RHASH_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RHASH_API
/**
 * Modifier for LibRHash functions
 */
# define RHASH_API
#endif

/**
 * Identifiers of supported hash functions.
 * The rhash_init() function allows mixing several ids using
 * binary OR, to calculate several hash functions for one message.
 */
enum rhash_ids
{
#if 0
	RHASH_CRC32 = 0x01,
	RHASH_MD4   = 0x02,
	RHASH_MD5   = 0x04,
	RHASH_SHA1  = 0x08,
	RHASH_TIGER = 0x10,
	RHASH_TTH   = 0x20,
	RHASH_BTIH  = 0x40,
	RHASH_ED2K  = 0x80,
	RHASH_AICH  = 0x100,
	RHASH_WHIRLPOOL = 0x200,
	RHASH_RIPEMD160 = 0x400,
	RHASH_GOST94    = 0x800,
	RHASH_GOST94_CRYPTOPRO = 0x1000,
	RHASH_HAS160     = 0x2000,
	RHASH_GOST12_256 = 0x4000,
	RHASH_GOST12_512 = 0x8000,
	RHASH_SHA224    = 0x10000,
	RHASH_SHA256    = 0x20000,
	RHASH_SHA384    = 0x40000,
	RHASH_SHA512    = 0x80000,
	RHASH_EDONR256  = 0x0100000,
	RHASH_EDONR512  = 0x0200000,
	RHASH_SHA3_224  = 0x0400000,
	RHASH_SHA3_256  = 0x0800000,
	RHASH_SHA3_384  = 0x1000000,
	RHASH_SHA3_512  = 0x2000000,
	RHASH_CRC32C    = 0x4000000,
	RHASH_SNEFRU128 = 0x8000000,
	RHASH_SNEFRU256 = 0x10000000,

	/**
	 * The bit-mask containing all supported hashe functions.
	 */
	RHASH_ALL_HASHES = RHASH_CRC32 | RHASH_CRC32C | RHASH_MD4 | RHASH_MD5 |
		RHASH_ED2K | RHASH_SHA1 |RHASH_TIGER | RHASH_TTH |
		RHASH_GOST94 | RHASH_GOST94_CRYPTOPRO | RHASH_GOST12_256 | RHASH_GOST12_512 |
		RHASH_BTIH | RHASH_AICH | RHASH_WHIRLPOOL | RHASH_RIPEMD160 |
		RHASH_HAS160 | RHASH_SNEFRU128 | RHASH_SNEFRU256 |
		RHASH_SHA224 | RHASH_SHA256 | RHASH_SHA384 | RHASH_SHA512 |
		RHASH_SHA3_224 | RHASH_SHA3_256 | RHASH_SHA3_384 | RHASH_SHA3_512 |
		RHASH_EDONR256 | RHASH_EDONR512,

	RHASH_GOST = RHASH_GOST94, /* deprecated constant name */
	RHASH_GOST_CRYPTOPRO = RHASH_GOST94_CRYPTOPRO, /* deprecated constant name */
	/**
	 * The number of supported hash functions.
	 */
	RHASH_HASH_COUNT = 29
#else
	RHASH_MD5        = 0x01,
	RHASH_SHA1       = 0x02,
	RHASH_SHA224     = 0x04,
	RHASH_SHA256     = 0x08,
	RHASH_SHA384     = 0x10,
	RHASH_SHA512     = 0x20,
	RHASH_SHA3_224   = 0x40,
	RHASH_SHA3_256   = 0x80,
	RHASH_SHA3_384   = 0x100,
	RHASH_SHA3_512   = 0x200,
	RHASH_ALL_HASHES =
		RHASH_MD5 |
		RHASH_SHA1 |
		RHASH_SHA224 |
		RHASH_SHA256 |
		RHASH_SHA384 |
		RHASH_SHA512 |
		RHASH_SHA3_224 |
		RHASH_SHA3_256 |
		RHASH_SHA3_384 |
		RHASH_SHA3_512,
	RHASH_HASH_COUNT = 10
#endif
};

/**
 * The rhash context structure contains contexts for several hash functions.
 */
typedef struct rhash_context
{
	/**
	 * The size of the hashed message.
	 */
	unsigned long long msg_size;

	/**
	 * The bit-mask containing identifiers of the hashes being calculated.
	 */
	unsigned hash_id;
} rhash_context;

#ifndef LIBRHASH_RHASH_CTX_DEFINED
#define LIBRHASH_RHASH_CTX_DEFINED
/**
 * Hashing context.
 */
typedef struct rhash_context* rhash;
#endif /* LIBRHASH_RHASH_CTX_DEFINED */

/**
 * Type of a callback to be called periodically while hashing a file.
 */
typedef void (*rhash_callback_t)(void* data, unsigned long long offset);

/**
 * Initialize static data of rhash algorithms
 */
RHASH_API void rhash_library_init(void);


/* HIGH-LEVEL LIBRHASH INTERFACE */

/**
 * Compute a hash of the given message.
 *
 * @param hash_id id of hash sum to compute
 * @param message the message to process
 * @param length message length
 * @param result buffer to receive binary hash string
 * @return 0 on success, -1 on error
 */
RHASH_API int rhash_msg(unsigned hash_id, const void* message, size_t length, unsigned char* result);

/**
 * Compute a single hash for given file.
 *
 * @param hash_id id of hash sum to compute
 * @param filepath path to the file to hash
 * @param result buffer to receive hash value with the lowest requested id
 * @return 0 on success, -1 on error and errno is set
 */
RHASH_API int rhash_file(unsigned hash_id, const char* filepath, unsigned char* result);

#ifdef _WIN32
/**
 * Compute a single hash for given file (Windows-specific function).
 *
 * @param hash_id id of hash sum to compute
 * @param filepath path to the file to hash
 * @param result buffer to receive hash value with the lowest requested id
 * @return 0 on success, -1 on error, -1 on error and errno is set
 */
RHASH_API int rhash_wfile(unsigned hash_id, const wchar_t* filepath, unsigned char* result);
#endif


/* LOW-LEVEL LIBRHASH INTERFACE */

/**
 * Allocate and initialize RHash context for calculating hash(es).
 * After initializing rhash_update()/rhash_final() functions should be used.
 * Then the context must be freed by calling rhash_free().
 *
 * @param hash_id union of bit flags, containing ids of hashes to calculate.
 * @return initialized rhash context, NULL on error and errno is set
 */
RHASH_API rhash rhash_init(unsigned hash_id);

/**
 * Calculate hashes of message.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the rhash context
 * @param message message chunk
 * @param length length of the message chunk
 * @return 0 on success; On fail return -1 and set errno
 */
RHASH_API int rhash_update(rhash ctx, const void* message, size_t length);

/**
 * Hash a file or stream. Multiple hashes can be computed.
 * First, inintialize ctx parameter with rhash_init() before calling
 * rhash_file_update(). Then use rhash_final() and rhash_print()
 * to retrive hash values. Finaly call rhash_free() on ctx
 * to free allocated memory or call rhash_reset() to reuse ctx.
 *
 * @param ctx rhash context
 * @param fd descriptor of the file to hash
 * @return 0 on success, -1 on error and errno is set
 */
RHASH_API int rhash_file_update(rhash ctx, FILE* fd);

/**
 * Finalize hash calculation and optionally store the first hash.
 *
 * @param ctx the rhash context
 * @param first_result optional buffer to store a calculated hash with the lowest available id
 * @return 0 on success; On fail return -1 and set errno
 */
RHASH_API int rhash_final(rhash ctx, unsigned char* first_result);

/**
 * Re-initialize RHash context to reuse it.
 * Useful to speed up processing of many small messages.
 *
 * @param ctx context to reinitialize
 */
RHASH_API void rhash_reset(rhash ctx);

/**
 * Free RHash context memory.
 *
 * @param ctx the context to free.
 */
RHASH_API void rhash_free(rhash ctx);

/**
 * Set the callback function to be called from the
 * rhash_file() and rhash_file_update() functions
 * on processing every file block. The file block
 * size is set internally by rhash and now is 8 KiB.
 *
 * @param ctx rhash context
 * @param callback pointer to the callback function
 * @param callback_data pointer to data passed to the callback
 */
RHASH_API void  rhash_set_callback(rhash ctx, rhash_callback_t callback, void* callback_data);


/* INFORMATION FUNCTIONS */

/**
 * Returns the number of supported hash algorithms.
 *
 * @return the number of supported hash functions
 */
RHASH_API int  rhash_count(void); /* number of supported hashes */

/**
 * Returns size of binary digest for given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return digest size in bytes
 */
RHASH_API int  rhash_get_digest_size(unsigned hash_id); /* size of binary message digest */

/**
 * Returns length of digest hash string in default output format.
 *
 * @param hash_id the id of hash algorithm
 * @return the length of hash string
 */
RHASH_API int  rhash_get_hash_length(unsigned hash_id); /* length of formatted hash string */

/**
 * Detect default digest output format for given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return 1 for base32 format, 0 for hexadecimal
 */
RHASH_API int  rhash_is_base32(unsigned hash_id); /* default digest output format */

/**
 * Returns a name of given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return algorithm name
 */
RHASH_API const char* rhash_get_name(unsigned hash_id); /* get hash function name */

/**
 * Returns a name part of magnet urn of the given hash algorithm.
 * Such magnet_name is used to generate a magnet link of the form
 * urn:&lt;magnet_name&gt;=&lt;hash_value&gt;.
 *
 * @param hash_id the id of hash algorithm
 * @return name
 */
RHASH_API const char* rhash_get_magnet_name(unsigned hash_id); /* get name part of magnet urn */

/* HASH SUM OUTPUT INTERFACE */

#if 0
/**
 * Flags for printing a hash sum.
 */
enum rhash_print_sum_flags
{
	/*
	 * Print in a default format
	 */
	RHPR_DEFAULT   = 0x0,
	/*
	 * Output as binary message digest
	 */
	RHPR_RAW       = 0x1,
	/*
	 * Print as a hexadecimal string
	 */
	RHPR_HEX       = 0x2,
	/*
	 * Print as a base32-encoded string
	 */
	RHPR_BASE32    = 0x3,
	/*
	 * Print as a base64-encoded string
	 */
	RHPR_BASE64    = 0x4,
	/*
	 * Print as an uppercase string. Can be used
	 * for base32 or hexadecimal format only.
	 */
	RHPR_UPPERCASE = 0x8,
	/*
	 * Reverse hash bytes. Can be used for GOST hash.
	 */
	RHPR_REVERSE   = 0x10,
	/*
	 * Don't print 'magnet:?' prefix in rhash_print_magnet
	 */
	RHPR_NO_MAGNET  = 0x20,
	/*
	 * Print file size in rhash_print_magnet
	 */
	RHPR_FILESIZE  = 0x40,
	/*
	 * Print as URL-encoded string
	 */
	RHPR_URLENCODE  = 0x80
};
#endif


/**
 * Print a text presentation of a given hash sum to the specified buffer.
 *
 * @param output a buffer to print the hash to
 * @param bytes a hash sum to print
 * @param size a size of hash sum in bytes
 * @param flags  a bit-mask controlling how to format the hash sum,
 *               can be a mix of the flags: RHPR_RAW, RHPR_HEX, RHPR_BASE32,
 *               RHPR_BASE64, RHPR_URLENCODE, RHPR_UPPERCASE, RHPR_REVERSE
 * @return the number of written characters
 */
RHASH_API size_t rhash_print_bytes(char* output,
	const unsigned char* bytes, size_t size, int flags);

/**
 * Print text presentation of a hash sum with given hash_id to the specified
 * output buffer. If the hash_id is zero, then print the hash sum with
 * the lowest id stored in the hash context.
 * The function call fails if the context doesn't include a hash with the
 * given hash_id.
 *
 * @param output a buffer to print the hash to
 * @param ctx    algorithms state
 * @param hash_id id of the hash sum to print or 0 to print the first hash
 *                saved in the context.
 * @param flags  a bitmask controlling how to print the hash. Can contain flags
 *               RHPR_UPPERCASE, RHPR_HEX, RHPR_BASE32, RHPR_BASE64, etc.
 * @return the number of written characters on success or 0 on fail
 */
RHASH_API size_t rhash_print(char* output, rhash ctx, unsigned hash_id,
	int flags);

/**
 * Print magnet link with given filepath and calculated hash sums into the
 * output buffer. The hash_mask can limit which hash values will be printed.
 * The function returns the size of the required buffer.
 * If output is NULL the .
 *
 * @param output a string buffer to receive the magnet link or NULL
 * @param filepath the file path to be printed or NULL
 * @param context algorithms state
 * @param hash_mask bit mask of the hash sums to add to the link
 * @param flags   can be combination of bits RHPR_UPPERCASE, RHPR_NO_MAGNET,
 *                RHPR_FILESIZE
 * @return number of written characters, including terminating '\0' on success, 0 on fail
 */
RHASH_API size_t rhash_print_magnet(char* output, const char* filepath,
	rhash context, unsigned hash_mask, int flags);


/* MESSAGE API */

/**
 * The type of an unsigned integer large enough to hold a pointer.
 */
#if defined(UINTPTR_MAX)
typedef uintptr_t rhash_uptr_t;
#elif defined(_LP64) || defined(__LP64__) || defined(__x86_64) || \
	defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
typedef unsigned long long rhash_uptr_t;
#else
typedef unsigned long rhash_uptr_t;
#endif

/**
 * The value returned by rhash_transmit on error.
 */
#define RHASH_ERROR ((rhash_uptr_t)-1)
/**
 * Convert a pointer to rhash_uptr_t.
 */
#define RHASH_STR2UPTR(str) ((rhash_uptr_t)(char*)(str))
/**
 * Convert a rhash_uptr_t to a void* pointer.
 */
#define RHASH_UPTR2PVOID(u) ((void*)((u) + 0))

/**
 * Process a rhash message.
 *
 * @param msg_id message identifier
 * @param dst message destination (can be NULL for generic messages)
 * @param ldata data depending on message
 * @param rdata data depending on message
 * @return message-specific data
 */
RHASH_API rhash_uptr_t rhash_transmit(
	unsigned msg_id, void* dst, rhash_uptr_t ldata, rhash_uptr_t rdata);

/* rhash message constants */

#define RMSG_GET_CONTEXT 1
#define RMSG_CANCEL      2
#define RMSG_IS_CANCELED 3
#define RMSG_GET_FINALIZED 4
#define RMSG_SET_AUTOFINAL 5
#define RMSG_SET_OPENSSL_MASK 10
#define RMSG_GET_OPENSSL_MASK 11
#define RMSG_GET_OPENSSL_SUPPORTED_MASK 12
#define RMSG_GET_OPENSSL_AVAILABLE_MASK 13

/* HELPER MACROS */

/**
 * Get a pointer to context of the specified hash function.
 */
#define rhash_get_context_ptr(ctx, hash_id) RHASH_UPTR2PVOID(rhash_transmit(RMSG_GET_CONTEXT, ctx, hash_id, 0))
/**
 * Cancel hash calculation of a file.
 */
#define rhash_cancel(ctx) rhash_transmit(RMSG_CANCEL, ctx, 0, 0)
/**
 * Return non-zero if hash calculation was canceled, zero otherwise.
 */
#define rhash_is_canceled(ctx) rhash_transmit(RMSG_IS_CANCELED, ctx, 0, 0)
/**
 * Return non-zero if rhash_final was called for rhash_context.
 */
#define rhash_get_finalized(ctx) rhash_transmit(RMSG_GET_FINALIZED, ctx, 0, 0)

/**
 * Turn on/off the auto-final flag for the given rhash_context. By default
 * auto-final is on, which means rhash_final is called automatically, if
 * needed when a hash value is retrieved by rhash_print call.
 */
#define rhash_set_autofinal(ctx, on) rhash_transmit(RMSG_SET_AUTOFINAL, ctx, on, 0)

/**
 * Set the bit-mask of hash algorithms to be calculated by OpenSSL library.
 * The call rhash_set_openssl_mask(0) made before rhash_library_init(),
 * turns off loading of the OpenSSL dynamic library.
 * This call works if the LibRHash was compiled with OpenSSL support.
 */
#define rhash_set_openssl_mask(mask) rhash_transmit(RMSG_SET_OPENSSL_MASK, NULL, mask, 0)

/**
 * Return current bit-mask of hash algorithms selected to be calculated by OpenSSL
 * library. Return RHASH_ERROR if LibRHash is compiled without OpenSSL support.
 */
#define rhash_get_openssl_mask() rhash_transmit(RMSG_GET_OPENSSL_MASK, NULL, 0, 0)

/**
 * Return the bit-mask of algorithms that can be provided by the OpenSSL plugin,
 * if the library is compiled with OpenSSL support, 0 otherwise. This bit-mask is
 * a constant value computed at compile-time.
 */
#define rhash_get_openssl_supported_mask() rhash_transmit(RMSG_GET_OPENSSL_SUPPORTED_MASK, NULL, 0, 0)

/**
 * Return the bit-mask of algorithms that are successfully loaded from
 * OpenSSL library. If the library is not loaded or not supported by LibRHash,
 * then return 0.
 */
#define rhash_get_openssl_available_mask() rhash_transmit(RMSG_GET_OPENSSL_AVAILABLE_MASK, NULL, 0, 0)


/**
 * Return non-zero if LibRHash hash been compiled with OpenSSL support,
 * and zero otherwise.
 */
#define rhash_is_openssl_supported() (rhash_get_openssl_mask() != RHASH_ERROR)

/**
 * Legacy macro. The bit mask of hash algorithms implemented by OpenSSL.
 */
# define RHASH_OPENSSL_SUPPORTED_HASHES (rhash_get_openssl_supported_mask())

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* RHASH_H */
