/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmExportFileGenerator.h"
#include "cmGeneratorExpression.h"
#include "cmInstallExportGenerator.h"
#include "cmStateTypes.h"

class cmExportSet;
class cmGeneratorTarget;
class cmInstallTargetGenerator;
class cmTargetExport;

/** \class cmExportInstallFileGenerator
 * \brief Generate a file exporting targets from an install tree.
 *
 * cmExportInstallFileGenerator is the generic interface class for generating
 * export files for an install tree.
 */
class cmExportInstallFileGenerator : virtual public cmExportFileGenerator
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

  /** Get the temporary location of the config-agnostic C++ module file.  */
  std::string GetCxxModuleFile() const;

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
  virtual std::string GetConfigImportFileGlob() const = 0;

protected:
  cmStateEnums::TargetType GetExportTargetType(
    cmTargetExport const* targetExport) const;

  virtual std::string const& GetExportName() const;

  std::string GetInstallPrefix() const
  {
    cm::string_view const& prefixWithSlash = this->GetImportPrefixWithSlash();
    return std::string(prefixWithSlash.data(), prefixWithSlash.length() - 1);
  }
  virtual char GetConfigFileNameSeparator() const = 0;

  void HandleMissingTarget(std::string& link_libs,
                           cmGeneratorTarget const* depender,
                           cmGeneratorTarget* dependee) override;

  void ReplaceInstallPrefix(std::string& input) const override;

  void ComplainAboutMissingTarget(
    cmGeneratorTarget const* depender, cmGeneratorTarget const* dependee,
    std::vector<std::string> const& exportFiles) const;

  void ComplainAboutDuplicateTarget(
    std::string const& targetName) const override;

  ExportInfo FindExportInfo(cmGeneratorTarget const* target) const override;

  void ReportError(std::string const& errorMessage) const override;

  /** Generate a per-configuration file for the targets.  */
  virtual bool GenerateImportFileConfig(std::string const& config);

  /** Fill in properties indicating installed file locations.  */
  void SetImportLocationProperty(std::string const& config,
                                 std::string const& suffix,
                                 cmInstallTargetGenerator* itgen,
                                 ImportPropertyMap& properties,
                                 std::set<std::string>& importedLocations);

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             std::string const& config) override;

  using cmExportFileGenerator::GetCxxModuleFile;

  /** Walk the list of targets to be exported.  Returns true iff no duplicates
      are found.  */
  bool CollectExports(
    std::function<void(cmTargetExport const*)> const& visitor);

  cmExportSet* GetExportSet() const override
  {
    return this->IEGen->GetExportSet();
  }

  std::string GetImportXcFrameworkLocation(
    std::string const& config, cmTargetExport const* targetExport) const;

  using cmExportFileGenerator::PopulateInterfaceProperties;
  bool PopulateInterfaceProperties(cmTargetExport const* targetExport,
                                   ImportPropertyMap& properties);

  void PopulateImportProperties(std::string const& config,
                                std::string const& suffix,
                                cmTargetExport const* targetExport,
                                ImportPropertyMap& properties,
                                std::set<std::string>& importedLocations);

  cmInstallExportGenerator* IEGen;

  // The import file generated for each configuration.
  std::map<std::string, std::string> ConfigImportFiles;
  // The C++ module property file generated for each configuration.
  std::map<std::string, std::string> ConfigCxxModuleFiles;
  // The C++ module property target files generated for each configuration.
  std::map<std::string, std::vector<std::string>> ConfigCxxModuleTargetFiles;

private:
  bool CheckInterfaceDirs(std::string const& prepro,
                          cmGeneratorTarget const* target,
                          std::string const& prop) const;
  void PopulateCompatibleInterfaceProperties(cmGeneratorTarget const* target,
                                             ImportPropertyMap& properties);
  void PopulateCustomTransitiveInterfaceProperties(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  void PopulateIncludeDirectoriesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties, cmTargetExport const& te,
    std::string& includesDestinationDirs);
  void PopulateSourcesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  void PopulateLinkDirectoriesInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
  void PopulateLinkDependsInterface(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);
};
