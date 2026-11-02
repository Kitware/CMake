/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSbomBuilder.h"

#include <cstddef>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/algorithm>

#include "cmArgumentParserTypes.h"
#include "cmDiagnostics.h"
#include "cmExportSet.h"
#include "cmFindPackageStack.h"
#include "cmGenExContext.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmSbomArguments.h"
#include "cmSbomObject.h"
#include "cmSpdx.h"
#include "cmSpdxSerializer.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"
#include "cmTargetTypes.h"
#include "cmValue.h"

cmSpdxPackage::PurposeId GetPurpose(cm::TargetType const type)
{
  switch (type) {
    case cm::TargetType::EXECUTABLE:
      return cmSpdxPackage::PurposeId::APPLICATION;
    case cm::TargetType::STATIC_LIBRARY:
    case cm::TargetType::SHARED_LIBRARY:
    case cm::TargetType::MODULE_LIBRARY:
    case cm::TargetType::OBJECT_LIBRARY:
    case cm::TargetType::INTERFACE_LIBRARY:
      return cmSpdxPackage::PurposeId::LIBRARY;
    case cm::TargetType::UTILITY:
      return cmSpdxPackage::PurposeId::SOURCE;
    case cm::TargetType::GLOBAL_TARGET:
    case cm::TargetType::UNKNOWN_LIBRARY:
    default:
      return cmSpdxPackage::PurposeId::ARCHIVE;
  }
}

cmSbomBuilder::cmSbomBuilder(cmSbomArguments const& args,
                             std::vector<cmExportSet*> const& exportSets)
  : PackageName(args.GetPackageName())
  , PackageVersion(args.Version)
  , PackageDescription(args.Description)
  , PackageWebsite(args.Website)
  , PackageUrl(args.PackageUrl)
  , DataLicense(args.GetDataLicense())
  , DefaultLicense(args.DefaultLicense)
  , NoProjectDefaults(args.NoProjectDefaults)
  , PackageFormat(args.GetFormat())
  , ExportSets(exportSets)
{
}

bool cmSbomBuilder::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  this->SbomTargets.clear();
  for (cmExportSet* exportSet : this->ExportSets) {
    if (!exportSet->Compute(lg)) {
      return false;
    }
  }
  this->SbomTargets = this->CollectTargets();
  return true;
}

std::set<cmGeneratorTarget const*> cmSbomBuilder::CollectTargets() const
{
  std::set<cmGeneratorTarget const*> targets;
  for (cmExportSet const* exportSet : this->ExportSets) {
    for (auto const& te : exportSet->GetTargetExports()) {
      targets.emplace(te->Target);
    }
  }
  return targets;
}

bool cmSbomBuilder::CoversTarget(cmGeneratorTarget const* target) const
{
  return cm::contains(this->SbomTargets, target);
}

bool cmSbomBuilder::CoversExportSet(cmExportSet const* set) const
{
  return cm::contains(this->ExportSets, set);
}

bool cmSbomBuilder::GenerateForTargets(
  std::ostream& os, std::string const& config,
  cmGeneratorExpression::PreprocessContext const preprocessContext)
{
  if (!this->LocalGenerator) {
    return false;
  }

  this->Relationships.clear();
  this->Pending.clear();
  this->LocalPackages.clear();
  this->LinkedPackages.clear();
  this->LinkTargets.clear();
  this->Requirements.clear();
  this->CreationInfo = this->GenerateCreationInfo();
  this->Project = this->GenerateSbom();

  this->Pending.reserve(this->SbomTargets.size());
  for (cmGeneratorTarget const* target : this->SbomTargets) {
    this->GetOrCreateLocalPackage(target);
  }

  bool status = true;
  // GenerateTarget can append to Pending while this loop is running.
  // NOLINTNEXTLINE(modernize-loop-convert)
  for (std::size_t i = 0; i < this->Pending.size(); ++i) {
    auto const* const target = this->Pending[i];
    status &= this->GenerateTarget(target, config, preprocessContext);
  }
  return status && this->WriteSbom(os);
}

bool cmSbomBuilder::WriteSbom(std::ostream& os) const
{
  cmSbomDocument document;
  document.Graph.reserve(2 + this->Relationships.size());
  document.Graph.emplace_back(this->CreationInfo);
  document.Graph.emplace_back(this->Project);
  for (auto const& relationship : this->Relationships) {
    document.Graph.emplace_back(relationship);
  }

  switch (this->PackageFormat) {
    case cmSbomArguments::SbomFormat::SPDX_3_0_JSON:
      return cmSpdxSerializer{}.WriteSbom(os,
                                          cmSbomObject(std::move(document)));
    case cmSbomArguments::SbomFormat::NONE:
      break;
  }
  return false;
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
  externalRef.Locator = cmStrCat("cmake:find_package(", name, ')');
  externalRef.ExternalRefType = "buildSystem";
  artifact.ExternalRef.emplace_back(std::move(externalRef));
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

cmSpdxDocument cmSbomBuilder::GenerateSbom() const
{
  cmSpdxDocument proj;
  proj.Name = this->PackageName;
  proj.SpdxId = cmStrCat("urn:", this->PackageName, "#SPDXDocument");
  proj.ProfileConformance = { "core", "software" };
  proj.CreationInfo = &this->CreationInfo;

  if (!this->PackageDescription.empty()) {
    proj.Description = this->PackageDescription;
  }

  if (!this->DataLicense.empty()) {
    cmSpdxLicenseExpression licenseObject;
    licenseObject.SpdxId =
      cmStrCat("urn:", this->PackageName, "#DataLicenseExpression");
    licenseObject.CreationInfo = &this->CreationInfo;
    licenseObject.LicenseExpression = this->DataLicense;
    proj.DataLicense = std::move(licenseObject);
  }

  return proj;
}

cmSpdxPackage cmSbomBuilder::GenerateLocalPackage(
  cmGeneratorTarget const* target) const
{
  cmSpdxPackage package;
  package.SpdxId = cmStrCat("urn:", target->GetName(), "#Package");
  package.Name = target->GetName();
  package.PrimaryPurpose = GetPurpose(target->GetType());
  package.CreationInfo = &this->CreationInfo;

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

cmSpdxPackage const* cmSbomBuilder::GetOrCreateLocalPackage(
  cmGeneratorTarget const* target)
{
  auto const it = this->LocalPackages.find(target);
  if (it != this->LocalPackages.end()) {
    return it->second;
  }

  auto& elements = cm::contains(this->SbomTargets, target)
    ? this->Project.RootElements
    : this->Project.Elements;
  auto const* package =
    insert_back(elements, this->GenerateLocalPackage(target));
  this->LocalPackages.emplace(target, package);
  this->Pending.push_back(target);
  return package;
}

cmSpdxPackage const* cmSbomBuilder::GetOrCreateLinkedPackage(
  cmGeneratorTarget const* source, cmLinkItem const& item)
{
  LinkInfo const* info = nullptr;
  if (item.Target) {
    auto link = this->LinkTargets.find(item.Target);
    if (link == this->LinkTargets.end()) {
      link =
        this->LinkTargets
          .emplace(item.Target, this->NoteLinkedTarget(source, item.Target))
          .first;
    }
    info = &link->second;
  }
  if (info && info->Local) {
    return this->GetOrCreateLocalPackage(item.Target);
  }

  // An empty package name identifies a dependency without provenance.
  auto const& name = item.AsStr();
  auto key = std::make_pair(info ? info->Package : std::string{},
                            info ? info->Component : name);
  auto const it = this->LinkedPackages.find(key);
  if (it != this->LinkedPackages.end()) {
    return it->second;
  }

  std::string const packageName = info && !info->Package.empty()
    ? cmStrCat(info->Package, ':', info->Component)
    : name;
  cmSpdxPackage package;
  package.Name = packageName;
  package.SpdxId = cmStrCat("urn:", packageName, "#Package");
  package.CreationInfo = &this->CreationInfo;
  if (info) {
    auto const requirement = this->Requirements.find(info->Package);
    if (requirement != this->Requirements.end()) {
      this->AddPackageInformation(package, info->Package, requirement->second);
    }
  }

  auto const* result = insert_back(this->Project.Elements, std::move(package));
  this->LinkedPackages.emplace(std::move(key), result);
  if (item.Target) {
    this->GenerateDeclaredLicense(
      result, item.Target->GetSafeProperty("SPDX_LICENSE"));
  }
  return result;
}

bool cmSbomBuilder::EvaluateLinkProperty(
  cmGeneratorTarget const* target, std::string const& property,
  std::string const& config,
  cmGeneratorExpression::PreprocessContext preprocessContext,
  std::vector<cmLinkItem>& dependencies) const
{
  cmValue const input = target->GetProperty(property);
  if (!input) {
    return true;
  }
  auto const preprocessed =
    cmGeneratorExpression::Preprocess(*input, preprocessContext);
  if (preprocessed.empty()) {
    return true;
  }

  cmGeneratorExpression const ge(*target->Makefile->GetCMakeInstance());
  auto const cge = ge.Parse(preprocessed);
  cm::GenEx::Context const context(target->GetLocalGenerator(), config);
  for (auto const usage :
       { cmGeneratorTarget::UseTo::Compile, cmGeneratorTarget::UseTo::Link }) {
    cmGeneratorExpressionDAGChecker dagChecker{ target, property, nullptr,
                                                nullptr, context };
    if (usage == cmGeneratorTarget::UseTo::Compile) {
      dagChecker.SetTransitivePropertiesOnly();
      if (property != "LINK_LIBRARIES" ||
          target->GetPolicyStatusCMP0131() == cmPolicies::NEW) {
        dagChecker.SetTransitivePropertiesOnlyCMP0131();
      }
    }
    cmList const items{ cge->Evaluate(context, &dagChecker, target) };
    if (cge->GetHadHeadSensitiveCondition()) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Property \"", property, "\" of target \"", target->GetName(),
                 "\" contains a generator expression that is not allowed for "
                 "SBOM generation."));
      return false;
    }

    cmLocalGenerator const* scope = target->GetLocalGenerator();
    auto feature = cmLinkItem::DEFAULT;
    for (auto const& item : items) {
      if (auto linkFeature = ParseLinkFeature(item)) {
        feature = std::move(*linkFeature);
        continue;
      }
      if ((cmHasPrefix(item, "<LINK_GROUP:") ||
           cmHasPrefix(item, "</LINK_GROUP:")) &&
          cmHasSuffix(item, '>')) {
        continue;
      }
      if (item.empty() || target->IsLinkLookupScope(item, scope)) {
        continue;
      }
      dependencies.emplace_back(target->ResolveLinkItem(
        BT<std::string>(item, cmListFileBacktrace{}), scope, feature));
    }
  }
  return true;
}

bool cmSbomBuilder::GenerateTarget(
  cmGeneratorTarget const* target, std::string const& config,
  cmGeneratorExpression::PreprocessContext preprocessContext)
{
  auto const* package = this->LocalPackages.at(target);
  std::vector<cmLinkItem> dependencies;
  bool status = this->EvaluateLinkProperty(target, "LINK_LIBRARIES", config,
                                           preprocessContext, dependencies);
  status &=
    this->EvaluateLinkProperty(target, "INTERFACE_LINK_LIBRARIES", config,
                               preprocessContext, dependencies);

  cmSpdxRelationship staticLibraries;
  cmSpdxRelationship sharedLibraries;
  std::set<cmSpdxPackage const*> seen;
  for (auto const& item : dependencies) {
    auto const* dependency = this->GetOrCreateLinkedPackage(target, item);
    if (!seen.insert(dependency).second) {
      continue;
    }
    auto const type =
      item.Target ? item.Target->GetType() : cm::TargetType::UNKNOWN_LIBRARY;
    switch (type) {
      case cm::TargetType::INTERFACE_LIBRARY:
      case cm::TargetType::STATIC_LIBRARY:
        staticLibraries.To.emplace_back(dependency);
        break;
      case cm::TargetType::SHARED_LIBRARY:
      case cm::TargetType::MODULE_LIBRARY:
        sharedLibraries.To.emplace_back(dependency);
        break;
      default:
        break;
    }
  }

  auto const addRelationship = [&](cmSpdxRelationship& relationship,
                                   char const* id, char const* description) {
    if (relationship.To.empty()) {
      return;
    }
    relationship.SpdxId =
      cmStrCat("urn:", *package->Name, '#', id, "Relationship");
    relationship.RelationshipType = cmSpdxRelationship::DEPENDS_ON;
    relationship.Description = description;
    relationship.From = package;
    relationship.CreationInfo = &this->CreationInfo;
    this->Relationships.emplace_back(std::move(relationship));
  };

  addRelationship(staticLibraries, "Static", "Required Build-Time Libraries");
  addRelationship(sharedLibraries, "Shared", "Required Run-Time Libraries");

  cmValue license = target->GetProperty("SPDX_LICENSE");
  if (!license && !this->DefaultLicense.empty()) {
    license = cmValue(this->DefaultLicense);
  }
  if (!license && !this->NoProjectDefaults) {
    cmMakefile const* mf = target->Target->GetMakefile();
    std::string const projectName = mf->GetStateSnapshot().GetProjectName();
    license = mf->GetDefinition(cmStrCat(projectName, "_SPDX_LICENSE"));
  }

  this->GenerateDeclaredLicense(package, *license);
  return status;
}

void cmSbomBuilder::GenerateDeclaredLicense(cmSpdxPackage const* package,
                                            std::string const& license)
{
  if (license.empty()) {
    return;
  }
  cmSpdxLicenseExpression expression;
  expression.SpdxId = cmStrCat("urn:", *package->Name, "#LicenseExpression");
  expression.CreationInfo = &this->CreationInfo;
  expression.LicenseExpression = license;

  cmSpdxRelationship relationship;
  relationship.SpdxId =
    cmStrCat("urn:", *package->Name, "#DeclaredLicenseRelationship");
  relationship.CreationInfo = &this->CreationInfo;
  relationship.RelationshipType = cmSpdxRelationship::HAS_DECLARED_LICENSE;
  relationship.From = package;
  relationship.To.emplace_back(std::move(expression));
  this->Relationships.emplace_back(std::move(relationship));
}

cmSbomBuilder::LinkInfo cmSbomBuilder::NoteLinkedTarget(
  cmGeneratorTarget const* target, cmGeneratorTarget const* linkedTarget)
{
  auto const& linkedName = linkedTarget->GetName();
  if (cm::contains(this->SbomTargets, linkedTarget)) {
    return { "", linkedTarget->GetExportName(), true };
  }

  if (linkedTarget->IsImported()) {
    using Package = cm::optional<std::pair<std::string, cmPackageInformation>>;
    auto pkgInfo = [](cmTarget const* t) -> Package {
      cmFindPackageStack const& pkgStack = t->GetFindPackageStack();
      if (!pkgStack.Empty()) {
        return std::make_pair(pkgStack.Top().Name,
                              *pkgStack.Top().PackageInfo);
      }
      std::string pkgName = t->GetSafeProperty("EXPORT_FIND_PACKAGE_NAME");
      if (pkgName.empty()) {
        return cm::nullopt;
      }
      return std::make_pair(std::move(pkgName), cmPackageInformation{});
    }(linkedTarget->Target);

    if (!pkgInfo) {
      target->Makefile->IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
        cmStrCat("Target \"", target->GetName(),
                 "\" references imported target \"", linkedName,
                 "\" which does not come from any known package."));
      return { "", linkedName };
    }

    std::string const& pkgName = pkgInfo->first;
    auto const prefix = cmStrCat(pkgName, "::");
    std::string component = cmHasPrefix(linkedName, prefix)
      ? linkedName.substr(prefix.length())
      : linkedName;
    LinkInfo info{ pkgName, component };
    cmPackageInformation& req =
      this->Requirements.insert(std::move(*pkgInfo)).first->second;
    req.Components.emplace(std::move(component));
    return info;
  }

  // The leaf class selects build or install export provenance.
  auto const exportInfo = this->FindExportInfoFor(linkedTarget);
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
      return { "", linkedName };
    }

    std::string pkgName{ linkNamespace.data(), linkNamespace.size() - 2 };
    std::string component = linkedTarget->GetExportName();
    if (pkgName == this->PackageName) {
      return { "", std::move(component), true };
    }
    this->Requirements[pkgName].Components.emplace(component);
    return { std::move(pkgName), std::move(component) };
  }

  if (exportInfo.Sets.empty()) {
    // A peer SBOM supplies provenance when no export namespace is available.
    auto const sbomPackages = this->FindSbomPackagesFor(linkedTarget);
    if (sbomPackages.empty()) {
      return { "", linkedName, true };
    }
    auto const& pkgName = sbomPackages.front();
    if (sbomPackages.size() > 1) {
      target->Makefile->IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
        cmStrCat(
          "Target \"", target->GetName(), "\" references target \"",
          linkedName,
          "\" which has no install(EXPORT)/export(EXPORT) namespace and is "
          "covered by multiple SBOMs: ",
          cmJoin(sbomPackages, ", "), ".  Attributing to \"", pkgName,
          "\" (first alphabetically)."));
    }
    std::string component = linkedTarget->GetExportName();
    this->Requirements[pkgName].Components.emplace(component);
    return { pkgName, std::move(component) };
  }

  target->Makefile->IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat("Target \"", target->GetName(), "\" references target \"",
             linkedName, "\" ",
             exportInfo.Sets.size() == 1
               ? "that is in an export set which is exported multiple times "
                 "with different namespaces: "
               : "that is in multiple export sets: ",
             cmJoin(exportInfo.Files, ", "),
             ".\n"
             "An SBOM cannot attribute a dependency exported in more than one "
             "export set or with more than one namespace.  Consider "
             "consolidating the exports of the \"",
             linkedTarget->GetName(), "\" target to a single export."));
  return { "", linkedName };
}
