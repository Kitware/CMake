/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool_h
#define cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool_h

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

  bool GetFileInfo(const std::string& file,
                   std::vector<std::string>& needed) override;
};

#endif // cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool_h
