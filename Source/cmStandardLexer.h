/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#if defined(__linux)
/* Needed for glibc < 2.12 */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 600
#endif
#if !defined(_POSIX_C_SOURCE) && !defined(_WIN32) && !defined(__sun) &&       \
  !defined(__OpenBSD__)
/* POSIX APIs are needed */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _POSIX_C_SOURCE 200809L
#endif
#if defined(__sun) && defined(__GNUC__) && !defined(__cplusplus)
/* C sources: for fileno and strdup */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 600
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__)
/* For isascii */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 700
#endif

#include "cmsys/Configure.h" // IWYU pragma: keep

/* Disable some warnings.  */
#if defined(_MSC_VER)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4127)
#  pragma warning(disable : 4131)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4251)
#  pragma warning(disable : 4267)
#  pragma warning(disable : 4305)
#  pragma warning(disable : 4309)
#  pragma warning(disable : 4706)
#  pragma warning(disable : 4786)
#endif

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
#  if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-compare"
#  endif
#  if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 403
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#endif

#if defined(__LCC__)
#  pragma diag_suppress 1873 /* comparison between signed and unsigned */
#endif

#if defined(__NVCOMPILER)
#  pragma diag_suppress 111 /* statement is unreachable */
#  pragma diag_suppress 550 /* variable set but never used */
#endif

/* Make sure isatty is available. */
#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  if defined(_MSC_VER)
#    define isatty _isatty
#  endif
#else
#  include <unistd.h> // IWYU pragma: export
#endif

/* Make sure malloc and free are available on QNX.  */
#ifdef __QNX__
#  include <malloc.h>
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1
#define ECHO

#include <cm3p/kwiml/int.h>
typedef KWIML_INT_int8_t flex_int8_t;
typedef KWIML_INT_uint8_t flex_uint8_t;
typedef KWIML_INT_int16_t flex_int16_t;
typedef KWIML_INT_uint16_t flex_uint16_t;
typedef KWIML_INT_int32_t flex_int32_t;
typedef KWIML_INT_uint32_t flex_uint32_t;
