/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the libzstd configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_ZSTD
#  include <zstd.h> // IWYU pragma: export
#else
#  include <cmzstd/lib/zstd.h> // IWYU pragma: export
#endif
