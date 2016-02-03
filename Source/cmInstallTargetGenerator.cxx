/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallTargetGenerator.h"

#include "cmComputeLinkInformation.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratorTarget.h"
#include "cmake.h"
#include "cmGeneratorTarget.h"

#include <assert.h>

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(const std::string& targetName,
                           const char* dest, bool implib,
                           const char* file_permissions,
                           std::vector<std::string> const& configurations,
                           const char* component,
                           MessageLevel message,
                           bool optional):
  cmInstallGenerator(dest, configurations, component, message),
  TargetName(targetName),
  Target(0),
  FilePermissions(file_permissions),
  ImportLibrary(implib),
  Optional(optional)
{
  this->ActionsPerConfig = true;
  this->NamelinkMode = NamelinkModeNone;
}

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::~cmInstallTargetGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::GenerateScript(std::ostream& os)
{
  // Warn if installing an exclude-from-all target.
  if(this->Target->GetPropertyAsBool("EXCLUDE_FROM_ALL"))
    {
    std::ostringstream msg;
    msg << "WARNING: Target \"" << this->Target->GetName()
        << "\" has EXCLUDE_FROM_ALL set and will not be built by default "
        << "but an install rule has been provided for it.  CMake does "
        << "not define behavior for this case.";
    cmSystemTools::Message(msg.str().c_str(), "Warning");
    }

  // Perform the main install script generation.
  this->cmInstallGenerator::GenerateScript(os);
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::GenerateScriptForConfig(std::ostream& os,
                                                    const std::string& config,
                                                    Indent const& indent)
{
  // Compute the build tree directory from which to copy the target.
  std::string fromDirConfig;
  if(this->Target->NeedRelinkBeforeInstall(config))
    {
    fromDirConfig =
        this->Target->GetLocalGenerator()->GetCurrentBinaryDirectory();
    fromDirConfig += cmake::GetCMakeFilesDirectory();
    fromDirConfig += "/CMakeRelink.dir/";
    }
  else
    {
    fromDirConfig =
        this->Target->GetDirectory(config, this->ImportLibrary);
    fromDirConfig += "/";
    }
  std::string toDir =
    this->ConvertToAbsoluteDestination(this->GetDestination(config));
  toDir += "/";

  // Compute the list of files to install for this target.
  std::vector<std::string> filesFrom;
  std::vector<std::string> filesTo;
  std::string literal_args;
  cmState::TargetType targetType = this->Target->GetType();
  cmInstallType type = cmInstallType();
  switch(targetType)
    {
    case cmState::EXECUTABLE: type = cmInstallType_EXECUTABLE; break;
    case cmState::STATIC_LIBRARY: type = cmInstallType_STATIC_LIBRARY; break;
    case cmState::SHARED_LIBRARY: type = cmInstallType_SHARED_LIBRARY; break;
    case cmState::MODULE_LIBRARY: type = cmInstallType_MODULE_LIBRARY; break;
    case cmState::INTERFACE_LIBRARY:
      // Not reachable. We never create a cmInstallTargetGenerator for
      // an INTERFACE_LIBRARY.
      assert(0 && "INTERFACE_LIBRARY targets have no installable outputs.");
      break;
    case cmState::OBJECT_LIBRARY:
    case cmState::UTILITY:
    case cmState::GLOBAL_TARGET:
    case cmState::UNKNOWN_LIBRARY:
      this->Target->GetLocalGenerator()->IssueMessage(cmake::INTERNAL_ERROR,
        "cmInstallTargetGenerator created with non-installable target.");
      return;
    }
  if(targetType == cmState::EXECUTABLE)
    {
    // There is a bug in cmInstallCommand if this fails.
    assert(this->NamelinkMode == NamelinkModeNone);

    std::string targetName;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    this->Target->GetExecutableNames(targetName, targetNameReal,
                                     targetNameImport, targetNamePDB,
                                     config);
    if(this->ImportLibrary)
      {
      std::string from1 = fromDirConfig + targetNameImport;
      std::string to1 = toDir + targetNameImport;
      filesFrom.push_back(from1);
      filesTo.push_back(to1);
      std::string targetNameImportLib;
      if(this->Target->GetImplibGNUtoMS(targetNameImport,
                                        targetNameImportLib))
        {
        filesFrom.push_back(fromDirConfig + targetNameImportLib);
        filesTo.push_back(toDir + targetNameImportLib);
        }

      // An import library looks like a static library.
      type = cmInstallType_STATIC_LIBRARY;
      }
    else
      {
      std::string from1 = fromDirConfig + targetName;
      std::string to1 = toDir + targetName;

      // Handle OSX Bundles.
      if(this->Target->IsAppBundleOnApple())
        {
        cmMakefile const* mf = this->Target->Target->GetMakefile();

        // Install the whole app bundle directory.
        type = cmInstallType_DIRECTORY;
        literal_args += " USE_SOURCE_PERMISSIONS";
        from1 += ".app";

        // Tweaks apply to the binary inside the bundle.
        to1 += ".app/";
        if(!mf->PlatformIsAppleIos())
          {
          to1 += "Contents/MacOS/";
          }
        to1 += targetName;
        }
      else
        {
        // Tweaks apply to the real file, so list it first.
        if(targetNameReal != targetName)
          {
          std::string from2 = fromDirConfig + targetNameReal;
          std::string to2 = toDir += targetNameReal;
          filesFrom.push_back(from2);
          filesTo.push_back(to2);
          }
        }

      filesFrom.push_back(from1);
      filesTo.push_back(to1);
      }
    }
  else
    {
    std::string targetName;
    std::string targetNameSO;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    this->Target->GetLibraryNames(targetName, targetNameSO,
                                          targetNameReal,
                                  targetNameImport, targetNamePDB,
                                  config);
    if(this->ImportLibrary)
      {
      // There is a bug in cmInstallCommand if this fails.
      assert(this->NamelinkMode == NamelinkModeNone);

      std::string from1 = fromDirConfig + targetNameImport;
      std::string to1 = toDir + targetNameImport;
      filesFrom.push_back(from1);
      filesTo.push_back(to1);
      std::string targetNameImportLib;
      if(this->Target->GetImplibGNUtoMS(targetNameImport,
                                        targetNameImportLib))
        {
        filesFrom.push_back(fromDirConfig + targetNameImportLib);
        filesTo.push_back(toDir + targetNameImportLib);
        }

      // An import library looks like a static library.
      type = cmInstallType_STATIC_LIBRARY;
      }
    else if(this->Target->IsFrameworkOnApple())
      {
      // There is a bug in cmInstallCommand if this fails.
      assert(this->NamelinkMode == NamelinkModeNone);

      // Install the whole framework directory.
      type = cmInstallType_DIRECTORY;
      literal_args += " USE_SOURCE_PERMISSIONS";

      std::string from1 = fromDirConfig + targetName;
      from1 = cmSystemTools::GetFilenamePath(from1);

      // Tweaks apply to the binary inside the bundle.
      std::string to1 = toDir + targetNameReal;

      filesFrom.push_back(from1);
      filesTo.push_back(to1);
      }
    else if(this->Target->IsCFBundleOnApple())
      {
      // Install the whole app bundle directory.
      type = cmInstallType_DIRECTORY;
      literal_args += " USE_SOURCE_PERMISSIONS";

      std::string targetNameBase = targetName.substr(0, targetName.find('/'));

      std::string from1 = fromDirConfig + targetNameBase;
      std::string to1 = toDir + targetName;

      filesFrom.push_back(from1);
      filesTo.push_back(to1);
      }
    else
      {
      bool haveNamelink = false;

      // Library link name.
      std::string fromName = fromDirConfig + targetName;
      std::string toName = toDir + targetName;

      // Library interface name.
      std::string fromSOName;
      std::string toSOName;
      if(targetNameSO != targetName)
        {
        haveNamelink = true;
        fromSOName = fromDirConfig + targetNameSO;
        toSOName = toDir + targetNameSO;
        }

      // Library implementation name.
      std::string fromRealName;
      std::string toRealName;
      if(targetNameReal != targetName &&
         targetNameReal != targetNameSO)
        {
        haveNamelink = true;
        fromRealName = fromDirConfig + targetNameReal;
        toRealName = toDir + targetNameReal;
        }

      // Add the names based on the current namelink mode.
      if(haveNamelink)
        {
        // With a namelink we need to check the mode.
        if(this->NamelinkMode == NamelinkModeOnly)
          {
          // Install the namelink only.
          filesFrom.push_back(fromName);
          filesTo.push_back(toName);
          }
        else
          {
          // Install the real file if it has its own name.
          if(!fromRealName.empty())
            {
            filesFrom.push_back(fromRealName);
            filesTo.push_back(toRealName);
            }

          // Install the soname link if it has its own name.
          if(!fromSOName.empty())
            {
            filesFrom.push_back(fromSOName);
            filesTo.push_back(toSOName);
            }

          // Install the namelink if it is not to be skipped.
          if(this->NamelinkMode != NamelinkModeSkip)
            {
            filesFrom.push_back(fromName);
            filesTo.push_back(toName);
            }
          }
        }
      else
        {
        // Without a namelink there will be only one file.  Install it
        // if this is not a namelink-only rule.
        if(this->NamelinkMode != NamelinkModeOnly)
          {
          filesFrom.push_back(fromName);
          filesTo.push_back(toName);
          }
        }
      }
    }

  // If this fails the above code is buggy.
  assert(filesFrom.size() == filesTo.size());

  // Skip this rule if no files are to be installed for the target.
  if(filesFrom.empty())
    {
    return;
    }

  // Add pre-installation tweaks.
  this->AddTweak(os, indent, config, filesTo,
                 &cmInstallTargetGenerator::PreReplacementTweaks);

  // Write code to install the target file.
  const char* no_dir_permissions = 0;
  const char* no_rename = 0;
  bool optional = this->Optional || this->ImportLibrary;
  this->AddInstallRule(os, this->GetDestination(config),
                       type, filesFrom, optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_rename, literal_args.c_str(),
                       indent);

  // Add post-installation tweaks.
  this->AddTweak(os, indent, config, filesTo,
                 &cmInstallTargetGenerator::PostReplacementTweaks);
}

//----------------------------------------------------------------------------
std::string
cmInstallTargetGenerator::GetDestination(std::string const& config) const
{
  cmGeneratorExpression ge;
  return ge.Parse(this->Destination)
    ->Evaluate(this->Target->GetLocalGenerator(), config);
}

//----------------------------------------------------------------------------
std::string
cmInstallTargetGenerator::GetInstallFilename(const std::string& config) const
{
  NameType nameType = this->ImportLibrary? NameImplib : NameNormal;
  return
    cmInstallTargetGenerator::GetInstallFilename(this->Target, config,
                                                 nameType);
}

//----------------------------------------------------------------------------
std::string
cmInstallTargetGenerator::GetInstallFilename(cmGeneratorTarget const* target,
                                             const std::string& config,
                                             NameType nameType)
{
  std::string fname;
  // Compute the name of the library.
  if(target->GetType() == cmState::EXECUTABLE)
    {
    std::string targetName;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    target->GetExecutableNames(targetName, targetNameReal,
                               targetNameImport, targetNamePDB,
                               config);
    if(nameType == NameImplib)
      {
      // Use the import library name.
      if(!target->GetImplibGNUtoMS(targetNameImport, fname,
                                   "${CMAKE_IMPORT_LIBRARY_SUFFIX}"))
        {
        fname = targetNameImport;
        }
      }
    else if(nameType == NameReal)
      {
      // Use the canonical name.
      fname = targetNameReal;
      }
    else
      {
      // Use the canonical name.
      fname = targetName;
      }
    }
  else
    {
    std::string targetName;
    std::string targetNameSO;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    target->GetLibraryNames(targetName, targetNameSO, targetNameReal,
                            targetNameImport, targetNamePDB, config);
    if(nameType == NameImplib)
      {
      // Use the import library name.
      if(!target->GetImplibGNUtoMS(targetNameImport, fname,
                                   "${CMAKE_IMPORT_LIBRARY_SUFFIX}"))
        {
        fname = targetNameImport;
        }
      }
    else if(nameType == NameSO)
      {
      // Use the soname.
      fname = targetNameSO;
      }
    else if(nameType == NameReal)
      {
      // Use the real name.
      fname = targetNameReal;
      }
    else
      {
      // Use the canonical name.
      fname = targetName;
      }
    }

  return fname;
}

void cmInstallTargetGenerator::Compute(cmLocalGenerator* lg)
{
  this->Target = lg->FindGeneratorTarget(this->TargetName);
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddTweak(std::ostream& os, Indent const& indent, const std::string& config,
           std::string const& file, TweakMethod tweak)
{
  std::ostringstream tw;
  (this->*tweak)(tw, indent.Next(), config, file);
  std::string tws = tw.str();
  if(!tws.empty())
    {
    os << indent << "if(EXISTS \"" << file << "\" AND\n"
       << indent << "   NOT IS_SYMLINK \"" << file << "\")\n";
    os << tws;
    os << indent << "endif()\n";
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddTweak(std::ostream& os, Indent const& indent, const std::string& config,
           std::vector<std::string> const& files, TweakMethod tweak)
{
  if(files.size() == 1)
    {
    // Tweak a single file.
    this->AddTweak(os, indent, config, this->GetDestDirPath(files[0]), tweak);
    }
  else
    {
    // Generate a foreach loop to tweak multiple files.
    std::ostringstream tw;
    this->AddTweak(tw, indent.Next(), config, "${file}", tweak);
    std::string tws = tw.str();
    if(!tws.empty())
      {
      Indent indent2 = indent.Next().Next();
      os << indent << "foreach(file\n";
      for(std::vector<std::string>::const_iterator i = files.begin();
          i != files.end(); ++i)
        {
        os << indent2 << "\"" << this->GetDestDirPath(*i) << "\"\n";
        }
      os << indent2 << ")\n";
      os << tws;
      os << indent << "endforeach()\n";
      }
    }
}

//----------------------------------------------------------------------------
std::string cmInstallTargetGenerator::GetDestDirPath(std::string const& file)
{
  // Construct the path of the file on disk after installation on
  // which tweaks may be performed.
  std::string toDestDirPath = "$ENV{DESTDIR}";
  if(file[0] != '/' && file[0] != '$')
    {
    toDestDirPath += "/";
    }
  toDestDirPath += file;
  return toDestDirPath;
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::PreReplacementTweaks(std::ostream& os,
                                                    Indent const& indent,
                                                    const std::string& config,
                                                    std::string const& file)
{
  this->AddRPathCheckRule(os, indent, config, file);
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::PostReplacementTweaks(std::ostream& os,
                                                    Indent const& indent,
                                                    const std::string& config,
                                                    std::string const& file)
{
  this->AddInstallNamePatchRule(os, indent, config, file);
  this->AddChrpathPatchRule(os, indent, config, file);
  this->AddUniversalInstallRule(os, indent, file);
  this->AddRanlibRule(os, indent, file);
  this->AddStripRule(os, indent, file);
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddInstallNamePatchRule(std::ostream& os, Indent const& indent,
                          const std::string& config,
                          std::string const& toDestDirPath)
{
  if(this->ImportLibrary ||
     !(this->Target->GetType() == cmState::SHARED_LIBRARY ||
       this->Target->GetType() == cmState::MODULE_LIBRARY ||
       this->Target->GetType() == cmState::EXECUTABLE))
    {
    return;
    }

  // Fix the install_name settings in installed binaries.
  std::string installNameTool = this->Target->Target->GetMakefile()
      ->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL");

  if(installNameTool.empty())
    {
    return;
    }

  // Build a map of build-tree install_name to install-tree install_name for
  // shared libraries linked to this target.
  std::map<std::string, std::string> install_name_remap;
  if(cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config))
    {
    std::set<cmGeneratorTarget const*> const& sharedLibs
                                            = cli->GetSharedLibrariesLinked();
    for(std::set<cmGeneratorTarget const*>::const_iterator j
        = sharedLibs.begin(); j != sharedLibs.end(); ++j)
      {
      cmGeneratorTarget const* tgt = *j;

      // The install_name of an imported target does not change.
      if(tgt->IsImported())
        {
        continue;
        }

      // If the build tree and install tree use different path
      // components of the install_name field then we need to create a
      // mapping to be applied after installation.
      std::string for_build = tgt->GetInstallNameDirForBuildTree(config);
      std::string for_install = tgt->GetInstallNameDirForInstallTree();
      if(for_build != for_install)
        {
        // The directory portions differ.  Append the filename to
        // create the mapping.
        std::string fname =
          this->GetInstallFilename(tgt, config, NameSO);

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
  if(this->Target->GetType() == cmState::SHARED_LIBRARY)
    {
    std::string for_build =
      this->Target->GetInstallNameDirForBuildTree(config);
    std::string for_install =
      this->Target->GetInstallNameDirForInstallTree();

    if(this->Target->IsFrameworkOnApple() && for_install.empty())
      {
      // Frameworks seem to have an id corresponding to their own full
      // path.
      // ...
      // for_install = fullDestPath_without_DESTDIR_or_name;
      }

    // If the install name will change on installation set the new id
    // on the installed file.
    if(for_build != for_install)
      {
      // Prepare to refer to the install-tree install_name.
      new_id = for_install;
      new_id += this->GetInstallFilename(this->Target, config, NameSO);
      }
    }

  // Write a rule to run install_name_tool to set the install-tree
  // install_name value and references.
  if(!new_id.empty() || !install_name_remap.empty())
    {
    os << indent << "execute_process(COMMAND \"" << installNameTool;
    os << "\"";
    if(!new_id.empty())
      {
      os << "\n" << indent << "  -id \"" << new_id << "\"";
      }
    for(std::map<std::string, std::string>::const_iterator
          i = install_name_remap.begin();
        i != install_name_remap.end(); ++i)
      {
      os << "\n" << indent << "  -change \""
         << i->first << "\" \"" << i->second << "\"";
      }
    os << "\n" << indent << "  \"" << toDestDirPath << "\")\n";
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddRPathCheckRule(std::ostream& os, Indent const& indent,
                    const std::string& config,
                    std::string const& toDestDirPath)
{
  // Skip the chrpath if the target does not need it.
  if(this->ImportLibrary || !this->Target->IsChrpathUsed(config))
    {
    return;
    }
  // Skip if on Apple
  if(this->Target->Target->GetMakefile()
     ->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    return;
    }

  // Get the link information for this target.
  // It can provide the RPATH.
  cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config);
  if(!cli)
    {
    return;
    }

  // Get the install RPATH from the link information.
  std::string newRpath = cli->GetChrpathString();

  // Write a rule to remove the installed file if its rpath is not the
  // new rpath.  This is needed for existing build/install trees when
  // the installed rpath changes but the file is not rebuilt.
  os << indent << "file(RPATH_CHECK\n"
     << indent << "     FILE \"" << toDestDirPath << "\"\n"
     << indent << "     RPATH \"" << newRpath << "\")\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddChrpathPatchRule(std::ostream& os, Indent const& indent,
                      const std::string& config,
                      std::string const& toDestDirPath)
{
  // Skip the chrpath if the target does not need it.
  if(this->ImportLibrary || !this->Target->IsChrpathUsed(config))
    {
    return;
    }

  // Get the link information for this target.
  // It can provide the RPATH.
  cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config);
  if(!cli)
    {
    return;
    }

  cmMakefile* mf = this->Target->Target->GetMakefile();

  if(mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    // If using install_name_tool, set up the rules to modify the rpaths.
    std::string installNameTool =
      mf->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL");

    std::vector<std::string> oldRuntimeDirs, newRuntimeDirs;
    cli->GetRPath(oldRuntimeDirs, false);
    cli->GetRPath(newRuntimeDirs, true);

    std::string darwin_major_version_s =
      mf->GetSafeDefinition("DARWIN_MAJOR_VERSION");

    std::stringstream ss(darwin_major_version_s);
    int darwin_major_version;
    ss >> darwin_major_version;
    if(!ss.fail() && darwin_major_version <= 9 &&
       (!oldRuntimeDirs.empty() || !newRuntimeDirs.empty())
      )
      {
      std::ostringstream msg;
      msg << "WARNING: Target \"" << this->Target->GetName()
        << "\" has runtime paths which cannot be changed during install.  "
        << "To change runtime paths, OS X version 10.6 or newer is required.  "
        << "Therefore, runtime paths will not be changed when installing.  "
        << "CMAKE_BUILD_WITH_INSTALL_RPATH may be used to work around"
           " this limitation.";
      mf->IssueMessage(cmake::WARNING, msg.str());
      }
    else
      {
      // Note: These paths are kept unique to avoid
      // install_name_tool corruption.
      std::set<std::string> runpaths;
      for(std::vector<std::string>::const_iterator i = oldRuntimeDirs.begin();
          i != oldRuntimeDirs.end(); ++i)
        {
        std::string runpath =
          mf->GetGlobalGenerator()->ExpandCFGIntDir(*i, config);

        if(runpaths.find(runpath) == runpaths.end())
          {
          runpaths.insert(runpath);
          os << indent << "execute_process(COMMAND " << installNameTool <<"\n";
          os << indent << "  -delete_rpath \"" << runpath << "\"\n";
          os << indent << "  \"" << toDestDirPath << "\")\n";
          }
        }

      runpaths.clear();
      for(std::vector<std::string>::const_iterator i = newRuntimeDirs.begin();
          i != newRuntimeDirs.end(); ++i)
        {
        std::string runpath =
          mf->GetGlobalGenerator()->ExpandCFGIntDir(*i, config);

        if(runpaths.find(runpath) == runpaths.end())
          {
          os << indent << "execute_process(COMMAND " << installNameTool <<"\n";
          os << indent << "  -add_rpath \"" << runpath << "\"\n";
          os << indent << "  \"" << toDestDirPath << "\")\n";
          }
        }
      }
    }
  else
    {
    // Construct the original rpath string to be replaced.
    std::string oldRpath = cli->GetRPathString(false);

    // Get the install RPATH from the link information.
    std::string newRpath = cli->GetChrpathString();

    // Skip the rule if the paths are identical
    if(oldRpath == newRpath)
      {
      return;
      }

    // Write a rule to run chrpath to set the install-tree RPATH
    os << indent << "file(RPATH_CHANGE\n"
       << indent << "     FILE \"" << toDestDirPath << "\"\n"
       << indent << "     OLD_RPATH \"" << oldRpath << "\"\n"
       << indent << "     NEW_RPATH \"" << newRpath << "\")\n";
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator::AddStripRule(std::ostream& os,
                                       Indent const& indent,
                                       const std::string& toDestDirPath)
{

  // don't strip static and import libraries, because it removes the only
  // symbol table they have so you can't link to them anymore
  if(this->Target->GetType()==cmState::STATIC_LIBRARY || this->ImportLibrary)
    {
    return;
    }

  // Don't handle OSX Bundles.
  if(this->Target->Target->GetMakefile()->IsOn("APPLE") &&
     this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
    {
    return;
    }

  if(! this->Target->Target->GetMakefile()->IsSet("CMAKE_STRIP"))
    {
    return;
    }

  os << indent << "if(CMAKE_INSTALL_DO_STRIP)\n";
  os << indent << "  execute_process(COMMAND \""
     << this->Target->Target->GetMakefile()->GetDefinition("CMAKE_STRIP")
     << "\" \"" << toDestDirPath << "\")\n";
  os << indent << "endif()\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator::AddRanlibRule(std::ostream& os,
                                        Indent const& indent,
                                        const std::string& toDestDirPath)
{
  // Static libraries need ranlib on this platform.
  if(this->Target->GetType() != cmState::STATIC_LIBRARY)
    {
    return;
    }

  // Perform post-installation processing on the file depending
  // on its type.
  if(!this->Target->Target->GetMakefile()->IsOn("APPLE"))
    {
    return;
    }

  std::string ranlib =
    this->Target->Target->GetMakefile()->GetRequiredDefinition("CMAKE_RANLIB");
  if(ranlib.empty())
    {
    return;
    }

  os << indent << "execute_process(COMMAND \""
     << ranlib << "\" \"" << toDestDirPath << "\")\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddUniversalInstallRule(std::ostream& os,
                          Indent const& indent,
                          const std::string& toDestDirPath)
{
  cmMakefile const* mf = this->Target->Target->GetMakefile();

  if(!mf->PlatformIsAppleIos() || !mf->IsOn("XCODE"))
    {
    return;
    }

  const char* xcodeVersion = mf->GetDefinition("XCODE_VERSION");
  if(!xcodeVersion || cmSystemTools::VersionCompareGreater("6", xcodeVersion))
    {
    return;
    }

  switch(this->Target->GetType())
    {
    case cmState::EXECUTABLE:
    case cmState::STATIC_LIBRARY:
    case cmState::SHARED_LIBRARY:
    case cmState::MODULE_LIBRARY:
      break;

    default:
      return;
    }

  if(!this->Target->Target->GetPropertyAsBool("IOS_INSTALL_COMBINED"))
   {
   return;
   }

  os << indent << "include(CMakeIOSInstallCombined)\n";
  os << indent << "ios_install_combined("
               << "\"" << this->Target->Target->GetName() << "\" "
               << "\"" << toDestDirPath << "\")\n";
}
