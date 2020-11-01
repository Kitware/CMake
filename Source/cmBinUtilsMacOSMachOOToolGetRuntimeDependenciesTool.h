/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include <string>
#include <vector>

#include "cmBinUtilsMacOSMachOGetRuntimeDependenciesTool.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool
  : public cmBinUtilsMacOSMachOGetRuntimeDependenciesTool
{
public:
  cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);

  bool GetFileInfo(std::string const& file, std::vector<std::string>& libs,
                   std::vector<std::string>& rpaths) override;
};
