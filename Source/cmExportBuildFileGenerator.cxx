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
#include "cmExportBuildFileGenerator.h"

#include "cmExportCommand.h"

//----------------------------------------------------------------------------
cmExportBuildFileGenerator::cmExportBuildFileGenerator()
{
  this->ExportCommand = 0;
}

//----------------------------------------------------------------------------
bool cmExportBuildFileGenerator::GenerateMainFile(std::ostream& os)
{
  // Create all the imported targets.
  for(std::vector<cmTarget*>::const_iterator
        tei = this->Exports->begin();
      tei != this->Exports->end(); ++tei)
    {
    cmTarget* te = *tei;
    this->ExportedTargets.insert(te);
    this->GenerateImportTargetCode(os, te);
    }

  // Generate import file content for each configuration.
  for(std::vector<std::string>::const_iterator
        ci = this->Configurations.begin();
      ci != this->Configurations.end(); ++ci)
    {
    this->GenerateImportConfig(os, ci->c_str());
    }

  return true;
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::GenerateImportTargetsConfig(std::ostream& os,
                              const char* config, std::string const& suffix)
{
  for(std::vector<cmTarget*>::const_iterator
        tei = this->Exports->begin();
      tei != this->Exports->end(); ++tei)
    {
    // Collect import properties for this target.
    cmTarget* target = *tei;
    ImportPropertyMap properties;
    this->SetImportLocationProperty(config, suffix, target, properties);
    if(!properties.empty())
      {
      // Get the rest of the target details.
      this->SetImportDetailProperties(config, suffix,
                                      target, properties);

      // TOOD: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, target, properties);
      }
    }
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::SetImportLocationProperty(const char* config, std::string const& suffix,
                            cmTarget* target, ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->GetMakefile();

  // Add the main target file.
  {
  std::string prop = "IMPORTED_LOCATION";
  prop += suffix;
  std::string value = target->GetFullPath(config, false);
  if(target->IsAppBundleOnApple())
    {
    value += ".app/Contents/MacOS/";
    value += target->GetFullName(config, false);
    }
  properties[prop] = value;
  }

  // Check whether this is a DLL platform.
  bool dll_platform =
    (mf->IsOn("WIN32") || mf->IsOn("CYGWIN") || mf->IsOn("MINGW"));

  // Add the import library for windows DLLs.
  if(dll_platform &&
     (target->GetType() == cmTarget::SHARED_LIBRARY ||
      target->IsExecutableWithExports()) &&
     mf->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    std::string prop = "IMPORTED_IMPLIB";
    prop += suffix;
    std::string value = target->GetFullPath(config, true);
    properties[prop] = value;
    }
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::ComplainAboutMissingTarget(cmTarget* target, const char* dep)
{
  if(!this->ExportCommand || !this->ExportCommand->ErrorMessage.empty())
    {
    return;
    }

  cmOStringStream e;
  e << "called with target \"" << target->GetName()
    << "\" which links to target \"" << dep
    << "\" that is not in the export list.\n"
    << "If the link dependency is not part of the public interface "
    << "consider setting the LINK_INTERFACE_LIBRARIES property on \""
    << target->GetName() << "\".  Otherwise add it to the export list.  "
    << "If the link dependency is not easy to reference in this call, "
    << "consider using the APPEND option with multiple separate calls.";
  this->ExportCommand->ErrorMessage = e.str();
}
