/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmSbomArguments.h"

class cmExportSet;
class cmGeneratorTarget;
class cmLocalGenerator;
struct cmSbomDocument;
struct cmSpdxDocument;
struct cmSpdxPackage;
struct cmSpdxCreationInfo;

/** \class cmSbomBuilder
 *  \brief Abstract base for SBOM document generators.
 *
 *  Concrete leaves (cmBuildSbomBuilder, cmInstallSbomBuilder) supply two
 *  pure virtuals:
 *
 *  - Generate(): selects the generator-expression preprocess context
 *    (BuildInterface vs InstallInterface) and delegates to the shared
 *    GenerateForTargets() body.
 *  - FindExportInfoFor(): consults the mode's export map (build-tree vs
 *    install-tree) to resolve cross-export references.
 *
 *  All shared SPDX assembly, link-graph walking, and serialization lives
 *  here so that the leaves stay trivial.
 */
class cmSbomBuilder
{
public:
  virtual ~cmSbomBuilder() = default;

  /** Compute phase: wire the local generator, run Compute() on the owned
   *  export sets, and populate the SbomTargets cache so peer SBOMs can
   *  query CoversTarget() before any Generate() runs. */
  void Compute(cmLocalGenerator* lg);

  /** Produce the SBOM document on `os`.  Implementations select their own
   *  target set (via CollectTargets) and preprocess context. */
  virtual bool Generate(std::ostream& os, std::string const& config) = 0;

  /** Identifier this SBOM publishes itself as (the SPDX document name and
   *  the namespace under which other SBOMs refer to its contents). */
  std::string const& GetPackageName() const { return this->PackageName; }

  /** True if `target` is one of the targets this SBOM directly describes. */
  bool CoversTarget(cmGeneratorTarget const* target) const;
  bool CoversExportSet(cmExportSet const* set) const;

  void AddConfiguration(std::string const& config)
  {
    this->Configurations.push_back(config);
  }

  /** Names of peer SBOMs (same build/install mode) that cover a target.
   *  Used by NoteLinkedTarget to attribute a cross-reference when no
   *  install(export) namespace is available.  Sorted alphabetically. */
  struct SbomInfo
  {
    std::vector<std::string> Packages;
  };

protected:
  cmSbomBuilder(cmSbomArguments args, std::vector<cmExportSet*> exportSets,
                cmLocalGenerator* lg);

  /** Mode-specific: where does `target` appear in the project's exports?
   *  Build leaves consult cmGlobalGenerator::FindBuildExportInfo;
   *  install leaves consult cmGlobalGenerator::FindInstallExportInfo. */
  virtual cmExportFileGenerator::ExportInfo FindExportInfoFor(
    cmGeneratorTarget const* target) const = 0;

  /** Mode-specific: which peer SBOMs cover `target`?
   *  Build leaves consult cmGlobalGenerator::FindBuildSbomInfo;
   *  install leaves consult cmGlobalGenerator::FindInstallSbomInfo. */
  virtual SbomInfo FindSbomInfoFor(cmGeneratorTarget const* target) const = 0;

  /** The set of targets the SBOM directly describes — derived from the
   *  associated export sets. */
  std::set<cmGeneratorTarget const*> CollectTargets() const;

  /** Generate an sbom for the targets in this->SbomTargets. Each leaf class
   * calls this internally */
  bool GenerateForTargets(
    std::ostream& os, std::string const& config,
    cmGeneratorExpression::PreprocessContext preprocessContext);

  using ImportPropertyMap = std::map<std::string, std::string>;

  struct TargetProperties
  {
    cmSpdxPackage const* Package;
    cmGeneratorTarget const* Target;
    ImportPropertyMap Properties;
  };

  void WriteSbom(cmSbomDocument& doc, std::ostream& os) const;

  cmSpdxCreationInfo GenerateCreationInfo() const;
  cmSpdxDocument GenerateSbom(cmSpdxCreationInfo const* ci) const;
  cmSpdxPackage GenerateImportTarget(cmSpdxCreationInfo const* ci,
                                     cmGeneratorTarget const* target) const;

  bool AddPackageInformation(cmSpdxPackage& artifact, std::string const& name,
                             cmPackageInformation const& package) const;

  bool GenerateProperties(cmSbomDocument& doc, cmSpdxDocument* project,
                          cmSpdxCreationInfo const* ci,
                          TargetProperties const& current,
                          std::vector<TargetProperties> const& allTargets,
                          std::string const& config) const;

  bool GenerateLinkProperties(cmSbomDocument& doc, cmSpdxDocument* project,
                              cmSpdxCreationInfo const* ci,
                              std::string const& libraries,
                              TargetProperties const& current,
                              std::vector<TargetProperties> const& allTargets,
                              std::string const& config) const;
  bool GenerateMetaProperties(cmSbomDocument& doc, cmSpdxDocument* project,
                              cmSpdxCreationInfo const* ci,
                              TargetProperties const& current) const;

  bool NoteLinkedTarget(cmGeneratorTarget const* target,
                        std::string const& linkedName,
                        cmGeneratorTarget const* linkedTarget);

  bool AddTargetNamespace(std::string& input, cmGeneratorTarget const* target,
                          cmLocalGenerator const* lg);

  void ResolveTargetsInGeneratorExpression(std::string& input,
                                           cmGeneratorTarget const* target,
                                           cmLocalGenerator const* lg);
  void ResolveTargetsInGeneratorExpressions(std::string& input,
                                            cmGeneratorTarget const* target);

  bool PopulateInterfaceLinkLibrariesProperty(
    cmGeneratorTarget const* target,
    cmGeneratorExpression::PreprocessContext preprocessRule,
    ImportPropertyMap& properties);

  bool PopulateLinkLibrariesProperty(cmGeneratorTarget const* target,
                                     cmGeneratorExpression::PreprocessContext,
                                     ImportPropertyMap& properties);

  // Set at construction or via setters
  cmLocalGenerator* LocalGenerator = nullptr;

  // Inputs
  std::vector<cmExportSet*> ExportSets;

private:
  struct LinkInfo
  {
    std::string Package;
    std::string Component;
    std::string License;
  };

  // Metadata
  std::string const PackageName;
  std::string const Namespace;
  std::string const PackageVersion;
  std::string const PackageDescription;
  std::string const PackageWebsite;
  std::string const PackageUrl;
  std::string const DataLicense;
  std::string const DefaultLicense;
  cmSbomArguments::SbomFormat const PackageFormat;

  // Derived from inputs at generate time
  std::set<cmGeneratorTarget const*> SbomTargets;

  // Accumulated during generation
  std::map<std::string, LinkInfo> LinkTargets;
  std::map<std::string, cmPackageInformation> Requirements;
  std::vector<std::string> Configurations;

  // Targets for which license data has been generated
  mutable std::set<std::string> GeneratedLinkLicenses;
};
