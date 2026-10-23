/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#include "llpkgc_dep.h"

llpkgc_dep_comparator_t llpkgc_dep_get_comparator(const llpkgc_dep_t* parser) {
  return (llpkgc_dep_comparator_t) parser->comparator;
}
