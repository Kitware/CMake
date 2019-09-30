/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmFileInstaller.h"

#include <sstream>

#include "cm_sys_stat.h"

#include "cmExecutionStatus.h"
#include "cmFSPermissions.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

using namespace cmFSPermissions;

cmFileInstaller::cmFileInstaller(cmExecutionStatus& status)
  : cmFileCopier(status, "INSTALL")
  , InstallType(cmInstallType_FILES)
  , Optional(false)
  , MessageAlways(false)
  , MessageLazy(false)
  , MessageNever(false)
  , DestDirLength(0)
{
  // Installation does not use source permissions by default.
  this->UseSourcePermissions = false;
  // Check whether to copy files always or only if they have changed.
  std::string install_always;
  if (cmSystemTools::GetEnv("CMAKE_INSTALL_ALWAYS", install_always)) {
    this->Always = cmIsOn(install_always);
  }
  // Get the current manifest.
  this->Manifest =
    this->Makefile->GetSafeDefinition("CMAKE_INSTALL_MANIFEST_FILES");
}
cmFileInstaller::~cmFileInstaller()
{
  // Save the updated install manifest.
  this->Makefile->AddDefinition("CMAKE_INSTALL_MANIFEST_FILES",
                                this->Manifest);
}

void cmFileInstaller::ManifestAppend(std::string const& file)
{
  if (!this->Manifest.empty()) {
    this->Manifest += ";";
  }
  this->Manifest += file.substr(this->DestDirLength);
}

std::string const& cmFileInstaller::ToName(std::string const& fromName)
{
  return this->Rename.empty() ? fromName : this->Rename;
}

void cmFileInstaller::ReportCopy(const std::string& toFile, Type type,
                                 bool copy)
{
  if (!this->MessageNever && (copy || !this->MessageLazy)) {
    std::string message =
      cmStrCat((copy ? "Installing: " : "Up-to-date: "), toFile);
    this->Makefile->DisplayStatus(message, -1);
  }
  if (type != TypeDir) {
    // Add the file to the manifest.
    this->ManifestAppend(toFile);
  }
}
bool cmFileInstaller::ReportMissing(const std::string& fromFile)
{
  return (this->Optional || this->cmFileCopier::ReportMissing(fromFile));
}
bool cmFileInstaller::Install(const std::string& fromFile,
                              const std::string& toFile)
{
  // Support installing from empty source to make a directory.
  if (this->InstallType == cmInstallType_DIRECTORY && fromFile.empty()) {
    return this->InstallDirectory(fromFile, toFile, MatchProperties());
  }
  return this->cmFileCopier::Install(fromFile, toFile);
}

void cmFileInstaller::DefaultFilePermissions()
{
  this->cmFileCopier::DefaultFilePermissions();
  // Add execute permissions based on the target type.
  switch (this->InstallType) {
    case cmInstallType_SHARED_LIBRARY:
    case cmInstallType_MODULE_LIBRARY:
      if (this->Makefile->IsOn("CMAKE_INSTALL_SO_NO_EXE")) {
        break;
      }
      CM_FALLTHROUGH;
    case cmInstallType_EXECUTABLE:
    case cmInstallType_PROGRAMS:
      this->FilePermissions |= mode_owner_execute;
      this->FilePermissions |= mode_group_execute;
      this->FilePermissions |= mode_world_execute;
      break;
    default:
      break;
  }
}

bool cmFileInstaller::Parse(std::vector<std::string> const& args)
{
  if (!this->cmFileCopier::Parse(args)) {
    return false;
  }

  if (!this->Rename.empty()) {
    if (!this->FilesFromDir.empty()) {
      this->Status.SetError("INSTALL option RENAME may not be "
                            "combined with FILES_FROM_DIR.");
      return false;
    }
    if (this->InstallType != cmInstallType_FILES &&
        this->InstallType != cmInstallType_PROGRAMS) {
      this->Status.SetError("INSTALL option RENAME may be used "
                            "only with FILES or PROGRAMS.");
      return false;
    }
    if (this->Files.size() > 1) {
      this->Status.SetError("INSTALL option RENAME may be used "
                            "only with one file.");
      return false;
    }
  }

  if (!this->HandleInstallDestination()) {
    return false;
  }

  if (((this->MessageAlways ? 1 : 0) + (this->MessageLazy ? 1 : 0) +
       (this->MessageNever ? 1 : 0)) > 1) {
    this->Status.SetError("INSTALL options MESSAGE_ALWAYS, "
                          "MESSAGE_LAZY, and MESSAGE_NEVER "
                          "are mutually exclusive.");
    return false;
  }

  return true;
}

bool cmFileInstaller::CheckKeyword(std::string const& arg)
{
  if (arg == "TYPE") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingType;
    }
  } else if (arg == "FILES") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingFiles;
    }
  } else if (arg == "RENAME") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingRename;
    }
  } else if (arg == "OPTIONAL") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->Optional = true;
    }
  } else if (arg == "MESSAGE_ALWAYS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->MessageAlways = true;
    }
  } else if (arg == "MESSAGE_LAZY") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->MessageLazy = true;
    }
  } else if (arg == "MESSAGE_NEVER") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->MessageNever = true;
    }
  } else if (arg == "PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->Doing = DoingPermissionsMatch;
    } else {
      // file(INSTALL) aliases PERMISSIONS to FILE_PERMISSIONS
      this->Doing = DoingPermissionsFile;
      this->UseGivenPermissionsFile = true;
    }
  } else if (arg == "DIR_PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      // file(INSTALL) aliases DIR_PERMISSIONS to DIRECTORY_PERMISSIONS
      this->Doing = DoingPermissionsDir;
      this->UseGivenPermissionsDir = true;
    }
  } else if (arg == "COMPONENTS" || arg == "CONFIGURATIONS" ||
             arg == "PROPERTIES") {
    std::ostringstream e;
    e << "INSTALL called with old-style " << arg << " argument.  "
      << "This script was generated with an older version of CMake.  "
      << "Re-run this cmake version on your build tree.";
    this->Status.SetError(e.str());
    this->Doing = DoingError;
  } else {
    return this->cmFileCopier::CheckKeyword(arg);
  }
  return true;
}

bool cmFileInstaller::CheckValue(std::string const& arg)
{
  switch (this->Doing) {
    case DoingType:
      if (!this->GetTargetTypeFromString(arg)) {
        this->Doing = DoingError;
      }
      break;
    case DoingRename:
      this->Rename = arg;
      break;
    default:
      return this->cmFileCopier::CheckValue(arg);
  }
  return true;
}

bool cmFileInstaller::GetTargetTypeFromString(const std::string& stype)
{
  if (stype == "EXECUTABLE") {
    this->InstallType = cmInstallType_EXECUTABLE;
  } else if (stype == "FILE") {
    this->InstallType = cmInstallType_FILES;
  } else if (stype == "PROGRAM") {
    this->InstallType = cmInstallType_PROGRAMS;
  } else if (stype == "STATIC_LIBRARY") {
    this->InstallType = cmInstallType_STATIC_LIBRARY;
  } else if (stype == "SHARED_LIBRARY") {
    this->InstallType = cmInstallType_SHARED_LIBRARY;
  } else if (stype == "MODULE") {
    this->InstallType = cmInstallType_MODULE_LIBRARY;
  } else if (stype == "DIRECTORY") {
    this->InstallType = cmInstallType_DIRECTORY;
  } else {
    std::ostringstream e;
    e << "Option TYPE given unknown value \"" << stype << "\".";
    this->Status.SetError(e.str());
    return false;
  }
  return true;
}

bool cmFileInstaller::HandleInstallDestination()
{
  std::string& destination = this->Destination;

  // allow for / to be a valid destination
  if (destination.size() < 2 && destination != "/") {
    this->Status.SetError("called with inappropriate arguments. "
                          "No DESTINATION provided or .");
    return false;
  }

  std::string sdestdir;
  if (cmSystemTools::GetEnv("DESTDIR", sdestdir) && !sdestdir.empty()) {
    cmSystemTools::ConvertToUnixSlashes(sdestdir);
    char ch1 = destination[0];
    char ch2 = destination[1];
    char ch3 = 0;
    if (destination.size() > 2) {
      ch3 = destination[2];
    }
    int skip = 0;
    if (ch1 != '/') {
      int relative = 0;
      if (((ch1 >= 'a' && ch1 <= 'z') || (ch1 >= 'A' && ch1 <= 'Z')) &&
          ch2 == ':') {
        // Assume windows
        // let's do some destdir magic:
        skip = 2;
        if (ch3 != '/') {
          relative = 1;
        }
      } else {
        relative = 1;
      }
      if (relative) {
        // This is relative path on unix or windows. Since we are doing
        // destdir, this case does not make sense.
        this->Status.SetError(
          "called with relative DESTINATION. This "
          "does not make sense when using DESTDIR. Specify "
          "absolute path or remove DESTDIR environment variable.");
        return false;
      }
    } else {
      if (ch2 == '/') {
        // looks like a network path.
        std::string message =
          cmStrCat("called with network path DESTINATION. This "
                   "does not make sense when using DESTDIR. Specify local "
                   "absolute path or remove DESTDIR environment variable."
                   "\nDESTINATION=\n",
                   destination);
        this->Status.SetError(message);
        return false;
      }
    }
    destination = sdestdir + destination.substr(skip);
    this->DestDirLength = int(sdestdir.size());
  }

  // check if default dir creation permissions were set
  mode_t default_dir_mode_v = 0;
  mode_t* default_dir_mode = &default_dir_mode_v;
  if (!this->GetDefaultDirectoryPermissions(&default_dir_mode)) {
    return false;
  }

  if (this->InstallType != cmInstallType_DIRECTORY) {
    if (!cmSystemTools::FileExists(destination)) {
      if (!cmSystemTools::MakeDirectory(destination, default_dir_mode)) {
        std::string errstring = "cannot create directory: " + destination +
          ". Maybe need administrative privileges.";
        this->Status.SetError(errstring);
        return false;
      }
    }
    if (!cmSystemTools::FileIsDirectory(destination)) {
      std::string errstring =
        "INSTALL destination: " + destination + " is not a directory.";
      this->Status.SetError(errstring);
      return false;
    }
  }
  return true;
}
