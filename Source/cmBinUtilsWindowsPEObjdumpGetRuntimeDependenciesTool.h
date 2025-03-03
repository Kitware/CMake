/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include <string>
#include <vector>

#include "cmBinUtilsWindowsPEGetRuntimeDependenciesTool.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool
  : public cmBinUtilsWindowsPEGetRuntimeDependenciesTool
{
public:
  cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);

  bool GetFileInfo(std::string const& file,
                   std::vector<std::string>& needed) override;
};
