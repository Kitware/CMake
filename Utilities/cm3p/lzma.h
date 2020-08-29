/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the liblzma configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_LIBLZMA
#  include <lzma.h> // IWYU pragma: export
#else
#  include <cmliblzma/liblzma/api/lzma.h> // IWYU pragma: export
#endif
