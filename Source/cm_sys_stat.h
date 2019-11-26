/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm_sys_stat_h
#define cm_sys_stat_h

#if defined(_MSC_VER)
using mode_t = unsigned short;
#endif

#include <sys/types.h>
// include sys/stat.h after sys/types.h
#include <sys/stat.h> // IWYU pragma: export

#endif
