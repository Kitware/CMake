/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm_utf8_h
#define cm_utf8_h

#ifdef __cplusplus
extern "C" {
#endif

/** Decode one UTF-8 character from the input byte range.  On success,
    stores the unicode character number in *pc and returns the first
    position not extracted.  On failure, returns 0.  */
const char* cm_utf8_decode_character(const char* first, const char* last,
                                     unsigned int* pc);

/** Returns whether a C string is a sequence of valid UTF-8 encoded Unicode
    codepoints.  Returns non-zero on success. */
int cm_utf8_is_valid(const char* s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
