/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "cmExportCMakeConfigGenerator.h"

class cmFileSet;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmTargetExport;

class cmExportTryCompileFileGenerator : public cmExportCMakeConfigGenerator
{
public:
  cmExportTryCompileFileGenerator(cmGlobalGenerator* gg,
                                  std::vector<std::string> const& targets,
                                  cmMakefile* mf,
                                  std::set<std::string> const& langs);

  /** Set the list of targets to export.  */
  void SetConfig(std::string const& config) { this->Config = config; }

protected:
  // Implement virtual methods from the superclass.
  void ComplainAboutDuplicateTarget(
    std::string const& /*targetName*/) const override{};
  void ReportError(std::string const& errorMessage) const override;

  bool GenerateMainFile(std::ostream& os) override;

  void GenerateImportTargetsConfig(std::ostream&, std::string const&,
                                   std::string const&) override
  {
  }
  void HandleMissingTarget(std::string&, cmGeneratorTarget const*,
                           cmGeneratorTarget*) override
  {
  }

  ExportInfo FindExportInfo(cmGeneratorTarget const* /*target*/) const override
  {
    return { {}, {} };
  }

  void PopulateProperties(cmGeneratorTarget const* target,
                          ImportPropertyMap& properties,
                          std::set<cmGeneratorTarget const*>& emitted);

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             std::string const& config) override;

  std::string GetFileSetDirectories(cmGeneratorTarget* target,
                                    cmFileSet* fileSet,
                                    cmTargetExport const* te) override;

  std::string GetFileSetFiles(cmGeneratorTarget* target, cmFileSet* fileSet,
                              cmTargetExport const* te) override;

  std::string GetCxxModulesDirectory() const override { return {}; }
  void GenerateCxxModuleConfigInformation(std::string const&,
                                          std::ostream&) const override
  {
  }

private:
  std::string FindTargets(std::string const& prop,
                          cmGeneratorTarget const* tgt,
                          std::string const& language,
                          std::set<cmGeneratorTarget const*>& emitted);

  std::vector<cmGeneratorTarget const*> Exports;
  std::string Config;
  std::vector<std::string> Languages;
};
