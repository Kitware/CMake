/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmCPackGenerator.h"
#include "cmWIXSourceWriter.h"

/** \class cmWIXDirectoriesSourceWriter
 * \brief Helper class to generate directories.wxs
 */
class cmWIXDirectoriesSourceWriter : public cmWIXSourceWriter
{
public:
  cmWIXDirectoriesSourceWriter(unsigned long wixVersion, cmCPackLog* logger,
                               std::string const& filename,
                               GuidType componentGuidType);

  void EmitStartMenuFolder(std::string const& startMenuFolder);

  void EmitDesktopFolder();

  void EmitStartupFolder();

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
};
