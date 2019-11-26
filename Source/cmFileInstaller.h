/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileInstaller_h
#define cmFileInstaller_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmFileCopier.h"
#include "cmInstallType.h"

class cmExecutionStatus;

struct cmFileInstaller : public cmFileCopier
{
  cmFileInstaller(cmExecutionStatus& status);
  ~cmFileInstaller() override;

protected:
  cmInstallType InstallType;
  bool Optional;
  bool MessageAlways;
  bool MessageLazy;
  bool MessageNever;
  int DestDirLength;
  std::string Rename;

  std::string Manifest;
  void ManifestAppend(std::string const& file);

  std::string const& ToName(std::string const& fromName) override;

  void ReportCopy(const std::string& toFile, Type type, bool copy) override;
  bool ReportMissing(const std::string& fromFile) override;
  bool Install(const std::string& fromFile,
               const std::string& toFile) override;

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

#endif
