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
#include "cmTarget.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(cmTarget& t, const char* dest, bool implib,
                           const char* file_permissions,
                           std::vector<std::string> const& configurations,
                           const char* component, bool optional):
  Target(&t), Destination(dest), ImportLibrary(implib),
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
    fromDir = this->Target->GetDirectory();
    fromDir += "/";
    }

  // Write variable settings to do per-configuration references.
  this->PrepareScriptReference(os, this->Target, "BUILD", true, false);

  // Create the per-configuration reference.
  std::string fromName = this->GetScriptReference(this->Target, "BUILD",
                                                  false);
  std::string fromFile = fromDir;
  fromFile += fromName;

  // Choose the final destination.  This may be modified for certain
  // target types.
  std::string destination = this->Destination;

  // Setup special properties for some target types.
  std::string literal_args;
  std::string props;
  const char* properties = 0;
  cmTarget::TargetType type = this->Target->GetType();
  switch(type)
    {
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      {
      // Add shared library installation properties if this platform
      // supports them.
      const char* lib_version = 0;
      const char* lib_soversion = 0;

        // Versioning is supported only for shared libraries and modules,
        // and then only when the platform supports an soname flag.
      cmGlobalGenerator* gg =
        (this->Target->GetMakefile()
         ->GetLocalGenerator()->GetGlobalGenerator());
      if(const char* linkLanguage = this->Target->GetLinkerLanguage(gg))
        {
        std::string sonameFlagVar = "CMAKE_SHARED_LIBRARY_SONAME_";
        sonameFlagVar += linkLanguage;
        sonameFlagVar += "_FLAG";
        if(this->Target->GetMakefile()->GetDefinition(sonameFlagVar.c_str()))
          {
          lib_version = this->Target->GetProperty("VERSION");
          lib_soversion = this->Target->GetProperty("SOVERSION");
        }
        }

      if(lib_version)
        {
        props += " VERSION ";
        props += lib_version;
        }
      if(lib_soversion)
        {
        props += " SOVERSION ";
        props += lib_soversion;
        }
      properties = props.c_str();
      }
      break;
    case cmTarget::EXECUTABLE:
      {
      // Add executable installation properties if this platform
      // supports them.
#if defined(_WIN32) && !defined(__CYGWIN__)
      const char* exe_version = 0;
#else
      const char* exe_version = this->Target->GetProperty("VERSION");
#endif
      if(exe_version)
        {
        props += " VERSION ";
        props += exe_version;
        properties = props.c_str();
        }

      // Handle OSX Bundles.
      if(this->Target->GetMakefile()->IsOn("APPLE") &&
         this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        // Compute the source locations of the bundle executable and
        // Info.plist file.
        this->PrepareScriptReference(os, this->Target, "INSTALL",
                                     false, false);
        fromFile += ".app";
        type = cmTarget::INSTALL_DIRECTORY;
        literal_args += " USE_SOURCE_PERMISSIONS";
        }
      }
      break;
    case cmTarget::STATIC_LIBRARY:
      // Nothing special for static libraries.
      break;
    default:
      break;
    }

  // An import library looks like a static library.
  if(this->ImportLibrary)
    {
    type = cmTarget::STATIC_LIBRARY;
    }

  // Write code to install the target file.
  const char* no_dir_permissions = 0;
  const char* no_rename = 0;
  bool optional = this->Optional | this->ImportLibrary;
  this->AddInstallRule(os, destination.c_str(), type, fromFile.c_str(),
                       optional, properties,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       this->Configurations,
                       this->Component.c_str(),
                       no_rename, literal_args.c_str());

  // Fix the install_name settings in installed binaries.
  if(type == cmTarget::SHARED_LIBRARY ||
     type == cmTarget::MODULE_LIBRARY ||
     type == cmTarget::EXECUTABLE)
    {
    this->AddInstallNamePatchRule(os, destination.c_str());
    }
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::PrepareScriptReference(std::ostream& os, cmTarget* target,
                         const char* place, bool useConfigDir,
                         bool useSOName)
{
  // If the target name may vary with the configuration type then
  // store all possible names ahead of time in variables.
  std::string fname;
  for(std::vector<std::string>::const_iterator i =
        this->ConfigurationTypes->begin();
      i != this->ConfigurationTypes->end(); ++i)
    {
    // Initialize the name.
    fname = "";

    if(useConfigDir)
      {
      // Start with the configuration's subdirectory.
      target->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()->
        AppendDirectoryForConfig("", i->c_str(), "/", fname);
      }

    // Compute the name of the library.
    std::string targetName;
    std::string targetNameSO;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    target->GetLibraryNames(targetName, targetNameSO, targetNameReal,
                            targetNameImport, targetNamePDB, i->c_str());
    if(this->ImportLibrary)
      {
      // Use the import library name.
      fname += targetNameImport;
      }
    else if(useSOName)
      {
      // Use the soname.
      fname += targetNameSO;
      }
    else
      {
      // Use the canonical name.
      fname += targetName;
      }

    // Set a variable with the target name for this configuration.
    os << "SET(" << target->GetName() << "_" << place
       << (this->ImportLibrary? "_IMPNAME_" : "_NAME_") << *i
       << " \"" << fname << "\")\n";
    }
}

//----------------------------------------------------------------------------
std::string cmInstallTargetGenerator::GetScriptReference(cmTarget* target,
                                                         const char* place,
                                                         bool useSOName)
{
  if(this->ConfigurationTypes->empty())
    {
    // Reference the target by its one configuration name.
    std::string targetName;
    std::string targetNameSO;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    target->GetLibraryNames(targetName, targetNameSO, targetNameReal,
                            targetNameImport, targetNamePDB,
                            this->ConfigurationName);
    if(this->ImportLibrary)
      {
      // Use the import library name.
      return targetNameImport;
      }
    else if(useSOName)
      {
      // Use the soname.
      return targetNameSO;
      }
    else
      {
      // Use the canonical name.
      return targetName;
      }
    }
  else
    {
    // Reference the target using the per-configuration variable.
    std::string ref = "${";
    ref += target->GetName();
    if(this->ImportLibrary)
      {
      ref += "_";
      ref += place;
      ref += "_IMPNAME_";
      }
    else
      {
      ref += "_";
      ref += place;
      ref += "_NAME_";
      }
    ref += "${CMAKE_INSTALL_CONFIG_NAME}}";
    return ref;
    }
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator
::AddInstallNamePatchRule(std::ostream& os,
                                                       const char* destination)
{
  // Build a map of build-tree install_name to install-tree install_name for
  // shared libraries linked to this target.
  std::map<cmStdString, cmStdString> install_name_remap;
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  const char* config = this->ConfigurationName;
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
         FindTarget(0, lib.c_str()))
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
            // Map from the build-tree install_name.
            this->PrepareScriptReference(os, tgt, "REMAP_FROM",
                                         !for_build.empty(), true);
            for_build += this->GetScriptReference(tgt, "REMAP_FROM", true);

            // Map to the install-tree install_name.
            this->PrepareScriptReference(os, tgt, "REMAP_TO",
                                         false, true);
            for_install += this->GetScriptReference(tgt, "REMAP_TO", true);

            // Store the mapping entry.
            install_name_remap[for_build] = for_install;
            }
          }
        }
      }
    }

  // Edit the install_name of the target itself if necessary.
  this->PrepareScriptReference(os, this->Target, "REMAPPED", false, true);
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
      new_id += this->GetScriptReference(this->Target, "REMAPPED", true);
      }
    }

  // Write a rule to run install_name_tool to set the install-tree
  // install_name value and references.
  if(!new_id.empty() || !install_name_remap.empty())
    {
    std::string component_test = "NOT CMAKE_INSTALL_COMPONENT OR "
      "\"${CMAKE_INSTALL_COMPONENT}\" MATCHES \"^(";
    component_test += this->Component;
    component_test += ")$\"";
    os << "IF(" << component_test << ")\n";
    os << "  EXECUTE_PROCESS(COMMAND install_name_tool";
    if(!new_id.empty())
      {
      os << "\n    -id \"" << new_id << "\"";
      }
    for(std::map<cmStdString, cmStdString>::const_iterator
          i = install_name_remap.begin();
        i != install_name_remap.end(); ++i)
      {
      os << "\n    -change \"" << i->first << "\" \"" << i->second << "\"";
      }
    os << "\n    \"$ENV{DESTDIR}" << destination << "/"
       << this->GetScriptReference(this->Target, "REMAPPED", true) << "\")\n";
    os << "ENDIF(" << component_test << ")\n";
    }
}
