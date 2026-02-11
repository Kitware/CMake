/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#ifdef KWSYS_STRING_C
/*
All code in this source file is conditionally compiled to work-around
template definition auto-search on VMS.  Other source files in this
directory that use the stl string cause the compiler to load this
source to try to get the definition of the string template.  This
condition blocks the compiler from seeing the symbols defined here.
*/
#  include "kwsysPrivate.h"
#  include KWSYS_HEADER(String.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#  if 0
#    include "String.h.in"
#  endif

#  include <ctype.h>

int kwsysString_isalnum(char c)
{
  return (kwsysString_isalpha(c) | /* bitwise-or to avoid branching.  */
          kwsysString_isdigit(c));
}

int kwsysString_isalpha(char c)
{
  return (kwsysString_islower(c) | /* bitwise-or to avoid branching.  */
          kwsysString_isupper(c));
}

int kwsysString_isascii(char c)
{
  unsigned char const uc = (unsigned char)c;
  return (uc & 0x80) ^ 0x80;
}

int kwsysString_isblank(char c)
{
  unsigned char const uc = (unsigned char)c;
  return uc < 0x80 && isblank(uc);
}

int kwsysString_iscntrl(char c)
{
  unsigned char const uc = (unsigned char)c;
  return uc < 0x80 && iscntrl(uc);
}

int kwsysString_isdigit(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push '0-9' to 246-255 so integer division by 246 is 1 only for them.  */
  return ((uc + 246 - '0') & 0xFF) / 246;
}

int kwsysString_isgraph(char c)
{
  unsigned char const uc = (unsigned char)c;
  return uc < 0x80 && isgraph(uc);
}

int kwsysString_islower(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push 'a-z' to 230-255 so integer division by 230 is 1 only for them.  */
  return ((uc + 230 - 'a') & 0xFF) / 230;
}

int kwsysString_isprint(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push 32-126 to 161-255 so integer division by 161 is 1 only for them.  */
  return ((uc + 161 - ' ') & 0xFF) / 161;
}

int kwsysString_ispunct(char c)
{
  unsigned char const uc = (unsigned char)c;
  return uc < 0x80 && ispunct(uc);
}

int kwsysString_isspace(char c)
{
  unsigned char const uc = (unsigned char)c;
  return uc < 0x80 && isspace(uc);
}

int kwsysString_isupper(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push 'A-Z' to 230-255 so integer division by 230 is 1 only for them.  */
  return ((uc + 230 - 'A') & 0xFF) / 230;
}

int kwsysString_isxdigit(char c)
{
  unsigned char const uc = (unsigned char)c;
  return (
    /* Push '0-9' to 246-255 so integer division by 246 is 1 only for them.  */
    (((uc + 246 - '0') & 0xFF) / 246) | /* bitwise-or to avoid branching.    */
    /* Push 'A-F' to 250-255 so integer division by 250 is 1 only for them.  */
    (((uc + 250 - 'A') & 0xFF) / 250) | /* bitwise-or to avoid branching.    */
    /* Push 'a-f' to 250-255 so integer division by 250 is 1 only for them.  */
    (((uc + 250 - 'a') & 0xFF) / 250));
}

unsigned char kwsysString_tolower(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push 'A-Z' to 230-255 so integer division by 230 is 1 only for them.  */
  return (unsigned char)(uc ^ ((((uc + 230 - 'A') & 0xFF) / 230) << 5));
}

unsigned char kwsysString_toupper(char c)
{
  unsigned char const uc = (unsigned char)c;
  /* Push 'a-z' to 230-255 so integer division by 230 is 1 only for them.  */
  return (unsigned char)(uc ^ ((((uc + 230 - 'a') & 0xFF) / 230) << 5));
}

int kwsysString_strcasecmp(char const* lhs, char const* rhs)
{
  int result;
  while ((result = kwsysString_tolower(*lhs) - kwsysString_tolower(*rhs++)) ==
           0 &&
         *lhs++) {
  }
  return result;
}

int kwsysString_strncasecmp(char const* lhs, char const* rhs, size_t n)
{
  int result = 0;
  while (n &&
         (result = kwsysString_tolower(*lhs) - kwsysString_tolower(*rhs++)) ==
           0 &&
         *lhs++) {
    --n;
  }
  return result;
}

#endif /* KWSYS_STRING_C */
