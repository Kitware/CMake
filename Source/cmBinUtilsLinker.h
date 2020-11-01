/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include <string>

#include "cmStateTypes.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsLinker
{
public:
  cmBinUtilsLinker(cmRuntimeDependencyArchive* archive);
  virtual ~cmBinUtilsLinker() = default;

  virtual bool Prepare() { return true; }

  virtual bool ScanDependencies(std::string const& file,
                                cmStateEnums::TargetType type) = 0;

protected:
  cmRuntimeDependencyArchive* Archive;

  void SetError(const std::string& e);
};
