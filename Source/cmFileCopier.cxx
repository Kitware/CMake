/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmFileCopier.h"

#include "cmsys/Directory.hxx"
#include "cmsys/Glob.hxx"

#include "cmExecutionStatus.h"
#include "cmFSPermissions.h"
#include "cmFileTimes.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

#ifdef _WIN32
#  include "cmsys/FStream.hxx"
#endif

#include <cstring>
#include <sstream>

using namespace cmFSPermissions;

cmFileCopier::cmFileCopier(cmExecutionStatus& status, const char* name)
  : Status(status)
  , Makefile(&status.GetMakefile())
  , Name(name)
{
}

cmFileCopier::~cmFileCopier() = default;

cmFileCopier::MatchProperties cmFileCopier::CollectMatchProperties(
  const std::string& file)
{
  // Match rules are case-insensitive on some platforms.
#if defined(_WIN32) || defined(__APPLE__) || defined(__CYGWIN__)
  const std::string file_to_match = cmSystemTools::LowerCase(file);
#else
  const std::string& file_to_match = file;
#endif

  // Collect properties from all matching rules.
  bool matched = false;
  MatchProperties result;
  for (MatchRule& mr : this->MatchRules) {
    if (mr.Regex.find(file_to_match)) {
      matched = true;
      result.Exclude |= mr.Properties.Exclude;
      result.Permissions |= mr.Properties.Permissions;
    }
  }
  if (!matched && !this->MatchlessFiles) {
    result.Exclude = !cmSystemTools::FileIsDirectory(file);
  }
  return result;
}

bool cmFileCopier::SetPermissions(const std::string& toFile,
                                  mode_t permissions)
{
  if (permissions) {
#ifdef _WIN32
    if (Makefile->IsOn("CMAKE_CROSSCOMPILING")) {
      // Store the mode in an NTFS alternate stream.
      std::string mode_t_adt_filename = toFile + ":cmake_mode_t";

      // Writing to an NTFS alternate stream changes the modification
      // time, so we need to save and restore its original value.
      cmFileTimes file_time_orig(toFile);
      {
        cmsys::ofstream permissionStream(mode_t_adt_filename.c_str());
        if (permissionStream) {
          permissionStream << std::oct << permissions << std::endl;
        }
        permissionStream.close();
      }
      file_time_orig.Store(toFile);
    }
#endif

    if (!cmSystemTools::SetPermissions(toFile, permissions)) {
      std::ostringstream e;
      e << this->Name << " cannot set permissions on \"" << toFile
        << "\": " << cmSystemTools::GetLastSystemError() << ".";
      this->Status.SetError(e.str());
      return false;
    }
  }
  return true;
}

// Translate an argument to a permissions bit.
bool cmFileCopier::CheckPermissions(std::string const& arg,
                                    mode_t& permissions)
{
  if (!cmFSPermissions::stringToModeT(arg, permissions)) {
    std::ostringstream e;
    e << this->Name << " given invalid permission \"" << arg << "\".";
    this->Status.SetError(e.str());
    return false;
  }
  return true;
}

std::string const& cmFileCopier::ToName(std::string const& fromName)
{
  return fromName;
}

bool cmFileCopier::ReportMissing(const std::string& fromFile)
{
  // The input file does not exist and installation is not optional.
  std::ostringstream e;
  e << this->Name << " cannot find \"" << fromFile
    << "\": " << cmSystemTools::GetLastSystemError() << ".";
  this->Status.SetError(e.str());
  return false;
}

void cmFileCopier::NotBeforeMatch(std::string const& arg)
{
  std::ostringstream e;
  e << "option " << arg << " may not appear before PATTERN or REGEX.";
  this->Status.SetError(e.str());
  this->Doing = DoingError;
}

void cmFileCopier::NotAfterMatch(std::string const& arg)
{
  std::ostringstream e;
  e << "option " << arg << " may not appear after PATTERN or REGEX.";
  this->Status.SetError(e.str());
  this->Doing = DoingError;
}

void cmFileCopier::DefaultFilePermissions()
{
  // Use read/write permissions.
  this->FilePermissions = 0;
  this->FilePermissions |= mode_owner_read;
  this->FilePermissions |= mode_owner_write;
  this->FilePermissions |= mode_group_read;
  this->FilePermissions |= mode_world_read;
}

void cmFileCopier::DefaultDirectoryPermissions()
{
  // Use read/write/executable permissions.
  this->DirPermissions = 0;
  this->DirPermissions |= mode_owner_read;
  this->DirPermissions |= mode_owner_write;
  this->DirPermissions |= mode_owner_execute;
  this->DirPermissions |= mode_group_read;
  this->DirPermissions |= mode_group_execute;
  this->DirPermissions |= mode_world_read;
  this->DirPermissions |= mode_world_execute;
}

bool cmFileCopier::GetDefaultDirectoryPermissions(mode_t** mode)
{
  // check if default dir creation permissions were set
  cmValue default_dir_install_permissions = this->Makefile->GetDefinition(
    "CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS");
  if (cmNonempty(default_dir_install_permissions)) {
    std::vector<std::string> items =
      cmExpandedList(*default_dir_install_permissions);
    for (const auto& arg : items) {
      if (!this->CheckPermissions(arg, **mode)) {
        this->Status.SetError(
          " Set with CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS variable.");
        return false;
      }
    }
  } else {
    *mode = nullptr;
  }

  return true;
}

bool cmFileCopier::Parse(std::vector<std::string> const& args)
{
  this->Doing = DoingFiles;
  for (unsigned int i = 1; i < args.size(); ++i) {
    // Check this argument.
    if (!this->CheckKeyword(args[i]) && !this->CheckValue(args[i])) {
      std::ostringstream e;
      e << "called with unknown argument \"" << args[i] << "\".";
      this->Status.SetError(e.str());
      return false;
    }

    // Quit if an argument is invalid.
    if (this->Doing == DoingError) {
      return false;
    }
  }

  // Require a destination.
  if (this->Destination.empty()) {
    std::ostringstream e;
    e << this->Name << " given no DESTINATION";
    this->Status.SetError(e.str());
    return false;
  }

  // If file permissions were not specified set default permissions.
  if (!this->UseGivenPermissionsFile && !this->UseSourcePermissions) {
    this->DefaultFilePermissions();
  }

  // If directory permissions were not specified set default permissions.
  if (!this->UseGivenPermissionsDir && !this->UseSourcePermissions) {
    this->DefaultDirectoryPermissions();
  }

  return true;
}

bool cmFileCopier::CheckKeyword(std::string const& arg)
{
  if (arg == "DESTINATION") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingDestination;
    }
  } else if (arg == "FILES_FROM_DIR") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingFilesFromDir;
    }
  } else if (arg == "PATTERN") {
    this->Doing = DoingPattern;
  } else if (arg == "REGEX") {
    this->Doing = DoingRegex;
  } else if (arg == "FOLLOW_SYMLINK_CHAIN") {
    this->FollowSymlinkChain = true;
    this->Doing = DoingNone;
  } else if (arg == "EXCLUDE") {
    // Add this property to the current match rule.
    if (this->CurrentMatchRule) {
      this->CurrentMatchRule->Properties.Exclude = true;
      this->Doing = DoingNone;
    } else {
      this->NotBeforeMatch(arg);
    }
  } else if (arg == "PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->Doing = DoingPermissionsMatch;
    } else {
      this->NotBeforeMatch(arg);
    }
  } else if (arg == "FILE_PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingPermissionsFile;
      this->UseGivenPermissionsFile = true;
    }
  } else if (arg == "DIRECTORY_PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingPermissionsDir;
      this->UseGivenPermissionsDir = true;
    }
  } else if (arg == "USE_SOURCE_PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->UseSourcePermissions = true;
    }
  } else if (arg == "NO_SOURCE_PERMISSIONS") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->UseSourcePermissions = false;
    }
  } else if (arg == "FILES_MATCHING") {
    if (this->CurrentMatchRule) {
      this->NotAfterMatch(arg);
    } else {
      this->Doing = DoingNone;
      this->MatchlessFiles = false;
    }
  } else {
    return false;
  }
  return true;
}

bool cmFileCopier::CheckValue(std::string const& arg)
{
  switch (this->Doing) {
    case DoingFiles:
      this->Files.push_back(arg);
      break;
    case DoingDestination:
      if (arg.empty() || cmSystemTools::FileIsFullPath(arg)) {
        this->Destination = arg;
      } else {
        this->Destination =
          cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/', arg);
      }
      this->Doing = DoingNone;
      break;
    case DoingFilesFromDir:
      if (cmSystemTools::FileIsFullPath(arg)) {
        this->FilesFromDir = arg;
      } else {
        this->FilesFromDir =
          cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', arg);
      }
      cmSystemTools::ConvertToUnixSlashes(this->FilesFromDir);
      this->Doing = DoingNone;
      break;
    case DoingPattern: {
      // Convert the pattern to a regular expression.  Require a
      // leading slash and trailing end-of-string in the matched
      // string to make sure the pattern matches only whole file
      // names.
      std::string regex =
        cmStrCat('/', cmsys::Glob::PatternToRegex(arg, false), '$');
      this->MatchRules.emplace_back(regex);
      this->CurrentMatchRule = &*(this->MatchRules.end() - 1);
      if (this->CurrentMatchRule->Regex.is_valid()) {
        this->Doing = DoingNone;
      } else {
        std::ostringstream e;
        e << "could not compile PATTERN \"" << arg << "\".";
        this->Status.SetError(e.str());
        this->Doing = DoingError;
      }
    } break;
    case DoingRegex:
      this->MatchRules.emplace_back(arg);
      this->CurrentMatchRule = &*(this->MatchRules.end() - 1);
      if (this->CurrentMatchRule->Regex.is_valid()) {
        this->Doing = DoingNone;
      } else {
        std::ostringstream e;
        e << "could not compile REGEX \"" << arg << "\".";
        this->Status.SetError(e.str());
        this->Doing = DoingError;
      }
      break;
    case DoingPermissionsFile:
      if (!this->CheckPermissions(arg, this->FilePermissions)) {
        this->Doing = DoingError;
      }
      break;
    case DoingPermissionsDir:
      if (!this->CheckPermissions(arg, this->DirPermissions)) {
        this->Doing = DoingError;
      }
      break;
    case DoingPermissionsMatch:
      if (!this->CheckPermissions(
            arg, this->CurrentMatchRule->Properties.Permissions)) {
        this->Doing = DoingError;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool cmFileCopier::Run(std::vector<std::string> const& args)
{
  if (!this->Parse(args)) {
    return false;
  }

  for (std::string const& f : this->Files) {
    std::string file;
    if (!f.empty() && !cmSystemTools::FileIsFullPath(f)) {
      if (!this->FilesFromDir.empty()) {
        file = this->FilesFromDir;
      } else {
        file = this->Makefile->GetCurrentSourceDirectory();
      }
      file += "/";
      file += f;
    } else if (!this->FilesFromDir.empty()) {
      this->Status.SetError("option FILES_FROM_DIR requires all files "
                            "to be specified as relative paths.");
      return false;
    } else {
      file = f;
    }

    // Split the input file into its directory and name components.
    std::vector<std::string> fromPathComponents;
    cmSystemTools::SplitPath(file, fromPathComponents);
    std::string fromName = *(fromPathComponents.end() - 1);
    std::string fromDir = cmSystemTools::JoinPath(
      fromPathComponents.begin(), fromPathComponents.end() - 1);

    // Compute the full path to the destination file.
    std::string toFile = this->Destination;
    if (!this->FilesFromDir.empty()) {
      std::string dir = cmSystemTools::GetFilenamePath(f);
      if (!dir.empty()) {
        toFile += "/";
        toFile += dir;
      }
    }
    std::string const& toName = this->ToName(fromName);
    if (!toName.empty()) {
      toFile += "/";
      toFile += toName;
    }

    // Construct the full path to the source file.  The file name may
    // have been changed above.
    std::string fromFile = fromDir;
    if (!fromName.empty()) {
      fromFile += "/";
      fromFile += fromName;
    }

    if (!this->Install(fromFile, toFile)) {
      return false;
    }
  }
  return true;
}

bool cmFileCopier::Install(const std::string& fromFile,
                           const std::string& toFile)
{
  if (fromFile.empty()) {
    this->Status.SetError(
      "INSTALL encountered an empty string input file name.");
    return false;
  }

  // Collect any properties matching this file name.
  MatchProperties match_properties = this->CollectMatchProperties(fromFile);

  // Skip the file if it is excluded.
  if (match_properties.Exclude) {
    return true;
  }

  if (cmSystemTools::SameFile(fromFile, toFile)) {
    return true;
  }

  std::string newFromFile = fromFile;
  std::string newToFile = toFile;

  if (this->FollowSymlinkChain &&
      !this->InstallSymlinkChain(newFromFile, newToFile)) {
    return false;
  }

  if (cmSystemTools::FileIsSymlink(newFromFile)) {
    return this->InstallSymlink(newFromFile, newToFile);
  }
  if (cmSystemTools::FileIsDirectory(newFromFile)) {
    return this->InstallDirectory(newFromFile, newToFile, match_properties);
  }
  if (cmSystemTools::FileExists(newFromFile)) {
    return this->InstallFile(newFromFile, newToFile, match_properties);
  }
  return this->ReportMissing(newFromFile);
}

bool cmFileCopier::InstallSymlinkChain(std::string& fromFile,
                                       std::string& toFile)
{
  std::string newFromFile;
  std::string toFilePath = cmSystemTools::GetFilenamePath(toFile);
  while (cmSystemTools::ReadSymlink(fromFile, newFromFile)) {
    if (!cmSystemTools::FileIsFullPath(newFromFile)) {
      std::string fromFilePath = cmSystemTools::GetFilenamePath(fromFile);
      newFromFile = cmStrCat(fromFilePath, "/", newFromFile);
    }

    std::string symlinkTarget = cmSystemTools::GetFilenameName(newFromFile);

    bool copy = true;
    if (!this->Always) {
      std::string oldSymlinkTarget;
      if (cmSystemTools::ReadSymlink(toFile, oldSymlinkTarget)) {
        if (symlinkTarget == oldSymlinkTarget) {
          copy = false;
        }
      }
    }

    this->ReportCopy(toFile, TypeLink, copy);

    if (copy) {
      cmSystemTools::RemoveFile(toFile);
      cmSystemTools::MakeDirectory(toFilePath);

      if (!cmSystemTools::CreateSymlink(symlinkTarget, toFile)) {
        std::ostringstream e;
        e << this->Name << " cannot create symlink \"" << toFile
          << "\": " << cmSystemTools::GetLastSystemError() << ".";
        this->Status.SetError(e.str());
        return false;
      }
    }

    fromFile = newFromFile;
    toFile = cmStrCat(toFilePath, "/", symlinkTarget);
  }

  return true;
}

bool cmFileCopier::InstallSymlink(const std::string& fromFile,
                                  const std::string& toFile)
{
  // Read the original symlink.
  std::string symlinkTarget;
  if (!cmSystemTools::ReadSymlink(fromFile, symlinkTarget)) {
    std::ostringstream e;
    e << this->Name << " cannot read symlink \"" << fromFile
      << "\" to duplicate at \"" << toFile
      << "\": " << cmSystemTools::GetLastSystemError() << ".";
    this->Status.SetError(e.str());
    return false;
  }

  // Compare the symlink value to that at the destination if not
  // always installing.
  bool copy = true;
  if (!this->Always) {
    std::string oldSymlinkTarget;
    if (cmSystemTools::ReadSymlink(toFile, oldSymlinkTarget)) {
      if (symlinkTarget == oldSymlinkTarget) {
        copy = false;
      }
    }
  }

  // Inform the user about this file installation.
  this->ReportCopy(toFile, TypeLink, copy);

  if (copy) {
    // Remove the destination file so we can always create the symlink.
    cmSystemTools::RemoveFile(toFile);

    // Create destination directory if it doesn't exist
    cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(toFile));

    // Create the symlink.
    if (!cmSystemTools::CreateSymlink(symlinkTarget, toFile)) {
      std::ostringstream e;
      e << this->Name << " cannot duplicate symlink \"" << fromFile
        << "\" at \"" << toFile
        << "\": " << cmSystemTools::GetLastSystemError() << ".";
      this->Status.SetError(e.str());
      return false;
    }
  }

  return true;
}

bool cmFileCopier::InstallFile(const std::string& fromFile,
                               const std::string& toFile,
                               MatchProperties match_properties)
{
  // Determine whether we will copy the file.
  bool copy = true;
  if (!this->Always) {
    // If both files exist with the same time do not copy.
    if (!this->FileTimes.DifferS(fromFile, toFile)) {
      copy = false;
    }
  }

  // Inform the user about this file installation.
  this->ReportCopy(toFile, TypeFile, copy);

  // Copy the file.
  if (copy && !cmSystemTools::CopyAFile(fromFile, toFile, true)) {
    std::ostringstream e;
    e << this->Name << " cannot copy file \"" << fromFile << "\" to \""
      << toFile << "\": " << cmSystemTools::GetLastSystemError() << ".";
    this->Status.SetError(e.str());
    return false;
  }

  // Set the file modification time of the destination file.
  if (copy && !this->Always) {
    // Add write permission so we can set the file time.
    // Permissions are set unconditionally below anyway.
    mode_t perm = 0;
    if (cmSystemTools::GetPermissions(toFile, perm)) {
      cmSystemTools::SetPermissions(toFile, perm | mode_owner_write);
    }
    if (!cmFileTimes::Copy(fromFile, toFile)) {
      std::ostringstream e;
      e << this->Name << " cannot set modification time on \"" << toFile
        << "\": " << cmSystemTools::GetLastSystemError() << ".";
      this->Status.SetError(e.str());
      return false;
    }
  }

  // Set permissions of the destination file.
  mode_t permissions =
    (match_properties.Permissions ? match_properties.Permissions
                                  : this->FilePermissions);
  if (!permissions) {
    // No permissions were explicitly provided but the user requested
    // that the source file permissions be used.
    cmSystemTools::GetPermissions(fromFile, permissions);
  }
  return this->SetPermissions(toFile, permissions);
}

bool cmFileCopier::InstallDirectory(const std::string& source,
                                    const std::string& destination,
                                    MatchProperties match_properties)
{
  // Inform the user about this directory installation.
  this->ReportCopy(destination, TypeDir,
                   !cmSystemTools::FileIsDirectory(destination));

  // check if default dir creation permissions were set
  mode_t default_dir_mode_v = 0;
  mode_t* default_dir_mode = &default_dir_mode_v;
  if (!this->GetDefaultDirectoryPermissions(&default_dir_mode)) {
    return false;
  }

  // Make sure the destination directory exists.
  if (!cmSystemTools::MakeDirectory(destination, default_dir_mode)) {
    std::ostringstream e;
    e << this->Name << " cannot make directory \"" << destination
      << "\": " << cmSystemTools::GetLastSystemError() << ".";
    this->Status.SetError(e.str());
    return false;
  }

  // Compute the requested permissions for the destination directory.
  mode_t permissions =
    (match_properties.Permissions ? match_properties.Permissions
                                  : this->DirPermissions);
  if (!permissions) {
    // No permissions were explicitly provided but the user requested
    // that the source directory permissions be used.
    cmSystemTools::GetPermissions(source, permissions);
  }

  // Compute the set of permissions required on this directory to
  // recursively install files and subdirectories safely.
  mode_t required_permissions =
    mode_owner_read | mode_owner_write | mode_owner_execute;

  // If the required permissions are specified it is safe to set the
  // final permissions now.  Otherwise we must add the required
  // permissions temporarily during file installation.
  mode_t permissions_before = 0;
  mode_t permissions_after = 0;
  if ((permissions & required_permissions) == required_permissions) {
    permissions_before = permissions;
  } else {
    permissions_before = permissions | required_permissions;
    permissions_after = permissions;
  }

  // Set the required permissions of the destination directory.
  if (!this->SetPermissions(destination, permissions_before)) {
    return false;
  }

  // Load the directory contents to traverse it recursively.
  cmsys::Directory dir;
  if (!source.empty()) {
    dir.Load(source);
  }
  unsigned long numFiles = static_cast<unsigned long>(dir.GetNumberOfFiles());
  for (unsigned long fileNum = 0; fileNum < numFiles; ++fileNum) {
    if (!(strcmp(dir.GetFile(fileNum), ".") == 0 ||
          strcmp(dir.GetFile(fileNum), "..") == 0)) {
      std::string fromPath = cmStrCat(source, '/', dir.GetFile(fileNum));
      std::string toPath = cmStrCat(destination, '/', dir.GetFile(fileNum));
      if (!this->Install(fromPath, toPath)) {
        return false;
      }
    }
  }

  // Set the requested permissions of the destination directory.
  return this->SetPermissions(destination, permissions_after);
}
