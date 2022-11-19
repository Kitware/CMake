/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmStateTypes.h"

class cmFileSet;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmInstallExportGenerator;
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
class cmExportInstallFileGenerator : public cmExportFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallFileGenerator(cmInstallExportGenerator* iegen);

  /** Get the per-config file generated for each configuration.  This
      maps from the configuration name to the file temporary location
      for installation.  */
  std::map<std::string, std::string> const& GetConfigImportFiles()
  {
    return this->ConfigImportFiles;
  }

  /** Get the per-config C++ module file generated for each configuration.
      This maps from the configuration name to the file temporary location
      for installation.  */
  std::map<std::string, std::string> const& GetConfigCxxModuleFiles()
  {
    return this->ConfigCxxModuleFiles;
  }

  /** Get the per-config C++ module file generated for each configuration.
      This maps from the configuration name to the file temporary location
      for installation for each target in the export set.  */
  std::map<std::string, std::vector<std::string>> const&
  GetConfigCxxModuleTargetFiles()
  {
    return this->ConfigCxxModuleTargetFiles;
  }

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob();

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream& os, const std::string& config,
                                   std::string const& suffix) override;
  cmStateEnums::TargetType GetExportTargetType(
    cmTargetExport const* targetExport) const;
  void HandleMissingTarget(std::string& link_libs,
                           cmGeneratorTarget const* depender,
                           cmGeneratorTarget* dependee) override;

  void ReplaceInstallPrefix(std::string& input) override;

  void ComplainAboutMissingTarget(cmGeneratorTarget const* depender,
                                  cmGeneratorTarget const* dependee,
                                  std::vector<std::string> const& exportFiles);

  std::pair<std::vector<std::string>, std::string> FindNamespaces(
    cmGlobalGenerator* gg, const std::string& name);

  /** Generate the relative import prefix.  */
  virtual void GenerateImportPrefix(std::ostream&);

  /** Generate the relative import prefix.  */
  virtual void LoadConfigFiles(std::ostream&);

  virtual void CleanupTemporaryVariables(std::ostream&);

  /** Generate a per-configuration file for the targets.  */
  virtual bool GenerateImportFileConfig(const std::string& config);

  /** Fill in properties indicating installed file locations.  */
  void SetImportLocationProperty(const std::string& config,
                                 std::string const& suffix,
                                 cmInstallTargetGenerator* itgen,
                                 ImportPropertyMap& properties,
                                 std::set<std::string>& importedLocations);

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             const std::string& config) override;

  std::string GetFileSetDirectories(cmGeneratorTarget* gte, cmFileSet* fileSet,
                                    cmTargetExport* te) override;
  std::string GetFileSetFiles(cmGeneratorTarget* gte, cmFileSet* fileSet,
                              cmTargetExport* te) override;

  std::string GetCxxModulesDirectory() const override;
  void GenerateCxxModuleConfigInformation(std::ostream&) const override;
  bool GenerateImportCxxModuleConfigTargetInclusion(std::string const&);

  cmInstallExportGenerator* IEGen;

  // The import file generated for each configuration.
  std::map<std::string, std::string> ConfigImportFiles;
  // The C++ module property file generated for each configuration.
  std::map<std::string, std::string> ConfigCxxModuleFiles;
  // The C++ module property target files generated for each configuration.
  std::map<std::string, std::vector<std::string>> ConfigCxxModuleTargetFiles;
};
