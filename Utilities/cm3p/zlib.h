/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the zlib library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_ZLIB
#  include <zlib.h> // IWYU pragma: export
#else
#  include <cmzlib/zlib.h> // IWYU pragma: export
#endif
