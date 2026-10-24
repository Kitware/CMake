/* Distributed under the OSI-approved BSD 3-Clause License.
   See accompanying file LICENSE.rst for details.  */

#include "llpkgc_file.h"
#include "llpkgc_whitespace__internal.h"

/* Callback dispatch and EOF recipes are generated from the grammar. Pending
 * punctuation is owned by the parser. Deferred whitespace is borrowed only
 * under the explicit PFCE_NEED_MORE retention contract. */
int llpkgc_file__on_ident(llpkgc_file_t*, const unsigned char*,
    const unsigned char*);
int llpkgc_file__on_value_literal(llpkgc_file_t*, const unsigned char*,
    const unsigned char*);
int llpkgc_file__on_value_literal_complete(llpkgc_file_t*, const unsigned char*,
    const unsigned char*);

int llpkgc_file__mark(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  s->mark = *p;
  return 0;
}

int llpkgc_file__emit_ident_mark(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  const unsigned char* mark = &s->mark;
  return llpkgc_file__on_ident(s, mark, mark + 1);
}

int llpkgc_file__emit_literal_mark(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  const unsigned char* mark = &s->mark;
  return llpkgc_file__on_value_literal(s, mark, mark + 1);
}

int llpkgc_file__clear_whitespace(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  s->whitespace_start = NULL;
  s->whitespace_end = NULL;
  return 0;
}

int llpkgc_file__whitespace(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  if(s->whitespace_start == NULL)
    s->whitespace_start = (void*) p;
  s->whitespace_end = (void*) endp;
  return 0;
}

/* The private delivery grammar emits each physical range separately. */
int llpkgc_file__deferred_span(llpkgc_whitespace__internal_t* delivery,
    const unsigned char* p, const unsigned char* endp) {
  llpkgc_file_t* s = delivery->parent;
  int err;
  if(p == endp)
    return 0;
  err = llpkgc_file__on_value_literal(s, p, endp);
  if(err != 0)
    return err;
  return llpkgc_file__on_value_literal_complete(s, p, endp);
}

int llpkgc_file__flush_whitespace_all(llpkgc_file_t* s, const unsigned char* p,
    const unsigned char* endp) {
  llpkgc_whitespace__internal_t delivery;
  int err;
  if(s->whitespace_start == NULL)
    return 0;
  llpkgc_whitespace__internal_init(&delivery);
  delivery.parent = s;
  err = llpkgc_whitespace__internal_execute(&delivery, s->whitespace_start,
      s->whitespace_end);
  if(err != 0 && s->reason == NULL)
    s->reason = delivery.reason;
  llpkgc_file__clear_whitespace(s, p, endp);
  return err;
}

const char* llpkgc_file_get_retained_start(const llpkgc_file_t* s) {
  return s->whitespace_start;
}

size_t llpkgc_file_get_retained_length(const llpkgc_file_t* s) {
  if(s->whitespace_start == NULL)
    return 0;
  return (size_t) ((const char*) s->end - (const char*) s->whitespace_start);
}
