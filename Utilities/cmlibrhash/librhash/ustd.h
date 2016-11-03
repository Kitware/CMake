/* ustd.h common macros and includes */
#ifndef LIBRHASH_USTD_H
#define LIBRHASH_USTD_H

#if _MSC_VER >= 1300

# define int64_t __int64
# define int32_t __int32
# define int16_t __int16
# define int8_t  __int8
# define uint64_t unsigned __int64
# define uint32_t unsigned __int32
# define uint16_t unsigned __int16
# define uint8_t  unsigned __int8

/* disable warnings: The POSIX name for this item is deprecated. Use the ISO C++ conformant name. */
#pragma warning(disable : 4996)

#else /* _MSC_VER >= 1300 */

# include <stdint.h>
# include <unistd.h>

#endif /* _MSC_VER >= 1300 */

#if _MSC_VER <= 1300
# include <stdlib.h> /* size_t for vc6.0 */
#endif /* _MSC_VER <= 1300 */

#endif /* LIBRHASH_USTD_H */
