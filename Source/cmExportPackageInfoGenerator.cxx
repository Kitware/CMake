/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportPackageInfoGenerator.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmExportSet.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

static const std::string kCPS_VERSION_STR = "0.12.0";

cmExportPackageInfoGenerator::cmExportPackageInfoGenerator(
  std::string packageName, std::string version, std::string versionCompat,
  std::string versionSchema, std::vector<std::string> defaultTargets,
  std::vector<std::string> defaultConfigurations)
  : PackageName(std::move(packageName))
  , PackageVersion(std::move(version))
  , PackageVersionCompat(std::move(versionCompat))
  , PackageVersionSchema(std::move(versionSchema))
  , DefaultTargets(std::move(defaultTargets))
  , DefaultConfigurations(std::move(defaultConfigurations))
{
}

cm::string_view cmExportPackageInfoGenerator::GetImportPrefixWithSlash() const
{
  return "@prefix@/"_s;
}

bool cmExportPackageInfoGenerator::GenerateImportFile(std::ostream& os)
{
  return this->GenerateMainFile(os);
}

void cmExportPackageInfoGenerator::WritePackageInfo(
  Json::Value const& packageInfo, std::ostream& os) const
{
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";
  builder["commentStyle"] = "None";
  std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());
  writer->write(packageInfo, &os);
}

namespace {
template <typename T>
void buildArray(Json::Value& object, std::string const& property,
                T const& values)
{
  if (!values.empty()) {
    Json::Value& array = object[property];
    for (auto const& item : values) {
      array.append(item);
    }
  }
}
}

bool cmExportPackageInfoGenerator::CheckDefaultTargets() const
{
  bool result = true;
  std::set<std::string> exportedTargetNames;
  for (auto const* te : this->ExportedTargets) {
    exportedTargetNames.emplace(te->GetExportName());
  }

  for (auto const& name : this->DefaultTargets) {
    if (!cm::contains(exportedTargetNames, name)) {
      this->ReportError(
        cmStrCat("Package \"", this->GetPackageName(),
                 "\" specifies DEFAULT_TARGETS \"", name,
                 "\", which is not a target in the export set \"",
                 this->GetExportSet()->GetName(), "\"."));
      result = false;
    }
  }

  return result;
}

Json::Value cmExportPackageInfoGenerator::GeneratePackageInfo() const
{
  Json::Value package;

  package["name"] = this->GetPackageName();
  package["cps_version"] = std::string(kCPS_VERSION_STR);

  if (!this->PackageVersion.empty()) {
    package["version"] = this->PackageVersion;
    if (!this->PackageVersion.empty()) {
      package["compat_version"] = this->PackageVersionCompat;
    }
    if (!this->PackageVersion.empty()) {
      package["version_schema"] = this->PackageVersionSchema;
    }
  }

  buildArray(package, "default_components", this->DefaultTargets);
  buildArray(package, "configurations", this->DefaultConfigurations);

  // TODO: description, website, license

  return package;
}

void cmExportPackageInfoGenerator::GeneratePackageRequires(
  Json::Value& package) const
{
  if (!this->Requirements.empty()) {
    Json::Value& requirements = package["requires"];
    for (auto const& requirement : this->Requirements) {
      // TODO: version, hint
      requirements[requirement] = Json::Value{};
    }
  }
}

Json::Value* cmExportPackageInfoGenerator::GenerateImportTarget(
  Json::Value& components, cmGeneratorTarget const* target,
  cmStateEnums::TargetType targetType) const
{
  auto const& name = target->GetExportName();
  if (name.empty()) {
    return nullptr;
  }

  Json::Value& component = components[name];
  Json::Value& type = component["type"];
  switch (targetType) {
    case cmStateEnums::EXECUTABLE:
      type = "executable";
      break;
    case cmStateEnums::STATIC_LIBRARY:
      type = "archive";
      break;
    case cmStateEnums::SHARED_LIBRARY:
      type = "dylib";
      break;
    case cmStateEnums::MODULE_LIBRARY:
      type = "module";
      break;
    case cmStateEnums::INTERFACE_LIBRARY:
      type = "interface";
      break;
    default:
      type = "unknown";
      break;
  }
  return &component;
}

bool cmExportPackageInfoGenerator::GenerateInterfaceProperties(
  Json::Value& component, cmGeneratorTarget const* target,
  ImportPropertyMap const& properties) const
{
  bool result = true;

  this->GenerateInterfaceLinkProperties(result, component, target, properties);

  this->GenerateInterfaceCompileFeatures(result, component, target,
                                         properties);
  this->GenerateInterfaceCompileDefines(result, component, target, properties);

  this->GenerateInterfaceListProperty(result, component, target,
                                      "compile_flags", "COMPILE_OPTIONS"_s,
                                      properties);
  this->GenerateInterfaceListProperty(result, component, target, "link_flags",
                                      "LINK_OPTIONS"_s, properties);
  this->GenerateInterfaceListProperty(result, component, target,
                                      "link_directories", "LINK_DIRECTORIES"_s,
                                      properties);
  this->GenerateInterfaceListProperty(result, component, target, "includes",
                                      "INCLUDE_DIRECTORIES"_s, properties);

  // TODO: description, license

  return result;
}

namespace {
bool forbidGeneratorExpressions(std::string const& propertyName,
                                std::string const& propertyValue,
                                cmGeneratorTarget const* target)
{
  std::string const& evaluatedValue = cmGeneratorExpression::Preprocess(
    propertyValue, cmGeneratorExpression::StripAllGeneratorExpressions);
  if (evaluatedValue != propertyValue) {
    target->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Property \"", propertyName, "\" of target \"",
               target->GetName(),
               "\" contains a generator expression. This is not allowed."));
    return false;
  }
  return true;
}
}

bool cmExportPackageInfoGenerator::NoteLinkedTarget(
  cmGeneratorTarget const* target, std::string const& linkedName,
  cmGeneratorTarget const* linkedTarget)
{
  if (cm::contains(this->ExportedTargets, linkedTarget)) {
    // Target is internal to this package.
    this->LinkTargets.emplace(linkedName,
                              cmStrCat(':', linkedTarget->GetExportName()));
    return true;
  }

  if (linkedTarget->IsImported()) {
    // Target is imported from a found package.
    auto pkgName = [linkedTarget]() -> std::string {
      auto const& pkgStack = linkedTarget->Target->GetFindPackageStack();
      if (!pkgStack.Empty()) {
        return pkgStack.Top().Name;
      }

      return linkedTarget->Target->GetProperty("EXPORT_FIND_PACKAGE_NAME");
    }();

    if (pkgName.empty()) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(),
                 "\" references imported target \"", linkedName,
                 "\" which does not come from any known package."));
      return false;
    }

    auto const& prefix = cmStrCat(pkgName, "::");
    if (!cmHasPrefix(linkedName, prefix)) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName, "\", which comes from the \"", pkgName,
                 "\" package, but does not belong to the package's "
                 "canonical namespace. This is not allowed."));
      return false;
    }

    // TODO: Record package version, hint.
    this->Requirements.emplace(pkgName);
    this->LinkTargets.emplace(
      linkedName, cmStrCat(pkgName, ':', linkedName.substr(prefix.length())));
    return true;
  }

  // Target belongs to another export from this build.
  auto const& exportInfo = this->FindExportInfo(linkedTarget);
  if (exportInfo.first.size() == 1) {
    auto const& linkNamespace = exportInfo.second;
    if (!cmHasSuffix(linkNamespace, "::")) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName,
                 "\", which does not use the standard namespace separator. "
                 "This is not allowed."));
      return false;
    }

    auto pkgName =
      cm::string_view{ linkNamespace.data(), linkNamespace.size() - 2 };

    if (pkgName == this->GetPackageName()) {
      this->LinkTargets.emplace(linkedName,
                                cmStrCat(':', linkedTarget->GetExportName()));
    } else {
      this->Requirements.emplace(pkgName);
      this->LinkTargets.emplace(
        linkedName, cmStrCat(pkgName, ':', linkedTarget->GetExportName()));
    }
    return true;
  }

  // cmExportFileGenerator::HandleMissingTarget should have complained about
  // this already. (In fact, we probably shouldn't ever get here.)
  return false;
}

void cmExportPackageInfoGenerator::GenerateInterfaceLinkProperties(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  ImportPropertyMap const& properties) const
{
  auto const& iter = properties.find("INTERFACE_LINK_LIBRARIES");
  if (iter == properties.end()) {
    return;
  }

  // TODO: Support $<LINK_ONLY>.
  if (!forbidGeneratorExpressions(iter->first, iter->second, target)) {
    result = false;
    return;
  }

  std::vector<std::string> buildRequires;
  // std::vector<std::string> linkRequires; TODO
  std::vector<std::string> linkLibraries;

  for (auto const& name : cmList{ iter->second }) {
    auto const& ti = this->LinkTargets.find(name);
    if (ti != this->LinkTargets.end()) {
      if (ti->second.empty()) {
        result = false;
      } else {
        buildRequires.emplace_back(ti->second);
      }
    } else {
      linkLibraries.emplace_back(name);
    }
  }

  buildArray(component, "requires", buildRequires);
  // buildArray(component, "link_requires", linkRequires); TODO
  buildArray(component, "link_libraries", linkLibraries);
}

void cmExportPackageInfoGenerator::GenerateInterfaceCompileFeatures(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  ImportPropertyMap const& properties) const
{
  auto const& iter = properties.find("INTERFACE_COMPILE_FEATURES");
  if (iter == properties.end()) {
    return;
  }

  if (!forbidGeneratorExpressions(iter->first, iter->second, target)) {
    result = false;
    return;
  }

  std::set<std::string> features;
  for (auto const& value : cmList{ iter->second }) {
    if (cmHasLiteralPrefix(value, "c_std_")) {
      auto suffix = cm::string_view{ value }.substr(6, 2);
      features.emplace(cmStrCat("cxx", suffix));
    } else if (cmHasLiteralPrefix(value, "cxx_std_")) {
      auto suffix = cm::string_view{ value }.substr(8, 2);
      features.emplace(cmStrCat("c++", suffix));
    }
  }

  buildArray(component, "compile_features", features);
}

void cmExportPackageInfoGenerator::GenerateInterfaceCompileDefines(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  ImportPropertyMap const& properties) const
{
  auto const& iter = properties.find("INTERFACE_COMPILE_DEFINITIONS");
  if (iter == properties.end()) {
    return;
  }

  // TODO: Support language-specific defines.
  if (!forbidGeneratorExpressions(iter->first, iter->second, target)) {
    result = false;
    return;
  }

  Json::Value defines;
  for (auto const& def : cmList{ iter->second }) {
    auto const n = def.find('=');
    if (n == std::string::npos) {
      defines[def] = Json::Value{};
    } else {
      defines[def.substr(0, n)] = def.substr(n + 1);
    }
  }

  if (!defines.empty()) {
    component["compile_definitions"]["*"] = std::move(defines);
  }
}

void cmExportPackageInfoGenerator::GenerateInterfaceListProperty(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  std::string const& outName, cm::string_view inName,
  ImportPropertyMap const& properties) const
{
  auto const& prop = cmStrCat("INTERFACE_", inName);
  auto const& iter = properties.find(prop);
  if (iter == properties.end()) {
    return;
  }

  if (!forbidGeneratorExpressions(prop, iter->second, target)) {
    result = false;
    return;
  }

  Json::Value& array = component[outName];
  for (auto const& value : cmList{ iter->second }) {
    array.append(value);
  }
}

void cmExportPackageInfoGenerator::GenerateInterfaceConfigProperties(
  Json::Value& components, cmGeneratorTarget const* target,
  std::string const& suffix, ImportPropertyMap const& properties) const
{
  Json::Value component;
  auto const suffixLength = suffix.length();

  for (auto const& p : properties) {
    if (!cmHasSuffix(p.first, suffix)) {
      continue;
    }
    auto const n = p.first.length() - suffixLength - 9;
    auto const prop = cm::string_view{ p.first }.substr(9, n);

    if (prop == "LOCATION") {
      component["location"] = p.second;
    } else if (prop == "IMPLIB") {
      component["link_location"] = p.second;
    } else if (prop == "LINK_INTERFACE_LANGUAGES") {
      std::vector<std::string> languages;
      for (auto const& lang : cmList{ p.second }) {
        auto ll = cmSystemTools::LowerCase(lang);
        if (ll == "cxx") {
          languages.emplace_back("cpp");
        } else {
          languages.emplace_back(std::move(ll));
        }
      }
      buildArray(component, "link_languages", languages);
    }
  }

  if (!component.empty()) {
    components[target->GetExportName()] = component;
  }
}
