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
#include "cmExportFileGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
void cmExportFileGenerator::AddConfiguration(const char* config)
{
  this->Configurations.push_back(config);
}

//----------------------------------------------------------------------------
void cmExportFileGenerator::SetExportFile(const char* mainFile)
{
  this->MainImportFile = mainFile;
  this->FileDir =
    cmSystemTools::GetFilenamePath(this->MainImportFile);
  this->FileBase =
    cmSystemTools::GetFilenameWithoutLastExtension(this->MainImportFile);
  this->FileExt =
    cmSystemTools::GetFilenameLastExtension(this->MainImportFile);
}

//----------------------------------------------------------------------------
bool cmExportFileGenerator::GenerateImportFile()
{
  // Open the output file to generate it.
  cmGeneratedFileStream exportFileStream(this->MainImportFile.c_str(), true);
  if(!exportFileStream)
    {
    std::string se = cmSystemTools::GetLastSystemError();
    cmOStringStream e;
    e << "cannot write to file \"" << this->MainImportFile
      << "\": " << se;
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  std::ostream& os = exportFileStream;

  // Start with the import file header.
  this->GenerateImportHeaderCode(os);

  // Create all the imported targets.
  bool result = this->GenerateMainFile(os);

  // End with the import file footer.
  this->GenerateImportFooterCode(os);

  return result;
}

//----------------------------------------------------------------------------
void cmExportFileGenerator::GenerateImportConfig(std::ostream& os,
                                                 const char* config)
{
  // Construct the property configuration suffix.
  std::string suffix = "_";
  if(config && *config)
    {
    suffix += cmSystemTools::UpperCase(config);
    }
  else
    {
    suffix += "NOCONFIG";
    }

  // Generate the per-config target information.
  this->GenerateImportTargetsConfig(os, config, suffix);
}

//----------------------------------------------------------------------------
void
cmExportFileGenerator
::SetImportDetailProperties(const char* config, std::string const& suffix,
                            cmTarget* target, ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->GetMakefile();

  // Add the soname for unix shared libraries.
  if(target->GetType() == cmTarget::SHARED_LIBRARY ||
     target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    // Check whether this is a DLL platform.
    bool dll_platform =
      (mf->IsOn("WIN32") || mf->IsOn("CYGWIN") || mf->IsOn("MINGW"));
    if(!dll_platform)
      {
      std::string soname = target->GetSOName(config);
      std::string prop = "IMPORTED_SONAME";
      prop += suffix;
      properties[prop] = soname;
      }
    }

  // Add the transitive link dependencies for this configuration.
  {
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(config && cmSystemTools::UpperCase(config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }

  // Construct the property value.
  cmTarget::LinkLibraryVectorType const& libs =
    target->GetOriginalLinkLibraries();
  std::string link_libs;
  const char* sep = "";
  for(cmTarget::LinkLibraryVectorType::const_iterator li = libs.begin();
      li != libs.end(); ++li)
    {
    // Skip entries that will resolve to the target itself, are empty,
    // or are not meant for this configuration.
    if(li->first == target->GetName() || li->first.empty() ||
       !(li->second == cmTarget::GENERAL || li->second == linkType))
      {
      continue;
      }

    // Separate this from the previous entry.
    link_libs += sep;
    sep = ";";

    // Append this entry.
    if(cmTarget* tgt = mf->FindTargetToUse(li->first.c_str()))
      {
      // This is a target.  Make sure it is included in the export.
      if(this->ExportedTargets.find(tgt) != this->ExportedTargets.end())
        {
        // The target is in the export.  Append it with the export
        // namespace.
        link_libs += this->Namespace;
        link_libs += li->first;
        }
      else
        {
        // The target is not in the export.  This is probably
        // user-error.  Warn but add it anyway.
        this->ComplainAboutMissingTarget(target, li->first.c_str());
        link_libs += li->first;
        }
      }
    else
      {
      // Append the raw name.
      link_libs += li->first;
      }
    }

  // Store the property.
  std::string prop = "IMPORTED_LINK_LIBRARIES";
  prop += suffix;
  properties[prop] = link_libs;
  }
}

//----------------------------------------------------------------------------
void cmExportFileGenerator::GenerateImportHeaderCode(std::ostream& os,
                                                     const char* config)
{
  os << "#----------------------------------------------------------------\n"
     << "# Generated CMake target import file";
  if(config)
    {
    os << " for configuration \"" << config << "\".\n";
    }
  else
    {
    os << ".\n";
    }
  os << "#----------------------------------------------------------------\n"
     << "\n";
  this->GenerateImportVersionCode(os);
}

//----------------------------------------------------------------------------
void cmExportFileGenerator::GenerateImportFooterCode(std::ostream& os)
{
  os << "# Commands beyond this point should not need to know the version.\n"
     << "SET(CMAKE_IMPORT_FILE_VERSION)\n";
}

//----------------------------------------------------------------------------
void cmExportFileGenerator::GenerateImportVersionCode(std::ostream& os)
{
  // Store an import file format version.  This will let us change the
  // format later while still allowing old import files to work.
  os << "# Commands may need to know the format version.\n"
     << "SET(CMAKE_IMPORT_FILE_VERSION 1)\n"
     << "\n";
}

//----------------------------------------------------------------------------
void
cmExportFileGenerator
::GenerateImportTargetCode(std::ostream& os, cmTarget* target)
{
  // Construct the imported target name.
  std::string targetName = this->Namespace;
  targetName += target->GetName();

  // Create the imported target.
  os << "# Create imported target " << targetName << "\n";
  switch(target->GetType())
    {
    case cmTarget::EXECUTABLE:
      os << "ADD_EXECUTABLE(" << targetName << " IMPORTED)\n";
      break;
    case cmTarget::STATIC_LIBRARY:
      os << "ADD_LIBRARY(" << targetName << " STATIC IMPORTED)\n";
      break;
    case cmTarget::SHARED_LIBRARY:
      os << "ADD_LIBRARY(" << targetName << " SHARED IMPORTED)\n";
      break;
    case cmTarget::MODULE_LIBRARY:
      os << "ADD_LIBRARY(" << targetName << " MODULE IMPORTED)\n";
      break;
    default:  // should never happen
      break;
    }
  if(target->IsExecutableWithExports())
    {
    os << "SET_PROPERTY(TARGET " << targetName
       << " PROPERTY IMPORTED_ENABLE_EXPORTS 1)\n";
    }
  os << "\n";
}

//----------------------------------------------------------------------------
void
cmExportFileGenerator
::GenerateImportPropertyCode(std::ostream& os, const char* config,
                             cmTarget* target,
                             ImportPropertyMap const& properties)
{
  // Construct the imported target name.
  std::string targetName = this->Namespace;
  targetName += target->GetName();

  // Set the import properties.
  os << "# Import target \"" << targetName << "\" for configuration \""
     << config << "\"\n";
  os << "SET_PROPERTY(TARGET " << targetName
     << " APPEND PROPERTY IMPORTED_CONFIGURATIONS ";
  if(config && *config)
    {
    os << cmSystemTools::UpperCase(config);
    }
  else
    {
    os << "NOCONFIG";
    }
  os << ")\n";
  os << "SET_TARGET_PROPERTIES(" << targetName << " PROPERTIES\n";
  for(ImportPropertyMap::const_iterator pi = properties.begin();
      pi != properties.end(); ++pi)
    {
    os << "  " << pi->first << " \"" << pi->second << "\"\n";
    }
  os << "  )\n"
     << "\n";
}
