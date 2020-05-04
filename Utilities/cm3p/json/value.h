/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm3p_json_value_h
#define cm3p_json_value_h

/* Use the jsoncpp library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_JSONCPP
#  include <json/value.h> // IWYU pragma: export
#else
#  include <cmjsoncpp/include/json/value.h> // IWYU pragma: export
#endif

#endif
