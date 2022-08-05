/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmFileCopier.h"
#include "cmInstallMode.h"
#include "cmInstallType.h"

class cmExecutionStatus;

struct cmFileInstaller : public cmFileCopier
{
  cmFileInstaller(cmExecutionStatus& status);
  ~cmFileInstaller() override;

protected:
  cmInstallType InstallType = cmInstallType_FILES;
  cmInstallMode InstallMode = cmInstallMode::COPY;
  bool Optional = false;
  bool MessageAlways = false;
  bool MessageLazy = false;
  bool MessageNever = false;
  int DestDirLength = 0;
  std::string Rename;

  std::string Manifest;
  void ManifestAppend(std::string const& file);

  std::string const& ToName(std::string const& fromName) override;

  void ReportCopy(const std::string& toFile, Type type, bool copy) override;
  bool ReportMissing(const std::string& fromFile) override;
  bool Install(const std::string& fromFile,
               const std::string& toFile) override;
  bool InstallFile(const std::string& fromFile, const std::string& toFile,
                   MatchProperties match_properties) override;
  bool Parse(std::vector<std::string> const& args) override;
  enum
  {
    DoingType = DoingLast1,
    DoingRename,
    DoingLast2
  };
  bool CheckKeyword(std::string const& arg) override;
  bool CheckValue(std::string const& arg) override;
  void DefaultFilePermissions() override;
  bool GetTargetTypeFromString(const std::string& stype);
  bool HandleInstallDestination();
};
