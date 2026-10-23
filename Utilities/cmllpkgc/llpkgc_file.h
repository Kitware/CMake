/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#ifndef INCLUDE_LLPKGC_FILE_API_H_
#define INCLUDE_LLPKGC_FILE_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#include "llpkgc_file__internal.h"

#if defined(_MSC_VER)
#define LLPKGC_FILE_EXPORT __declspec(dllexport)
#else
#define LLPKGC_FILE_EXPORT __attribute__((visibility("default")))
#endif

typedef llpkgc_file__internal_t llpkgc_file_t;
typedef struct llpkgc_file_settings_s llpkgc_file_settings_t;

typedef int (
    *llpkgc_file_data_cb)(llpkgc_file_t*, const char* at, size_t length);
typedef int (*llpkgc_file_cb)(llpkgc_file_t*);

struct llpkgc_file_settings_s {
  llpkgc_file_data_cb on_ident;
  llpkgc_file_data_cb on_value_literal;
  llpkgc_file_data_cb on_value_substitution;

  llpkgc_file_cb on_property_complete;
  llpkgc_file_cb on_variable_complete;
  llpkgc_file_cb on_barewords_complete;
  llpkgc_file_cb on_value_literal_complete;
  llpkgc_file_cb on_value_substitution_complete;
  llpkgc_file_cb on_value_complete;
  llpkgc_file_cb on_pkgc_complete;
};

enum llpkgc_file_errno {
  PFCE_OK = 0,
  PFCE_NEED_MORE = 1,
  PFCE_USER = 2,
  PFCE_UNFINISHED = 3,
  PFCE_INVALID = 4,
  PFCE_INTERNAL = 5,
};
typedef enum llpkgc_file_errno llpkgc_file_errno_t;

LLPKGC_FILE_EXPORT
void llpkgc_file_init(llpkgc_file_t* parser,
    const llpkgc_file_settings_t* settings);

LLPKGC_FILE_EXPORT
void llpkgc_file_reset(llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
void llpkgc_file_settings_init(llpkgc_file_settings_t* settings);

/* OK consumes and releases the supplied input; the next chunk may have
 * independent storage/lifetime. NEED_MORE consumes input but retains the
 * explicit range below. Callback spans must be copied or retained by callers. */
LLPKGC_FILE_EXPORT
llpkgc_file_errno_t llpkgc_file_execute(llpkgc_file_t* parser, const char* data,
    size_t len);

LLPKGC_FILE_EXPORT
llpkgc_file_errno_t llpkgc_file_finish(llpkgc_file_t* parser);

/* On NEED_MORE only: keep this range alive and extend input contiguously, or
 * call finish(). Bytes before this range may be released. No input replay. */
LLPKGC_FILE_EXPORT
const char* llpkgc_file_get_retained_start(const llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
size_t llpkgc_file_get_retained_length(const llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
llpkgc_file_errno_t llpkgc_file_get_errno(const llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
const char* llpkgc_file_get_error_reason(const llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
void llpkgc_file_set_error_reason(llpkgc_file_t* parser, const char* reason);

LLPKGC_FILE_EXPORT
const char* llpkgc_file_get_error_pos(const llpkgc_file_t* parser);

LLPKGC_FILE_EXPORT
const char* llpkgc_file_errno_name(llpkgc_file_errno_t err);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* INCLUDE_LLPKGC_FILE_API_H_ */
