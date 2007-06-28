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

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

// TODO:
//   - Fix indentation of generated code
//   - Consolidate component/configuration checks across multiple
//     install generators
//   - Skip IF(EXISTS) checks if nothing is done with the installed file

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(cmTarget& t, const char* dest, bool implib,
                           const char* file_permissions,
                           std::vector<std::string> const& configurations,
                           const char* component, bool optional):
  cmInstallGenerator(dest), Target(&t), ImportLibrary(implib),
  FilePermissions(file_permissions), Configurations(configurations),
  Component(component), Optional(optional)
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
  // Begin this block of installation.
  std::string component_test = "NOT CMAKE_INSTALL_COMPONENT OR "
    "\"${CMAKE_INSTALL_COMPONENT}\" MATCHES \"^(";
  component_test += this->Component;
  component_test += ")$\"";
  os << "IF(" << component_test << ")\n";

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
                                  this->ConfigurationName);
    }
  else
    {
    for(std::vector<std::string>::const_iterator i =
          this->ConfigurationTypes->begin();
        i != this->ConfigurationTypes->end(); ++i)
      {
      this->GenerateScriptForConfig(os, fromDir.c_str(), i->c_str());
      }
    }

  // End this block of installation.
  os << "ENDIF(" << component_test << ")\n";
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::GenerateScriptForConfig(std::ostream& os,
                                                       const char* fromDir,
                                                       const char* config)
{
  // Compute the per-configuration directory containing the files.
  std::string fromDirConfig = fromDir;
  this->Target->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()
    ->AppendDirectoryForConfig("", config, "/", fromDirConfig);

  std::string config_test;
  if(config && *config)
    {
    std::string config_upper = cmSystemTools::UpperCase(config);
    // Skip this configuration for config-specific installation that
    // does not match it.
    if(!this->Configurations.empty())
      {
      bool found = false;
      for(std::vector<std::string>::const_iterator i =
            this->Configurations.begin();
          !found && i != this->Configurations.end(); ++i)
        {
        found = found || (cmSystemTools::UpperCase(*i) == config_upper);
        }
      if(!found)
        {
        return;
        }
      }

    // Begin this configuration block.
    config_test = "\"${CMAKE_INSTALL_CONFIG_NAME}\" MATCHES \"^(";
    config_test += config;
    config_test += ")$\"";
    os << "  IF(" << config_test << ")\n";
    }

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
      if(this->Target->GetMakefile()->IsOn("APPLE") &&
         this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        // Compute the source locations of the bundle executable and
        // Info.plist file.
        from1 += ".app";
        files.push_back(from1);
        type = cmTarget::INSTALL_DIRECTORY;
        literal_args += " USE_SOURCE_PERMISSIONS";
        // TODO: Still need to apply install_name_tool and stripping
        // to binaries inside bundle.
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
  const char* no_component = 0;
  std::vector<std::string> no_configurations;
  bool optional = this->Optional || this->ImportLibrary;
  this->AddInstallRule(os, this->Destination.c_str(), type, files,
                       optional, no_properties,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       no_configurations, no_component,
                       no_rename, literal_args.c_str());

  std::string toFullPath = "$ENV{DESTDIR}";
  toFullPath += this->Destination;
  toFullPath += "/";
  toFullPath += this->GetInstallFilename(this->Target, config,
                                         this->ImportLibrary, false);

  os << "    IF(EXISTS \"" << toFullPath << "\")\n";
  this->AddInstallNamePatchRule(os, config, toFullPath);
  this->AddRanlibRule(os, type, toFullPath);
  this->AddStripRule(os, type, toFullPath);
  os << "    ENDIF(EXISTS \"" << toFullPath << "\")\n";

  if(config && *config)
    {
    // End this configuration block.
    os << "  ENDIF(" << config_test << ")\n";
    }
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
::AddInstallNamePatchRule(std::ostream& os, const char* config,
                          std::string const& toFullPath)
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
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(config && cmSystemTools::UpperCase(config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }
  // TODO: Merge with ComputeLinkInformation.
  const cmTarget::LinkLibraryVectorType& inLibs = 
    this->Target->GetLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator j = inLibs.begin();
      j != inLibs.end(); ++j)
    {
    std::string lib = j->first;
    if((this->Target->GetType() == cmTarget::EXECUTABLE ||
        lib != this->Target->GetName()) &&
       (j->second == cmTarget::GENERAL || j->second == linkType))
      {
      if(cmTarget* tgt = this->Target->GetMakefile()->
         GetLocalGenerator()->GetGlobalGenerator()->
         FindTarget(0, lib.c_str(), false))
        {
        if(tgt->GetType() == cmTarget::SHARED_LIBRARY)
          {
          // If the build tree and install tree use different path
          // components of the install_name field then we need to create a
          // mapping to be applied after installation.
          std::string for_build = tgt->GetInstallNameDirForBuildTree(config);
          std::string for_install = 
            tgt->GetInstallNameDirForInstallTree(config);
          if(for_build != for_install)
            {
            std::string fname =
              this->GetInstallFilename(tgt, config, false, true);
            // Map from the build-tree install_name.
            for_build += fname;

            // Map to the install-tree install_name.
            for_install += fname;

            // Store the mapping entry.
            install_name_remap[for_build] = for_install;
            }
          }
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
    if(for_build != for_install)
      {
      // Prepare to refer to the install-tree install_name.
      new_id = for_install;
      new_id += this->GetInstallFilename(this->Target, config,
                                         this->ImportLibrary, true);
      }
    }

  // Write a rule to run install_name_tool to set the install-tree
  // install_name value and references.
  if(!new_id.empty() || !install_name_remap.empty())
    {
    os << "      EXECUTE_PROCESS(COMMAND \"" << installNameTool;
    os << "\"";
    if(!new_id.empty())
      {
      os << "\n        -id \"" << new_id << "\"";
      }
    for(std::map<cmStdString, cmStdString>::const_iterator
          i = install_name_remap.begin();
        i != install_name_remap.end(); ++i)
      {
      os << "\n        -change \"" << i->first << "\" \"" << i->second << "\"";
      }
    os << "\n        \"" << toFullPath << "\")\n";
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator::AddStripRule(std::ostream& os,
                                       cmTarget::TargetType type,
                                       const std::string& toFullPath)
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

  os << "      IF(CMAKE_INSTALL_DO_STRIP)\n";
  os << "        EXECUTE_PROCESS(COMMAND \""
     << this->Target->GetMakefile()->GetDefinition("CMAKE_STRIP")
     << "\" \"" << toFullPath << "\")\n";
  os << "      ENDIF(CMAKE_INSTALL_DO_STRIP)\n";
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator::AddRanlibRule(std::ostream& os,
                                        cmTarget::TargetType type,
                                        const std::string& toFullPath)
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

  os << "      EXECUTE_PROCESS(COMMAND \""
     << ranlib << "\" \"" << toFullPath << "\")\n";
}
