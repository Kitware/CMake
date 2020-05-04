/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm3p_bzlib_h
#define cm3p_bzlib_h

/* Use the bzip2 library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_BZIP2
#  include <bzlib.h> // IWYU pragma: export
#else
#  include <cmbzip2/bzlib.h> // IWYU pragma: export
#endif

#endif
