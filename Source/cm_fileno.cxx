/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#if !defined(_POSIX_C_SOURCE) && !defined(_WIN32) && !defined(__sun) &&       \
  !defined(__OpenBSD__)
/* POSIX APIs are needed */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _POSIX_C_SOURCE 200809L
#endif

#include "cm_fileno.hxx"

int cm_fileno(FILE* f)
{
  return fileno(f);
}
