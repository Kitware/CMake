/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include <cm/optional>
#include <cm/string_view>

#include "cmExportInstallFileGenerator.h"
#include "cmExportSbomGenerator.h"

class cmFileSet;
class cmGeneratorTarget;
class cmInstallExportGenerator;
class cmSbomArguments;
class cmTargetExport;

class cmExportInstallSbomGenerator
  : public cmExportSbomGenerator
  , public cmExportInstallFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallSbomGenerator(cmInstallExportGenerator* iegen,
                               cmSbomArguments arguments);

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob() const override;

protected:
  std::string const& GetExportName() const override;

  cm::string_view GetImportPrefixWithSlash() const override;
  std::string GetCxxModuleFile(std::string const& name) const override;
  void GenerateCxxModuleConfigInformation(std::string const&,
                                          std::ostream& os) const override;

  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream& os, std::string const& config,
                                   std::string const& suffix) override;

  void HandleMissingTarget(std::string& /* link_libs */,
                           cmGeneratorTarget const* /* depender */,
                           cmGeneratorTarget* /* dependee */) override;

  bool CheckInterfaceDirs(std::string const& /* prepro */,
                          cmGeneratorTarget const* /* target */,
                          std::string const& /* prop */) const override;

  char GetConfigFileNameSeparator() const override { return '@'; }

  std::string GenerateImportPrefix() const;
  std::string InstallNameDir(cmGeneratorTarget const* target,
                             std::string const& config) override;

  std::string GetCxxModulesDirectory() const override;

  cm::optional<std::string> GetFileSetDirectory(
    cmGeneratorTarget* gte, cmTargetExport const* te, cmFileSet* fileSet,
    cm::optional<std::string> const& config = {});
};
