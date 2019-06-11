/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsWindowsPEGetRuntimeDependenciesTool_h
#define cmBinUtilsWindowsPEGetRuntimeDependenciesTool_h

#include <string>
#include <vector>

class cmRuntimeDependencyArchive;

class cmBinUtilsWindowsPEGetRuntimeDependenciesTool
{
public:
  cmBinUtilsWindowsPEGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);
  virtual ~cmBinUtilsWindowsPEGetRuntimeDependenciesTool() = default;

  virtual bool GetFileInfo(const std::string& file,
                           std::vector<std::string>& needed) = 0;

protected:
  cmRuntimeDependencyArchive* Archive;

  void SetError(const std::string& error);
};

#endif // cmBinUtilsWindowsPEGetRuntimeDependenciesTool_h
