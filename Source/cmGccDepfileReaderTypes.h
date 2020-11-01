/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

struct cmGccStyleDependency
{
  std::vector<std::string> rules;
  std::vector<std::string> paths;
};

using cmGccDepfileContent = std::vector<cmGccStyleDependency>;
