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
#include "cmExportInstallFileGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallTargetGenerator.h"

//----------------------------------------------------------------------------
cmExportInstallFileGenerator
::cmExportInstallFileGenerator(cmInstallExportGenerator* iegen):
  InstallExportGenerator(iegen)
{
}

//----------------------------------------------------------------------------
bool cmExportInstallFileGenerator::GenerateMainFile(std::ostream& os)
{
  // Create all the imported targets.
  for(std::vector<cmTargetExport*>::const_iterator
        tei = this->ExportSet->begin();
      tei != this->ExportSet->end(); ++tei)
    {
    cmTargetExport* te = *tei;
    this->ExportedTargets.insert(te->Target);
    this->GenerateImportTargetCode(os, te->Target);
    }

  // Now load per-configuration properties for them.
  os << "# Load information for each installed configuration.\n"
     << "GET_FILENAME_COMPONENT(_DIR \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
     << "FILE(GLOB CONFIG_FILES \"${_DIR}/"
     << this->FileBase << "-*" << this->FileExt << "\")\n"
     << "FOREACH(f ${CONFIG_FILES})\n"
     << "  INCLUDE(${f})\n"
     << "ENDFOREACH(f)\n"
     << "\n";

  // Generate an import file for each configuration.
  bool result = true;
  for(std::vector<std::string>::const_iterator
        ci = this->Configurations.begin();
      ci != this->Configurations.end(); ++ci)
    {
    if(!this->GenerateImportFileConfig(ci->c_str()))
      {
      result = false;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
bool
cmExportInstallFileGenerator::GenerateImportFileConfig(const char* config)
{
  // Skip configurations not enabled for this export.
  if(!this->InstallExportGenerator->InstallsForConfig(config))
    {
    return true;
    }

  // Construct the name of the file to generate.
  std::string fileName = this->FileDir;
  fileName += "/";
  fileName += this->FileBase;
  fileName += "-";
  if(config && *config)
    {
    fileName += cmSystemTools::LowerCase(config);
    }
  else
    {
    fileName += "noconfig";
    }
  fileName += this->FileExt;

  // Open the output file to generate it.
  cmGeneratedFileStream exportFileStream(fileName.c_str(), true);
  if(!exportFileStream)
    {
    std::string se = cmSystemTools::GetLastSystemError();
    cmOStringStream e;
    e << "cannot write to file \"" << fileName.c_str()
      << "\": " << se;
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  std::ostream& os = exportFileStream;

  // Start with the import file header.
  this->GenerateImportHeaderCode(os, config);

  // Generate the per-config target information.
  this->GenerateImportConfig(os, config);

  // End with the import file footer.
  this->GenerateImportFooterCode(os);

  // Record this per-config import file.
  this->ConfigImportFiles[config] = fileName;

  return true;
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::GenerateImportTargetsConfig(std::ostream& os,
                              const char* config, std::string const& suffix)
{
  // Add code to compute the installation prefix relative to the
  // import file location.
  const char* installDest = this->InstallExportGenerator->GetDestination();
  if(!cmSystemTools::FileIsFullPath(installDest))
    {
    std::string dest = installDest;
    os << "# Compute the installation prefix relative to this file.\n"
       << "GET_FILENAME_COMPONENT(_IMPORT_PREFIX "
       << "\"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n";
    while(!dest.empty())
      {
      os <<
        "GET_FILENAME_COMPONENT(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" PATH)\n";
      dest = cmSystemTools::GetFilenamePath(dest);
      }
    os << "\n";

    // Import location properties may reference this variable.
    this->ImportPrefix = "${_IMPORT_PREFIX}/";
    }

  // Add each target in the set to the export.
  for(std::vector<cmTargetExport*>::const_iterator
        tei = this->ExportSet->begin();
      tei != this->ExportSet->end(); ++tei)
    {
    // Collect import properties for this target.
    cmTargetExport* te = *tei;
    ImportPropertyMap properties;
    this->SetImportLocationProperty(config, suffix,
                                    te->ArchiveGenerator, properties);
    this->SetImportLocationProperty(config, suffix,
                                    te->LibraryGenerator, properties);
    this->SetImportLocationProperty(config, suffix,
                                    te->RuntimeGenerator, properties);
    this->SetImportLocationProperty(config, suffix,
                                    te->FrameworkGenerator, properties);
    this->SetImportLocationProperty(config, suffix,
                                    te->BundleGenerator, properties);

    // If any file location was set for the target add it to the
    // import file.
    if(!properties.empty())
      {
      // Get the rest of the target details.
      this->SetImportDetailProperties(config, suffix,
                                      te->Target, properties);

      // TOOD: PUBLIC_HEADER_LOCATION
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, te->Target, properties);
      }
    }

  // Cleanup the import prefix variable.
  if(!this->ImportPrefix.empty())
    {
    os << "# Cleanup temporary variables.\n"
       << "SET(_IMPORT_PREFIX)\n"
       << "\n";
    }
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::SetImportLocationProperty(const char* config, std::string const& suffix,
                            cmInstallTargetGenerator* itgen,
                            ImportPropertyMap& properties)
{
  // Skip rules that do not match this configuration.
  if(!(itgen && itgen->InstallsForConfig(config)))
    {
    return;
    }

  {
  // Construct the property name.
  std::string prop = (itgen->IsImportLibrary()?
                      "IMPORTED_IMPLIB" : "IMPORTED_LOCATION");
  prop += suffix;

  // Construct the installed location of the target.
  std::string dest = itgen->GetDestination();
  std::string value;
  if(!cmSystemTools::FileIsFullPath(dest.c_str()))
    {
    // The target is installed relative to the installation prefix.
    if(this->ImportPrefix.empty())
      {
      this->ComplainAboutImportPrefix(itgen);
      }
    value = this->ImportPrefix;
    }
  value += dest;
  value += "/";

  // Append the installed file name.
  std::string fname = itgen->GetInstallFilename(config);
  value += fname;

  // Fix name for frameworks and bundles.
  if(itgen->GetTarget()->IsFrameworkOnApple())
    {
    value += ".framework/";
    value += fname;
    }
  else if(itgen->GetTarget()->IsAppBundleOnApple())
    {
    value += ".app/Contents/MacOS/";
    value += fname;
    }

  // Store the property.
  properties[prop] = value;
  }
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::ComplainAboutImportPrefix(cmInstallTargetGenerator* itgen)
{
  const char* installDest = this->InstallExportGenerator->GetDestination();
  cmOStringStream e;
  e << "INSTALL(EXPORT \"" << this->Name << "\") given absolute "
    << "DESTINATION \"" << installDest << "\" but the export "
    << "references an installation of target \""
    << itgen->GetTarget()->GetName() << "\" which has relative "
    << "DESTINATION \"" << itgen->GetDestination() << "\".";
  cmSystemTools::Error(e.str().c_str());
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::ComplainAboutMissingTarget(cmTarget* target, const char* dep)
{
  cmOStringStream e;
  e << "WARNING: INSTALL(EXPORT \"" << this->Name << "\" ...) "
    << "includes target " << target->GetName()
    << " which links to target \"" << dep
    << "\" that is not in the export set.";
  cmSystemTools::Message(e.str().c_str());
}
