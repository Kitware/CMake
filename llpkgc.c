/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#include <string.h>

#include "llpkgc.h"

static int field_equal(const char* name, size_t length, const char* field) {
  size_t i;
  if(strlen(field) != length)
    return 0;
  for(i = 0; i < length; i++) {
    unsigned char c = (unsigned char) name[i];
    if(c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
    if(c != (unsigned char) field[i])
      return 0;
  }
  return 1;
}

llpkgc_value_kind_t llpkgc_classify_field(const char* name, size_t length) {
  static const char* const fragments[] = {
      "cflags",
      "cflags.private",
      "cflags.shared",
      "libs",
      "libs.private",
      "libs.shared",
  };
  static const char* const dependencies[] = {
      "conflicts",
      "provides",
      "requires",
      "requires.private",
      "requires.internal",
      "requires.shared",
  };
  size_t i;
  for(i = 0; i < sizeof(fragments) / sizeof(*fragments); i++)
    if(field_equal(name, length, fragments[i]))
      return LLPKGC_VALUE_FRAGMENTS;
  for(i = 0; i < sizeof(dependencies) / sizeof(*dependencies); i++)
    if(field_equal(name, length, dependencies[i]))
      return LLPKGC_VALUE_DEPENDENCIES;
  return LLPKGC_VALUE_LITERAL;
}

const char* llpkgc_dep_comparator_name(llpkgc_dep_comparator_t comparator) {
  switch(comparator) {
    case LLPKGC_DEP_LESS_THAN:
      return "<";
    case LLPKGC_DEP_LESS_THAN_EQUAL:
      return "<=";
    case LLPKGC_DEP_EQUAL:
      return "=";
    case LLPKGC_DEP_NOT_EQUAL:
      return "!=";
    case LLPKGC_DEP_GREATER_THAN:
      return ">";
    case LLPKGC_DEP_GREATER_THAN_EQUAL:
      return ">=";
    default:
      return "";
  }
}
