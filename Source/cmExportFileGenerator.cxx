/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportFileGenerator.h"

#include <array>
#include <cstddef>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"

#include "cmComputeLinkInformation.h"
#include "cmFindPackageStack.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPropertyMap.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmExportFileGenerator::cmExportFileGenerator() = default;

void cmExportFileGenerator::AddConfiguration(std::string const& config)
{
  this->Configurations.push_back(config);
}

void cmExportFileGenerator::SetExportFile(char const* mainFile)
{
  this->MainImportFile = mainFile;
  this->FileDir = cmSystemTools::GetFilenamePath(this->MainImportFile);
  this->FileBase =
    cmSystemTools::GetFilenameWithoutLastExtension(this->MainImportFile);
  this->FileExt =
    cmSystemTools::GetFilenameLastExtension(this->MainImportFile);
}

std::string const& cmExportFileGenerator::GetMainExportFileName() const
{
  return this->MainImportFile;
}

bool cmExportFileGenerator::GenerateImportFile()
{
  // Open the output file to generate it.
  std::unique_ptr<cmsys::ofstream> foutPtr;
  if (this->AppendMode) {
    // Open for append.
    auto openmodeApp = std::ios::app;
    foutPtr = cm::make_unique<cmsys::ofstream>(this->MainImportFile.c_str(),
                                               openmodeApp);
  } else {
    // Generate atomically and with copy-if-different.
    std::unique_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(this->MainImportFile, true));
    ap->SetCopyIfDifferent(true);
    foutPtr = std::move(ap);
  }
  if (!foutPtr || !*foutPtr) {
    std::string se = cmSystemTools::GetLastSystemError();
    std::ostringstream e;
    e << "cannot write to file \"" << this->MainImportFile << "\": " << se;
    cmSystemTools::Error(e.str());
    return false;
  }

  return this->GenerateImportFile(*foutPtr);
}

void cmExportFileGenerator::GenerateImportConfig(std::ostream& os,
                                                 std::string const& config)
{
  // Construct the property configuration suffix.
  std::string suffix = "_";
  if (!config.empty()) {
    suffix += cmSystemTools::UpperCase(config);
  } else {
    suffix += "NOCONFIG";
  }

  // Generate the per-config target information.
  this->GenerateImportTargetsConfig(os, config, suffix);
}

bool cmExportFileGenerator::PopulateInterfaceProperties(
  cmGeneratorTarget const* target, std::string const& includesDestinationDirs,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_COMPILE_OPTIONS", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_PRECOMPILE_HEADERS", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_AUTOUIC_OPTIONS", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_AUTOMOC_MACRO_NAMES", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_COMPILE_FEATURES", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_LINK_OPTIONS", target,
                                  preprocessRule, properties);
  this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE",
                                  target, properties);

  std::string errorMessage;
  if (!this->PopulateCxxModuleExportProperties(
        target, properties, preprocessRule, includesDestinationDirs,
        errorMessage)) {
    this->ReportError(errorMessage);
    return false;
  }

  if (!this->PopulateExportProperties(target, properties, errorMessage)) {
    this->ReportError(errorMessage);
    return false;
  }
  this->PopulateCompatibleInterfaceProperties(target, properties);
  this->PopulateCustomTransitiveInterfaceProperties(target, preprocessRule,
                                                    properties);

  return true;
}

void cmExportFileGenerator::PopulateInterfaceProperty(
  std::string const& propName, cmGeneratorTarget const* target,
  ImportPropertyMap& properties) const
{
  cmValue input = target->GetProperty(propName);
  if (input) {
    properties[propName] = *input;
  }
}

void cmExportFileGenerator::PopulateInterfaceProperty(
  std::string const& propName, std::string const& outputName,
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  cmValue input = target->GetProperty(propName);
  if (input) {
    if (input->empty()) {
      // Set to empty
      properties[outputName].clear();
      return;
    }

    std::string prepro =
      cmGeneratorExpression::Preprocess(*input, preprocessRule);
    if (!prepro.empty()) {
      this->ResolveTargetsInGeneratorExpressions(prepro, target);
      properties[outputName] = prepro;
    }
  }
}

void cmExportFileGenerator::PopulateInterfaceProperty(
  std::string const& propName, cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  this->PopulateInterfaceProperty(propName, propName, target, preprocessRule,
                                  properties);
}

bool cmExportFileGenerator::PopulateInterfaceLinkLibrariesProperty(
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
        this->ResolveTargetsInGeneratorExpressions(prepro, target,
                                                   ReplaceFreeTargets);
        properties[linkIfaceProp] = prepro;
        hadINTERFACE_LINK_LIBRARIES = true;
      }
    }
  }
  return hadINTERFACE_LINK_LIBRARIES;
}

void cmExportFileGenerator::AddImportPrefix(std::string& exportDirs) const
{
  std::vector<std::string> entries;
  cmGeneratorExpression::Split(exportDirs, entries);
  exportDirs.clear();
  char const* sep = "";
  cm::string_view const& prefixWithSlash = this->GetImportPrefixWithSlash();
  for (std::string const& e : entries) {
    exportDirs += sep;
    sep = ";";
    if (!cmSystemTools::FileIsFullPath(e) &&
        !cmHasPrefix(e, prefixWithSlash)) {
      exportDirs += prefixWithSlash;
    }
    exportDirs += e;
  }
}

namespace {
void getPropertyContents(cmGeneratorTarget const* tgt, std::string const& prop,
                         std::set<std::string>& ifaceProperties)
{
  cmValue p = tgt->GetProperty(prop);
  if (!p) {
    return;
  }
  cmList content{ *p };
  ifaceProperties.insert(content.begin(), content.end());
}

void getCompatibleInterfaceProperties(cmGeneratorTarget const* target,
                                      std::set<std::string>& ifaceProperties,
                                      std::string const& config)
{
  if (target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    // object libraries have no link information, so nothing to compute
    return;
  }

  cmComputeLinkInformation* info = target->GetLinkInformation(config);

  if (!info) {
    cmLocalGenerator* lg = target->GetLocalGenerator();
    std::ostringstream e;
    e << "Exporting the target \"" << target->GetName()
      << "\" is not "
         "allowed since its linker language cannot be determined";
    lg->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  cmComputeLinkInformation::ItemVector const& deps = info->GetItems();

  for (auto const& dep : deps) {
    if (!dep.Target || dep.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      continue;
    }
    getPropertyContents(dep.Target, "COMPATIBLE_INTERFACE_BOOL",
                        ifaceProperties);
    getPropertyContents(dep.Target, "COMPATIBLE_INTERFACE_STRING",
                        ifaceProperties);
    getPropertyContents(dep.Target, "COMPATIBLE_INTERFACE_NUMBER_MIN",
                        ifaceProperties);
    getPropertyContents(dep.Target, "COMPATIBLE_INTERFACE_NUMBER_MAX",
                        ifaceProperties);
  }
}
}

void cmExportFileGenerator::PopulateCompatibleInterfaceProperties(
  cmGeneratorTarget const* gtarget, ImportPropertyMap& properties) const
{
  this->PopulateInterfaceProperty("COMPATIBLE_INTERFACE_BOOL", gtarget,
                                  properties);
  this->PopulateInterfaceProperty("COMPATIBLE_INTERFACE_STRING", gtarget,
                                  properties);
  this->PopulateInterfaceProperty("COMPATIBLE_INTERFACE_NUMBER_MIN", gtarget,
                                  properties);
  this->PopulateInterfaceProperty("COMPATIBLE_INTERFACE_NUMBER_MAX", gtarget,
                                  properties);

  std::set<std::string> ifaceProperties;

  getPropertyContents(gtarget, "COMPATIBLE_INTERFACE_BOOL", ifaceProperties);
  getPropertyContents(gtarget, "COMPATIBLE_INTERFACE_STRING", ifaceProperties);
  getPropertyContents(gtarget, "COMPATIBLE_INTERFACE_NUMBER_MIN",
                      ifaceProperties);
  getPropertyContents(gtarget, "COMPATIBLE_INTERFACE_NUMBER_MAX",
                      ifaceProperties);

  if (gtarget->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
    std::vector<std::string> configNames =
      gtarget->Target->GetMakefile()->GetGeneratorConfigs(
        cmMakefile::IncludeEmptyConfig);

    for (std::string const& cn : configNames) {
      getCompatibleInterfaceProperties(gtarget, ifaceProperties, cn);
    }
  }

  for (std::string const& ip : ifaceProperties) {
    this->PopulateInterfaceProperty("INTERFACE_" + ip, gtarget, properties);
  }
}

void cmExportFileGenerator::PopulateCustomTransitiveInterfaceProperties(
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  this->PopulateInterfaceProperty("TRANSITIVE_COMPILE_PROPERTIES", target,
                                  properties);
  this->PopulateInterfaceProperty("TRANSITIVE_LINK_PROPERTIES", target,
                                  properties);
  cmGeneratorTarget::CheckLinkLibrariesSuppressionRAII cllSuppressRAII;
  std::set<std::string> ifaceProperties;
  for (std::string const& config : this->Configurations) {
    for (auto const& i : target->GetCustomTransitiveProperties(
           config, cmGeneratorTarget::PropertyFor::Interface)) {
      ifaceProperties.emplace(i.second.InterfaceName);
    }
  }
  for (std::string const& ip : ifaceProperties) {
    this->PopulateInterfaceProperty(ip, target, preprocessRule, properties);
  }
}

bool cmExportFileGenerator::NoteLinkedTarget(
  cmGeneratorTarget const* /*target*/, std::string const& /*linkedName*/,
  cmGeneratorTarget const* /*linkedTarget*/)
{
  // Default implementation does nothing; only needed by some generators.
  return true;
}

bool cmExportFileGenerator::AddTargetNamespace(std::string& input,
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

  cmFindPackageStack const& pkgStack = tgt->Target->GetFindPackageStack();
  if (!pkgStack.Empty() ||
      tgt->Target->GetProperty("EXPORT_FIND_PACKAGE_NAME")) {
    this->ExternalTargets.emplace(tgt);
  }

  if (tgt->IsImported()) {
    input = tgt->GetName();
    return this->NoteLinkedTarget(target, input, tgt);
  }

  if (this->ExportedTargets.find(tgt) != this->ExportedTargets.end()) {
    input = this->Namespace + tgt->GetExportName();
  } else {
    std::string namespacedTarget;
    this->HandleMissingTarget(namespacedTarget, target, tgt);
    if (!namespacedTarget.empty()) {
      input = namespacedTarget;
    } else {
      input = tgt->GetName();
    }
  }

  return this->NoteLinkedTarget(target, input, tgt);
}

void cmExportFileGenerator::ResolveTargetsInGeneratorExpressions(
  std::string& input, cmGeneratorTarget const* target,
  FreeTargetsReplace replace)
{
  cmLocalGenerator const* lg = target->GetLocalGenerator();
  if (replace == NoReplaceFreeTargets) {
    this->ResolveTargetsInGeneratorExpression(input, target, lg);
    return;
  }
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

void cmExportFileGenerator::ResolveTargetsInGeneratorExpression(
  std::string& input, cmGeneratorTarget const* target,
  cmLocalGenerator const* lg)
{
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;

  while ((pos = input.find("$<TARGET_PROPERTY:", lastPos)) !=
         std::string::npos) {
    std::string::size_type nameStartPos = pos + cmStrLen("$<TARGET_PROPERTY:");
    std::string::size_type closePos = input.find('>', nameStartPos);
    std::string::size_type commaPos = input.find(',', nameStartPos);
    std::string::size_type nextOpenPos = input.find("$<", nameStartPos);
    if (commaPos == std::string::npos    // Implied 'this' target
        || closePos == std::string::npos // Incomplete expression.
        || closePos < commaPos           // Implied 'this' target
        || nextOpenPos < commaPos)       // Non-literal
    {
      lastPos = nameStartPos;
      continue;
    }

    std::string targetName =
      input.substr(nameStartPos, commaPos - nameStartPos);

    if (this->AddTargetNamespace(targetName, target, lg)) {
      input.replace(nameStartPos, commaPos - nameStartPos, targetName);
    }
    lastPos = nameStartPos + targetName.size() + 1;
  }

  std::string errorString;
  pos = 0;
  lastPos = pos;
  while ((pos = input.find("$<TARGET_NAME:", lastPos)) != std::string::npos) {
    std::string::size_type nameStartPos = pos + cmStrLen("$<TARGET_NAME:");
    std::string::size_type endPos = input.find('>', nameStartPos);
    if (endPos == std::string::npos) {
      errorString = "$<TARGET_NAME:...> expression incomplete";
      break;
    }
    std::string targetName = input.substr(nameStartPos, endPos - nameStartPos);
    if (targetName.find("$<") != std::string::npos) {
      errorString = "$<TARGET_NAME:...> requires its parameter to be a "
                    "literal.";
      break;
    }
    if (!this->AddTargetNamespace(targetName, target, lg)) {
      errorString = "$<TARGET_NAME:...> requires its parameter to be a "
                    "reachable target.";
      break;
    }
    input.replace(pos, endPos - pos + 1, targetName);
    lastPos = pos + targetName.size();
  }

  pos = 0;
  lastPos = pos;
  while (errorString.empty() &&
         (pos = input.find("$<LINK_ONLY:", lastPos)) != std::string::npos) {
    std::string::size_type nameStartPos = pos + cmStrLen("$<LINK_ONLY:");
    std::string::size_type endPos = input.find('>', nameStartPos);
    if (endPos == std::string::npos) {
      errorString = "$<LINK_ONLY:...> expression incomplete";
      break;
    }
    std::string libName = input.substr(nameStartPos, endPos - nameStartPos);
    if (cmGeneratorExpression::IsValidTargetName(libName) &&
        this->AddTargetNamespace(libName, target, lg)) {
      input.replace(nameStartPos, endPos - nameStartPos, libName);
    }
    lastPos = nameStartPos + libName.size() + 1;
  }

  while (errorString.empty() &&
         (pos = input.find("$<COMPILE_ONLY:", lastPos)) != std::string::npos) {
    std::string::size_type nameStartPos = pos + cmStrLen("$<COMPILE_ONLY:");
    std::string::size_type endPos = input.find('>', nameStartPos);
    if (endPos == std::string::npos) {
      errorString = "$<COMPILE_ONLY:...> expression incomplete";
      break;
    }
    std::string libName = input.substr(nameStartPos, endPos - nameStartPos);
    if (cmGeneratorExpression::IsValidTargetName(libName) &&
        this->AddTargetNamespace(libName, target, lg)) {
      input.replace(nameStartPos, endPos - nameStartPos, libName);
    }
    lastPos = nameStartPos + libName.size() + 1;
  }

  this->ReplaceInstallPrefix(input);

  if (!errorString.empty()) {
    target->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR,
                                              errorString);
  }
}

void cmExportFileGenerator::ReplaceInstallPrefix(std::string& /*unused*/) const
{
  // Do nothing
}

void cmExportFileGenerator::SetImportDetailProperties(
  std::string const& config, std::string const& suffix,
  cmGeneratorTarget const* target, ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->Makefile;

  // Add the soname for unix shared libraries.
  if (target->GetType() == cmStateEnums::SHARED_LIBRARY ||
      target->GetType() == cmStateEnums::MODULE_LIBRARY) {
    if (!target->IsDLLPlatform()) {
      std::string prop;
      std::string value;
      if (target->HasSOName(config)) {
        if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
          value = this->InstallNameDir(target, config);
        }
        prop = "IMPORTED_SONAME";
        value += target->GetSOName(config);
      } else {
        prop = "IMPORTED_NO_SONAME";
        value = "TRUE";
      }
      prop += suffix;
      properties[prop] = value;
    }
  }

  // Add the transitive link dependencies for this configuration.
  if (cmLinkInterface const* iface =
        target->GetLinkInterface(config, target)) {
    this->SetImportLinkProperty(
      suffix, target, "IMPORTED_LINK_INTERFACE_LANGUAGES", iface->Languages,
      properties, ImportLinkPropertyTargetNames::No);

    // Export IMPORTED_LINK_DEPENDENT_LIBRARIES to help consuming linkers
    // find private dependencies of shared libraries.
    std::size_t oldMissingTargetsSize = this->MissingTargets.size();
    auto oldExternalTargets = this->ExternalTargets;
    this->SetImportLinkProperty(
      suffix, target, "IMPORTED_LINK_DEPENDENT_LIBRARIES", iface->SharedDeps,
      properties, ImportLinkPropertyTargetNames::Yes);
    // Avoid enforcing shared library private dependencies as public package
    // dependencies by ignoring missing targets added for them.
    this->MissingTargets.resize(oldMissingTargetsSize);
    this->ExternalTargets = std::move(oldExternalTargets);

    if (iface->Multiplicity > 0) {
      std::string prop =
        cmStrCat("IMPORTED_LINK_INTERFACE_MULTIPLICITY", suffix);
      properties[prop] = std::to_string(iface->Multiplicity);
    }
  }

  // Add information if this target is a managed target
  if (target->GetManagedType(config) !=
      cmGeneratorTarget::ManagedType::Native) {
    std::string prop = cmStrCat("IMPORTED_COMMON_LANGUAGE_RUNTIME", suffix);
    std::string propval;
    if (cmValue p = target->GetProperty("COMMON_LANGUAGE_RUNTIME")) {
      propval = *p;
    } else if (target->IsCSharpOnly()) {
      // C# projects do not have the /clr flag, so we set the property
      // here to mark the target as (only) managed (i.e. no .lib file
      // to link to). Otherwise the  COMMON_LANGUAGE_RUNTIME target
      // property would have to be set manually for C# targets to make
      // exporting/importing work.
      propval = "CSharp";
    }
    properties[prop] = propval;
  }
}

namespace {
std::string const& asString(std::string const& l)
{
  return l;
}

std::string const& asString(cmLinkItem const& l)
{
  return l.AsStr();
}
}

template <typename T>
void cmExportFileGenerator::SetImportLinkProperty(
  std::string const& suffix, cmGeneratorTarget const* target,
  std::string const& propName, std::vector<T> const& entries,
  ImportPropertyMap& properties, ImportLinkPropertyTargetNames targetNames)
{
  // Skip the property if there are no entries.
  if (entries.empty()) {
    return;
  }

  cmLocalGenerator const* lg = target->GetLocalGenerator();

  // Construct the property value.
  std::string link_entries;
  char const* sep = "";
  for (T const& l : entries) {
    // Separate this from the previous entry.
    link_entries += sep;
    sep = ";";

    if (targetNames == ImportLinkPropertyTargetNames::Yes) {
      std::string temp = asString(l);
      this->AddTargetNamespace(temp, target, lg);
      link_entries += temp;
    } else {
      link_entries += asString(l);
    }
  }

  // Store the property.
  std::string prop = cmStrCat(propName, suffix);
  properties[prop] = link_entries;
}

template void cmExportFileGenerator::SetImportLinkProperty<std::string>(
  std::string const&, cmGeneratorTarget const*, std::string const&,
  std::vector<std::string> const&, ImportPropertyMap& properties,
  ImportLinkPropertyTargetNames);

template void cmExportFileGenerator::SetImportLinkProperty<cmLinkItem>(
  std::string const&, cmGeneratorTarget const*, std::string const&,
  std::vector<cmLinkItem> const&, ImportPropertyMap& properties,
  ImportLinkPropertyTargetNames);

namespace {
enum class ExportWhen
{
  Defined,
  Always,
};

enum class PropertyType
{
  Strings,
  Paths,
  IncludePaths,
};

bool PropertyTypeIsForPaths(PropertyType pt)
{
  switch (pt) {
    case PropertyType::Strings:
      return false;
    case PropertyType::Paths:
    case PropertyType::IncludePaths:
      return true;
  }
  return false;
}
}

bool cmExportFileGenerator::PopulateCxxModuleExportProperties(
  cmGeneratorTarget const* gte, ImportPropertyMap& properties,
  cmGeneratorExpression::PreprocessContext ctx,
  std::string const& includesDestinationDirs, std::string& errorMessage)
{
  if (!gte->HaveCxx20ModuleSources(&errorMessage)) {
    return true;
  }

  struct ModuleTargetPropertyTable
  {
    cm::static_string_view Name;
    ExportWhen Cond;
  };

  ModuleTargetPropertyTable const exportedDirectModuleProperties[] = {
    { "CXX_EXTENSIONS"_s, ExportWhen::Defined },
    // Always define this property as it is an intrinsic property of the target
    // and should not be inherited from the in-scope `CMAKE_CXX_MODULE_STD`
    // variable.
    //
    // TODO(cxxmodules): A future policy may make this "ON" based on the target
    // policies if unset. Add a new `ExportWhen` condition to handle it when
    // this happens.
    { "CXX_MODULE_STD"_s, ExportWhen::Always },
  };
  for (auto const& prop : exportedDirectModuleProperties) {
    auto const propNameStr = std::string(prop.Name);
    cmValue propValue = gte->Target->GetComputedProperty(
      propNameStr, *gte->Target->GetMakefile());
    if (!propValue) {
      propValue = gte->Target->GetProperty(propNameStr);
    }
    if (propValue) {
      properties[propNameStr] =
        cmGeneratorExpression::Preprocess(*propValue, ctx);
    } else if (prop.Cond == ExportWhen::Always) {
      properties[propNameStr] = "";
    }
  }

  struct ModulePropertyTable
  {
    cm::static_string_view Name;
    PropertyType Type;
  };

  ModulePropertyTable const exportedModuleProperties[] = {
    { "INCLUDE_DIRECTORIES"_s, PropertyType::IncludePaths },
    { "COMPILE_DEFINITIONS"_s, PropertyType::Strings },
    { "COMPILE_OPTIONS"_s, PropertyType::Strings },
    { "COMPILE_FEATURES"_s, PropertyType::Strings },
  };
  for (auto const& propEntry : exportedModuleProperties) {
    auto const propNameStr = std::string(propEntry.Name);
    cmValue prop = gte->Target->GetComputedProperty(
      propNameStr, *gte->Target->GetMakefile());
    if (!prop) {
      prop = gte->Target->GetProperty(propNameStr);
    }
    if (prop) {
      auto const exportedPropName =
        cmStrCat("IMPORTED_CXX_MODULES_", propEntry.Name);
      properties[exportedPropName] =
        cmGeneratorExpression::Preprocess(*prop, ctx);
      if (ctx == cmGeneratorExpression::InstallInterface &&
          PropertyTypeIsForPaths(propEntry.Type)) {
        this->ReplaceInstallPrefix(properties[exportedPropName]);
        this->AddImportPrefix(properties[exportedPropName]);
        if (propEntry.Type == PropertyType::IncludePaths &&
            !includesDestinationDirs.empty()) {
          if (!properties[exportedPropName].empty()) {
            properties[exportedPropName] += ';';
          }
          properties[exportedPropName] += includesDestinationDirs;
        }
      }
    }
  }

  cm::static_string_view const exportedLinkModuleProperties[] = {
    "LINK_LIBRARIES"_s,
  };
  for (auto const& propName : exportedLinkModuleProperties) {
    auto const propNameStr = std::string(propName);
    cmValue prop = gte->Target->GetComputedProperty(
      propNameStr, *gte->Target->GetMakefile());
    if (!prop) {
      prop = gte->Target->GetProperty(propNameStr);
    }
    if (prop) {
      auto const exportedPropName =
        cmStrCat("IMPORTED_CXX_MODULES_", propName);
      auto value = cmGeneratorExpression::Preprocess(*prop, ctx);
      this->ResolveTargetsInGeneratorExpressions(value, gte,
                                                 ReplaceFreeTargets);
      properties[exportedPropName] = value;
    }
  }

  return true;
}

bool cmExportFileGenerator::PopulateExportProperties(
  cmGeneratorTarget const* gte, ImportPropertyMap& properties,
  std::string& errorMessage) const
{
  auto const& targetProperties = gte->Target->GetProperties();
  if (cmValue exportProperties =
        targetProperties.GetPropertyValue("EXPORT_PROPERTIES")) {
    for (auto& prop : cmList{ *exportProperties }) {
      /* Black list reserved properties */
      if (cmHasLiteralPrefix(prop, "IMPORTED_") ||
          cmHasLiteralPrefix(prop, "INTERFACE_")) {
        std::ostringstream e;
        e << "Target \"" << gte->Target->GetName() << "\" contains property \""
          << prop << "\" in EXPORT_PROPERTIES but IMPORTED_* and INTERFACE_* "
          << "properties are reserved.";
        errorMessage = e.str();
        return false;
      }
      cmValue propertyValue = targetProperties.GetPropertyValue(prop);
      if (!propertyValue) {
        // Asked to export a property that isn't defined on the target. Do not
        // consider this an error, there's just nothing to export.
        continue;
      }
      std::string evaluatedValue = cmGeneratorExpression::Preprocess(
        *propertyValue, cmGeneratorExpression::StripAllGeneratorExpressions);
      if (evaluatedValue != *propertyValue) {
        std::ostringstream e;
        e << "Target \"" << gte->Target->GetName() << "\" contains property \""
          << prop << "\" in EXPORT_PROPERTIES but this property contains a "
          << "generator expression. This is not allowed.";
        errorMessage = e.str();
        return false;
      }
      properties[prop] = *propertyValue;
    }
  }
  return true;
}
