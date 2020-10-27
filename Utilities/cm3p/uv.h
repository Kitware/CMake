/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the libuv library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_LIBUV
#  include <uv.h> // IWYU pragma: export
#else
#  include <cmlibuv/include/uv.h> // IWYU pragma: export
#endif
