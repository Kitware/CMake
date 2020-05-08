/* ustd.h common macros and includes */
#ifndef LIBRHASH_USTD_H
#define LIBRHASH_USTD_H

/* Include KWSys Large File Support configuration. */
#include <cmsys/Configure.h>

#if defined(_MSC_VER)
# pragma warning(push,1)
#endif

#include <cm3p/kwiml/int.h>

#ifndef KWIML_INT_HAVE_INT64_T
# define int64_t KWIML_INT_int64_t
#endif
#ifndef KWIML_INT_HAVE_INT32_T
# define int32_t KWIML_INT_int32_t
#endif
#ifndef KWIML_INT_HAVE_INT16_T
# define int16_t KWIML_INT_int16_t
#endif
#ifndef KWIML_INT_HAVE_INT8_T
# define int8_t KWIML_INT_int8_t
#endif
#ifndef KWIML_INT_HAVE_UINT64_T
# define uint64_t KWIML_INT_uint64_t
#endif
#ifndef KWIML_INT_HAVE_UINT32_T
# define uint32_t KWIML_INT_uint32_t
#endif
#ifndef KWIML_INT_HAVE_UINT16_T
# define uint16_t KWIML_INT_uint16_t
#endif
#ifndef KWIML_INT_HAVE_UINT8_T
# define uint8_t KWIML_INT_uint8_t
#endif

#include <stddef.h>

#if 0
#if _MSC_VER > 1000
# include <stddef.h> /* size_t for vc6.0 */

# if _MSC_VER >= 1600
/* Visual Studio >= 2010 has stdint.h */
#  include <stdint.h>
# else
  /* vc6.0 has bug with __int8, so using char instead */
  typedef signed char       int8_t;
  typedef signed __int16    int16_t;
  typedef signed __int32    int32_t;
  typedef signed __int64    int64_t;
  typedef unsigned char     uint8_t;
  typedef unsigned __int16  uint16_t;
  typedef unsigned __int32  uint32_t;
  typedef unsigned __int64  uint64_t;
# endif /* _MSC_VER >= 1600 */

/* disable warnings: The POSIX name for this item is deprecated. Use the ISO C++ conformant name. */
# pragma warning(disable : 4996)

#else /* _MSC_VER > 1000 */

# include <stdint.h>
# include <unistd.h>

#endif /* _MSC_VER > 1000 */
#endif

#endif /* LIBRHASH_USTD_H */
