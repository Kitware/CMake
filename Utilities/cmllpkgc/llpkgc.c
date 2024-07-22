/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <stdlib.h>
#include <string.h>

#include "llpkgc.h"

#define CALLBACK_MAYBE(PARSER, NAME)                                           \
  do {                                                                         \
    const llpkgc_settings_t* settings;                                         \
    settings = (const llpkgc_settings_t*) (PARSER)->settings;                  \
    if(settings == NULL || settings->NAME == NULL) {                           \
      err = 0;                                                                 \
      break;                                                                   \
    }                                                                          \
    err = settings->NAME((PARSER));                                            \
  } while(0)

#define SPAN_CALLBACK_MAYBE(PARSER, NAME, START, LEN)                          \
  do {                                                                         \
    const llpkgc_settings_t* settings;                                         \
    settings = (const llpkgc_settings_t*) (PARSER)->settings;                  \
    if(settings == NULL || settings->NAME == NULL) {                           \
      err = 0;                                                                 \
      break;                                                                   \
    }                                                                          \
    err = settings->NAME((PARSER), (START), (LEN));                            \
    if(err == -1) {                                                            \
      err = PCE_USER;                                                          \
      llpkgc_set_error_reason((PARSER), "Span callback error in " #NAME);      \
    }                                                                          \
  } while(0)

void llpkgc_init(llpkgc_t* parser, const llpkgc_settings_t* settings) {
  llpkgc__internal_init(parser);

  parser->settings = (void*) settings;
}

void llpkgc_reset(llpkgc_t* parser) {
  llpkgc_settings_t* settings = parser->settings;
  void* data = parser->data;

  llpkgc__internal_init(parser);

  parser->settings = settings;
  parser->data = data;
}

void llpkgc_settings_init(llpkgc_settings_t* settings) {
  memset(settings, 0, sizeof(*settings));
}

llpkgc_errno_t llpkgc_execute(llpkgc_t* parser, const char* data, size_t len) {
  return llpkgc__internal_execute(parser, data, data + len);
}

llpkgc_errno_t llpkgc_finish(llpkgc_t* parser) {
  if(parser->error != 0)
    return parser->error;

  int err;
  // ToDo: Better handling of user callback errors here
  if(parser->unfinished_ == 1) {
    parser->reason = "Invalid EOF state";
    parser->error = PCE_UNFINISHED;
    return PCE_UNFINISHED;
  } else if(parser->unfinished_ == 2) {
    CALLBACK_MAYBE(parser, on_value_literal_complete);
    if(err != PCE_OK) {
      parser->error = err;
      return err;
    }
    CALLBACK_MAYBE(parser, on_value_complete);
    if(err != PCE_OK) {
      parser->error = err;
      return err;
    }
  } else if(parser->unfinished_ == 3) {
    CALLBACK_MAYBE(parser, on_value_complete);
    if(err != PCE_OK) {
      parser->error = err;
      return err;
    }
  }

  CALLBACK_MAYBE(parser, on_pkgc_complete);
  return err;
}

void llpkgc_pause(llpkgc_t* parser) {
  if(parser->error != PCE_OK) {
    return;
  }

  parser->error = PCE_PAUSED;
  parser->reason = "Paused";
}

void llpkgc_resume(llpkgc_t* parser) {
  if(parser->error != PCE_PAUSED) {
    return;
  }

  parser->error = 0;
}

llpkgc_errno_t llpkgc_get_errno(const llpkgc_t* parser) {
  return parser->error;
}

const char* llpkgc_get_error_reason(const llpkgc_t* parser) {
  return parser->reason;
}

void llpkgc_set_error_reason(llpkgc_t* parser, const char* reason) {
  parser->reason = reason;
}

const char* llpkgc_get_error_pos(const llpkgc_t* parser) {
  return parser->error_pos;
}

const char* llpkgc_errno_name(llpkgc_errno_t err) {
  switch(err) {
    case PCE_OK:
      return "PCE_OK";
    case PCE_INTERNAL:
      return "PCE_INTERNAL";
    case PCE_PAUSED:
      return "PCE_PAUSED";
    case PCE_USER:
      return "PCE_USER";
    case PCE_UNFINISHED:
      return "PCE_UNFINISHED";
  }
  return "INVALID_ERRNO";
}

int llpkgc__line_begin(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 1;
  CALLBACK_MAYBE(s, on_line_begin);
  return err;
}

int llpkgc__key_span(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_key, p, endp - p);
  return err;
}

int llpkgc__keyword_complete(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 3;
  CALLBACK_MAYBE(s, on_keyword_complete);
  return err;
}

int llpkgc__variable_complete(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 3;
  CALLBACK_MAYBE(s, on_variable_complete);
  return err;
}

int llpkgc__vallit_span(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  if(s->escaped_) {
    --endp;
    s->escaped_ = 0;
  }
  s->unfinished_ = 2;
  SPAN_CALLBACK_MAYBE(s, on_value_literal, p, endp - p);
  return err;
}

int llpkgc__vallit_complete(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 3;
  CALLBACK_MAYBE(s, on_value_literal_complete);
  return err;
}

int llpkgc__valvar_span(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 1;
  SPAN_CALLBACK_MAYBE(s, on_value_variable, p, endp - p);
  return err;
}

int llpkgc__valvar_complete(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 3;
  CALLBACK_MAYBE(s, on_value_variable_complete);
  return err;
}

int llpkgc__value_complete(llpkgc_t* s, const char* p, const char* endp) {
  int err;
  s->unfinished_ = 0;
  CALLBACK_MAYBE(s, on_value_complete);
  return err;
}
