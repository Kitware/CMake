/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#ifndef INCLUDE_LLPKGC_H_
#define INCLUDE_LLPKGC_H_

#include "llpkgc_dep.h"
#include "llpkgc_file.h"
#include "llpkgc_frag.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ASCII case-insensitive field classification. Unknown fields are literals. */
LLPKGC_FILE_EXPORT
llpkgc_value_kind_t llpkgc_classify_field(const char* name, size_t length);

LLPKGC_DEP_EXPORT
const char* llpkgc_dep_comparator_name(llpkgc_dep_comparator_t comparator);

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_LLPKGC_H_ */
