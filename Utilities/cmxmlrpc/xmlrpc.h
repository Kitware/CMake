/* Copyright (C) 2001 by First Peer, Inc. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission. 
**  
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE. */


#ifndef  _XMLRPC_H_
#define  _XMLRPC_H_ 1

#include <stddef.h>
#include <stdarg.h>
#include <cmxmlrpc/xmlrpc_config.h>

#ifdef HAVE_UNICODE_WCHAR
#include <wchar.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*=========================================================================
**  Typedefs
**=========================================================================
**  We define names for these types, because they may change from platform
**  to platform.
*/

typedef signed int xmlrpc_int;  
    /* An integer of the type defined by XML-RPC <int>; i.e. 32 bit */
typedef signed int xmlrpc_int32;
    /* An integer of the type defined by XML-RPC <int4>; i.e. 32 bit */
typedef int xmlrpc_bool;
    /* A boolean (of the type defined by XML-RPC <boolean>, but there's
       really only one kind)
    */
typedef double xmlrpc_double;
    /* A double precision floating point number as defined by
       XML-RPC <float>.  But the C "double" type is universally the same,
       so it's probably clearer just to use that.  This typedef is here 
       for mathematical completeness.
    */

#define XMLRPC_INT32_MAX (2147483647)
#define XMLRPC_INT32_MIN (-XMLRPC_INT32_MAX - 1)



/*=========================================================================
**  C struct size computations
**=======================================================================*/

/* Use XMLRPC_STRUCT_MEMBER_SIZE() to determine how big a structure is
   up to and including a specified member.  E.g. if you have
   struct mystruct {int red; int green; int blue};, then
   XMLRPC_STRUCT_MEMBER_SIZE(mystruct, green) is (8).
*/

#define _XMLRPC_STRUCT_MEMBER_OFFSET(TYPE, MBRNAME) \
  ((unsigned int)(char*)&((TYPE *)0)->MBRNAME)
#define _XMLRPC_STRUCT_MEMBER_SIZE(TYPE, MBRNAME) \
  sizeof(((TYPE *)0)->MBRNAME)
#define XMLRPC_STRUCTSIZE(TYPE, MBRNAME) \
  (_XMLRPC_STRUCT_MEMBER_OFFSET(TYPE, MBRNAME) + \
  _XMLRPC_STRUCT_MEMBER_SIZE(TYPE, MBRNAME))

/*=========================================================================
**  Assertions and Debugging
**=========================================================================
**  We use xmlrpc_assert for internal sanity checks. For example:
**
**    xmlrpc_assert(ptr != NULL);
**
**  Assertions are only evaluated when debugging code is turned on. (To
**  turn debugging off, define NDEBUG.) Some rules for using assertions:
**
**    1) Assertions should never have side effects.
**    2) Assertions should never be used for run-time error checking.
**       Instead, they should be used to check for "can't happen" errors.
*/

#ifndef NDEBUG

#define XMLRPC_ASSERT(cond) \
    do \
        if (!(cond)) \
            xmlrpc_assertion_failed(__FILE__, __LINE__); \
    while (0)

#else
#define XMLRPC_ASSERT(cond) (0)
#endif

extern void xmlrpc_assertion_failed (char* file, int line);

/* Validate a pointer. */
#define XMLRPC_ASSERT_PTR_OK(ptr) \
    XMLRPC_ASSERT((ptr) != NULL)

/* We only call this if something truly drastic happens. */
#define XMLRPC_FATAL_ERROR(msg) xmlrpc_fatal_error(__FILE__, __LINE__, (msg))

extern void xmlrpc_fatal_error (char* file, int line, char* msg);


/*=========================================================================
**  Strings
**=======================================================================*/

/* Traditional C strings are char *, because they come from a time before
   there was 'const'.  Now, const char * makes a lot more sense.  Also,
   in modern times, we tend to dynamically allocate memory for strings.
   We need this free function accordingly.  Ordinary free() doesn't check
   the type, and can generate a warning due to the 'const'.
*/
void
xmlrpc_strfree(const char * const string);



/*=========================================================================
**  xmlrpc_env
**=========================================================================
**  XML-RPC represents runtime errors as <fault> elements. These contain
**  <faultCode> and <faultString> elements.
**
**  Since we need as much thread-safety as possible, we borrow an idea from
**  CORBA--we store exception information in an "environment" object.
**  You'll pass this to many different functions, and it will get filled
**  out appropriately.
**
**  For example:
**
**    xmlrpc_env env;
**
**    xmlrpc_env_init(&env);
**
**    xmlrpc_do_something(&env);
**    if (env.fault_occurred)
**        report_error_appropriately();
**
**    xmlrpc_env_clean(&env);
*/

#define XMLRPC_INTERNAL_ERROR               (-500)
#define XMLRPC_TYPE_ERROR                   (-501)
#define XMLRPC_INDEX_ERROR                  (-502)
#define XMLRPC_PARSE_ERROR                  (-503)
#define XMLRPC_NETWORK_ERROR                (-504)
#define XMLRPC_TIMEOUT_ERROR                (-505)
#define XMLRPC_NO_SUCH_METHOD_ERROR         (-506)
#define XMLRPC_REQUEST_REFUSED_ERROR        (-507)
#define XMLRPC_INTROSPECTION_DISABLED_ERROR (-508)
#define XMLRPC_LIMIT_EXCEEDED_ERROR         (-509)
#define XMLRPC_INVALID_UTF8_ERROR           (-510)

typedef struct _xmlrpc_env {
    int   fault_occurred;
    xmlrpc_int32 fault_code;
    char* fault_string;
} xmlrpc_env;

/* Initialize and destroy the contents of the provided xmlrpc_env object.
** These functions will never fail. */
void xmlrpc_env_init (xmlrpc_env* env);
void xmlrpc_env_clean (xmlrpc_env* env);

/* Fill out an xmlrpc_fault with the specified values, and set the
** fault_occurred flag. This function will make a private copy of 'string',
** so you retain responsibility for your copy. */
void 
xmlrpc_env_set_fault(xmlrpc_env * const env, 
                     int          const faultCode, 
                     const char * const faultDescription);

/* The same as the above, but using a printf-style format string. */
void 
xmlrpc_env_set_fault_formatted (xmlrpc_env * const envP, 
                                int          const code,
                                const char * const format, 
                                ...);

/* A simple debugging assertion. */
#define XMLRPC_ASSERT_ENV_OK(env) \
    XMLRPC_ASSERT((env) != NULL && !(env)->fault_occurred)

/* This version must *not* interpret 'str' as a format string, to avoid
** several evil attacks. */
#define XMLRPC_FAIL(env,code,str) \
    do { xmlrpc_env_set_fault((env),(code),(str)); if(*(str)) goto cleanup; } while (0)

#define XMLRPC_FAIL1(env,code,str,arg1) \
    do { \
        xmlrpc_env_set_fault_formatted((env),(code),(str),(arg1)); \
        if(*(str)) goto cleanup; \
    } while (0)

#define XMLRPC_FAIL2(env,code,str,arg1,arg2) \
    do { \
        xmlrpc_env_set_fault_formatted((env),(code),(str),(arg1),(arg2)); \
        if(*(str)) goto cleanup; \
    } while (0)

#define XMLRPC_FAIL3(env,code,str,arg1,arg2,arg3) \
    do { \
        xmlrpc_env_set_fault_formatted((env),(code), \
                                       (str),(arg1),(arg2),(arg3)); \
        if(*(str)) goto cleanup; \
    } while (0)

#define XMLRPC_FAIL_IF_NULL(ptr,env,code,str) \
    do { \
        if ((ptr) == NULL) \
            XMLRPC_FAIL((env),(code),(str)); \
    } while (0)

#define XMLRPC_FAIL_IF_FAULT(env) \
    do { if ((env)->fault_occurred) goto cleanup; } while (0)


/*=========================================================================
**  Resource Limits
**=========================================================================
**  To discourage denial-of-service attacks, we provide several adjustable
**  resource limits. These functions are *not* re-entrant.
*/

/* Limit IDs. There will be more of these as time goes on. */
#define XMLRPC_NESTING_LIMIT_ID   (0)
#define XMLRPC_XML_SIZE_LIMIT_ID  (1)
#define XMLRPC_LAST_LIMIT_ID      (XMLRPC_XML_SIZE_LIMIT_ID)

/* By default, deserialized data may be no more than 64 levels deep. */
#define XMLRPC_NESTING_LIMIT_DEFAULT  (64)

/* By default, XML data from the network may be no larger than 512K.
** Some client and server modules may fail to enforce this properly. */
#define XMLRPC_XML_SIZE_LIMIT_DEFAULT (512*1024)

/* Set a specific limit to the specified value. */
extern void xmlrpc_limit_set (int limit_id, size_t value);

/* Get the value of a specified limit. */
extern size_t xmlrpc_limit_get (int limit_id);


/*=========================================================================
**  xmlrpc_mem_block
**=========================================================================
**  A resizable chunk of memory. This is mostly used internally, but it is
**  also used by the public API in a few places.
**  The struct fields are private!
*/

typedef struct _xmlrpc_mem_block {
    size_t _size;
    size_t _allocated;
    void*  _block;
} xmlrpc_mem_block;

/* Allocate a new xmlrpc_mem_block. */
xmlrpc_mem_block* xmlrpc_mem_block_new (xmlrpc_env* const env, size_t const size);

/* Destroy an existing xmlrpc_mem_block, and everything it contains. */
void xmlrpc_mem_block_free (xmlrpc_mem_block* block);

/* Initialize the contents of the provided xmlrpc_mem_block. */
void xmlrpc_mem_block_init
    (xmlrpc_env* env, xmlrpc_mem_block* block, size_t size);

/* Deallocate the contents of the provided xmlrpc_mem_block, but not the
** block itself. */
void xmlrpc_mem_block_clean (xmlrpc_mem_block* block);

/* Get the size and contents of the xmlrpc_mem_block. */
size_t 
xmlrpc_mem_block_size(const xmlrpc_mem_block * const block);

void * 
xmlrpc_mem_block_contents(const xmlrpc_mem_block * const block);

/* Resize an xmlrpc_mem_block, preserving as much of the contents as
** possible. */
void xmlrpc_mem_block_resize
    (xmlrpc_env* const env, xmlrpc_mem_block* const block, size_t const size);

/* Append data to an existing xmlrpc_mem_block. */
void xmlrpc_mem_block_append
    (xmlrpc_env* const env, xmlrpc_mem_block* const block, void *const data, size_t const len);

#define XMLRPC_MEMBLOCK_NEW(type,env,size) \
    xmlrpc_mem_block_new((env), sizeof(type) * (size))
#define XMLRPC_MEMBLOCK_FREE(type,block) \
    xmlrpc_mem_block_free(block)
#define XMLRPC_MEMBLOCK_INIT(type,env,block,size) \
    xmlrpc_mem_block_init((env), (block), sizeof(type) * (size))
#define XMLRPC_MEMBLOCK_CLEAN(type,block) \
    xmlrpc_mem_block_clean(block)
#define XMLRPC_MEMBLOCK_SIZE(type,block) \
    (xmlrpc_mem_block_size(block) / sizeof(type))
#define XMLRPC_MEMBLOCK_CONTENTS(type,block) \
    ((type*) xmlrpc_mem_block_contents(block))
#define XMLRPC_MEMBLOCK_RESIZE(type,env,block,size) \
    xmlrpc_mem_block_resize(env, block, sizeof(type) * (size))
#define XMLRPC_MEMBLOCK_APPEND(type,env,block,data,size) \
    xmlrpc_mem_block_append(env, block, data, sizeof(type) * (size))

/* Here are some backward compatibility definitions.  These longer names
   used to be the only ones and typed memory blocks were considered
   special.
*/
#define XMLRPC_TYPED_MEM_BLOCK_NEW(type,env,size) \
    XMLRPC_MEMBLOCK_NEW(type,env,size)
#define XMLRPC_TYPED_MEM_BLOCK_FREE(type,block) \
    XMLRPC_MEMBLOCK_FREE(type,block)
#define XMLRPC_TYPED_MEM_BLOCK_INIT(type,env,block,size) \
    XMLRPC_MEMBLOCK_INIT(type,env,block,size)
#define XMLRPC_TYPED_MEM_BLOCK_CLEAN(type,block) \
    XMLRPC_MEMBLOCK_CLEAN(type,block)
#define XMLRPC_TYPED_MEM_BLOCK_SIZE(type,block) \
    XMLRPC_MEMBLOCK_SIZE(type,block)
#define XMLRPC_TYPED_MEM_BLOCK_CONTENTS(type,block) \
    XMLRPC_MEMBLOCK_CONTENTS(type,block)
#define XMLRPC_TYPED_MEM_BLOCK_RESIZE(type,env,block,size) \
    XMLRPC_MEMBLOCK_RESIZE(type,env,block,size)
#define XMLRPC_TYPED_MEM_BLOCK_APPEND(type,env,block,data,size) \
    XMLRPC_MEMBLOCK_APPEND(type,env,block,data,size)



/*=========================================================================
**  xmlrpc_value
**=========================================================================
**  An XML-RPC value (of any type).
*/

typedef enum {
    XMLRPC_TYPE_INT      = 0,
    XMLRPC_TYPE_BOOL     = 1,
    XMLRPC_TYPE_DOUBLE   = 2,
    XMLRPC_TYPE_DATETIME = 3,
    XMLRPC_TYPE_STRING   = 4,
    XMLRPC_TYPE_BASE64   = 5,
    XMLRPC_TYPE_ARRAY    = 6,
    XMLRPC_TYPE_STRUCT   = 7,
    XMLRPC_TYPE_C_PTR    = 8,
    XMLRPC_TYPE_DEAD     = 0xDEAD
} xmlrpc_type;

/* These are *always* allocated on the heap. No exceptions. */
typedef struct _xmlrpc_value xmlrpc_value;

#define XMLRPC_ASSERT_VALUE_OK(val) \
    XMLRPC_ASSERT((val) != NULL && (val)->_type != XMLRPC_TYPE_DEAD)

/* A handy type-checking routine. */
#define XMLRPC_TYPE_CHECK(env,v,t) \
    do \
        if ((v)->_type != (t)) \
            XMLRPC_FAIL(env, XMLRPC_TYPE_ERROR, "Expected " #t); \
    while (0)

void
xmlrpc_abort_if_array_bad(xmlrpc_value * const arrayP);

#define XMLRPC_ASSERT_ARRAY_OK(val) \
    xmlrpc_abort_if_array_bad(val)

/* Increment the reference count of an xmlrpc_value. */
extern void xmlrpc_INCREF (xmlrpc_value* const value);

/* Decrement the reference count of an xmlrpc_value. If there
** are no more references, free it. */
extern void xmlrpc_DECREF (xmlrpc_value* const value);

/* Get the type of an XML-RPC value. */
extern xmlrpc_type xmlrpc_value_type (xmlrpc_value* value);

/* Build an xmlrpc_value from a format string.
** Increments the reference counts of input arguments if necessary.
** See the xmlrpc-c documentation for more information. */
xmlrpc_value * 
xmlrpc_build_value(xmlrpc_env * const env,
                   const char * const format, 
                   ...);

/* The same as the above, but using a va_list and more general */
void
xmlrpc_build_value_va(xmlrpc_env *    const env,
                      const char *    const format,
                      va_list               args,
                      xmlrpc_value ** const valPP,
                      const char **   const tailP);

/* Extract values from an xmlrpc_value and store them into C variables.
** Does not increment the reference counts of output values.
** See the xmlrpc-c documentation for more information. */
void 
xmlrpc_parse_value(xmlrpc_env *   const envP,
                   xmlrpc_value * const value,
                   const char *   const format, 
                   ...);

/* The same as the above, but using a va_list. */
void 
xmlrpc_parse_value_va(xmlrpc_env *   const envP,
                      xmlrpc_value * const value,
                      const char *   const format,
                      va_list              args);

void 
xmlrpc_read_int(xmlrpc_env *         const envP,
                const xmlrpc_value * const valueP,
                int *                const intValueP);

void
xmlrpc_read_double(xmlrpc_env *         const envP,
                   const xmlrpc_value * const valueP,
                   xmlrpc_double *      const doubleValueP);

void
xmlrpc_read_bool(xmlrpc_env *         const envP,
                 const xmlrpc_value * const valueP,
                 xmlrpc_bool *        const boolValueP);

void
xmlrpc_read_string(xmlrpc_env *         const envP,
                   const xmlrpc_value * const valueP,
                   const char **        const stringValueP);


void
xmlrpc_read_string_lp(xmlrpc_env *         const envP,
                      const xmlrpc_value * const valueP,
                      unsigned int *       const lengthP,
                      const char **        const stringValueP);

/* Return the number of elements in an XML-RPC array.
** Sets XMLRPC_TYPE_ERROR if 'array' is not an array. */
int 
xmlrpc_array_size(xmlrpc_env *         const env, 
                  const xmlrpc_value * const array);

/* Append an item to an XML-RPC array.
** Increments the reference count of 'value' if no fault occurs.
** Sets XMLRPC_TYPE_ERROR if 'array' is not an array. */
extern void
xmlrpc_array_append_item (xmlrpc_env*   const env,
                          xmlrpc_value* const array,
                          xmlrpc_value* const value);

void
xmlrpc_array_read_item(xmlrpc_env *         const envP,
                       const xmlrpc_value * const arrayP,
                       unsigned int         const xmIndex,
                       xmlrpc_value **      const valuePP);

/* Get an item from an XML-RPC array.
** Does not increment the reference count of the returned value.
** Sets XMLRPC_TYPE_ERROR if 'array' is not an array.
** Sets XMLRPC_INDEX_ERROR if 'index' is out of bounds. */
xmlrpc_value * 
xmlrpc_array_get_item(xmlrpc_env *         const env,
                      const xmlrpc_value * const array,
                      int                  const xmIndex);

/* Not implemented--we don't need it yet.
extern 
int xmlrpc_array_set_item (xmlrpc_env* env,
xmlrpc_value* array,
int index,
                                  xmlrpc_value* value);
*/

/* Create a new struct.  Deprecated.  xmlrpc_build_value() is the
   general way to create an xmlrpc_value, including an empty struct.
*/
xmlrpc_value *
xmlrpc_struct_new(xmlrpc_env * env);

/* Return the number of key/value pairs in a struct.
** Sets XMLRPC_TYPE_ERROR if 'strct' is not a struct. */
int
xmlrpc_struct_size (xmlrpc_env   * env, 
                    xmlrpc_value * strct);

/* Returns true iff 'strct' contains 'key'.
** Sets XMLRPC_TYPE_ERROR if 'strct' is not a struct. */
int 
xmlrpc_struct_has_key(xmlrpc_env *   const envP,
                      xmlrpc_value * const strctP,
                      const char *   const key);

/* The same as the above, but the key may contain zero bytes.
   Deprecated.  xmlrpc_struct_get_value_v() is more general, and this
   case is not common enough to warrant a shortcut.
*/
int 
xmlrpc_struct_has_key_n(xmlrpc_env   * const envP,
                        xmlrpc_value * const strctP,
                        const char *   const key, 
                        size_t         const key_len);

#if 0
/* Not implemented yet, but needed for completeness. */
int
xmlrpc_struct_has_key_v(xmlrpc_env *   env, 
                        xmlrpc_value * strct,
                        xmlrpc_value * const keyval);
#endif


void
xmlrpc_struct_find_value(xmlrpc_env *    const envP,
                         xmlrpc_value *  const structP,
                         const char *    const key,
                         xmlrpc_value ** const valuePP);


void
xmlrpc_struct_find_value_v(xmlrpc_env *    const envP,
                           xmlrpc_value *  const structP,
                           xmlrpc_value *  const keyP,
                           xmlrpc_value ** const valuePP);

void
xmlrpc_struct_read_value_v(xmlrpc_env *    const envP,
                           xmlrpc_value *  const structP,
                           xmlrpc_value *  const keyP,
                           xmlrpc_value ** const valuePP);

void
xmlrpc_struct_read_value(xmlrpc_env *    const envP,
                         xmlrpc_value *  const strctP,
                         const char *    const key,
                         xmlrpc_value ** const valuePP);

/* The "get_value" functions are deprecated.  Use the "find_value"
   and "read_value" functions instead.
*/
xmlrpc_value * 
xmlrpc_struct_get_value(xmlrpc_env *   const envP,
                        xmlrpc_value * const strctP,
                        const char *   const key);

/* The same as above, but the key may contain zero bytes. 
   Deprecated.  xmlrpc_struct_get_value_v() is more general, and this
   case is not common enough to warrant a shortcut.
*/
xmlrpc_value * 
xmlrpc_struct_get_value_n(xmlrpc_env *   const envP,
                          xmlrpc_value * const strctP,
                          const char *   const key, 
                          size_t         const key_len);

/* Set the value associated with 'key' in 'strct' to 'value'.
** Increments the reference count of value.
** Sets XMLRPC_TYPE_ERROR if 'strct' is not a struct. */
void 
xmlrpc_struct_set_value(xmlrpc_env *   const env,
                        xmlrpc_value * const strct,
                        const char *   const key,
                        xmlrpc_value * const value);

/* The same as above, but the key may contain zero bytes.  Deprecated.
   The general way to set a structure value is xmlrpc_struct_set_value_v(),
   and this case is not common enough to deserve a shortcut.
*/
void 
xmlrpc_struct_set_value_n(xmlrpc_env *    const env,
                          xmlrpc_value *  const strct,
                          const char *    const key, 
                          size_t          const key_len,
                          xmlrpc_value *  const value);

/* The same as above, but the key must be an XML-RPC string.
** Fails with XMLRPC_TYPE_ERROR if 'keyval' is not a string. */
void 
xmlrpc_struct_set_value_v(xmlrpc_env *   const env,
                          xmlrpc_value * const strct,
                          xmlrpc_value * const keyval,
                          xmlrpc_value * const value);

/* Given a zero-based index, return the matching key and value. This
** is normally used in conjunction with xmlrpc_struct_size.
** Fails with XMLRPC_TYPE_ERROR if 'struct' is not a struct.
** Fails with XMLRPC_INDEX_ERROR if 'index' is out of bounds. */

void 
xmlrpc_struct_read_member(xmlrpc_env *    const envP,
                          xmlrpc_value *  const structP,
                          unsigned int    const xmIndex,
                          xmlrpc_value ** const keyvalP,
                          xmlrpc_value ** const valueP);

/* The same as above, but does not increment the reference count of the
   two values it returns, and return NULL for both if it fails, and
   takes a signed integer for the index (but fails if it is negative).

   Deprecated.
*/
void
xmlrpc_struct_get_key_and_value(xmlrpc_env *    const env,
                                xmlrpc_value *  const strct,
                                int             const xmIndex,
                                xmlrpc_value ** const out_keyval,
                                xmlrpc_value ** const out_value);


/*=========================================================================
**  Encoding XML
**=======================================================================*/

/* Serialize an XML value without any XML header. This is primarily used
** for testing purposes. */
void
xmlrpc_serialize_value(xmlrpc_env *       env,
                       xmlrpc_mem_block * output,
                       xmlrpc_value *     value);

/* Serialize a list of parameters without any XML header. This is
** primarily used for testing purposes. */
void
xmlrpc_serialize_params(xmlrpc_env *       env,
                        xmlrpc_mem_block * output,
                        xmlrpc_value *     param_array);

/* Serialize an XML-RPC call. */
void 
xmlrpc_serialize_call (xmlrpc_env *       const env,
                       xmlrpc_mem_block * const output,
                       const char *       const method_name,
                       xmlrpc_value *     const param_array);

/* Serialize an XML-RPC return value. */
extern void
xmlrpc_serialize_response(xmlrpc_env *       env,
                          xmlrpc_mem_block * output,
                          xmlrpc_value *     value);

/* Serialize an XML-RPC fault (as specified by 'fault'). */
extern void
xmlrpc_serialize_fault(xmlrpc_env *       env,
                       xmlrpc_mem_block * output,
                       xmlrpc_env *       fault);


/*=========================================================================
**  Decoding XML
**=======================================================================*/

/* Parse an XML-RPC call. If an error occurs, set a fault and set
** the output variables to NULL.
** The caller is responsible for calling free(*out_method_name) and
** xmlrpc_DECREF(*out_param_array). */
void 
xmlrpc_parse_call(xmlrpc_env *    const envP,
                  const char *    const xml_data,
                  size_t          const xml_len,
                  const char **   const out_method_name,
                  xmlrpc_value ** const out_param_array);

/* Parse an XML-RPC response. If a fault occurs (or was received over the
** wire), return NULL and set up 'env'. The calling is responsible for
** calling xmlrpc_DECREF on the return value (if it isn't NULL). */
xmlrpc_value *
xmlrpc_parse_response(xmlrpc_env * env, 
                      const char * xml_data, 
                      size_t       xml_len);


/*=========================================================================
**  XML-RPC Base64 Utilities
**=========================================================================
**  Here are some lightweight utilities which can be used to encode and
**  decode Base64 data. These are exported mainly for testing purposes.
*/

/* This routine inserts newlines every 76 characters, as required by the
** Base64 specification. */
xmlrpc_mem_block *
xmlrpc_base64_encode(xmlrpc_env *    env,
                     unsigned char * bin_data,
                     size_t          bin_len);

/* This routine encodes everything in one line. This is needed for HTTP
** authentication and similar tasks. */
xmlrpc_mem_block *
xmlrpc_base64_encode_without_newlines(xmlrpc_env *    env,
                                      unsigned char * bin_data,
                                      size_t          bin_len);

/* This decodes Base64 data with or without newlines. */
extern xmlrpc_mem_block *
xmlrpc_base64_decode(xmlrpc_env * env,
                     char *       ascii_data,
                     size_t       ascii_len);


/*=========================================================================
**  UTF-8 Encoding and Decoding
**=========================================================================
**  We need a correct, reliable and secure UTF-8 decoder. This decoder
**  raises a fault if it encounters invalid UTF-8.
**
**  Note that ANSI C does not precisely define the representation used
**  by wchar_t--it may be UCS-2, UTF-16, UCS-4, or something from outer
**  space. If your platform does something especially bizarre, you may
**  need to reimplement these routines.
*/

#ifdef HAVE_UNICODE_WCHAR

/* Ensure that a string contains valid, legally-encoded UTF-8 data.
** (Incorrectly-encoded UTF-8 strings are often used to bypass security
** checks.) */
void 
xmlrpc_validate_utf8 (xmlrpc_env * const env,
                      const char * const utf8_data,
                      size_t       const utf8_len);

/* Decode a UTF-8 string. */
xmlrpc_mem_block *
xmlrpc_utf8_to_wcs(xmlrpc_env * env,
                   char *       utf8_data,
                   size_t       utf8_len);

/* Encode a UTF-8 string. */
xmlrpc_mem_block *
xmlrpc_wcs_to_utf8(xmlrpc_env * env,
                   wchar_t *    wcs_data,
                   size_t       wcs_len);

#endif /* HAVE_UNICODE_WCHAR */

/*=========================================================================
**  Authorization Cookie Handling
**=========================================================================
**  Routines to get and set values for authorizing via authorization
**  cookies. Both the client and server use HTTP_COOKIE_AUTH to store
**  the representation of the authorization value, which is actually
**  just a base64 hash of username:password. (This entire method is
**  a cookie replacement of basic authentication.)
**/

extern void xmlrpc_authcookie_set(xmlrpc_env * env,
                                  const char * username,
                                  const char * password);

char *xmlrpc_authcookie(void);

#ifdef __cplusplus
}
#endif

/* In the days before xmlrpc_server.h existed, some of what's in it was
   in here.  For backward compatibility, we need to include it here, even
   though it really isn't logical to do so.
*/
#include <cmxmlrpc/xmlrpc_server.h>

#endif

