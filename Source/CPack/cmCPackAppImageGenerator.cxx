/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmCPackAppImageGenerator.h"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include <fcntl.h>

#include <sys/types.h>

#include "cmsys/FStream.hxx"
#include "cmsys/String.h"

#include "cmCPackLog.h"
#include "cmELF.h"
#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmCPackAppImageGenerator::cmCPackAppImageGenerator() = default;

cmCPackAppImageGenerator::~cmCPackAppImageGenerator() = default;

int cmCPackAppImageGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_APPIMAGE_TOOL_EXECUTABLE", "appimagetool");
  this->AppimagetoolPath = cmSystemTools::FindProgram(
    *this->GetOption("CPACK_APPIMAGE_TOOL_EXECUTABLE"));

  if (this->AppimagetoolPath.empty()) {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Cannot find AppImageTool: '"
        << *this->GetOption("CPACK_APPIMAGE_TOOL_EXECUTABLE")
        << "' check if it's installed, is executable, or is in your PATH"
        << std::endl);

    return 0;
  }

  this->SetOptionIfNotSet("CPACK_APPIMAGE_PATCHELF_EXECUTABLE", "patchelf");
  this->PatchElfPath = cmSystemTools::FindProgram(
    *this->GetOption("CPACK_APPIMAGE_PATCHELF_EXECUTABLE"));

  if (this->PatchElfPath.empty()) {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Cannot find patchelf: '"
        << *this->GetOption("CPACK_APPIMAGE_PATCHELF_EXECUTABLE")
        << "' check if it's installed, is executable, or is in your PATH"
        << std::endl);

    return 0;
  }

  return Superclass::InitializeInternal();
}

int cmCPackAppImageGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                "AppDir: \"" << this->toplevel << "\"" << std::endl);

  // Desktop file must be in the toplevel dir
  auto const desktopFile = FindDesktopFile();
  if (!desktopFile) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "A desktop file is required to build an AppImage, make sure "
                  "it's listed for install()."
                    << std::endl);
    return 0;
  }

  {
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "Found Desktop file: \"" << desktopFile.value() << "\""
                                           << std::endl);
    std::string desktopSymLink = this->toplevel + "/" +
      cmSystemTools::GetFilenameName(desktopFile.value());
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "Desktop file destination: \"" << desktopSymLink << "\""
                                                 << std::endl);
    auto status = cmSystemTools::CreateSymlink(
      cmSystemTools::RelativePath(toplevel, *desktopFile), desktopSymLink);
    if (status.IsSuccess()) {
      cmCPackLogger(cmCPackLog::LOG_DEBUG,
                    "Desktop symbolic link created successfully."
                      << std::endl);
    } else {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error creating symbolic link." << status.GetString()
                                                    << std::endl);
      return 0;
    }
  }

  auto const desktopEntry = ParseDesktopFile(*desktopFile);

  {
    // Prepare Icon file
    auto const iconValue = desktopEntry.find("Icon");
    if (iconValue == desktopEntry.end()) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "An Icon key is required to build an AppImage, make sure "
                    "the desktop file has a reference to one."
                      << std::endl);
      return 0;
    }

    auto icon = this->GetOption("CPACK_PACKAGE_ICON");
    if (!icon) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_PACKAGE_ICON is required to build an AppImage."
                      << std::endl);
      return 0;
    }

    if (!cmSystemTools::StringStartsWith(*icon, iconValue->second.c_str())) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_PACKAGE_ICON must match the file name referenced "
                    "in the desktop file."
                      << std::endl);
      return 0;
    }

    auto const iconFile = FindFile(icon);
    if (!iconFile) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Could not find the Icon referenced in the desktop file: "
                      << *icon << std::endl);
      return 0;
    }

    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "Icon file: \"" << *iconFile << "\"" << std::endl);
    std::string iconSymLink =
      this->toplevel + "/" + cmSystemTools::GetFilenameName(*iconFile);
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "Icon link destination: \"" << iconSymLink << "\""
                                              << std::endl);
    auto status = cmSystemTools::CreateSymlink(
      cmSystemTools::RelativePath(toplevel, *iconFile), iconSymLink);
    if (status.IsSuccess()) {
      cmCPackLogger(cmCPackLog::LOG_DEBUG,
                    "Icon symbolic link created successfully." << std::endl);
    } else {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error creating symbolic link." << status.GetString()
                                                    << std::endl);
      return 0;
    }
  }

  std::string application;
  {
    // Prepare executable file
    auto const execValue = desktopEntry.find("Exec");
    if (execValue == desktopEntry.end() || execValue->second.empty()) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "An Exec key is required to build an AppImage, make sure "
                    "the desktop file has a reference to one."
                      << std::endl);
      return 0;
    }

    auto const execName =
      cmSystemTools::SplitString(execValue->second, ' ').front();
    auto const mainExecutable = FindFile(execName);

    if (!mainExecutable) {
      cmCPackLogger(
        cmCPackLog::LOG_ERROR,
        "Could not find the Executable referenced in the desktop file: "
          << execName << std::endl);
      return 0;
    }
    application = cmSystemTools::RelativePath(toplevel, *mainExecutable);
  }

  std::string const appRunFile = this->toplevel + "/AppRun";
  if (cmSystemTools::PathExists(appRunFile)) {
    // User provided an AppRun file
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  cmStrCat("Found AppRun file: \"", appRunFile, '"')
                    << std::endl);
  } else {
    // Generate a default AppRun script that will run our application
    cmCPackLogger(
      cmCPackLog::LOG_OUTPUT,
      cmStrCat("No AppRun found, generating a default one that will run: \"",
               application, '"')
        << std::endl);
    cmGeneratedFileStream appRun(appRunFile);
    appRun << R"sh(#! /usr/bin/env bash

  # autogenerated by CPack

  # make sure errors in sourced scripts will cause this script to stop
  set -e

  this_dir="$(readlink -f "$(dirname "$0")")"
  )sh" << std::endl;
    appRun << R"sh(exec "$this_dir"/)sh" << application << R"sh( "$@")sh"
           << std::endl;
  }

  mode_t permissions;
  {
    auto status = cmSystemTools::GetPermissions(appRunFile, permissions);
    if (!status.IsSuccess()) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error getting AppRun permission: " << status.GetString()
                                                        << std::endl);
      return 0;
    }
  }

  auto status =
    cmSystemTools::SetPermissions(appRunFile, permissions | S_IXUSR);
  if (!status.IsSuccess()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error changing AppRun permission: " << status.GetString()
                                                       << std::endl);
    return 0;
  }

  // Set RPATH to "$ORIGIN/../lib"
  if (!ChangeRPath()) {
    return 0;
  }

  // Run appimagetool
  std::vector<std::string> command{
    this->AppimagetoolPath,
    this->toplevel,
  };
  command.emplace_back("../" + *this->GetOption("CPACK_PACKAGE_FILE_NAME") +
                       this->GetOutputExtension());

  auto addOptionFlag = [&command, this](std::string const& op,
                                        std::string commandFlag) {
    auto opt = this->GetOption(op);
    if (opt) {
      command.emplace_back(commandFlag);
    }
  };

  auto addOption = [&command, this](std::string const& op,
                                    std::string commandFlag) {
    auto opt = this->GetOption(op);
    if (opt) {
      command.emplace_back(commandFlag);
      command.emplace_back(*opt);
    }
  };

  auto addOptions = [&command, this](std::string const& op,
                                     std::string commandFlag) {
    auto opt = this->GetOption(op);
    if (opt) {
      auto const options = cmSystemTools::SplitString(*opt, ';');
      for (auto const& mkOpt : options) {
        command.emplace_back(commandFlag);
        command.emplace_back(mkOpt);
      }
    }
  };

  addOption("CPACK_APPIMAGE_UPDATE_INFORMATION", "--updateinformation");

  addOptionFlag("CPACK_APPIMAGE_GUESS_UPDATE_INFORMATION", "--guess");

  addOption("CPACK_APPIMAGE_COMPRESSOR", "--comp");

  addOptions("CPACK_APPIMAGE_MKSQUASHFS_OPTIONS", "--mksquashfs-opt");

  addOptionFlag("CPACK_APPIMAGE_NO_APPSTREAM", "--no-appstream");

  addOption("CPACK_APPIMAGE_EXCLUDE_FILE", "--exclude-file");

  addOption("CPACK_APPIMAGE_RUNTIME_FILE", "--runtime-file");

  addOptionFlag("CPACK_APPIMAGE_SIGN", "--sign");

  addOption("CPACK_APPIMAGE_SIGN_KEY", "--sign-key");

  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                "Running AppImageTool: "
                  << cmSystemTools::PrintSingleCommand(command) << std::endl);
  int retVal = 1;
  bool resS = cmSystemTools::RunSingleCommand(
    command, nullptr, nullptr, &retVal, this->toplevel.c_str(),
    cmSystemTools::OutputOption::OUTPUT_PASSTHROUGH);
  if (!resS || retVal) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem running appimagetool: " << this->AppimagetoolPath
                                                   << std::endl);
    return 0;
  }

  return 1;
}

cm::optional<std::string> cmCPackAppImageGenerator::FindFile(
  std::string const& filename) const
{
  for (std::string const& file : this->files) {
    if (cmSystemTools::GetFilenameName(file) == filename) {
      cmCPackLogger(cmCPackLog::LOG_DEBUG, "Found file:" << file << std::endl);
      return file;
    }
  }
  return cm::nullopt;
}

cm::optional<std::string> cmCPackAppImageGenerator::FindDesktopFile() const
{
  cmValue desktopFileOpt = GetOption("CPACK_APPIMAGE_DESKTOP_FILE");
  if (desktopFileOpt) {
    return FindFile(*desktopFileOpt);
  }

  for (std::string const& file : this->files) {
    if (cmSystemTools::StringEndsWith(file, ".desktop")) {
      cmCPackLogger(cmCPackLog::LOG_DEBUG,
                    "Found desktop file:" << file << std::endl);
      return file;
    }
  }

  return cm::nullopt;
}

namespace {
// Trim leading and trailing whitespace from a string
std::string trim(std::string const& str)
{
  auto start = std::find_if_not(str.begin(), str.end(), cmsysString_isspace);
  auto end =
    std::find_if_not(str.rbegin(), str.rend(), cmsysString_isspace).base();
  return (start < end) ? std::string(start, end) : std::string();
}
} // namespace

std::unordered_map<std::string, std::string>
cmCPackAppImageGenerator::ParseDesktopFile(std::string const& filePath) const
{
  std::unordered_map<std::string, std::string> ret;

  cmsys::ifstream file(filePath);
  if (!file.is_open()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Failed to open desktop file:" << filePath << std::endl);
    return ret;
  }

  bool inDesktopEntry = false;
  std::string line;
  while (std::getline(file, line)) {
    line = trim(line);

    if (line.empty() || line[0] == '#') {
      // Skip empty lines or comments
      continue;
    }

    if (line.front() == '[' && line.back() == ']') {
      // We only care for [Desktop Entry] section
      inDesktopEntry = (line == "[Desktop Entry]");
      continue;
    }

    if (inDesktopEntry) {
      size_t delimiter_pos = line.find('=');
      if (delimiter_pos == std::string::npos) {
        cmCPackLogger(cmCPackLog::LOG_WARNING,
                      "Invalid desktop file line format: " << line
                                                           << std::endl);
        continue;
      }

      std::string key = trim(line.substr(0, delimiter_pos));
      std::string value = trim(line.substr(delimiter_pos + 1));
      if (!key.empty()) {
        ret.emplace(key, value);
      }
    }
  }

  return ret;
}

bool cmCPackAppImageGenerator::ChangeRPath()
{
  // AppImages are mounted in random locations so we need RPATH to resolve to
  // that location
  std::string const newRPath = "$ORIGIN/../lib";

  for (std::string const& file : this->files) {
    cmELF elf(file.c_str());

    auto const type = elf.GetFileType();
    switch (type) {
      case cmELF::FileType::FileTypeExecutable:
      case cmELF::FileType::FileTypeSharedLibrary: {
        std::string oldRPath;
        auto const* rpath = elf.GetRPath();
        if (rpath) {
          oldRPath = rpath->Value;
        } else {
          auto const* runpath = elf.GetRunPath();
          if (runpath) {
            oldRPath = runpath->Value;
          } else {
            oldRPath = "";
          }
        }

        if (cmSystemTools::StringStartsWith(oldRPath, "$ORIGIN")) {
          // Skip libraries with ORIGIN RPATH set
          continue;
        }

        if (!PatchElfSetRPath(file, newRPath)) {
          return false;
        }

        break;
      }
      default:
        cmCPackLogger(cmCPackLog::LOG_DEBUG,
                      "ELF <" << file << "> type: " << type << std::endl);
        break;
    }
  }

  return true;
}

bool cmCPackAppImageGenerator::PatchElfSetRPath(std::string const& file,
                                                std::string const& rpath) const
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Changing RPATH: " << file << " to: " << rpath << std::endl);
  int retVal = 1;
  bool resS = cmSystemTools::RunSingleCommand(
    {
      this->PatchElfPath,
      "--set-rpath",
      rpath,
      file,
    },
    nullptr, nullptr, &retVal, nullptr,
    cmSystemTools::OutputOption::OUTPUT_NONE);
  if (!resS || retVal) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Problem running patchelf to change RPATH: " << file
                                                               << std::endl);
    return false;
  }

  return true;
}
