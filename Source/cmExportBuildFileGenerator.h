/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmExportFileGenerator.h"
#include "cmStateTypes.h"

class cmExportSet;
class cmGeneratorTarget;
class cmLocalGenerator;

/** \class cmExportBuildCMakeConfigGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildCMakeConfigGenerator is the interface class for generating a
 * file exporting targets from a build tree.
 *
 * This is used to implement the export() command.
 */
class cmExportBuildFileGenerator : virtual public cmExportFileGenerator
{
public:
  struct TargetExport
  {
    TargetExport(std::string name, std::string xcFrameworkLocation)
      : Name(std::move(name))
      , XcFrameworkLocation(std::move(xcFrameworkLocation))
    {
    }

    std::string Name;
    std::string XcFrameworkLocation;
  };

  cmExportBuildFileGenerator();

  /** Set the list of targets to export.  */
  void SetTargets(std::vector<TargetExport> const& targets)
  {
    this->Targets = targets;
  }
  void GetTargets(std::vector<TargetExport>& targets) const;
  void AppendTargets(std::vector<TargetExport> const& targets)
  {
    cm::append(this->Targets, targets);
  }
  void SetExportSet(cmExportSet*);

  /** Set the name of the C++ module directory.  */
  void SetCxxModuleDirectory(std::string cxx_module_dir)
  {
    this->CxxModulesDirectory = std::move(cxx_module_dir);
  }
  std::string const& GetCxxModuleDirectory() const
  {
    return this->CxxModulesDirectory;
  }

  void Compute(cmLocalGenerator* lg);

protected:
  cmStateEnums::TargetType GetExportTargetType(
    cmGeneratorTarget const* target) const;

  /** Walk the list of targets to be exported.  Returns true iff no duplicates
      are found.  */
  bool CollectExports(std::function<void(cmGeneratorTarget const*)> visitor);

  void HandleMissingTarget(std::string& link_libs,
                           cmGeneratorTarget const* depender,
                           cmGeneratorTarget* dependee) override;

  void ComplainAboutMissingTarget(
    cmGeneratorTarget const* depender, cmGeneratorTarget const* dependee,
    std::vector<std::string> const& exportFiles) const;

  void ComplainAboutDuplicateTarget(
    std::string const& targetName) const override;

  void ReportError(std::string const& errorMessage) const override;

  /** Fill in properties indicating built file locations.  */
  void SetImportLocationProperty(std::string const& config,
                                 std::string const& suffix,
                                 cmGeneratorTarget* target,
                                 ImportPropertyMap& properties);

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             std::string const& config) override;

  cmExportSet* GetExportSet() const override { return this->ExportSet; }

  std::string GetCxxModulesDirectory() const override
  {
    return this->CxxModulesDirectory;
  }

  ExportInfo FindExportInfo(cmGeneratorTarget const* target) const override;

  using cmExportFileGenerator::PopulateInterfaceProperties;
  bool PopulateInterfaceProperties(cmGeneratorTarget const* target,
                                   ImportPropertyMap& properties);

  struct TargetExportPrivate
  {
    TargetExportPrivate(cmGeneratorTarget* target,
                        std::string xcFrameworkLocation)
      : Target(target)
      , XcFrameworkLocation(std::move(xcFrameworkLocation))
    {
    }

    cmGeneratorTarget* Target;
    std::string XcFrameworkLocation;
  };

  std::vector<TargetExport> Targets;
  cmExportSet* ExportSet;
  std::vector<TargetExportPrivate> Exports;
  cmLocalGenerator* LG;
  // The directory for C++ module information.
  std::string CxxModulesDirectory;
};
