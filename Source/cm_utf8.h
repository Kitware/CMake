/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char const cm_utf8_ones[256];

/** Decode one UTF-8 character from the input byte range.  On success,
    stores the unicode character number in *pc and returns the first
    position not extracted.  On failure, returns 0.  */
char const* cm_utf8_decode_character(char const* first, char const* last,
                                     unsigned int* pc);

/** Returns whether size bytes form a sequence of valid UTF-8 encoded Unicode
    codepoints.  The input need not be null-terminated and may contain null
    bytes.  Returns non-zero on success, or zero for invalid or null input. */
int cm_utf8_is_valid(char const* s, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif
