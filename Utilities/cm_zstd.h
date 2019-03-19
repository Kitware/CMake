/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm_zstd_h
#define cm_zstd_h

/* Use the libzstd configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_ZSTD
#  include <zstd.h>
#else
#  include <cmzstd/lib/zstd.h>
#endif

#endif
