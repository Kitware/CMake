/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmBinUtilsMacOSMachOLinker_h
#define cmBinUtilsMacOSMachOLinker_h

#include <memory>
#include <string>
#include <vector>

#include "cmBinUtilsLinker.h"
#include "cmBinUtilsMacOSMachOGetRuntimeDependenciesTool.h"
#include "cmStateTypes.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsMacOSMachOLinker : public cmBinUtilsLinker
{
public:
  cmBinUtilsMacOSMachOLinker(cmRuntimeDependencyArchive* archive);

  bool Prepare() override;

  bool ScanDependencies(std::string const& file,
                        cmStateEnums::TargetType type) override;

private:
  std::unique_ptr<cmBinUtilsMacOSMachOGetRuntimeDependenciesTool> Tool;

  bool ScanDependencies(std::string const& file,
                        std::string const& executablePath);

  bool GetFileDependencies(std::vector<std::string> const& names,
                           std::string const& executablePath,
                           std::string const& loaderPath,
                           std::vector<std::string> const& rpaths);

  bool ResolveDependency(std::string const& name,
                         std::string const& executablePath,
                         std::string const& loaderPath,
                         std::vector<std::string> const& rpaths,
                         std::string& path, bool& resolved);

  bool ResolveExecutablePathDependency(std::string const& name,
                                       std::string const& executablePath,
                                       std::string& path, bool& resolved);

  bool ResolveLoaderPathDependency(std::string const& name,
                                   std::string const& loaderPath,
                                   std::string& path, bool& resolved);

  bool ResolveRPathDependency(std::string const& name,
                              std::string const& executablePath,
                              std::string const& loaderPath,
                              std::vector<std::string> const& rpaths,
                              std::string& path, bool& resolved);
};

#endif // cmBinUtilsMacOSMachOLinker_h
