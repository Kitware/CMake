/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmSbomArguments.h"
#include "cmSpdx.h"

class cmExportSet;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmLinkItem;

/** \class cmSbomBuilder
 *  \brief Builds SBOM documents for the targets in a set of exports.
 *
 *  Concrete leaves (cmBuildSbomBuilder, cmInstallSbomBuilder) supply the
 *  mode-specific operations:
 *
 *  - Generate(): selects the generator-expression preprocess context and
 *    delegates to GenerateForTargets().
 *  - FindExportInfoFor(): consults the mode's export map (build-tree vs
 *    install-tree) to resolve cross-export references.
 *  - FindSbomPackagesFor(): finds peer SBOMs that cover a dependency.
 *
 *  All shared SPDX assembly, link-graph walking, and serialization lives
 *  here so that the leaves stay trivial.
 */
class cmSbomBuilder
{
public:
  cmSbomBuilder(cmSbomBuilder const&) = delete;
  cmSbomBuilder& operator=(cmSbomBuilder const&) = delete;
  virtual ~cmSbomBuilder() = default;

  /** Resolve export sets and collect explicit targets for peer SBOM lookups
   *  before any documents are generated. */
  bool Compute(cmLocalGenerator* lg);

  /** Produce one configuration's document on `os`. */
  virtual bool Generate(std::ostream& os, std::string const& config) = 0;

  std::string const& GetPackageName() const { return this->PackageName; }

  /** True if `target` is explicitly included through an export set. */
  bool CoversTarget(cmGeneratorTarget const* target) const;
  bool CoversExportSet(cmExportSet const* set) const;

protected:
  cmSbomBuilder(cmSbomArguments const& args,
                std::vector<cmExportSet*> const& exportSets);

  std::set<cmGeneratorTarget const*> CollectTargets() const;

  bool GenerateForTargets(
    std::ostream& os, std::string const& config,
    cmGeneratorExpression::PreprocessContext preprocessContext);

  /** Mode-specific: where does `target` appear in the project's exports?
   *  Build leaves consult cmGlobalGenerator::FindBuildExportInfo;
   *  install leaves consult cmGlobalGenerator::FindInstallExportInfo. */
  virtual cmExportFileGenerator::ExportInfo FindExportInfoFor(
    cmGeneratorTarget const* target) const = 0;

  /** Names of peer SBOMs in this mode that cover `target`, sorted
   *  alphabetically.  Used when no export namespace is available. */
  virtual std::vector<std::string> FindSbomPackagesFor(
    cmGeneratorTarget const* target) const = 0;

private:
  struct LinkInfo
  {
    LinkInfo(std::string package, std::string component, bool local = false)
      : Package(std::move(package))
      , Component(std::move(component))
      , Local(local)
    {
    }

    std::string Package;
    std::string Component;
    bool Local;
  };

  bool WriteSbom(std::ostream& os) const;

  cmSpdxCreationInfo GenerateCreationInfo() const;
  cmSpdxDocument GenerateSbom() const;
  cmSpdxPackage GenerateLocalPackage(cmGeneratorTarget const* target) const;

  bool AddPackageInformation(cmSpdxPackage& artifact, std::string const& name,
                             cmPackageInformation const& package) const;

  cmSpdxPackage const* GetOrCreateLocalPackage(
    cmGeneratorTarget const* target);
  cmSpdxPackage const* GetOrCreateLinkedPackage(
    cmGeneratorTarget const* source, cmLinkItem const& item);

  bool EvaluateLinkProperty(
    cmGeneratorTarget const* target, std::string const& property,
    std::string const& config,
    cmGeneratorExpression::PreprocessContext preprocessContext,
    std::vector<cmLinkItem>& dependencies) const;

  bool GenerateTarget(
    cmGeneratorTarget const* target, std::string const& config,
    cmGeneratorExpression::PreprocessContext preprocessContext);
  void GenerateDeclaredLicense(cmSpdxPackage const* package,
                               std::string const& license);

  LinkInfo NoteLinkedTarget(cmGeneratorTarget const* target,
                            cmGeneratorTarget const* linkedTarget);

  std::string const PackageName;
  std::string const PackageVersion;
  std::string const PackageDescription;
  std::string const PackageWebsite;
  std::string const PackageUrl;
  std::string const DataLicense;
  std::string const DefaultLicense;
  bool const NoProjectDefaults;
  cmSbomArguments::SbomFormat const PackageFormat;
  std::vector<cmExportSet*> const ExportSets;
  cmLocalGenerator const* LocalGenerator = nullptr;

  std::set<cmGeneratorTarget const*> SbomTargets;

  cmSpdxCreationInfo CreationInfo;
  cmSpdxDocument Project;
  std::vector<cmSpdxRelationship> Relationships;

  std::vector<cmGeneratorTarget const*> Pending;
  std::map<cmGeneratorTarget const*, cmSpdxPackage const*> LocalPackages;
  std::map<std::pair<std::string, std::string>, cmSpdxPackage const*>
    LinkedPackages;
  std::map<cmGeneratorTarget const*, LinkInfo> LinkTargets;
  std::map<std::string, cmPackageInformation> Requirements;
};
