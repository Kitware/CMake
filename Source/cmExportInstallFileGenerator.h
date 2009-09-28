/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportInstallFileGenerator_h
#define cmExportInstallFileGenerator_h

#include "cmExportFileGenerator.h"

class cmInstallExportGenerator;
class cmInstallFilesGenerator;
class cmInstallTargetGenerator;
class cmTargetExport;

/** \class cmExportInstallFileGenerator
 * \brief Generate a file exporting targets from an install tree.
 *
 * cmExportInstallFileGenerator generates files exporting targets from
 * install an installation tree.  The files are placed in a temporary
 * location for installation by cmInstallExportGenerator.  One main
 * file is generated that creates the imported targets and loads
 * per-configuration files.  Target locations and settings for each
 * configuration are written to these per-configuration files.  After
 * installation the main file loads the configurations that have been
 * installed.
 *
 * This is used to implement the INSTALL(EXPORT) command.
 */
class cmExportInstallFileGenerator: public cmExportFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallFileGenerator(cmInstallExportGenerator* iegen);

  /** Set the name of the export associated with the files.  This is
      the name given to the install(EXPORT) command mode.  */
  void SetName(const char* name) { this->Name = name; }

  /** Set the set of targets to be exported.  These are the targets
      associated with the export name.  */
  void SetExportSet(std::vector<cmTargetExport*> const* eSet)
    { this->ExportSet = eSet; }

  /** Get the per-config file generated for each configuraiton.  This
      maps from the configuration name to the file temporary location
      for installation.  */
  std::map<cmStdString, cmStdString> const& GetConfigImportFiles()
    { return this->ConfigImportFiles; }

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob();
protected:

  // Implement virtual methods from the superclass.
  virtual bool GenerateMainFile(std::ostream& os);
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           const char* config,
                                           std::string const& suffix);
  virtual void ComplainAboutMissingTarget(cmTarget* depender,
                                          cmTarget* dependee);

  /** Generate a per-configuration file for the targets.  */
  bool GenerateImportFileConfig(const char* config);

  /** Fill in properties indicating installed file locations.  */
  void SetImportLocationProperty(const char* config,
                                 std::string const& suffix,
                                 cmInstallTargetGenerator* itgen,
                                 ImportPropertyMap& properties);

  void ComplainAboutImportPrefix(cmInstallTargetGenerator* itgen);

  cmInstallExportGenerator* InstallExportGenerator;
  std::string Name;
  std::vector<cmTargetExport*> const* ExportSet;

  std::string ImportPrefix;

  // The import file generated for each configuration.
  std::map<cmStdString, cmStdString> ConfigImportFiles;
};

/*
  cmTargetExport is used in cmGlobalGenerator to collect the install
  generators for targets associated with an export.
*/
class cmTargetExport
{
public:
  cmTargetExport(cmTarget* tgt,
                 cmInstallTargetGenerator* archive,
                 cmInstallTargetGenerator* runtime,
                 cmInstallTargetGenerator* library,
                 cmInstallTargetGenerator* framework,
                 cmInstallTargetGenerator* bundle,
                 cmInstallFilesGenerator* headers
                ) : Target(tgt), ArchiveGenerator(archive),
                    RuntimeGenerator(runtime), LibraryGenerator(library),
                    FrameworkGenerator(framework), BundleGenerator(bundle),
                    HeaderGenerator(headers) {}

  cmTarget* Target;
  cmInstallTargetGenerator* ArchiveGenerator;
  cmInstallTargetGenerator* RuntimeGenerator;
  cmInstallTargetGenerator* LibraryGenerator;
  cmInstallTargetGenerator* FrameworkGenerator;
  cmInstallTargetGenerator* BundleGenerator;
  cmInstallFilesGenerator* HeaderGenerator;
private:
  cmTargetExport();
};

#endif
