/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportPackageInfoGenerator.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParserTypes.h"
#include "cmExportSet.h"
#include "cmFindPackageStack.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPackageInfoArguments.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

static std::string const kCPS_VERSION_STR = "0.14.0";

cmExportPackageInfoGenerator::cmExportPackageInfoGenerator(
  cmPackageInfoArguments arguments)
  : PackageName(std::move(arguments.PackageName))
  , PackageVersion(std::move(arguments.Version))
  , PackageVersionCompat(std::move(arguments.VersionCompat))
  , PackageVersionSchema(std::move(arguments.VersionSchema))
  , PackageDescription(std::move(arguments.Description))
  , PackageWebsite(std::move(arguments.Website))
  , PackageLicense(std::move(arguments.License))
  , DefaultLicense(std::move(arguments.DefaultLicense))
  , DefaultTargets(std::move(arguments.DefaultTargets))
  , DefaultConfigurations(std::move(arguments.DefaultConfigs))
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
bool SetProperty(Json::Value& object, std::string const& property,
                 std::string const& value)
{
  if (!value.empty()) {
    object[property] = value;
    return true;
  }
  return false;
}

template <typename T>
void BuildArray(Json::Value& object, std::string const& property,
                T const& values)
{
  if (!values.empty()) {
    Json::Value& array = object[property];
    for (auto const& item : values) {
      array.append(item);
    }
  }
}

bool CheckSimpleVersion(std::string const& version)
{
  cmsys::RegularExpression regex("^[0-9]+([.][0-9]+)*([-+].*)?$");
  return regex.find(version);
}
}

bool cmExportPackageInfoGenerator::CheckVersion() const
{
  if (!this->PackageVersion.empty()) {
    std::string const& schema = [&] {
      if (this->PackageVersionSchema.empty()) {
        return std::string{ "simple" };
      }
      return cmSystemTools::LowerCase(this->PackageVersionSchema);
    }();
    bool (*validator)(std::string const&) = nullptr;
    bool result = true;

    if (schema == "simple"_s) {
      validator = &CheckSimpleVersion;
    } else if (schema == "dpkg"_s || schema == "rpm"_s ||
               schema == "pep440"_s) {
      // TODO
      // We don't validate these at this time. Eventually, we would like to do
      // so, but will probably need to introduce a policy whether to treat
      // invalid versions as an error.
    } else if (schema != "custom"_s) {
      this->IssueMessage(MessageType::AUTHOR_WARNING,
                         cmStrCat("Package \""_s, this->GetPackageName(),
                                  "\" uses unrecognized version schema \""_s,
                                  this->PackageVersionSchema, "\"."_s));
    }

    if (validator) {
      if (!(*validator)(this->PackageVersion)) {
        this->ReportError(cmStrCat("Package \""_s, this->GetPackageName(),
                                   "\" version \""_s, this->PackageVersion,
                                   "\" does not conform to the \""_s, schema,
                                   "\" schema."_s));
        result = false;
      }
      if (!this->PackageVersionCompat.empty() &&
          !(*validator)(this->PackageVersionCompat)) {
        this->ReportError(
          cmStrCat("Package \""_s, this->GetPackageName(),
                   "\" compatibility version \""_s, this->PackageVersionCompat,
                   "\" does not conform to the \""_s, schema, "\" schema."_s));
        result = false;
      }
    }

    return result;
  }

  return true;
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

  if (SetProperty(package, "version", this->PackageVersion)) {
    SetProperty(package, "compat_version", this->PackageVersionCompat);
    SetProperty(package, "version_schema", this->PackageVersionSchema);
  }

  BuildArray(package, "default_components", this->DefaultTargets);
  BuildArray(package, "configurations", this->DefaultConfigurations);

  SetProperty(package, "description", this->PackageDescription);
  SetProperty(package, "website", this->PackageWebsite);
  SetProperty(package, "license", this->PackageLicense);
  SetProperty(package, "default_license", this->DefaultLicense);

  return package;
}

void cmExportPackageInfoGenerator::GeneratePackageRequires(
  Json::Value& package) const
{
  if (!this->Requirements.empty()) {
    Json::Value& requirements = package["requires"];

    // Build description for each requirement.
    for (auto const& requirement : this->Requirements) {
      auto data = Json::Value{ Json::objectValue };

      // Add required components.
      if (!requirement.second.Components.empty()) {
        auto components = Json::Value{ Json::arrayValue };
        for (std::string const& component : requirement.second.Components) {
          components.append(component);
        }
        data["components"] = components;
      }

      // Add additional dependency information.
      if (requirement.second.Directory) {
        auto hints = Json::Value{ Json::arrayValue };
        hints.append(*requirement.second.Directory);
        data["hints"] = hints;
      }

      if (requirement.second.Version) {
        data["version"] = *requirement.second.Version;
      }

      requirements[requirement.first] = data;
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
      type = target->IsSymbolic() ? "symbolic" : "interface";
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

  this->GenerateProperty(result, component, target, "license", "SPDX_LICENSE",
                         properties);

  // TODO: description

  return result;
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
    using Package = cm::optional<std::pair<std::string, cmPackageInformation>>;
    auto pkgInfo = [](cmTarget* t) -> Package {
      cmFindPackageStack pkgStack = t->GetFindPackageStack();
      if (!pkgStack.Empty()) {
        return std::make_pair(pkgStack.Top().Name, pkgStack.Top().PackageInfo);
      }

      cmPackageInformation package;
      std::string const pkgName =
        t->GetSafeProperty("EXPORT_FIND_PACKAGE_NAME");
      if (pkgName.empty()) {
        return cm::nullopt;
      }

      return std::make_pair(pkgName, package);
    }(linkedTarget->Target);

    if (!pkgInfo) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(),
                 "\" references imported target \"", linkedName,
                 "\" which does not come from any known package."));
      return false;
    }

    std::string const& pkgName = pkgInfo->first;

    auto const& prefix = cmStrCat(pkgName, "::");
    if (!cmHasPrefix(linkedName, prefix)) {
      target->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName, "\", which comes from the \"", pkgName,
                 "\" package, but does not belong to the package's "
                 "canonical namespace (\"",
                 prefix, "\"). This is not allowed."));
      return false;
    }

    std::string component = linkedName.substr(prefix.length());
    this->LinkTargets.emplace(linkedName, cmStrCat(pkgName, ':', component));
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
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", target->GetName(), "\" references target \"",
                 linkedName,
                 "\", which does not use the standard namespace separator. "
                 "This is not allowed."));
      return false;
    }

    std::string pkgName{ linkNamespace.data(), linkNamespace.size() - 2 };
    std::string component = linkedTarget->GetExportName();
    if (pkgName == this->GetPackageName()) {
      this->LinkTargets.emplace(linkedName, cmStrCat(':', component));
    } else {
      this->LinkTargets.emplace(linkedName, cmStrCat(pkgName, ':', component));
      this->Requirements[pkgName].Components.emplace(std::move(component));
    }
    return true;
  }

  // Target belongs to multiple namespaces or multiple export sets.
  // cmExportFileGenerator::HandleMissingTarget should have complained about
  // this already.
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

  // Extract any $<LINK_ONLY:...> from the link libraries, and assert that no
  // other generator expressions are present.
  std::map<std::string, std::vector<std::string>> allowList = {
    { "COMPILE_ONLY", {} },
    { "LINK_ONLY", {} },
  };
  std::string interfaceLinkLibraries;
  if (!cmGeneratorExpression::ForbidGeneratorExpressions(
        target, iter->first, iter->second, interfaceLinkLibraries,
        allowList)) {
    result = false;
    return;
  }

  std::vector<std::string> buildRequires;
  std::vector<std::string> compileRequires;
  std::vector<std::string> linkRequires;
  std::vector<std::string> linkLibraries;

  auto addLibraries = [this, &linkLibraries,
                       &result](std::vector<std::string> const& names,
                                std::vector<std::string>& output) -> void {
    for (auto const& name : names) {
      auto const& ti = this->LinkTargets.find(name);
      if (ti != this->LinkTargets.end()) {
        if (ti->second.empty()) {
          result = false;
        } else {
          output.emplace_back(ti->second);
        }
      } else {
        linkLibraries.emplace_back(name);
      }
    }
  };

  addLibraries(allowList["COMPILE_ONLY"], compileRequires);
  addLibraries(allowList["LINK_ONLY"], linkRequires);
  addLibraries(cmList{ interfaceLinkLibraries }, buildRequires);

  BuildArray(component, "requires", buildRequires);
  BuildArray(component, "link_requires", linkRequires);
  BuildArray(component, "link_libraries", linkLibraries);
  BuildArray(component, "compile_requires", compileRequires);
}

void cmExportPackageInfoGenerator::GenerateInterfaceCompileFeatures(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  ImportPropertyMap const& properties) const
{
  auto const& iter = properties.find("INTERFACE_COMPILE_FEATURES");
  if (iter == properties.end()) {
    return;
  }

  if (!cmGeneratorExpression::ForbidGeneratorExpressions(target, iter->first,
                                                         iter->second)) {
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

  BuildArray(component, "compile_features", features);
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
  if (!cmGeneratorExpression::ForbidGeneratorExpressions(target, iter->first,
                                                         iter->second)) {
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
    component["definitions"]["*"] = std::move(defines);
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

  if (!cmGeneratorExpression::ForbidGeneratorExpressions(target, prop,
                                                         iter->second)) {
    result = false;
    return;
  }

  Json::Value& array = component[outName];
  for (auto const& value : cmList{ iter->second }) {
    array.append(value);
  }
}

void cmExportPackageInfoGenerator::GenerateProperty(
  bool& result, Json::Value& component, cmGeneratorTarget const* target,
  std::string const& outName, std::string const& inName,
  ImportPropertyMap const& properties) const
{
  auto const& iter = properties.find(inName);
  if (iter == properties.end()) {
    return;
  }

  if (!cmGeneratorExpression::ForbidGeneratorExpressions(target, inName,
                                                         iter->second)) {
    result = false;
    return;
  }

  component[outName] = iter->second;
}

Json::Value cmExportPackageInfoGenerator::GenerateInterfaceConfigProperties(
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
      BuildArray(component, "link_languages", languages);
    }
  }

  return component;
}

std::string cmExportPackageInfoGenerator::GenerateCxxModules(
  Json::Value& component, cmGeneratorTarget* target,
  std::string const& packagePath, std::string const& config)
{
  std::string manifestPath;

  std::string const cxxModulesDirName = this->GetCxxModulesDirectory();
  if (cxxModulesDirName.empty() || !target->HaveCxx20ModuleSources()) {
    return manifestPath;
  }

  manifestPath =
    cmStrCat(cxxModulesDirName, "/target-", target->GetFilesystemExportName(),
             '-', config.empty() ? "noconfig" : config, ".modules.json");

  component["cpp_module_metadata"] = cmStrCat(packagePath, '/', manifestPath);
  return manifestPath;
}
