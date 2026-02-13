/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

#include "cmCPackGenerator.h"
#include "cmWIXInstallScope.h"
#include "cmWIXSourceWriter.h"

/** \class cmWIXDirectoriesSourceWriter
 * \brief Helper class to generate directories.wxs
 */
class cmWIXDirectoriesSourceWriter : public cmWIXSourceWriter
{
public:
  cmWIXDirectoriesSourceWriter(unsigned long wixVersion, cmCPackLog* logger,
                               std::string const& filename,
                               GuidType componentGuidType,
                               cmWIXInstallScope installScope,
                               std::string componentKeysRegistryPath);

  void EmitStartMenuFolder(std::string const& startMenuFolder);

  void EmitDesktopFolder();

  void EmitStartupFolder();

  cm::optional<std::string> EmitRemoveFolderComponentOnUserInstall(
    std::string const& directoryId);

  struct InstallationPrefixDirectory
  {
    bool HasStandardDirectory = false;
    size_t Depth = 0;
  };

  InstallationPrefixDirectory BeginInstallationPrefixDirectory(
    std::string const& programFilesFolderId,
    std::string const& installRootString);

  void EndInstallationPrefixDirectory(
    InstallationPrefixDirectory installationPrefixDirectory);

private:
  bool PerUserInstall = false;
  std::string ComponentKeysRegistryPath;
};
