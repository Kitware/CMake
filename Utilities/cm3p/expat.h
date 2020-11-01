/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the expat library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_EXPAT
#  include <expat.h> // IWYU pragma: export
#else
#  include <cmexpat/lib/expat.h> // IWYU pragma: export
#endif
