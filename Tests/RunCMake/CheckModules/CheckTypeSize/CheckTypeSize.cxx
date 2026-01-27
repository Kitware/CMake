#include "config.h"
#include "config.hxx"
#include "someclass.hxx"
#include "somestruct.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#ifdef HAVE_STDDEF_H
#  include <stddef.h>
#endif
#ifdef HAVE_CSTDINT
#  include <cstdint>
#endif
#ifdef HAVE_CSTDDEF
#  include <cstddef>
#endif

#include <stdio.h>

#define CHECK(t, m)                                                           \
  do {                                                                        \
    if (sizeof(t) != m) {                                                     \
      printf(#m ": expected %d, got %d (line %d)\n", (int)sizeof(t), (int)m,  \
             __LINE__);                                                       \
      result = 1;                                                             \
    }                                                                         \
  } while (0)

#define NODEF(m)                                                              \
  do {                                                                        \
    printf(#m ": not defined (line %d)\n", __LINE__);                         \
    result = 1;                                                               \
  } while (0)

#define DEF(m)                                                                \
  do {                                                                        \
    printf(#m ": expected undefined, got defined (line %d)\n", __LINE__);     \
    result = 1;                                                               \
  } while (0)

int main()
{
  int result = 0;
  ns::somestruct x;
  ns::someclass y;

/* void* */
#if !defined(HAVE_SIZEOF_DATA_PTR)
  NODEF(HAVE_SIZEOF_DATA_PTR);
#endif
#if defined(SIZEOF_DATA_PTR)
  CHECK(void*, SIZEOF_DATA_PTR);
#else
  NODEF(SIZEOF_DATA_PTR);
#endif

/* char */
#if !defined(HAVE_SIZEOF_CHAR)
  NODEF(HAVE_SIZEOF_CHAR);
#endif
#if defined(SIZEOF_CHAR)
  CHECK(char, SIZEOF_CHAR);
#else
  NODEF(SIZEOF_CHAR);
#endif

/* short */
#if !defined(HAVE_SIZEOF_SHORT)
  NODEF(HAVE_SIZEOF_SHORT);
#endif
#if defined(SIZEOF_SHORT)
  CHECK(short, SIZEOF_SHORT);
#else
  NODEF(SIZEOF_SHORT);
#endif

/* int */
#if !defined(HAVE_SIZEOF_INT)
  NODEF(HAVE_SIZEOF_INT);
#endif
#if defined(SIZEOF_INT)
  CHECK(int, SIZEOF_INT);
#else
  NODEF(SIZEOF_INT);
#endif

/* long */
#if !defined(HAVE_SIZEOF_LONG)
  NODEF(HAVE_SIZEOF_LONG);
#endif
#if defined(SIZEOF_LONG)
  CHECK(long, SIZEOF_LONG);
#else
  NODEF(SIZEOF_LONG);
#endif

/* long long */
#if defined(SIZEOF_LONG_LONG)
  CHECK(long long, SIZEOF_LONG_LONG);
#  if !defined(HAVE_SIZEOF_LONG_LONG)
  NODEF(HAVE_SIZEOF_LONG_LONG);
#  endif
#endif

/* __int64 */
#if defined(SIZEOF___INT64)
  CHECK(__int64, SIZEOF___INT64);
#  if !defined(HAVE_SIZEOF___INT64)
  NODEF(HAVE_SIZEOF___INT64);
#  endif
#elif defined(HAVE_SIZEOF___INT64)
  NODEF(SIZEOF___INT64);
#endif

/* size_t */
#if !defined(HAVE_SIZEOF_SIZE_T)
  NODEF(HAVE_SIZEOF_SIZE_T);
#endif
#if defined(SIZEOF_SIZE_T)
  CHECK(size_t, SIZEOF_SIZE_T);
#else
  NODEF(SIZEOF_SIZE_T);
#endif

/* ssize_t */
#if defined(SIZEOF_SSIZE_T)
  CHECK(ssize_t, SIZEOF_SSIZE_T);
#  if !defined(HAVE_SIZEOF_SSIZE_T)
  NODEF(HAVE_SIZEOF_SSIZE_T);
#  endif
#elif defined(HAVE_SIZEOF_SSIZE_T)
  NODEF(SIZEOF_SSIZE_T);
#endif

/* uint8_t */
#if defined(SIZEOF_UINT8_T)
  CHECK(uint8_t, SIZEOF_UINT8_T);
#  if !defined(HAVE_SIZEOF_UINT8_T)
  NODEF(HAVE_SIZEOF_UINT8_T);
#  endif
#elif defined(HAVE_SIZEOF_UINT8_T)
  NODEF(SIZEOF_UINT8_T);
#endif

/* std::uint8_t */
#if defined(SIZEOF_STD_UINT8_T)
  CHECK(std::uint8_t, SIZEOF_STD_UINT8_T);
#  if !defined(HAVE_SIZEOF_STD_UINT8_T)
  NODEF(HAVE_SIZEOF_STD_UINT8_T);
#  endif
#elif defined(HAVE_SIZEOF_STD_UINT8_T)
  NODEF(SIZEOF_STD_UINT8_T);
#endif

/* struct ns::somestruct::someint */
#if defined(SIZEOF_NS_STRUCTMEMBER_INT)
  CHECK(x.someint, SIZEOF_NS_STRUCTMEMBER_INT);
  CHECK(x.someint, SIZEOF_INT);
#  if !defined(HAVE_SIZEOF_NS_STRUCTMEMBER_INT)
  NODEF(HAVE_SIZEOF_NS_STRUCTMEMBER_INT);
#  endif
#elif defined(HAVE_SIZEOF_NS_STRUCTMEMBER_INT)
  NODEF(SIZEOF_NS_STRUCTMEMBER_INT);
#endif

/* struct ns::somestruct::someptr */
#if defined(SIZEOF_NS_STRUCTMEMBER_PTR)
  CHECK(x.someptr, SIZEOF_NS_STRUCTMEMBER_PTR);
  CHECK(x.someptr, SIZEOF_DATA_PTR);
#  if !defined(HAVE_SIZEOF_NS_STRUCTMEMBER_PTR)
  NODEF(HAVE_SIZEOF_NS_STRUCTMEMBER_PTR);
#  endif
#elif defined(HAVE_SIZEOF_NS_STRUCTMEMBER_PTR)
  NODEF(SIZEOF_NS_STRUCTMEMBER_PTR);
#endif

/* struct ns::somestruct::somechar */
#if defined(SIZEOF_NS_STRUCTMEMBER_CHAR)
  CHECK(x.somechar, SIZEOF_NS_STRUCTMEMBER_CHAR);
  CHECK(x.somechar, SIZEOF_CHAR);
#  if !defined(HAVE_SIZEOF_NS_STRUCTMEMBER_CHAR)
  NODEF(HAVE_SIZEOF_NS_STRUCTMEMBER_CHAR);
#  endif
#elif defined(HAVE_SIZEOF_NS_STRUCTMEMBER_CHAR)
  NODEF(SIZEOF_NS_STRUCTMEMBER_CHAR);
#endif

/* struct ns::somestruct::somelong */
#if defined(SIZEOF_NS_STRUCTMEMBER_LONG)
  CHECK(x.somelong, SIZEOF_NS_STRUCTMEMBER_LONG);
  CHECK(x.somelong, SIZEOF_LONG);
#  if !defined(HAS_SIZEOF_NS_STRUCTMEMBER_LONG)
  NODEF(HAS_SIZEOF_NS_STRUCTMEMBER_LONG);
#  endif
#elif defined(HAS_SIZEOF_NS_STRUCTMEMBER_LONG)
  NODEF(SIZEOF_NS_STRUCTMEMBER_LONG);
#endif
#if defined(HAVE_SIZEOF_NS_STRUCTMEMBER_LONG)
  DEF(HAVE_SIZEOF_NS_STRUCTMEMBER_LONG);
#endif

/* ns::someclass::someint */
#if defined(SIZEOF_NS_CLASSMEMBER_INT)
  CHECK(y.someint, SIZEOF_NS_CLASSMEMBER_INT);
  CHECK(y.someint, SIZEOF_INT);
#  if !defined(HAVE_SIZEOF_NS_CLASSMEMBER_INT)
  NODEF(HAVE_SIZEOF_NS_CLASSMEMBER_INT);
#  endif
#elif defined(HAVE_SIZEOF_NS_CLASSMEMBER_INT)
  NODEF(SIZEOF_NS_CLASSMEMBER_INT);
#endif

/* ns::someclass::someptr */
#if defined(SIZEOF_NS_CLASSMEMBER_PTR)
  CHECK(y.someptr, SIZEOF_NS_CLASSMEMBER_PTR);
  CHECK(y.someptr, SIZEOF_DATA_PTR);
#  if !defined(HAVE_SIZEOF_NS_CLASSMEMBER_PTR)
  NODEF(HAVE_SIZEOF_NS_CLASSMEMBER_PTR);
#  endif
#elif defined(HAVE_SIZEOF_NS_CLASSMEMBER_PTR)
  NODEF(SIZEOF_NS_CLASSMEMBER_PTR);
#endif

/* ns::someclass::somechar */
#if defined(SIZEOF_NS_CLASSMEMBER_CHAR)
  CHECK(y.somechar, SIZEOF_NS_CLASSMEMBER_CHAR);
  CHECK(y.somechar, SIZEOF_CHAR);
#  if !defined(HAVE_SIZEOF_NS_CLASSMEMBER_CHAR)
  NODEF(HAVE_SIZEOF_NS_CLASSMEMBER_CHAR);
#  endif
#elif defined(HAVE_SIZEOF_NS_CLASSMEMBER_CHAR)
  NODEF(SIZEOF_NS_CLASSMEMBER_CHAR);
#endif

/* ns::someclass::somebool */
#if defined(SIZEOF_NS_CLASSMEMBER_BOOL)
  CHECK(y.somebool, SIZEOF_NS_CLASSMEMBER_BOOL);
  CHECK(y.somebool, SIZEOF_BOOL);
#  if !defined(HAVE_SIZEOF_NS_CLASSMEMBER_BOOL)
  NODEF(HAVE_SIZEOF_NS_CLASSMEMBER_BOOL);
#  endif
#elif defined(HAVE_SIZEOF_NS_CLASSMEMBER_BOOL)
  NODEF(SIZEOF_NS_CLASSMEMBER_BOOL);
#endif

/* ns::someclass::somelong */
#if defined(SIZEOF_NS_CLASSMEMBER_LONG)
  CHECK(y.somelong, SIZEOF_NS_CLASSMEMBER_LONG);
  CHECK(y.somelong, SIZEOF_LONG);
#  if !defined(HAS_SIZEOF_NS_CLASSMEMBER_LONG)
  NODEF(HAS_SIZEOF_NS_CLASSMEMBER_LONG);
#  endif
#elif defined(HAS_SIZEOF_NS_CLASSMEMBER_LONG)
  NODEF(SIZEOF_NS_CLASSMEMBER_LONG);
#endif
#if defined(HAVE_SIZEOF_NS_CLASSMEMBER_LONG)
  DEF(HAVE_SIZEOF_NS_CLASSMEMBER_LONG);
#endif

  /* to avoid possible warnings about unused or write-only variable */
  y.someint = result;

  return y.someint;
}
