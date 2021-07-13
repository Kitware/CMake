/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "cmBinUtilsLinker.h"
#include "cmBinUtilsLinuxELFGetRuntimeDependenciesTool.h"
#include "cmLDConfigTool.h"
#include "cmStateTypes.h"

class cmRuntimeDependencyArchive;

class cmBinUtilsLinuxELFLinker : public cmBinUtilsLinker
{
public:
  cmBinUtilsLinuxELFLinker(cmRuntimeDependencyArchive* archive);

  bool Prepare() override;

  bool ScanDependencies(std::string const& file,
                        cmStateEnums::TargetType type) override;

private:
  std::unique_ptr<cmBinUtilsLinuxELFGetRuntimeDependenciesTool> Tool;
  std::unique_ptr<cmLDConfigTool> LDConfigTool;
  bool HaveLDConfigPaths = false;
  std::vector<std::string> LDConfigPaths;
  std::uint16_t Machine = 0;

  bool ScanDependencies(std::string const& file,
                        std::vector<std::string> const& parentRpaths);

  bool ResolveDependency(std::string const& name,
                         std::vector<std::string> const& searchPaths,
                         std::string& path, bool& resolved);

  bool GetLDConfigPaths();
};
