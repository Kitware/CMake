/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportInstallAndroidMKGenerator_h
#define cmExportInstallAndroidMKGenerator_h

#include "cmExportInstallFileGenerator.h"

class cmInstallExportGenerator;
class cmInstallTargetGenerator;

/** \class cmExportInstallAndroidMKGenerator
 * \brief Generate a file exporting targets from an install tree.
 *
 * cmExportInstallAndroidMKGenerator generates files exporting targets from
 * install an installation tree.  The files are placed in a temporary
 * location for installation by cmInstallExportGenerator.  The file format
 * is for the ndk build system and is a makefile fragment specifing prebuilt
 * libraries to the ndk build system.
 *
 * This is used to implement the INSTALL(EXPORT_ANDROID_MK) command.
 */
class cmExportInstallAndroidMKGenerator : public cmExportInstallFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallAndroidMKGenerator(cmInstallExportGenerator* iegen);

protected:
  // Implement virtual methods from the superclass.
  virtual void GeneratePolicyHeaderCode(std::ostream&) {}
  virtual void GeneratePolicyFooterCode(std::ostream&) {}
  virtual void GenerateImportHeaderCode(std::ostream& os,
                                        const std::string& config = "");
  virtual void GenerateImportFooterCode(std::ostream& os);
  virtual void GenerateImportTargetCode(std::ostream& os,
                                        const cmGeneratorTarget* target);
  virtual void GenerateExpectedTargetsCode(std::ostream& os,
                                           const std::string& expectedTargets);
  virtual void GenerateImportPropertyCode(std::ostream& os,
                                          const std::string& config,
                                          cmGeneratorTarget const* target,
                                          ImportPropertyMap const& properties);
  virtual void GenerateMissingTargetsCheckCode(
    std::ostream& os, const std::vector<std::string>& missingTargets);
  virtual void GenerateInterfaceProperties(
    cmGeneratorTarget const* target, std::ostream& os,
    const ImportPropertyMap& properties);
  virtual void GenerateImportPrefix(std::ostream& os);
  virtual void LoadConfigFiles(std::ostream&);
  virtual void GenerateRequiredCMakeVersion(std::ostream& os,
                                            const char* versionString);
  virtual void CleanupTemporaryVariables(std::ostream&);
  virtual void GenerateImportedFileCheckLoop(std::ostream& os);
  virtual void GenerateImportedFileChecksCode(
    std::ostream& os, cmGeneratorTarget* target,
    ImportPropertyMap const& properties,
    const std::set<std::string>& importedLocations);
  virtual bool GenerateImportFileConfig(const std::string& config,
                                        std::vector<std::string>&);
};

#endif
