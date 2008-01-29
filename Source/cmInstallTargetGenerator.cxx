/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmInstallTargetGenerator.h"

#include "cmComputeLinkInformation.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

// TODO:
//   - Skip IF(EXISTS) checks if nothing is done with the installed file

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(cmTarget& t, const char* dest, bool implib,
                           const char* file_permissions,
                           std::vector<std::string> const& configurations,
                           const char* component, bool optional):
  cmInstallGenerator(dest, configurations, component), Target(&t),
  ImportLibrary(implib), FilePermissions(file_permissions), Optional(optional)
{
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

  // Track indentation.
  Indent indent;

  // Begin this block of installation.
  std::string component_test =
    this->CreateComponentTest(this->Component.c_str());
  os << indent << "IF(" << component_test << ")\n";

  // Compute the build tree directory from which to copy the target.
  std::string fromDir;
  if(this->Target->NeedRelinkBeforeInstall())
    {
    fromDir = this->Target->GetMakefile()->GetStartOutputDirectory();
    fromDir += cmake::GetCMakeFilesDirectory();
    fromDir += "/CMakeRelink.dir/";
    }
  else
    {
    fromDir = this->Target->GetDirectory(0, this->ImportLibrary);
    fromDir += "/";
    }

  // Generate a portion of the script for each configuration.
  if(this->ConfigurationTypes->empty())
    {
    this->GenerateScriptForConfig(os, fromDir.c_str(),
                                  this->ConfigurationName,
                                  indent.Next());
    }
  else
    {
    for(std::vector<std::string>::const_iterator i =
          this->ConfigurationTypes->begin();
        i != this->ConfigurationTypes->end(); ++i)
      {
      this->GenerateScriptForConfig(os, fromDir.c_str(), i->c_str(),
                                    indent.Next());
      }
    }

  // End this block of installation.
  os << indent << "ENDIF(" << component_test << ")\n\n";
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::GenerateScriptForConfig(std::ostream& os,
                                                       const char* fromDir,
                                                       const char* config,
                                                       Indent const& indent)
{
  // Compute the per-configuration directory containing the files.
  std::string fromDirConfig = fromDir;
  this->Target->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()
    ->AppendDirectoryForConfig("", config, "/", fromDirConfig);

  if(config && *config)
    {
    // Skip this configuration for config-specific installation that
    // does not match it.
    if(!this->InstallsForConfig(config))
      {
      return;
      }

    // Generate a per-configuration block.
    std::string config_test = this->CreateConfigTest(config);
    os << indent << "IF(" << config_test << ")\n";
    this->GenerateScriptForConfigDir(os, fromDirConfig.c_str(), config,
                                     indent.Next());
    os << indent << "ENDIF(" << config_test << ")\n";
    }
  else
    {
    this->GenerateScriptForConfigDir(os, fromDirConfig.c_str(), config,
                                     indent);
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::GenerateScriptForConfigDir(std::ostream& os,
                             const char* fromDirConfig,
                             const char* config,
                             Indent const& indent)
{
  // Compute the full path to the main installed file for this target.
  std::string toInstallPath = this->GetInstallDestination();
  toInstallPath += "/";
  toInstallPath += this->GetInstallFilename(this->Target, config,
                                              this->ImportLibrary, false);

  // Compute the list of files to install for this target.
  std::vector<std::string> files;
  std::string literal_args;
  cmTarget::TargetType type = this->Target->GetType();
  if(type == cmTarget::EXECUTABLE)
    {
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
        toInstallPath += this->GetInstallFilename(this->Target, config,
                                                  this->ImportLibrary, false);
        literal_args += " USE_SOURCE_PERMISSIONS";
        }
      else
        {
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
      std::string from1 = fromDirConfig;
      from1 += targetNameImport;
      files.push_back(from1);

      // An import library looks like a static library.
      type = cmTarget::STATIC_LIBRARY;
      }
    else if(this->Target->IsFrameworkOnApple())
      {
      // Compute the build tree location of the framework directory
      std::string from1 = fromDirConfig;
      // Remove trailing slashes... so that from1 ends with ".framework":
      //
      cmSystemTools::ConvertToUnixSlashes(from1);
      files.push_back(from1);

      type = cmTarget::INSTALL_DIRECTORY;

      // Need to apply install_name_tool and stripping to binary
      // inside framework.
      toInstallPath += ".framework/";
      toInstallPath += this->GetInstallFilename(this->Target, config,
                                                this->ImportLibrary, false);

      literal_args += " USE_SOURCE_PERMISSIONS";
      }
    else
      {
      std::string from1 = fromDirConfig;
      from1 += targetName;
      files.push_back(from1);
      if(targetNameSO != targetName)
        {
        std::string from2 = fromDirConfig;
        from2 += targetNameSO;
        files.push_back(from2);
        }
      if(targetNameReal != targetName &&
         targetNameReal != targetNameSO)
        {
        std::string from3 = fromDirConfig;
        from3 += targetNameReal;
        files.push_back(from3);
        }
      }
    }

  // Write code to install the target file.
  const char* no_dir_permissions = 0;
  const char* no_rename = 0;
  const char* no_properties = 0;
  bool optional = this->Optional || this->ImportLibrary;
  this->AddInstallRule(os, type, files,
                       optional, no_properties,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_rename, literal_args.c_str(),
                       indent);

  std::string toDestDirPath = "$ENV{DESTDIR}";
  if(toInstallPath[0] != '/')
    {
    toDestDirPath += "/";
    }
  toDestDirPath += toInstallPath;

  this->Target->SetInstallNameFixupPath(toInstallPath.c_str());

  os << indent << "IF(EXISTS \"" << toDestDirPath << "\")\n";
  this->AddInstallNamePatchRule(os, indent.Next(), config, toDestDirPath);
  this->AddChrpathPatchRule(os, indent.Next(), config, toDestDirPath);
  this->AddRanlibRule(os, indent.Next(), type, toDestDirPath);
  this->AddStripRule(os, indent.Next(), type, toDestDirPath);
  os << indent << "ENDIF(EXISTS \"" << toDestDirPath << "\")\n";
}

//----------------------------------------------------------------------------
std::string
cmInstallTargetGenerator::GetInstallFilename(const char* config) const
{
  return
    cmInstallTargetGenerator::GetInstallFilename(this->Target, config,
                                                 this->ImportLibrary, false);
}

//----------------------------------------------------------------------------
std::string cmInstallTargetGenerator::GetInstallFilename(cmTarget* target,
                                                         const char* config,
                                                         bool implib,
                                                         bool useSOName)
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
    if(implib)
      {
      // Use the import library name.
      fname = targetNameImport;
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
    if(implib)
      {
      // Use the import library name.
      fname = targetNameImport;
      }
    else if(useSOName)
      {
      // Use the soname.
      fname = targetNameSO;
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
      // If the build tree and install tree use different path
      // components of the install_name field then we need to create a
      // mapping to be applied after installation.
      cmTarget* tgt = *j;
      std::string for_build = tgt->GetInstallNameDirForBuildTree(config);
      std::string for_install = tgt->GetInstallNameDirForInstallTree(config);
      std::string fname = this->GetInstallFilename(tgt, config, false, true);

      // Map from the build-tree install_name.
      for_build += fname;

      // Map to the install-tree install_name.
      if (!for_install.empty())
        {
        for_install += fname;
        }
      else
        {
        for_install = tgt->GetInstallNameFixupPath();
        }

      if(for_build != for_install)
        {
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
    std::string fname =
      this->GetInstallFilename(this->Target, config, this->ImportLibrary,
                               true);
    for_build += fname;
    if (!for_install.empty())
      {
      for_install += fname;
      }
    else
      {
      for_install = this->Target->GetInstallNameFixupPath();
      }
    if(for_build != for_install)
      {
      // Prepare to refer to the install-tree install_name.
      new_id = for_install;
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
::AddChrpathPatchRule(std::ostream& os, Indent const& indent,
                      const char* config, std::string const& toDestDirPath)
{
  if(this->ImportLibrary ||
     !(this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
       this->Target->GetType() == cmTarget::MODULE_LIBRARY ||
       this->Target->GetType() == cmTarget::EXECUTABLE))
    {
    return;
    }

  if(!this->Target->IsChrpathUsed())
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

  // Fix the RPATH in installed ELF binaries using chrpath.
  std::string chrpathTool = cli->GetChrpathTool();

  // Write a rule to run chrpath to set the install-tree RPATH
  os << indent << "EXECUTE_PROCESS(COMMAND \"" << chrpathTool;
  os << "\" -r \"" << newRpath << "\" \"" << toDestDirPath << "\")\n";
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
