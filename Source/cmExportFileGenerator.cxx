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

#include <cmsys/auto_ptr.hxx>

//----------------------------------------------------------------------------
cmExportFileGenerator::cmExportFileGenerator()
{
  this->AppendMode = false;
}

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
  cmsys::auto_ptr<std::ofstream> foutPtr;
  if(this->AppendMode)
    {
    // Open for append.
    cmsys::auto_ptr<std::ofstream>
      ap(new std::ofstream(this->MainImportFile.c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    // Generate atomically and with copy-if-different.
    cmsys::auto_ptr<cmGeneratedFileStream>
      ap(new cmGeneratedFileStream(this->MainImportFile.c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  if(!foutPtr.get() || !*foutPtr)
    {
    std::string se = cmSystemTools::GetLastSystemError();
    cmOStringStream e;
    e << "cannot write to file \"" << this->MainImportFile
      << "\": " << se;
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  std::ostream& os = *foutPtr;

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
  if(cmTargetLinkInterface const* interface =
     target->GetLinkInterface(config))
    {
    // This target provides a link interface, so use it.
    this->SetImportLinkProperties(config, suffix, target,
                                  *interface, properties);
    }
  else if(target->GetType() == cmTarget::STATIC_LIBRARY ||
          target->GetType() == cmTarget::SHARED_LIBRARY)
    {
    // The default link interface for static and shared libraries is
    // their link implementation library list.
    this->SetImportLinkProperties(config, suffix, target, properties);
    }
}

//----------------------------------------------------------------------------
void
cmExportFileGenerator
::SetImportLinkProperties(const char* config, std::string const& suffix,
                          cmTarget* target, ImportPropertyMap& properties)
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(config && cmSystemTools::UpperCase(config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }

  // Construct the list of libs linked for this configuration.
  std::vector<std::string> actual_libs;
  cmTarget::LinkLibraryVectorType const& libs =
    target->GetOriginalLinkLibraries();
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

    // Store this entry.
    actual_libs.push_back(li->first);
    }

  // Store the entries in the property.
  this->SetImportLinkProperties(config, suffix, target,
                                actual_libs, properties);
}

//----------------------------------------------------------------------------
void
cmExportFileGenerator
::SetImportLinkProperties(const char* config,
                          std::string const& suffix,
                          cmTarget* target,
                          std::vector<std::string> const& libs,
                          ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->GetMakefile();

  // Construct the property value.
  std::string link_libs;
  const char* sep = "";
  for(std::vector<std::string>::const_iterator li = libs.begin();
      li != libs.end(); ++li)
    {
    // Append this entry.
    if(cmTarget* tgt = mf->FindTargetToUse(li->c_str()))
      {
      // This is a target.
      if(tgt->IsImported())
        {
        // The target is imported (and therefore is not in the
        // export).  Append the raw name.
        link_libs += *li;
        }
      else if(this->ExportedTargets.find(tgt) != this->ExportedTargets.end())
        {
        // The target is in the export.  Append it with the export
        // namespace.
        link_libs += this->Namespace;
        link_libs += *li;
        }
      else
        {
        // The target is not in the export.
        if(!this->AppendMode)
          {
          // We are not appending, so all exported targets should be
          // known here.  This is probably user-error.
          this->ComplainAboutMissingTarget(target, li->c_str());
          }
        link_libs += *li;
        }
      }
    else
      {
      // Append the raw name.
      link_libs += *li;
      }
    }

  // Store the property.
  std::string prop = "IMPORTED_LINK_LIBRARIES";
  prop += suffix;
  properties[prop] = link_libs;
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

  // Mark the imported executable if it has exports.
  if(target->IsExecutableWithExports())
    {
    os << "SET_PROPERTY(TARGET " << targetName
       << " PROPERTY ENABLE_EXPORTS 1)\n";
    }

  // Mark the imported library if it is a framework.
  if(target->IsFrameworkOnApple())
    {
    os << "SET_PROPERTY(TARGET " << targetName
       << " PROPERTY FRAMEWORK 1)\n";
    }

  // Mark the imported executable if it is an application bundle.
  if(target->IsAppBundleOnApple())
    {
    os << "SET_PROPERTY(TARGET " << targetName
       << " PROPERTY MACOSX_BUNDLE 1)\n";
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
