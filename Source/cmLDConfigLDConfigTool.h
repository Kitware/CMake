/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include <string>
#include <vector>

#include "cmLDConfigTool.h"

class cmRuntimeDependencyArchive;

class cmLDConfigLDConfigTool : public cmLDConfigTool
{
public:
  cmLDConfigLDConfigTool(cmRuntimeDependencyArchive* archive);

  bool GetLDConfigPaths(std::vector<std::string>& paths) override;
};
