/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
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
  struct FileInfo
  {
    std::vector<std::string> libs;
    std::vector<std::string> rpaths;
  };

  std::unique_ptr<cmBinUtilsMacOSMachOGetRuntimeDependenciesTool> Tool;
  std::unordered_map<std::string, FileInfo> ScannedFileInfo;

  const FileInfo* GetFileInfo(std::string const& file);

  bool ScanDependencies(std::string const& file,
                        std::vector<std::string> const& libs,
                        std::vector<std::string> const& rpaths,
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
