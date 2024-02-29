/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "cmExportFileGenerator.h"

class cmFileSet;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmTargetExport;

class cmExportTryCompileFileGenerator : public cmExportFileGenerator
{
public:
  cmExportTryCompileFileGenerator(cmGlobalGenerator* gg,
                                  std::vector<std::string> const& targets,
                                  cmMakefile* mf,
                                  std::set<std::string> const& langs);

  /** Set the list of targets to export.  */
  void SetConfig(const std::string& config) { this->Config = config; }

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;

  void GenerateImportTargetsConfig(std::ostream&, const std::string&,
                                   std::string const&) override
  {
  }
  void HandleMissingTarget(std::string&, cmGeneratorTarget const*,
                           cmGeneratorTarget*) override
  {
  }

  void PopulateProperties(cmGeneratorTarget const* target,
                          ImportPropertyMap& properties,
                          std::set<const cmGeneratorTarget*>& emitted);

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             const std::string& config) override;

  std::string GetFileSetDirectories(cmGeneratorTarget* target,
                                    cmFileSet* fileSet,
                                    cmTargetExport* te) override;

  std::string GetFileSetFiles(cmGeneratorTarget* target, cmFileSet* fileSet,
                              cmTargetExport* te) override;

  std::string GetCxxModulesDirectory() const override { return {}; }
  void GenerateCxxModuleConfigInformation(std::string const&,
                                          std::ostream&) const override
  {
  }

private:
  std::string FindTargets(const std::string& prop,
                          const cmGeneratorTarget* tgt,
                          std::string const& language,
                          std::set<const cmGeneratorTarget*>& emitted);

  std::vector<cmGeneratorTarget const*> Exports;
  std::string Config;
  std::vector<std::string> Languages;
};
