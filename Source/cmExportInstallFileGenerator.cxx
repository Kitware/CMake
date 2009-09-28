/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
std::string cmExportInstallFileGenerator::GetConfigImportFileGlob()
{
  std::string glob = this->FileBase;
  glob += "-*";
  glob += this->FileExt;
  return glob;
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
    if(this->ExportedTargets.insert(te->Target).second)
      {
      this->GenerateImportTargetCode(os, te->Target);
      }
    else
      {
      cmOStringStream e;
      e << "INSTALL(EXPORT \"" << this->Name << "\" ...) "
        << "includes target \"" << te->Target->GetName()
        << "\" more than once in the export set.";
      cmSystemTools::Error(e.str().c_str());
      return false;
      }
    }

  // Now load per-configuration properties for them.
  os << "# Load information for each installed configuration.\n"
     << "GET_FILENAME_COMPONENT(_DIR \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
     << "FILE(GLOB CONFIG_FILES \"${_DIR}/"
     << this->GetConfigImportFileGlob() << "\")\n"
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
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
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

  // Get the target to be installed.
  cmTarget* target = itgen->GetTarget();

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

  if(itgen->IsImportLibrary())
    {
    // Construct the property name.
    std::string prop = "IMPORTED_IMPLIB";
    prop += suffix;

    // Append the installed file name.
    value += itgen->GetInstallFilename(target, config,
                                       cmInstallTargetGenerator::NameImplib);

    // Store the property.
    properties[prop] = value;
    }
  else
    {
    // Construct the property name.
    std::string prop = "IMPORTED_LOCATION";
    prop += suffix;

    // Append the installed file name.
    if(target->IsFrameworkOnApple())
      {
      value += itgen->GetInstallFilename(target, config);
      value += ".framework/";
      value += itgen->GetInstallFilename(target, config);
      }
    else if(target->IsAppBundleOnApple())
      {
      value += itgen->GetInstallFilename(target, config);
      value += ".app/Contents/MacOS/";
      value += itgen->GetInstallFilename(target, config);
      }
    else
      {
      value += itgen->GetInstallFilename(target, config,
                                         cmInstallTargetGenerator::NameReal);
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
::ComplainAboutMissingTarget(cmTarget* depender, cmTarget* dependee)
{
  cmOStringStream e;
  e << "INSTALL(EXPORT \"" << this->Name << "\" ...) "
    << "includes target \"" << depender->GetName()
    << "\" which requires target \"" << dependee->GetName()
    << "\" that is not in the export set.";
  cmSystemTools::Error(e.str().c_str());
}
