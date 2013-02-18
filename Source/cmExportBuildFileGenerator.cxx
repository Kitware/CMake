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
  std::vector<cmTarget*> allTargets;
  {
  std::string expectedTargets;
  std::string sep;
  for(std::vector<cmTarget*>::const_iterator
        tei = this->Exports->begin();
      tei != this->Exports->end(); ++tei)
    {
    expectedTargets += sep + this->Namespace + (*tei)->GetName();
    sep = " ";
    cmTarget* te = *tei;
    if(this->ExportedTargets.insert(te).second)
      {
      allTargets.push_back(te);
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

  this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  std::vector<std::string> missingTargets;

  // Create all the imported targets.
  for(std::vector<cmTarget*>::const_iterator
        tei = allTargets.begin();
      tei != allTargets.end(); ++tei)
    {
    cmTarget* te = *tei;
    this->GenerateImportTargetCode(os, te);

    te->AppendBuildInterfaceIncludes();

    ImportPropertyMap properties;

    this->PopulateInterfaceProperty("INTERFACE_INCLUDE_DIRECTORIES", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE",
                                  te, properties);
    this->PopulateCompatibleInterfaceProperties(te, properties);

    this->GenerateInterfaceProperties(te, os, properties);
    }

  // Generate import file content for each configuration.
  for(std::vector<std::string>::const_iterator
        ci = this->Configurations.begin();
      ci != this->Configurations.end(); ++ci)
    {
    this->GenerateImportConfig(os, ci->c_str(), missingTargets);
    }

  this->GenerateMissingTargetsCheckCode(os, missingTargets);

  return true;
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::GenerateImportTargetsConfig(std::ostream& os,
                              const char* config, std::string const& suffix,
                            std::vector<std::string> &missingTargets)
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
                                      target, properties, missingTargets);
      this->SetImportLinkInterface(config, suffix,
                                   cmGeneratorExpression::BuildInterface,
                                   target, properties, missingTargets);


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
    target->GetImplibGNUtoMS(value, value,
                             "${CMAKE_IMPORT_LIBRARY_SUFFIX}");
    properties[prop] = value;
    }
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator::HandleMissingTarget(
  std::string& link_libs, std::vector<std::string>&,
  cmMakefile*, cmTarget* depender, cmTarget* dependee)
{
  // The target is not in the export.
  if(!this->AppendMode)
    {
    // We are not appending, so all exported targets should be
    // known here.  This is probably user-error.
    this->ComplainAboutMissingTarget(depender, dependee);
    }
  // Assume the target will be exported by another command.
  // Append it with the export namespace.
  link_libs += this->Namespace;
  link_libs += dependee->GetName();
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
