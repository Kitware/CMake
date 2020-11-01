/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include <string>
#include <vector>

class cmRuntimeDependencyArchive;

class cmBinUtilsMacOSMachOGetRuntimeDependenciesTool
{
public:
  cmBinUtilsMacOSMachOGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);
  virtual ~cmBinUtilsMacOSMachOGetRuntimeDependenciesTool() = default;

  virtual bool GetFileInfo(std::string const& file,
                           std::vector<std::string>& libs,
                           std::vector<std::string>& rpaths) = 0;

protected:
  cmRuntimeDependencyArchive* Archive;

  void SetError(const std::string& error);
};
