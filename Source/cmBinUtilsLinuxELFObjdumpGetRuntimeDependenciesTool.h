/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsLinuxELFGetRuntimeCollectDependenciesTool_h
#define cmBinUtilsLinuxELFGetRuntimeCollectDependenciesTool_h

#include <string>
#include <vector>

#include "cmBinUtilsLinuxELFGetRuntimeDependenciesTool.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool
  : public cmBinUtilsLinuxELFGetRuntimeDependenciesTool
{
public:
  cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);

  bool GetFileInfo(std::string const& file, std::vector<std::string>& needed,
                   std::vector<std::string>& rpaths,
                   std::vector<std::string>& runpaths) override;
};

#endif // cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool_h
