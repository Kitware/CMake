/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSbomBuilder.h"

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmArgumentParserTypes.h"
#include "cmDiagnostics.h"
#include "cmExportFileGenerator.h"
#include "cmExportSet.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
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
#include "cmTargetExport.h"
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

cmSbomBuilder::cmSbomBuilder(cmSbomArguments args,
                             std::vector<cmExportSet*> exportSets,
                             cmLocalGenerator* lg)
  : LocalGenerator(lg)
  , ExportSets(std::move(exportSets))
  , PackageName(std::move(args.PackageName))
  , Namespace(cmStrCat(this->PackageName, "::"_s))
  , PackageVersion(std::move(args.Version))
  , PackageDescription(std::move(args.Description))
  , PackageWebsite(std::move(args.Website))
  , PackageUrl(std::move(args.PackageUrl))
  , DataLicense(std::move(args.DataLicense))
  , DefaultLicense(std::move(args.DefaultLicense))
  , PackageFormat(args.GetFormat())
{
}

std::set<cmGeneratorTarget const*> cmSbomBuilder::CollectTargets() const
{
  std::set<cmGeneratorTarget const*> targets;
  for (cmExportSet* exportSet : this->ExportSets) {
    for (auto const& te : exportSet->GetTargetExports()) {
      if (cmGeneratorTarget const* gt =
            this->LocalGenerator->FindGeneratorTargetToUse(te->TargetName)) {
        targets.emplace(gt);
      }
    }
  }
  return targets;
}

void cmSbomBuilder::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  if (!lg) {
    return;
  }
  for (cmExportSet* es : this->ExportSets) {
    es->Compute(lg);
  }
  // Populate the cache now (rather than at Generate time) so peer SBOMs can
  // query CoversTarget() during their own NoteLinkedTarget walks.
  this->SbomTargets = this->CollectTargets();
}

bool cmSbomBuilder::CoversTarget(cmGeneratorTarget const* target) const
{
  return cm::contains(this->SbomTargets, target);
}

bool cmSbomBuilder::CoversExportSet(cmExportSet const* set) const
{
  return std::find(this->ExportSets.cbegin(), this->ExportSets.cend(), set) !=
    this->ExportSets.cend();
}

bool cmSbomBuilder::GenerateForTargets(
  std::ostream& os, std::string const& config,
  cmGeneratorExpression::PreprocessContext preprocessContext)
{
  cmSbomDocument doc;
  doc.Graph.reserve(512);

  cmSpdxCreationInfo const* ci =
    insert_back(doc.Graph, this->GenerateCreationInfo());
  cmSpdxDocument* project = insert_back(doc.Graph, this->GenerateSbom(ci));
  std::vector<TargetProperties> targetProps;
  targetProps.reserve(this->SbomTargets.size());

  for (cmGeneratorTarget const* target : this->SbomTargets) {
    ImportPropertyMap properties;
    this->PopulateLinkLibrariesProperty(target, preprocessContext, properties);
    this->PopulateInterfaceLinkLibrariesProperty(target, preprocessContext,
                                                 properties);
    targetProps.push_back(TargetProperties{
      insert_back(project->RootElements,
                  this->GenerateImportTarget(ci, target)),
      target,
      std::move(properties),
    });
  }

  bool status = true;
  for (TargetProperties const& target : targetProps) {
    status &=
      this->GenerateProperties(doc, project, ci, target, targetProps, config);
  }
  if (status) {
    this->WriteSbom(doc, os);
  }
  return status;
}

void cmSbomBuilder::WriteSbom(cmSbomDocument& doc, std::ostream& os) const
{
  switch (this->PackageFormat) {
    case cmSbomArguments::SbomFormat::SPDX_3_0_JSON:
      cmSpdxSerializer{}.WriteSbom(os, cmSbomObject(doc));
      break;
    case cmSbomArguments::SbomFormat::NONE:
      break;
  }
}

bool cmSbomBuilder::AddPackageInformation(
  cmSpdxPackage& artifact, std::string const& name,
  cmPackageInformation const& package) const
{
  if (name.empty()) {
    return false;
  }

  cmSpdxOrganization org;
  org.SpdxId = cmStrCat("urn:", name, "#Organization");
  org.Name = name;
  org.CreationInfo = artifact.CreationInfo;
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

cmSpdxCreationInfo cmSbomBuilder::GenerateCreationInfo() const
{
  cmSpdxCreationInfo ci;
  ci.SpdxId = "_:Build#CreationInfo";
  ci.Created = cmSystemTools::GetCurrentDateTime("%FT%TZ");
  ci.CreatedBy = { "https://gitlab.kitware.com/cmake/cmake" };
  ci.Comment = "This SBOM was generated from the CMakeLists.txt File";
  ci.SpecVersion = "3.0.1";
  return ci;
}

cmSpdxDocument cmSbomBuilder::GenerateSbom(cmSpdxCreationInfo const* ci) const
{
  cmSpdxDocument proj;
  proj.Name = PackageName;
  proj.SpdxId = cmStrCat("urn:", PackageName, "#SPDXDocument");
  proj.ProfileConformance = { "core", "software" };
  proj.CreationInfo = ci;

  if (!this->PackageDescription.empty()) {
    proj.Description = this->PackageDescription;
  }

  if (!this->DataLicense.empty()) {
    cmSpdxLicenseExpression license;
    license.SpdxId = cmStrCat("urn:", PackageName, "#LicenseExpression");
    license.CreationInfo = ci;
    license.LicenseExpression = this->DataLicense;
    proj.DataLicense = license;
  }

  return proj;
}

cmSpdxPackage cmSbomBuilder::GenerateImportTarget(
  cmSpdxCreationInfo const* ci, cmGeneratorTarget const* target) const
{
  cmSpdxPackage package;
  package.SpdxId = cmStrCat("urn:", target->GetName(), "#Package");
  package.Name = target->GetName();
  package.PrimaryPurpose = GetPurpose(target->GetType());
  package.CreationInfo = ci;

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

bool cmSbomBuilder::GenerateLinkProperties(
  cmSbomDocument& doc, cmSpdxDocument* project, cmSpdxCreationInfo const* ci,
  std::string const& libraries, TargetProperties const& current,
  std::vector<TargetProperties> const& allTargets,
  std::string const& config) const
{
  auto itProp = current.Properties.find(libraries);
  if (itProp == current.Properties.end()) {
    return true;
  }

  cmGeneratorExpression ge(*current.Target->Makefile->GetCMakeInstance());
  std::unique_ptr<cmCompiledGeneratorExpression> cge =
    ge.Parse(itProp->second);
  std::string evaluatedLibraries =
    cge->Evaluate(current.Target->GetLocalGenerator(), config, current.Target);

  if (cge->GetHadHeadSensitiveCondition()) {
    current.Target->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Property \"", libraries, "\" of target \"",
               current.Target->GetName(),
               "\" contains a generator expression that is not allowed for "
               "SBOM generation."));
    return false;
  }

  auto makeRel = [&](char const* id, char const* desc) {
    cmSpdxRelationship r;
    r.SpdxId = cmStrCat("urn:", id, "#Relationship");
    r.RelationshipType = cmSpdxRelationship::RelationshipTypeId::DEPENDS_ON;
    r.Description = desc;
    r.From = current.Package;
    r.CreationInfo = ci;
    return r;
  };

  auto linkLibraries = makeRel("Static", "Linked Libraries");
  auto linkRequires = makeRel("Dynamic", "Required Runtime Libraries");
  auto buildRequires = makeRel("Shared", "Required Build-Time Libraries");

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
      pkg.SpdxId = cmStrCat("urn:", pkgName, "#Package");
      pkg.CreationInfo = ci;
      if (!linkInfo.Package.empty()) {
        auto const& pkgIt = this->Requirements.find(linkInfo.Package);
        if (pkgIt != this->Requirements.end() &&
            pkgIt->second.Components.count(linkInfo.Component) > 0) {
          this->AddPackageInformation(pkg, pkgIt->first, pkgIt->second);
        }
      }

      cmSpdxPackage const* pkgPtr =
        insert_back(project->Elements, std::move(pkg));
      if (!linkInfo.License.empty() &&
          !cm::contains(this->GeneratedLinkLicenses, name)) {
        this->GeneratedLinkLicenses.emplace(name);

        cmSpdxLicenseExpression license;
        license.SpdxId = cmStrCat("urn:", name, "#LicenseExpression");
        license.CreationInfo = ci;
        license.LicenseExpression = linkInfo.License;

        cmSpdxRelationship relHasLicense;
        relHasLicense.SpdxId =
          cmStrCat("urn:", name, "#DeclaredLicenseRelationship");
        relHasLicense.CreationInfo = ci;
        relHasLicense.RelationshipType =
          cmSpdxRelationship::HAS_DECLARED_LICENSE;
        relHasLicense.From = pkgPtr;
        relHasLicense.To.emplace_back(std::move(license));

        insert_back(doc.Graph, std::move(relHasLicense));
      }

      return { true, pkgPtr };
    }

    cmSpdxPackage pkg;
    pkg.SpdxId = cmStrCat("urn:", name, "#Package");
    pkg.Name = name;
    pkg.CreationInfo = ci;
    return { false, insert_back(project->Elements, std::move(pkg)) };
  };

  cmList names{ evaluatedLibraries };
  names.sort();
  names.remove_duplicates();
  for (std::string const& n : names) {
    auto res = addArtifact(n);
    if (!res.second) {
      continue;
    }

    if (res.first) {
      linkLibraries.To.push_back(res.second);
    } else {
      buildRequires.To.push_back(res.second);
    }
  }

  if (!linkLibraries.To.empty()) {
    insert_back(doc.Graph, std::move(linkLibraries));
  }
  if (!linkRequires.To.empty()) {
    insert_back(doc.Graph, std::move(linkRequires));
  }
  if (!buildRequires.To.empty()) {
    insert_back(doc.Graph, std::move(buildRequires));
  }
  return true;
}

bool cmSbomBuilder::GenerateProperties(
  cmSbomDocument& doc, cmSpdxDocument* proj, cmSpdxCreationInfo const* ci,
  TargetProperties const& current,
  std::vector<TargetProperties> const& allTargets,
  std::string const& config) const
{
  bool status = true;
  status &= this->GenerateLinkProperties(doc, proj, ci, "LINK_LIBRARIES",
                                         current, allTargets, config);
  status &= this->GenerateLinkProperties(
    doc, proj, ci, "INTERFACE_LINK_LIBRARIES", current, allTargets, config);
  status &= this->GenerateMetaProperties(doc, proj, ci, current);
  return status;
}

bool cmSbomBuilder::GenerateMetaProperties(
  cmSbomDocument& doc, cmSpdxDocument* /*project*/,
  cmSpdxCreationInfo const* ci, TargetProperties const& current) const
{
  std::string licenseExpr = this->DefaultLicense;
  cmValue licenseExprProp = current.Target->GetProperty("SPDX_LICENSE");
  if (licenseExprProp) {
    licenseExpr = licenseExprProp;
  }
  if (!licenseExpr.empty()) {
    auto const& tgtName = current.Target->GetName();

    cmSpdxLicenseExpression license;
    license.SpdxId = cmStrCat("urn:", tgtName, "#LicenseExpression");
    license.CreationInfo = ci;
    license.LicenseExpression = std::move(licenseExpr);

    cmSpdxRelationship relHasLicense;
    relHasLicense.SpdxId =
      cmStrCat("urn:", tgtName, "#DeclaredLicenseRelationship");
    relHasLicense.CreationInfo = ci;
    relHasLicense.RelationshipType = cmSpdxRelationship::HAS_DECLARED_LICENSE;
    relHasLicense.From = current.Package;
    relHasLicense.To.emplace_back(std::move(license));

    insert_back(doc.Graph, std::move(relHasLicense));
  }
  return true;
}

bool cmSbomBuilder::PopulateLinkLibrariesProperty(
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  static std::array<std::string, 3> const linkIfaceProps = { {
    "LINK_LIBRARIES",
    "LINK_LIBRARIES_DIRECT",
    "LINK_LIBRARIES_DIRECT_EXCLUDE",
  } };
  bool hadLINK_LIBRARIES = false;
  for (std::string const& linkIfaceProp : linkIfaceProps) {
    if (cmValue input = target->GetProperty(linkIfaceProp)) {
      std::string prepro =
        cmGeneratorExpression::Preprocess(*input, preprocessRule);
      if (!prepro.empty()) {
        this->ResolveTargetsInGeneratorExpressions(prepro, target);
        properties[linkIfaceProp] = prepro;
        hadLINK_LIBRARIES = true;
      }
    }
  }
  return hadLINK_LIBRARIES;
}

bool cmSbomBuilder::AddTargetNamespace(std::string& input,
                                       cmGeneratorTarget const* target,
                                       cmLocalGenerator const* lg)
{
  cmGeneratorTarget::TargetOrString resolved =
    target->ResolveTargetReference(input, lg);

  cmGeneratorTarget* tgt = resolved.Target;
  if (!tgt) {
    input = resolved.String;
    return false;
  }

  if (tgt->IsImported()) {
    input = tgt->GetName();
    return this->NoteLinkedTarget(target, input, tgt);
  }

  if (this->SbomTargets.find(tgt) != this->SbomTargets.end()) {
    input = this->Namespace + tgt->GetExportName();
  } else {
    input = tgt->GetName();
  }

  return this->NoteLinkedTarget(target, input, tgt);
}

bool cmSbomBuilder::NoteLinkedTarget(cmGeneratorTarget const* target,
                                     std::string const& linkedName,
                                     cmGeneratorTarget const* linkedTarget)
{
  auto linkedLicense = linkedTarget->GetSafeProperty("SPDX_LICENSE");

  if (cm::contains(this->SbomTargets, linkedTarget)) {
    this->LinkTargets.emplace(
      linkedName,
      LinkInfo{ "", linkedTarget->GetExportName(), linkedLicense });
    return true;
  }

  if (linkedTarget->IsImported()) {
    using Package = cm::optional<std::pair<std::string, cmPackageInformation>>;
    auto pkgInfo = [](cmTarget* t) -> Package {
      cmFindPackageStack pkgStack = t->GetFindPackageStack();
      if (!pkgStack.Empty()) {
        return std::make_pair(pkgStack.Top().Name,
                              *pkgStack.Top().PackageInfo);
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
      target->Makefile->IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
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
    this->LinkTargets.emplace(linkedName,
                              LinkInfo{ pkgName, component, linkedLicense });
    cmPackageInformation& req =
      this->Requirements.insert(std::move(*pkgInfo)).first->second;
    req.Components.emplace(std::move(component));
    return true;
  }

  // Target belongs to another export from this build or install.
  // The leaf class chooses which export map to consult.
  auto const& exportInfo = this->FindExportInfoFor(linkedTarget);
  if (exportInfo.Namespaces.size() == 1 && exportInfo.Sets.size() == 1) {
    auto const& linkNamespace = *exportInfo.Namespaces.begin();
    if (!cmHasSuffix(linkNamespace, "::")) {
      target->Makefile->IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName,
                 "\", whose export does not use the standard namespace "
                 "separator.  The dependency will be recorded by its bare "
                 "target name without provenance."));
      return false;
    }

    std::string pkgName{ linkNamespace.data(), linkNamespace.size() - 2 };
    std::string component = linkedTarget->GetExportName();
    if (pkgName == this->GetPackageName()) {
      this->LinkTargets.emplace(linkedName,
                                LinkInfo{ "", component, linkedLicense });
    } else {
      this->LinkTargets.emplace(linkedName,
                                LinkInfo{ pkgName, component, linkedLicense });
      this->Requirements[pkgName].Components.emplace(std::move(component));
    }
    return true;
  }

  if (exportInfo.Sets.empty()) {
    // install(export) provenance is unavailable.  Fall back to SBOM-level
    // attribution: any peer SBOM (of the same build/install mode) that
    // covers this target lends its package name.  install(export) wins
    // when both are available; only reached here when install(export) is
    // absent.
    auto const& sbomInfo = this->FindSbomInfoFor(linkedTarget);
    if (sbomInfo.Packages.empty()) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName,
                 "\" which has no install(EXPORT)/export(EXPORT) namespace "
                 "and is not covered by any SBOM.  An SBOM cannot attribute "
                 "this dependency.  Give \"",
                 linkedTarget->GetName(),
                 "\" an install(EXPORT)/export(EXPORT) with a NAMESPACE, or "
                 "include it in an install(SBOM)/export(SBOM)."));
      return false;
    }
    std::string const& pkgName = sbomInfo.Packages.front();
    if (sbomInfo.Packages.size() > 1) {
      target->Makefile->IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
        cmStrCat(
          "Target \"", target->GetName(), "\" references target \"",
          linkedName,
          "\" which has no install(EXPORT)/export(EXPORT) namespace and is "
          "covered by multiple SBOMs: ",
          cmJoin(sbomInfo.Packages, ", "), ".  Attributing to \"", pkgName,
          "\" (first alphabetically)."));
    }
    std::string component = linkedTarget->GetExportName();
    this->LinkTargets.emplace(linkedName,
                              LinkInfo{ pkgName, component, linkedLicense });
    this->Requirements[pkgName].Components.emplace(std::move(component));
    return true;
  }

  std::ostringstream e;
  e << "Target \"" << target->GetName() << "\" references target \""
    << linkedName << "\" ";
  if (exportInfo.Sets.size() == 1) {
    e << "that is in an export set which is exported multiple times "
         "with different namespaces: ";
  } else {
    e << "that is in multiple export sets: ";
  }
  e << cmJoin(exportInfo.Files, ", ") << ".\n"
    << "An SBOM cannot attribute a dependency exported in more than one "
       "export set or with more than one namespace.  Consider "
       "consolidating the exports of the \""
    << linkedTarget->GetName() << "\" target to a single export.";
  target->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
  return false;
}

void cmSbomBuilder::ResolveTargetsInGeneratorExpression(
  std::string& input, cmGeneratorTarget const* target,
  cmLocalGenerator const* lg)
{
  auto err = cmResolveTargetsInGeneratorExpression(
    input, [this, target, lg](std::string& name) {
      return this->AddTargetNamespace(name, target, lg);
    });
  if (err) {
    target->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR, *err);
  }
}

void cmSbomBuilder::ResolveTargetsInGeneratorExpressions(
  std::string& input, cmGeneratorTarget const* target)
{
  cmLocalGenerator const* lg = target->GetLocalGenerator();
  std::vector<std::string> parts;
  cmGeneratorExpression::Split(input, parts);

  std::string sep;
  input.clear();
  for (std::string& li : parts) {
    if (target->IsLinkLookupScope(li, lg)) {
      continue;
    }
    if (cmGeneratorExpression::Find(li) == std::string::npos) {
      this->AddTargetNamespace(li, target, lg);
    } else {
      this->ResolveTargetsInGeneratorExpression(li, target, lg);
    }
    input += sep + li;
    sep = ";";
  }
}

bool cmSbomBuilder::PopulateInterfaceLinkLibrariesProperty(
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  if (!target->IsLinkable()) {
    return false;
  }
  static std::array<std::string, 3> const linkIfaceProps = {
    { "INTERFACE_LINK_LIBRARIES", "INTERFACE_LINK_LIBRARIES_DIRECT",
      "INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE" }
  };
  bool hadINTERFACE_LINK_LIBRARIES = false;
  for (std::string const& linkIfaceProp : linkIfaceProps) {
    if (cmValue input = target->GetProperty(linkIfaceProp)) {
      std::string prepro =
        cmGeneratorExpression::Preprocess(*input, preprocessRule);
      if (!prepro.empty()) {
        this->ResolveTargetsInGeneratorExpressions(prepro, target);
        properties[linkIfaceProp] = prepro;
        hadINTERFACE_LINK_LIBRARIES = true;
      }
    }
  }
  return hadINTERFACE_LINK_LIBRARIES;
}
