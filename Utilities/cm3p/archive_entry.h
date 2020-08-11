/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm3p_archive_entry_h
#define cm3p_archive_entry_h

/* Use the libarchive configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_LIBARCHIVE
#  include <archive_entry.h> // IWYU pragma: export
#else
#  include <cmlibarchive/libarchive/archive_entry.h> // IWYU pragma: export
#endif

#endif
