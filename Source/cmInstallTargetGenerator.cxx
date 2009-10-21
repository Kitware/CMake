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
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

#include <assert.h>

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(cmTarget& t, const char* dest, bool implib,
                           const char* file_permissions,
                           std::vector<std::string> const& configurations,
                           const char* component, bool optional):
  cmInstallGenerator(dest, configurations, component), Target(&t),
  ImportLibrary(implib), FilePermissions(file_permissions), Optional(optional)
{
  this->ActionsPerConfig = true;
  this->NamelinkMode = NamelinkModeNone;
  this->Target->SetHaveInstallRule(true);
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
    cmOStringStream msg;
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
                                                       const char* config,
                                                       Indent const& indent)
{
  // Compute the build tree directory from which to copy the target.
  std::string fromDirConfig;
  if(this->Target->NeedRelinkBeforeInstall(config))
    {
    fromDirConfig = this->Target->GetMakefile()->GetStartOutputDirectory();
    fromDirConfig += cmake::GetCMakeFilesDirectory();
    fromDirConfig += "/CMakeRelink.dir/";
    }
  else
    {
    fromDirConfig = this->Target->GetDirectory(config, this->ImportLibrary);
    fromDirConfig += "/";
    }
  std::string toDir = this->GetInstallDestination();
  toDir += "/";

  // Compute the list of files to install for this target.
  std::vector<std::string> filesFrom;
  std::vector<std::string> filesTo;
  std::string literal_args;
  cmTarget::TargetType type = this->Target->GetType();
  if(type == cmTarget::EXECUTABLE)
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

      // An import library looks like a static library.
      type = cmTarget::STATIC_LIBRARY;
      }
    else
      {
      std::string from1 = fromDirConfig + targetName;
      std::string to1 = toDir + targetName;

      // Handle OSX Bundles.
      if(this->Target->IsAppBundleOnApple())
        {
        // Install the whole app bundle directory.
        type = cmTarget::INSTALL_DIRECTORY;
        literal_args += " USE_SOURCE_PERMISSIONS";
        from1 += ".app";

        // Tweaks apply to the binary inside the bundle.
        to1 += ".app/Contents/MacOS/";
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
    this->Target->GetLibraryNames(targetName, targetNameSO, targetNameReal,
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

      // An import library looks like a static library.
      type = cmTarget::STATIC_LIBRARY;
      }
    else if(this->Target->IsFrameworkOnApple())
      {
      // There is a bug in cmInstallCommand if this fails.
      assert(this->NamelinkMode == NamelinkModeNone);

      // Install the whole framework directory.
      type = cmTarget::INSTALL_DIRECTORY;
      literal_args += " USE_SOURCE_PERMISSIONS";
      std::string from1 = fromDirConfig + targetName + ".framework";

      // Tweaks apply to the binary inside the bundle.
      std::string to1 = toDir + targetName;
      to1 += ".framework/Versions/";
      to1 += this->Target->GetFrameworkVersion();
      to1 += "/";
      to1 += targetName;

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
  this->AddInstallRule(os, type, filesFrom,
                       optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_rename, literal_args.c_str(),
                       indent);

  // Add post-installation tweaks.
  this->AddTweak(os, indent, config, filesTo,
                 &cmInstallTargetGenerator::PostReplacementTweaks);
}

//----------------------------------------------------------------------------
std::string
cmInstallTargetGenerator::GetInstallFilename(const char* config) const
{
  NameType nameType = this->ImportLibrary? NameImplib : NameNormal;
  return
    cmInstallTargetGenerator::GetInstallFilename(this->Target, config,
                                                 nameType);
}

//----------------------------------------------------------------------------
std::string cmInstallTargetGenerator::GetInstallFilename(cmTarget* target,
                                                         const char* config,
                                                         NameType nameType)
{
  std::string fname;
  // Compute the name of the library.
  if(target->GetType() == cmTarget::EXECUTABLE)
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
      fname = targetNameImport;
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
      fname = targetNameImport;
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

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddTweak(std::ostream& os, Indent const& indent, const char* config,
           std::string const& file, TweakMethod tweak)
{
  cmOStringStream tw;
  (this->*tweak)(tw, indent.Next(), config, file);
  std::string tws = tw.str();
  if(!tws.empty())
    {
    os << indent << "IF(EXISTS \"" << file << "\" AND\n"
       << indent << "   NOT IS_SYMLINK \"" << file << "\")\n";
    os << tws;
    os << indent << "ENDIF()\n";
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddTweak(std::ostream& os, Indent const& indent, const char* config,
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
    cmOStringStream tw;
    this->AddTweak(tw, indent.Next(), config, "${file}", tweak);
    std::string tws = tw.str();
    if(!tws.empty())
      {
      Indent indent2 = indent.Next().Next();
      os << indent << "FOREACH(file\n";
      for(std::vector<std::string>::const_iterator i = files.begin();
          i != files.end(); ++i)
        {
        os << indent2 << "\"" << this->GetDestDirPath(*i) << "\"\n";
        }
      os << indent2 << ")\n";
      os << tws;
      os << indent << "ENDFOREACH()\n";
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
                                                    const char* config,
                                                    std::string const& file)
{
  this->AddRPathCheckRule(os, indent, config, file);
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::PostReplacementTweaks(std::ostream& os,
                                                     Indent const& indent,
                                                     const char* config,
                                                     std::string const& file)
{
  this->AddInstallNamePatchRule(os, indent, config, file);
  this->AddChrpathPatchRule(os, indent, config, file);
  this->AddRanlibRule(os, indent, file);
  this->AddStripRule(os, indent, file);
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddInstallNamePatchRule(std::ostream& os, Indent const& indent,
                          const char* config, std::string const& toDestDirPath)
{
  if(this->ImportLibrary ||
     !(this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
       this->Target->GetType() == cmTarget::MODULE_LIBRARY ||
       this->Target->GetType() == cmTarget::EXECUTABLE))
    {
    return;
    }

  // Fix the install_name settings in installed binaries.
  std::string installNameTool =
    this->Target->GetMakefile()->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL");

  if(!installNameTool.size())
    {
    return;
    }

  // Build a map of build-tree install_name to install-tree install_name for
  // shared libraries linked to this target.
  std::map<cmStdString, cmStdString> install_name_remap;
  if(cmComputeLinkInformation* cli = this->Target->GetLinkInformation(config))
    {
    std::set<cmTarget*> const& sharedLibs = cli->GetSharedLibrariesLinked();
    for(std::set<cmTarget*>::const_iterator j = sharedLibs.begin();
        j != sharedLibs.end(); ++j)
      {
      cmTarget* tgt = *j;

      // The install_name of an imported target does not change.
      if(tgt->IsImported())
        {
        continue;
        }

      // If the build tree and install tree use different path
      // components of the install_name field then we need to create a
      // mapping to be applied after installation.
      std::string for_build = tgt->GetInstallNameDirForBuildTree(config);
      std::string for_install = tgt->GetInstallNameDirForInstallTree(config);
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
  if(this->Target->GetType() == cmTarget::SHARED_LIBRARY)
    {
    std::string for_build =
      this->Target->GetInstallNameDirForBuildTree(config);
    std::string for_install =
      this->Target->GetInstallNameDirForInstallTree(config);

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
    os << indent << "EXECUTE_PROCESS(COMMAND \"" << installNameTool;
    os << "\"";
    if(!new_id.empty())
      {
      os << "\n" << indent << "  -id \"" << new_id << "\"";
      }
    for(std::map<cmStdString, cmStdString>::const_iterator
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
                    const char* config, std::string const& toDestDirPath)
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

  // Get the install RPATH from the link information.
  std::string newRpath = cli->GetChrpathString();

  // Write a rule to remove the installed file if its rpath is not the
  // new rpath.  This is needed for existing build/install trees when
  // the installed rpath changes but the file is not rebuilt.
  os << indent << "FILE(RPATH_CHECK\n"
     << indent << "     FILE \"" << toDestDirPath << "\"\n"
     << indent << "     RPATH \"" << newRpath << "\")\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::AddChrpathPatchRule(std::ostream& os, Indent const& indent,
                      const char* config, std::string const& toDestDirPath)
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
  if(newRpath.empty())
    {
    os << indent << "FILE(RPATH_REMOVE\n"
       << indent << "     FILE \"" << toDestDirPath << "\")\n";
    }
  else
    {
    os << indent << "FILE(RPATH_CHANGE\n"
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

  // don't strip static libraries, because it removes the only symbol table
  // they have so you can't link to them anymore
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY)
    {
    return;
    }

  // Don't handle OSX Bundles.
  if(this->Target->GetMakefile()->IsOn("APPLE") &&
     this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
    {
    return;
    }

  if(! this->Target->GetMakefile()->IsSet("CMAKE_STRIP"))
    {
    return;
    }

  os << indent << "IF(CMAKE_INSTALL_DO_STRIP)\n";
  os << indent << "  EXECUTE_PROCESS(COMMAND \""
     << this->Target->GetMakefile()->GetDefinition("CMAKE_STRIP")
     << "\" \"" << toDestDirPath << "\")\n";
  os << indent << "ENDIF(CMAKE_INSTALL_DO_STRIP)\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator::AddRanlibRule(std::ostream& os,
                                        Indent const& indent,
                                        const std::string& toDestDirPath)
{
  // Static libraries need ranlib on this platform.
  if(this->Target->GetType() != cmTarget::STATIC_LIBRARY)
    {
    return;
    }

  // Perform post-installation processing on the file depending
  // on its type.
  if(!this->Target->GetMakefile()->IsOn("APPLE"))
    {
    return;
    }

  std::string ranlib =
    this->Target->GetMakefile()->GetRequiredDefinition("CMAKE_RANLIB");
  if(ranlib.empty())
    {
    return;
    }

  os << indent << "EXECUTE_PROCESS(COMMAND \""
     << ranlib << "\" \"" << toDestDirPath << "\")\n";
}
