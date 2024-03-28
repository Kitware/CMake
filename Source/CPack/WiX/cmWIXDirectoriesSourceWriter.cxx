/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWIXDirectoriesSourceWriter.h"

#include <cmext/string_view>

cmWIXDirectoriesSourceWriter::cmWIXDirectoriesSourceWriter(
  unsigned long wixVersion, cmCPackLog* logger, std::string const& filename,
  GuidType componentGuidType)
  : cmWIXSourceWriter(wixVersion, logger, filename, componentGuidType)
{
}

void cmWIXDirectoriesSourceWriter::EmitStartMenuFolder(
  std::string const& startMenuFolder)
{
  BeginElement_StandardDirectory();
  AddAttribute("Id", "ProgramMenuFolder");

  if (startMenuFolder != "."_s) {
    BeginElement("Directory");
    AddAttribute("Id", "PROGRAM_MENU_FOLDER");
    AddAttribute("Name", startMenuFolder);
    EndElement("Directory");
  }

  EndElement_StandardDirectory();
}

void cmWIXDirectoriesSourceWriter::EmitDesktopFolder()
{
  BeginElement_StandardDirectory();
  AddAttribute("Id", "DesktopFolder");
  if (this->WixVersion == 3) {
    AddAttribute("Name", "Desktop");
  }
  EndElement_StandardDirectory();
}

void cmWIXDirectoriesSourceWriter::EmitStartupFolder()
{
  BeginElement_StandardDirectory();
  AddAttribute("Id", "StartupFolder");
  if (this->WixVersion == 3) {
    AddAttribute("Name", "Startup");
  }
  EndElement_StandardDirectory();
}

cmWIXDirectoriesSourceWriter::InstallationPrefixDirectory
cmWIXDirectoriesSourceWriter::BeginInstallationPrefixDirectory(
  std::string const& programFilesFolderId,
  std::string const& installRootString)
{
  InstallationPrefixDirectory installationPrefixDirectory;
  if (!programFilesFolderId.empty()) {
    installationPrefixDirectory.HasStandardDirectory = true;
    this->BeginElement_StandardDirectory();
    AddAttribute("Id", programFilesFolderId);
  }

  std::vector<std::string> installRoot;

  cmSystemTools::SplitPath(installRootString, installRoot);

  if (!installRoot.empty() && installRoot.back().empty()) {
    installRoot.pop_back();
  }

  for (size_t i = 1; i < installRoot.size(); ++i) {
    ++installationPrefixDirectory.Depth;
    BeginElement("Directory");

    if (i == installRoot.size() - 1) {
      AddAttribute("Id", "INSTALL_ROOT");
    } else {
      std::ostringstream tmp;
      tmp << "INSTALL_PREFIX_" << i;
      AddAttribute("Id", tmp.str());
    }

    AddAttribute("Name", installRoot[i]);
  }

  return installationPrefixDirectory;
}

void cmWIXDirectoriesSourceWriter::EndInstallationPrefixDirectory(
  InstallationPrefixDirectory installationPrefixDirectory)
{
  for (size_t i = 0; i < installationPrefixDirectory.Depth; ++i) {
    EndElement("Directory");
  }
  if (installationPrefixDirectory.HasStandardDirectory) {
    this->EndElement_StandardDirectory();
  }
}
