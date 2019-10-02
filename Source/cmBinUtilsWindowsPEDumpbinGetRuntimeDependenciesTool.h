/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool_h
#define cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool_h

#include <string>
#include <vector>

#include "cmBinUtilsWindowsPEGetRuntimeDependenciesTool.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool
  : public cmBinUtilsWindowsPEGetRuntimeDependenciesTool
{
public:
  cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);

  bool GetFileInfo(const std::string& file,
                   std::vector<std::string>& needed) override;
};

#endif // cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool_h
