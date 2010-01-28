#include "config.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#endif

#include <stdio.h>

#define CHECK(t,m) do {                                                 \
  if(sizeof(t) != m)                                                    \
    {                                                                   \
    printf(#m ": expected %d, got %d (line %d)\n",                      \
           (int)sizeof(t), (int)m, __LINE__);                           \
    result = 1;                                                         \
    }                                                                   \
  } while(0)

#define NODEF(m) do {                                                   \
  printf(#m": not defined (line %d)\n", __LINE__);                      \
  result = 1;                                                           \
  } while(0)

int main()
{
  int result = 0;

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
# if !defined(HAVE_SIZEOF_LONG_LONG)
  NODEF(HAVE_SIZEOF_LONG_LONG);
# endif
#endif

  /* __int64 */
#if defined(SIZEOF___INT64)
  CHECK(__int64, SIZEOF___INT64);
# if !defined(HAVE_SIZEOF___INT64)
  NODEF(HAVE_SIZEOF___INT64);
# endif
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
# if !defined(HAVE_SIZEOF_SSIZE_T)
  NODEF(HAVE_SIZEOF_SSIZE_T);
# endif
#elif defined(HAVE_SIZEOF_SSIZE_T)
  NODEF(SIZEOF_SSIZE_T);
#endif

  return result;
}
