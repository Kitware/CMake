/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmDuration.h"

class cmBuildArgs
{
public:
  static constexpr int NO_BUILD_PARALLEL_LEVEL = -1;
  static constexpr int DEFAULT_BUILD_PARALLEL_LEVEL = 0;

  std::string projectName;
  std::string binaryDir;
  int jobs = NO_BUILD_PARALLEL_LEVEL;
  bool verbose = false;
  std::string config;
  cmDuration timeout;
};
