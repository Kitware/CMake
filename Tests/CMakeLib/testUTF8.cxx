/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <stdio.h>

#include <cm_utf8.h>

typedef char test_utf8_char[5];

static void test_utf8_char_print(test_utf8_char const c)
{
  unsigned char const* d = reinterpret_cast<unsigned char const*>(c);
#ifndef __clang_analyzer__ // somehow thinks arguments are not initialized
  printf("[0x%02X,0x%02X,0x%02X,0x%02X]", static_cast<int>(d[0]),
         static_cast<int>(d[1]), static_cast<int>(d[2]),
         static_cast<int>(d[3]));
#endif
}

static void byte_array_print(char const* s)
{
  unsigned char const* d = reinterpret_cast<unsigned char const*>(s);
  bool started = false;
  printf("[");
  for (; *d; ++d) {
    if (started) {
      printf(",");
    }
    started = true;
    printf("0x%02X", static_cast<int>(*d));
  }
  printf("]");
}

struct test_utf8_entry
{
  int n;
  test_utf8_char str;
  unsigned int chr;
};

static test_utf8_entry const good_entry[] = {
  { 1, "\x20\x00\x00\x00", 0x0020 },   /* Space.  */
  { 2, "\xC2\xA9\x00\x00", 0x00A9 },   /* Copyright.  */
  { 3, "\xE2\x80\x98\x00", 0x2018 },   /* Open-single-quote.  */
  { 3, "\xE2\x80\x99\x00", 0x2019 },   /* Close-single-quote.  */
  { 4, "\xF0\xA3\x8E\xB4", 0x233B4 },  /* Example from RFC 3629.  */
  { 3, "\xED\x80\x80\x00", 0xD000 },   /* Valid 0xED prefixed codepoint.  */
  { 4, "\xF4\x8F\xBF\xBF", 0x10FFFF }, /* Highest valid RFC codepoint. */
  { 0, { 0, 0, 0, 0, 0 }, 0 }
};

static test_utf8_char const bad_chars[] = {
  "\x80\x00\x00\x00", /* Leading continuation byte. */
  "\xC0\x80\x00\x00", /* Overlong encoding. */
  "\xC1\x80\x00\x00", /* Overlong encoding. */
  "\xC2\x00\x00\x00", /* Missing continuation byte. */
  "\xE0\x00\x00\x00", /* Missing continuation bytes. */
  "\xE0\x80\x80\x00", /* Overlong encoding. */
  "\xF0\x80\x80\x80", /* Overlong encoding. */
  "\xED\xA0\x80\x00", /* UTF-16 surrogate half. */
  "\xED\xBF\xBF\x00", /* UTF-16 surrogate half. */
  "\xF4\x90\x80\x80", /* Lowest out-of-range codepoint. */
  "\xF5\x80\x80\x80", /* Prefix forces out-of-range codepoints. */
  { 0, 0, 0, 0, 0 }
};

static char const* good_strings[] = { "", "ASCII", "\xC2\xA9 Kitware", 0 };

static char const* bad_strings[] = {
  "\xC0\x80", /* Modified UTF-8 for embedded 0-byte. */
  0
};

static void report_good(bool passed, test_utf8_char const c)
{
  printf("%s: decoding good ", passed ? "pass" : "FAIL");
  test_utf8_char_print(c);
  printf(" (%s) ", c);
}

static void report_bad(bool passed, test_utf8_char const c)
{
  printf("%s: decoding bad  ", passed ? "pass" : "FAIL");
  test_utf8_char_print(c);
  printf(" ");
}

static bool decode_good(test_utf8_entry const entry)
{
  unsigned int uc;
  if (const char* e =
        cm_utf8_decode_character(entry.str, entry.str + 4, &uc)) {
    int used = static_cast<int>(e - entry.str);
    if (uc != entry.chr) {
      report_good(false, entry.str);
      printf("expected 0x%04X, got 0x%04X\n", entry.chr, uc);
      return false;
    }
    if (used != entry.n) {
      report_good(false, entry.str);
      printf("had %d bytes, used %d\n", entry.n, used);
      return false;
    }
    report_good(true, entry.str);
    printf("got 0x%04X\n", uc);
    return true;
  }
  report_good(false, entry.str);
  printf("failed\n");
  return false;
}

static bool decode_bad(test_utf8_char const s)
{
  unsigned int uc = 0xFFFFu;
  const char* e = cm_utf8_decode_character(s, s + 4, &uc);
  if (e) {
    report_bad(false, s);
    printf("expected failure, got 0x%04X\n", uc);
    return false;
  }
  report_bad(true, s);
  printf("failed as expected\n");
  return true;
}

static void report_valid(bool passed, char const* s)
{
  printf("%s: validity good ", passed ? "pass" : "FAIL");
  byte_array_print(s);
  printf(" (%s) ", s);
}

static void report_invalid(bool passed, char const* s)
{
  printf("%s: validity bad  ", passed ? "pass" : "FAIL");
  byte_array_print(s);
  printf(" ");
}

static bool is_valid(const char* s)
{
  bool valid = cm_utf8_is_valid(s) != 0;
  if (!valid) {
    report_valid(false, s);
    printf("expected valid, reported as invalid\n");
    return false;
  }
  report_valid(true, s);
  printf("valid as expected\n");
  return true;
}

static bool is_invalid(const char* s)
{
  bool valid = cm_utf8_is_valid(s) != 0;
  if (valid) {
    report_invalid(false, s);
    printf("expected invalid, reported as valid\n");
    return false;
  }
  report_invalid(true, s);
  printf("invalid as expected\n");
  return true;
}

int testUTF8(int /*unused*/, char* /*unused*/[])
{
  int result = 0;
  for (test_utf8_entry const* e = good_entry; e->n; ++e) {
    if (!decode_good(*e)) {
      result = 1;
    }
    if (!is_valid(e->str)) {
      result = 1;
    }
  }
  for (test_utf8_char const* c = bad_chars; (*c)[0]; ++c) {
    if (!decode_bad(*c)) {
      result = 1;
    }
    if (!is_invalid(*c)) {
      result = 1;
    }
  }
  for (char const** s = good_strings; *s; ++s) {
    if (!is_valid(*s)) {
      result = 1;
    }
  }
  for (char const** s = bad_strings; *s; ++s) {
    if (!is_invalid(*s)) {
      result = 1;
    }
  }
  return result;
}
