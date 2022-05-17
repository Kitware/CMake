/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallTargetGenerator.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallType.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
std::string computeInstallObjectDir(cmGeneratorTarget* gt,
                                    std::string const& config)
{
  std::string objectDir = "objects";
  if (!config.empty()) {
    objectDir += "-";
    objectDir += config;
  }
  objectDir += "/";
  objectDir += gt->GetName();
  return objectDir;
}
}

cmInstallTargetGenerator::cmInstallTargetGenerator(
  std::string targetName, std::string const& dest, bool implib,
  std::string file_permissions, std::vector<std::string> const& configurations,
  std::string const& component, MessageLevel message, bool exclude_from_all,
  bool optional, cmListFileBacktrace backtrace)
  : cmInstallGenerator(dest, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , TargetName(std::move(targetName))
  , FilePermissions(std::move(file_permissions))
  , ImportLibrary(implib)
  , Optional(optional)
{
  this->ActionsPerConfig = true;
  this->NamelinkMode = NamelinkModeNone;
}

cmInstallTargetGenerator::~cmInstallTargetGenerator() = default;

void cmInstallTargetGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  // Compute the list of files to install for this target.
  Files files = this->GetFiles(config);

  // Skip this rule if no files are to be installed for the target.
  if (files.From.empty()) {
    return;
  }

  // Compute the effective install destination.
  std::string dest = this->GetDestination(config);
  if (!files.ToDir.empty()) {
    dest = cmStrCat(dest, '/', files.ToDir);
  }

  // Tweak files located in the destination directory.
  std::string toDir = cmStrCat(ConvertToAbsoluteDestination(dest), '/');

  // Add pre-installation tweaks.
  if (!files.NoTweak) {
    AddTweak(os, indent, config, toDir, files.To,
             [this](std::ostream& o, Indent i, const std::string& c,
                    const std::string& f) {
               this->PreReplacementTweaks(o, i, c, f);
             });
  }

  // Write code to install the target file.
  const char* no_dir_permissions = nullptr;
  const char* no_rename = nullptr;
  bool optional = this->Optional || this->ImportLibrary;
  std::string literal_args;
  if (!files.FromDir.empty()) {
    literal_args += " FILES_FROM_DIR \"" + files.FromDir + "\"";
  }
  if (files.UseSourcePermissions) {
    literal_args += " USE_SOURCE_PERMISSIONS";
  }
  this->AddInstallRule(os, dest, files.Type, files.From, optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_rename, literal_args.c_str(), indent);

  // Add post-installation tweaks.
  if (!files.NoTweak) {
    AddTweak(os, indent, config, toDir, files.To,
             [this](std::ostream& o, Indent i, const std::string& c,
                    const std::string& f) {
               this->PostReplacementTweaks(o, i, c, f);
             });
  }
}

cmInstallTargetGenerator::Files cmInstallTargetGenerator::GetFiles(
  std::string const& config) const
{
  Files files;

  cmStateEnums::TargetType targetType = this->Target->GetType();
  switch (targetType) {
    case cmStateEnums::EXECUTABLE:
      files.Type = cmInstallType_EXECUTABLE;
      break;
    case cmStateEnums::STATIC_LIBRARY:
      files.Type = cmInstallType_STATIC_LIBRARY;
      break;
    case cmStateEnums::SHARED_LIBRARY:
      files.Type = cmInstallType_SHARED_LIBRARY;
      break;
    case cmStateEnums::MODULE_LIBRARY:
      files.Type = cmInstallType_MODULE_LIBRARY;
      break;
    case cmStateEnums::INTERFACE_LIBRARY:
      // Not reachable. We never create a cmInstallTargetGenerator for
      // an INTERFACE_LIBRARY.
      assert(false &&
             "INTERFACE_LIBRARY targets have no installable outputs.");
      break;

    case cmStateEnums::OBJECT_LIBRARY: {
      // Compute all the object files inside this target
      std::vector<std::string> objects;
      this->Target->GetTargetObjectNames(config, objects);

      files.Type = cmInstallType_FILES;
      files.NoTweak = true;
      files.FromDir = this->Target->GetObjectDirectory(config);
      files.ToDir = computeInstallObjectDir(this->Target, config);
      for (std::string& obj : objects) {
        files.From.emplace_back(obj);
        files.To.emplace_back(std::move(obj));
      }
      return files;
    }

    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::UNKNOWN_LIBRARY:
      this->Target->GetLocalGenerator()->IssueMessage(
        MessageType::INTERNAL_ERROR,
        "cmInstallTargetGenerator created with non-installable target.");
      return files;
  }

  // Compute the build tree directory from which to copy the target.
  std::string fromDirConfig;
  if (this->Target->NeedRelinkBeforeInstall(config)) {
    fromDirConfig =
      cmStrCat(this->Target->GetLocalGenerator()->GetCurrentBinaryDirectory(),
               "/CMakeFiles/CMakeRelink.dir/");
  } else {
    cmStateEnums::ArtifactType artifact = this->ImportLibrary
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;
    fromDirConfig =
      cmStrCat(this->Target->GetDirectory(config, artifact), '/');
  }

  if (targetType == cmStateEnums::EXECUTABLE) {
    // There is a bug in cmInstallCommand if this fails.
    assert(this->NamelinkMode == NamelinkModeNone);

    cmGeneratorTarget::Names targetNames =
      this->Target->GetExecutableNames(config);
    if (this->ImportLibrary) {
      std::string from1 = fromDirConfig + targetNames.ImportLibrary;
      std::string to1 = targetNames.ImportLibrary;
      files.From.emplace_back(std::move(from1));
      files.To.emplace_back(std::move(to1));
      std::string targetNameImportLib;
      if (this->Target->GetImplibGNUtoMS(config, targetNames.ImportLibrary,
                                         targetNameImportLib)) {
        files.From.emplace_back(fromDirConfig + targetNameImportLib);
        files.To.emplace_back(targetNameImportLib);
      }

      // An import library looks like a static library.
      files.Type = cmInstallType_STATIC_LIBRARY;
    } else {
      std::string from1 = fromDirConfig + targetNames.Output;
      std::string to1 = targetNames.Output;

      // Handle OSX Bundles.
      if (this->Target->IsAppBundleOnApple()) {
        cmMakefile const* mf = this->Target->Target->GetMakefile();

        // Get App Bundle Extension
        std::string ext;
        if (cmValue p = this->Target->GetProperty("BUNDLE_EXTENSION")) {
          ext = *p;
        } else {
          ext = "app";
        }

        // Install the whole app bundle directory.
        files.Type = cmInstallType_DIRECTORY;
        files.UseSourcePermissions = true;
        from1 += ".";
        from1 += ext;

        // Tweaks apply to the binary inside the bundle.
        to1 += ".";
        to1 += ext;
        to1 += "/";
        if (!mf->PlatformIsAppleEmbedded()) {
          to1 += "Contents/MacOS/";
        }
        to1 += targetNames.Output;
      } else {
        // Tweaks apply to the real file, so list it first.
        if (targetNames.Real != targetNames.Output) {
          std::string from2 = fromDirConfig + targetNames.Real;
          std::string to2 = targetNames.Real;
          files.From.emplace_back(std::move(from2));
          files.To.emplace_back(std::move(to2));
        }
      }

      files.From.emplace_back(std::move(from1));
      files.To.emplace_back(std::move(to1));
    }
  } else {
    cmGeneratorTarget::Names targetNames =
      this->Target->GetLibraryNames(config);
    if (this->ImportLibrary) {
      // There is a bug in cmInstallCommand if this fails.
      assert(this->NamelinkMode == NamelinkModeNone);

      std::string from1 = fromDirConfig + targetNames.ImportLibrary;
      std::string to1 = targetNames.ImportLibrary;
      files.From.emplace_back(std::move(from1));
      files.To.emplace_back(std::move(to1));
      std::string targetNameImportLib;
      if (this->Target->GetImplibGNUtoMS(config, targetNames.ImportLibrary,
                                         targetNameImportLib)) {
        files.From.emplace_back(fromDirConfig + targetNameImportLib);
        files.To.emplace_back(targetNameImportLib);
      }

      // An import library looks like a static library.
      files.Type = cmInstallType_STATIC_LIBRARY;
    } else if (this->Target->IsFrameworkOnApple()) {
      // FIXME: In principle we should be able to
      //   assert(this->NamelinkMode == NamelinkModeNone);
      // but since the current install() command implementation checks
      // the FRAMEWORK property immediately instead of delaying until
      // generate time, it is possible for project code to set the
      // property after calling install().  In such a case, the install()
      // command will use the LIBRARY code path and create two install
      // generators, one for the namelink component (NamelinkModeOnly)
      // and one for the primary artifact component (NamelinkModeSkip).
      // Historically this was not diagnosed and resulted in silent
      // installation of a framework to the LIBRARY destination.
      // Retain that behavior and warn about the case.
      switch (this->NamelinkMode) {
        case NamelinkModeNone:
          // Normal case.
          break;
        case NamelinkModeOnly:
          // Assume the NamelinkModeSkip instance will warn and install.
          return files;
        case NamelinkModeSkip: {
          std::string e = "Target '" + this->Target->GetName() +
            "' was changed to a FRAMEWORK sometime after install().  "
            "This may result in the wrong install DESTINATION.  "
            "Set the FRAMEWORK property earlier.";
          this->Target->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
            MessageType::AUTHOR_WARNING, e, this->GetBacktrace());
        } break;
      }

      // Install the whole framework directory.
      files.Type = cmInstallType_DIRECTORY;
      files.UseSourcePermissions = true;

      std::string from1 = fromDirConfig + targetNames.Output;
      from1 = cmSystemTools::GetFilenamePath(from1);

      // Tweaks apply to the binary inside the bundle.
      std::string to1 = targetNames.Real;

      files.From.emplace_back(std::move(from1));
      files.To.emplace_back(std::move(to1));
    } else if (this->Target->IsCFBundleOnApple()) {
      // Install the whole app bundle directory.
      files.Type = cmInstallType_DIRECTORY;
      files.UseSourcePermissions = true;

      std::string targetNameBase =
        targetNames.Output.substr(0, targetNames.Output.find('/'));

      std::string from1 = fromDirConfig + targetNameBase;
      std::string to1 = targetNames.Output;

      files.From.emplace_back(std::move(from1));
      files.To.emplace_back(std::move(to1));
    } else {
      bool haveNamelink = false;

      // Library link name.
      std::string fromName = fromDirConfig + targetNames.Output;
      std::string toName = targetNames.Output;

      // Library interface name.
      std::string fromSOName;
      std::string toSOName;
      if (targetNames.SharedObject != targetNames.Output) {
        haveNamelink = true;
        fromSOName = fromDirConfig + targetNames.SharedObject;
        toSOName = targetNames.SharedObject;
      }

      // Library implementation name.
      std::string fromRealName;
      std::string toRealName;
      if (targetNames.Real != targetNames.Output &&
          targetNames.Real != targetNames.SharedObject) {
        haveNamelink = true;
        fromRealName = fromDirConfig + targetNames.Real;
        toRealName = targetNames.Real;
      }

      // Add the names based on the current namelink mode.
      if (haveNamelink) {
        files.NamelinkMode = this->NamelinkMode;
        // With a namelink we need to check the mode.
        if (this->NamelinkMode == NamelinkModeOnly) {
          // Install the namelink only.
          files.From.emplace_back(fromName);
          files.To.emplace_back(toName);
        } else {
          // Install the real file if it has its own name.
          if (!fromRealName.empty()) {
            files.From.emplace_back(fromRealName);
            files.To.emplace_back(toRealName);
          }

          // Install the soname link if it has its own name.
          if (!fromSOName.empty()) {
            files.From.emplace_back(fromSOName);
            files.To.emplace_back(toSOName);
          }

          // Install the namelink if it is not to be skipped.
          if (this->NamelinkMode != NamelinkModeSkip) {
            files.From.emplace_back(fromName);
            files.To.emplace_back(toName);
          }
        }
      } else {
        // Without a namelink there will be only one file.  Install it
        // if this is not a namelink-only rule.
        if (this->NamelinkMode != NamelinkModeOnly) {
          files.From.emplace_back(fromName);
          files.To.emplace_back(toName);
        }
      }
    }
  }

  // If this fails the above code is buggy.
  assert(files.From.size() == files.To.size());

  return files;
}

void cmInstallTargetGenerator::GetInstallObjectNames(
  std::string const& config, std::vector<std::string>& objects) const
{
  this->Target->GetTargetObjectNames(config, objects);
  for (std::string& o : objects) {
    o = cmStrCat(computeInstallObjectDir(this->Target, config), "/", o);
  }
}

std::string cmInstallTargetGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(
    this->Destination, this->Target->GetLocalGenerator(), config);
}

std::string cmInstallTargetGenerator::GetInstallFilename(
  const std::string& config) const
{
  NameType nameType = this->ImportLibrary ? NameImplib : NameNormal;
  return cmInstallTargetGenerator::GetInstallFilename(this->Target, config,
                                                      nameType);
}

std::string cmInstallTargetGenerator::GetInstallFilename(
  cmGeneratorTarget const* target, const std::string& config,
  NameType nameType)
{
  std::string fname;
  // Compute the name of the library.
  if (target->GetType() == cmStateEnums::EXECUTABLE) {
    cmGeneratorTarget::Names targetNames = target->GetExecutableNames(config);
    if (nameType == NameImplib) {
      // Use the import library name.
      if (!target->GetImplibGNUtoMS(config, targetNames.ImportLibrary, fname,
                                    "${CMAKE_IMPORT_LIBRARY_SUFFIX}")) {
        fname = targetNames.ImportLibrary;
      }
    } else if (nameType == NameReal) {
      // Use the canonical name.
      fname = targetNames.Real;
    } else {
      // Use the canonical name.
      fname = targetNames.Output;
    }
  } else {
    cmGeneratorTarget::Names targetNames = target->GetLibraryNames(config);
    if (nameType == NameImplib) {
      // Use the import library name.
      if (!target->GetImplibGNUtoMS(config, targetNames.ImportLibrary, fname,
                                    "${CMAKE_IMPORT_LIBRARY_SUFFIX}")) {
        fname = targetNames.ImportLibrary;
      }
    } else if (nameType == NameSO) {
      // Use the soname.
      fname = targetNames.SharedObject;
    } else if (nameType == NameReal) {
      // Use the real name.
      fname = targetNames.Real;
    } else {
      // Use the canonical name.
      fname = targetNames.Output;
    }
  }

  return fname;
}

bool cmInstallTargetGenerator::Compute(cmLocalGenerator* lg)
{
  // Lookup this target in the current directory.
  this->Target = lg->FindLocalNonAliasGeneratorTarget(this->TargetName);
  if (!this->Target) {
    // If no local target has been found, find it in the global scope.
    this->Target =
      lg->GetGlobalGenerator()->FindGeneratorTarget(this->TargetName);
  }

  return true;
}

void cmInstallTargetGenerator::PreReplacementTweaks(std::ostream& os,
                                                    Indent indent,
                                                    const std::string& config,
                                                    std::string const& file)
{
  this->AddRPathCheckRule(os, indent, config, file);
}

void cmInstallTargetGenerator::PostReplacementTweaks(std::ostream& os,
                                                     Indent indent,
                                                     const std::string& config,
                                                     std::string const& file)
{
  this->AddInstallNamePatchRule(os, indent, config, file);
  this->AddChrpathPatchRule(os, indent, config, file);
  this->AddUniversalInstallRule(os, indent, file);
  this->AddRanlibRule(os, indent, file);
  this->AddStripRule(os, indent, file);
}

void cmInstallTargetGenerator::AddInstallNamePatchRule(
  std::ostream& os, Indent indent, const std::string& config,
  std::string const& toDestDirPath)
{
  if (this->ImportLibrary ||
      !(this->Target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->Target->GetType() == cmStateEnums::MODULE_LIBRARY ||
        this->Target->GetType() == cmStateEnums::EXECUTABLE)) {
    return;
  }

  // Fix the install_name settings in installed binaries.
  std::string installNameTool =
    this->Target->Target->GetMakefile()->GetSafeDefinition(
      "CMAKE_INSTALL_NAME_TOOL");

  if (installNameTool.empty()) {
    return;
  }

  // Build a map of build-tree install_name to install-tree install_name for
  // shared libraries linked to this target.
  std::map<std::string, std::string> install_name_remap;
  if (cmComputeLinkInformation* cli =
        this->Target->GetLinkInformation(config)) {
    std::set<cmGeneratorTarget const*> const& sharedLibs =
      cli->GetSharedLibrariesLinked();
    for (cmGeneratorTarget const* tgt : sharedLibs) {
      // The install_name of an imported target does not change.
      if (tgt->IsImported()) {
        continue;
      }

      // If the build tree and install tree use different path
      // components of the install_name field then we need to create a
      // mapping to be applied after installation.
      std::string for_build = tgt->GetInstallNameDirForBuildTree(config);
      std::string for_install = tgt->GetInstallNameDirForInstallTree(
        config, "${CMAKE_INSTALL_PREFIX}");
      if (for_build != for_install) {
        // The directory portions differ.  Append the filename to
        // create the mapping.
        std::string fname = this->GetInstallFilename(tgt, config, NameSO);

        // Map from the build-tree install_name.
        for_build += fname;

        // Map to the install-tree install_name.
        for_install += fname;

        // Store the mapping entry.
        install_name_remap[for_build] = for_install;
      }
    }
  }

  // Edit the install_name of the target itself if necessary.
  std::string new_id;
  if (this->Target->GetType() == cmStateEnums::SHARED_LIBRARY) {
    std::string for_build =
      this->Target->GetInstallNameDirForBuildTree(config);
    std::string for_install = this->Target->GetInstallNameDirForInstallTree(
      config, "${CMAKE_INSTALL_PREFIX}");

    if (this->Target->IsFrameworkOnApple() && for_install.empty()) {
      // Frameworks seem to have an id corresponding to their own full
      // path.
      // ...
      // for_install = fullDestPath_without_DESTDIR_or_name;
    }

    // If the install name will change on installation set the new id
    // on the installed file.
    if (for_build != for_install) {
      // Prepare to refer to the install-tree install_name.
      new_id = cmStrCat(
        for_install, this->GetInstallFilename(this->Target, config, NameSO));
    }
  }

  // Write a rule to run install_name_tool to set the install-tree
  // install_name value and references.
  if (!new_id.empty() || !install_name_remap.empty()) {
    os << indent << "execute_process(COMMAND \"" << installNameTool;
    os << "\"";
    if (!new_id.empty()) {
      os << "\n" << indent << "  -id \"" << new_id << "\"";
    }
    for (auto const& i : install_name_remap) {
      os << "\n"
         << indent << "  -change \"" << i.first << "\" \"" << i.second << "\"";
    }
    os << "\n" << indent << "  \"" << toDestDirPath << "\")\n";
  }
}

void cmInstallTargetGenerator::AddRPathCheckRule(
  std::ostream& os, Indent indent, const std::string& config,
  std::string const& toDestDirPath)
{
  // Skip the chrpath if the target does not need it.
  if (this->ImportLibrary || !this->Target->IsChrpathUsed(config)) {
    return;
  }
  // Skip if on Apple
  if (this->Target->Target->GetMakefile()->IsOn(
        "CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    return;
  }

  // Get the link information for this target.
  // It can provide the RPATH.
  cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config);
  if (!cli) {
    return;
  }

  // Write a rule to remove the installed file if its rpath is not the
  // new rpath.  This is needed for existing build/install trees when
  // the installed rpath changes but the file is not rebuilt.
  os << indent << "file(RPATH_CHECK\n"
     << indent << "     FILE \"" << toDestDirPath << "\"\n";

  // CMP0095: ``RPATH`` entries are properly escaped in the intermediary
  // CMake install script.
  switch (this->Target->GetPolicyStatusCMP0095()) {
    case cmPolicies::WARN:
      // No author warning needed here, we warn later in
      // cmInstallTargetGenerator::AddChrpathPatchRule().
      CM_FALLTHROUGH;
    case cmPolicies::OLD: {
      // Get the install RPATH from the link information.
      std::string newRpath = cli->GetChrpathString();
      os << indent << "     RPATH \"" << newRpath << "\")\n";
      break;
    }
    default: {
      // Get the install RPATH from the link information and
      // escape any CMake syntax in the install RPATH.
      std::string escapedNewRpath =
        cmOutputConverter::EscapeForCMake(cli->GetChrpathString());
      os << indent << "     RPATH " << escapedNewRpath << ")\n";
      break;
    }
  }
}

void cmInstallTargetGenerator::AddChrpathPatchRule(
  std::ostream& os, Indent indent, const std::string& config,
  std::string const& toDestDirPath)
{
  // Skip the chrpath if the target does not need it.
  if (this->ImportLibrary || !this->Target->IsChrpathUsed(config)) {
    return;
  }

  // Get the link information for this target.
  // It can provide the RPATH.
  cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config);
  if (!cli) {
    return;
  }

  cmMakefile* mf = this->Target->Target->GetMakefile();

  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    // If using install_name_tool, set up the rules to modify the rpaths.
    std::string installNameTool =
      mf->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL");

    std::vector<std::string> oldRuntimeDirs;
    std::vector<std::string> newRuntimeDirs;
    cli->GetRPath(oldRuntimeDirs, false);
    cli->GetRPath(newRuntimeDirs, true);

    std::string darwin_major_version_s =
      mf->GetSafeDefinition("DARWIN_MAJOR_VERSION");

    std::istringstream ss(darwin_major_version_s);
    int darwin_major_version;
    ss >> darwin_major_version;
    if (!ss.fail() && darwin_major_version <= 9 &&
        (!oldRuntimeDirs.empty() || !newRuntimeDirs.empty())) {
      std::ostringstream msg;
      msg
        << "WARNING: Target \"" << this->Target->GetName()
        << "\" has runtime paths which cannot be changed during install.  "
        << "To change runtime paths, OS X version 10.6 or newer is required.  "
        << "Therefore, runtime paths will not be changed when installing.  "
        << "CMAKE_BUILD_WITH_INSTALL_RPATH may be used to work around"
           " this limitation.";
      mf->IssueMessage(MessageType::WARNING, msg.str());
    } else {
      // To be consistent with older versions, runpath changes must be ordered,
      // deleted first, then added, *and* the same path must only appear once.
      std::map<std::string, std::string> runpath_change;
      std::vector<std::string> ordered;
      for (std::string const& dir : oldRuntimeDirs) {
        // Normalize path and add to map of changes to make
        auto iter_inserted = runpath_change.insert(
          { mf->GetGlobalGenerator()->ExpandCFGIntDir(dir, config),
            "delete" });
        if (iter_inserted.second) {
          // Add path to ordered list of changes
          ordered.push_back(iter_inserted.first->first);
        }
      }

      for (std::string const& dir : newRuntimeDirs) {
        // Normalize path and add to map of changes to make
        auto iter_inserted = runpath_change.insert(
          { mf->GetGlobalGenerator()->ExpandCFGIntDir(dir, config), "add" });
        if (iter_inserted.second) {
          // Add path to ordered list of changes
          ordered.push_back(iter_inserted.first->first);
        } else if (iter_inserted.first->second != "add") {
          // Rpath was requested to be deleted and then later re-added. Drop it
          // from the list by marking as an empty value.
          iter_inserted.first->second.clear();
        }
      }

      // Remove rpaths that are unchanged (value was set to empty)
      ordered.erase(
        std::remove_if(ordered.begin(), ordered.end(),
                       [&runpath_change](const std::string& runpath) {
                         return runpath_change.find(runpath)->second.empty();
                       }),
        ordered.end());

      if (!ordered.empty()) {
        os << indent << "execute_process(COMMAND " << installNameTool << "\n";
        for (std::string const& runpath : ordered) {
          // Either 'add_rpath' or 'delete_rpath' since we've removed empty
          // entries
          os << indent << "  -" << runpath_change.find(runpath)->second
             << "_rpath \"" << runpath << "\"\n";
        }
        os << indent << "  \"" << toDestDirPath << "\")\n";
      }
    }
  } else {
    // Construct the original rpath string to be replaced.
    std::string oldRpath = cli->GetRPathString(false);

    // Get the install RPATH from the link information.
    std::string newRpath = cli->GetChrpathString();

    // Skip the rule if the paths are identical
    if (oldRpath == newRpath) {
      return;
    }

    // Escape any CMake syntax in the RPATHs.
    std::string escapedOldRpath = cmOutputConverter::EscapeForCMake(oldRpath);
    std::string escapedNewRpath = cmOutputConverter::EscapeForCMake(newRpath);

    // Write a rule to run chrpath to set the install-tree RPATH
    os << indent << "file(RPATH_CHANGE\n"
       << indent << "     FILE \"" << toDestDirPath << "\"\n"
       << indent << "     OLD_RPATH " << escapedOldRpath << "\n";

    // CMP0095: ``RPATH`` entries are properly escaped in the intermediary
    // CMake install script.
    switch (this->Target->GetPolicyStatusCMP0095()) {
      case cmPolicies::WARN:
        this->IssueCMP0095Warning(newRpath);
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        os << indent << "     NEW_RPATH \"" << newRpath << "\"";
        break;
      default:
        os << indent << "     NEW_RPATH " << escapedNewRpath;
        break;
    }

    if (this->Target->GetPropertyAsBool("INSTALL_REMOVE_ENVIRONMENT_RPATH")) {
      os << "\n" << indent << "     INSTALL_REMOVE_ENVIRONMENT_RPATH)\n";
    } else {
      os << ")\n";
    }
  }
}

void cmInstallTargetGenerator::AddStripRule(std::ostream& os, Indent indent,
                                            const std::string& toDestDirPath)
{

  // don't strip static and import libraries, because it removes the only
  // symbol table they have so you can't link to them anymore
  if (this->Target->GetType() == cmStateEnums::STATIC_LIBRARY ||
      this->ImportLibrary) {
    return;
  }

  // Don't handle OSX Bundles.
  if (this->Target->Target->GetMakefile()->IsOn("APPLE") &&
      this->Target->GetPropertyAsBool("MACOSX_BUNDLE")) {
    return;
  }

  if (!this->Target->Target->GetMakefile()->IsSet("CMAKE_STRIP")) {
    return;
  }

  std::string stripArgs;

  // macOS 'strip' is picky, executables need '-u -r' and dylibs need '-x'.
  if (this->Target->Target->GetMakefile()->IsOn("APPLE")) {
    if (this->Target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->Target->GetType() == cmStateEnums::MODULE_LIBRARY) {
      stripArgs = "-x ";
    } else if (this->Target->GetType() == cmStateEnums::EXECUTABLE) {
      stripArgs = "-u -r ";
    }
  }

  os << indent << "if(CMAKE_INSTALL_DO_STRIP)\n";
  os << indent << "  execute_process(COMMAND \""
     << this->Target->Target->GetMakefile()->GetSafeDefinition("CMAKE_STRIP")
     << "\" " << stripArgs << "\"" << toDestDirPath << "\")\n";
  os << indent << "endif()\n";
}

void cmInstallTargetGenerator::AddRanlibRule(std::ostream& os, Indent indent,
                                             const std::string& toDestDirPath)
{
  // Static libraries need ranlib on this platform.
  if (this->Target->GetType() != cmStateEnums::STATIC_LIBRARY) {
    return;
  }

  // Perform post-installation processing on the file depending
  // on its type.
  if (!this->Target->Target->GetMakefile()->IsOn("APPLE")) {
    return;
  }

  const std::string& ranlib =
    this->Target->Target->GetMakefile()->GetRequiredDefinition("CMAKE_RANLIB");
  if (ranlib.empty()) {
    return;
  }

  os << indent << "execute_process(COMMAND \"" << ranlib << "\" \""
     << toDestDirPath << "\")\n";
}

void cmInstallTargetGenerator::AddUniversalInstallRule(
  std::ostream& os, Indent indent, const std::string& toDestDirPath)
{
  cmMakefile const* mf = this->Target->Target->GetMakefile();

  if (!mf->PlatformIsAppleEmbedded() || !mf->IsOn("XCODE")) {
    return;
  }

  cmValue xcodeVersion = mf->GetDefinition("XCODE_VERSION");
  if (!xcodeVersion ||
      cmSystemTools::VersionCompareGreater("6", *xcodeVersion)) {
    return;
  }

  switch (this->Target->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      break;

    default:
      return;
  }

  if (!this->Target->Target->GetPropertyAsBool("IOS_INSTALL_COMBINED")) {
    return;
  }

  os << indent << "include(CMakeIOSInstallCombined)\n";
  os << indent << "ios_install_combined("
     << "\"" << this->Target->Target->GetName() << "\" "
     << "\"" << toDestDirPath << "\")\n";
}

void cmInstallTargetGenerator::IssueCMP0095Warning(
  const std::string& unescapedRpath)
{
  // Reduce warning noise to cases where used RPATHs may actually be affected
  // by CMP0095. This filter is meant to skip warnings in cases when
  // non-curly-braces syntax (e.g. $ORIGIN) or no keyword is used which has
  // worked already before CMP0095. We intend to issue a warning in all cases
  // with curly-braces syntax, even if the workaround of double-escaping is in
  // place, since we deprecate the need for it with CMP0095.
  const bool potentially_affected(unescapedRpath.find("${") !=
                                  std::string::npos);

  if (potentially_affected) {
    std::ostringstream w;
    w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0095) << "\n";
    w << "RPATH entries for target '" << this->Target->GetName() << "' "
      << "will not be escaped in the intermediary "
      << "cmake_install.cmake script.";
    this->Target->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
      MessageType::AUTHOR_WARNING, w.str(), this->GetBacktrace());
  }
}
