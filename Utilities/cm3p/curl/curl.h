/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the curl library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_CURL
#  include <curl/curl.h> // IWYU pragma: export
#else
#  include <cmcurl/include/curl/curl.h> // IWYU pragma: export
#endif
