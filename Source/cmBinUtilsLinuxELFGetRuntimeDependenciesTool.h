/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsLinuxELFGetRuntimeDependenciesTool_h
#define cmBinUtilsLinuxELFGetRuntimeDependenciesTool_h

#include <string>
#include <vector>

class cmRuntimeDependencyArchive;

class cmBinUtilsLinuxELFGetRuntimeDependenciesTool
{
public:
  cmBinUtilsLinuxELFGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive);
  virtual ~cmBinUtilsLinuxELFGetRuntimeDependenciesTool() = default;

  virtual bool GetFileInfo(std::string const& file,
                           std::vector<std::string>& needed,
                           std::vector<std::string>& rpaths,
                           std::vector<std::string>& runpaths) = 0;

protected:
  cmRuntimeDependencyArchive* Archive;

  void SetError(const std::string& e);
};

#endif // cmBinUtilsLinuxELFGetRuntimeDependenciesTool_h
