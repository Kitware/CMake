/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmSbomArguments.h"

class cmGeneratorTarget;
struct cmSbomDocument;
struct cmSpdxDocument;
struct cmSpdxPackage;

class cmExportSbomGenerator : virtual public cmExportFileGenerator
{
public:
  cmExportSbomGenerator(cmSbomArguments args);
  using cmExportFileGenerator::GenerateImportFile;

protected:
  using ImportPropertyMap = std::map<std::string, std::string>;

  struct TargetProperties
  {
    cmSpdxPackage const* Package;
    cmGeneratorTarget const* Target;
    ImportPropertyMap Properties;
  };

  void WriteSbom(cmSbomDocument& doc, std::ostream& os) const;

  cmSpdxDocument GenerateSbom() const;
  cmSpdxPackage GenerateImportTarget(cmGeneratorTarget const* target) const;

  std::string const& GetPackageName() const { return this->PackageName; }

  bool GenerateImportFile(std::ostream& os) override;
  bool AddPackageInformation(cmSpdxPackage& artifact, std::string const& name,
                             cmPackageInformation const& package) const;

  bool GenerateProperties(
    cmSbomDocument& doc, cmSpdxDocument* project,
    TargetProperties const& current,
    std::vector<TargetProperties> const& allTargets) const;

  void GenerateLinkProperties(
    cmSbomDocument& doc, cmSpdxDocument* project, std::string const& libraries,
    TargetProperties const& current,
    std::vector<TargetProperties> const& allTargets) const;

  bool NoteLinkedTarget(cmGeneratorTarget const* target,
                        std::string const& linkedName,
                        cmGeneratorTarget const* linkedTarget) override;

  bool PopulateLinkLibrariesProperty(cmGeneratorTarget const* target,
                                     cmGeneratorExpression::PreprocessContext,
                                     ImportPropertyMap& properties);

private:
  struct LinkInfo
  {
    std::string Package;
    std::string Component;
  };

  std::string const PackageName;
  std::string const PackageVersion;
  std::string const PackageDescription;
  std::string const PackageWebsite;
  std::string const PackageLicense;
  std::string const PackageUrl;
  cmSbomArguments::SbomFormat const PackageFormat;
  std::map<std::string, LinkInfo> LinkTargets;
  std::map<std::string, cmPackageInformation> Requirements;
};
