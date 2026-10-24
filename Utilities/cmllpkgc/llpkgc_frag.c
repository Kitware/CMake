/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#include "llpkgc_frag.h"

int llpkgc_frag__on_fragment_text(llpkgc_frag_t*, const unsigned char*,
    const unsigned char*);

int llpkgc_frag__emit_mark(llpkgc_frag_t* s, const unsigned char* p,
    const unsigned char* endp) {
  static const unsigned char backslash[] = "\\";
  return llpkgc_frag__on_fragment_text(s, backslash, backslash + 1);
}
