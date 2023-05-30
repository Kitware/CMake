/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/* Use the cppdap library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_CPPDAP
#  include <dap/session.h> // IWYU pragma: export
#else
#  include <cmcppdap/include/dap/session.h> // IWYU pragma: export
#endif
