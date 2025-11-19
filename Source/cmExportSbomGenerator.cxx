/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportSbomGenerator.h"

#include <array>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/algorithm>

#include "cmArgumentParserTypes.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSbomArguments.h"
#include "cmSbomObject.h"
#include "cmSpdx.h"
#include "cmSpdxSerializer.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmSpdxPackage::PurposeId GetPurpose(cmStateEnums::TargetType type)
{
  switch (type) {
    case cmStateEnums::TargetType::EXECUTABLE:
      return cmSpdxPackage::PurposeId::APPLICATION;
    case cmStateEnums::TargetType::STATIC_LIBRARY:
    case cmStateEnums::TargetType::SHARED_LIBRARY:
    case cmStateEnums::TargetType::MODULE_LIBRARY:
    case cmStateEnums::TargetType::OBJECT_LIBRARY:
    case cmStateEnums::TargetType::INTERFACE_LIBRARY:
      return cmSpdxPackage::PurposeId::LIBRARY;
    case cmStateEnums::TargetType::UTILITY:
      return cmSpdxPackage::PurposeId::SOURCE;
    case cmStateEnums::TargetType::GLOBAL_TARGET:
    case cmStateEnums::TargetType::UNKNOWN_LIBRARY:
    default:
      return cmSpdxPackage::PurposeId::ARCHIVE;
  }
}

cmExportSbomGenerator::cmExportSbomGenerator(cmSbomArguments args)
  : PackageName(std::move(args.PackageName))
  , PackageVersion(std::move(args.Version))
  , PackageDescription(std::move(args.Description))
  , PackageWebsite(std::move(args.Website))
  , PackageLicense(std::move(args.License))
  , PackageFormat(args.GetFormat())
{
}

bool cmExportSbomGenerator::GenerateImportFile(std::ostream& os)
{
  return this->GenerateMainFile(os);
}

void cmExportSbomGenerator::WriteSbom(cmSbomDocument& doc,
                                      std::ostream& os) const
{
  switch (this->PackageFormat) {
    case cmSbomArguments::SbomFormat::SPDX_3_0_JSON:
      cmSpdxSerializer{}.WriteSbom(os, cmSbomObject(doc));
      break;
    case cmSbomArguments::SbomFormat::NONE:
      break;
  }
}

bool cmExportSbomGenerator::AddPackageInformation(
  cmSpdxPackage& artifact, std::string const& name,
  cmPackageInformation const& package) const
{
  if (name.empty()) {
    return false;
  }

  cmSpdxOrganization org;
  org.Name = name;
  artifact.OriginatedBy.emplace_back(std::move(org));

  if (package.Description) {
    artifact.Description = *package.Description;
  }

  if (package.Version) {
    artifact.PackageVersion = *package.Version;
  }

  if (package.PackageUrl) {
    artifact.PackageUrl = *package.PackageUrl;
  }

  if (package.License) {
    artifact.CopyrightText = *package.License;
  }

  artifact.BuiltTime = cmSystemTools::GetCurrentDateTime("%FT%TZ");
  cmSpdxExternalRef externalRef;
  externalRef.Locator = cmStrCat("cmake:find_package(", name, ")");
  externalRef.ExternalRefType = "buildSystem";
  return true;
}

cmSpdxDocument cmExportSbomGenerator::GenerateSbom() const
{
  cmSpdxTool tool;
  tool.SpdxId = "CMake#Agent";
  tool.Name = "CMake";

  cmSpdxCreationInfo ci;
  ci.Created = cmSystemTools::GetCurrentDateTime("%FT%TZ");
  ci.CreatedUsing = { tool };
  ci.Comment = "This SBOM was generated from the CMakeLists.txt File";

  cmSpdxDocument proj;
  proj.Name = PackageName;
  proj.SpdxId = cmStrCat(PackageName, "#SPDXDocument");
  proj.ProfileConformance = { "core", "software" };
  proj.CreationInfo = ci;

  if (!this->PackageDescription.empty()) {
    proj.Description = this->PackageDescription;
  }

  if (!this->PackageLicense.empty()) {
    proj.DataLicense = this->PackageLicense;
  }

  return proj;
}

cmSpdxPackage cmExportSbomGenerator::GenerateImportTarget(
  cmGeneratorTarget const* target) const
{
  cmSpdxPackage package;
  package.SpdxId = cmStrCat(target->GetName(), "#Package");
  package.Name = target->GetName();
  package.PrimaryPurpose = GetPurpose(target->GetType());

  cmSpdxExternalRef buildSystem;
  buildSystem.Locator = "CMake#Agent";
  buildSystem.ExternalRefType = "buildSystem";
  buildSystem.Comment = "Build System used for this target";
  package.ExternalRef = { buildSystem };

  if (!this->PackageVersion.empty()) {
    package.PackageVersion = this->PackageVersion;
  }

  if (!this->PackageWebsite.empty()) {
    package.Homepage = this->PackageWebsite;
  }

  if (!this->PackageUrl.empty()) {
    package.DownloadLocation = this->PackageUrl;
  }

  return package;
}

void cmExportSbomGenerator::GenerateLinkProperties(
  cmSbomDocument& doc, cmSpdxDocument* project, std::string const& libraries,
  TargetProperties const& current,
  std::vector<TargetProperties> const& allTargets) const
{
  auto itProp = current.Properties.find(libraries);
  if (itProp == current.Properties.end()) {
    return;
  }

  std::map<std::string, std::vector<std::string>> allowList = { { "LINK_ONLY",
                                                                  {} } };
  std::string interfaceLinkLibraries;
  if (!cmGeneratorExpression::ForbidGeneratorExpressions(
        current.Target, itProp->first, itProp->second, interfaceLinkLibraries,
        allowList)) {
    return;
  }

  auto makeRel = [&](char const* desc) {
    cmSpdxRelationship r;
    r.RelationshipType = cmSpdxRelationship::RelationshipTypeId::DEPENDS_ON;
    r.Description = desc;
    r.From = current.Package;
    return r;
  };

  auto linkLibraries = makeRel("Linked Libraries");
  auto linkRequires = makeRel("Required Runtime Libraries");
  auto buildRequires = makeRel("Required Build-Time Libraries");

  auto addArtifact =
    [&](std::string const& name) -> std::pair<bool, cmSpdxPackage const*> {
    auto it = this->LinkTargets.find(name);
    if (it != this->LinkTargets.end()) {
      LinkInfo const& linkInfo = it->second;
      if (linkInfo.Package.empty()) {
        for (auto const& t : allTargets) {
          if (t.Target->GetName() == linkInfo.Component) {
            return { true, t.Package };
          }
        }
      }
      std::string pkgName =
        cmStrCat(linkInfo.Package, ":", linkInfo.Component);
      cmSpdxPackage pkg;
      pkg.Name = pkgName;
      pkg.SpdxId = cmStrCat(pkgName, "#Package");
      if (!linkInfo.Package.empty()) {
        auto const& pkgIt = this->Requirements.find(linkInfo.Package);
        if (pkgIt != this->Requirements.end() &&
            pkgIt->second.Components.count(linkInfo.Component) > 0) {
          this->AddPackageInformation(pkg, pkgIt->first, pkgIt->second);
        }
      }

      return { true, insert_back(project->Elements, std::move(pkg)) };
    }

    cmSpdxPackage pkg;
    pkg.SpdxId = cmStrCat(name, "#Package");
    pkg.Name = name;
    return { false, insert_back(project->Elements, std::move(pkg)) };
  };

  auto handleDependencies = [&](std::vector<std::string> const& names,
                                cmSpdxRelationship& internalDeps,
                                cmSpdxRelationship& externalDeps) {
    for (auto const& n : names) {
      auto res = addArtifact(n);
      if (!res.second) {
        continue;
      }

      if (res.first) {
        internalDeps.To.push_back(res.second);
      } else {
        externalDeps.To.push_back(res.second);
      }
    }
  };

  handleDependencies(allowList["LINK_ONLY"], linkLibraries, linkRequires);
  handleDependencies(cmList{ interfaceLinkLibraries }, linkLibraries,
                     buildRequires);

  if (!linkLibraries.To.empty()) {
    insert_back(doc.Graph, std::move(linkLibraries));
  }
  if (!linkRequires.To.empty()) {
    insert_back(doc.Graph, std::move(linkRequires));
  }
  if (!buildRequires.To.empty()) {
    insert_back(doc.Graph, std::move(buildRequires));
  }
}

bool cmExportSbomGenerator::GenerateProperties(
  cmSbomDocument& doc, cmSpdxDocument* proj, TargetProperties const& current,
  std::vector<TargetProperties> const& allTargets) const
{
  this->GenerateLinkProperties(doc, proj, "LINK_LIBRARIES", current,
                               allTargets);
  this->GenerateLinkProperties(doc, proj, "INTERFACE_LINK_LIBRARIES", current,
                               allTargets);
  return true;
}

bool cmExportSbomGenerator::PopulateLinkLibrariesProperty(
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  static std::array<std::string, 3> const linkIfaceProps = {
    { "LINK_LIBRARIES", "LINK_LIBRARIES_DIRECT",
      "LINK_LIBRARIES_DIRECT_EXCLUDE" }
  };
  bool hadLINK_LIBRARIES = false;
  for (std::string const& linkIfaceProp : linkIfaceProps) {
    if (cmValue input = target->GetProperty(linkIfaceProp)) {
      std::string prepro =
        cmGeneratorExpression::Preprocess(*input, preprocessRule);
      if (!prepro.empty()) {
        this->ResolveTargetsInGeneratorExpressions(prepro, target,
                                                   ReplaceFreeTargets);
        properties[linkIfaceProp] = prepro;
        hadLINK_LIBRARIES = true;
      }
    }
  }
  return hadLINK_LIBRARIES;
}

bool cmExportSbomGenerator::NoteLinkedTarget(
  cmGeneratorTarget const* target, std::string const& linkedName,
  cmGeneratorTarget const* linkedTarget)
{
  if (cm::contains(this->ExportedTargets, linkedTarget)) {
    this->LinkTargets.emplace(linkedName,
                              LinkInfo{ "", linkedTarget->GetExportName() });
    return true;
  }

  if (linkedTarget->IsImported()) {
    using Package = cm::optional<std::pair<std::string, cmPackageInformation>>;
    auto pkgInfo = [](cmTarget* t) -> Package {
      cmFindPackageStack pkgStack = t->GetFindPackageStack();
      if (!pkgStack.Empty()) {
        return std::make_pair(pkgStack.Top().Name, pkgStack.Top().PackageInfo);
      }
      std::string const pkgName =
        t->GetSafeProperty("EXPORT_FIND_PACKAGE_NAME");
      if (pkgName.empty()) {
        return cm::nullopt;
      }
      cmPackageInformation package;
      return std::make_pair(pkgName, package);
    }(linkedTarget->Target);

    if (!pkgInfo) {
      target->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat("Target \"", target->GetName(),
                 "\" references imported target \"", linkedName,
                 "\" which does not come from any known package."));
      return false;
    }

    std::string const& pkgName = pkgInfo->first;
    auto const& prefix = cmStrCat(pkgName, "::");
    std::string component;
    if (!cmHasPrefix(linkedName, prefix)) {
      component = linkedName;
    } else {
      component = linkedName.substr(prefix.length());
    }
    this->LinkTargets.emplace(linkedName, LinkInfo{ pkgName, component });
    cmPackageInformation& req =
      this->Requirements.insert(std::move(*pkgInfo)).first->second;
    req.Components.emplace(std::move(component));

    return true;
  }

  // Target belongs to another export from this build.
  auto const& exportInfo = this->FindExportInfo(linkedTarget);
  if (exportInfo.Namespaces.size() == 1 && exportInfo.Sets.size() == 1) {
    auto const& linkNamespace = *exportInfo.Namespaces.begin();
    if (!cmHasSuffix(linkNamespace, "::")) {
      target->Makefile->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName,
                 "\", which does not use the standard namespace separator. "
                 "This is not allowed."));
    }

    std::string pkgName{ linkNamespace.data(), linkNamespace.size() - 2 };
    std::string component = linkedTarget->GetExportName();
    if (pkgName == this->GetPackageName()) {
      this->LinkTargets.emplace(linkedName, LinkInfo{ "", component });
    } else {
      this->LinkTargets.emplace(linkedName, LinkInfo{ pkgName, component });
      this->Requirements[pkgName].Components.emplace(std::move(component));
    }
    return true;
  }

  // Target belongs to multiple namespaces or multiple export sets.
  // cmExportFileGenerator::HandleMissingTarget should have complained about
  // this already.
  return false;
}
