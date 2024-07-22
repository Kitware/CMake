/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef INCLUDE_LLPKGC_API_H_
#define INCLUDE_LLPKGC_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#include "llpkgc__internal.h"

#if defined(_MSC_VER)
#define LLPKGC_EXPORT __declspec(dllexport)
#else
#define LLPKGC_EXPORT __attribute__((visibility("default")))
#endif

typedef llpkgc__internal_t llpkgc_t;
typedef struct llpkgc_settings_s llpkgc_settings_t;

typedef int (*llpkgc_data_cb)(llpkgc_t*, const char* at, size_t length);
typedef int (*llpkgc_cb)(llpkgc_t*);

struct llpkgc_settings_s {
  /* Possible return values 0, -1, PCE_USER */
  llpkgc_data_cb on_key;
  llpkgc_data_cb on_value_literal;
  llpkgc_data_cb on_value_variable;

  /* Possible return values 0, -1, `PCE_PAUSED` */
  llpkgc_cb on_line_begin;
  llpkgc_cb on_keyword_complete;
  llpkgc_cb on_variable_complete;
  llpkgc_cb on_value_literal_complete;
  llpkgc_cb on_value_variable_complete;
  llpkgc_cb on_value_complete;
  llpkgc_cb on_pkgc_complete;
};

enum llpkgc_errno {
  PCE_OK = 0,
  PCE_INTERNAL = 1,
  PCE_PAUSED = 2,
  PCE_USER = 3,
  PCE_UNFINISHED = 4,
};
typedef enum llpkgc_errno llpkgc_errno_t;

/* Initialize the parser with user settings.
 *
 * NOTE: lifetime of `settings` has to be at least the same as the lifetime of
 * the `parser` here. In practice, `settings` has to be either a static
 * variable or be allocated with `malloc`, `new`, etc.
 */
LLPKGC_EXPORT
void llpkgc_init(llpkgc_t* parser, const llpkgc_settings_t* settings);

/* Reset an already initialized parser back to the start state, preserving the
 * existing callback settings and user data.
 */
LLPKGC_EXPORT
void llpkgc_reset(llpkgc_t* parser);

/* Initialize the settings object */
LLPKGC_EXPORT
void llpkgc_settings_init(llpkgc_settings_t* settings);

/* Parse full or partial pc data, invoking user callbacks along the way.
 *
 * If any of `llpkgc_data_cb` returns errno not equal to `PCE_OK` - the parsing
 * interrupts, and such errno is returned from `llpkgc_execute()`. If
 * `PCE_PAUSED` was used as an errno, the execution can be resumed with
 * `llpkgc_resume()` call.
 *
 * NOTE: if this function ever returns a non-pause type error, it will continue
 * to return the same error upon each successive call up until `llpkgc_init()`
 * or `llpkgc_reset()` are called.
 */
LLPKGC_EXPORT
llpkgc_errno_t llpkgc_execute(llpkgc_t* parser, const char* data, size_t len);

/* This method should be called when the input has reached EOF
 *
 * This method will invoke `on_pkgc_complete()` callback if the file was
 * terminated safely. Otherwise an error code will be returned.
 */
LLPKGC_EXPORT
llpkgc_errno_t llpkgc_finish(llpkgc_t* parser);

/* Make further calls of `llpkgc_execute()` return `PCE_PAUSED` and set
 * appropriate error reason.
 *
 * Important: do not call this from user callbacks! User callbacks must return
 * `PCE_PAUSED` if pausing is required.
 */
LLPKGC_EXPORT
void llpkgc_pause(llpkgc_t* parser);

/* Might be called to resume the execution after the pause in user's callback.
 * See `llpkgc_execute()` above for details.
 *
 * Call this only if `llpkgc_execute()` returns `PCE_PAUSED`.
 */
LLPKGC_EXPORT
void llpkgc_resume(llpkgc_t* parser);

/* Returns the latest return error */
LLPKGC_EXPORT
llpkgc_errno_t llpkgc_get_errno(const llpkgc_t* parser);

/* Returns the verbal explanation of the latest returned error.
 *
 * Note: User callback should set error reason when returning the error. See
 * `llpkgc_set_error_reason()` for details.
 */
LLPKGC_EXPORT
const char* llpkgc_get_error_reason(const llpkgc_t* parser);

/* Assign verbal description to the returned error. Must be called in user
 * callbacks right before returning the errno.
 *
 * Note: `PCE_USER` error code might be useful in user callbacks.
 */
LLPKGC_EXPORT
void llpkgc_set_error_reason(llpkgc_t* parser, const char* reason);

/* Returns the pointer to the last parsed byte before the returned error. The
 * pointer is relative to the `data` argument of `llpkgc_execute()`.
 *
 * Note: this method might be useful for counting the number of parsed bytes.
 */
LLPKGC_EXPORT
const char* llpkgc_get_error_pos(const llpkgc_t* parser);

/* Returns textual name of error code */
LLPKGC_EXPORT
const char* llpkgc_errno_name(llpkgc_errno_t err);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* INCLUDE_LLPKGC_API_H_ */
