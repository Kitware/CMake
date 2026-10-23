/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#ifndef INCLUDE_LLPKGC_FRAG_API_H_
#define INCLUDE_LLPKGC_FRAG_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#include "llpkgc_frag__internal.h"

#if defined(_MSC_VER)
#define LLPKGC_FRAG_EXPORT __declspec(dllexport)
#else
#define LLPKGC_FRAG_EXPORT __attribute__((visibility("default")))
#endif

typedef llpkgc_frag__internal_t llpkgc_frag_t;
typedef struct llpkgc_frag_settings_s llpkgc_frag_settings_t;

typedef int (
    *llpkgc_frag_data_cb)(llpkgc_frag_t*, const char* at, size_t length);
typedef int (*llpkgc_frag_cb)(llpkgc_frag_t*);

struct llpkgc_frag_settings_s {
  llpkgc_frag_data_cb on_fragment_text;
  llpkgc_frag_cb on_frag_complete;
  llpkgc_frag_cb on_fraglist_complete;
};

enum llpkgc_frag_errno {
  PFFE_OK = 0,
  PFFE_USER = 2,
  PFFE_UNFINISHED = 3,
  PFFE_INVALID = 4,
  PFFE_INTERNAL = 5,
};
typedef enum llpkgc_frag_errno llpkgc_frag_errno_t;

LLPKGC_FRAG_EXPORT
void llpkgc_frag_init(llpkgc_frag_t* parser,
    const llpkgc_frag_settings_t* settings);

LLPKGC_FRAG_EXPORT
void llpkgc_frag_reset(llpkgc_frag_t* parser);

LLPKGC_FRAG_EXPORT
void llpkgc_frag_settings_init(llpkgc_frag_settings_t* settings);

/* Successful chunks are fully consumed and may be released immediately.
 * Subsequent chunks need not be contiguous. Callback spans are borrowed. */
LLPKGC_FRAG_EXPORT
llpkgc_frag_errno_t llpkgc_frag_execute(llpkgc_frag_t* parser, const char* data,
    size_t len);

LLPKGC_FRAG_EXPORT
llpkgc_frag_errno_t llpkgc_frag_finish(llpkgc_frag_t* parser);

LLPKGC_FRAG_EXPORT
llpkgc_frag_errno_t llpkgc_frag_get_errno(const llpkgc_frag_t* parser);

LLPKGC_FRAG_EXPORT
const char* llpkgc_frag_get_error_reason(const llpkgc_frag_t* parser);

LLPKGC_FRAG_EXPORT
void llpkgc_frag_set_error_reason(llpkgc_frag_t* parser, const char* reason);

LLPKGC_FRAG_EXPORT
const char* llpkgc_frag_get_error_pos(const llpkgc_frag_t* parser);

LLPKGC_FRAG_EXPORT
const char* llpkgc_frag_errno_name(llpkgc_frag_errno_t err);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* INCLUDE_LLPKGC_FRAG_API_H_ */
