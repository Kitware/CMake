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

  // Compute the full path to the main installed file for this target.
  NameType nameType = this->ImportLibrary? NameImplib : NameNormal;
  std::string toInstallPath = this->GetInstallDestination();
  toInstallPath += "/";
  toInstallPath += this->GetInstallFilename(this->Target, config, nameType);

  // Track whether post-install operations should be added to the
  // script.
  bool tweakInstalledFile = true;

  // Compute the list of files to install for this target.
  std::vector<std::string> files;
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
      std::string from1 = fromDirConfig;
      from1 += targetNameImport;
      files.push_back(from1);

      // An import library looks like a static library.
      type = cmTarget::STATIC_LIBRARY;
      }
    else
      {
      std::string from1 = fromDirConfig;
      from1 += targetName;

      // Handle OSX Bundles.
      if(this->Target->IsAppBundleOnApple())
        {
        // Compute the source locations of the bundle executable and
        // Info.plist file.
        from1 += ".app";
        files.push_back(from1);
        type = cmTarget::INSTALL_DIRECTORY;
        // Need to apply install_name_tool and stripping to binary
        // inside bundle.
        toInstallPath += ".app/Contents/MacOS/";
        toInstallPath +=
          this->GetInstallFilename(this->Target, config, nameType);
        literal_args += " USE_SOURCE_PERMISSIONS";
        }
      else
        {
        // Operations done at install time on the installed file should
        // be done on the real file and not any of the symlinks.
        toInstallPath = this->GetInstallDestination();
        toInstallPath += "/";
        toInstallPath += targetNameReal;

        files.push_back(from1);
        if(targetNameReal != targetName)
          {
          std::string from2 = fromDirConfig;
          from2 += targetNameReal;
          files.push_back(from2);
          }
        }
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

      std::string from1 = fromDirConfig;
      from1 += targetNameImport;
      files.push_back(from1);

      // An import library looks like a static library.
      type = cmTarget::STATIC_LIBRARY;
      }
    else if(this->Target->IsFrameworkOnApple())
      {
      // There is a bug in cmInstallCommand if this fails.
      assert(this->NamelinkMode == NamelinkModeNone);

      // Compute the build tree location of the framework directory
      std::string from1 = fromDirConfig;
      from1 += targetName;
      from1 += ".framework";
      files.push_back(from1);

      type = cmTarget::INSTALL_DIRECTORY;

      // Need to apply install_name_tool and stripping to binary
      // inside framework.
      toInstallPath += ".framework/Versions/";
      toInstallPath += this->Target->GetFrameworkVersion();
      toInstallPath += "/";
      toInstallPath += this->GetInstallFilename(this->Target, config,
                                                NameNormal);

      literal_args += " USE_SOURCE_PERMISSIONS";
      }
    else
      {
      // Operations done at install time on the installed file should
      // be done on the real file and not any of the symlinks.
      toInstallPath = this->GetInstallDestination();
      toInstallPath += "/";
      toInstallPath += targetNameReal;

      // Construct the list of file names to install for this library.
      bool haveNamelink = false;
      std::string fromName;
      std::string fromSOName;
      std::string fromRealName;
      fromName = fromDirConfig;
      fromName += targetName;
      if(targetNameSO != targetName)
        {
        haveNamelink = true;
        fromSOName = fromDirConfig;
        fromSOName += targetNameSO;
        }
      if(targetNameReal != targetName &&
         targetNameReal != targetNameSO)
        {
        haveNamelink = true;
        fromRealName = fromDirConfig;
        fromRealName += targetNameReal;
        }

      // Add the names based on the current namelink mode.
      if(haveNamelink)
        {
        // With a namelink we need to check the mode.
        if(this->NamelinkMode == NamelinkModeOnly)
          {
          // Install the namelink only.
          files.push_back(fromName);
          tweakInstalledFile = false;
          }
        else
          {
          // Install the real file if it has its own name.
          if(!fromRealName.empty())
            {
            files.push_back(fromRealName);
            }

          // Install the soname link if it has its own name.
          if(!fromSOName.empty())
            {
            files.push_back(fromSOName);
            }

          // Install the namelink if it is not to be skipped.
          if(this->NamelinkMode != NamelinkModeSkip)
            {
            files.push_back(fromName);
            }
          }
        }
      else
        {
        // Without a namelink there will be only one file.  Install it
        // if this is not a namelink-only rule.
        if(this->NamelinkMode != NamelinkModeOnly)
          {
          files.push_back(fromName);
          }
        }
      }
    }

  // Skip this rule if no files are to be installed for the target.
  if(files.empty())
    {
    return;
    }

  // Construct the path of the file on disk after installation on
  // which tweaks may be performed.
  std::string toDestDirPath = "$ENV{DESTDIR}";
  if(toInstallPath[0] != '/' && toInstallPath[0] != '$')
    {
    toDestDirPath += "/";
    }
  toDestDirPath += toInstallPath;

  // Add pre-installation tweaks.
  if(tweakInstalledFile)
    {
    // Collect tweaking rules.
    cmOStringStream tw;
    this->AddRPathCheckRule(tw, indent.Next(), config, toDestDirPath);
    std::string tws = tw.str();

    // Add the rules, if any.
    if(!tws.empty())
      {
      os << indent << "IF(EXISTS \"" << toDestDirPath << "\")\n";
      os << tws;
      os << indent << "ENDIF(EXISTS \"" << toDestDirPath << "\")\n";
      }
    }

  // Write code to install the target file.
  const char* no_dir_permissions = 0;
  const char* no_rename = 0;
  bool optional = this->Optional || this->ImportLibrary;
  this->AddInstallRule(os, type, files,
                       optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_rename, literal_args.c_str(),
                       indent);

  // Add post-installation tweaks.
  if(tweakInstalledFile)
    {
    // Collect tweaking rules.
    cmOStringStream tw;
    this->AddInstallNamePatchRule(tw, indent.Next(), config, toDestDirPath);
    this->AddChrpathPatchRule(tw, indent.Next(), config, toDestDirPath);
    this->AddRanlibRule(tw, indent.Next(), type, toDestDirPath);
    this->AddStripRule(tw, indent.Next(), type, toDestDirPath);
    std::string tws = tw.str();

    // Add the rules, if any.
    if(!tws.empty())
      {
      os << indent << "IF(EXISTS \"" << toDestDirPath << "\")\n";
      os << tws;
      os << indent << "ENDIF(EXISTS \"" << toDestDirPath << "\")\n";
      }
    }
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
                                       cmTarget::TargetType type,
                                       const std::string& toDestDirPath)
{

  // don't strip static libraries, because it removes the only symbol table
  // they have so you can't link to them anymore
  if(type == cmTarget::STATIC_LIBRARY)
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
                                        cmTarget::TargetType type,
                                        const std::string& toDestDirPath)
{
  // Static libraries need ranlib on this platform.
  if(type != cmTarget::STATIC_LIBRARY)
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
