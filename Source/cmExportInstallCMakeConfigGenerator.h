/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmExportCMakeConfigGenerator.h"
#include "cmExportInstallFileGenerator.h"

class cmFileSet;
class cmGeneratorTarget;
class cmInstallExportGenerator;
class cmTargetExport;

/** \class cmExportInstallCMakeConfigGenerator
 * \brief Generate files exporting targets from an install tree.
 *
 * cmExportInstallCMakeConfigGenerator generates files exporting targets from
 * an installation tree.  The files are placed in a temporary location for
 * installation by cmInstallExportGenerator.  The file format is CMake's native
 * package configuration format.
 *
 * One main file is generated that creates the imported targets and loads
 * per-configuration files.  Target locations and settings for each
 * configuration are written to these per-configuration files.  After
 * installation the main file loads the configurations that have been
 * installed.
 *
 * This is used to implement the INSTALL(EXPORT) command.
 */
class cmExportInstallCMakeConfigGenerator
  : public cmExportCMakeConfigGenerator
  , public cmExportInstallFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallCMakeConfigGenerator(cmInstallExportGenerator* iegen);

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob() const override;

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream& os, std::string const& config,
                                   std::string const& suffix) override;
  void GenerateImportConfig(std::ostream& os,
                            std::string const& config) override;

  char GetConfigFileNameSeparator() const override { return '-'; }

  /** Generate the relative import prefix.  */
  virtual void GenerateImportPrefix(std::ostream&);

  /** Generate the relative import prefix.  */
  virtual void LoadConfigFiles(std::ostream&);

  virtual void CleanupTemporaryVariables(std::ostream&);

  std::string GetFileSetDirectories(cmGeneratorTarget* gte, cmFileSet* fileSet,
                                    cmTargetExport const* te) override;
  std::string GetFileSetFiles(cmGeneratorTarget* gte, cmFileSet* fileSet,
                              cmTargetExport const* te) override;

  std::string GetCxxModulesDirectory() const override;
  void GenerateCxxModuleConfigInformation(std::string const&,
                                          std::ostream&) const override;
  bool GenerateImportCxxModuleConfigTargetInclusion(std::string const&,
                                                    std::string const&);
};
