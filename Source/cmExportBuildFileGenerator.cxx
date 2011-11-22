/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
    if(this->ExportedTargets.insert(te).second)
      {
      this->GenerateImportTargetCode(os, te);
      }
    else
      {
      if(this->ExportCommand && this->ExportCommand->ErrorMessage.empty())
        {
        cmOStringStream e;
        e << "given target \"" << te->GetName() << "\" more than once.";
        this->ExportCommand->ErrorMessage = e.str();
        }
      return false;
      }
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
  std::string value;
  if(target->IsFrameworkOnApple() || target->IsAppBundleOnApple())
    {
    value = target->GetFullPath(config, false);
    }
  else
    {
    value = target->GetFullPath(config, false, true);
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
::ComplainAboutMissingTarget(cmTarget* depender,
                             cmTarget* dependee)
{
  if(!this->ExportCommand || !this->ExportCommand->ErrorMessage.empty())
    {
    return;
    }

  cmOStringStream e;
  e << "called with target \"" << depender->GetName()
    << "\" which requires target \"" << dependee->GetName()
    << "\" that is not in the export list.\n"
    << "If the required target is not easy to reference in this call, "
    << "consider using the APPEND option with multiple separate calls.";
  this->ExportCommand->ErrorMessage = e.str();
}
