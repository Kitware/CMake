/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPackageInfoReader.h"

#include <initializer_list>
#include <limits>
#include <unordered_map>
#include <utility>

#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/version.h>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

namespace {

// Map of CPS language names to CMake language name.  Case insensitivity is
// achieved by converting the CPS value to lower case, so keys in this map must
// be lower case.
std::unordered_map<std::string, std::string> Languages = {
  // clang-format off
  { "c", "C" },
  { "c++", "CXX" },
  { "cpp", "CXX" },
  { "cxx", "CXX" },
  { "objc", "OBJC" },
  { "objc++", "OBJCXX" },
  { "objcpp", "OBJCXX" },
  { "objcxx", "OBJCXX" },
  { "swift", "swift" },
  { "hip", "HIP" },
  { "cuda", "CUDA" },
  { "ispc", "ISPC" },
  { "c#", "CSharp" },
  { "csharp", "CSharp" },
  { "fortran", "Fortran" },
  // clang-format on
};

enum LanguageGlobOption
{
  DisallowGlob,
  AllowGlob,
};

cm::string_view MapLanguage(cm::string_view lang,
                            LanguageGlobOption glob = AllowGlob)
{
  if (glob == AllowGlob && lang == "*"_s) {
    return "*"_s;
  }
  auto const li = Languages.find(cmSystemTools::LowerCase(lang));
  if (li != Languages.end()) {
    return li->second;
  }
  return {};
}

std::string GetRealPath(std::string const& path)
{
  return cmSystemTools::GetRealPath(path);
}

std::string GetRealDir(std::string const& path)
{
  return cmSystemTools::GetFilenamePath(cmSystemTools::GetRealPath(path));
}

Json::Value ReadJson(std::string const& fileName)
{
  // Open the specified file.
  cmsys::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
  if (!file) {
#if JSONCPP_VERSION_HEXA < 0x01070300
    return Json::Value::null;
#else
    return Json::Value::nullSingleton();
#endif
  }

  // Read file content and translate JSON.
  Json::Value data;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  if (!Json::parseFromStream(builder, file, &data, nullptr)) {
#if JSONCPP_VERSION_HEXA < 0x01070300
    return Json::Value::null;
#else
    return Json::Value::nullSingleton();
#endif
  }

  return data;
}

bool CheckSchemaVersion(Json::Value const& data)
{
  std::string const& version = data["cps_version"].asString();

  // Check that a valid version is specified.
  if (version.empty()) {
    return false;
  }

  // Check that we understand this version.
  return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                       version, "0.13") &&
    cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, version, "1");

  // TODO Eventually this probably needs to return the version tuple, and
  // should share code with cmPackageInfoReader::ParseVersion.
}

bool ComparePathSuffix(std::string const& path, std::string const& suffix)
{
  std::string const& tail = path.substr(path.size() - suffix.size());
  return cmSystemTools::ComparePath(tail, suffix);
}

std::string DeterminePrefix(std::string const& filepath,
                            Json::Value const& data)
{
  // First check if an absolute prefix was supplied.
  std::string prefix = data["prefix"].asString();
  if (!prefix.empty()) {
    // Ensure that the specified prefix is valid.
    if (cmsys::SystemTools::FileIsFullPath(prefix) &&
        cmsys::SystemTools::FileIsDirectory(prefix)) {
      cmSystemTools::ConvertToUnixSlashes(prefix);
      return prefix;
    }
    // The specified absolute prefix is not valid.
    return {};
  }

  // Get and validate prefix-relative path.
  std::string const& absPath = cmSystemTools::GetFilenamePath(filepath);
  std::string relPath = data["cps_path"].asString();
  cmSystemTools::ConvertToUnixSlashes(relPath);
  if (relPath.empty() || !cmHasLiteralPrefix(relPath, "@prefix@")) {
    // The relative prefix is not valid.
    return {};
  }
  if (relPath.size() == 8) {
    // The relative path is exactly "@prefix@".
    return absPath;
  }
  if (relPath[8] != '/') {
    // The relative prefix is not valid.
    return {};
  }
  relPath = relPath.substr(8);

  // Get directory portion of the absolute path.
  if (ComparePathSuffix(absPath, relPath)) {
    return absPath.substr(0, absPath.size() - relPath.size());
  }

  for (auto* const f : { GetRealPath, GetRealDir }) {
    std::string const& tmpPath = (*f)(absPath);
    if (!cmSystemTools::ComparePath(tmpPath, absPath) &&
        ComparePathSuffix(tmpPath, relPath)) {
      return tmpPath.substr(0, tmpPath.size() - relPath.size());
    }
  }

  return {};
}

// Extract key name from value iterator as string_view.
cm::string_view IterKey(Json::Value::const_iterator const& iter)
{
  char const* end;
  char const* const start = iter.memberName(&end);
  return { start, static_cast<std::string::size_type>(end - start) };
}

// Get list-of-strings value from object.
std::vector<std::string> ReadList(Json::Value const& arr)
{
  std::vector<std::string> result;

  if (arr.isArray()) {
    for (Json::Value const& val : arr) {
      if (val.isString()) {
        result.push_back(val.asString());
      }
    }
  }

  return result;
}

std::vector<std::string> ReadList(Json::Value const& data, char const* key)
{
  return ReadList(data[key]);
}

std::string NormalizeTargetName(std::string const& name,
                                std::string const& context)
{
  if (cmHasLiteralPrefix(name, ":")) {
    return cmStrCat(context, name);
  }

  std::string::size_type const n = name.find_first_of(':');
  if (n != std::string::npos) {
    cm::string_view v{ name };
    return cmStrCat(v.substr(0, n), ':', v.substr(n));
  }
  return name;
}

void AppendProperty(cmMakefile* makefile, cmTarget* target,
                    cm::string_view property, cm::string_view configuration,
                    std::string const& value)
{
  std::string fullprop;
  if (configuration.empty()) {
    fullprop = cmStrCat("INTERFACE_"_s, property);
  } else {
    fullprop = cmStrCat("INTERFACE_"_s, property, '_',
                        cmSystemTools::UpperCase(configuration));
  }

  target->AppendProperty(fullprop, value, makefile->GetBacktrace());
}

template <typename Transform>
void AppendLanguageProperties(cmMakefile* makefile, cmTarget* target,
                              cm::string_view property,
                              cm::string_view configuration,
                              Json::Value const& data, char const* key,
                              Transform transform)
{
  Json::Value const& value = data[key];
  if (value.isArray()) {
    for (std::string v : ReadList(value)) {
      AppendProperty(makefile, target, property, configuration,
                     transform(std::move(v)));
    }
  } else if (value.isObject()) {
    for (auto vi = value.begin(), ve = value.end(); vi != ve; ++vi) {
      cm::string_view const originalLang = IterKey(vi);
      cm::string_view const lang = MapLanguage(originalLang);
      if (lang.empty()) {
        makefile->IssueMessage(MessageType::WARNING,
                               cmStrCat(R"(ignoring unknown language ")"_s,
                                        originalLang, R"(" in )"_s, key,
                                        " for "_s, target->GetName()));
        continue;
      }

      if (lang == "*"_s) {
        for (std::string v : ReadList(*vi)) {
          AppendProperty(makefile, target, property, configuration,
                         transform(std::move(v)));
        }
      } else {
        for (std::string v : ReadList(*vi)) {
          v = cmStrCat("$<$<COMPILE_LANGUAGE:"_s, lang, ">:"_s,
                       transform(std::move(v)), '>');
          AppendProperty(makefile, target, property, configuration, v);
        }
      }
    }
  }
}

void AddCompileFeature(cmMakefile* makefile, cmTarget* target,
                       cm::string_view configuration, std::string const& value)
{
  auto reLanguageLevel = []() -> cmsys::RegularExpression {
    static cmsys::RegularExpression re{ "^[Cc]([+][+])?([0-9][0-9])$" };
    return re;
  }();

  if (reLanguageLevel.find(value)) {
    std::string::size_type const n = reLanguageLevel.end() - 2;
    cm::string_view const featurePrefix = (n == 3 ? "cxx_std_"_s : "c_std_"_s);
    if (configuration.empty()) {
      AppendProperty(makefile, target, "COMPILE_FEATURES"_s, {},
                     cmStrCat(featurePrefix, value.substr(n)));
    } else {
      std::string const& feature =
        cmStrCat("$<$<CONFIG:"_s, configuration, ">:"_s, featurePrefix,
                 value.substr(n), '>');
      AppendProperty(makefile, target, "COMPILE_FEATURES"_s, {}, feature);
    }
  } else if (cmStrCaseEq(value, "gnu"_s)) {
    // Not implemented in CMake at this time
  } else if (cmStrCaseEq(value, "threads"_s)) {
    AppendProperty(makefile, target, "LINK_LIBRARIES"_s, configuration,
                   "Threads::Threads");
  }
}

void AddLinkFeature(cmMakefile* makefile, cmTarget* target,
                    cm::string_view configuration, std::string const& value)
{
  if (cmStrCaseEq(value, "thread"_s)) {
    AppendProperty(makefile, target, "LINK_LIBRARIES"_s, configuration,
                   "Threads::Threads");
  }
}

std::string BuildDefinition(std::string const& name, Json::Value const& value)
{
  if (!value.isNull() && value.isConvertibleTo(Json::stringValue)) {
    return cmStrCat(name, '=', value.asString());
  }
  return name;
}

void AddDefinition(cmMakefile* makefile, cmTarget* target,
                   cm::string_view configuration,
                   std::string const& definition)
{
  AppendProperty(makefile, target, "COMPILE_DEFINITIONS"_s, configuration,
                 definition);
}

using DefinitionLanguageMap = std::map<cm::string_view, Json::Value>;
using DefinitionsMap = std::map<std::string, DefinitionLanguageMap>;

void AddDefinitions(cmMakefile* makefile, cmTarget* target,
                    cm::string_view configuration,
                    DefinitionsMap const& definitions)
{
  for (auto const& di : definitions) {
    auto const& g = di.second.find("*"_s);
    if (g != di.second.end()) {
      std::string const& def = BuildDefinition(di.first, g->second);
      if (di.second.size() == 1) {
        // Only the non-language-specific definition exists.
        AddDefinition(makefile, target, configuration, def);
        continue;
      }

      // Create a genex to apply this definition to all languages except
      // those that override it.
      std::vector<cm::string_view> excludedLanguages;
      for (auto const& li : di.second) {
        if (li.first != "*"_s) {
          excludedLanguages.emplace_back(li.first);
        }
      }
      AddDefinition(makefile, target, configuration,
                    cmStrCat("$<$<NOT:$<COMPILE_LANGUAGE:"_s,
                             cmJoin(excludedLanguages, ","_s), ">>:"_s, def,
                             '>'));
    }

    // Add language-specific definitions.
    for (auto const& li : di.second) {
      if (li.first != "*"_s) {
        AddDefinition(makefile, target, configuration,
                      cmStrCat("$<$<COMPILE_LANGUAGE:"_s, li.first, ">:"_s,
                               BuildDefinition(di.first, li.second), '>'));
      }
    }
  }
}

cm::optional<cmPackageInfoReader::Pep440Version> ParseSimpleVersion(
  std::string const& version)
{
  if (version.empty()) {
    return cm::nullopt;
  }

  cmPackageInfoReader::Pep440Version result;
  result.Simple = true;

  cm::string_view remnant{ version };
  for (;;) {
    // Find the next part separator.
    std::string::size_type const n = remnant.find_first_of(".+-"_s);
    if (n == 0) {
      // The part is an empty string.
      return cm::nullopt;
    }

    // Extract the part as a number.
    cm::string_view const part = remnant.substr(0, n);
    std::string::size_type const l = part.size();
    std::string::size_type p;
    unsigned long const value = std::stoul(std::string{ part }, &p);
    if (p != l || value > std::numeric_limits<unsigned>::max()) {
      // The part was not a valid number or is too big.
      return cm::nullopt;
    }
    result.ReleaseComponents.push_back(static_cast<unsigned>(value));

    // Have we consumed the entire input?
    if (n == std::string::npos) {
      return { std::move(result) };
    }

    // Lop off the current part.
    char const sep = remnant[n];
    remnant = remnant.substr(n + 1);
    if (sep == '+' || sep == '-') {
      // If we hit the local label, we're done.
      result.LocalLabel = remnant;
      return { std::move(result) };
    }

    // We just consumed a '.'; check that there's more.
    if (remnant.empty()) {
      // A trailing part separator is not allowed.
      return cm::nullopt;
    }

    // Continue with the remaining input.
  }

  // Unreachable.
}

} // namespace

std::unique_ptr<cmPackageInfoReader> cmPackageInfoReader::Read(
  std::string const& path, cmPackageInfoReader const* parent)
{
  // Read file and perform some basic validation:
  //   - the input is valid JSON
  //   - the input is a JSON object
  //   - the input has a "cps_version" that we (in theory) know how to parse
  Json::Value data = ReadJson(path);
  if (!data.isObject() || (!parent && !CheckSchemaVersion(data))) {
    return nullptr;
  }

  //   - the input has a "name" attribute that is a non-empty string
  Json::Value const& name = data["name"];
  if (!name.isString() || name.empty()) {
    return nullptr;
  }

  //   - the input has a "components" attribute that is a JSON object
  if (!data["components"].isObject()) {
    return nullptr;
  }

  std::string prefix = (parent ? parent->Prefix : DeterminePrefix(path, data));
  if (prefix.empty()) {
    return nullptr;
  }

  // Seems sane enough to hand back to the caller.
  std::unique_ptr<cmPackageInfoReader> reader{ new cmPackageInfoReader };
  reader->Data = std::move(data);
  reader->Prefix = std::move(prefix);
  reader->Path = path;

  // Determine other information we need to know immediately, or (if this is
  // a supplemental reader) copy from the parent.
  if (parent) {
    reader->ComponentTargets = parent->ComponentTargets;
    reader->DefaultConfigurations = parent->DefaultConfigurations;
  } else {
    for (std::string const& config :
         ReadList(reader->Data, "configurations")) {
      reader->DefaultConfigurations.emplace_back(
        cmSystemTools::UpperCase(config));
    }
  }

  return reader;
}

std::string cmPackageInfoReader::GetName() const
{
  return this->Data["name"].asString();
}

cm::optional<std::string> cmPackageInfoReader::GetVersion() const
{
  Json::Value const& version = this->Data["version"];
  if (version.isString()) {
    return version.asString();
  }
  return cm::nullopt;
}

cm::optional<std::string> cmPackageInfoReader::GetCompatVersion() const
{
  Json::Value const& version = this->Data["compat_version"];
  if (version.isString()) {
    return version.asString();
  }
  return cm::nullopt;
}

cm::optional<cmPackageInfoReader::Pep440Version>
cmPackageInfoReader::ParseVersion(
  cm::optional<std::string> const& version) const
{
  // Check that we have a version.
  if (!version) {
    return cm::nullopt;
  }

  // Check if we know how to parse the version.
  Json::Value const& schema = this->Data["version_schema"];
  if (schema.isNull() || cmStrCaseEq(schema.asString(), "simple"_s)) {
    return ParseSimpleVersion(*version);
  }

  return cm::nullopt;
}

std::vector<cmPackageRequirement> cmPackageInfoReader::GetRequirements() const
{
  std::vector<cmPackageRequirement> requirements;

  auto const& requirementObjects = this->Data["requires"];

  for (auto ri = requirementObjects.begin(), re = requirementObjects.end();
       ri != re; ++ri) {
    cmPackageRequirement r{ ri.name(), (*ri)["version"].asString(),
                            ReadList(*ri, "components"),
                            ReadList(*ri, "hints") };
    requirements.emplace_back(std::move(r));
  }

  return requirements;
}

std::vector<std::string> cmPackageInfoReader::GetComponentNames() const
{
  std::vector<std::string> componentNames;

  Json::Value const& components = this->Data["components"];
  for (auto ci = components.begin(), ce = components.end(); ci != ce; ++ci) {
    componentNames.emplace_back(ci.name());
  }

  return componentNames;
}

std::string cmPackageInfoReader::ResolvePath(std::string path) const
{
  cmSystemTools::ConvertToUnixSlashes(path);
  if (cmHasPrefix(path, "@prefix@"_s)) {
    return cmStrCat(this->Prefix, path.substr(8));
  }
  if (!cmSystemTools::FileIsFullPath(path)) {
    return cmStrCat(cmSystemTools::GetFilenamePath(this->Path), '/', path);
  }
  return path;
}

void cmPackageInfoReader::SetOptionalProperty(cmTarget* target,
                                              cm::string_view property,
                                              cm::string_view configuration,
                                              Json::Value const& value) const
{
  if (!value.isNull()) {
    std::string fullprop;
    if (configuration.empty()) {
      fullprop = cmStrCat("IMPORTED_"_s, property);
    } else {
      fullprop = cmStrCat("IMPORTED_"_s, property, '_',
                          cmSystemTools::UpperCase(configuration));
    }

    target->SetProperty(fullprop, this->ResolvePath(value.asString()));
  }
}

void cmPackageInfoReader::SetTargetProperties(
  cmMakefile* makefile, cmTarget* target, Json::Value const& data,
  std::string const& package, cm::string_view configuration) const
{
  // Add configuration (if applicable).
  if (!configuration.empty()) {
    target->AppendProperty("IMPORTED_CONFIGURATIONS",
                           cmSystemTools::UpperCase(configuration),
                           makefile->GetBacktrace());
  }

  // Add compile and link features.
  for (std::string const& def : ReadList(data, "compile_features")) {
    AddCompileFeature(makefile, target, configuration, def);
  }

  for (std::string const& def : ReadList(data, "link_features")) {
    AddLinkFeature(makefile, target, configuration, def);
  }

  // Add compile definitions.
  Json::Value const& defs = data["definitions"];
  DefinitionsMap definitionsMap;
  for (auto ldi = defs.begin(), lde = defs.end(); ldi != lde; ++ldi) {
    cm::string_view const originalLang = IterKey(ldi);
    cm::string_view const lang = MapLanguage(originalLang);
    if (lang.empty()) {
      makefile->IssueMessage(
        MessageType::WARNING,
        cmStrCat(R"(ignoring unknown language ")"_s, originalLang,
                 R"(" in definitions for )"_s, target->GetName()));
      continue;
    }

    for (auto di = ldi->begin(), de = ldi->end(); di != de; ++di) {
      definitionsMap[di.name()].emplace(lang, *di);
    }
  }
  AddDefinitions(makefile, target, configuration, definitionsMap);

  // Add include directories.
  AppendLanguageProperties(makefile, target, "INCLUDE_DIRECTORIES"_s,
                           configuration, data, "includes",
                           [this](std::string p) -> std::string {
                             return this->ResolvePath(std::move(p));
                           });

  // Add link name/location(s).
  this->SetOptionalProperty(target, "LOCATION"_s, configuration,
                            data["location"]);

  this->SetOptionalProperty(target, "IMPLIB"_s, configuration,
                            data["link_location"]);

  this->SetOptionalProperty(target, "SONAME"_s, configuration,
                            data["link_name"]);

  // Add link languages.
  for (std::string const& originalLang : ReadList(data, "link_languages")) {
    cm::string_view const lang = MapLanguage(originalLang, DisallowGlob);
    if (!lang.empty()) {
      AppendProperty(makefile, target, "LINK_LANGUAGES"_s, configuration,
                     std::string{ lang });
    }
  }

  // Add transitive dependencies.
  for (std::string const& dep : ReadList(data, "requires")) {
    AppendProperty(makefile, target, "LINK_LIBRARIES"_s, configuration,
                   NormalizeTargetName(dep, package));
  }

  for (std::string const& dep : ReadList(data, "link_requires")) {
    std::string const& lib =
      cmStrCat("$<LINK_ONLY:"_s, NormalizeTargetName(dep, package), '>');
    AppendProperty(makefile, target, "LINK_LIBRARIES"_s, configuration, lib);
  }
}

cmTarget* cmPackageInfoReader::AddLibraryComponent(
  cmMakefile* makefile, cmStateEnums::TargetType type, std::string const& name,
  Json::Value const& data, std::string const& package) const
{
  // Create the imported target.
  cmTarget* const target = makefile->AddImportedTarget(name, type, false);

  // Set default configurations.
  if (!this->DefaultConfigurations.empty()) {
    target->SetProperty("IMPORTED_CONFIGURATIONS",
                        cmJoin(this->DefaultConfigurations, ";"_s));
  }

  // Set target properties.
  this->SetTargetProperties(makefile, target, data, package, {});
  auto const& cfgData = data["configurations"];
  for (auto ci = cfgData.begin(), ce = cfgData.end(); ci != ce; ++ci) {
    this->SetTargetProperties(makefile, target, *ci, package, IterKey(ci));
  }

  return target;
}

bool cmPackageInfoReader::ImportTargets(cmMakefile* makefile,
                                        cmExecutionStatus& status)
{
  std::string const& package = this->GetName();

  // Read components.
  Json::Value const& components = this->Data["components"];

  for (auto ci = components.begin(), ce = components.end(); ci != ce; ++ci) {
    cm::string_view const& name = IterKey(ci);
    std::string const& type =
      cmSystemTools::LowerCase((*ci)["type"].asString());

    // Get and validate full target name.
    std::string const& fullName = cmStrCat(package, "::"_s, name);
    {
      std::string msg;
      if (!makefile->EnforceUniqueName(fullName, msg)) {
        status.SetError(msg);
        return false;
      }
    }

    cmTarget* target = nullptr;
    if (type == "symbolic"_s) {
      // TODO
    } else if (type == "dylib"_s) {
      target = this->AddLibraryComponent(
        makefile, cmStateEnums::SHARED_LIBRARY, fullName, *ci, package);
    } else if (type == "module"_s) {
      target = this->AddLibraryComponent(
        makefile, cmStateEnums::MODULE_LIBRARY, fullName, *ci, package);
    } else if (type == "archive"_s) {
      target = this->AddLibraryComponent(
        makefile, cmStateEnums::STATIC_LIBRARY, fullName, *ci, package);
    } else if (type == "interface"_s) {
      target = this->AddLibraryComponent(
        makefile, cmStateEnums::INTERFACE_LIBRARY, fullName, *ci, package);
    } else {
      makefile->IssueMessage(MessageType::WARNING,
                             cmStrCat(R"(component ")"_s, fullName,
                                      R"(" has unknown type ")"_s, type,
                                      R"(" and was not imported)"_s));
    }

    if (target) {
      this->ComponentTargets.emplace(std::string{ name }, target);
    }
  }

  // Read default components.
  std::vector<std::string> const& defaultComponents =
    ReadList(this->Data, "default_components");
  if (!defaultComponents.empty()) {
    std::string msg;
    if (!makefile->EnforceUniqueName(package, msg)) {
      status.SetError(msg);
      return false;
    }

    cmTarget* const target = makefile->AddImportedTarget(
      package, cmStateEnums::INTERFACE_LIBRARY, false);
    for (std::string const& name : defaultComponents) {
      std::string const& fullName = cmStrCat(package, "::"_s, name);
      AppendProperty(makefile, target, "LINK_LIBRARIES"_s, {}, fullName);
    }
  }

  return true;
}

bool cmPackageInfoReader::ImportTargetConfigurations(
  cmMakefile* makefile, cmExecutionStatus& status) const
{
  std::string const& configuration = this->Data["configuration"].asString();

  if (configuration.empty()) {
    makefile->IssueMessage(MessageType::WARNING,
                           cmStrCat("supplemental file "_s, this->Path,
                                    " does not specify a configuration"_s));
    return true;
  }

  std::string const& package = this->GetName();
  Json::Value const& components = this->Data["components"];

  for (auto ci = components.begin(), ce = components.end(); ci != ce; ++ci) {
    // Get component name and look up target.
    cm::string_view const& name = IterKey(ci);
    auto const& ti = this->ComponentTargets.find(std::string{ name });
    if (ti == this->ComponentTargets.end()) {
      status.SetError(cmStrCat("component "_s, name, " was not found"_s));
      return false;
    }

    // Read supplemental data for component.
    this->SetTargetProperties(makefile, ti->second, *ci, package,
                              configuration);
  }

  return true;
}
