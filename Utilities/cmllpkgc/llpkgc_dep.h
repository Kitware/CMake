/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#ifndef INCLUDE_LLPKGC_DEP_API_H_
#define INCLUDE_LLPKGC_DEP_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#include "llpkgc_dep__internal.h"
#include "llpkgc_types.h"

#if defined(_MSC_VER)
#define LLPKGC_DEP_EXPORT __declspec(dllexport)
#else
#define LLPKGC_DEP_EXPORT __attribute__((visibility("default")))
#endif

typedef llpkgc_dep__internal_t llpkgc_dep_t;
typedef struct llpkgc_dep_settings_s llpkgc_dep_settings_t;

typedef int (*llpkgc_dep_data_cb)(llpkgc_dep_t*, const char* at, size_t length);
typedef int (*llpkgc_dep_cb)(llpkgc_dep_t*);

struct llpkgc_dep_settings_s {
  llpkgc_dep_data_cb on_dep_name;
  llpkgc_dep_data_cb on_dep_version;

  llpkgc_dep_cb on_dep_complete;
  llpkgc_dep_cb on_dep_list_complete;
};

enum llpkgc_dep_errno {
  PDEP_OK = 0,
  PDEP_USER = 2,
  PDEP_UNFINISHED = 3,
  PDEP_INVALID = 4,
  PDEP_INTERNAL = 5,
};
typedef enum llpkgc_dep_errno llpkgc_dep_errno_t;

LLPKGC_DEP_EXPORT
void llpkgc_dep_init(llpkgc_dep_t* parser,
    const llpkgc_dep_settings_t* settings);

LLPKGC_DEP_EXPORT
void llpkgc_dep_reset(llpkgc_dep_t* parser);

LLPKGC_DEP_EXPORT
void llpkgc_dep_settings_init(llpkgc_dep_settings_t* settings);

/* Successful chunks are fully consumed and may be released immediately.
 * Subsequent chunks need not be contiguous. Callback spans are borrowed. */
LLPKGC_DEP_EXPORT
llpkgc_dep_errno_t llpkgc_dep_execute(llpkgc_dep_t* parser, const char* data,
    size_t len);

LLPKGC_DEP_EXPORT
llpkgc_dep_errno_t llpkgc_dep_finish(llpkgc_dep_t* parser);

/* Read during on_dep_complete. Unknown/absent operators are ANY. */
LLPKGC_DEP_EXPORT
llpkgc_dep_comparator_t llpkgc_dep_get_comparator(const llpkgc_dep_t* parser);

LLPKGC_DEP_EXPORT
llpkgc_dep_errno_t llpkgc_dep_get_errno(const llpkgc_dep_t* parser);

LLPKGC_DEP_EXPORT
const char* llpkgc_dep_get_error_reason(const llpkgc_dep_t* parser);

LLPKGC_DEP_EXPORT
void llpkgc_dep_set_error_reason(llpkgc_dep_t* parser, const char* reason);

LLPKGC_DEP_EXPORT
const char* llpkgc_dep_get_error_pos(const llpkgc_dep_t* parser);

LLPKGC_DEP_EXPORT
const char* llpkgc_dep_errno_name(llpkgc_dep_errno_t err);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* INCLUDE_LLPKGC_DEP_API_H_ */
