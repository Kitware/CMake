/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorTarget.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <queue>
#include <sstream>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCryptoHash.h"
#include "cmCustomCommandGenerator.h"
#include "cmCxxModuleUsageEffects.h"
#include "cmEvaluatedTargetProperty.h"
#include "cmFileSet.h"
#include "cmFileTimes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionNode.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPropertyMap.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmSourceGroup.h"
#include "cmStandardLevel.h"
#include "cmStandardLevelResolver.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSyntheticTargetCache.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmTargetPropertyComputer.h"
#include "cmake.h"

namespace {
using LinkInterfaceFor = cmGeneratorTarget::LinkInterfaceFor;

const std::string kINTERFACE_LINK_LIBRARIES = "INTERFACE_LINK_LIBRARIES";
const std::string kINTERFACE_LINK_LIBRARIES_DIRECT =
  "INTERFACE_LINK_LIBRARIES_DIRECT";
const std::string kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE =
  "INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE";
}

template <>
cmValue cmTargetPropertyComputer::GetSources<cmGeneratorTarget>(
  cmGeneratorTarget const* tgt, cmMakefile const& /* mf */)
{
  return tgt->GetSourcesProperty();
}

template <>
const std::string&
cmTargetPropertyComputer::ComputeLocationForBuild<cmGeneratorTarget>(
  cmGeneratorTarget const* tgt)
{
  return tgt->GetLocation("");
}

template <>
const std::string&
cmTargetPropertyComputer::ComputeLocation<cmGeneratorTarget>(
  cmGeneratorTarget const* tgt, const std::string& config)
{
  return tgt->GetLocation(config);
}

cmLinkImplItem cmGeneratorTarget::TargetPropertyEntry::NoLinkImplItem;

class TargetPropertyEntryString : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryString(BT<std::string> propertyValue,
                            cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , PropertyValue(std::move(propertyValue))
  {
  }

  const std::string& Evaluate(cmLocalGenerator*, const std::string&,
                              cmGeneratorTarget const*,
                              cmGeneratorExpressionDAGChecker*,
                              std::string const&) const override
  {
    return this->PropertyValue.Value;
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->PropertyValue.Backtrace;
  }
  std::string const& GetInput() const override
  {
    return this->PropertyValue.Value;
  }

private:
  BT<std::string> PropertyValue;
};

class TargetPropertyEntryGenex : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryGenex(std::unique_ptr<cmCompiledGeneratorExpression> cge,
                           cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , ge(std::move(cge))
  {
  }

  const std::string& Evaluate(cmLocalGenerator* lg, const std::string& config,
                              cmGeneratorTarget const* headTarget,
                              cmGeneratorExpressionDAGChecker* dagChecker,
                              std::string const& language) const override
  {
    return this->ge->Evaluate(lg, config, headTarget, dagChecker, nullptr,
                              language);
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->ge->GetBacktrace();
  }

  std::string const& GetInput() const override { return this->ge->GetInput(); }

  bool GetHadContextSensitiveCondition() const override
  {
    return this->ge->GetHadContextSensitiveCondition();
  }

private:
  const std::unique_ptr<cmCompiledGeneratorExpression> ge;
};

class TargetPropertyEntryFileSet
  : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryFileSet(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
    const cmFileSet* fileSet, cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , BaseDirs(std::move(dirs))
    , ContextSensitiveDirs(contextSensitiveDirs)
    , EntryCge(std::move(entryCge))
    , FileSet(fileSet)
  {
  }

  const std::string& Evaluate(cmLocalGenerator* lg, const std::string& config,
                              cmGeneratorTarget const* headTarget,
                              cmGeneratorExpressionDAGChecker* dagChecker,
                              std::string const& /*lang*/) const override
  {
    std::map<std::string, std::vector<std::string>> filesPerDir;
    this->FileSet->EvaluateFileEntry(this->BaseDirs, filesPerDir,
                                     this->EntryCge, lg, config, headTarget,
                                     dagChecker);

    std::vector<std::string> files;
    for (auto const& it : filesPerDir) {
      files.insert(files.end(), it.second.begin(), it.second.end());
    }

    static std::string filesStr;
    filesStr = cmList::to_string(files);
    return filesStr;
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->EntryCge->GetBacktrace();
  }

  std::string const& GetInput() const override
  {
    return this->EntryCge->GetInput();
  }

  bool GetHadContextSensitiveCondition() const override
  {
    return this->ContextSensitiveDirs ||
      this->EntryCge->GetHadContextSensitiveCondition();
  }

private:
  const std::vector<std::string> BaseDirs;
  const bool ContextSensitiveDirs;
  const std::unique_ptr<cmCompiledGeneratorExpression> EntryCge;
  const cmFileSet* FileSet;
};

std::unique_ptr<
  cmGeneratorTarget::
    TargetPropertyEntry> static CreateTargetPropertyEntry(cmake& cmakeInstance,
                                                          const BT<
                                                            std::string>&
                                                            propertyValue,
                                                          bool
                                                            evaluateForBuildsystem =
                                                              false)
{
  if (cmGeneratorExpression::Find(propertyValue.Value) != std::string::npos) {
    cmGeneratorExpression ge(cmakeInstance, propertyValue.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(propertyValue.Value);
    cge->SetEvaluateForBuildsystem(evaluateForBuildsystem);
    return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
      cm::make_unique<TargetPropertyEntryGenex>(std::move(cge)));
  }

  return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
    cm::make_unique<TargetPropertyEntryString>(propertyValue));
}

cmGeneratorTarget::TargetPropertyEntry::TargetPropertyEntry(
  cmLinkImplItem const& item)
  : LinkImplItem(item)
{
}

bool cmGeneratorTarget::TargetPropertyEntry::GetHadContextSensitiveCondition()
  const
{
  return false;
}

static void CreatePropertyGeneratorExpressions(
  cmake& cmakeInstance, cmBTStringRange entries,
  std::vector<std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>>& items,
  bool evaluateForBuildsystem = false)
{
  for (auto const& entry : entries) {
    items.push_back(
      CreateTargetPropertyEntry(cmakeInstance, entry, evaluateForBuildsystem));
  }
}

cmGeneratorTarget::cmGeneratorTarget(cmTarget* t, cmLocalGenerator* lg)
  : Target(t)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = lg;
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();

  this->GlobalGenerator->ComputeTargetObjectDirectory(this);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetIncludeDirectoriesEntries(),
                                     this->IncludeDirectoriesEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetCompileOptionsEntries(),
                                     this->CompileOptionsEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetCompileFeaturesEntries(),
                                     this->CompileFeaturesEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetCompileDefinitionsEntries(),
                                     this->CompileDefinitionsEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetLinkOptionsEntries(),
                                     this->LinkOptionsEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetLinkDirectoriesEntries(),
                                     this->LinkDirectoriesEntries);

  CreatePropertyGeneratorExpressions(*lg->GetCMakeInstance(),
                                     t->GetPrecompileHeadersEntries(),
                                     this->PrecompileHeadersEntries);

  CreatePropertyGeneratorExpressions(
    *lg->GetCMakeInstance(), t->GetSourceEntries(), this->SourceEntries, true);

  this->PolicyMap = t->GetPolicyMap();

  // Get hard-coded linker language
  if (this->Target->GetProperty("HAS_CXX")) {
    this->LinkerLanguage = "CXX";
  } else {
    this->LinkerLanguage = this->Target->GetSafeProperty("LINKER_LANGUAGE");
  }
}

cmGeneratorTarget::~cmGeneratorTarget() = default;

cmValue cmGeneratorTarget::GetSourcesProperty() const
{
  std::vector<std::string> values;
  for (auto const& se : this->SourceEntries) {
    values.push_back(se->GetInput());
  }
  static std::string value;
  value = cmList::to_string(values);
  return cmValue(value);
}

cmGlobalGenerator* cmGeneratorTarget::GetGlobalGenerator() const
{
  return this->GetLocalGenerator()->GetGlobalGenerator();
}

cmLocalGenerator* cmGeneratorTarget::GetLocalGenerator() const
{
  return this->LocalGenerator;
}

cmStateEnums::TargetType cmGeneratorTarget::GetType() const
{
  return this->Target->GetType();
}

const std::string& cmGeneratorTarget::GetName() const
{
  return this->Target->GetName();
}

std::string cmGeneratorTarget::GetExportName() const
{
  cmValue exportName = this->GetProperty("EXPORT_NAME");

  if (cmNonempty(exportName)) {
    if (!cmGeneratorExpression::IsValidTargetName(*exportName)) {
      std::ostringstream e;
      e << "EXPORT_NAME property \"" << *exportName << "\" for \""
        << this->GetName() << "\": is not valid.";
      cmSystemTools::Error(e.str());
      return "";
    }
    return *exportName;
  }
  return this->GetName();
}

std::string cmGeneratorTarget::GetFilesystemExportName() const
{
  auto fs_safe = this->GetExportName();
  // First escape any `_` characters to avoid collisions.
  cmSystemTools::ReplaceString(fs_safe, "_", "__");
  // Escape other characters that are not generally filesystem-safe.
  cmSystemTools::ReplaceString(fs_safe, ":", "_c");
  return fs_safe;
}

cmValue cmGeneratorTarget::GetProperty(const std::string& prop) const
{
  if (cmValue result =
        cmTargetPropertyComputer::GetProperty(this, prop, *this->Makefile)) {
    return result;
  }
  if (cmSystemTools::GetFatalErrorOccurred()) {
    return nullptr;
  }
  return this->Target->GetProperty(prop);
}

std::string const& cmGeneratorTarget::GetSafeProperty(
  std::string const& prop) const
{
  return this->GetProperty(prop);
}

const char* cmGeneratorTarget::GetOutputTargetType(
  cmStateEnums::ArtifactType artifact) const
{
  if (this->IsFrameworkOnApple() || this->GetGlobalGenerator()->IsXcode()) {
    // import file (i.e. .tbd file) is always in same location as library
    artifact = cmStateEnums::RuntimeBinaryArtifact;
  }

  switch (this->GetType()) {
    case cmStateEnums::SHARED_LIBRARY:
      if (this->IsDLLPlatform()) {
        switch (artifact) {
          case cmStateEnums::RuntimeBinaryArtifact:
            // A DLL shared library is treated as a runtime target.
            return "RUNTIME";
          case cmStateEnums::ImportLibraryArtifact:
            // A DLL import library is treated as an archive target.
            return "ARCHIVE";
        }
      } else {
        switch (artifact) {
          case cmStateEnums::RuntimeBinaryArtifact:
            // For non-DLL platforms shared libraries are treated as
            // library targets.
            return "LIBRARY";
          case cmStateEnums::ImportLibraryArtifact:
            // Library import libraries are treated as archive targets.
            return "ARCHIVE";
        }
      }
      break;
    case cmStateEnums::STATIC_LIBRARY:
      // Static libraries are always treated as archive targets.
      return "ARCHIVE";
    case cmStateEnums::MODULE_LIBRARY:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          // Module libraries are always treated as library targets.
          return "LIBRARY";
        case cmStateEnums::ImportLibraryArtifact:
          // Module import libraries are treated as archive targets.
          return "ARCHIVE";
      }
      break;
    case cmStateEnums::OBJECT_LIBRARY:
      // Object libraries are always treated as object targets.
      return "OBJECT";
    case cmStateEnums::EXECUTABLE:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          // Executables are always treated as runtime targets.
          return "RUNTIME";
        case cmStateEnums::ImportLibraryArtifact:
          // Executable import libraries are treated as archive targets.
          return "ARCHIVE";
      }
      break;
    default:
      break;
  }
  return "";
}

std::string cmGeneratorTarget::GetOutputName(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  // Lookup/compute/cache the output name for this configuration.
  OutputNameKey key(config, artifact);
  auto i = this->OutputNameMap.find(key);
  if (i == this->OutputNameMap.end()) {
    // Add empty name in map to detect potential recursion.
    OutputNameMapType::value_type entry(key, "");
    i = this->OutputNameMap.insert(entry).first;

    // Compute output name.
    std::vector<std::string> props;
    std::string type = this->GetOutputTargetType(artifact);
    std::string configUpper = cmSystemTools::UpperCase(config);
    if (!type.empty() && !configUpper.empty()) {
      // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME_<CONFIG>
      props.push_back(type + "_OUTPUT_NAME_" + configUpper);
    }
    if (!type.empty()) {
      // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME
      props.push_back(type + "_OUTPUT_NAME");
    }
    if (!configUpper.empty()) {
      // OUTPUT_NAME_<CONFIG>
      props.push_back("OUTPUT_NAME_" + configUpper);
      // <CONFIG>_OUTPUT_NAME
      props.push_back(configUpper + "_OUTPUT_NAME");
    }
    // OUTPUT_NAME
    props.emplace_back("OUTPUT_NAME");

    std::string outName;
    for (std::string const& p : props) {
      if (cmValue outNameProp = this->GetProperty(p)) {
        outName = *outNameProp;
        break;
      }
    }

    if (outName.empty()) {
      outName = this->GetName();
    }

    // Now evaluate genex and update the previously-prepared map entry.
    i->second =
      cmGeneratorExpression::Evaluate(outName, this->LocalGenerator, config);
  } else if (i->second.empty()) {
    // An empty map entry indicates we have been called recursively
    // from the above block.
    this->LocalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      "Target '" + this->GetName() + "' OUTPUT_NAME depends on itself.",
      this->GetBacktrace());
  }
  return i->second;
}

std::string cmGeneratorTarget::GetFilePrefix(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    cmValue prefix = this->GetFilePrefixInternal(config, artifact);
    return prefix ? *prefix : std::string();
  }
  return this->GetFullNameInternalComponents(config, artifact).prefix;
}
std::string cmGeneratorTarget::GetFileSuffix(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    cmValue suffix = this->GetFileSuffixInternal(config, artifact);
    return suffix ? *suffix : std::string();
  }
  return this->GetFullNameInternalComponents(config, artifact).suffix;
}

std::string cmGeneratorTarget::GetFilePostfix(const std::string& config) const
{
  cmValue postfix = nullptr;
  std::string frameworkPostfix;
  if (!config.empty()) {
    std::string configProp =
      cmStrCat(cmSystemTools::UpperCase(config), "_POSTFIX");
    postfix = this->GetProperty(configProp);

    // Mac application bundles and frameworks have no regular postfix like
    // libraries do.
    if (!this->IsImported() && postfix &&
        (this->IsAppBundleOnApple() || this->IsFrameworkOnApple())) {
      postfix = nullptr;
    }

    // Frameworks created by multi config generators can have a special
    // framework postfix.
    frameworkPostfix = this->GetFrameworkMultiConfigPostfix(config);
    if (!frameworkPostfix.empty()) {
      postfix = cmValue(frameworkPostfix);
    }
  }
  return postfix ? *postfix : std::string();
}

std::string cmGeneratorTarget::GetFrameworkMultiConfigPostfix(
  const std::string& config) const
{
  cmValue postfix = nullptr;
  if (!config.empty()) {
    std::string configProp = cmStrCat("FRAMEWORK_MULTI_CONFIG_POSTFIX_",
                                      cmSystemTools::UpperCase(config));
    postfix = this->GetProperty(configProp);

    if (!this->IsImported() && postfix &&
        (this->IsFrameworkOnApple() &&
         !this->GetGlobalGenerator()->IsMultiConfig())) {
      postfix = nullptr;
    }
  }
  return postfix ? *postfix : std::string();
}

cmValue cmGeneratorTarget::GetFilePrefixInternal(
  std::string const& config, cmStateEnums::ArtifactType artifact,
  const std::string& language) const
{
  // no prefix for non-main target types.
  if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    return nullptr;
  }

  const bool isImportedLibraryArtifact =
    (artifact == cmStateEnums::ImportLibraryArtifact);

  // Return an empty prefix for the import library if this platform
  // does not support import libraries.
  if (isImportedLibraryArtifact && !this->NeedImportLibraryName(config)) {
    return nullptr;
  }

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    artifact = cmStateEnums::RuntimeBinaryArtifact;
  }

  // Compute prefix value.
  cmValue targetPrefix =
    (isImportedLibraryArtifact ? this->GetProperty("IMPORT_PREFIX")
                               : this->GetProperty("PREFIX"));

  if (!targetPrefix) {
    const char* prefixVar = this->Target->GetPrefixVariableInternal(artifact);
    if (!language.empty() && cmNonempty(prefixVar)) {
      std::string langPrefix = cmStrCat(prefixVar, "_", language);
      targetPrefix = this->Makefile->GetDefinition(langPrefix);
    }

    // if there is no prefix on the target nor specific language
    // use the cmake definition.
    if (!targetPrefix && prefixVar) {
      targetPrefix = this->Makefile->GetDefinition(prefixVar);
    }
  }

  return targetPrefix;
}

cmValue cmGeneratorTarget::GetFileSuffixInternal(
  std::string const& config, cmStateEnums::ArtifactType artifact,
  const std::string& language) const
{
  // no suffix for non-main target types.
  if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    return nullptr;
  }

  const bool isImportedLibraryArtifact =
    (artifact == cmStateEnums::ImportLibraryArtifact);

  // Return an empty suffix for the import library if this platform
  // does not support import libraries.
  if (isImportedLibraryArtifact && !this->NeedImportLibraryName(config)) {
    return nullptr;
  }

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    artifact = cmStateEnums::RuntimeBinaryArtifact;
  }

  // Compute suffix value.
  cmValue targetSuffix =
    (isImportedLibraryArtifact ? this->GetProperty("IMPORT_SUFFIX")
                               : this->GetProperty("SUFFIX"));

  if (!targetSuffix) {
    const char* suffixVar = this->Target->GetSuffixVariableInternal(artifact);
    if (!language.empty() && cmNonempty(suffixVar)) {
      std::string langSuffix = cmStrCat(suffixVar, "_", language);
      targetSuffix = this->Makefile->GetDefinition(langSuffix);
    }

    // if there is no suffix on the target nor specific language
    // use the cmake definition.
    if (!targetSuffix && suffixVar) {
      targetSuffix = this->Makefile->GetDefinition(suffixVar);
    }
  }

  return targetSuffix;
}

void cmGeneratorTarget::ClearSourcesCache()
{
  this->AllConfigSources.clear();
  this->KindedSourcesMap.clear();
  this->SourcesAreContextDependent = Tribool::Indeterminate;
  this->Objects.clear();
  this->VisitedConfigsForObjects.clear();
  this->LinkImplMap.clear();
  this->LinkImplUsageRequirementsOnlyMap.clear();
  this->IncludeDirectoriesCache.clear();
  this->CompileOptionsCache.clear();
  this->CompileDefinitionsCache.clear();
  this->PrecompileHeadersCache.clear();
  this->LinkOptionsCache.clear();
  this->LinkDirectoriesCache.clear();
  this->RuntimeBinaryFullNameCache.clear();
  this->ImportLibraryFullNameCache.clear();
}

void cmGeneratorTarget::ClearLinkInterfaceCache()
{
  this->LinkInterfaceMap.clear();
  this->LinkInterfaceUsageRequirementsOnlyMap.clear();
}

void cmGeneratorTarget::AddSourceCommon(const std::string& src, bool before)
{
  this->SourceEntries.insert(
    before ? this->SourceEntries.begin() : this->SourceEntries.end(),
    CreateTargetPropertyEntry(
      *this->LocalGenerator->GetCMakeInstance(),
      BT<std::string>(src, this->Makefile->GetBacktrace()), true));
  this->ClearSourcesCache();
}

void cmGeneratorTarget::AddSource(const std::string& src, bool before)
{
  this->Target->AddSource(src, before);
  this->AddSourceCommon(src, before);
}

void cmGeneratorTarget::AddTracedSources(std::vector<std::string> const& srcs)
{
  this->Target->AddTracedSources(srcs);
  if (!srcs.empty()) {
    this->AddSourceCommon(cmJoin(srcs, ";"));
  }
}

void cmGeneratorTarget::AddIncludeDirectory(const std::string& src,
                                            bool before)
{
  this->Target->InsertInclude(
    BT<std::string>(src, this->Makefile->GetBacktrace()), before);
  this->IncludeDirectoriesEntries.insert(
    before ? this->IncludeDirectoriesEntries.begin()
           : this->IncludeDirectoriesEntries.end(),
    CreateTargetPropertyEntry(
      *this->Makefile->GetCMakeInstance(),
      BT<std::string>(src, this->Makefile->GetBacktrace()), true));
}

void cmGeneratorTarget::AddSystemIncludeDirectory(std::string const& inc,
                                                  std::string const& lang)
{
  std::string config_upper;
  auto const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  for (auto const& config : configs) {
    std::string inc_with_config = inc;
    if (!config.empty()) {
      cmSystemTools::ReplaceString(inc_with_config, "$<CONFIG>", config);
      config_upper = cmSystemTools::UpperCase(config);
    }
    auto const& key = cmStrCat(config_upper, "/", lang);
    this->Target->AddSystemIncludeDirectories({ inc_with_config });
    this->SystemIncludesCache[key].emplace_back(inc_with_config);

    // SystemIncludesCache should be sorted so that binary search can be used
    std::sort(this->SystemIncludesCache[key].begin(),
              this->SystemIncludesCache[key].end());
  }
}

std::vector<cmSourceFile*> const* cmGeneratorTarget::GetSourceDepends(
  cmSourceFile const* sf) const
{
  auto i = this->SourceDepends.find(sf);
  if (i != this->SourceDepends.end()) {
    return &i->second.Depends;
  }
  return nullptr;
}

namespace {
void handleSystemIncludesDep(cmLocalGenerator* lg,
                             cmGeneratorTarget const* depTgt,
                             const std::string& config,
                             cmGeneratorTarget const* headTarget,
                             cmGeneratorExpressionDAGChecker* dagChecker,
                             cmList& result, bool excludeImported,
                             std::string const& language)
{
  if (cmValue dirs =
        depTgt->GetProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES")) {
    result.append(cmGeneratorExpression::Evaluate(
      *dirs, lg, config, headTarget, dagChecker, depTgt, language));
  }
  if (!depTgt->GetPropertyAsBool("SYSTEM")) {
    return;
  }
  if (depTgt->IsImported()) {
    if (excludeImported) {
      return;
    }
    if (depTgt->GetPropertyAsBool("IMPORTED_NO_SYSTEM")) {
      return;
    }
  }

  if (cmValue dirs = depTgt->GetProperty("INTERFACE_INCLUDE_DIRECTORIES")) {
    result.append(cmGeneratorExpression::Evaluate(
      *dirs, lg, config, headTarget, dagChecker, depTgt, language));
  }

  if (depTgt->Target->IsFrameworkOnApple() ||
      depTgt->IsImportedFrameworkFolderOnApple(config)) {
    if (auto fwDescriptor = depTgt->GetGlobalGenerator()->SplitFrameworkPath(
          depTgt->GetLocation(config))) {
      result.push_back(fwDescriptor->Directory);
      result.push_back(fwDescriptor->GetFrameworkPath());
    }
  }
}
}

/* clang-format off */
#define IMPLEMENT_VISIT(KIND)                                                 \
  do {                                                                        \
    KindedSources const& kinded = this->GetKindedSources(config);             \
    for (SourceAndKind const& s : kinded.Sources) {                           \
      if (s.Kind == KIND) {                                                   \
        data.push_back(s.Source.Value);                                       \
      }                                                                       \
    }                                                                         \
  } while (false)
/* clang-format on */

void cmGeneratorTarget::GetObjectSources(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindObjectSource);

  if (this->VisitedConfigsForObjects.count(config)) {
    return;
  }

  for (cmSourceFile const* it : data) {
    this->Objects[it];
  }

  this->LocalGenerator->ComputeObjectFilenames(this->Objects, this);
  this->VisitedConfigsForObjects.insert(config);
}

void cmGeneratorTarget::ComputeObjectMapping()
{
  auto const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  std::set<std::string> configSet(configs.begin(), configs.end());
  if (configSet == this->VisitedConfigsForObjects) {
    return;
  }

  for (std::string const& c : configs) {
    std::vector<cmSourceFile const*> sourceFiles;
    this->GetObjectSources(sourceFiles, c);
  }
}

cmValue cmGeneratorTarget::GetFeature(const std::string& feature,
                                      const std::string& config) const
{
  if (!config.empty()) {
    std::string featureConfig =
      cmStrCat(feature, '_', cmSystemTools::UpperCase(config));
    if (cmValue value = this->GetProperty(featureConfig)) {
      return value;
    }
  }
  if (cmValue value = this->GetProperty(feature)) {
    return value;
  }
  return this->LocalGenerator->GetFeature(feature, config);
}

std::string cmGeneratorTarget::GetLinkerTypeProperty(
  std::string const& lang, std::string const& config) const
{
  std::string propName{ "LINKER_TYPE" };
  auto linkerType = this->GetProperty(propName);
  if (!linkerType.IsEmpty()) {
    cmGeneratorExpressionDAGChecker dagChecker(this, propName, nullptr,
                                               nullptr);
    auto ltype =
      cmGeneratorExpression::Evaluate(*linkerType, this->GetLocalGenerator(),
                                      config, this, &dagChecker, this, lang);
    if (this->IsDeviceLink()) {
      cmList list{ ltype };
      const auto DL_BEGIN = "<DEVICE_LINK>"_s;
      const auto DL_END = "</DEVICE_LINK>"_s;
      cm::erase_if(list, [&](const std::string& item) {
        return item == DL_BEGIN || item == DL_END;
      });
      return list.to_string();
    }
    return ltype;
  }
  return std::string{};
}

const char* cmGeneratorTarget::GetLinkPIEProperty(
  const std::string& config) const
{
  static std::string PICValue;

  PICValue = this->GetLinkInterfaceDependentStringAsBoolProperty(
    "POSITION_INDEPENDENT_CODE", config);

  if (PICValue == "(unset)") {
    // POSITION_INDEPENDENT_CODE is not set
    return nullptr;
  }

  auto status = this->GetPolicyStatusCMP0083();
  return (status != cmPolicies::WARN && status != cmPolicies::OLD)
    ? PICValue.c_str()
    : nullptr;
}

bool cmGeneratorTarget::IsIPOEnabled(std::string const& lang,
                                     std::string const& config) const
{
  cmValue feature = this->GetFeature("INTERPROCEDURAL_OPTIMIZATION", config);

  if (!cmIsOn(feature)) {
    // 'INTERPROCEDURAL_OPTIMIZATION' is off, no need to check policies
    return false;
  }

  if (lang != "C" && lang != "CXX" && lang != "CUDA" && lang != "Fortran") {
    // We do not define IPO behavior for other languages.
    return false;
  }

  if (lang == "CUDA") {
    // CUDA IPO requires both CUDA_ARCHITECTURES and CUDA_SEPARABLE_COMPILATION
    if (cmIsOff(this->GetSafeProperty("CUDA_ARCHITECTURES")) ||
        cmIsOff(this->GetSafeProperty("CUDA_SEPARABLE_COMPILATION"))) {
      return false;
    }
  }

  cmPolicies::PolicyStatus cmp0069 = this->GetPolicyStatusCMP0069();

  if (cmp0069 == cmPolicies::OLD || cmp0069 == cmPolicies::WARN) {
    if (this->Makefile->IsOn("_CMAKE_" + lang + "_IPO_LEGACY_BEHAVIOR")) {
      return true;
    }
    if (this->PolicyReportedCMP0069) {
      // problem is already reported, no need to issue a message
      return false;
    }
    const bool in_try_compile =
      this->LocalGenerator->GetCMakeInstance()->GetIsInTryCompile();
    if (cmp0069 == cmPolicies::WARN && !in_try_compile) {
      std::ostringstream w;
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0069) << "\n";
      w << "INTERPROCEDURAL_OPTIMIZATION property will be ignored for target "
        << "'" << this->GetName() << "'.";
      this->LocalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::AUTHOR_WARNING, w.str(), this->GetBacktrace());

      this->PolicyReportedCMP0069 = true;
    }
    return false;
  }

  // Note: check consistency with messages from CheckIPOSupported
  const char* message = nullptr;
  if (!this->Makefile->IsOn("_CMAKE_" + lang + "_IPO_SUPPORTED_BY_CMAKE")) {
    message = "CMake doesn't support IPO for current compiler";
  } else if (!this->Makefile->IsOn("_CMAKE_" + lang +
                                   "_IPO_MAY_BE_SUPPORTED_BY_COMPILER")) {
    message = "Compiler doesn't support IPO";
  } else if (!this->GlobalGenerator->IsIPOSupported()) {
    message = "CMake doesn't support IPO for current generator";
  }

  if (!message) {
    // No error/warning messages
    return true;
  }

  if (this->PolicyReportedCMP0069) {
    // problem is already reported, no need to issue a message
    return false;
  }

  this->PolicyReportedCMP0069 = true;

  this->LocalGenerator->GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR, message, this->GetBacktrace());
  return false;
}

const std::string& cmGeneratorTarget::GetObjectName(cmSourceFile const* file)
{
  this->ComputeObjectMapping();
  return this->Objects[file];
}

const char* cmGeneratorTarget::GetCustomObjectExtension() const
{
  struct compiler_mode
  {
    std::string variable;
    std::string extension;
  };
  static std::array<compiler_mode, 4> const modes{
    { { "CUDA_PTX_COMPILATION", ".ptx" },
      { "CUDA_CUBIN_COMPILATION", ".cubin" },
      { "CUDA_FATBIN_COMPILATION", ".fatbin" },
      { "CUDA_OPTIX_COMPILATION", ".optixir" } }
  };

  std::string const& compiler =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_ID");
  if (!compiler.empty()) {
    for (const auto& m : modes) {
      const bool has_extension = this->GetPropertyAsBool(m.variable);
      if (has_extension) {
        return m.extension.c_str();
      }
    }
  }
  return nullptr;
}

void cmGeneratorTarget::AddExplicitObjectName(cmSourceFile const* sf)
{
  this->ExplicitObjectName.insert(sf);
}

bool cmGeneratorTarget::HasExplicitObjectName(cmSourceFile const* file) const
{
  const_cast<cmGeneratorTarget*>(this)->ComputeObjectMapping();
  auto it = this->ExplicitObjectName.find(file);
  return it != this->ExplicitObjectName.end();
}

BTs<std::string> const* cmGeneratorTarget::GetLanguageStandardProperty(
  std::string const& lang, std::string const& config) const
{
  std::string key = cmStrCat(cmSystemTools::UpperCase(config), '-', lang);
  auto langStandardIter = this->LanguageStandardMap.find(key);
  if (langStandardIter != this->LanguageStandardMap.end()) {
    return &langStandardIter->second;
  }

  return this->Target->GetLanguageStandardProperty(
    cmStrCat(lang, "_STANDARD"));
}

cmValue cmGeneratorTarget::GetLanguageStandard(std::string const& lang,
                                               std::string const& config) const
{
  BTs<std::string> const* languageStandard =
    this->GetLanguageStandardProperty(lang, config);

  if (languageStandard) {
    return cmValue(languageStandard->Value);
  }

  return nullptr;
}

cmValue cmGeneratorTarget::GetPropertyWithPairedLanguageSupport(
  std::string const& lang, const char* suffix) const
{
  cmValue propertyValue = this->Target->GetProperty(cmStrCat(lang, suffix));
  if (!propertyValue) {
    // Check if we should use the value set by another language.
    if (lang == "OBJC") {
      propertyValue = this->GetPropertyWithPairedLanguageSupport("C", suffix);
    } else if (lang == "OBJCXX" || lang == "CUDA" || lang == "HIP") {
      propertyValue =
        this->GetPropertyWithPairedLanguageSupport("CXX", suffix);
    }
  }
  return propertyValue;
}

cmValue cmGeneratorTarget::GetLanguageExtensions(std::string const& lang) const
{
  return this->GetPropertyWithPairedLanguageSupport(lang, "_EXTENSIONS");
}

bool cmGeneratorTarget::GetLanguageStandardRequired(
  std::string const& lang) const
{
  return cmIsOn(
    this->GetPropertyWithPairedLanguageSupport(lang, "_STANDARD_REQUIRED"));
}

void cmGeneratorTarget::GetModuleDefinitionSources(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindModuleDefinition);
}

void cmGeneratorTarget::GetHeaderSources(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindHeader);
}

void cmGeneratorTarget::GetCxxModuleSources(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindCxxModuleSource);
}

void cmGeneratorTarget::GetExtraSources(std::vector<cmSourceFile const*>& data,
                                        const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindExtra);
}

void cmGeneratorTarget::GetCustomCommands(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindCustomCommand);
}

void cmGeneratorTarget::GetExternalObjects(
  std::vector<cmSourceFile const*>& data, const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindExternalObject);
}

void cmGeneratorTarget::GetManifests(std::vector<cmSourceFile const*>& data,
                                     const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindManifest);
}

std::set<cmLinkItem> const& cmGeneratorTarget::GetUtilityItems() const
{
  if (!this->UtilityItemsDone) {
    this->UtilityItemsDone = true;
    std::set<BT<std::pair<std::string, bool>>> const& utilities =
      this->GetUtilities();
    for (BT<std::pair<std::string, bool>> const& i : utilities) {
      if (cmGeneratorTarget* gt =
            this->LocalGenerator->FindGeneratorTargetToUse(i.Value.first)) {
        this->UtilityItems.insert(cmLinkItem(gt, i.Value.second, i.Backtrace));
      } else {
        this->UtilityItems.insert(
          cmLinkItem(i.Value.first, i.Value.second, i.Backtrace));
      }
    }
  }
  return this->UtilityItems;
}

const std::string& cmGeneratorTarget::GetLocation(
  const std::string& config) const
{
  static std::string location;
  if (this->IsImported()) {
    location = this->Target->ImportedGetFullPath(
      config, cmStateEnums::RuntimeBinaryArtifact);
  } else {
    location = this->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
  }
  return location;
}

cm::optional<std::string> cmGeneratorTarget::MaybeGetLocation(
  std::string const& config) const
{
  cm::optional<std::string> location;
  if (cmGeneratorTarget::ImportInfo const* imp = this->GetImportInfo(config)) {
    if (!imp->Location.empty()) {
      location = imp->Location;
    }
  } else {
    location = this->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
  }
  return location;
}

std::vector<cmCustomCommand> const& cmGeneratorTarget::GetPreBuildCommands()
  const
{
  return this->Target->GetPreBuildCommands();
}

std::vector<cmCustomCommand> const& cmGeneratorTarget::GetPreLinkCommands()
  const
{
  return this->Target->GetPreLinkCommands();
}

std::vector<cmCustomCommand> const& cmGeneratorTarget::GetPostBuildCommands()
  const
{
  return this->Target->GetPostBuildCommands();
}

void cmGeneratorTarget::AppendCustomCommandSideEffects(
  std::set<cmGeneratorTarget const*>& sideEffects) const
{
  if (!this->GetPreBuildCommands().empty() ||
      !this->GetPreLinkCommands().empty() ||
      !this->GetPostBuildCommands().empty()) {
    sideEffects.insert(this);
  } else {
    for (auto const& source : this->GetAllConfigSources()) {
      if (source.Source->GetCustomCommand() != nullptr) {
        sideEffects.insert(this);
        break;
      }
    }
  }
}

void cmGeneratorTarget::AppendLanguageSideEffects(
  std::map<std::string, std::set<cmGeneratorTarget const*>>& sideEffects) const
{
  static const std::set<cm::string_view> LANGS_WITH_NO_SIDE_EFFECTS = {
    "C"_s, "CXX"_s, "OBJC"_s, "OBJCXX"_s, "ASM"_s, "CUDA"_s, "HIP"_s
  };

  for (auto const& lang : this->GetAllConfigCompileLanguages()) {
    if (!LANGS_WITH_NO_SIDE_EFFECTS.count(lang)) {
      sideEffects[lang].insert(this);
    }
  }
}

bool cmGeneratorTarget::IsInBuildSystem() const
{
  if (this->IsImported()) {
    return false;
  }
  switch (this->Target->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::OBJECT_LIBRARY:
    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
      return true;
    case cmStateEnums::INTERFACE_LIBRARY:
      // An INTERFACE library is in the build system if it has SOURCES
      // or C++ module filesets.
      if (!this->SourceEntries.empty() ||
          !this->Target->GetHeaderSetsEntries().empty() ||
          !this->Target->GetCxxModuleSetsEntries().empty()) {
        return true;
      }
      break;
    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
  }
  return false;
}

bool cmGeneratorTarget::IsNormal() const
{
  return this->Target->IsNormal();
}

bool cmGeneratorTarget::IsRuntimeBinary() const
{
  return this->Target->IsRuntimeBinary();
}

bool cmGeneratorTarget::IsSynthetic() const
{
  return this->Target->IsSynthetic();
}

bool cmGeneratorTarget::IsImported() const
{
  return this->Target->IsImported();
}

bool cmGeneratorTarget::IsImportedGloballyVisible() const
{
  return this->Target->IsImportedGloballyVisible();
}

bool cmGeneratorTarget::CanCompileSources() const
{
  return this->Target->CanCompileSources();
}

bool cmGeneratorTarget::HasKnownRuntimeArtifactLocation(
  std::string const& config) const
{
  if (!this->IsRuntimeBinary()) {
    return false;
  }
  if (!this->IsImported()) {
    return true;
  }
  ImportInfo const* info = this->GetImportInfo(config);
  return info && !info->Location.empty();
}

const std::string& cmGeneratorTarget::GetLocationForBuild() const
{
  static std::string location;
  if (this->IsImported()) {
    location = this->Target->ImportedGetFullPath(
      "", cmStateEnums::RuntimeBinaryArtifact);
    return location;
  }

  // Now handle the deprecated build-time configuration location.
  std::string const noConfig;
  location = this->GetDirectory(noConfig);
  cmValue cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if (cfgid && (*cfgid != ".")) {
    location += "/";
    location += *cfgid;
  }

  if (this->IsAppBundleOnApple()) {
    std::string macdir = this->BuildBundleDirectory("", "", FullLevel);
    if (!macdir.empty()) {
      location += "/";
      location += macdir;
    }
  }
  location += "/";
  location += this->GetFullName("", cmStateEnums::RuntimeBinaryArtifact);
  return location;
}

bool cmGeneratorTarget::IsSystemIncludeDirectory(
  const std::string& dir, const std::string& config,
  const std::string& language) const
{
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }

  std::string key = cmStrCat(config_upper, '/', language);
  auto iter = this->SystemIncludesCache.find(key);

  if (iter == this->SystemIncludesCache.end()) {
    cmGeneratorExpressionDAGChecker dagChecker(
      this, "SYSTEM_INCLUDE_DIRECTORIES", nullptr, nullptr);

    bool excludeImported = this->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED");

    cmList result;
    for (std::string const& it : this->Target->GetSystemIncludeDirectories()) {
      result.append(cmGeneratorExpression::Evaluate(it, this->LocalGenerator,
                                                    config, this, &dagChecker,
                                                    nullptr, language));
    }

    std::vector<cmGeneratorTarget const*> const& deps =
      this->GetLinkImplementationClosure(config);
    for (cmGeneratorTarget const* dep : deps) {
      handleSystemIncludesDep(this->LocalGenerator, dep, config, this,
                              &dagChecker, result, excludeImported, language);
    }

    cmLinkImplementation const* impl =
      this->GetLinkImplementation(config, LinkInterfaceFor::Usage);
    if (impl != nullptr) {
      auto runtimeEntries = impl->LanguageRuntimeLibraries.find(language);
      if (runtimeEntries != impl->LanguageRuntimeLibraries.end()) {
        for (auto const& lib : runtimeEntries->second) {
          if (lib.Target) {
            handleSystemIncludesDep(this->LocalGenerator, lib.Target, config,
                                    this, &dagChecker, result, excludeImported,
                                    language);
          }
        }
      }
    }

    std::for_each(result.begin(), result.end(),
                  cmSystemTools::ConvertToUnixSlashes);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    iter = this->SystemIncludesCache.emplace(key, result).first;
  }

  return std::binary_search(iter->second.begin(), iter->second.end(), dir);
}

bool cmGeneratorTarget::GetPropertyAsBool(const std::string& prop) const
{
  return this->Target->GetPropertyAsBool(prop);
}

bool cmGeneratorTarget::MaybeHaveInterfaceProperty(
  std::string const& prop, cmGeneratorExpressionContext* context,
  LinkInterfaceFor interfaceFor) const
{
  std::string const key = prop + '@' + context->Config;
  auto i = this->MaybeInterfacePropertyExists.find(key);
  if (i == this->MaybeInterfacePropertyExists.end()) {
    // Insert an entry now in case there is a cycle.
    i = this->MaybeInterfacePropertyExists.emplace(key, false).first;
    bool& maybeInterfaceProp = i->second;

    // If this target itself has a non-empty property value, we are done.
    maybeInterfaceProp = cmNonempty(this->GetProperty(prop));

    // Otherwise, recurse to interface dependencies.
    if (!maybeInterfaceProp) {
      cmGeneratorTarget const* headTarget =
        context->HeadTarget ? context->HeadTarget : this;
      if (cmLinkInterfaceLibraries const* iface =
            this->GetLinkInterfaceLibraries(context->Config, headTarget,
                                            interfaceFor)) {
        if (iface->HadHeadSensitiveCondition) {
          // With a different head target we may get to a library with
          // this interface property.
          maybeInterfaceProp = true;
        } else {
          // The transitive interface libraries do not depend on the
          // head target, so we can follow them.
          for (cmLinkItem const& lib : iface->Libraries) {
            if (lib.Target &&
                lib.Target->MaybeHaveInterfaceProperty(prop, context,
                                                       interfaceFor)) {
              maybeInterfaceProp = true;
              break;
            }
          }
        }
      }
    }
  }
  return i->second;
}

std::string cmGeneratorTarget::EvaluateInterfaceProperty(
  std::string const& prop, cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagCheckerParent,
  LinkInterfaceFor interfaceFor) const
{
  std::string result;

  // If the property does not appear transitively at all, we are done.
  if (!this->MaybeHaveInterfaceProperty(prop, context, interfaceFor)) {
    return result;
  }

  // Evaluate $<TARGET_PROPERTY:this,prop> as if it were compiled.  This is
  // a subset of TargetPropertyNode::Evaluate without stringify/parse steps
  // but sufficient for transitive interface properties.
  cmGeneratorExpressionDAGChecker dagChecker(context->Backtrace, this, prop,
                                             nullptr, dagCheckerParent);
  switch (dagChecker.Check()) {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dagChecker.ReportError(
        context, "$<TARGET_PROPERTY:" + this->GetName() + "," + prop + ">");
      return result;
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return result;
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  cmGeneratorTarget const* headTarget =
    context->HeadTarget ? context->HeadTarget : this;

  if (cmValue p = this->GetProperty(prop)) {
    result = cmGeneratorExpressionNode::EvaluateDependentExpression(
      *p, context->LG, context, headTarget, &dagChecker, this);
  }

  if (cmLinkInterfaceLibraries const* iface = this->GetLinkInterfaceLibraries(
        context->Config, headTarget, interfaceFor)) {
    context->HadContextSensitiveCondition =
      context->HadContextSensitiveCondition ||
      iface->HadContextSensitiveCondition;
    for (cmLinkItem const& lib : iface->Libraries) {
      // Broken code can have a target in its own link interface.
      // Don't follow such link interface entries so as not to create a
      // self-referencing loop.
      if (lib.Target && lib.Target != this) {
        // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in the
        // above property and hand-evaluate it as if it were compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cmGeneratorExpressionContext libContext(
          context->LG, context->Config, context->Quiet, headTarget, this,
          context->EvaluateForBuildsystem, context->Backtrace,
          context->Language);
        std::string libResult = cmGeneratorExpression::StripEmptyListElements(
          lib.Target->EvaluateInterfaceProperty(prop, &libContext, &dagChecker,
                                                interfaceFor));
        if (!libResult.empty()) {
          if (result.empty()) {
            result = std::move(libResult);
          } else {
            result.reserve(result.size() + 1 + libResult.size());
            result += ";";
            result += libResult;
          }
        }
        context->HadContextSensitiveCondition =
          context->HadContextSensitiveCondition ||
          libContext.HadContextSensitiveCondition;
        context->HadHeadSensitiveCondition =
          context->HadHeadSensitiveCondition ||
          libContext.HadHeadSensitiveCondition;
      }
    }
  }

  return result;
}

namespace {

enum class IncludeDirectoryFallBack
{
  BINARY,
  OBJECT
};

std::string AddLangSpecificInterfaceIncludeDirectories(
  const cmGeneratorTarget* root, const cmGeneratorTarget* target,
  const std::string& lang, const std::string& config,
  const std::string& propertyName, IncludeDirectoryFallBack mode,
  cmGeneratorExpressionDAGChecker* context)
{
  cmGeneratorExpressionDAGChecker dag{ target->GetBacktrace(), target,
                                       propertyName, nullptr, context };
  switch (dag.Check()) {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dag.ReportError(
        nullptr, "$<TARGET_PROPERTY:" + target->GetName() + ",propertyName");
      CM_FALLTHROUGH;
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return "";
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  std::string directories;
  if (const auto* link_interface = target->GetLinkInterfaceLibraries(
        config, root, LinkInterfaceFor::Usage)) {
    for (const cmLinkItem& library : link_interface->Libraries) {
      if (const cmGeneratorTarget* dependency = library.Target) {
        if (cm::contains(dependency->GetAllConfigCompileLanguages(), lang)) {
          auto* lg = dependency->GetLocalGenerator();
          std::string value = dependency->GetSafeProperty(propertyName);
          if (value.empty()) {
            if (mode == IncludeDirectoryFallBack::BINARY) {
              value = lg->GetCurrentBinaryDirectory();
            } else if (mode == IncludeDirectoryFallBack::OBJECT) {
              value = cmStrCat(lg->GetCurrentBinaryDirectory(), '/',
                               lg->GetTargetDirectory(dependency));
            }
          }

          if (!directories.empty()) {
            directories += ";";
          }
          directories += value;
        }
      }
    }
  }
  return directories;
}

void AddLangSpecificImplicitIncludeDirectories(
  const cmGeneratorTarget* target, const std::string& lang,
  const std::string& config, const std::string& propertyName,
  IncludeDirectoryFallBack mode, EvaluatedTargetPropertyEntries& entries)
{
  if (const auto* libraries = target->GetLinkImplementationLibraries(
        config, LinkInterfaceFor::Usage)) {
    cmGeneratorExpressionDAGChecker dag{ target->GetBacktrace(), target,
                                         propertyName, nullptr, nullptr };

    for (const cmLinkImplItem& library : libraries->Libraries) {
      if (const cmGeneratorTarget* dependency = library.Target) {
        if (!dependency->IsInBuildSystem()) {
          continue;
        }
        if (cm::contains(dependency->GetAllConfigCompileLanguages(), lang)) {
          auto* lg = dependency->GetLocalGenerator();
          EvaluatedTargetPropertyEntry entry{ library, library.Backtrace };

          if (cmValue val = dependency->GetProperty(propertyName)) {
            entry.Values.emplace_back(*val);
          } else {
            if (mode == IncludeDirectoryFallBack::BINARY) {
              entry.Values.emplace_back(lg->GetCurrentBinaryDirectory());
            } else if (mode == IncludeDirectoryFallBack::OBJECT) {
              entry.Values.emplace_back(
                dependency->GetObjectDirectory(config));
            }
          }

          cmExpandList(
            AddLangSpecificInterfaceIncludeDirectories(
              target, dependency, lang, config, propertyName, mode, &dag),
            entry.Values);
          entries.Entries.emplace_back(std::move(entry));
        }
      }
    }
  }
}

void AddObjectEntries(cmGeneratorTarget const* headTarget,
                      std::string const& config,
                      cmGeneratorExpressionDAGChecker* dagChecker,
                      EvaluatedTargetPropertyEntries& entries)
{
  if (cmLinkImplementationLibraries const* impl =
        headTarget->GetLinkImplementationLibraries(config,
                                                   LinkInterfaceFor::Usage)) {
    entries.HadContextSensitiveCondition = impl->HadContextSensitiveCondition;
    for (cmLinkImplItem const& lib : impl->Libraries) {
      if (lib.Target &&
          lib.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        std::string uniqueName =
          headTarget->GetGlobalGenerator()->IndexGeneratorTargetUniquely(
            lib.Target);
        std::string genex = "$<TARGET_OBJECTS:" + std::move(uniqueName) + ">";
        cmGeneratorExpression ge(*headTarget->Makefile->GetCMakeInstance(),
                                 lib.Backtrace);
        std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(genex);
        cge->SetEvaluateForBuildsystem(true);

        EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
        cmExpandList(cge->Evaluate(headTarget->GetLocalGenerator(), config,
                                   headTarget, dagChecker),
                     ee.Values);
        if (cge->GetHadContextSensitiveCondition()) {
          ee.ContextDependent = true;
        }
        entries.Entries.emplace_back(std::move(ee));
      }
    }
  }
}

void addFileSetEntry(cmGeneratorTarget const* headTarget,
                     std::string const& config,
                     cmGeneratorExpressionDAGChecker* dagChecker,
                     cmFileSet const* fileSet,
                     EvaluatedTargetPropertyEntries& entries)
{
  auto dirCges = fileSet->CompileDirectoryEntries();
  auto dirs = fileSet->EvaluateDirectoryEntries(
    dirCges, headTarget->GetLocalGenerator(), config, headTarget, dagChecker);
  bool contextSensitiveDirs = false;
  for (auto const& dirCge : dirCges) {
    if (dirCge->GetHadContextSensitiveCondition()) {
      contextSensitiveDirs = true;
      break;
    }
  }
  cmake* cm = headTarget->GetLocalGenerator()->GetCMakeInstance();
  for (auto& entryCge : fileSet->CompileFileEntries()) {
    TargetPropertyEntryFileSet tpe(dirs, contextSensitiveDirs,
                                   std::move(entryCge), fileSet);
    entries.Entries.emplace_back(
      EvaluateTargetPropertyEntry(headTarget, config, "", dagChecker, tpe));
    EvaluatedTargetPropertyEntry const& entry = entries.Entries.back();
    for (auto const& file : entry.Values) {
      auto* sf = headTarget->Makefile->GetOrCreateSource(file);
      if (fileSet->GetType() == "HEADERS"_s) {
        sf->SetProperty("HEADER_FILE_ONLY", "TRUE");
      }

#ifndef CMAKE_BOOTSTRAP
      std::string e;
      std::string w;
      auto path = sf->ResolveFullPath(&e, &w);
      if (!w.empty()) {
        cm->IssueMessage(MessageType::AUTHOR_WARNING, w, entry.Backtrace);
      }
      if (path.empty()) {
        if (!e.empty()) {
          cm->IssueMessage(MessageType::FATAL_ERROR, e, entry.Backtrace);
        }
        return;
      }
      bool found = false;
      for (auto const& sg : headTarget->Makefile->GetSourceGroups()) {
        if (sg.MatchChildrenFiles(path)) {
          found = true;
          break;
        }
      }
      if (!found) {
        if (fileSet->GetType() == "HEADERS"_s) {
          headTarget->Makefile->GetOrCreateSourceGroup("Header Files")
            ->AddGroupFile(path);
        }
      }
#endif
    }
  }
}

void AddFileSetEntries(cmGeneratorTarget const* headTarget,
                       std::string const& config,
                       cmGeneratorExpressionDAGChecker* dagChecker,
                       EvaluatedTargetPropertyEntries& entries)
{
  for (auto const& entry : headTarget->Target->GetHeaderSetsEntries()) {
    for (auto const& name : cmList{ entry.Value }) {
      auto const* headerSet = headTarget->Target->GetFileSet(name);
      addFileSetEntry(headTarget, config, dagChecker, headerSet, entries);
    }
  }
  for (auto const& entry : headTarget->Target->GetCxxModuleSetsEntries()) {
    for (auto const& name : cmList{ entry.Value }) {
      auto const* cxxModuleSet = headTarget->Target->GetFileSet(name);
      addFileSetEntry(headTarget, config, dagChecker, cxxModuleSet, entries);
    }
  }
}

bool processSources(cmGeneratorTarget const* tgt,
                    EvaluatedTargetPropertyEntries& entries,
                    std::vector<BT<std::string>>& srcs,
                    std::unordered_set<std::string>& uniqueSrcs,
                    bool debugSources)
{
  cmMakefile* mf = tgt->Target->GetMakefile();

  bool contextDependent = entries.HadContextSensitiveCondition;

  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    if (entry.ContextDependent) {
      contextDependent = true;
    }

    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();

    for (std::string& src : entry.Values) {
      cmSourceFile* sf = mf->GetOrCreateSource(src);
      std::string e;
      std::string w;
      std::string fullPath = sf->ResolveFullPath(&e, &w);
      cmake* cm = tgt->GetLocalGenerator()->GetCMakeInstance();
      if (!w.empty()) {
        cm->IssueMessage(MessageType::AUTHOR_WARNING, w, entry.Backtrace);
      }
      if (fullPath.empty()) {
        if (!e.empty()) {
          cm->IssueMessage(MessageType::FATAL_ERROR, e, entry.Backtrace);
        }
        return contextDependent;
      }

      if (!targetName.empty() && !cmSystemTools::FileIsFullPath(src)) {
        std::ostringstream err;
        if (!targetName.empty()) {
          err << "Target \"" << targetName
              << "\" contains relative path in its INTERFACE_SOURCES:\n  \""
              << src << "\"";
        } else {
          err << "Found relative path while evaluating sources of \""
              << tgt->GetName() << "\":\n  \"" << src << "\"\n";
        }
        tgt->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR,
                                               err.str());
        return contextDependent;
      }
      src = fullPath;
    }
    std::string usedSources;
    for (std::string const& src : entry.Values) {
      if (uniqueSrcs.insert(src).second) {
        srcs.emplace_back(src, entry.Backtrace);
        if (debugSources) {
          usedSources += " * " + src + "\n";
        }
      }
    }
    if (!usedSources.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used sources for target ") + tgt->GetName() + ":\n" +
          usedSources,
        entry.Backtrace);
    }
  }
  return contextDependent;
}
}

std::vector<BT<std::string>> cmGeneratorTarget::GetSourceFilePaths(
  std::string const& config) const
{
  std::vector<BT<std::string>> files;

  if (!this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    // At configure-time, this method can be called as part of getting the
    // LOCATION property or to export() a file to be include()d.  However
    // there is no cmGeneratorTarget at configure-time, so search the SOURCES
    // for TARGET_OBJECTS instead for backwards compatibility with OLD
    // behavior of CMP0024 and CMP0026 only.

    cmBTStringRange sourceEntries = this->Target->GetSourceEntries();
    for (auto const& entry : sourceEntries) {
      cmList items{ entry.Value };
      for (auto const& item : items) {
        if (cmHasLiteralPrefix(item, "$<TARGET_OBJECTS:") &&
            item.back() == '>') {
          continue;
        }
        files.emplace_back(item);
      }
    }
    return files;
  }

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugSources =
    !this->DebugSourcesDone && cm::contains(debugProperties, "SOURCES");

  if (this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    this->DebugSourcesDone = true;
  }

  cmGeneratorExpressionDAGChecker dagChecker(this, "SOURCES", nullptr,
                                             nullptr);

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, std::string(), &dagChecker, this->SourceEntries);

  std::unordered_set<std::string> uniqueSrcs;
  bool contextDependentDirectSources =
    processSources(this, entries, files, uniqueSrcs, debugSources);

  // Collect INTERFACE_SOURCES of all direct link-dependencies.
  EvaluatedTargetPropertyEntries linkInterfaceSourcesEntries;
  AddInterfaceEntries(this, config, "INTERFACE_SOURCES", std::string(),
                      &dagChecker, linkInterfaceSourcesEntries,
                      IncludeRuntimeInterface::No, LinkInterfaceFor::Usage);
  bool contextDependentInterfaceSources = processSources(
    this, linkInterfaceSourcesEntries, files, uniqueSrcs, debugSources);

  // Collect TARGET_OBJECTS of direct object link-dependencies.
  bool contextDependentObjects = false;
  if (this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    EvaluatedTargetPropertyEntries linkObjectsEntries;
    AddObjectEntries(this, config, &dagChecker, linkObjectsEntries);
    contextDependentObjects = processSources(this, linkObjectsEntries, files,
                                             uniqueSrcs, debugSources);
    // Note that for imported targets or multi-config generators supporting
    // cross-config builds the paths to the object files must be per-config,
    // so contextDependentObjects will be true here even if object libraries
    // are specified without per-config generator expressions.
  }

  // Collect this target's file sets.
  EvaluatedTargetPropertyEntries fileSetEntries;
  AddFileSetEntries(this, config, &dagChecker, fileSetEntries);
  bool contextDependentFileSets =
    processSources(this, fileSetEntries, files, uniqueSrcs, debugSources);

  // Determine if sources are context-dependent or not.
  if (!contextDependentDirectSources && !contextDependentInterfaceSources &&
      !contextDependentObjects && !contextDependentFileSets) {
    this->SourcesAreContextDependent = Tribool::False;
  } else {
    this->SourcesAreContextDependent = Tribool::True;
  }

  return files;
}

void cmGeneratorTarget::GetSourceFiles(std::vector<cmSourceFile*>& files,
                                       const std::string& config) const
{
  std::vector<BT<cmSourceFile*>> tmp = this->GetSourceFiles(config);
  files.reserve(tmp.size());
  for (BT<cmSourceFile*>& v : tmp) {
    files.push_back(v.Value);
  }
}

std::vector<BT<cmSourceFile*>> cmGeneratorTarget::GetSourceFiles(
  std::string const& config) const
{
  std::vector<BT<cmSourceFile*>> files;
  if (!this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    // Since we are still configuring not all sources may exist yet,
    // so we need to avoid full source classification because that
    // requires the absolute paths to all sources to be determined.
    // Since this is only for compatibility with old policies that
    // projects should not depend on anymore, just compute the files
    // without memoizing them.
    std::vector<BT<std::string>> srcs = this->GetSourceFilePaths(config);
    std::set<cmSourceFile*> emitted;
    for (BT<std::string> const& s : srcs) {
      cmSourceFile* sf = this->Makefile->GetOrCreateSource(s.Value);
      if (emitted.insert(sf).second) {
        files.emplace_back(sf, s.Backtrace);
      }
    }
    return files;
  }

  KindedSources const& kinded = this->GetKindedSources(config);
  files.reserve(kinded.Sources.size());
  for (SourceAndKind const& si : kinded.Sources) {
    files.push_back(si.Source);
  }
  return files;
}

void cmGeneratorTarget::GetSourceFilesWithoutObjectLibraries(
  std::vector<cmSourceFile*>& files, const std::string& config) const
{
  std::vector<BT<cmSourceFile*>> tmp =
    this->GetSourceFilesWithoutObjectLibraries(config);
  files.reserve(tmp.size());
  for (BT<cmSourceFile*>& v : tmp) {
    files.push_back(v.Value);
  }
}

std::vector<BT<cmSourceFile*>>
cmGeneratorTarget::GetSourceFilesWithoutObjectLibraries(
  std::string const& config) const
{
  std::vector<BT<cmSourceFile*>> files;
  KindedSources const& kinded = this->GetKindedSources(config);
  files.reserve(kinded.Sources.size());
  for (SourceAndKind const& si : kinded.Sources) {
    if (si.Source.Value->GetObjectLibrary().empty()) {
      files.push_back(si.Source);
    }
  }
  return files;
}

cmGeneratorTarget::KindedSources const& cmGeneratorTarget::GetKindedSources(
  std::string const& config) const
{
  // If we already processed one configuration and found no dependency
  // on configuration then always use the one result.
  if (this->SourcesAreContextDependent == Tribool::False) {
    return this->KindedSourcesMap.begin()->second;
  }

  // Lookup any existing link implementation for this configuration.
  std::string const key = cmSystemTools::UpperCase(config);
  auto it = this->KindedSourcesMap.find(key);
  if (it != this->KindedSourcesMap.end()) {
    if (!it->second.Initialized) {
      std::ostringstream e;
      e << "The SOURCES of \"" << this->GetName()
        << "\" use a generator expression that depends on the "
           "SOURCES themselves.";
      this->GlobalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR, e.str(), this->GetBacktrace());
      static KindedSources empty;
      return empty;
    }
    return it->second;
  }

  // Add an entry to the map for this configuration.
  KindedSources& files = this->KindedSourcesMap[key];
  this->ComputeKindedSources(files, config);
  files.Initialized = true;
  return files;
}

void cmGeneratorTarget::ComputeKindedSources(KindedSources& files,
                                             std::string const& config) const
{
  // Get the source file paths by string.
  std::vector<BT<std::string>> srcs = this->GetSourceFilePaths(config);

  cmsys::RegularExpression header_regex(CM_HEADER_REGEX);
  std::vector<cmSourceFile*> badObjLib;

  std::set<cmSourceFile*> emitted;
  for (BT<std::string> const& s : srcs) {
    // Create each source at most once.
    cmSourceFile* sf = this->Makefile->GetOrCreateSource(s.Value);
    if (!emitted.insert(sf).second) {
      continue;
    }

    // Compute the kind (classification) of this source file.
    SourceKind kind;
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    cmFileSet const* fs = this->GetFileSetForSource(config, sf);
    if (sf->GetCustomCommand()) {
      kind = SourceKindCustomCommand;
    } else if (!this->Target->IsNormal() && !this->Target->IsImported() &&
               fs && (fs->GetType() == "CXX_MODULES"_s)) {
      kind = SourceKindCxxModuleSource;
    } else if (this->Target->GetType() == cmStateEnums::UTILITY ||
               this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY
               // XXX(clang-tidy): https://bugs.llvm.org/show_bug.cgi?id=44165
               // NOLINTNEXTLINE(bugprone-branch-clone)
    ) {
      kind = SourceKindExtra;
    } else if (this->IsSourceFilePartOfUnityBatch(sf->ResolveFullPath())) {
      kind = SourceKindUnityBatched;
      // XXX(clang-tidy): https://bugs.llvm.org/show_bug.cgi?id=44165
      // NOLINTNEXTLINE(bugprone-branch-clone)
    } else if (sf->GetPropertyAsBool("HEADER_FILE_ONLY")) {
      kind = SourceKindHeader;
    } else if (sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
      kind = SourceKindExternalObject;
    } else if (!sf->GetOrDetermineLanguage().empty()) {
      kind = SourceKindObjectSource;
    } else if (ext == "def") {
      kind = SourceKindModuleDefinition;
      if (this->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        badObjLib.push_back(sf);
      }
    } else if (ext == "idl") {
      kind = SourceKindIDL;
      if (this->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        badObjLib.push_back(sf);
      }
    } else if (ext == "resx") {
      kind = SourceKindResx;
    } else if (ext == "appxmanifest") {
      kind = SourceKindAppManifest;
    } else if (ext == "manifest") {
      if (sf->GetPropertyAsBool("VS_DEPLOYMENT_CONTENT")) {
        kind = SourceKindExtra;
      } else {
        kind = SourceKindManifest;
      }
    } else if (ext == "pfx") {
      kind = SourceKindCertificate;
    } else if (ext == "xaml") {
      kind = SourceKindXaml;
    } else if (header_regex.find(sf->ResolveFullPath())) {
      kind = SourceKindHeader;
    } else {
      kind = SourceKindExtra;
    }

    // Save this classified source file in the result vector.
    files.Sources.push_back({ BT<cmSourceFile*>(sf, s.Backtrace), kind });
  }

  if (!badObjLib.empty()) {
    std::ostringstream e;
    e << "OBJECT library \"" << this->GetName() << "\" contains:\n";
    for (cmSourceFile* i : badObjLib) {
      e << "  " << i->GetLocation().GetName() << "\n";
    }
    e << "but may contain only sources that compile, header files, and "
         "other files that would not affect linking of a normal library.";
    this->GlobalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR, e.str(), this->GetBacktrace());
  }
}

std::vector<cmGeneratorTarget::AllConfigSource> const&
cmGeneratorTarget::GetAllConfigSources() const
{
  if (this->AllConfigSources.empty()) {
    this->ComputeAllConfigSources();
  }
  return this->AllConfigSources;
}

void cmGeneratorTarget::ComputeAllConfigSources() const
{
  std::vector<std::string> configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  std::map<cmSourceFile const*, size_t> index;

  for (size_t ci = 0; ci < configs.size(); ++ci) {
    KindedSources const& sources = this->GetKindedSources(configs[ci]);
    for (SourceAndKind const& src : sources.Sources) {
      auto mi = index.find(src.Source.Value);
      if (mi == index.end()) {
        AllConfigSource acs;
        acs.Source = src.Source.Value;
        acs.Kind = src.Kind;
        this->AllConfigSources.push_back(std::move(acs));
        std::map<cmSourceFile const*, size_t>::value_type entry(
          src.Source.Value, this->AllConfigSources.size() - 1);
        mi = index.insert(entry).first;
      }
      this->AllConfigSources[mi->second].Configs.push_back(ci);
    }
  }
}

std::vector<cmGeneratorTarget::AllConfigSource>
cmGeneratorTarget::GetAllConfigSources(SourceKind kind) const
{
  std::vector<AllConfigSource> result;
  for (AllConfigSource const& source : this->GetAllConfigSources()) {
    if (source.Kind == kind) {
      result.push_back(source);
    }
  }
  return result;
}

std::set<std::string> cmGeneratorTarget::GetAllConfigCompileLanguages() const
{
  std::set<std::string> languages;
  std::vector<AllConfigSource> const& sources = this->GetAllConfigSources();
  for (AllConfigSource const& si : sources) {
    std::string const& lang = si.Source->GetOrDetermineLanguage();
    if (!lang.empty()) {
      languages.emplace(lang);
    }
  }
  return languages;
}

std::string cmGeneratorTarget::GetCompilePDBName(
  const std::string& config) const
{
  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config);
  std::string configProp = cmStrCat("COMPILE_PDB_NAME_", configUpper);
  cmValue config_name = this->GetProperty(configProp);
  if (cmNonempty(config_name)) {
    NameComponents const& components = GetFullNameInternalComponents(
      config, cmStateEnums::RuntimeBinaryArtifact);
    return components.prefix + *config_name + ".pdb";
  }

  cmValue name = this->GetProperty("COMPILE_PDB_NAME");
  if (cmNonempty(name)) {
    NameComponents const& components = GetFullNameInternalComponents(
      config, cmStateEnums::RuntimeBinaryArtifact);
    return components.prefix + *name + ".pdb";
  }

  return "";
}

std::string cmGeneratorTarget::GetCompilePDBPath(
  const std::string& config) const
{
  std::string dir = this->GetCompilePDBDirectory(config);
  std::string name = this->GetCompilePDBName(config);
  if (dir.empty() && !name.empty() && this->HaveWellDefinedOutputFiles()) {
    dir = this->GetPDBDirectory(config);
  }
  if (!dir.empty()) {
    dir += "/";
  }
  return dir + name;
}

bool cmGeneratorTarget::HasSOName(const std::string& config) const
{
  // soname is supported only for shared libraries and modules,
  // and then only when the platform supports an soname flag.
  return ((this->GetType() == cmStateEnums::SHARED_LIBRARY) &&
          !this->GetPropertyAsBool("NO_SONAME") &&
          this->Makefile->GetSONameFlag(this->GetLinkerLanguage(config)));
}

bool cmGeneratorTarget::NeedRelinkBeforeInstall(
  const std::string& config) const
{
  // Only executables and shared libraries can have an rpath and may
  // need relinking.
  if (this->GetType() != cmStateEnums::EXECUTABLE &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY) {
    return false;
  }

  // If there is no install location this target will not be installed
  // and therefore does not need relinking.
  if (!this->Target->GetHaveInstallRule()) {
    return false;
  }

  // If skipping all rpaths completely then no relinking is needed.
  if (this->Makefile->IsOn("CMAKE_SKIP_RPATH")) {
    return false;
  }

  // If building with the install-tree rpath no relinking is needed.
  if (this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH")) {
    return false;
  }

  // If chrpath is going to be used no relinking is needed.
  if (this->IsChrpathUsed(config)) {
    return false;
  }

  // Check for rpath support on this platform.
  std::string ll = this->GetLinkerLanguage(config);
  if (!ll.empty()) {
    std::string flagVar =
      cmStrCat("CMAKE_SHARED_LIBRARY_RUNTIME_", ll, "_FLAG");
    if (!this->Makefile->IsSet(flagVar)) {
      // There is no rpath support on this platform so nothing needs
      // relinking.
      return false;
    }
  } else {
    // No linker language is known.  This error will be reported by
    // other code.
    return false;
  }

  // If either a build or install tree rpath is set then the rpath
  // will likely change between the build tree and install tree and
  // this target must be relinked.
  bool have_rpath =
    this->HaveBuildTreeRPATH(config) || this->HaveInstallTreeRPATH(config);
  bool is_ninja = this->LocalGenerator->GetGlobalGenerator()->IsNinja();

  if (have_rpath && is_ninja) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      "The install of the " << this->GetName() << " target requires changing "
      "an RPATH from the build tree, but this is not supported with the Ninja "
      "generator unless on an ELF-based or XCOFF-based platform.  "
      "The CMAKE_BUILD_WITH_INSTALL_RPATH variable may be set to avoid this "
      "relinking step."
      ;
    /* clang-format on */

    cmake* cm = this->LocalGenerator->GetCMakeInstance();
    cm->IssueMessage(MessageType::FATAL_ERROR, w.str(), this->GetBacktrace());
  }

  return have_rpath;
}

bool cmGeneratorTarget::IsChrpathUsed(const std::string& config) const
{
  // Only certain target types have an rpath.
  if (!(this->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GetType() == cmStateEnums::MODULE_LIBRARY ||
        this->GetType() == cmStateEnums::EXECUTABLE)) {
    return false;
  }

  // If the target will not be installed we do not need to change its
  // rpath.
  if (!this->Target->GetHaveInstallRule()) {
    return false;
  }

  // Skip chrpath if skipping rpath altogether.
  if (this->Makefile->IsOn("CMAKE_SKIP_RPATH")) {
    return false;
  }

  // Skip chrpath if it does not need to be changed at install time.
  if (this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH")) {
    return false;
  }

  // Allow the user to disable builtin chrpath explicitly.
  if (this->Makefile->IsOn("CMAKE_NO_BUILTIN_CHRPATH")) {
    return false;
  }

  if (this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    return true;
  }

  // Enable if the rpath flag uses a separator and the target uses
  // binaries we know how to edit.
  std::string ll = this->GetLinkerLanguage(config);
  if (!ll.empty()) {
    std::string sepVar =
      cmStrCat("CMAKE_SHARED_LIBRARY_RUNTIME_", ll, "_FLAG_SEP");
    cmValue sep = this->Makefile->GetDefinition(sepVar);
    if (cmNonempty(sep)) {
      // TODO: Add binary format check to ABI detection and get rid of
      // CMAKE_EXECUTABLE_FORMAT.
      if (cmValue fmt =
            this->Makefile->GetDefinition("CMAKE_EXECUTABLE_FORMAT")) {
        if (*fmt == "ELF") {
          return true;
        }
#if defined(CMake_USE_XCOFF_PARSER)
        if (*fmt == "XCOFF") {
          return true;
        }
#endif
      }
    }
  }
  return false;
}

bool cmGeneratorTarget::IsImportedSharedLibWithoutSOName(
  const std::string& config) const
{
  if (this->IsImported() && this->GetType() == cmStateEnums::SHARED_LIBRARY) {
    if (cmGeneratorTarget::ImportInfo const* info =
          this->GetImportInfo(config)) {
      return info->NoSOName;
    }
  }
  return false;
}

bool cmGeneratorTarget::HasMacOSXRpathInstallNameDir(
  const std::string& config) const
{
  TargetPtrToBoolMap& cache = this->MacOSXRpathInstallNameDirCache[config];
  const auto lookup = cache.find(this->Target);

  if (lookup != cache.cend()) {
    return lookup->second;
  }

  const bool result = this->DetermineHasMacOSXRpathInstallNameDir(config);
  cache[this->Target] = result;
  return result;
}

bool cmGeneratorTarget::DetermineHasMacOSXRpathInstallNameDir(
  const std::string& config) const
{
  bool install_name_is_rpath = false;
  bool macosx_rpath = false;

  if (!this->IsImported()) {
    if (this->GetType() != cmStateEnums::SHARED_LIBRARY) {
      return false;
    }
    cmValue install_name = this->GetProperty("INSTALL_NAME_DIR");
    bool use_install_name = this->MacOSXUseInstallNameDir();
    if (install_name && use_install_name && *install_name == "@rpath") {
      install_name_is_rpath = true;
    } else if (install_name && use_install_name) {
      return false;
    }
    if (!install_name_is_rpath) {
      macosx_rpath = this->MacOSXRpathInstallNameDirDefault();
    }
  } else {
    // Lookup the imported soname.
    if (cmGeneratorTarget::ImportInfo const* info =
          this->GetImportInfo(config)) {
      if (!info->NoSOName && !info->SOName.empty()) {
        if (cmHasLiteralPrefix(info->SOName, "@rpath/")) {
          install_name_is_rpath = true;
        }
      } else {
        std::string install_name;
        cmSystemTools::GuessLibraryInstallName(info->Location, install_name);
        if (install_name.find("@rpath") != std::string::npos) {
          install_name_is_rpath = true;
        }
      }
    }
  }

  if (!install_name_is_rpath && !macosx_rpath) {
    return false;
  }

  if (!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG")) {
    std::ostringstream w;
    w << "Attempting to use ";
    if (macosx_rpath) {
      w << "MACOSX_RPATH";
    } else {
      w << "@rpath";
    }
    w << " without CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG being set.";
    w << "  This could be because you are using a Mac OS X version";
    w << " less than 10.5 or because CMake's platform configuration is";
    w << " corrupt.";
    cmake* cm = this->LocalGenerator->GetCMakeInstance();
    cm->IssueMessage(MessageType::FATAL_ERROR, w.str(), this->GetBacktrace());
  }

  return true;
}

bool cmGeneratorTarget::MacOSXRpathInstallNameDirDefault() const
{
  // we can't do rpaths when unsupported
  if (!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG")) {
    return false;
  }

  cmValue macosx_rpath_str = this->GetProperty("MACOSX_RPATH");
  if (macosx_rpath_str) {
    return this->GetPropertyAsBool("MACOSX_RPATH");
  }

  cmPolicies::PolicyStatus cmp0042 = this->GetPolicyStatusCMP0042();

  if (cmp0042 == cmPolicies::WARN) {
    this->LocalGenerator->GetGlobalGenerator()->AddCMP0042WarnTarget(
      this->GetName());
  }

  return cmp0042 == cmPolicies::NEW;
}

bool cmGeneratorTarget::MacOSXUseInstallNameDir() const
{
  cmValue build_with_install_name =
    this->GetProperty("BUILD_WITH_INSTALL_NAME_DIR");
  if (build_with_install_name) {
    return cmIsOn(*build_with_install_name);
  }

  cmPolicies::PolicyStatus cmp0068 = this->GetPolicyStatusCMP0068();
  if (cmp0068 == cmPolicies::NEW) {
    return false;
  }

  bool use_install_name = this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH");

  if (use_install_name && cmp0068 == cmPolicies::WARN) {
    this->LocalGenerator->GetGlobalGenerator()->AddCMP0068WarnTarget(
      this->GetName());
  }

  return use_install_name;
}

bool cmGeneratorTarget::CanGenerateInstallNameDir(
  InstallNameType name_type) const
{
  cmPolicies::PolicyStatus cmp0068 = this->GetPolicyStatusCMP0068();

  if (cmp0068 == cmPolicies::NEW) {
    return true;
  }

  bool skip = this->Makefile->IsOn("CMAKE_SKIP_RPATH");
  if (name_type == INSTALL_NAME_FOR_INSTALL) {
    skip |= this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
  } else {
    skip |= this->GetPropertyAsBool("SKIP_BUILD_RPATH");
  }

  if (skip && cmp0068 == cmPolicies::WARN) {
    this->LocalGenerator->GetGlobalGenerator()->AddCMP0068WarnTarget(
      this->GetName());
  }

  return !skip;
}

std::string cmGeneratorTarget::GetSOName(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    // Lookup the imported soname.
    if (cmGeneratorTarget::ImportInfo const* info =
          this->GetImportInfo(config)) {
      if (info->NoSOName) {
        // The imported library has no builtin soname so the name
        // searched at runtime will be just the filename.
        return cmSystemTools::GetFilenameName(info->Location);
      }
      // Use the soname given if any.
      if (this->IsFrameworkOnApple()) {
        auto fwDescriptor = this->GetGlobalGenerator()->SplitFrameworkPath(
          info->SOName, cmGlobalGenerator::FrameworkFormat::Strict);
        if (fwDescriptor) {
          return fwDescriptor->GetVersionedName();
        }
      }
      if (cmHasLiteralPrefix(info->SOName, "@rpath/")) {
        return info->SOName.substr(cmStrLen("@rpath/"));
      }
      return info->SOName;
    }
    return "";
  }
  // Compute the soname that will be built.
  return artifact == cmStateEnums::RuntimeBinaryArtifact
    ? this->GetLibraryNames(config).SharedObject
    : this->GetLibraryNames(config).ImportLibrary;
}

namespace {
bool shouldAddFullLevel(cmGeneratorTarget::BundleDirectoryLevel level)
{
  return level == cmGeneratorTarget::FullLevel;
}

bool shouldAddContentLevel(cmGeneratorTarget::BundleDirectoryLevel level)
{
  return level == cmGeneratorTarget::ContentLevel || shouldAddFullLevel(level);
}
}

std::string cmGeneratorTarget::GetAppBundleDirectory(
  const std::string& config, BundleDirectoryLevel level) const
{
  std::string fpath = cmStrCat(
    this->GetFullName(config, cmStateEnums::RuntimeBinaryArtifact), '.');
  cmValue ext = this->GetProperty("BUNDLE_EXTENSION");
  fpath += (ext ? *ext : "app");
  if (shouldAddContentLevel(level) &&
      !this->Makefile->PlatformIsAppleEmbedded()) {
    fpath += "/Contents";
    if (shouldAddFullLevel(level)) {
      fpath += "/MacOS";
    }
  }
  return fpath;
}

bool cmGeneratorTarget::IsBundleOnApple() const
{
  return this->IsFrameworkOnApple() || this->IsAppBundleOnApple() ||
    this->IsCFBundleOnApple();
}

bool cmGeneratorTarget::IsWin32Executable(const std::string& config) const
{
  return cmIsOn(cmGeneratorExpression::Evaluate(
    this->GetSafeProperty("WIN32_EXECUTABLE"), this->LocalGenerator, config));
}

std::string cmGeneratorTarget::GetCFBundleDirectory(
  const std::string& config, BundleDirectoryLevel level) const
{
  std::string fpath = cmStrCat(
    this->GetOutputName(config, cmStateEnums::RuntimeBinaryArtifact), '.');
  std::string ext;
  if (cmValue p = this->GetProperty("BUNDLE_EXTENSION")) {
    ext = *p;
  } else {
    if (this->IsXCTestOnApple()) {
      ext = "xctest";
    } else {
      ext = "bundle";
    }
  }
  fpath += ext;
  if (shouldAddContentLevel(level) &&
      !this->Makefile->PlatformIsAppleEmbedded()) {
    fpath += "/Contents";
    if (shouldAddFullLevel(level)) {
      fpath += "/MacOS";
    }
  }
  return fpath;
}

std::string cmGeneratorTarget::GetFrameworkDirectory(
  const std::string& config, BundleDirectoryLevel level) const
{
  std::string fpath = cmStrCat(
    this->GetOutputName(config, cmStateEnums::RuntimeBinaryArtifact), '.');
  cmValue ext = this->GetProperty("BUNDLE_EXTENSION");
  fpath += (ext ? *ext : "framework");
  if (shouldAddFullLevel(level) &&
      !this->Makefile->PlatformIsAppleEmbedded()) {
    fpath += "/Versions/";
    fpath += this->GetFrameworkVersion();
  }
  return fpath;
}

std::string cmGeneratorTarget::GetFullName(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    return this->GetFullNameImported(config, artifact);
  }
  return this->GetFullNameInternal(config, artifact);
}

std::string cmGeneratorTarget::GetInstallNameDirForBuildTree(
  const std::string& config) const
{
  if (this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {

    // If building directly for installation then the build tree install_name
    // is the same as the install tree.
    if (this->MacOSXUseInstallNameDir()) {
      std::string installPrefix =
        this->Makefile->GetSafeDefinition("CMAKE_INSTALL_PREFIX");
      return this->GetInstallNameDirForInstallTree(config, installPrefix);
    }

    // Use the build tree directory for the target.
    if (this->CanGenerateInstallNameDir(INSTALL_NAME_FOR_BUILD)) {
      std::string dir;
      if (this->MacOSXRpathInstallNameDirDefault()) {
        dir = "@rpath";
      } else {
        dir = this->GetDirectory(config);
      }
      dir += "/";
      return dir;
    }
  }
  return "";
}

std::string cmGeneratorTarget::GetInstallNameDirForInstallTree(
  const std::string& config, const std::string& installPrefix) const
{
  if (this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    std::string dir;
    cmValue install_name_dir = this->GetProperty("INSTALL_NAME_DIR");

    if (this->CanGenerateInstallNameDir(INSTALL_NAME_FOR_INSTALL)) {
      if (cmNonempty(install_name_dir)) {
        dir = *install_name_dir;
        cmGeneratorExpression::ReplaceInstallPrefix(dir, installPrefix);
        dir =
          cmGeneratorExpression::Evaluate(dir, this->LocalGenerator, config);
        if (!dir.empty()) {
          dir = cmStrCat(dir, '/');
        }
      }
    }
    if (!install_name_dir) {
      if (this->MacOSXRpathInstallNameDirDefault()) {
        dir = "@rpath/";
      }
    }
    return dir;
  }
  return "";
}

cmListFileBacktrace cmGeneratorTarget::GetBacktrace() const
{
  return this->Target->GetBacktrace();
}

const std::set<BT<std::pair<std::string, bool>>>&
cmGeneratorTarget::GetUtilities() const
{
  return this->Target->GetUtilities();
}

bool cmGeneratorTarget::HaveWellDefinedOutputFiles() const
{
  return this->GetType() == cmStateEnums::STATIC_LIBRARY ||
    this->GetType() == cmStateEnums::SHARED_LIBRARY ||
    this->GetType() == cmStateEnums::MODULE_LIBRARY ||
    this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
    this->GetType() == cmStateEnums::EXECUTABLE;
}

const std::string* cmGeneratorTarget::GetExportMacro() const
{
  // Define the symbol for targets that export symbols.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GetType() == cmStateEnums::MODULE_LIBRARY ||
      this->IsExecutableWithExports()) {
    if (cmValue custom_export_name = this->GetProperty("DEFINE_SYMBOL")) {
      this->ExportMacro = *custom_export_name;
    } else {
      std::string in = cmStrCat(this->GetName(), "_EXPORTS");
      this->ExportMacro = cmSystemTools::MakeCidentifier(in);
    }
    return &this->ExportMacro;
  }
  return nullptr;
}

class cmTargetCollectLinkLanguages
{
public:
  cmTargetCollectLinkLanguages(cmGeneratorTarget const* target,
                               std::string config,
                               std::unordered_set<std::string>& languages,
                               cmGeneratorTarget const* head, bool secondPass)
    : Config(std::move(config))
    , Languages(languages)
    , HeadTarget(head)
    , SecondPass(secondPass)
  {
    this->Visited.insert(target);
  }

  void Visit(cmLinkItem const& item)
  {
    if (!item.Target) {
      return;
    }
    if (!this->Visited.insert(item.Target).second) {
      return;
    }
    cmLinkInterface const* iface = item.Target->GetLinkInterface(
      this->Config, this->HeadTarget, this->SecondPass);
    if (!iface) {
      return;
    }
    if (iface->HadLinkLanguageSensitiveCondition) {
      this->HadLinkLanguageSensitiveCondition = true;
    }

    for (std::string const& language : iface->Languages) {
      this->Languages.insert(language);
    }

    for (cmLinkItem const& lib : iface->Libraries) {
      this->Visit(lib);
    }
  }

  bool GetHadLinkLanguageSensitiveCondition() const
  {
    return this->HadLinkLanguageSensitiveCondition;
  }

private:
  std::string Config;
  std::unordered_set<std::string>& Languages;
  cmGeneratorTarget const* HeadTarget;
  std::set<cmGeneratorTarget const*> Visited;
  bool SecondPass;
  bool HadLinkLanguageSensitiveCondition = false;
};

cmGeneratorTarget::LinkClosure const* cmGeneratorTarget::GetLinkClosure(
  const std::string& config) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    static LinkClosure const empty = { {}, {} };
    return &empty;
  }

  std::string key(cmSystemTools::UpperCase(config));
  auto i = this->LinkClosureMap.find(key);
  if (i == this->LinkClosureMap.end()) {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc);
    LinkClosureMapType::value_type entry(key, lc);
    i = this->LinkClosureMap.insert(entry).first;
  }
  return &i->second;
}

class cmTargetSelectLinker
{
  int Preference = 0;
  cmGeneratorTarget const* Target;
  cmGlobalGenerator* GG;
  std::set<std::string> Preferred;

public:
  cmTargetSelectLinker(cmGeneratorTarget const* target)
    : Target(target)
  {
    this->GG = this->Target->GetLocalGenerator()->GetGlobalGenerator();
  }
  void Consider(const std::string& lang)
  {
    int preference = this->GG->GetLinkerPreference(lang);
    if (preference > this->Preference) {
      this->Preference = preference;
      this->Preferred.clear();
    }
    if (preference == this->Preference) {
      this->Preferred.insert(lang);
    }
  }
  std::string Choose()
  {
    if (this->Preferred.empty()) {
      return "";
    }
    if (this->Preferred.size() > 1) {
      std::ostringstream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for (std::string const& li : this->Preferred) {
        e << "  " << li << "\n";
      }
      e << "Set the LINKER_LANGUAGE property for this target.";
      cmake* cm = this->Target->GetLocalGenerator()->GetCMakeInstance();
      cm->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                       this->Target->GetBacktrace());
    }
    return *this->Preferred.begin();
  }
};

bool cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc,
                                           bool secondPass) const
{
  // Get languages built in this target.
  std::unordered_set<std::string> languages;
  cmLinkImplementation const* impl =
    this->GetLinkImplementation(config, LinkInterfaceFor::Link, secondPass);
  assert(impl);
  languages.insert(impl->Languages.cbegin(), impl->Languages.cend());

  // Add interface languages from linked targets.
  // cmTargetCollectLinkLanguages cll(this, config, languages, this,
  // secondPass);
  cmTargetCollectLinkLanguages cll(this, config, languages, this, secondPass);
  for (cmLinkImplItem const& lib : impl->Libraries) {
    cll.Visit(lib);
  }

  // Store the transitive closure of languages.
  cm::append(lc.Languages, languages);

  // Choose the language whose linker should be used.
  if (secondPass || lc.LinkerLanguage.empty()) {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for (std::string const& l : impl->Languages) {
      tsl.Consider(l);
    }

    // Now consider languages that propagate from linked targets.
    for (std::string const& lang : languages) {
      std::string propagates =
        "CMAKE_" + lang + "_LINKER_PREFERENCE_PROPAGATES";
      if (this->Makefile->IsOn(propagates)) {
        tsl.Consider(lang);
      }
    }

    lc.LinkerLanguage = tsl.Choose();
  }

  return impl->HadLinkLanguageSensitiveCondition ||
    cll.GetHadLinkLanguageSensitiveCondition();
}

void cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc) const
{
  bool secondPass = false;

  {
    LinkClosure linkClosure;
    linkClosure.LinkerLanguage = this->LinkerLanguage;

    bool hasHardCodedLinkerLanguage = this->Target->GetProperty("HAS_CXX") ||
      !this->Target->GetSafeProperty("LINKER_LANGUAGE").empty();

    // Get languages built in this target.
    secondPass = this->ComputeLinkClosure(config, linkClosure, false) &&
      !hasHardCodedLinkerLanguage;
    this->LinkerLanguage = linkClosure.LinkerLanguage;
    if (!secondPass) {
      lc = std::move(linkClosure);
    }
  }

  if (secondPass) {
    LinkClosure linkClosure;

    this->ComputeLinkClosure(config, linkClosure, secondPass);
    lc = std::move(linkClosure);

    // linker language must not be changed between the two passes
    if (this->LinkerLanguage != lc.LinkerLanguage) {
      std::ostringstream e;
      e << "Evaluation of $<LINK_LANGUAGE:...> or $<LINK_LAND_AND_ID:...> "
           "changes\nthe linker language for target \""
        << this->GetName() << "\" (from '" << this->LinkerLanguage << "' to '"
        << lc.LinkerLanguage << "') which is invalid.";
      cmSystemTools::Error(e.str());
    }
  }
}

cmGeneratorTarget::NameComponents const&
cmGeneratorTarget::GetFullNameComponents(
  std::string const& config, cmStateEnums::ArtifactType artifact) const
{
  return this->GetFullNameInternalComponents(config, artifact);
}

std::string cmGeneratorTarget::BuildBundleDirectory(
  const std::string& base, const std::string& config,
  BundleDirectoryLevel level) const
{
  std::string fpath = base;
  if (this->IsAppBundleOnApple()) {
    fpath += this->GetAppBundleDirectory(config, level);
  }
  if (this->IsFrameworkOnApple()) {
    fpath += this->GetFrameworkDirectory(config, level);
  }
  if (this->IsCFBundleOnApple()) {
    fpath += this->GetCFBundleDirectory(config, level);
  }
  return fpath;
}

std::string cmGeneratorTarget::GetMacContentDirectory(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  // Start with the output directory for the target.
  std::string fpath = cmStrCat(this->GetDirectory(config, artifact), '/');
  BundleDirectoryLevel level = ContentLevel;
  if (this->IsFrameworkOnApple()) {
    // additional files with a framework go into the version specific
    // directory
    level = FullLevel;
  }
  fpath = this->BuildBundleDirectory(fpath, config, level);
  return fpath;
}

std::string cmGeneratorTarget::GetEffectiveFolderName() const
{
  std::string effectiveFolder;

  if (!this->GlobalGenerator->UseFolderProperty()) {
    return effectiveFolder;
  }

  cmValue targetFolder = this->GetProperty("FOLDER");
  if (targetFolder) {
    effectiveFolder += *targetFolder;
  }

  return effectiveFolder;
}

cmGeneratorTarget::CompileInfo const* cmGeneratorTarget::GetCompileInfo(
  const std::string& config) const
{
  // There is no compile information for imported targets.
  if (this->IsImported()) {
    return nullptr;
  }

  if (this->GetType() > cmStateEnums::OBJECT_LIBRARY) {
    std::string msg = cmStrCat("cmTarget::GetCompileInfo called for ",
                               this->GetName(), " which has type ",
                               cmState::GetTargetTypeName(this->GetType()));
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
    return nullptr;
  }

  // Lookup/compute/cache the compile information for this configuration.
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto i = this->CompileInfoMap.find(config_upper);
  if (i == this->CompileInfoMap.end()) {
    CompileInfo info;
    this->ComputePDBOutputDir("COMPILE_PDB", config, info.CompilePdbDir);
    CompileInfoMapType::value_type entry(config_upper, info);
    i = this->CompileInfoMap.insert(entry).first;
  }
  return &i->second;
}

cmGeneratorTarget::ModuleDefinitionInfo const*
cmGeneratorTarget::GetModuleDefinitionInfo(std::string const& config) const
{
  // A module definition file only makes sense on certain target types.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup/compute/cache the compile information for this configuration.
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto i = this->ModuleDefinitionInfoMap.find(config_upper);
  if (i == this->ModuleDefinitionInfoMap.end()) {
    ModuleDefinitionInfo info;
    this->ComputeModuleDefinitionInfo(config, info);
    ModuleDefinitionInfoMapType::value_type entry(config_upper, info);
    i = this->ModuleDefinitionInfoMap.insert(entry).first;
  }
  return &i->second;
}

void cmGeneratorTarget::ComputeModuleDefinitionInfo(
  std::string const& config, ModuleDefinitionInfo& info) const
{
  this->GetModuleDefinitionSources(info.Sources, config);
  info.WindowsExportAllSymbols =
    this->Makefile->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS") &&
    this->GetPropertyAsBool("WINDOWS_EXPORT_ALL_SYMBOLS");
#if !defined(CMAKE_BOOTSTRAP)
  info.DefFileGenerated =
    info.WindowsExportAllSymbols || info.Sources.size() > 1;
#else
  // Our __create_def helper is not available during CMake bootstrap.
  info.DefFileGenerated = false;
#endif
  if (info.DefFileGenerated) {
    info.DefFile =
      this->GetObjectDirectory(config) /* has slash */ + "exports.def";
  } else if (!info.Sources.empty()) {
    info.DefFile = info.Sources.front()->GetFullPath();
  }
}

bool cmGeneratorTarget::IsAIX() const
{
  return this->Target->IsAIX();
}

bool cmGeneratorTarget::IsApple() const
{
  return this->Target->IsApple();
}

bool cmGeneratorTarget::IsDLLPlatform() const
{
  return this->Target->IsDLLPlatform();
}

void cmGeneratorTarget::GetAutoUicOptions(std::vector<std::string>& result,
                                          const std::string& config) const
{
  const char* prop =
    this->GetLinkInterfaceDependentStringProperty("AUTOUIC_OPTIONS", config);
  if (!prop) {
    return;
  }

  cmGeneratorExpressionDAGChecker dagChecker(this, "AUTOUIC_OPTIONS", nullptr,
                                             nullptr);
  cmExpandList(cmGeneratorExpression::Evaluate(prop, this->LocalGenerator,
                                               config, this, &dagChecker),
               result);
}

static void processILibs(const std::string& config,
                         cmGeneratorTarget const* headTarget,
                         cmLinkItem const& item, cmGlobalGenerator* gg,
                         std::vector<cmGeneratorTarget const*>& tgts,
                         std::set<cmGeneratorTarget const*>& emitted)
{
  if (item.Target && emitted.insert(item.Target).second) {
    tgts.push_back(item.Target);
    if (cmLinkInterfaceLibraries const* iface =
          item.Target->GetLinkInterfaceLibraries(config, headTarget,
                                                 LinkInterfaceFor::Usage)) {
      for (cmLinkItem const& lib : iface->Libraries) {
        processILibs(config, headTarget, lib, gg, tgts, emitted);
      }
    }
  }
}

const std::vector<const cmGeneratorTarget*>&
cmGeneratorTarget::GetLinkImplementationClosure(
  const std::string& config) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    static std::vector<const cmGeneratorTarget*> const empty;
    return empty;
  }

  LinkImplClosure& tgts = this->LinkImplClosureMap[config];
  if (!tgts.Done) {
    tgts.Done = true;
    std::set<cmGeneratorTarget const*> emitted;

    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibraries(config, LinkInterfaceFor::Usage);
    assert(impl);

    for (cmLinkImplItem const& lib : impl->Libraries) {
      processILibs(config, this, lib,
                   this->LocalGenerator->GetGlobalGenerator(), tgts, emitted);
    }
  }
  return tgts;
}

class cmTargetTraceDependencies
{
public:
  cmTargetTraceDependencies(cmGeneratorTarget* target);
  void Trace();

private:
  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator const* GlobalGenerator;
  using SourceEntry = cmGeneratorTarget::SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  using NameMapType = std::map<std::string, cmSourcesWithOutput>;
  NameMapType NameMap;
  std::vector<std::string> NewSources;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
};

cmTargetTraceDependencies::cmTargetTraceDependencies(cmGeneratorTarget* target)
  : GeneratorTarget(target)
{
  // Convenience.
  this->Makefile = target->Target->GetMakefile();
  this->LocalGenerator = target->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
  this->CurrentEntry = nullptr;

  // Queue all the source files already specified for the target.
  std::set<cmSourceFile*> emitted;
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  for (std::string const& c : configs) {
    std::vector<cmSourceFile*> sources;
    this->GeneratorTarget->GetSourceFiles(sources, c);
    for (cmSourceFile* sf : sources) {
      const std::set<cmGeneratorTarget const*> tgts =
        this->GlobalGenerator->GetFilenameTargetDepends(sf);
      if (cm::contains(tgts, this->GeneratorTarget)) {
        std::ostringstream e;
        e << "Evaluation output file\n  \"" << sf->ResolveFullPath()
          << "\"\ndepends on the sources of a target it is used in.  This "
             "is a dependency loop and is not allowed.";
        this->GeneratorTarget->LocalGenerator->IssueMessage(
          MessageType::FATAL_ERROR, e.str());
        return;
      }
      if (emitted.insert(sf).second && this->SourcesQueued.insert(sf).second) {
        this->SourceQueue.push(sf);
      }
    }
  }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->GeneratorTarget->GetPreBuildCommands());
  this->CheckCustomCommands(this->GeneratorTarget->GetPreLinkCommands());
  this->CheckCustomCommands(this->GeneratorTarget->GetPostBuildCommands());
}

void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while (!this->SourceQueue.empty()) {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->GeneratorTarget->SourceDepends[sf];

    // Queue dependencies added explicitly by the user.
    if (cmValue additionalDeps = sf->GetProperty("OBJECT_DEPENDS")) {
      cmList objDeps{ *additionalDeps };
      for (auto& objDep : objDeps) {
        if (cmSystemTools::FileIsFullPath(objDep)) {
          objDep = cmSystemTools::CollapseFullPath(objDep);
        }
      }
      this->FollowNames(objDeps);
    }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->ResolveFullPath());

    // Queue dependencies added programmatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if (cmCustomCommand const* cc = sf->GetCustomCommand()) {
      this->CheckCustomCommand(*cc);
    }
  }
  this->CurrentEntry = nullptr;

  this->GeneratorTarget->AddTracedSources(this->NewSources);
}

void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if (this->SourcesQueued.insert(sf).second) {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target at the end.
    this->NewSources.push_back(sf->ResolveFullPath());
  }
}

void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  // Use lower bound with key comparison to not repeat the search for the
  // insert position if the name could not be found (which is the common case).
  auto i = this->NameMap.lower_bound(name);
  if (i == this->NameMap.end() || i->first != name) {
    // Check if we know how to generate this file.
    cmSourcesWithOutput sources =
      this->LocalGenerator->GetSourcesWithOutput(name);
    // If we failed to find a target or source and we have a relative path, it
    // might be a valid source if made relative to the current binary
    // directory.
    if (!sources.Target && !sources.Source &&
        !cmSystemTools::FileIsFullPath(name)) {
      auto fullname =
        cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/', name);
      fullname = cmSystemTools::CollapseFullPath(
        fullname, this->Makefile->GetHomeOutputDirectory());
      sources = this->LocalGenerator->GetSourcesWithOutput(fullname);
    }
    i = this->NameMap.emplace_hint(i, name, sources);
  }
  if (cmTarget* t = i->second.Target) {
    // The name is a byproduct of a utility target or a PRE_BUILD, PRE_LINK, or
    // POST_BUILD command.
    this->GeneratorTarget->Target->AddUtility(t->GetName(), false);
  }
  if (cmSourceFile* sf = i->second.Source) {
    // For now only follow the dependency if the source file is not a
    // byproduct.  Semantics of byproducts in a non-Ninja context will have to
    // be defined first.
    if (!i->second.SourceIsByproduct) {
      // Record the dependency we just followed.
      if (this->CurrentEntry) {
        this->CurrentEntry->Depends.push_back(sf);
      }
      this->QueueSource(sf);
    }
  }
}

void cmTargetTraceDependencies::FollowNames(
  std::vector<std::string> const& names)
{
  for (std::string const& name : names) {
    this->FollowName(name);
  }
}

bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if (cmSystemTools::GetFilenameLastExtension(util) == ".exe") {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
  }

  // Check for a target with this name.
  if (cmGeneratorTarget* t =
        this->GeneratorTarget->GetLocalGenerator()->FindGeneratorTargetToUse(
          util)) {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if (cmSystemTools::FileIsFullPath(dep)) {
      if (t->GetType() >= cmStateEnums::EXECUTABLE &&
          t->GetType() <= cmStateEnums::MODULE_LIBRARY) {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocationForBuild();
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation);
        if (depLocation == tLocation) {
          this->GeneratorTarget->Target->AddUtility(util, false);
          return true;
        }
      }
    } else {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->GeneratorTarget->Target->AddUtility(util, true);
      return true;
    }
  }

  // The dependency does not name a target built in this project.
  return false;
}

void cmTargetTraceDependencies::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Collect dependencies referenced by all configurations.
  std::set<std::string> depends;
  for (std::string const& config :
       this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig)) {
    for (cmCustomCommandGenerator const& ccg :
         this->LocalGenerator->MakeCustomCommandGenerators(cc, config)) {
      // Collect target-level dependencies referenced in command lines.
      for (auto const& util : ccg.GetUtilities()) {
        this->GeneratorTarget->Target->AddUtility(util);
      }

      // Collect file-level dependencies referenced in DEPENDS.
      depends.insert(ccg.GetDepends().begin(), ccg.GetDepends().end());
    }
  }

  // Queue file-level dependencies.
  for (std::string const& dep : depends) {
    if (!this->IsUtility(dep)) {
      // The dependency does not name a target and may be a file we
      // know how to generate.  Queue it.
      this->FollowName(dep);
    }
  }
}

void cmTargetTraceDependencies::CheckCustomCommands(
  const std::vector<cmCustomCommand>& commands)
{
  for (cmCustomCommand const& command : commands) {
    this->CheckCustomCommand(command);
  }
}

void cmGeneratorTarget::TraceDependencies()
{
  // CMake-generated targets have no dependencies to trace.  Normally tracing
  // would find nothing anyway, but when building CMake itself the "install"
  // target command ends up referencing the "cmake" target but we do not
  // really want the dependency because "install" depend on "all" anyway.
  if (this->GetType() == cmStateEnums::GLOBAL_TARGET) {
    return;
  }

  // Use a helper object to trace the dependencies.
  cmTargetTraceDependencies tracer(this);
  tracer.Trace();
}

std::string cmGeneratorTarget::GetCompilePDBDirectory(
  const std::string& config) const
{
  if (CompileInfo const* info = this->GetCompileInfo(config)) {
    return info->CompilePdbDir;
  }
  return "";
}

std::vector<std::string> cmGeneratorTarget::GetAppleArchs(
  std::string const& config, cm::optional<std::string> lang) const
{
  cmList archList;
  if (!this->IsApple()) {
    return std::move(archList.data());
  }
  cmValue archs = nullptr;
  if (!config.empty()) {
    std::string defVarName =
      cmStrCat("OSX_ARCHITECTURES_", cmSystemTools::UpperCase(config));
    archs = this->GetProperty(defVarName);
  }
  if (!archs) {
    archs = this->GetProperty("OSX_ARCHITECTURES");
  }
  if (archs) {
    archList.assign(*archs);
  }
  if (archList.empty() &&
      // Fall back to a default architecture if no compiler target is set.
      (!lang ||
       this->Makefile
         ->GetDefinition(cmStrCat("CMAKE_", *lang, "_COMPILER_TARGET"))
         .IsEmpty())) {
    archList.assign(
      this->Makefile->GetDefinition("_CMAKE_APPLE_ARCHS_DEFAULT"));
  }
  return std::move(archList.data());
}

void cmGeneratorTarget::AddExplicitLanguageFlags(std::string& flags,
                                                 cmSourceFile const& sf) const
{
  cmValue lang = sf.GetProperty("LANGUAGE");
  if (!lang) {
    return;
  }

  switch (this->GetPolicyStatusCMP0119()) {
    case cmPolicies::WARN:
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // The OLD behavior is to not add explicit language flags.
      return;
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::NEW:
      // The NEW behavior is to add explicit language flags.
      break;
  }

  this->LocalGenerator->AppendFeatureOptions(flags, *lang,
                                             "EXPLICIT_LANGUAGE");
}

void cmGeneratorTarget::AddCUDAArchitectureFlags(cmBuildStep compileOrLink,
                                                 std::string const& config,
                                                 std::string& flags) const
{
  std::string arch = this->GetSafeProperty("CUDA_ARCHITECTURES");

  if (arch.empty()) {
    switch (this->GetPolicyStatusCMP0104()) {
      case cmPolicies::WARN:
        if (!this->LocalGenerator->GetCMakeInstance()->GetIsInTryCompile()) {
          this->Makefile->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0104) +
              "\nCUDA_ARCHITECTURES is empty for target \"" + this->GetName() +
              "\".");
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      default:
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          "CUDA_ARCHITECTURES is empty for target \"" + this->GetName() +
            "\".");
    }
  }

  // If CUDA_ARCHITECTURES is false we don't add any architectures.
  if (cmIsOff(arch)) {
    return;
  }

  return this->AddCUDAArchitectureFlagsImpl(compileOrLink, config, "CUDA",
                                            std::move(arch), flags);
}

void cmGeneratorTarget::AddCUDAArchitectureFlagsImpl(cmBuildStep compileOrLink,
                                                     std::string const& config,
                                                     std::string const& lang,
                                                     std::string arch,
                                                     std::string& flags) const
{
  std::string const& compiler = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILER_ID"));
  const bool ipoEnabled = this->IsIPOEnabled(lang, config);

  // Check for special modes: `all`, `all-major`.
  if (arch == "all" || arch == "all-major") {
    if (compiler == "NVIDIA" &&
        cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                      this->Makefile->GetDefinition(cmStrCat(
                                        "CMAKE_", lang, "_COMPILER_VERSION")),
                                      "11.5")) {
      flags = cmStrCat(flags, " -arch=", arch);
      return;
    }
    if (arch == "all") {
      arch = *this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "_ARCHITECTURES_ALL"));
    } else if (arch == "all-major") {
      arch = *this->Makefile->GetDefinition(
        cmStrCat("CMAKE_", lang, "_ARCHITECTURES_ALL_MAJOR"));
    }
  } else if (arch == "native") {
    cmValue native = this->Makefile->GetDefinition(
      cmStrCat("CMAKE_", lang, "_ARCHITECTURES_NATIVE"));
    if (native.IsEmpty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(lang,
                 "_ARCHITECTURES is set to \"native\", but no NVIDIA GPU was "
                 "detected."));
    }
    if (compiler == "NVIDIA" &&
        cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                      this->Makefile->GetDefinition(cmStrCat(
                                        "CMAKE_", lang, "_COMPILER_VERSION")),
                                      "11.6")) {
      flags = cmStrCat(flags, " -arch=", arch);
      return;
    }
    arch = *native;
  }

  struct CudaArchitecture
  {
    std::string name;
    bool real{ true };
    bool virtual_{ true };
  };
  std::vector<CudaArchitecture> architectures;

  {
    cmList options(arch);

    for (auto& option : options) {
      CudaArchitecture architecture;

      // Architecture name is up to the first specifier.
      std::size_t pos = option.find_first_of('-');
      architecture.name = option.substr(0, pos);

      if (pos != std::string::npos) {
        cm::string_view specifier{ option.c_str() + pos + 1,
                                   option.length() - pos - 1 };

        if (specifier == "real") {
          architecture.real = true;
          architecture.virtual_ = false;
        } else if (specifier == "virtual") {
          architecture.real = false;
          architecture.virtual_ = true;
        } else {
          this->Makefile->IssueMessage(
            MessageType::FATAL_ERROR,
            "Unknown CUDA architecture specifier \"" + std::string(specifier) +
              "\".");
        }
      }

      architectures.emplace_back(architecture);
    }
  }

  if (compiler == "NVIDIA") {
    if (ipoEnabled && compileOrLink == cmBuildStep::Link) {
      if (cmValue cudaIPOFlags = this->Makefile->GetDefinition(
            cmStrCat("CMAKE_", lang, "_LINK_OPTIONS_IPO"))) {
        flags += *cudaIPOFlags;
      }
    }

    for (CudaArchitecture& architecture : architectures) {
      flags +=
        " \"--generate-code=arch=compute_" + architecture.name + ",code=[";

      if (architecture.virtual_) {
        flags += "compute_" + architecture.name;

        if (ipoEnabled || architecture.real) {
          flags += ",";
        }
      }

      if (ipoEnabled) {
        if (compileOrLink == cmBuildStep::Compile) {
          flags += "lto_" + architecture.name;
        } else if (compileOrLink == cmBuildStep::Link) {
          flags += "sm_" + architecture.name;
        }
      } else if (architecture.real) {
        flags += "sm_" + architecture.name;
      }

      flags += "]\"";
    }
  } else if (compiler == "Clang" && compileOrLink == cmBuildStep::Compile) {
    for (CudaArchitecture& architecture : architectures) {
      flags += " --cuda-gpu-arch=sm_" + architecture.name;

      if (!architecture.real) {
        this->Makefile->IssueMessage(
          MessageType::WARNING,
          "Clang doesn't support disabling CUDA real code generation.");
      }

      if (!architecture.virtual_) {
        flags += " --no-cuda-include-ptx=sm_" + architecture.name;
      }
    }
  }
}

void cmGeneratorTarget::AddISPCTargetFlags(std::string& flags) const
{
  const std::string& arch = this->GetSafeProperty("ISPC_INSTRUCTION_SETS");

  // If ISPC_TARGET is false we don't add any architectures.
  if (cmIsOff(arch)) {
    return;
  }

  std::string const& compiler =
    this->Makefile->GetSafeDefinition("CMAKE_ISPC_COMPILER_ID");

  if (compiler == "Intel") {
    cmList targets(arch);
    if (!targets.empty()) {
      flags += cmStrCat(" --target=", cmWrap("", targets, "", ","));
    }
  }
}

void cmGeneratorTarget::AddHIPArchitectureFlags(cmBuildStep compileOrLink,
                                                std::string const& config,
                                                std::string& flags) const
{
  std::string arch = this->GetSafeProperty("HIP_ARCHITECTURES");

  if (arch.empty()) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 "HIP_ARCHITECTURES is empty for target \"" +
                                   this->GetName() + "\".");
  }

  // If HIP_ARCHITECTURES is false we don't add any architectures.
  if (cmIsOff(arch)) {
    return;
  }

  if (this->Makefile->GetSafeDefinition("CMAKE_HIP_PLATFORM") == "nvidia") {
    return this->AddCUDAArchitectureFlagsImpl(compileOrLink, config, "HIP",
                                              std::move(arch), flags);
  }

  cmList options(arch);

  for (std::string& option : options) {
    flags += " --offload-arch=" + option;
  }
}

void cmGeneratorTarget::AddCUDAToolkitFlags(std::string& flags) const
{
  std::string const& compiler =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_ID");

  if (compiler == "Clang") {
    // Pass CUDA toolkit explicitly to Clang.
    // Clang's searching for the system CUDA toolkit isn't very good and it's
    // expected the user will explicitly pass the toolkit path.
    // This also avoids Clang having to search for the toolkit on every
    // invocation.
    std::string toolkitRoot =
      this->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_LIBRARY_ROOT");

    if (!toolkitRoot.empty()) {
      flags += " --cuda-path=" +
        this->LocalGenerator->ConvertToOutputFormat(toolkitRoot,
                                                    cmOutputConverter::SHELL);
    }
  }
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetFeatureSpecificLinkRuleVariable(
  std::string const& var, std::string const& lang,
  std::string const& config) const
{
  if (this->IsIPOEnabled(lang, config)) {
    std::string varIPO = var + "_IPO";
    if (this->Makefile->IsDefinitionSet(varIPO)) {
      return varIPO;
    }
  }

  return var;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetCreateRuleVariable(
  std::string const& lang, std::string const& config) const
{
  switch (this->GetType()) {
    case cmStateEnums::STATIC_LIBRARY: {
      std::string var = "CMAKE_" + lang + "_CREATE_STATIC_LIBRARY";
      return this->GetFeatureSpecificLinkRuleVariable(var, lang, config);
    }
    case cmStateEnums::SHARED_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_LIBRARY";
    case cmStateEnums::MODULE_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_MODULE";
    case cmStateEnums::EXECUTABLE:
      if (this->IsExecutableWithExports()) {
        std::string linkExeWithExports =
          "CMAKE_" + lang + "_LINK_EXECUTABLE_WITH_EXPORTS";
        if (this->Makefile->IsDefinitionSet(linkExeWithExports)) {
          return linkExeWithExports;
        }
      }
      return "CMAKE_" + lang + "_LINK_EXECUTABLE";
    default:
      break;
  }
  return "";
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetClangTidyExportFixesDirectory(
  const std::string& lang) const
{
  cmValue val =
    this->GetProperty(cmStrCat(lang, "_CLANG_TIDY_EXPORT_FIXES_DIR"));
  if (!cmNonempty(val)) {
    return {};
  }

  std::string path = *val;
  if (!cmSystemTools::FileIsFullPath(path)) {
    path =
      cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/', path);
  }
  return cmSystemTools::CollapseFullPath(path);
}

namespace {
void processIncludeDirectories(cmGeneratorTarget const* tgt,
                               EvaluatedTargetPropertyEntries& entries,
                               std::vector<BT<std::string>>& includes,
                               std::unordered_set<std::string>& uniqueIncludes,
                               bool debugIncludes)
{
  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();
    bool const fromImported = item.Target && item.Target->IsImported();
    bool const checkCMP0027 = item.CheckCMP0027;

    std::string usedIncludes;
    for (std::string& entryInclude : entry.Values) {
      if (fromImported && !cmSystemTools::FileExists(entryInclude)) {
        std::ostringstream e;
        MessageType messageType = MessageType::FATAL_ERROR;
        if (checkCMP0027) {
          switch (tgt->GetPolicyStatusCMP0027()) {
            case cmPolicies::WARN:
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0027) << "\n";
              CM_FALLTHROUGH;
            case cmPolicies::OLD:
              messageType = MessageType::AUTHOR_WARNING;
              break;
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::NEW:
              break;
          }
        }
        /* clang-format off */
        e << "Imported target \"" << targetName << "\" includes "
             "non-existent path\n  \"" << entryInclude << "\"\nin its "
             "INTERFACE_INCLUDE_DIRECTORIES. Possible reasons include:\n"
             "* The path was deleted, renamed, or moved to another "
             "location.\n"
             "* An install or uninstall procedure did not complete "
             "successfully.\n"
             "* The installation package was faulty and references files it "
             "does not provide.\n";
        /* clang-format on */
        tgt->GetLocalGenerator()->IssueMessage(messageType, e.str());
        return;
      }

      if (!cmSystemTools::FileIsFullPath(entryInclude)) {
        std::ostringstream e;
        bool noMessage = false;
        MessageType messageType = MessageType::FATAL_ERROR;
        if (!targetName.empty()) {
          /* clang-format off */
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_INCLUDE_DIRECTORIES:\n"
            "  \"" << entryInclude << "\"";
          /* clang-format on */
        } else {
          switch (tgt->GetPolicyStatusCMP0021()) {
            case cmPolicies::WARN: {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0021) << "\n";
              messageType = MessageType::AUTHOR_WARNING;
            } break;
            case cmPolicies::OLD:
              noMessage = true;
              break;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
          }
          e << "Found relative path while evaluating include directories of "
               "\""
            << tgt->GetName() << "\":\n  \"" << entryInclude << "\"\n";
        }
        if (!noMessage) {
          tgt->GetLocalGenerator()->IssueMessage(messageType, e.str());
          if (messageType == MessageType::FATAL_ERROR) {
            return;
          }
        }
      }

      if (!cmIsOff(entryInclude)) {
        cmSystemTools::ConvertToUnixSlashes(entryInclude);
      }

      if (uniqueIncludes.insert(entryInclude).second) {
        includes.emplace_back(entryInclude, entry.Backtrace);
        if (debugIncludes) {
          usedIncludes += " * " + entryInclude + "\n";
        }
      }
    }
    if (!usedIncludes.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used includes for target ") + tgt->GetName() + ":\n" +
          usedIncludes,
        entry.Backtrace);
    }
  }
}
}

std::vector<BT<std::string>> cmGeneratorTarget::GetIncludeDirectories(
  const std::string& config, const std::string& lang) const
{
  ConfigAndLanguage cacheKey(config, lang);
  {
    auto it = this->IncludeDirectoriesCache.find(cacheKey);
    if (it != this->IncludeDirectoriesCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> includes;
  std::unordered_set<std::string> uniqueIncludes;

  cmGeneratorExpressionDAGChecker dagChecker(this, "INCLUDE_DIRECTORIES",
                                             nullptr, nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugIncludes = !this->DebugIncludesDone &&
    cm::contains(debugProperties, "INCLUDE_DIRECTORIES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugIncludesDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, lang, &dagChecker, this->IncludeDirectoriesEntries);

  if (lang == "Swift") {
    AddLangSpecificImplicitIncludeDirectories(
      this, lang, config, "Swift_MODULE_DIRECTORY",
      IncludeDirectoryFallBack::BINARY, entries);
  }

  if (this->CanCompileSources() && (lang != "Swift" && lang != "Fortran")) {

    const std::string propertyName = "ISPC_HEADER_DIRECTORY";

    // If this target has ISPC sources make sure to add the header
    // directory to other compilation units
    if (cm::contains(this->GetAllConfigCompileLanguages(), "ISPC")) {
      if (cmValue val = this->GetProperty(propertyName)) {
        includes.emplace_back(*val);
      } else {
        includes.emplace_back(this->GetObjectDirectory(config));
      }
    }

    AddLangSpecificImplicitIncludeDirectories(
      this, "ISPC", config, propertyName, IncludeDirectoryFallBack::OBJECT,
      entries);
  }

  AddInterfaceEntries(this, config, "INTERFACE_INCLUDE_DIRECTORIES", lang,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  processIncludeDirectories(this, entries, includes, uniqueIncludes,
                            debugIncludes);

  if (this->IsApple()) {
    if (cmLinkImplementationLibraries const* impl =
          this->GetLinkImplementationLibraries(config,
                                               LinkInterfaceFor::Usage)) {
      for (cmLinkImplItem const& lib : impl->Libraries) {
        std::string libDir;
        if (lib.Target == nullptr) {
          libDir = cmSystemTools::CollapseFullPath(
            lib.AsStr(), this->Makefile->GetHomeOutputDirectory());
        } else if (lib.Target->Target->IsFrameworkOnApple() ||
                   this->IsImportedFrameworkFolderOnApple(config)) {
          libDir = lib.Target->GetLocation(config);
        } else {
          continue;
        }

        auto fwDescriptor =
          this->GetGlobalGenerator()->SplitFrameworkPath(libDir);
        if (!fwDescriptor) {
          continue;
        }

        auto fwInclude = fwDescriptor->GetFrameworkPath();
        if (uniqueIncludes.insert(fwInclude).second) {
          includes.emplace_back(fwInclude, cmListFileBacktrace());
        }
      }
    }
  }

  this->IncludeDirectoriesCache.emplace(cacheKey, includes);
  return includes;
}

enum class OptionsParse
{
  None,
  Shell
};

namespace {
const auto DL_BEGIN = "<DEVICE_LINK>"_s;
const auto DL_END = "</DEVICE_LINK>"_s;

void processOptions(cmGeneratorTarget const* tgt,
                    EvaluatedTargetPropertyEntries const& entries,
                    std::vector<BT<std::string>>& options,
                    std::unordered_set<std::string>& uniqueOptions,
                    bool debugOptions, const char* logName, OptionsParse parse,
                    bool processDeviceOptions = false)
{
  bool splitOption = !processDeviceOptions;
  for (EvaluatedTargetPropertyEntry const& entry : entries.Entries) {
    std::string usedOptions;
    for (std::string const& opt : entry.Values) {
      if (processDeviceOptions && (opt == DL_BEGIN || opt == DL_END)) {
        options.emplace_back(opt, entry.Backtrace);
        splitOption = opt == DL_BEGIN;
        continue;
      }

      if (uniqueOptions.insert(opt).second) {
        if (parse == OptionsParse::Shell &&
            cmHasLiteralPrefix(opt, "SHELL:")) {
          if (splitOption) {
            std::vector<std::string> tmp;
            cmSystemTools::ParseUnixCommandLine(opt.c_str() + 6, tmp);
            for (std::string& o : tmp) {
              options.emplace_back(std::move(o), entry.Backtrace);
            }
          } else {
            options.emplace_back(std::string(opt.c_str() + 6),
                                 entry.Backtrace);
          }
        } else {
          options.emplace_back(opt, entry.Backtrace);
        }
        if (debugOptions) {
          usedOptions += " * " + opt + "\n";
        }
      }
    }
    if (!usedOptions.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used ") + logName + std::string(" for target ") +
          tgt->GetName() + ":\n" + usedOptions,
        entry.Backtrace);
    }
  }
}

std::vector<BT<std::string>> wrapOptions(
  std::vector<std::string>& options, const cmListFileBacktrace& bt,
  const std::vector<std::string>& wrapperFlag, const std::string& wrapperSep,
  bool concatFlagAndArgs)
{
  std::vector<BT<std::string>> result;

  if (options.empty()) {
    return result;
  }

  if (wrapperFlag.empty()) {
    // nothing specified, insert elements as is
    result.reserve(options.size());
    for (std::string& o : options) {
      result.emplace_back(std::move(o), bt);
    }
    return result;
  }

  for (std::vector<std::string>::size_type index = 0; index < options.size();
       index++) {
    if (cmHasLiteralPrefix(options[index], "LINKER:")) {
      // LINKER wrapper specified, insert elements as is
      result.emplace_back(std::move(options[index]), bt);
      continue;
    }
    if (cmHasLiteralPrefix(options[index], "-Wl,")) {
      // replace option by LINKER wrapper
      result.emplace_back(options[index].replace(0, 4, "LINKER:"), bt);
      continue;
    }
    if (cmHasLiteralPrefix(options[index], "-Xlinker=")) {
      // replace option by LINKER wrapper
      result.emplace_back(options[index].replace(0, 9, "LINKER:"), bt);
      continue;
    }
    if (options[index] == "-Xlinker") {
      // replace option by LINKER wrapper
      if (index + 1 < options.size()) {
        result.emplace_back("LINKER:" + options[++index], bt);
      } else {
        result.emplace_back(std::move(options[index]), bt);
      }
      continue;
    }

    // collect all options which must be transformed
    std::vector<std::string> opts;
    while (index < options.size()) {
      if (!cmHasLiteralPrefix(options[index], "LINKER:") &&
          !cmHasLiteralPrefix(options[index], "-Wl,") &&
          !cmHasLiteralPrefix(options[index], "-Xlinker")) {
        opts.emplace_back(std::move(options[index++]));
      } else {
        --index;
        break;
      }
    }
    if (opts.empty()) {
      continue;
    }

    if (!wrapperSep.empty()) {
      if (concatFlagAndArgs) {
        // insert flag elements except last one
        for (auto i = wrapperFlag.begin(); i != wrapperFlag.end() - 1; ++i) {
          result.emplace_back(*i, bt);
        }
        // concatenate last flag element and all list values
        // in one option
        result.emplace_back(wrapperFlag.back() + cmJoin(opts, wrapperSep), bt);
      } else {
        for (std::string const& i : wrapperFlag) {
          result.emplace_back(i, bt);
        }
        // concatenate all list values in one option
        result.emplace_back(cmJoin(opts, wrapperSep), bt);
      }
    } else {
      // prefix each element of list with wrapper
      if (concatFlagAndArgs) {
        std::transform(opts.begin(), opts.end(), opts.begin(),
                       [&wrapperFlag](std::string const& o) -> std::string {
                         return wrapperFlag.back() + o;
                       });
      }
      for (std::string& o : opts) {
        for (auto i = wrapperFlag.begin(),
                  e = concatFlagAndArgs ? wrapperFlag.end() - 1
                                        : wrapperFlag.end();
             i != e; ++i) {
          result.emplace_back(*i, bt);
        }
        result.emplace_back(std::move(o), bt);
      }
    }
  }
  return result;
}
}

void cmGeneratorTarget::GetCompileOptions(std::vector<std::string>& result,
                                          const std::string& config,
                                          const std::string& language) const
{
  std::vector<BT<std::string>> tmp = this->GetCompileOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileOptionsCache.find(cacheKey);
    if (it != this->CompileOptionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_OPTIONS", nullptr,
                                             nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugOptions = !this->DebugCompileOptionsDone &&
    cm::contains(debugProperties, "COMPILE_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileOptionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->CompileOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_OPTIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "compile options", OptionsParse::Shell);

  CompileOptionsCache.emplace(cacheKey, result);
  return result;
}

void cmGeneratorTarget::GetCompileFeatures(std::vector<std::string>& result,
                                           const std::string& config) const
{
  std::vector<BT<std::string>> tmp = this->GetCompileFeatures(config);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileFeatures(
  std::string const& config) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueFeatures;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_FEATURES", nullptr,
                                             nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugFeatures = !this->DebugCompileFeaturesDone &&
    cm::contains(debugProperties, "COMPILE_FEATURES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileFeaturesDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, std::string(), &dagChecker, this->CompileFeaturesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_FEATURES",
                      std::string(), &dagChecker, entries,
                      IncludeRuntimeInterface::Yes);

  processOptions(this, entries, result, uniqueFeatures, debugFeatures,
                 "compile features", OptionsParse::None);

  return result;
}

void cmGeneratorTarget::GetCompileDefinitions(
  std::vector<std::string>& result, const std::string& config,
  const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetCompileDefinitions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetCompileDefinitions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->CompileDefinitionsCache.find(cacheKey);
    if (it != this->CompileDefinitionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> list;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_DEFINITIONS",
                                             nullptr, nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDefines = !this->DebugCompileDefinitionsDone &&
    cm::contains(debugProperties, "COMPILE_DEFINITIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileDefinitionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->CompileDefinitionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_DEFINITIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  if (!config.empty()) {
    std::string configPropName =
      "COMPILE_DEFINITIONS_" + cmSystemTools::UpperCase(config);
    cmValue configProp = this->GetProperty(configPropName);
    if (configProp) {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0043)) {
        case cmPolicies::WARN: {
          this->LocalGenerator->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0043));
          CM_FALLTHROUGH;
        }
        case cmPolicies::OLD: {
          std::unique_ptr<TargetPropertyEntry> entry =
            CreateTargetPropertyEntry(
              *this->LocalGenerator->GetCMakeInstance(), *configProp);
          entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
            this, config, language, &dagChecker, *entry));
        } break;
        case cmPolicies::NEW:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::REQUIRED_IF_USED:
          break;
      }
    }
  }

  processOptions(this, entries, list, uniqueOptions, debugDefines,
                 "compile definitions", OptionsParse::None);

  this->CompileDefinitionsCache.emplace(cacheKey, list);
  return list;
}

std::vector<BT<std::string>> cmGeneratorTarget::GetPrecompileHeaders(
  const std::string& config, const std::string& language) const
{
  ConfigAndLanguage cacheKey(config, language);
  {
    auto it = this->PrecompileHeadersCache.find(cacheKey);
    if (it != this->PrecompileHeadersCache.end()) {
      return it->second;
    }
  }
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "PRECOMPILE_HEADERS",
                                             nullptr, nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDefines = !this->DebugPrecompileHeadersDone &&
    std::find(debugProperties.begin(), debugProperties.end(),
              "PRECOMPILE_HEADERS") != debugProperties.end();

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugPrecompileHeadersDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->PrecompileHeadersEntries);

  AddInterfaceEntries(this, config, "INTERFACE_PRECOMPILE_HEADERS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes);

  std::vector<BT<std::string>> list;
  processOptions(this, entries, list, uniqueOptions, debugDefines,
                 "precompile headers", OptionsParse::None);

  this->PrecompileHeadersCache.emplace(cacheKey, list);
  return list;
}

std::string cmGeneratorTarget::GetPchHeader(const std::string& config,
                                            const std::string& language,
                                            const std::string& arch) const
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }

  if (this->GetPropertyAsBool("DISABLE_PRECOMPILE_HEADERS")) {
    return std::string();
  }
  const cmGeneratorTarget* generatorTarget = this;
  cmValue pchReuseFrom =
    generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");

  const auto inserted =
    this->PchHeaders.insert(std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    const std::vector<BT<std::string>> headers =
      this->GetPrecompileHeaders(config, language);
    if (headers.empty() && !pchReuseFrom) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    if (pchReuseFrom) {
      generatorTarget =
        this->GetGlobalGenerator()->FindGeneratorTarget(*pchReuseFrom);
    }

    const std::map<std::string, std::string> languageToExtension = {
      { "C", ".h" },
      { "CXX", ".hxx" },
      { "OBJC", ".objc.h" },
      { "OBJCXX", ".objcxx.hxx" }
    };

    filename = generatorTarget->GetSupportDirectory();

    if (this->GetGlobalGenerator()->IsMultiConfig()) {
      filename = cmStrCat(filename, "/", config);
    }

    filename =
      cmStrCat(filename, "/cmake_pch", arch.empty() ? "" : cmStrCat("_", arch),
               languageToExtension.at(language));

    const std::string filename_tmp = cmStrCat(filename, ".tmp");
    if (!pchReuseFrom) {
      cmValue pchPrologue =
        this->Makefile->GetDefinition("CMAKE_PCH_PROLOGUE");
      cmValue pchEpilogue =
        this->Makefile->GetDefinition("CMAKE_PCH_EPILOGUE");

      std::string firstHeaderOnDisk;
      {
        cmGeneratedFileStream file(
          filename_tmp, false,
          this->GetGlobalGenerator()->GetMakefileEncoding());
        file << "/* generated by CMake */\n\n";
        if (pchPrologue) {
          file << *pchPrologue << "\n";
        }
        if (this->GetGlobalGenerator()->IsXcode()) {
          file << "#ifndef CMAKE_SKIP_PRECOMPILE_HEADERS\n";
        }
        if (language == "CXX" && !this->GetGlobalGenerator()->IsXcode()) {
          file << "#ifdef __cplusplus\n";
        }
        for (auto const& header_bt : headers) {
          if (header_bt.Value.empty()) {
            continue;
          }
          if (header_bt.Value[0] == '<' || header_bt.Value[0] == '\"') {
            file << "#include " << header_bt.Value << "\n";
          } else {
            file << "#include \"" << header_bt.Value << "\"\n";
          }

          if (cmSystemTools::FileExists(header_bt.Value) &&
              firstHeaderOnDisk.empty()) {
            firstHeaderOnDisk = header_bt.Value;
          }
        }
        if (language == "CXX" && !this->GetGlobalGenerator()->IsXcode()) {
          file << "#endif // __cplusplus\n";
        }
        if (this->GetGlobalGenerator()->IsXcode()) {
          file << "#endif // CMAKE_SKIP_PRECOMPILE_HEADERS\n";
        }
        if (pchEpilogue) {
          file << *pchEpilogue << "\n";
        }
      }

      if (!firstHeaderOnDisk.empty()) {
        cmFileTimes::Copy(firstHeaderOnDisk, filename_tmp);
      }

      cmSystemTools::MoveFileIfDifferent(filename_tmp, filename);
    }
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchSource(const std::string& config,
                                            const std::string& language,
                                            const std::string& arch) const
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }
  const auto inserted =
    this->PchSources.insert(std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    const std::string pchHeader = this->GetPchHeader(config, language, arch);
    if (pchHeader.empty()) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    const cmGeneratorTarget* generatorTarget = this;
    cmValue pchReuseFrom =
      generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");
    if (pchReuseFrom) {
      generatorTarget =
        this->GetGlobalGenerator()->FindGeneratorTarget(*pchReuseFrom);
    }

    filename = cmStrCat(generatorTarget->GetSupportDirectory(), "/cmake_pch");

    // For GCC the source extension will be transformed into .h[xx].gch
    if (!this->Makefile->IsOn("CMAKE_LINK_PCH")) {
      const std::map<std::string, std::string> languageToExtension = {
        { "C", ".h.c" },
        { "CXX", ".hxx.cxx" },
        { "OBJC", ".objc.h.m" },
        { "OBJCXX", ".objcxx.hxx.mm" }
      };

      filename = cmStrCat(filename, arch.empty() ? "" : cmStrCat("_", arch),
                          languageToExtension.at(language));
    } else {
      const std::map<std::string, std::string> languageToExtension = {
        { "C", ".c" }, { "CXX", ".cxx" }, { "OBJC", ".m" }, { "OBJCXX", ".mm" }
      };

      filename = cmStrCat(filename, arch.empty() ? "" : cmStrCat("_", arch),
                          languageToExtension.at(language));
    }

    const std::string filename_tmp = cmStrCat(filename, ".tmp");
    if (!pchReuseFrom) {
      {
        cmGeneratedFileStream file(filename_tmp);
        file << "/* generated by CMake */\n";
      }
      cmFileTimes::Copy(pchHeader, filename_tmp);
      cmSystemTools::MoveFileIfDifferent(filename_tmp, filename);
    }
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchFileObject(const std::string& config,
                                                const std::string& language,
                                                const std::string& arch)
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }
  const auto inserted =
    this->PchObjectFiles.insert(std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    const std::string pchSource = this->GetPchSource(config, language, arch);
    if (pchSource.empty()) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    auto* pchSf = this->Makefile->GetOrCreateSource(
      pchSource, false, cmSourceFileLocationKind::Known);

    filename = cmStrCat(this->ObjectDirectory, this->GetObjectName(pchSf));
    if (this->GetGlobalGenerator()->IsMultiConfig()) {
      cmSystemTools::ReplaceString(
        filename, this->GetGlobalGenerator()->GetCMakeCFGIntDir(), config);
    }
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchFile(const std::string& config,
                                          const std::string& language,
                                          const std::string& arch)
{
  const auto inserted =
    this->PchFiles.insert(std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    std::string& pchFile = inserted.first->second;

    const std::string pchExtension =
      this->Makefile->GetSafeDefinition("CMAKE_PCH_EXTENSION");

    if (this->Makefile->IsOn("CMAKE_LINK_PCH")) {
      auto replaceExtension = [](const std::string& str,
                                 const std::string& ext) -> std::string {
        auto dot_pos = str.rfind('.');
        std::string result;
        if (dot_pos != std::string::npos) {
          result = str.substr(0, dot_pos);
        }
        result += ext;
        return result;
      };

      cmGeneratorTarget* generatorTarget = this;
      cmValue pchReuseFrom =
        generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");
      if (pchReuseFrom) {
        generatorTarget =
          this->GetGlobalGenerator()->FindGeneratorTarget(*pchReuseFrom);
      }

      const std::string pchFileObject =
        generatorTarget->GetPchFileObject(config, language, arch);
      if (!pchExtension.empty()) {
        pchFile = replaceExtension(pchFileObject, pchExtension);
      }
    } else {
      pchFile = this->GetPchHeader(config, language, arch);
      pchFile += pchExtension;
    }
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchCreateCompileOptions(
  const std::string& config, const std::string& language,
  const std::string& arch)
{
  const auto inserted = this->PchCreateCompileOptions.insert(
    std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    std::string& createOptionList = inserted.first->second;

    if (this->GetPropertyAsBool("PCH_WARN_INVALID")) {
      createOptionList = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_", language, "_COMPILE_OPTIONS_INVALID_PCH"));
    }

    if (this->GetPropertyAsBool("PCH_INSTANTIATE_TEMPLATES")) {
      std::string varName = cmStrCat(
        "CMAKE_", language, "_COMPILE_OPTIONS_INSTANTIATE_TEMPLATES_PCH");
      std::string instantiateOption =
        this->Makefile->GetSafeDefinition(varName);
      if (!instantiateOption.empty()) {
        createOptionList = cmStrCat(createOptionList, ";", instantiateOption);
      }
    }

    const std::string createOptVar =
      cmStrCat("CMAKE_", language, "_COMPILE_OPTIONS_CREATE_PCH");

    createOptionList = cmStrCat(
      createOptionList, ";", this->Makefile->GetSafeDefinition(createOptVar));

    const std::string pchHeader = this->GetPchHeader(config, language, arch);
    const std::string pchFile = this->GetPchFile(config, language, arch);

    cmSystemTools::ReplaceString(createOptionList, "<PCH_HEADER>", pchHeader);
    cmSystemTools::ReplaceString(createOptionList, "<PCH_FILE>", pchFile);
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchUseCompileOptions(
  const std::string& config, const std::string& language,
  const std::string& arch)
{
  const auto inserted = this->PchUseCompileOptions.insert(
    std::make_pair(language + config + arch, ""));
  if (inserted.second) {
    std::string& useOptionList = inserted.first->second;

    if (this->GetPropertyAsBool("PCH_WARN_INVALID")) {
      useOptionList = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_", language, "_COMPILE_OPTIONS_INVALID_PCH"));
    }

    const std::string useOptVar =
      cmStrCat(language, "_COMPILE_OPTIONS_USE_PCH");

    std::string const& useOptionListProperty =
      this->GetSafeProperty(useOptVar);

    useOptionList = cmStrCat(
      useOptionList, ";",
      useOptionListProperty.empty()
        ? this->Makefile->GetSafeDefinition(cmStrCat("CMAKE_", useOptVar))
        : useOptionListProperty);

    const std::string pchHeader = this->GetPchHeader(config, language, arch);
    const std::string pchFile = this->GetPchFile(config, language, arch);

    cmSystemTools::ReplaceString(useOptionList, "<PCH_HEADER>", pchHeader);
    cmSystemTools::ReplaceString(useOptionList, "<PCH_FILE>", pchFile);
  }
  return inserted.first->second;
}

void cmGeneratorTarget::AddSourceFileToUnityBatch(
  const std::string& sourceFilename)
{
  this->UnityBatchedSourceFiles.insert(sourceFilename);
}

bool cmGeneratorTarget::IsSourceFilePartOfUnityBatch(
  const std::string& sourceFilename) const
{
  if (!this->GetPropertyAsBool("UNITY_BUILD")) {
    return false;
  }

  return this->UnityBatchedSourceFiles.find(sourceFilename) !=
    this->UnityBatchedSourceFiles.end();
}

void cmGeneratorTarget::GetLinkOptions(std::vector<std::string>& result,
                                       const std::string& config,
                                       const std::string& language) const
{
  if (this->IsDeviceLink() &&
      this->GetPolicyStatusCMP0105() != cmPolicies::NEW) {
    // link options are not propagated to the device link step
    return;
  }

  std::vector<BT<std::string>> tmp = this->GetLinkOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkOptions(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(
    config, cmStrCat(language, this->IsDeviceLink() ? "-device" : ""));
  {
    auto it = this->LinkOptionsCache.find(cacheKey);
    if (it != this->LinkOptionsCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_OPTIONS", nullptr,
                                             nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugOptions = !this->DebugLinkOptionsDone &&
    cm::contains(debugProperties, "LINK_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkOptionsDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->LinkOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_OPTIONS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? LinkInterfaceFor::Link
                        : LinkInterfaceFor::Usage);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "link options", OptionsParse::Shell, this->IsDeviceLink());

  if (this->IsDeviceLink()) {
    // wrap host link options
    const std::string wrapper(this->Makefile->GetSafeDefinition(
      "CMAKE_" + language + "_DEVICE_COMPILER_WRAPPER_FLAG"));
    cmList wrapperFlag{ wrapper };
    const std::string wrapperSep(this->Makefile->GetSafeDefinition(
      "CMAKE_" + language + "_DEVICE_COMPILER_WRAPPER_FLAG_SEP"));
    bool concatFlagAndArgs = true;
    if (!wrapperFlag.empty() && wrapperFlag.back() == " ") {
      concatFlagAndArgs = false;
      wrapperFlag.pop_back();
    }

    auto it = result.begin();
    while (it != result.end()) {
      if (it->Value == DL_BEGIN) {
        // device link options, no treatment
        it = result.erase(it);
        it = std::find_if(it, result.end(), [](const BT<std::string>& item) {
          return item.Value == DL_END;
        });
        if (it != result.end()) {
          it = result.erase(it);
        }
      } else {
        // host link options must be wrapped
        std::vector<std::string> options;
        cmSystemTools::ParseUnixCommandLine(it->Value.c_str(), options);
        auto hostOptions = wrapOptions(options, it->Backtrace, wrapperFlag,
                                       wrapperSep, concatFlagAndArgs);
        it = result.erase(it);
        // some compilers (like gcc 4.8 or Intel 19.0 or XLC 16) do not respect
        // C++11 standard: 'std::vector::insert()' do not returns an iterator,
        // so need to recompute the iterator after insertion.
        if (it == result.end()) {
          cm::append(result, hostOptions);
          it = result.end();
        } else {
          auto index = it - result.begin();
          result.insert(it, hostOptions.begin(), hostOptions.end());
          it = result.begin() + index + hostOptions.size();
        }
      }
    }
  }

  // Last step: replace "LINKER:" prefixed elements by
  // actual linker wrapper
  result = this->ResolveLinkerWrapper(result, language);

  this->LinkOptionsCache.emplace(cacheKey, result);
  return result;
}

std::vector<BT<std::string>>& cmGeneratorTarget::ResolveLinkerWrapper(
  std::vector<BT<std::string>>& result, const std::string& language,
  bool joinItems) const
{
  // replace "LINKER:" prefixed elements by actual linker wrapper
  const std::string wrapper(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language +
    (this->IsDeviceLink() ? "_DEVICE_LINKER_WRAPPER_FLAG"
                          : "_LINKER_WRAPPER_FLAG")));
  cmList wrapperFlag{ wrapper };
  const std::string wrapperSep(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language +
    (this->IsDeviceLink() ? "_DEVICE_LINKER_WRAPPER_FLAG_SEP"
                          : "_LINKER_WRAPPER_FLAG_SEP")));
  bool concatFlagAndArgs = true;
  if (!wrapperFlag.empty() && wrapperFlag.back() == " ") {
    concatFlagAndArgs = false;
    wrapperFlag.pop_back();
  }

  const std::string LINKER{ "LINKER:" };
  const std::string SHELL{ "SHELL:" };
  const std::string LINKER_SHELL = LINKER + SHELL;

  std::vector<BT<std::string>>::iterator entry;
  while ((entry = std::find_if(result.begin(), result.end(),
                               [&LINKER](BT<std::string> const& item) -> bool {
                                 return item.Value.compare(0, LINKER.length(),
                                                           LINKER) == 0;
                               })) != result.end()) {
    std::string value = std::move(entry->Value);
    cmListFileBacktrace bt = std::move(entry->Backtrace);
    entry = result.erase(entry);

    std::vector<std::string> linkerOptions;
    if (value.compare(0, LINKER_SHELL.length(), LINKER_SHELL) == 0) {
      cmSystemTools::ParseUnixCommandLine(
        value.c_str() + LINKER_SHELL.length(), linkerOptions);
    } else {
      linkerOptions = cmTokenize(value.substr(LINKER.length()), ",");
    }

    if (linkerOptions.empty() ||
        (linkerOptions.size() == 1 && linkerOptions.front().empty())) {
      continue;
    }

    // for now, raise an error if prefix SHELL: is part of arguments
    if (std::find_if(linkerOptions.begin(), linkerOptions.end(),
                     [&SHELL](const std::string& item) -> bool {
                       return item.find(SHELL) != std::string::npos;
                     }) != linkerOptions.end()) {
      this->LocalGenerator->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR,
        "'SHELL:' prefix is not supported as part of 'LINKER:' arguments.",
        this->GetBacktrace());
      return result;
    }

    std::vector<BT<std::string>> options = wrapOptions(
      linkerOptions, bt, wrapperFlag, wrapperSep, concatFlagAndArgs);
    if (joinItems) {
      result.insert(entry,
                    cmJoin(cmRange<decltype(options.cbegin())>(
                             options.cbegin(), options.cend()),
                           " "_s));
    } else {
      result.insert(entry, options.begin(), options.end());
    }
  }
  return result;
}

void cmGeneratorTarget::GetStaticLibraryLinkOptions(
  std::vector<std::string>& result, const std::string& config,
  const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetStaticLibraryLinkOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetStaticLibraryLinkOptions(
  std::string const& config, std::string const& language) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "STATIC_LIBRARY_OPTIONS",
                                             nullptr, nullptr);

  EvaluatedTargetPropertyEntries entries;
  if (cmValue linkOptions = this->GetProperty("STATIC_LIBRARY_OPTIONS")) {
    std::unique_ptr<TargetPropertyEntry> entry = CreateTargetPropertyEntry(
      *this->LocalGenerator->GetCMakeInstance(), *linkOptions);
    entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
      this, config, language, &dagChecker, *entry));
  }
  processOptions(this, entries, result, uniqueOptions, false,
                 "static library link options", OptionsParse::Shell);

  return result;
}

namespace {
void processLinkDirectories(cmGeneratorTarget const* tgt,
                            EvaluatedTargetPropertyEntries& entries,
                            std::vector<BT<std::string>>& directories,
                            std::unordered_set<std::string>& uniqueDirectories,
                            bool debugDirectories)
{
  for (EvaluatedTargetPropertyEntry& entry : entries.Entries) {
    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();

    std::string usedDirectories;
    for (std::string& entryDirectory : entry.Values) {
      if (!cmSystemTools::FileIsFullPath(entryDirectory)) {
        std::ostringstream e;
        bool noMessage = false;
        MessageType messageType = MessageType::FATAL_ERROR;
        if (!targetName.empty()) {
          /* clang-format off */
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_LINK_DIRECTORIES:\n"
            "  \"" << entryDirectory << "\"";
          /* clang-format on */
        } else {
          switch (tgt->GetPolicyStatusCMP0081()) {
            case cmPolicies::WARN: {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0081) << "\n";
              messageType = MessageType::AUTHOR_WARNING;
            } break;
            case cmPolicies::OLD:
              noMessage = true;
              break;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
          }
          e << "Found relative path while evaluating link directories of "
               "\""
            << tgt->GetName() << "\":\n  \"" << entryDirectory << "\"\n";
        }
        if (!noMessage) {
          tgt->GetLocalGenerator()->IssueMessage(messageType, e.str());
          if (messageType == MessageType::FATAL_ERROR) {
            return;
          }
        }
      }

      // Sanitize the path the same way the link_directories command does
      // in case projects set the LINK_DIRECTORIES property directly.
      cmSystemTools::ConvertToUnixSlashes(entryDirectory);
      if (uniqueDirectories.insert(entryDirectory).second) {
        directories.emplace_back(entryDirectory, entry.Backtrace);
        if (debugDirectories) {
          usedDirectories += " * " + entryDirectory + "\n";
        }
      }
    }
    if (!usedDirectories.empty()) {
      tgt->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::LOG,
        std::string("Used link directories for target ") + tgt->GetName() +
          ":\n" + usedDirectories,
        entry.Backtrace);
    }
  }
}
}

void cmGeneratorTarget::GetLinkDirectories(std::vector<std::string>& result,
                                           const std::string& config,
                                           const std::string& language) const
{
  std::vector<BT<std::string>> tmp =
    this->GetLinkDirectories(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkDirectories(
  std::string const& config, std::string const& language) const
{
  ConfigAndLanguage cacheKey(
    config, cmStrCat(language, this->IsDeviceLink() ? "-device" : ""));
  {
    auto it = this->LinkDirectoriesCache.find(cacheKey);
    if (it != this->LinkDirectoriesCache.end()) {
      return it->second;
    }
  }
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueDirectories;

  cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_DIRECTORIES", nullptr,
                                             nullptr);

  cmList debugProperties{ this->Makefile->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugDirectories = !this->DebugLinkDirectoriesDone &&
    cm::contains(debugProperties, "LINK_DIRECTORIES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkDirectoriesDone = true;
  }

  EvaluatedTargetPropertyEntries entries = EvaluateTargetPropertyEntries(
    this, config, language, &dagChecker, this->LinkDirectoriesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_DIRECTORIES", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? LinkInterfaceFor::Link
                        : LinkInterfaceFor::Usage);

  processLinkDirectories(this, entries, result, uniqueDirectories,
                         debugDirectories);

  this->LinkDirectoriesCache.emplace(cacheKey, result);
  return result;
}

void cmGeneratorTarget::GetLinkDepends(std::vector<std::string>& result,
                                       const std::string& config,
                                       const std::string& language) const
{
  std::vector<BT<std::string>> tmp = this->GetLinkDepends(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkDepends(
  std::string const& config, std::string const& language) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;
  cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_DEPENDS", nullptr,
                                             nullptr);

  EvaluatedTargetPropertyEntries entries;
  if (cmValue linkDepends = this->GetProperty("LINK_DEPENDS")) {
    cmList depends{ *linkDepends };
    for (const auto& depend : depends) {
      std::unique_ptr<TargetPropertyEntry> entry = CreateTargetPropertyEntry(
        *this->LocalGenerator->GetCMakeInstance(), depend);
      entries.Entries.emplace_back(EvaluateTargetPropertyEntry(
        this, config, language, &dagChecker, *entry));
    }
  }
  AddInterfaceEntries(this, config, "INTERFACE_LINK_DEPENDS", language,
                      &dagChecker, entries, IncludeRuntimeInterface::Yes,
                      this->GetPolicyStatusCMP0099() == cmPolicies::NEW
                        ? LinkInterfaceFor::Link
                        : LinkInterfaceFor::Usage);

  processOptions(this, entries, result, uniqueOptions, false, "link depends",
                 OptionsParse::None);

  return result;
}

void cmGeneratorTarget::ComputeTargetManifest(const std::string& config) const
{
  if (this->IsImported()) {
    return;
  }
  cmGlobalGenerator* gg = this->LocalGenerator->GetGlobalGenerator();

  // Get the names.
  cmGeneratorTarget::Names targetNames;
  if (this->GetType() == cmStateEnums::EXECUTABLE) {
    targetNames = this->GetExecutableNames(config);
  } else if (this->GetType() == cmStateEnums::STATIC_LIBRARY ||
             this->GetType() == cmStateEnums::SHARED_LIBRARY ||
             this->GetType() == cmStateEnums::MODULE_LIBRARY) {
    targetNames = this->GetLibraryNames(config);
  } else {
    return;
  }

  // Get the directory.
  std::string dir =
    this->GetDirectory(config, cmStateEnums::RuntimeBinaryArtifact);

  // Add each name.
  std::string f;
  if (!targetNames.Output.empty()) {
    f = cmStrCat(dir, '/', targetNames.Output);
    gg->AddToManifest(f);
  }
  if (!targetNames.SharedObject.empty()) {
    f = cmStrCat(dir, '/', targetNames.SharedObject);
    gg->AddToManifest(f);
  }
  if (!targetNames.Real.empty()) {
    f = cmStrCat(dir, '/', targetNames.Real);
    gg->AddToManifest(f);
  }
  if (!targetNames.PDB.empty()) {
    f = cmStrCat(dir, '/', targetNames.PDB);
    gg->AddToManifest(f);
  }

  dir = this->GetDirectory(config, cmStateEnums::ImportLibraryArtifact);
  if (!targetNames.ImportOutput.empty()) {
    f = cmStrCat(dir, '/', targetNames.ImportOutput);
    gg->AddToManifest(f);
  }
  if (!targetNames.ImportLibrary.empty()) {
    f = cmStrCat(dir, '/', targetNames.ImportLibrary);
    gg->AddToManifest(f);
  }
  if (!targetNames.ImportReal.empty()) {
    f = cmStrCat(dir, '/', targetNames.ImportReal);
    gg->AddToManifest(f);
  }
}

cm::optional<cmStandardLevel> cmGeneratorTarget::GetExplicitStandardLevel(
  std::string const& lang, std::string const& config) const
{
  cm::optional<cmStandardLevel> level;
  std::string key = cmStrCat(cmSystemTools::UpperCase(config), '-', lang);
  auto i = this->ExplicitStandardLevel.find(key);
  if (i != this->ExplicitStandardLevel.end()) {
    level = i->second;
  }
  return level;
}

void cmGeneratorTarget::UpdateExplicitStandardLevel(std::string const& lang,
                                                    std::string const& config,
                                                    cmStandardLevel level)
{
  auto e = this->ExplicitStandardLevel.emplace(
    cmStrCat(cmSystemTools::UpperCase(config), '-', lang), level);
  if (!e.second && e.first->second < level) {
    e.first->second = level;
  }
}

bool cmGeneratorTarget::ComputeCompileFeatures(std::string const& config)
{
  cmStandardLevelResolver standardResolver(this->Makefile);

  for (std::string const& lang :
       this->Makefile->GetState()->GetEnabledLanguages()) {
    if (cmValue languageStd = this->GetLanguageStandard(lang, config)) {
      if (cm::optional<cmStandardLevel> langLevel =
            standardResolver.LanguageStandardLevel(lang, *languageStd)) {
        this->UpdateExplicitStandardLevel(lang, config, *langLevel);
      }
    }
  }

  // Compute the language standard based on the compile features.
  std::vector<BT<std::string>> features = this->GetCompileFeatures(config);
  for (BT<std::string> const& f : features) {
    std::string lang;
    if (!standardResolver.CompileFeatureKnown(this->Target->GetName(), f.Value,
                                              lang, nullptr)) {
      return false;
    }

    std::string key = cmStrCat(cmSystemTools::UpperCase(config), '-', lang);
    cmValue currentLanguageStandard = this->GetLanguageStandard(lang, config);

    cm::optional<cmStandardLevel> featureLevel;
    std::string newRequiredStandard;
    if (!standardResolver.GetNewRequiredStandard(
          this->Target->GetName(), f.Value, currentLanguageStandard,
          featureLevel, newRequiredStandard)) {
      return false;
    }

    if (featureLevel) {
      this->UpdateExplicitStandardLevel(lang, config, *featureLevel);
    }

    if (!newRequiredStandard.empty()) {
      BTs<std::string>& languageStandardProperty =
        this->LanguageStandardMap[key];
      if (languageStandardProperty.Value != newRequiredStandard) {
        languageStandardProperty.Value = newRequiredStandard;
        languageStandardProperty.Backtraces.clear();
      }
      languageStandardProperty.Backtraces.emplace_back(f.Backtrace);
    }
  }

  return true;
}

bool cmGeneratorTarget::ComputeCompileFeatures(
  std::string const& config, std::set<LanguagePair> const& languagePairs)
{
  for (const auto& language : languagePairs) {
    BTs<std::string> const* generatorTargetLanguageStandard =
      this->GetLanguageStandardProperty(language.first, config);
    if (!generatorTargetLanguageStandard) {
      // If the standard isn't explicitly set we copy it over from the
      // specified paired language.
      std::string key =
        cmStrCat(cmSystemTools::UpperCase(config), '-', language.first);
      BTs<std::string> const* standardToCopy =
        this->GetLanguageStandardProperty(language.second, config);
      if (standardToCopy) {
        this->LanguageStandardMap[key] = *standardToCopy;
        generatorTargetLanguageStandard = &this->LanguageStandardMap[key];
      } else {
        cmValue defaultStandard = this->Makefile->GetDefinition(
          cmStrCat("CMAKE_", language.second, "_STANDARD_DEFAULT"));
        if (defaultStandard) {
          this->LanguageStandardMap[key] = BTs<std::string>(*defaultStandard);
          generatorTargetLanguageStandard = &this->LanguageStandardMap[key];
        }
      }

      // Custom updates for the CUDA standard.
      if (generatorTargetLanguageStandard != nullptr &&
          (language.first == "CUDA")) {
        if (generatorTargetLanguageStandard->Value == "98") {
          this->LanguageStandardMap[key].Value = "03";
        }
      }
    }
  }

  return true;
}

std::string cmGeneratorTarget::GetImportedLibName(
  std::string const& config) const
{
  if (cmGeneratorTarget::ImportInfo const* info =
        this->GetImportInfo(config)) {
    return info->LibName;
  }
  return std::string();
}

std::string cmGeneratorTarget::GetFullPath(const std::string& config,
                                           cmStateEnums::ArtifactType artifact,
                                           bool realname) const
{
  if (this->IsImported()) {
    return this->Target->ImportedGetFullPath(config, artifact);
  }
  return this->NormalGetFullPath(config, artifact, realname);
}

std::string cmGeneratorTarget::NormalGetFullPath(
  const std::string& config, cmStateEnums::ArtifactType artifact,
  bool realname) const
{
  std::string fpath = cmStrCat(this->GetDirectory(config, artifact), '/');
  if (this->IsAppBundleOnApple()) {
    fpath =
      cmStrCat(this->BuildBundleDirectory(fpath, config, FullLevel), '/');
  }

  // Add the full name of the target.
  switch (artifact) {
    case cmStateEnums::RuntimeBinaryArtifact:
      if (realname) {
        fpath += this->NormalGetRealName(config);
      } else {
        fpath +=
          this->GetFullName(config, cmStateEnums::RuntimeBinaryArtifact);
      }
      break;
    case cmStateEnums::ImportLibraryArtifact:
      if (realname) {
        fpath +=
          this->NormalGetRealName(config, cmStateEnums::ImportLibraryArtifact);
      } else {
        fpath +=
          this->GetFullName(config, cmStateEnums::ImportLibraryArtifact);
      }
      break;
  }
  return fpath;
}

std::string cmGeneratorTarget::NormalGetRealName(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if (this->IsImported()) {
    std::string msg = cmStrCat("NormalGetRealName called on imported target: ",
                               this->GetName());
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
  }

  Names names = this->GetType() == cmStateEnums::EXECUTABLE
    ? this->GetExecutableNames(config)
    : this->GetLibraryNames(config);

  // Compute the real name that will be built.
  return artifact == cmStateEnums::RuntimeBinaryArtifact ? names.Real
                                                         : names.ImportReal;
}

cmGeneratorTarget::Names cmGeneratorTarget::GetLibraryNames(
  const std::string& config) const
{
  cmGeneratorTarget::Names targetNames;

  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if (this->IsImported()) {
    std::string msg =
      cmStrCat("GetLibraryNames called on imported target: ", this->GetName());
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
  }

  // Check for library version properties.
  cmValue version = this->GetProperty("VERSION");
  cmValue soversion = this->GetProperty("SOVERSION");
  if (!this->HasSOName(config) ||
      this->Makefile->IsOn("CMAKE_PLATFORM_NO_VERSIONED_SONAME") ||
      this->IsFrameworkOnApple()) {
    // Versioning is supported only for shared libraries and modules,
    // and then only when the platform supports an soname flag.
    version = nullptr;
    soversion = nullptr;
  }
  if (version && !soversion) {
    // The soversion must be set if the library version is set.  Use
    // the library version as the soversion.
    soversion = version;
  }
  if (!version && soversion) {
    // Use the soversion as the library version.
    version = soversion;
  }

  // Get the components of the library name.
  NameComponents const& components = this->GetFullNameInternalComponents(
    config, cmStateEnums::RuntimeBinaryArtifact);

  // The library name.
  targetNames.Base = components.base;
  targetNames.Output =
    cmStrCat(components.prefix, targetNames.Base, components.suffix);

  if (this->IsFrameworkOnApple()) {
    targetNames.Real = components.prefix;
    if (!this->Makefile->PlatformIsAppleEmbedded()) {
      targetNames.Real +=
        cmStrCat("Versions/", this->GetFrameworkVersion(), '/');
    }
    targetNames.Real += cmStrCat(targetNames.Base, components.suffix);
    targetNames.SharedObject = targetNames.Real;
  } else {
    // The library's soname.
    this->ComputeVersionedName(targetNames.SharedObject, components.prefix,
                               targetNames.Base, components.suffix,
                               targetNames.Output, soversion);

    // The library's real name on disk.
    this->ComputeVersionedName(targetNames.Real, components.prefix,
                               targetNames.Base, components.suffix,
                               targetNames.Output, version);
  }

  // The import library names.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GetType() == cmStateEnums::MODULE_LIBRARY) {
    NameComponents const& importComponents =
      this->GetFullNameInternalComponents(config,
                                          cmStateEnums::ImportLibraryArtifact);
    targetNames.ImportOutput = cmStrCat(
      importComponents.prefix, importComponents.base, importComponents.suffix);

    if (this->IsFrameworkOnApple() && this->IsSharedLibraryWithExports()) {
      targetNames.ImportReal = components.prefix;
      if (!this->Makefile->PlatformIsAppleEmbedded()) {
        targetNames.ImportReal +=
          cmStrCat("Versions/", this->GetFrameworkVersion(), '/');
      }
      targetNames.ImportReal +=
        cmStrCat(importComponents.base, importComponents.suffix);
      targetNames.ImportLibrary = targetNames.ImportOutput;
    } else {
      // The import library's soname.
      this->ComputeVersionedName(
        targetNames.ImportLibrary, importComponents.prefix,
        importComponents.base, importComponents.suffix,
        targetNames.ImportOutput, soversion);

      // The import library's real name on disk.
      this->ComputeVersionedName(
        targetNames.ImportReal, importComponents.prefix, importComponents.base,
        importComponents.suffix, targetNames.ImportOutput, version);
    }
  }

  // The program database file name.
  targetNames.PDB = this->GetPDBName(config);

  return targetNames;
}

cmGeneratorTarget::Names cmGeneratorTarget::GetExecutableNames(
  const std::string& config) const
{
  cmGeneratorTarget::Names targetNames;

  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if (this->IsImported()) {
    std::string msg = cmStrCat(
      "GetExecutableNames called on imported target: ", this->GetName());
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
  }

// This versioning is supported only for executables and then only
// when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  cmValue version;
#else
  // Check for executable version properties.
  cmValue version = this->GetProperty("VERSION");
  if (this->GetType() != cmStateEnums::EXECUTABLE ||
      this->Makefile->IsOn("XCODE")) {
    version = nullptr;
  }
#endif

  // Get the components of the executable name.
  NameComponents const& components = this->GetFullNameInternalComponents(
    config, cmStateEnums::RuntimeBinaryArtifact);

  // The executable name.
  targetNames.Base = components.base;
  targetNames.Output =
    components.prefix + targetNames.Base + components.suffix;

// The executable's real name on disk.
#if defined(__CYGWIN__)
  targetNames.Real = components.prefix + targetNames.Base;
#else
  targetNames.Real = targetNames.Output;
#endif
  if (version) {
    targetNames.Real += "-";
    targetNames.Real += *version;
  }
#if defined(__CYGWIN__)
  targetNames.Real += components.suffix;
#endif

  // The import library name.
  targetNames.ImportLibrary =
    this->GetFullNameInternal(config, cmStateEnums::ImportLibraryArtifact);
  targetNames.ImportReal = targetNames.ImportLibrary;
  targetNames.ImportOutput = targetNames.ImportLibrary;

  // The program database file name.
  targetNames.PDB = this->GetPDBName(config);

  return targetNames;
}

std::string cmGeneratorTarget::GetFullNameInternal(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  NameComponents const& components =
    this->GetFullNameInternalComponents(config, artifact);
  return components.prefix + components.base + components.suffix;
}

std::string cmGeneratorTarget::ImportedGetLocation(
  const std::string& config) const
{
  assert(this->IsImported());
  return this->Target->ImportedGetFullPath(
    config, cmStateEnums::RuntimeBinaryArtifact);
}

std::string cmGeneratorTarget::GetFullNameImported(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  return cmSystemTools::GetFilenameName(
    this->Target->ImportedGetFullPath(config, artifact));
}

cmGeneratorTarget::NameComponents const&
cmGeneratorTarget::GetFullNameInternalComponents(
  std::string const& config, cmStateEnums::ArtifactType artifact) const
{
  assert(artifact == cmStateEnums::RuntimeBinaryArtifact ||
         artifact == cmStateEnums::ImportLibraryArtifact);
  FullNameCache& cache = artifact == cmStateEnums::RuntimeBinaryArtifact
    ? RuntimeBinaryFullNameCache
    : ImportLibraryFullNameCache;
  auto search = cache.find(config);
  if (search != cache.end()) {
    return search->second;
  }
  // Use just the target name for non-main target types.
  if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    NameComponents components;
    components.base = this->GetName();
    return cache.emplace(config, std::move(components)).first->second;
  }

  const bool isImportedLibraryArtifact =
    (artifact == cmStateEnums::ImportLibraryArtifact);

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if (isImportedLibraryArtifact && !this->NeedImportLibraryName(config)) {
    return cache.emplace(config, NameComponents()).first->second;
  }

  NameComponents parts;
  std::string& outPrefix = parts.prefix;
  std::string& outBase = parts.base;
  std::string& outSuffix = parts.suffix;

  // retrieve prefix and suffix
  std::string ll = this->GetLinkerLanguage(config);
  cmValue targetPrefix = this->GetFilePrefixInternal(config, artifact, ll);
  cmValue targetSuffix = this->GetFileSuffixInternal(config, artifact, ll);

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    artifact = cmStateEnums::RuntimeBinaryArtifact;
  }

  // Compute the full name for main target types.
  std::string configPostfix = this->GetFilePostfix(config);

  // frameworks have directory prefix
  std::string fw_prefix;
  if (this->IsFrameworkOnApple()) {
    fw_prefix =
      cmStrCat(this->GetFrameworkDirectory(config, ContentLevel), '/');
    targetPrefix = cmValue(fw_prefix);
    if (!isImportedLibraryArtifact) {
      // no suffix
      targetSuffix = nullptr;
    }
  }

  if (this->IsCFBundleOnApple()) {
    fw_prefix = cmStrCat(this->GetCFBundleDirectory(config, FullLevel), '/');
    targetPrefix = cmValue(fw_prefix);
    targetSuffix = nullptr;
  }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix ? *targetPrefix : "";

  // Append the target name or property-specified name.
  outBase += this->GetOutputName(config, artifact);

  // Append the per-configuration postfix.
  // When using Xcode, the postfix should be part of the suffix rather than
  // the base, because the suffix ends up being used in Xcode's
  // EXECUTABLE_SUFFIX attribute.
  if (this->IsFrameworkOnApple() && this->GetGlobalGenerator()->IsXcode()) {
    configPostfix += *targetSuffix;
    targetSuffix = cmValue(configPostfix);
  } else {
    outBase += configPostfix;
  }

  // Name shared libraries with their version number on some platforms.
  if (cmValue soversion = this->GetProperty("SOVERSION")) {
    cmValue dllProp;
    if (this->IsDLLPlatform()) {
      dllProp = this->GetProperty("DLL_NAME_WITH_SOVERSION");
    }
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY &&
        !isImportedLibraryArtifact &&
        (dllProp.IsOn() ||
         (!dllProp.IsSet() &&
          this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION")))) {
      outBase += "-";
      outBase += *soversion;
    }
  }

  // Append the suffix.
  outSuffix = targetSuffix ? *targetSuffix : "";

  return cache.emplace(config, std::move(parts)).first->second;
}

std::string cmGeneratorTarget::GetLinkerLanguage(
  const std::string& config) const
{
  return this->GetLinkClosure(config)->LinkerLanguage;
}

std::string cmGeneratorTarget::GetLinkerTool(const std::string& config) const
{
  return this->GetLinkerTool(this->GetLinkerLanguage(config), config);
}

std::string cmGeneratorTarget::GetLinkerTool(const std::string& lang,
                                             const std::string& config) const
{
  auto usingLinker =
    cmStrCat("CMAKE_", lang, "_USING_", this->IsDeviceLink() ? "DEVICE_" : "",
             "LINKER_");
  auto format = this->Makefile->GetDefinition(cmStrCat(usingLinker, "MODE"));
  if (!format || format != "TOOL"_s) {
    return this->Makefile->GetDefinition("CMAKE_LINKER");
  }

  auto linkerType = this->GetLinkerTypeProperty(lang, config);
  if (linkerType.empty()) {
    linkerType = "DEFAULT";
  }
  usingLinker = cmStrCat(usingLinker, linkerType);
  auto linkerTool = this->Makefile->GetDefinition(usingLinker);

  if (!linkerTool) {
    if (this->GetGlobalGenerator()->IsVisualStudio() &&
        linkerType == "DEFAULT"_s) {
      return std::string{};
    }

    // fall-back to generic definition
    linkerTool = this->Makefile->GetDefinition("CMAKE_LINKER");

    if (linkerType != "DEFAULT"_s) {
      auto isCMakeLinkerType = [](const std::string& type) -> bool {
        return std::all_of(type.cbegin(), type.cend(),
                           [](char c) { return std::isupper(c); });
      };
      if (isCMakeLinkerType(linkerType)) {
        this->LocalGenerator->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("LINKER_TYPE '", linkerType,
                   "' is unknown or not supported by this toolchain."));
      } else {
        this->LocalGenerator->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("LINKER_TYPE '", linkerType,
                   "' is unknown. Did you forget to define the '", usingLinker,
                   "' variable?"));
      }
    }
  }

  return linkerTool;
}

bool cmGeneratorTarget::LinkerEnforcesNoAllowShLibUndefined(
  std::string const& config) const
{
  // FIXME(#25486): Account for the LINKER_TYPE target property.
  // Also factor out the hard-coded list below into a platform
  // information table based on the linker id.
  std::string ll = this->GetLinkerLanguage(config);
  std::string linkerIdVar = cmStrCat("CMAKE_", ll, "_COMPILER_LINKER_ID");
  cmValue linkerId = this->Makefile->GetDefinition(linkerIdVar);
  // The GNU bfd-based linker may enforce '--no-allow-shlib-undefined'
  // recursively by default.  The Solaris linker has similar behavior.
  return linkerId && (*linkerId == "GNU" || *linkerId == "Solaris");
}

std::string cmGeneratorTarget::GetPDBOutputName(
  const std::string& config) const
{
  std::string base =
    this->GetOutputName(config, cmStateEnums::RuntimeBinaryArtifact);

  std::vector<std::string> props;
  std::string configUpper = cmSystemTools::UpperCase(config);
  if (!configUpper.empty()) {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
  }

  // PDB_NAME
  props.emplace_back("PDB_NAME");

  for (std::string const& p : props) {
    if (cmValue outName = this->GetProperty(p)) {
      base = *outName;
      break;
    }
  }
  return base;
}

std::string cmGeneratorTarget::GetPDBName(const std::string& config) const
{
  NameComponents const& parts = this->GetFullNameInternalComponents(
    config, cmStateEnums::RuntimeBinaryArtifact);

  std::vector<std::string> props;
  std::string configUpper = cmSystemTools::UpperCase(config);
  if (!configUpper.empty()) {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
  }

  // PDB_NAME
  props.emplace_back("PDB_NAME");

  for (std::string const& p : props) {
    if (cmValue outName = this->GetProperty(p)) {
      return parts.prefix + *outName + ".pdb";
    }
  }
  return parts.prefix + parts.base + ".pdb";
}

std::string cmGeneratorTarget::GetObjectDirectory(
  std::string const& config) const
{
  std::string obj_dir =
    this->GlobalGenerator->ExpandCFGIntDir(this->ObjectDirectory, config);
#if defined(__APPLE__)
  // Replace Xcode's placeholder for the object file directory since
  // installation and export scripts need to know the real directory.
  // Xcode has build-time settings (e.g. for sanitizers) that affect this,
  // but we use the default here.  Users that want to enable sanitizers
  // will do so at the cost of object library installation and export.
  cmSystemTools::ReplaceString(obj_dir, "$(OBJECT_FILE_DIR_normal:base)",
                               "Objects-normal");
#endif
  return obj_dir;
}

void cmGeneratorTarget::GetTargetObjectNames(
  std::string const& config, std::vector<std::string>& objects) const
{
  std::vector<cmSourceFile const*> objectSources;
  this->GetObjectSources(objectSources, config);
  std::map<cmSourceFile const*, std::string> mapping;

  for (cmSourceFile const* sf : objectSources) {
    mapping[sf];
  }

  this->LocalGenerator->ComputeObjectFilenames(mapping, this);

  for (cmSourceFile const* src : objectSources) {
    // Find the object file name corresponding to this source file.
    auto map_it = mapping.find(src);
    // It must exist because we populated the mapping just above.
    assert(!map_it->second.empty());
    objects.push_back(map_it->second);
  }

  // We need to compute the relative path from the root of
  // of the object directory to handle subdirectory paths
  std::string rootObjectDir = this->GetObjectDirectory(config);
  rootObjectDir = cmSystemTools::CollapseFullPath(rootObjectDir);
  auto ispcObjects = this->GetGeneratedISPCObjects(config);
  for (std::string const& output : ispcObjects) {
    auto relativePathFromObjectDir = output.substr(rootObjectDir.size());
    objects.push_back(relativePathFromObjectDir);
  }
}

bool cmGeneratorTarget::StrictTargetComparison::operator()(
  cmGeneratorTarget const* t1, cmGeneratorTarget const* t2) const
{
  int nameResult = strcmp(t1->GetName().c_str(), t2->GetName().c_str());
  if (nameResult == 0) {
    return strcmp(
             t1->GetLocalGenerator()->GetCurrentBinaryDirectory().c_str(),
             t2->GetLocalGenerator()->GetCurrentBinaryDirectory().c_str()) < 0;
  }
  return nameResult < 0;
}

struct cmGeneratorTarget::SourceFileFlags
cmGeneratorTarget::GetTargetSourceFileFlags(const cmSourceFile* sf) const
{
  struct SourceFileFlags flags;
  this->ConstructSourceFileFlags();
  auto si = this->SourceFlagsMap.find(sf);
  if (si != this->SourceFlagsMap.end()) {
    flags = si->second;
  } else {
    // Handle the MACOSX_PACKAGE_LOCATION property on source files that
    // were not listed in one of the other lists.
    if (cmValue location = sf->GetProperty("MACOSX_PACKAGE_LOCATION")) {
      flags.MacFolder = location->c_str();
      const bool stripResources =
        this->GlobalGenerator->ShouldStripResourcePath(this->Makefile);
      if (*location == "Resources") {
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        if (stripResources) {
          flags.MacFolder = "";
        }
      } else if (cmHasLiteralPrefix(*location, "Resources/")) {
        flags.Type = cmGeneratorTarget::SourceFileTypeDeepResource;
        if (stripResources) {
          flags.MacFolder += cmStrLen("Resources/");
        }
      } else {
        flags.Type = cmGeneratorTarget::SourceFileTypeMacContent;
      }
    }
  }
  return flags;
}

void cmGeneratorTarget::ConstructSourceFileFlags() const
{
  if (this->SourceFileFlagsConstructed) {
    return;
  }
  this->SourceFileFlagsConstructed = true;

  // Process public headers to mark the source files.
  if (cmValue files = this->GetProperty("PUBLIC_HEADER")) {
    cmList relFiles{ *files };
    for (auto const& relFile : relFiles) {
      if (cmSourceFile* sf = this->Makefile->GetSource(relFile)) {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmGeneratorTarget::SourceFileTypePublicHeader;
      }
    }
  }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if (cmValue files = this->GetProperty("PRIVATE_HEADER")) {
    cmList relFiles{ *files };
    for (auto const& relFile : relFiles) {
      if (cmSourceFile* sf = this->Makefile->GetSource(relFile)) {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmGeneratorTarget::SourceFileTypePrivateHeader;
      }
    }
  }

  // Mark sources listed as resources.
  if (cmValue files = this->GetProperty("RESOURCE")) {
    cmList relFiles{ *files };
    for (auto const& relFile : relFiles) {
      if (cmSourceFile* sf = this->Makefile->GetSource(relFile)) {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "";
        if (!this->GlobalGenerator->ShouldStripResourcePath(this->Makefile)) {
          flags.MacFolder = "Resources";
        }
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
      }
    }
  }
}

const cmGeneratorTarget::CompatibleInterfacesBase&
cmGeneratorTarget::GetCompatibleInterfaces(std::string const& config) const
{
  cmGeneratorTarget::CompatibleInterfaces& compat =
    this->CompatibleInterfacesMap[config];
  if (!compat.Done) {
    compat.Done = true;
    compat.PropsBool.insert("POSITION_INDEPENDENT_CODE");
    compat.PropsString.insert("AUTOUIC_OPTIONS");
    std::vector<cmGeneratorTarget const*> const& deps =
      this->GetLinkImplementationClosure(config);
    for (cmGeneratorTarget const* li : deps) {
#define CM_READ_COMPATIBLE_INTERFACE(X, x)                                    \
  if (cmValue prop = li->GetProperty("COMPATIBLE_INTERFACE_" #X)) {           \
    cmList props(*prop);                                                      \
    compat.Props##x.insert(props.begin(), props.end());                       \
  }
      CM_READ_COMPATIBLE_INTERFACE(BOOL, Bool)
      CM_READ_COMPATIBLE_INTERFACE(STRING, String)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MIN, NumberMin)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MAX, NumberMax)
#undef CM_READ_COMPATIBLE_INTERFACE
    }
  }
  return compat;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentBoolProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsBool.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentStringProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsString.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMinProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsNumberMin.count(p) > 0;
}

bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMaxProperty(
  const std::string& p, const std::string& config) const
{
  if (this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return false;
  }
  return this->GetCompatibleInterfaces(config).PropsNumberMax.count(p) > 0;
}

enum CompatibleType
{
  BoolType,
  StringType,
  NumberMinType,
  NumberMaxType
};

template <typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string& prop,
                                               const std::string& config,
                                               CompatibleType, PropertyType*);

template <>
bool getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                       const std::string& prop,
                                       const std::string& config,
                                       CompatibleType /*unused*/,
                                       bool* /*unused*/)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template <>
const char* getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                              const std::string& prop,
                                              const std::string& config,
                                              CompatibleType t,
                                              const char** /*unused*/)
{
  switch (t) {
    case BoolType:
      assert(false &&
             "String compatibility check function called for boolean");
      return nullptr;
    case StringType:
      return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
    case NumberMinType:
      return tgt->GetLinkInterfaceDependentNumberMinProperty(prop, config);
    case NumberMaxType:
      return tgt->GetLinkInterfaceDependentNumberMaxProperty(prop, config);
  }
  assert(false && "Unreachable!");
  return nullptr;
}

template <typename PropertyType>
void checkPropertyConsistency(cmGeneratorTarget const* depender,
                              cmGeneratorTarget const* dependee,
                              const std::string& propName,
                              std::set<std::string>& emitted,
                              const std::string& config, CompatibleType t,
                              PropertyType* /*unused*/)
{
  cmValue prop = dependee->GetProperty(propName);
  if (!prop) {
    return;
  }

  cmList props{ *prop };
  std::string pdir =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Help/prop_tgt/");

  for (std::string const& p : props) {
    std::string pname = cmSystemTools::HelpFileName(p);
    std::string pfile = pdir + pname + ".rst";
    if (cmSystemTools::FileExists(pfile, true)) {
      std::ostringstream e;
      e << "Target \"" << dependee->GetName() << "\" has property \"" << p
        << "\" listed in its " << propName
        << " property.  "
           "This is not allowed.  Only user-defined properties may appear "
           "listed in the "
        << propName << " property.";
      depender->GetLocalGenerator()->IssueMessage(MessageType::FATAL_ERROR,
                                                  e.str());
      return;
    }
    if (emitted.insert(p).second) {
      getLinkInterfaceDependentProperty<PropertyType>(depender, p, config, t,
                                                      nullptr);
      if (cmSystemTools::GetErrorOccurredFlag()) {
        return;
      }
    }
  }
}

namespace {
std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2)
{
  std::set<std::string> intersect;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(intersect, intersect.begin()));
  if (!intersect.empty()) {
    return *intersect.begin();
  }
  return "";
}

std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2,
                      const std::set<std::string>& s3)
{
  std::string result;
  result = intersect(s1, s2);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s3);
  if (!result.empty()) {
    return result;
  }
  return intersect(s2, s3);
}

std::string intersect(const std::set<std::string>& s1,
                      const std::set<std::string>& s2,
                      const std::set<std::string>& s3,
                      const std::set<std::string>& s4)
{
  std::string result;
  result = intersect(s1, s2);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s3);
  if (!result.empty()) {
    return result;
  }
  result = intersect(s1, s4);
  if (!result.empty()) {
    return result;
  }
  return intersect(s2, s3, s4);
}
}

void cmGeneratorTarget::CheckPropertyCompatibility(
  cmComputeLinkInformation& info, const std::string& config) const
{
  const cmComputeLinkInformation::ItemVector& deps = info.GetItems();

  std::set<std::string> emittedBools;
  static const std::string strBool = "COMPATIBLE_INTERFACE_BOOL";
  std::set<std::string> emittedStrings;
  static const std::string strString = "COMPATIBLE_INTERFACE_STRING";
  std::set<std::string> emittedMinNumbers;
  static const std::string strNumMin = "COMPATIBLE_INTERFACE_NUMBER_MIN";
  std::set<std::string> emittedMaxNumbers;
  static const std::string strNumMax = "COMPATIBLE_INTERFACE_NUMBER_MAX";

  for (auto const& dep : deps) {
    if (!dep.Target || dep.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      continue;
    }

    checkPropertyConsistency<bool>(this, dep.Target, strBool, emittedBools,
                                   config, BoolType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strString,
                                          emittedStrings, config, StringType,
                                          nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMin,
                                          emittedMinNumbers, config,
                                          NumberMinType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMax,
                                          emittedMaxNumbers, config,
                                          NumberMaxType, nullptr);
    if (cmSystemTools::GetErrorOccurredFlag()) {
      return;
    }
  }

  std::string prop = intersect(emittedBools, emittedStrings, emittedMinNumbers,
                               emittedMaxNumbers);

  if (!prop.empty()) {
    // Use a sorted std::vector to keep the error message sorted.
    std::vector<std::string> props;
    auto i = emittedBools.find(prop);
    if (i != emittedBools.end()) {
      props.push_back(strBool);
    }
    i = emittedStrings.find(prop);
    if (i != emittedStrings.end()) {
      props.push_back(strString);
    }
    i = emittedMinNumbers.find(prop);
    if (i != emittedMinNumbers.end()) {
      props.push_back(strNumMin);
    }
    i = emittedMaxNumbers.find(prop);
    if (i != emittedMaxNumbers.end()) {
      props.push_back(strNumMax);
    }
    std::sort(props.begin(), props.end());

    std::string propsString = cmStrCat(
      cmJoin(cmMakeRange(props).retreat(1), ", "), " and the ", props.back());

    std::ostringstream e;
    e << "Property \"" << prop << "\" appears in both the " << propsString
      << " property in the dependencies of target \"" << this->GetName()
      << "\".  This is not allowed. A property may only require "
         "compatibility "
         "in a boolean interpretation, a numeric minimum, a numeric maximum "
         "or a "
         "string interpretation, but not a mixture.";
    this->LocalGenerator->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
}

template <typename PropertyType>
std::string valueAsString(PropertyType);
template <>
std::string valueAsString<bool>(bool value)
{
  return value ? "TRUE" : "FALSE";
}
template <>
std::string valueAsString<const char*>(const char* value)
{
  return value ? value : "(unset)";
}
template <>
std::string valueAsString<std::string>(std::string value)
{
  return value;
}
template <>
std::string valueAsString<cmValue>(cmValue value)
{
  return value ? *value : std::string("(unset)");
}
template <>
std::string valueAsString<std::nullptr_t>(std::nullptr_t /*unused*/)
{
  return "(unset)";
}

static std::string compatibilityType(CompatibleType t)
{
  switch (t) {
    case BoolType:
      return "Boolean compatibility";
    case StringType:
      return "String compatibility";
    case NumberMaxType:
      return "Numeric maximum compatibility";
    case NumberMinType:
      return "Numeric minimum compatibility";
  }
  assert(false && "Unreachable!");
  return "";
}

static std::string compatibilityAgree(CompatibleType t, bool dominant)
{
  switch (t) {
    case BoolType:
    case StringType:
      return dominant ? "(Disagree)\n" : "(Agree)\n";
    case NumberMaxType:
    case NumberMinType:
      return dominant ? "(Dominant)\n" : "(Ignored)\n";
  }
  assert(false && "Unreachable!");
  return "";
}

template <typename PropertyType>
PropertyType getTypedProperty(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter = nullptr);

template <>
bool getTypedProperty<bool>(cmGeneratorTarget const* tgt,
                            const std::string& prop,
                            cmGeneratorExpressionInterpreter* genexInterpreter)
{
  if (genexInterpreter == nullptr) {
    return tgt->GetPropertyAsBool(prop);
  }

  cmValue value = tgt->GetProperty(prop);
  return cmIsOn(genexInterpreter->Evaluate(value ? *value : "", prop));
}

template <>
const char* getTypedProperty<const char*>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  cmValue value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return value.GetCStr();
  }

  return genexInterpreter->Evaluate(value ? *value : "", prop).c_str();
}

template <>
std::string getTypedProperty<std::string>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  cmValue value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return valueAsString(value);
  }

  return genexInterpreter->Evaluate(value ? *value : "", prop);
}

template <typename PropertyType>
PropertyType impliedValue(PropertyType);
template <>
bool impliedValue<bool>(bool /*unused*/)
{
  return false;
}
template <>
const char* impliedValue<const char*>(const char* /*unused*/)
{
  return "";
}
template <>
std::string impliedValue<std::string>(std::string /*unused*/) // NOLINT(*)
{
  return std::string();
}

template <typename PropertyType>
std::pair<bool, PropertyType> consistentProperty(PropertyType lhs,
                                                 PropertyType rhs,
                                                 CompatibleType t);

template <>
std::pair<bool, bool> consistentProperty(bool lhs, bool rhs,
                                         CompatibleType /*unused*/)
{
  return { lhs == rhs, lhs };
}

static std::pair<bool, const char*> consistentStringProperty(const char* lhs,
                                                             const char* rhs)
{
  const bool b = strcmp(lhs, rhs) == 0;
  return { b, b ? lhs : nullptr };
}

static std::pair<bool, std::string> consistentStringProperty(
  const std::string& lhs, const std::string& rhs)
{
  const bool b = lhs == rhs;
  return { b, b ? lhs : valueAsString(nullptr) };
}

static std::pair<bool, const char*> consistentNumberProperty(const char* lhs,
                                                             const char* rhs,
                                                             CompatibleType t)
{
  char* pEnd;

  long lnum = strtol(lhs, &pEnd, 0);
  if (pEnd == lhs || *pEnd != '\0' || errno == ERANGE) {
    return { false, nullptr };
  }

  long rnum = strtol(rhs, &pEnd, 0);
  if (pEnd == rhs || *pEnd != '\0' || errno == ERANGE) {
    return { false, nullptr };
  }

  if (t == NumberMaxType) {
    return { true, std::max(lnum, rnum) == lnum ? lhs : rhs };
  }

  return { true, std::min(lnum, rnum) == lnum ? lhs : rhs };
}

template <>
std::pair<bool, const char*> consistentProperty(const char* lhs,
                                                const char* rhs,
                                                CompatibleType t)
{
  if (!lhs && !rhs) {
    return { true, lhs };
  }
  if (!lhs) {
    return { true, rhs };
  }
  if (!rhs) {
    return { true, lhs };
  }

  switch (t) {
    case BoolType: {
      bool same = cmIsOn(lhs) == cmIsOn(rhs);
      return { same, same ? lhs : nullptr };
    }
    case StringType:
      return consistentStringProperty(lhs, rhs);
    case NumberMinType:
    case NumberMaxType:
      return consistentNumberProperty(lhs, rhs, t);
  }
  assert(false && "Unreachable!");
  return { false, nullptr };
}

static std::pair<bool, std::string> consistentProperty(const std::string& lhs,
                                                       const std::string& rhs,
                                                       CompatibleType t)
{
  const std::string null_ptr = valueAsString(nullptr);

  if (lhs == null_ptr && rhs == null_ptr) {
    return { true, lhs };
  }
  if (lhs == null_ptr) {
    return { true, rhs };
  }
  if (rhs == null_ptr) {
    return { true, lhs };
  }

  switch (t) {
    case BoolType: {
      bool same = cmIsOn(lhs) == cmIsOn(rhs);
      return { same, same ? lhs : null_ptr };
    }
    case StringType:
      return consistentStringProperty(lhs, rhs);
    case NumberMinType:
    case NumberMaxType: {
      auto value = consistentNumberProperty(lhs.c_str(), rhs.c_str(), t);
      return { value.first,
               value.first ? std::string(value.second) : null_ptr };
    }
  }
  assert(false && "Unreachable!");
  return { false, null_ptr };
}

template <typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmGeneratorTarget const* tgt,
                                                 const std::string& p,
                                                 const std::string& config,
                                                 const char* defaultValue,
                                                 CompatibleType t,
                                                 PropertyType* /*unused*/)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt, p);

  std::vector<std::string> headPropKeys = tgt->GetPropertyKeys();
  const bool explicitlySet = cm::contains(headPropKeys, p);

  const bool impliedByUse = tgt->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet) || (!impliedByUse && !explicitlySet));

  std::vector<cmGeneratorTarget const*> const& deps =
    tgt->GetLinkImplementationClosure(config);

  if (deps.empty()) {
    return propContent;
  }
  bool propInitialized = explicitlySet;

  std::string report = cmStrCat(" * Target \"", tgt->GetName());
  if (explicitlySet) {
    report += "\" has property content \"";
    report += valueAsString<PropertyType>(propContent);
    report += "\"\n";
  } else if (impliedByUse) {
    report += "\" property is implied by use.\n";
  } else {
    report += "\" property not set.\n";
  }

  std::string interfaceProperty = "INTERFACE_" + p;
  std::unique_ptr<cmGeneratorExpressionInterpreter> genexInterpreter;
  if (p == "POSITION_INDEPENDENT_CODE") {
    genexInterpreter = cm::make_unique<cmGeneratorExpressionInterpreter>(
      tgt->GetLocalGenerator(), config, tgt);
  }

  for (cmGeneratorTarget const* theTarget : deps) {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    std::vector<std::string> propKeys = theTarget->GetPropertyKeys();

    const bool ifaceIsSet = cm::contains(propKeys, interfaceProperty);
    PropertyType ifacePropContent = getTypedProperty<PropertyType>(
      theTarget, interfaceProperty, genexInterpreter.get());

    std::string reportEntry;
    if (ifaceIsSet) {
      reportEntry += " * Target \"";
      reportEntry += theTarget->GetName();
      reportEntry += "\" property value \"";
      reportEntry += valueAsString<PropertyType>(ifacePropContent);
      reportEntry += "\" ";
    }

    if (explicitlySet) {
      if (ifaceIsSet) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "Property " << p << " on target \"" << tgt->GetName()
            << "\" does\nnot match the "
               "INTERFACE_"
            << p
            << " property requirement\nof "
               "dependency \""
            << theTarget->GetName() << "\".\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      // Explicitly set on target and not set in iface. Can't disagree.
      continue;
    }
    if (impliedByUse) {
      propContent = impliedValue<PropertyType>(propContent);

      if (ifaceIsSet) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "Property " << p << " on target \"" << tgt->GetName()
            << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_"
            << p << " property on\ndependency \"" << theTarget->GetName()
            << "\" is in conflict.\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      // Implicitly set on target and not set in iface. Can't disagree.
      continue;
    }
    if (ifaceIsSet) {
      if (propInitialized) {
        std::pair<bool, PropertyType> consistent =
          consistentProperty(propContent, ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first) {
          std::ostringstream e;
          e << "The INTERFACE_" << p << " property of \""
            << theTarget->GetName() << "\" does\nnot agree with the value of "
            << p << " already determined\nfor \"" << tgt->GetName() << "\".\n";
          cmSystemTools::Error(e.str());
          break;
        }
        propContent = consistent.second;
        continue;
      }
      report += reportEntry + "(Interface set)\n";
      propContent = ifacePropContent;
      propInitialized = true;
    } else {
      // Not set. Nothing to agree on.
      continue;
    }
  }

  tgt->ReportPropertyOrigin(p, valueAsString<PropertyType>(propContent),
                            report, compatibilityType(t));
  return propContent;
}

bool cmGeneratorTarget::SetDeviceLink(bool deviceLink)
{
  bool previous = this->DeviceLink;
  this->DeviceLink = deviceLink;
  return previous;
}

bool cmGeneratorTarget::GetLinkInterfaceDependentBoolProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config, "FALSE",
                                                   BoolType, nullptr);
}

std::string cmGeneratorTarget::GetLinkInterfaceDependentStringAsBoolProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<std::string>(
    this, p, config, "FALSE", BoolType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentStringProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", StringType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentNumberMinProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", NumberMinType, nullptr);
}

const char* cmGeneratorTarget::GetLinkInterfaceDependentNumberMaxProperty(
  const std::string& p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char*>(
    this, p, config, "empty", NumberMaxType, nullptr);
}

cmComputeLinkInformation* cmGeneratorTarget::GetLinkInformation(
  const std::string& config) const
{
  // Lookup any existing information for this configuration.
  std::string key(cmSystemTools::UpperCase(config));
  auto i = this->LinkInformation.find(key);
  if (i == this->LinkInformation.end()) {
    // Compute information for this configuration.
    auto info = cm::make_unique<cmComputeLinkInformation>(this, config);
    if (info && !info->Compute()) {
      info.reset();
    }

    // Store the information for this configuration.
    i = this->LinkInformation.emplace(key, std::move(info)).first;

    if (i->second) {
      this->CheckPropertyCompatibility(*i->second, config);
    }
  }
  return i->second.get();
}

void cmGeneratorTarget::CheckLinkLibraries() const
{
  bool linkLibrariesOnlyTargets =
    this->GetPropertyAsBool("LINK_LIBRARIES_ONLY_TARGETS");

  // Evaluate the link interface of this target if needed for extra checks.
  if (linkLibrariesOnlyTargets) {
    std::vector<std::string> const& configs =
      this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
    for (std::string const& config : configs) {
      this->GetLinkInterfaceLibraries(config, this, LinkInterfaceFor::Link);
    }
  }

  // Check link the implementation for each generated configuration.
  for (auto const& hmp : this->LinkImplMap) {
    HeadToLinkImplementationMap const& hm = hmp.second;
    // There could be several entries used when computing the pre-CMP0022
    // default link interface.  Check only the entry for our own link impl.
    auto const hmi = hm.find(this);
    if (hmi == hm.end() || !hmi->second.LibrariesDone) {
      continue;
    }
    for (cmLinkImplItem const& item : hmi->second.Libraries) {
      if (!this->VerifyLinkItemColons(LinkItemRole::Implementation, item)) {
        return;
      }
      if (linkLibrariesOnlyTargets &&
          !this->VerifyLinkItemIsTarget(LinkItemRole::Implementation, item)) {
        return;
      }
    }
  }

  // Check link the interface for each generated combination of
  // configuration and consuming head target.  We should not need to
  // consider LinkInterfaceUsageRequirementsOnlyMap because its entries
  // should be a subset of LinkInterfaceMap (with LINK_ONLY left out).
  for (auto const& hmp : this->LinkInterfaceMap) {
    for (auto const& hmi : hmp.second) {
      if (!hmi.second.LibrariesDone) {
        continue;
      }
      for (cmLinkItem const& item : hmi.second.Libraries) {
        if (!this->VerifyLinkItemColons(LinkItemRole::Interface, item)) {
          return;
        }
        if (linkLibrariesOnlyTargets &&
            !this->VerifyLinkItemIsTarget(LinkItemRole::Interface, item)) {
          return;
        }
      }
    }
  }
}

namespace {
cm::string_view missingTargetPossibleReasons =
  "Possible reasons include:\n"
  "  * There is a typo in the target name.\n"
  "  * A find_package call is missing for an IMPORTED target.\n"
  "  * An ALIAS target is missing.\n"_s;
}

bool cmGeneratorTarget::VerifyLinkItemColons(LinkItemRole role,
                                             cmLinkItem const& item) const
{
  if (item.Target || cmHasPrefix(item.AsStr(), "<LINK_GROUP:"_s) ||
      item.AsStr().find("::") == std::string::npos) {
    return true;
  }
  MessageType messageType = MessageType::FATAL_ERROR;
  std::string e;
  switch (this->GetLocalGenerator()->GetPolicyStatus(cmPolicies::CMP0028)) {
    case cmPolicies::WARN: {
      e = cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0028), "\n");
      messageType = MessageType::AUTHOR_WARNING;
    } break;
    case cmPolicies::OLD:
      return true;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Issue the fatal message.
      break;
  }

  if (role == LinkItemRole::Implementation) {
    e = cmStrCat(e, "Target \"", this->GetName(), "\" links to");
  } else {
    e = cmStrCat(e, "The link interface of target \"", this->GetName(),
                 "\" contains");
  }
  e =
    cmStrCat(e, ":\n  ", item.AsStr(), "\n", "but the target was not found.  ",
             missingTargetPossibleReasons);
  cmListFileBacktrace backtrace = item.Backtrace;
  if (backtrace.Empty()) {
    backtrace = this->GetBacktrace();
  }
  this->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(messageType, e,
                                                              backtrace);
  return false;
}

bool cmGeneratorTarget::VerifyLinkItemIsTarget(LinkItemRole role,
                                               cmLinkItem const& item) const
{
  if (item.Target) {
    return true;
  }
  std::string const& str = item.AsStr();
  if (!str.empty() &&
      (str[0] == '-' || str[0] == '$' || str[0] == '`' ||
       str.find_first_of("/\\") != std::string::npos ||
       cmHasPrefix(str, "<LINK_LIBRARY:"_s) ||
       cmHasPrefix(str, "<LINK_GROUP:"_s))) {
    return true;
  }

  std::string e = cmStrCat("Target \"", this->GetName(),
                           "\" has LINK_LIBRARIES_ONLY_TARGETS enabled, but ",
                           role == LinkItemRole::Implementation
                             ? "it links to"
                             : "its link interface contains",
                           ":\n  ", item.AsStr(), "\nwhich is not a target.  ",
                           missingTargetPossibleReasons);
  cmListFileBacktrace backtrace = item.Backtrace;
  if (backtrace.Empty()) {
    backtrace = this->GetBacktrace();
  }
  this->LocalGenerator->GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR, e, backtrace);
  return false;
}

void cmGeneratorTarget::GetTargetVersion(int& major, int& minor) const
{
  int patch;
  this->GetTargetVersion("VERSION", major, minor, patch);
}

void cmGeneratorTarget::GetTargetVersionFallback(
  const std::string& property, const std::string& fallback_property,
  int& major, int& minor, int& patch) const
{
  if (this->GetProperty(property)) {
    this->GetTargetVersion(property, major, minor, patch);
  } else {
    this->GetTargetVersion(fallback_property, major, minor, patch);
  }
}

void cmGeneratorTarget::GetTargetVersion(const std::string& property,
                                         int& major, int& minor,
                                         int& patch) const
{
  // Set the default values.
  major = 0;
  minor = 0;
  patch = 0;

  assert(this->GetType() != cmStateEnums::INTERFACE_LIBRARY);

  if (cmValue version = this->GetProperty(property)) {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    int parsed_patch;
    switch (sscanf(version->c_str(), "%d.%d.%d", &parsed_major, &parsed_minor,
                   &parsed_patch)) {
      case 3:
        patch = parsed_patch;
        CM_FALLTHROUGH;
      case 2:
        minor = parsed_minor;
        CM_FALLTHROUGH;
      case 1:
        major = parsed_major;
        CM_FALLTHROUGH;
      default:
        break;
    }
  }
}

std::string cmGeneratorTarget::GetRuntimeLinkLibrary(
  std::string const& lang, std::string const& config) const
{
  // This is activated by the presence of a default selection whether or
  // not it is overridden by a property.
  cmValue runtimeLibraryDefault = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_", lang, "_RUNTIME_LIBRARY_DEFAULT"));
  if (!cmNonempty(runtimeLibraryDefault)) {
    return std::string();
  }
  cmValue runtimeLibraryValue =
    this->Target->GetProperty(cmStrCat(lang, "_RUNTIME_LIBRARY"));
  if (!runtimeLibraryValue) {
    runtimeLibraryValue = runtimeLibraryDefault;
  }
  return cmSystemTools::UpperCase(cmGeneratorExpression::Evaluate(
    *runtimeLibraryValue, this->LocalGenerator, config, this));
}

std::string cmGeneratorTarget::GetFortranModuleDirectory(
  std::string const& working_dir) const
{
  if (!this->FortranModuleDirectoryCreated) {
    this->FortranModuleDirectory =
      this->CreateFortranModuleDirectory(working_dir);
    this->FortranModuleDirectoryCreated = true;
  }

  return this->FortranModuleDirectory;
}

bool cmGeneratorTarget::IsFortranBuildingInstrinsicModules() const
{
  if (cmValue prop =
        this->GetProperty("Fortran_BUILDING_INSTRINSIC_MODULES")) {
    return cmIsOn(*prop);
  }
  return false;
}

std::string cmGeneratorTarget::CreateFortranModuleDirectory(
  std::string const& working_dir) const
{
  std::string mod_dir;
  std::string target_mod_dir;
  if (cmValue prop = this->GetProperty("Fortran_MODULE_DIRECTORY")) {
    target_mod_dir = *prop;
  } else {
    std::string const& default_mod_dir =
      this->LocalGenerator->GetCurrentBinaryDirectory();
    if (default_mod_dir != working_dir) {
      target_mod_dir = default_mod_dir;
    }
  }
  cmValue moddir_flag =
    this->Makefile->GetDefinition("CMAKE_Fortran_MODDIR_FLAG");
  if (!target_mod_dir.empty() && moddir_flag) {
    // Compute the full path to the module directory.
    if (cmSystemTools::FileIsFullPath(target_mod_dir)) {
      // Already a full path.
      mod_dir = target_mod_dir;
    } else {
      // Interpret relative to the current output directory.
      mod_dir = cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(),
                         '/', target_mod_dir);
    }

    // Make sure the module output directory exists.
    cmSystemTools::MakeDirectory(mod_dir);
  }
  return mod_dir;
}

void cmGeneratorTarget::AddISPCGeneratedHeader(std::string const& header,
                                               std::string const& config)
{
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto iter = this->ISPCGeneratedHeaders.find(config_upper);
  if (iter == this->ISPCGeneratedHeaders.end()) {
    std::vector<std::string> headers;
    headers.emplace_back(header);
    this->ISPCGeneratedHeaders.insert({ config_upper, headers });
  } else {
    iter->second.emplace_back(header);
  }
}

std::vector<std::string> cmGeneratorTarget::GetGeneratedISPCHeaders(
  std::string const& config) const
{
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto iter = this->ISPCGeneratedHeaders.find(config_upper);
  if (iter == this->ISPCGeneratedHeaders.end()) {
    return std::vector<std::string>{};
  }
  return iter->second;
}

void cmGeneratorTarget::AddISPCGeneratedObject(std::vector<std::string>&& objs,
                                               std::string const& config)
{
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto iter = this->ISPCGeneratedObjects.find(config_upper);
  if (iter == this->ISPCGeneratedObjects.end()) {
    this->ISPCGeneratedObjects.insert({ config_upper, objs });
  } else {
    iter->second.insert(iter->second.end(), objs.begin(), objs.end());
  }
}

std::vector<std::string> cmGeneratorTarget::GetGeneratedISPCObjects(
  std::string const& config) const
{
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto iter = this->ISPCGeneratedObjects.find(config_upper);
  if (iter == this->ISPCGeneratedObjects.end()) {
    return std::vector<std::string>{};
  }
  return iter->second;
}

std::string cmGeneratorTarget::GetFrameworkVersion() const
{
  assert(this->GetType() != cmStateEnums::INTERFACE_LIBRARY);

  if (cmValue fversion = this->GetProperty("FRAMEWORK_VERSION")) {
    return *fversion;
  }
  if (cmValue tversion = this->GetProperty("VERSION")) {
    return *tversion;
  }
  return "A";
}

void cmGeneratorTarget::ComputeVersionedName(
  std::string& vName, std::string const& prefix, std::string const& base,
  std::string const& suffix, std::string const& name, cmValue version) const
{
  vName = this->IsApple() ? (prefix + base) : name;
  if (version) {
    vName += ".";
    vName += *version;
  }
  vName += this->IsApple() ? suffix : std::string();
}

std::vector<std::string> cmGeneratorTarget::GetPropertyKeys() const
{
  return this->Target->GetProperties().GetKeys();
}

void cmGeneratorTarget::ReportPropertyOrigin(
  const std::string& p, const std::string& result, const std::string& report,
  const std::string& compatibilityType) const
{
  cmList debugProperties{ this->Target->GetMakefile()->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES") };
  bool debugOrigin = !this->DebugCompatiblePropertiesDone[p] &&
    cm::contains(debugProperties, p);

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompatiblePropertiesDone[p] = true;
  }
  if (!debugOrigin) {
    return;
  }

  std::string areport =
    cmStrCat(compatibilityType, " of property \"", p, "\" for target \"",
             this->GetName(), "\" (result: \"", result, "\"):\n", report);

  this->LocalGenerator->GetCMakeInstance()->IssueMessage(MessageType::LOG,
                                                         areport);
}

bool cmGeneratorTarget::IsLinkLookupScope(std::string const& n,
                                          cmLocalGenerator const*& lg) const
{
  if (cmHasLiteralPrefix(n, CMAKE_DIRECTORY_ID_SEP)) {
    cmDirectoryId const dirId = n.substr(cmStrLen(CMAKE_DIRECTORY_ID_SEP));
    if (dirId.String.empty()) {
      lg = this->LocalGenerator;
      return true;
    }
    if (cmLocalGenerator const* otherLG =
          this->GlobalGenerator->FindLocalGenerator(dirId)) {
      lg = otherLG;
      return true;
    }
  }
  return false;
}

cm::optional<cmLinkItem> cmGeneratorTarget::LookupLinkItem(
  std::string const& n, cmListFileBacktrace const& bt,
  std::string const& linkFeature, LookupLinkItemScope* scope,
  LookupSelf lookupSelf) const
{
  cm::optional<cmLinkItem> maybeItem;
  if (this->IsLinkLookupScope(n, scope->LG)) {
    return maybeItem;
  }

  std::string name = this->CheckCMP0004(n);
  if (name.empty() ||
      (lookupSelf == LookupSelf::No && name == this->GetName())) {
    return maybeItem;
  }
  maybeItem =
    this->ResolveLinkItem(BT<std::string>(name, bt), scope->LG, linkFeature);
  return maybeItem;
}

void cmGeneratorTarget::ExpandLinkItems(
  std::string const& prop, cmBTStringRange entries, std::string const& config,
  cmGeneratorTarget const* headTarget, LinkInterfaceFor interfaceFor,
  LinkInterfaceField field, cmLinkInterface& iface) const
{
  if (entries.empty()) {
    return;
  }
  // Keep this logic in sync with ComputeLinkImplementationLibraries.
  cmGeneratorExpressionDAGChecker dagChecker(this, prop, nullptr, nullptr);
  // The $<LINK_ONLY> expression may be in a link interface to specify
  // private link dependencies that are otherwise excluded from usage
  // requirements.
  if (interfaceFor == LinkInterfaceFor::Usage) {
    dagChecker.SetTransitivePropertiesOnly();
    dagChecker.SetTransitivePropertiesOnlyCMP0131();
  }
  cmMakefile const* mf = this->LocalGenerator->GetMakefile();
  LookupLinkItemScope scope{ this->LocalGenerator };
  for (BT<std::string> const& entry : entries) {
    cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance(),
                             entry.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(entry.Value);
    cge->SetEvaluateForBuildsystem(true);
    cmList libs{ cge->Evaluate(this->LocalGenerator, config, headTarget,
                               &dagChecker, this,
                               headTarget->LinkerLanguage) };

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& lib : libs) {
      if (auto maybeLinkFeature = ParseLinkFeature(lib)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (cm::optional<cmLinkItem> maybeItem = this->LookupLinkItem(
            lib, cge->GetBacktrace(), linkFeature, &scope,
            field == LinkInterfaceField::Libraries ? LookupSelf::No
                                                   : LookupSelf::Yes)) {
        cmLinkItem item = std::move(*maybeItem);

        if (field == LinkInterfaceField::HeadInclude) {
          iface.HeadInclude.emplace_back(std::move(item));
          continue;
        }
        if (field == LinkInterfaceField::HeadExclude) {
          iface.HeadExclude.emplace_back(std::move(item));
          continue;
        }
        if (!item.Target) {
          // Report explicitly linked object files separately.
          std::string const& maybeObj = item.AsStr();
          if (cmSystemTools::FileIsFullPath(maybeObj)) {
            cmSourceFile const* sf =
              mf->GetSource(maybeObj, cmSourceFileLocationKind::Known);
            if (sf && sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
              item.ObjectSource = sf;
              iface.Objects.emplace_back(std::move(item));
              continue;
            }
          }
        }

        iface.Libraries.emplace_back(std::move(item));
      }
    }
    if (cge->GetHadHeadSensitiveCondition()) {
      iface.HadHeadSensitiveCondition = true;
    }
    if (cge->GetHadContextSensitiveCondition()) {
      iface.HadContextSensitiveCondition = true;
    }
    if (cge->GetHadLinkLanguageSensitiveCondition()) {
      iface.HadLinkLanguageSensitiveCondition = true;
    }
  }
}

cmLinkInterface const* cmGeneratorTarget::GetLinkInterface(
  const std::string& config, cmGeneratorTarget const* head) const
{
  return this->GetLinkInterface(config, head, false);
}

cmLinkInterface const* cmGeneratorTarget::GetLinkInterface(
  const std::string& config, cmGeneratorTarget const* head,
  bool secondPass) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, LinkInterfaceFor::Link,
                                        secondPass);
  }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if (this->GetType() == cmStateEnums::EXECUTABLE &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup any existing link interface for this configuration.
  cmHeadToLinkInterfaceMap& hm = this->GetHeadToLinkInterfaceMap(config);

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[head];
  if (secondPass) {
    iface = cmOptionalLinkInterface();
  }
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head,
                                        LinkInterfaceFor::Link);
  }
  if (!iface.AllDone) {
    iface.AllDone = true;
    if (iface.Exists) {
      this->ComputeLinkInterface(config, iface, head, secondPass);
      this->ComputeLinkInterfaceRuntimeLibraries(config, iface);
    }
  }

  return iface.Exists ? &iface : nullptr;
}

void cmGeneratorTarget::ComputeLinkInterface(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget) const
{
  this->ComputeLinkInterface(config, iface, headTarget, false);
}

void cmGeneratorTarget::ComputeLinkInterface(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget, bool secondPass) const
{
  if (iface.Explicit) {
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      // Shared libraries may have runtime implementation dependencies
      // on other shared libraries that are not in the interface.
      std::set<cmLinkItem> emitted;
      for (cmLinkItem const& lib : iface.Libraries) {
        emitted.insert(lib);
      }
      if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
        cmLinkImplementation const* impl = this->GetLinkImplementation(
          config, LinkInterfaceFor::Link, secondPass);
        for (cmLinkImplItem const& lib : impl->Libraries) {
          if (emitted.insert(lib).second) {
            if (lib.Target) {
              // This is a runtime dependency on another shared library.
              if (lib.Target->GetType() == cmStateEnums::SHARED_LIBRARY) {
                iface.SharedDeps.push_back(lib);
              }
            } else {
              // TODO: Recognize shared library file names.  Perhaps this
              // should be moved to cmComputeLinkInformation, but that
              // creates a chicken-and-egg problem since this list is needed
              // for its construction.
            }
          }
        }
      }
    }
  } else if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN ||
             this->GetPolicyStatusCMP0022() == cmPolicies::OLD) {
    // The link implementation is the default link interface.
    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibrariesInternal(config, headTarget,
                                                   LinkInterfaceFor::Link);
    iface.ImplementationIsInterface = true;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
  }

  if (this->LinkLanguagePropagatesToDependents()) {
    // Targets using this archive need its language runtime libraries.
    if (cmLinkImplementation const* impl = this->GetLinkImplementation(
          config, LinkInterfaceFor::Link, secondPass)) {
      iface.Languages = impl->Languages;
    }
  }

  if (this->GetType() == cmStateEnums::STATIC_LIBRARY) {
    // Construct the property name suffix for this configuration.
    std::string suffix = "_";
    if (!config.empty()) {
      suffix += cmSystemTools::UpperCase(config);
    } else {
      suffix += "NOCONFIG";
    }

    // How many repetitions are needed if this library has cyclic
    // dependencies?
    std::string propName = cmStrCat("LINK_INTERFACE_MULTIPLICITY", suffix);
    if (cmValue config_reps = this->GetProperty(propName)) {
      sscanf(config_reps->c_str(), "%u", &iface.Multiplicity);
    } else if (cmValue reps =
                 this->GetProperty("LINK_INTERFACE_MULTIPLICITY")) {
      sscanf(reps->c_str(), "%u", &iface.Multiplicity);
    }
  }
}

const cmLinkInterfaceLibraries* cmGeneratorTarget::GetLinkInterfaceLibraries(
  const std::string& config, cmGeneratorTarget const* head,
  LinkInterfaceFor interfaceFor) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, interfaceFor);
  }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if (this->GetType() == cmStateEnums::EXECUTABLE &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup any existing link interface for this configuration.
  cmHeadToLinkInterfaceMap& hm =
    (interfaceFor == LinkInterfaceFor::Usage
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[head];
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head, interfaceFor);
  }

  return iface.Exists ? &iface : nullptr;
}

std::string cmGeneratorTarget::GetDirectory(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    auto fullPath = this->Target->ImportedGetFullPath(config, artifact);
    if (this->IsFrameworkOnApple()) {
      auto fwDescriptor = this->GetGlobalGenerator()->SplitFrameworkPath(
        fullPath, cmGlobalGenerator::FrameworkFormat::Strict);
      if (fwDescriptor) {
        return fwDescriptor->Directory;
      }
    }
    // Return the directory from which the target is imported.
    return cmSystemTools::GetFilenamePath(fullPath);
  }
  if (OutputInfo const* info = this->GetOutputInfo(config)) {
    // Return the directory in which the target will be built.
    switch (artifact) {
      case cmStateEnums::RuntimeBinaryArtifact:
        return info->OutDir;
      case cmStateEnums::ImportLibraryArtifact:
        return info->ImpDir;
    }
  }
  return "";
}

bool cmGeneratorTarget::UsesDefaultOutputDir(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  std::string dir;
  return this->ComputeOutputDir(config, artifact, dir);
}

cmGeneratorTarget::OutputInfo const* cmGeneratorTarget::GetOutputInfo(
  const std::string& config) const
{
  // There is no output information for imported targets.
  if (this->IsImported()) {
    return nullptr;
  }

  // Synthetic targets don't have output.
  if (this->IsSynthetic()) {
    return nullptr;
  }

  // Only libraries and executables have well-defined output files.
  if (!this->HaveWellDefinedOutputFiles()) {
    std::string msg = cmStrCat("cmGeneratorTarget::GetOutputInfo called for ",
                               this->GetName(), " which has type ",
                               cmState::GetTargetTypeName(this->GetType()));
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
    return nullptr;
  }

  // Lookup/compute/cache the output information for this configuration.
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }
  auto i = this->OutputInfoMap.find(config_upper);
  if (i == this->OutputInfoMap.end()) {
    // Add empty info in map to detect potential recursion.
    OutputInfo info;
    OutputInfoMapType::value_type entry(config_upper, info);
    i = this->OutputInfoMap.insert(entry).first;

    // Compute output directories.
    this->ComputeOutputDir(config, cmStateEnums::RuntimeBinaryArtifact,
                           info.OutDir);
    this->ComputeOutputDir(config, cmStateEnums::ImportLibraryArtifact,
                           info.ImpDir);
    if (!this->ComputePDBOutputDir("PDB", config, info.PdbDir)) {
      info.PdbDir = info.OutDir;
    }

    // Now update the previously-prepared map entry.
    i->second = info;
  } else if (i->second.empty()) {
    // An empty map entry indicates we have been called recursively
    // from the above block.
    this->LocalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      "Target '" + this->GetName() + "' OUTPUT_DIRECTORY depends on itself.",
      this->GetBacktrace());
    return nullptr;
  }
  return &i->second;
}

bool cmGeneratorTarget::ComputeOutputDir(const std::string& config,
                                         cmStateEnums::ArtifactType artifact,
                                         std::string& out) const
{
  bool usesDefaultOutputDir = false;
  std::string conf = config;

  // Look for a target property defining the target output directory
  // based on the target type.
  std::string targetTypeName = this->GetOutputTargetType(artifact);
  std::string propertyName;
  if (!targetTypeName.empty()) {
    propertyName = cmStrCat(targetTypeName, "_OUTPUT_DIRECTORY");
  }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(conf);
  std::string configProp;
  if (!targetTypeName.empty()) {
    configProp = cmStrCat(targetTypeName, "_OUTPUT_DIRECTORY_", configUpper);
  }

  // Select an output directory.
  if (cmValue config_outdir = this->GetProperty(configProp)) {
    // Use the user-specified per-configuration output directory.
    out = cmGeneratorExpression::Evaluate(*config_outdir, this->LocalGenerator,
                                          config, this);

    // Skip per-configuration subdirectory.
    conf.clear();
  } else if (cmValue outdir = this->GetProperty(propertyName)) {
    // Use the user-specified output directory.
    out = cmGeneratorExpression::Evaluate(*outdir, this->LocalGenerator,
                                          config, this);
    // Skip per-configuration subdirectory if the value contained a
    // generator expression.
    if (out != *outdir) {
      conf.clear();
    }
  } else if (this->GetType() == cmStateEnums::EXECUTABLE) {
    // Lookup the output path for executables.
    out = this->Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
  } else if (this->GetType() == cmStateEnums::STATIC_LIBRARY ||
             this->GetType() == cmStateEnums::SHARED_LIBRARY ||
             this->GetType() == cmStateEnums::MODULE_LIBRARY) {
    // Lookup the output path for libraries.
    out = this->Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
  }
  if (out.empty()) {
    // Default to the current output directory.
    usesDefaultOutputDir = true;
    out = ".";
  }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath(
    out, this->LocalGenerator->GetCurrentBinaryDirectory()));

  // The generator may add the configuration's subdirectory.
  if (!conf.empty()) {
    bool useEPN =
      this->GlobalGenerator->UseEffectivePlatformName(this->Makefile);
    std::string suffix =
      usesDefaultOutputDir && useEPN ? "${EFFECTIVE_PLATFORM_NAME}" : "";
    this->LocalGenerator->GetGlobalGenerator()->AppendDirectoryForConfig(
      "/", conf, suffix, out);
  }

  return usesDefaultOutputDir;
}

bool cmGeneratorTarget::ComputePDBOutputDir(const std::string& kind,
                                            const std::string& config,
                                            std::string& out) const
{
  // Look for a target property defining the target output directory
  // based on the target type.
  std::string propertyName;
  if (!kind.empty()) {
    propertyName = cmStrCat(kind, "_OUTPUT_DIRECTORY");
  }
  std::string conf = config;

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(conf);
  std::string configProp;
  if (!kind.empty()) {
    configProp = cmStrCat(kind, "_OUTPUT_DIRECTORY_", configUpper);
  }

  // Select an output directory.
  if (cmValue config_outdir = this->GetProperty(configProp)) {
    // Use the user-specified per-configuration output directory.
    out = cmGeneratorExpression::Evaluate(*config_outdir, this->LocalGenerator,
                                          config);

    // Skip per-configuration subdirectory.
    conf.clear();
  } else if (cmValue outdir = this->GetProperty(propertyName)) {
    // Use the user-specified output directory.
    out =
      cmGeneratorExpression::Evaluate(*outdir, this->LocalGenerator, config);

    // Skip per-configuration subdirectory if the value contained a
    // generator expression.
    if (out != *outdir) {
      conf.clear();
    }
  }
  if (out.empty()) {
    return false;
  }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath(
    out, this->LocalGenerator->GetCurrentBinaryDirectory()));

  // The generator may add the configuration's subdirectory.
  if (!conf.empty()) {
    this->LocalGenerator->GetGlobalGenerator()->AppendDirectoryForConfig(
      "/", conf, "", out);
  }
  return true;
}

bool cmGeneratorTarget::HaveInstallTreeRPATH(const std::string& config) const
{
  std::string install_rpath;
  this->GetInstallRPATH(config, install_rpath);
  return !install_rpath.empty() &&
    !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
}

bool cmGeneratorTarget::GetBuildRPATH(const std::string& config,
                                      std::string& rpath) const
{
  return this->GetRPATH(config, "BUILD_RPATH", rpath);
}

bool cmGeneratorTarget::GetInstallRPATH(const std::string& config,
                                        std::string& rpath) const
{
  return this->GetRPATH(config, "INSTALL_RPATH", rpath);
}

bool cmGeneratorTarget::GetRPATH(const std::string& config,
                                 const std::string& prop,
                                 std::string& rpath) const
{
  cmValue value = this->GetProperty(prop);
  if (!value) {
    return false;
  }

  rpath =
    cmGeneratorExpression::Evaluate(*value, this->LocalGenerator, config);

  return true;
}

void cmGeneratorTarget::ComputeLinkInterfaceLibraries(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget, LinkInterfaceFor interfaceFor) const
{
  // Construct the property name suffix for this configuration.
  std::string suffix = "_";
  if (!config.empty()) {
    suffix += cmSystemTools::UpperCase(config);
  } else {
    suffix += "NOCONFIG";
  }

  // An explicit list of interface libraries may be set for shared
  // libraries and executables that export symbols.
  bool haveExplicitLibraries = false;
  cmValue explicitLibrariesCMP0022OLD;
  std::string linkIfacePropCMP0022OLD;
  bool const cmp0022NEW = (this->GetPolicyStatusCMP0022() != cmPolicies::OLD &&
                           this->GetPolicyStatusCMP0022() != cmPolicies::WARN);
  if (cmp0022NEW) {
    // CMP0022 NEW behavior is to use INTERFACE_LINK_LIBRARIES.
    haveExplicitLibraries = !this->Target->GetLinkInterfaceEntries().empty() ||
      !this->Target->GetLinkInterfaceDirectEntries().empty() ||
      !this->Target->GetLinkInterfaceDirectExcludeEntries().empty();
  } else {
    // CMP0022 OLD behavior is to use LINK_INTERFACE_LIBRARIES if set on a
    // shared lib or executable.
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->IsExecutableWithExports()) {
      // Lookup the per-configuration property.
      linkIfacePropCMP0022OLD = cmStrCat("LINK_INTERFACE_LIBRARIES", suffix);
      explicitLibrariesCMP0022OLD = this->GetProperty(linkIfacePropCMP0022OLD);

      // If not set, try the generic property.
      if (!explicitLibrariesCMP0022OLD) {
        linkIfacePropCMP0022OLD = "LINK_INTERFACE_LIBRARIES";
        explicitLibrariesCMP0022OLD =
          this->GetProperty(linkIfacePropCMP0022OLD);
      }
    }

    if (explicitLibrariesCMP0022OLD &&
        this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
        !this->PolicyWarnedCMP0022) {
      // Compare the explicitly set old link interface properties to the
      // preferred new link interface property one and warn if different.
      cmValue newExplicitLibraries =
        this->GetProperty("INTERFACE_LINK_LIBRARIES");
      if (newExplicitLibraries &&
          (*newExplicitLibraries != *explicitLibrariesCMP0022OLD)) {
        std::ostringstream w;
        /* clang-format off */
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0022) << "\n"
          "Target \"" << this->GetName() << "\" has an "
          "INTERFACE_LINK_LIBRARIES property which differs from its " <<
          linkIfacePropCMP0022OLD << " properties."
          "\n"
          "INTERFACE_LINK_LIBRARIES:\n"
          "  " << *newExplicitLibraries << "\n" <<
          linkIfacePropCMP0022OLD << ":\n"
          "  " << *explicitLibrariesCMP0022OLD << "\n";
        /* clang-format on */
        this->LocalGenerator->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
        this->PolicyWarnedCMP0022 = true;
      }
    }

    haveExplicitLibraries = static_cast<bool>(explicitLibrariesCMP0022OLD);
  }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  if (!haveExplicitLibraries &&
      (this->GetType() == cmStateEnums::EXECUTABLE ||
       (this->GetType() == cmStateEnums::MODULE_LIBRARY))) {
    return;
  }
  iface.Exists = true;

  // If CMP0022 is NEW then the plain tll signature sets the
  // INTERFACE_LINK_LIBRARIES property.  Even if the project
  // clears it, the link interface is still explicit.
  iface.Explicit = cmp0022NEW || explicitLibrariesCMP0022OLD;

  if (cmp0022NEW) {
    // The interface libraries are specified by INTERFACE_LINK_LIBRARIES.
    // Use its special representation directly to get backtraces.
    this->ExpandLinkItems(
      kINTERFACE_LINK_LIBRARIES, this->Target->GetLinkInterfaceEntries(),
      config, headTarget, interfaceFor, LinkInterfaceField::Libraries, iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT,
                          this->Target->GetLinkInterfaceDirectEntries(),
                          config, headTarget, interfaceFor,
                          LinkInterfaceField::HeadInclude, iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE,
                          this->Target->GetLinkInterfaceDirectExcludeEntries(),
                          config, headTarget, interfaceFor,
                          LinkInterfaceField::HeadExclude, iface);
  } else if (explicitLibrariesCMP0022OLD) {
    // The interface libraries have been explicitly set in pre-CMP0022 style.
    std::vector<BT<std::string>> entries;
    entries.emplace_back(*explicitLibrariesCMP0022OLD);
    this->ExpandLinkItems(linkIfacePropCMP0022OLD, cmMakeRange(entries),
                          config, headTarget, interfaceFor,
                          LinkInterfaceField::Libraries, iface);
  }

  // If the link interface is explicit, do not fall back to the link impl.
  if (iface.Explicit) {
    return;
  }

  // The link implementation is the default link interface.
  if (cmLinkImplementationLibraries const* impl =
        this->GetLinkImplementationLibrariesInternal(config, headTarget,
                                                     interfaceFor)) {
    iface.Libraries.insert(iface.Libraries.end(), impl->Libraries.begin(),
                           impl->Libraries.end());
    if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
        !this->PolicyWarnedCMP0022 && interfaceFor == LinkInterfaceFor::Link) {
      // Compare the link implementation fallback link interface to the
      // preferred new link interface property and warn if different.
      cmLinkInterface ifaceNew;
      this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES,
                            this->Target->GetLinkInterfaceEntries(), config,
                            headTarget, interfaceFor,
                            LinkInterfaceField::Libraries, ifaceNew);
      if (ifaceNew.Libraries != iface.Libraries) {
        std::string oldLibraries = cmJoin(impl->Libraries, ";");
        std::string newLibraries = cmJoin(ifaceNew.Libraries, ";");
        if (oldLibraries.empty()) {
          oldLibraries = "(empty)";
        }
        if (newLibraries.empty()) {
          newLibraries = "(empty)";
        }

        std::ostringstream w;
        /* clang-format off */
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0022) << "\n"
          "Target \"" << this->GetName() << "\" has an "
          "INTERFACE_LINK_LIBRARIES property.  "
          "This should be preferred as the source of the link interface "
          "for this library but because CMP0022 is not set CMake is "
          "ignoring the property and using the link implementation "
          "as the link interface instead."
          "\n"
          "INTERFACE_LINK_LIBRARIES:\n"
          "  " << newLibraries << "\n"
          "Link implementation:\n"
          "  " << oldLibraries << "\n";
        /* clang-format on */
        this->LocalGenerator->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
        this->PolicyWarnedCMP0022 = true;
      }
    }
  }
}

namespace {

template <typename ReturnType>
ReturnType constructItem(cmGeneratorTarget* target,
                         cmListFileBacktrace const& bt);

template <>
inline cmLinkImplItem constructItem(cmGeneratorTarget* target,
                                    cmListFileBacktrace const& bt)
{
  return cmLinkImplItem(cmLinkItem(target, false, bt), false);
}

template <>
inline cmLinkItem constructItem(cmGeneratorTarget* target,
                                cmListFileBacktrace const& bt)
{
  return cmLinkItem(target, false, bt);
}

template <typename ValueType>
std::vector<ValueType> computeImplicitLanguageTargets(
  std::string const& lang, std::string const& config,
  cmGeneratorTarget const* currentTarget)
{
  cmListFileBacktrace bt;
  std::vector<ValueType> result;
  cmLocalGenerator* lg = currentTarget->GetLocalGenerator();

  std::string const& runtimeLibrary =
    currentTarget->GetRuntimeLinkLibrary(lang, config);
  if (cmValue runtimeLinkOptions = currentTarget->Makefile->GetDefinition(
        "CMAKE_" + lang + "_RUNTIME_LIBRARIES_" + runtimeLibrary)) {
    cmList libsList{ *runtimeLinkOptions };
    result.reserve(libsList.size());

    for (auto const& i : libsList) {
      cmGeneratorTarget::TargetOrString resolved =
        currentTarget->ResolveTargetReference(i, lg);
      if (resolved.Target) {
        result.emplace_back(constructItem<ValueType>(resolved.Target, bt));
      }
    }
  }

  return result;
}
}

void cmGeneratorTarget::ComputeLinkInterfaceRuntimeLibraries(
  const std::string& config, cmOptionalLinkInterface& iface) const
{
  for (std::string const& lang : iface.Languages) {
    if ((lang == "CUDA" || lang == "HIP") &&
        iface.LanguageRuntimeLibraries.find(lang) ==
          iface.LanguageRuntimeLibraries.end()) {
      auto implicitTargets =
        computeImplicitLanguageTargets<cmLinkItem>(lang, config, this);
      iface.LanguageRuntimeLibraries[lang] = std::move(implicitTargets);
    }
  }
}

void cmGeneratorTarget::ComputeLinkImplementationRuntimeLibraries(
  const std::string& config, cmOptionalLinkImplementation& impl) const
{
  for (std::string const& lang : impl.Languages) {
    if ((lang == "CUDA" || lang == "HIP") &&
        impl.LanguageRuntimeLibraries.find(lang) ==
          impl.LanguageRuntimeLibraries.end()) {
      auto implicitTargets =
        computeImplicitLanguageTargets<cmLinkImplItem>(lang, config, this);
      impl.LanguageRuntimeLibraries[lang] = std::move(implicitTargets);
    }
  }
}

const cmLinkInterface* cmGeneratorTarget::GetImportLinkInterface(
  const std::string& config, cmGeneratorTarget const* headTarget,
  LinkInterfaceFor interfaceFor, bool secondPass) const
{
  cmGeneratorTarget::ImportInfo const* info = this->GetImportInfo(config);
  if (!info) {
    return nullptr;
  }

  cmHeadToLinkInterfaceMap& hm =
    (interfaceFor == LinkInterfaceFor::Usage
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    headTarget = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[headTarget];
  if (secondPass) {
    iface = cmOptionalLinkInterface();
  }
  if (!iface.AllDone) {
    iface.AllDone = true;
    iface.LibrariesDone = true;
    iface.Multiplicity = info->Multiplicity;
    cmExpandList(info->Languages, iface.Languages);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT,
                          cmMakeRange(info->LibrariesHeadInclude), config,
                          headTarget, interfaceFor,
                          LinkInterfaceField::HeadInclude, iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE,
                          cmMakeRange(info->LibrariesHeadExclude), config,
                          headTarget, interfaceFor,
                          LinkInterfaceField::HeadExclude, iface);
    this->ExpandLinkItems(info->LibrariesProp, cmMakeRange(info->Libraries),
                          config, headTarget, interfaceFor,
                          LinkInterfaceField::Libraries, iface);
    cmList deps{ info->SharedDeps };
    LookupLinkItemScope scope{ this->LocalGenerator };

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& dep : deps) {
      if (auto maybeLinkFeature = ParseLinkFeature(dep)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (cm::optional<cmLinkItem> maybeItem = this->LookupLinkItem(
            dep, cmListFileBacktrace(), linkFeature, &scope, LookupSelf::No)) {
        iface.SharedDeps.emplace_back(std::move(*maybeItem));
      }
    }
  }

  return &iface;
}

cmGeneratorTarget::ImportInfo const* cmGeneratorTarget::GetImportInfo(
  const std::string& config) const
{
  // There is no imported information for non-imported targets.
  if (!this->IsImported()) {
    return nullptr;
  }

  // Lookup/compute/cache the import information for this
  // configuration.
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  } else {
    config_upper = "NOCONFIG";
  }

  auto i = this->ImportInfoMap.find(config_upper);
  if (i == this->ImportInfoMap.end()) {
    ImportInfo info;
    this->ComputeImportInfo(config_upper, info);
    ImportInfoMapType::value_type entry(config_upper, info);
    i = this->ImportInfoMap.insert(entry).first;
  }

  if (this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return &i->second;
  }
  // If the location is empty then the target is not available for
  // this configuration.
  if (i->second.Location.empty() && i->second.ImportLibrary.empty()) {
    return nullptr;
  }

  // Return the import information.
  return &i->second;
}

void cmGeneratorTarget::ComputeImportInfo(std::string const& desired_config,
                                          ImportInfo& info) const
{
  // This method finds information about an imported target from its
  // properties.  The "IMPORTED_" namespace is reserved for properties
  // defined by the project exporting the target.

  // Initialize members.
  info.NoSOName = false;

  cmValue loc = nullptr;
  cmValue imp = nullptr;
  std::string suffix;
  if (!this->Target->GetMappedConfig(desired_config, loc, imp, suffix)) {
    return;
  }

  // Get the link interface.
  {
    // Use the INTERFACE_LINK_LIBRARIES special representation directly
    // to get backtraces.
    cmBTStringRange entries = this->Target->GetLinkInterfaceEntries();
    if (!entries.empty()) {
      info.LibrariesProp = "INTERFACE_LINK_LIBRARIES";
      for (BT<std::string> const& entry : entries) {
        info.Libraries.emplace_back(entry);
      }
    } else if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
      std::string linkProp =
        cmStrCat("IMPORTED_LINK_INTERFACE_LIBRARIES", suffix);
      cmValue propertyLibs = this->GetProperty(linkProp);
      if (!propertyLibs) {
        linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
        propertyLibs = this->GetProperty(linkProp);
      }
      if (propertyLibs) {
        info.LibrariesProp = linkProp;
        info.Libraries.emplace_back(*propertyLibs);
      }
    }
  }
  for (BT<std::string> const& entry :
       this->Target->GetLinkInterfaceDirectEntries()) {
    info.LibrariesHeadInclude.emplace_back(entry);
  }
  for (BT<std::string> const& entry :
       this->Target->GetLinkInterfaceDirectExcludeEntries()) {
    info.LibrariesHeadExclude.emplace_back(entry);
  }
  if (this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    if (loc) {
      info.LibName = *loc;
    }
    return;
  }

  // A provided configuration has been chosen.  Load the
  // configuration's properties.

  // Get the location.
  if (loc) {
    info.Location = *loc;
  } else {
    std::string impProp = cmStrCat("IMPORTED_LOCATION", suffix);
    if (cmValue config_location = this->GetProperty(impProp)) {
      info.Location = *config_location;
    } else if (cmValue location = this->GetProperty("IMPORTED_LOCATION")) {
      info.Location = *location;
    }
  }

  // Get the soname.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY) {
    std::string soProp = cmStrCat("IMPORTED_SONAME", suffix);
    if (cmValue config_soname = this->GetProperty(soProp)) {
      info.SOName = *config_soname;
    } else if (cmValue soname = this->GetProperty("IMPORTED_SONAME")) {
      info.SOName = *soname;
    }
  }

  // Get the "no-soname" mark.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY) {
    std::string soProp = cmStrCat("IMPORTED_NO_SONAME", suffix);
    if (cmValue config_no_soname = this->GetProperty(soProp)) {
      info.NoSOName = cmIsOn(*config_no_soname);
    } else if (cmValue no_soname = this->GetProperty("IMPORTED_NO_SONAME")) {
      info.NoSOName = cmIsOn(*no_soname);
    }
  }

  // Get the import library.
  if (imp) {
    info.ImportLibrary = *imp;
  } else if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
             this->IsExecutableWithExports()) {
    std::string impProp = cmStrCat("IMPORTED_IMPLIB", suffix);
    if (cmValue config_implib = this->GetProperty(impProp)) {
      info.ImportLibrary = *config_implib;
    } else if (cmValue implib = this->GetProperty("IMPORTED_IMPLIB")) {
      info.ImportLibrary = *implib;
    }
  }

  // Get the link dependencies.
  {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_DEPENDENT_LIBRARIES", suffix);
    if (cmValue config_libs = this->GetProperty(linkProp)) {
      info.SharedDeps = *config_libs;
    } else if (cmValue libs =
                 this->GetProperty("IMPORTED_LINK_DEPENDENT_LIBRARIES")) {
      info.SharedDeps = *libs;
    }
  }

  // Get the link languages.
  if (this->LinkLanguagePropagatesToDependents()) {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_INTERFACE_LANGUAGES", suffix);
    if (cmValue config_libs = this->GetProperty(linkProp)) {
      info.Languages = *config_libs;
    } else if (cmValue libs =
                 this->GetProperty("IMPORTED_LINK_INTERFACE_LANGUAGES")) {
      info.Languages = *libs;
    }
  }

  // Get information if target is managed assembly.
  {
    std::string linkProp = "IMPORTED_COMMON_LANGUAGE_RUNTIME";
    if (cmValue pc = this->GetProperty(linkProp + suffix)) {
      info.Managed = this->CheckManagedType(*pc);
    } else if (cmValue p = this->GetProperty(linkProp)) {
      info.Managed = this->CheckManagedType(*p);
    }
  }

  // Get the cyclic repetition count.
  if (this->GetType() == cmStateEnums::STATIC_LIBRARY) {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_INTERFACE_MULTIPLICITY", suffix);
    if (cmValue config_reps = this->GetProperty(linkProp)) {
      sscanf(config_reps->c_str(), "%u", &info.Multiplicity);
    } else if (cmValue reps =
                 this->GetProperty("IMPORTED_LINK_INTERFACE_MULTIPLICITY")) {
      sscanf(reps->c_str(), "%u", &info.Multiplicity);
    }
  }
}

cmHeadToLinkInterfaceMap& cmGeneratorTarget::GetHeadToLinkInterfaceMap(
  const std::string& config) const
{
  return this->LinkInterfaceMap[cmSystemTools::UpperCase(config)];
}

cmHeadToLinkInterfaceMap&
cmGeneratorTarget::GetHeadToLinkInterfaceUsageRequirementsMap(
  const std::string& config) const
{
  return this
    ->LinkInterfaceUsageRequirementsOnlyMap[cmSystemTools::UpperCase(config)];
}

const cmLinkImplementation* cmGeneratorTarget::GetLinkImplementation(
  const std::string& config, LinkInterfaceFor implFor) const
{
  return this->GetLinkImplementation(config, implFor, false);
}

const cmLinkImplementation* cmGeneratorTarget::GetLinkImplementation(
  const std::string& config, LinkInterfaceFor implFor, bool secondPass) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    return nullptr;
  }

  HeadToLinkImplementationMap& hm =
    (implFor == LinkInterfaceFor::Usage
       ? this->GetHeadToLinkImplementationUsageRequirementsMap(config)
       : this->GetHeadToLinkImplementationMap(config));
  cmOptionalLinkImplementation& impl = hm[this];
  if (secondPass) {
    impl = cmOptionalLinkImplementation();
  }
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, this, implFor);
  }
  if (!impl.LanguagesDone) {
    impl.LanguagesDone = true;
    this->ComputeLinkImplementationLanguages(config, impl);
    this->ComputeLinkImplementationRuntimeLibraries(config, impl);
  }
  return &impl;
}

cmGeneratorTarget::HeadToLinkImplementationMap&
cmGeneratorTarget::GetHeadToLinkImplementationMap(
  std::string const& config) const
{
  return this->LinkImplMap[cmSystemTools::UpperCase(config)];
}

cmGeneratorTarget::HeadToLinkImplementationMap&
cmGeneratorTarget::GetHeadToLinkImplementationUsageRequirementsMap(
  std::string const& config) const
{
  return this
    ->LinkImplUsageRequirementsOnlyMap[cmSystemTools::UpperCase(config)];
}

bool cmGeneratorTarget::GetConfigCommonSourceFilesForXcode(
  std::vector<cmSourceFile*>& files) const
{
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  auto it = configs.begin();
  const std::string& firstConfig = *it;
  this->GetSourceFilesWithoutObjectLibraries(files, firstConfig);

  for (; it != configs.end(); ++it) {
    std::vector<cmSourceFile*> configFiles;
    this->GetSourceFilesWithoutObjectLibraries(configFiles, *it);
    if (configFiles != files) {
      std::string firstConfigFiles;
      const char* sep = "";
      for (cmSourceFile* f : files) {
        firstConfigFiles += sep;
        firstConfigFiles += f->ResolveFullPath();
        sep = "\n  ";
      }

      std::string thisConfigFiles;
      sep = "";
      for (cmSourceFile* f : configFiles) {
        thisConfigFiles += sep;
        thisConfigFiles += f->ResolveFullPath();
        sep = "\n  ";
      }
      std::ostringstream e;
      /* clang-format off */
      e << "Target \"" << this->GetName()
        << "\" has source files which vary by "
        "configuration. This is not supported by the \""
        << this->GlobalGenerator->GetName()
        << "\" generator.\n"
          "Config \"" << firstConfig << "\":\n"
          "  " << firstConfigFiles << "\n"
          "Config \"" << *it << "\":\n"
          "  " << thisConfigFiles << "\n";
      /* clang-format on */
      this->LocalGenerator->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }
  return true;
}

void cmGeneratorTarget::GetObjectLibrariesCMP0026(
  std::vector<cmGeneratorTarget*>& objlibs) const
{
  // At configure-time, this method can be called as part of getting the
  // LOCATION property or to export() a file to be include()d.  However
  // there is no cmGeneratorTarget at configure-time, so search the SOURCES
  // for TARGET_OBJECTS instead for backwards compatibility with OLD
  // behavior of CMP0024 and CMP0026 only.
  cmBTStringRange rng = this->Target->GetSourceEntries();
  for (auto const& entry : rng) {
    cmList files{ entry.Value };
    for (auto const& li : files) {
      if (cmHasLiteralPrefix(li, "$<TARGET_OBJECTS:") && li.back() == '>') {
        std::string objLibName = li.substr(17, li.size() - 18);

        if (cmGeneratorExpression::Find(objLibName) != std::string::npos) {
          continue;
        }
        cmGeneratorTarget* objLib =
          this->LocalGenerator->FindGeneratorTargetToUse(objLibName);
        if (objLib) {
          objlibs.push_back(objLib);
        }
      }
    }
  }
}

std::string cmGeneratorTarget::CheckCMP0004(std::string const& item) const
{
  // Strip whitespace off the library names because we used to do this
  // in case variables were expanded at generate time.  We no longer
  // do the expansion but users link to libraries like " ${VAR} ".
  std::string lib = item;
  std::string::size_type pos = lib.find_first_not_of(" \t\r\n");
  if (pos != std::string::npos) {
    lib = lib.substr(pos);
  }
  pos = lib.find_last_not_of(" \t\r\n");
  if (pos != std::string::npos) {
    lib = lib.substr(0, pos + 1);
  }
  if (lib != item) {
    cmake* cm = this->LocalGenerator->GetCMakeInstance();
    switch (this->GetPolicyStatusCMP0004()) {
      case cmPolicies::WARN: {
        std::ostringstream w;
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0004) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \"" << item
          << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(MessageType::AUTHOR_WARNING, w.str(),
                         this->GetBacktrace());
      }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW: {
        std::ostringstream e;
        e << "Target \"" << this->GetName() << "\" links to item \"" << item
          << "\" which has leading or trailing whitespace.  "
          << "This is now an error according to policy CMP0004.";
        cm->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                         this->GetBacktrace());
      } break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS: {
        std::ostringstream e;
        e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0004) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \"" << item
          << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                         this->GetBacktrace());
      } break;
    }
  }
  return lib;
}

bool cmGeneratorTarget::IsDeprecated() const
{
  cmValue deprecation = this->GetProperty("DEPRECATION");
  return cmNonempty(deprecation);
}

std::string cmGeneratorTarget::GetDeprecation() const
{
  // find DEPRECATION property
  if (cmValue deprecation = this->GetProperty("DEPRECATION")) {
    return *deprecation;
  }
  return std::string();
}

void cmGeneratorTarget::GetLanguages(std::set<std::string>& languages,
                                     const std::string& config) const
{
  // Targets that do not compile anything have no languages.
  if (!this->CanCompileSources()) {
    return;
  }

  std::vector<cmSourceFile*> sourceFiles;
  this->GetSourceFiles(sourceFiles, config);
  for (cmSourceFile* src : sourceFiles) {
    const std::string& lang = src->GetOrDetermineLanguage();
    if (!lang.empty()) {
      languages.insert(lang);
    }
  }

  std::set<cmGeneratorTarget const*> objectLibraries;
  if (!this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    std::vector<cmGeneratorTarget*> objectTargets;
    this->GetObjectLibrariesCMP0026(objectTargets);
    for (cmGeneratorTarget* gt : objectTargets) {
      objectLibraries.insert(gt);
    }
  } else {
    objectLibraries = this->GetSourceObjectLibraries(config);
  }
  for (cmGeneratorTarget const* objLib : objectLibraries) {
    objLib->GetLanguages(languages, config);
  }
}

std::set<cmGeneratorTarget const*> cmGeneratorTarget::GetSourceObjectLibraries(
  std::string const& config) const
{
  std::set<cmGeneratorTarget const*> objectLibraries;
  std::vector<cmSourceFile const*> externalObjects;
  this->GetExternalObjects(externalObjects, config);
  for (cmSourceFile const* extObj : externalObjects) {
    std::string objLib = extObj->GetObjectLibrary();
    if (cmGeneratorTarget* tgt =
          this->LocalGenerator->FindGeneratorTargetToUse(objLib)) {
      objectLibraries.insert(tgt);
    }
  }

  return objectLibraries;
}

bool cmGeneratorTarget::IsLanguageUsed(std::string const& language,
                                       std::string const& config) const
{
  std::set<std::string> languages;
  this->GetLanguages(languages, config);
  return languages.count(language);
}

bool cmGeneratorTarget::IsCSharpOnly() const
{
  // Only certain target types may compile CSharp.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    return false;
  }
  std::set<std::string> languages = this->GetAllConfigCompileLanguages();
  // Consider an explicit linker language property, but *not* the
  // computed linker language that may depend on linked targets.
  cmValue linkLang = this->GetProperty("LINKER_LANGUAGE");
  if (cmNonempty(linkLang)) {
    languages.insert(*linkLang);
  }
  return languages.size() == 1 && languages.count("CSharp") > 0;
}

bool cmGeneratorTarget::IsDotNetSdkTarget() const
{
  return !this->GetProperty("DOTNET_SDK").IsEmpty();
}

void cmGeneratorTarget::ComputeLinkImplementationLanguages(
  const std::string& config, cmOptionalLinkImplementation& impl) const
{
  // This target needs runtime libraries for its source languages.
  std::set<std::string> languages;
  // Get languages used in our source files.
  this->GetLanguages(languages, config);
  // Copy the set of languages to the link implementation.
  impl.Languages.insert(impl.Languages.begin(), languages.begin(),
                        languages.end());
}

bool cmGeneratorTarget::HaveBuildTreeRPATH(const std::string& config) const
{
  if (this->GetPropertyAsBool("SKIP_BUILD_RPATH")) {
    return false;
  }
  std::string build_rpath;
  if (this->GetBuildRPATH(config, build_rpath)) {
    return true;
  }
  if (cmLinkImplementationLibraries const* impl =
        this->GetLinkImplementationLibraries(config, LinkInterfaceFor::Link)) {
    return !impl->Libraries.empty();
  }
  return false;
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibraries(
  const std::string& config, LinkInterfaceFor implFor) const
{
  return this->GetLinkImplementationLibrariesInternal(config, this, implFor);
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibrariesInternal(
  const std::string& config, cmGeneratorTarget const* head,
  LinkInterfaceFor implFor) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    return nullptr;
  }

  // Populate the link implementation libraries for this configuration.
  HeadToLinkImplementationMap& hm =
    (implFor == LinkInterfaceFor::Usage
       ? this->GetHeadToLinkImplementationUsageRequirementsMap(config)
       : this->GetHeadToLinkImplementationMap(config));

  // If the link implementation does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkImplementation& impl = hm[head];
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, head, implFor);
  }
  return &impl;
}

bool cmGeneratorTarget::IsNullImpliedByLinkLibraries(
  const std::string& p) const
{
  return cm::contains(this->LinkImplicitNullProperties, p);
}

namespace {
class TransitiveLinkImpl
{
  cmGeneratorTarget const* Self;
  std::string const& Config;
  LinkInterfaceFor ImplFor;
  cmLinkImplementation& Impl;

  std::set<cmLinkItem> Emitted;
  std::set<cmLinkItem> Excluded;
  std::unordered_set<cmGeneratorTarget const*> Followed;

  void Follow(cmGeneratorTarget const* target);

public:
  TransitiveLinkImpl(cmGeneratorTarget const* self, std::string const& config,
                     LinkInterfaceFor implFor, cmLinkImplementation& impl)
    : Self(self)
    , Config(config)
    , ImplFor(implFor)
    , Impl(impl)
  {
  }

  void Compute();
};

void TransitiveLinkImpl::Follow(cmGeneratorTarget const* target)
{
  if (!target || !this->Followed.insert(target).second ||
      target->GetPolicyStatusCMP0022() == cmPolicies::OLD ||
      target->GetPolicyStatusCMP0022() == cmPolicies::WARN) {
    return;
  }

  // Get this target's usage requirements.
  cmLinkInterfaceLibraries const* iface =
    target->GetLinkInterfaceLibraries(this->Config, this->Self, this->ImplFor);
  if (!iface) {
    return;
  }
  if (iface->HadContextSensitiveCondition) {
    this->Impl.HadContextSensitiveCondition = true;
  }

  // Process 'INTERFACE_LINK_LIBRARIES_DIRECT' usage requirements.
  for (cmLinkItem const& item : iface->HeadInclude) {
    // Inject direct dependencies from the item's usage requirements
    // before the item itself.
    this->Follow(item.Target);

    // Add the item itself, but at most once.
    if (this->Emitted.insert(item).second) {
      this->Impl.Libraries.emplace_back(item, /* checkCMP0027= */ false);
    }
  }

  // Follow transitive dependencies.
  for (cmLinkItem const& item : iface->Libraries) {
    this->Follow(item.Target);
  }

  // Record exclusions from 'INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE'
  // usage requirements.
  for (cmLinkItem const& item : iface->HeadExclude) {
    this->Excluded.insert(item);
  }
}

void TransitiveLinkImpl::Compute()
{
  // Save the original items and start with an empty list.
  std::vector<cmLinkImplItem> original = std::move(this->Impl.Libraries);

  // Avoid injecting any original items as usage requirements.
  // This gives LINK_LIBRARIES final control over the order
  // if it explicitly lists everything.
  this->Emitted.insert(original.cbegin(), original.cend());

  // Process each original item.
  for (cmLinkImplItem& item : original) {
    // Inject direct dependencies listed in 'INTERFACE_LINK_LIBRARIES_DIRECT'
    // usage requirements before the item itself.
    this->Follow(item.Target);

    // Add the item itself.
    this->Impl.Libraries.emplace_back(std::move(item));
  }

  // Remove items listed in 'INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE'
  // usage requirements found through any dependency above.
  this->Impl.Libraries.erase(
    std::remove_if(this->Impl.Libraries.begin(), this->Impl.Libraries.end(),
                   [this](cmLinkImplItem const& item) {
                     return this->Excluded.find(item) != this->Excluded.end();
                   }),
    this->Impl.Libraries.end());
}

void ComputeLinkImplTransitive(cmGeneratorTarget const* self,
                               std::string const& config,
                               LinkInterfaceFor implFor,
                               cmLinkImplementation& impl)
{
  TransitiveLinkImpl transitiveLinkImpl(self, config, implFor, impl);
  transitiveLinkImpl.Compute();
}
}

bool cmGeneratorTarget::DiscoverSyntheticTargets(cmSyntheticTargetCache& cache,
                                                 std::string const& config)
{
  cmOptionalLinkImplementation impl;
  this->ComputeLinkImplementationLibraries(config, impl, this,
                                           LinkInterfaceFor::Link);

  cmCxxModuleUsageEffects usage(this);

  auto& SyntheticDeps = this->Configs[config].SyntheticDeps;

  for (auto const& entry : impl.Libraries) {
    auto const* gt = entry.Target;
    if (!gt || !gt->IsImported()) {
      continue;
    }

    if (gt->HaveCxx20ModuleSources()) {
      cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_512);
      constexpr size_t HASH_TRUNCATION = 12;
      auto dirhash = hasher.HashString(
        gt->GetLocalGenerator()->GetCurrentBinaryDirectory());
      std::string safeName = gt->GetName();
      cmSystemTools::ReplaceString(safeName, ":", "_");
      auto targetIdent =
        hasher.HashString(cmStrCat("@d_", dirhash, "@u_", usage.GetHash()));
      std::string targetName =
        cmStrCat(safeName, "@synth_", targetIdent.substr(0, HASH_TRUNCATION));

      // Check the cache to see if this instance of the imported target has
      // already been created.
      auto cached = cache.CxxModuleTargets.find(targetName);
      cmGeneratorTarget const* synthDep = nullptr;
      if (cached == cache.CxxModuleTargets.end()) {
        auto const* model = gt->Target;
        auto* mf = gt->Makefile;
        auto* lg = gt->GetLocalGenerator();
        auto* tgt = mf->AddSynthesizedTarget(cmStateEnums::INTERFACE_LIBRARY,
                                             targetName);

        // Copy relevant information from the existing IMPORTED target.

        // Copy policies to the target.
        tgt->CopyPolicyStatuses(model);

        // Copy file sets.
        {
          auto fsNames = model->GetAllFileSetNames();
          for (auto const& fsName : fsNames) {
            auto const* fs = model->GetFileSet(fsName);
            if (!fs) {
              mf->IssueMessage(MessageType::INTERNAL_ERROR,
                               cmStrCat("Failed to find file set named '",
                                        fsName, "' on target '",
                                        tgt->GetName(), '\''));
              continue;
            }
            auto* newFs = tgt
                            ->GetOrCreateFileSet(fs->GetName(), fs->GetType(),
                                                 fs->GetVisibility())
                            .first;
            newFs->CopyEntries(fs);
          }
        }

        // Copy imported C++ module properties.
        tgt->CopyImportedCxxModulesEntries(model);

        // Copy other properties which may affect the C++ module BMI
        // generation.
        tgt->CopyImportedCxxModulesProperties(model);

        tgt->AddLinkLibrary(*mf,
                            cmStrCat("$<COMPILE_ONLY:", model->GetName(), '>'),
                            GENERAL_LibraryType);

        // Apply usage requirements to the target.
        usage.ApplyToTarget(tgt);

        // Create the generator target and attach it to the local generator.
        auto gtp = cm::make_unique<cmGeneratorTarget>(tgt, lg);
        synthDep = gtp.get();
        cache.CxxModuleTargets[targetName] = synthDep;
        gtp->DiscoverSyntheticTargets(cache, config);
        lg->AddGeneratorTarget(std::move(gtp));
      } else {
        synthDep = cached->second;
      }

      SyntheticDeps[gt].push_back(synthDep);
    }
  }

  return true;
}

void cmGeneratorTarget::ComputeLinkImplementationLibraries(
  const std::string& config, cmOptionalLinkImplementation& impl,
  cmGeneratorTarget const* head, LinkInterfaceFor implFor) const
{
  cmLocalGenerator const* lg = this->LocalGenerator;
  cmMakefile const* mf = lg->GetMakefile();
  cmBTStringRange entryRange = this->Target->GetLinkImplementationEntries();
  auto const& synthTargetsForConfig = this->Configs[config].SyntheticDeps;
  // Collect libraries directly linked in this configuration.
  for (auto const& entry : entryRange) {
    // Keep this logic in sync with ExpandLinkItems.
    cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_LIBRARIES", nullptr,
                                               nullptr);
    // The $<LINK_ONLY> expression may be used to specify link dependencies
    // that are otherwise excluded from usage requirements.
    if (implFor == LinkInterfaceFor::Usage) {
      dagChecker.SetTransitivePropertiesOnly();
      switch (this->GetPolicyStatusCMP0131()) {
        case cmPolicies::WARN:
        case cmPolicies::OLD:
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          dagChecker.SetTransitivePropertiesOnlyCMP0131();
          break;
      }
    }
    cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance(),
                             entry.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> const cge =
      ge.Parse(entry.Value);
    cge->SetEvaluateForBuildsystem(true);
    std::string const& evaluated =
      cge->Evaluate(this->LocalGenerator, config, head, &dagChecker, nullptr,
                    this->LinkerLanguage);
    bool const checkCMP0027 = evaluated != entry.Value;
    cmList llibs(evaluated);
    if (cge->GetHadHeadSensitiveCondition()) {
      impl.HadHeadSensitiveCondition = true;
    }
    if (cge->GetHadContextSensitiveCondition()) {
      impl.HadContextSensitiveCondition = true;
    }
    if (cge->GetHadLinkLanguageSensitiveCondition()) {
      impl.HadLinkLanguageSensitiveCondition = true;
    }

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& lib : llibs) {
      if (auto maybeLinkFeature = ParseLinkFeature(lib)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (this->IsLinkLookupScope(lib, lg)) {
        continue;
      }

      // Skip entries that resolve to the target itself or are empty.
      std::string name = this->CheckCMP0004(lib);
      if (this->GetPolicyStatusCMP0108() == cmPolicies::NEW) {
        // resolve alias name
        auto* target = this->Makefile->FindTargetToUse(name);
        if (target) {
          name = target->GetName();
        }
      }
      if (name == this->GetName() || name.empty()) {
        if (name == this->GetName()) {
          bool noMessage = false;
          MessageType messageType = MessageType::FATAL_ERROR;
          std::ostringstream e;
          switch (this->GetPolicyStatusCMP0038()) {
            case cmPolicies::WARN: {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0038) << "\n";
              messageType = MessageType::AUTHOR_WARNING;
            } break;
            case cmPolicies::OLD:
              noMessage = true;
              break;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
          }

          if (!noMessage) {
            e << "Target \"" << this->GetName() << "\" links to itself.";
            this->LocalGenerator->GetCMakeInstance()->IssueMessage(
              messageType, e.str(), this->GetBacktrace());
            if (messageType == MessageType::FATAL_ERROR) {
              return;
            }
          }
        }
        continue;
      }

      // The entry is meant for this configuration.
      cmLinkItem item = this->ResolveLinkItem(
        BT<std::string>(name, entry.Backtrace), lg, linkFeature);
      if (item.Target) {
        auto depsForTarget = synthTargetsForConfig.find(item.Target);
        if (depsForTarget != synthTargetsForConfig.end()) {
          for (auto const* depForTarget : depsForTarget->second) {
            cmLinkItem synthItem(depForTarget, item.Cross, item.Backtrace);
            impl.Libraries.emplace_back(std::move(synthItem), false);
          }
        }
      } else {
        // Report explicitly linked object files separately.
        std::string const& maybeObj = item.AsStr();
        if (cmSystemTools::FileIsFullPath(maybeObj)) {
          cmSourceFile const* sf =
            mf->GetSource(maybeObj, cmSourceFileLocationKind::Known);
          if (sf && sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
            item.ObjectSource = sf;
            impl.Objects.emplace_back(std::move(item));
            continue;
          }
        }
      }

      impl.Libraries.emplace_back(std::move(item), checkCMP0027);
    }

    std::set<std::string> const& seenProps = cge->GetSeenTargetProperties();
    for (std::string const& sp : seenProps) {
      if (!this->GetProperty(sp)) {
        this->LinkImplicitNullProperties.insert(sp);
      }
    }
    cge->GetMaxLanguageStandard(this, this->MaxLanguageStandards);
  }

  // Update the list of direct link dependencies from usage requirements.
  if (head == this) {
    ComputeLinkImplTransitive(this, config, implFor, impl);
  }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  cmTargetLinkLibraryType linkType =
    CMP0003_ComputeLinkType(config, debugConfigs);
  cmTarget::LinkLibraryVectorType const& oldllibs =
    this->Target->GetOriginalLinkLibraries();

  auto linkFeature = cmLinkItem::DEFAULT;
  for (cmTarget::LibraryID const& oldllib : oldllibs) {
    if (auto maybeLinkFeature = ParseLinkFeature(oldllib.first)) {
      linkFeature = std::move(*maybeLinkFeature);
      continue;
    }

    if (oldllib.second != GENERAL_LibraryType && oldllib.second != linkType) {
      std::string name = this->CheckCMP0004(oldllib.first);
      if (name == this->GetName() || name.empty()) {
        continue;
      }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(
        this->ResolveLinkItem(BT<std::string>(name), linkFeature));
    }
  }
}

cmGeneratorTarget::TargetOrString cmGeneratorTarget::ResolveTargetReference(
  std::string const& name) const
{
  return this->ResolveTargetReference(name, this->LocalGenerator);
}

cmGeneratorTarget::TargetOrString cmGeneratorTarget::ResolveTargetReference(
  std::string const& name, cmLocalGenerator const* lg) const
{
  TargetOrString resolved;

  if (cmGeneratorTarget* tgt = lg->FindGeneratorTargetToUse(name)) {
    resolved.Target = tgt;
  } else {
    resolved.String = name;
  }

  return resolved;
}

cmLinkItem cmGeneratorTarget::ResolveLinkItem(
  BT<std::string> const& name, std::string const& linkFeature) const
{
  return this->ResolveLinkItem(name, this->LocalGenerator, linkFeature);
}

cmLinkItem cmGeneratorTarget::ResolveLinkItem(
  BT<std::string> const& name, cmLocalGenerator const* lg,
  std::string const& linkFeature) const
{
  auto bt = name.Backtrace;
  TargetOrString resolved = this->ResolveTargetReference(name.Value, lg);

  if (!resolved.Target) {
    return cmLinkItem(resolved.String, false, bt, linkFeature);
  }

  // Check deprecation, issue message with `bt` backtrace.
  if (resolved.Target->IsDeprecated()) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      "The library that is being linked to, "  << resolved.Target->GetName() <<
      ", is marked as being deprecated by the owner.  The message provided by "
      "the developer is: \n" << resolved.Target->GetDeprecation() << "\n";
    /* clang-format on */
    this->LocalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::AUTHOR_WARNING, w.str(), bt);
  }

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if (resolved.Target->GetType() == cmStateEnums::EXECUTABLE &&
      !resolved.Target->IsExecutableWithExports()) {
    return cmLinkItem(resolved.Target->GetName(), false, bt, linkFeature);
  }

  return cmLinkItem(resolved.Target, false, bt, linkFeature);
}

bool cmGeneratorTarget::HasPackageReferences() const
{
  return this->IsInBuildSystem() &&
    !this->GetProperty("VS_PACKAGE_REFERENCES")->empty();
}

std::vector<std::string> cmGeneratorTarget::GetPackageReferences() const
{
  cmList packageReferences;

  if (this->IsInBuildSystem()) {
    if (cmValue vsPackageReferences =
          this->GetProperty("VS_PACKAGE_REFERENCES")) {
      packageReferences.assign(*vsPackageReferences);
    }
  }

  return std::move(packageReferences.data());
}

std::string cmGeneratorTarget::GetPDBDirectory(const std::string& config) const
{
  if (OutputInfo const* info = this->GetOutputInfo(config)) {
    // Return the directory in which the target will be built.
    return info->PdbDir;
  }
  return "";
}

bool cmGeneratorTarget::HasImplibGNUtoMS(std::string const& config) const
{
  return this->HasImportLibrary(config) && this->GetPropertyAsBool("GNUtoMS");
}

bool cmGeneratorTarget::GetImplibGNUtoMS(std::string const& config,
                                         std::string const& gnuName,
                                         std::string& out,
                                         const char* newExt) const
{
  if (this->HasImplibGNUtoMS(config) && gnuName.size() > 6 &&
      gnuName.substr(gnuName.size() - 6) == ".dll.a") {
    out = cmStrCat(cm::string_view(gnuName).substr(0, gnuName.size() - 6),
                   newExt ? newExt : ".lib");
    return true;
  }
  return false;
}

bool cmGeneratorTarget::HasContextDependentSources() const
{
  return this->SourcesAreContextDependent == Tribool::True;
}

bool cmGeneratorTarget::IsExecutableWithExports() const
{
  return this->Target->IsExecutableWithExports();
}

bool cmGeneratorTarget::IsSharedLibraryWithExports() const
{
  return this->Target->IsSharedLibraryWithExports();
}

bool cmGeneratorTarget::HasImportLibrary(std::string const& config) const
{
  bool generate_Stubs = true;
  if (this->GetGlobalGenerator()->IsXcode()) {
    // take care of CMAKE_XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS variable
    // as well as XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS property
    if (cmValue propGenStubs =
          this->GetProperty("XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS")) {
      generate_Stubs = propGenStubs == "YES";
    } else if (cmValue varGenStubs = this->Makefile->GetDefinition(
                 "CMAKE_XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS")) {
      generate_Stubs = varGenStubs == "YES";
    }
  }

  return (this->IsDLLPlatform() &&
          (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->IsExecutableWithExports()) &&
          // Assemblies which have only managed code do not have
          // import libraries.
          this->GetManagedType(config) != ManagedType::Managed) ||
    (this->IsAIX() && this->IsExecutableWithExports()) ||
    (this->Makefile->PlatformSupportsAppleTextStubs() &&
     this->IsSharedLibraryWithExports() && generate_Stubs);
}

bool cmGeneratorTarget::NeedImportLibraryName(std::string const& config) const
{
  return this->HasImportLibrary(config) ||
    // On DLL platforms we always generate the import library name
    // just in case the sources have export markup.
    (this->IsDLLPlatform() &&
     (this->GetType() == cmStateEnums::EXECUTABLE ||
      this->GetType() == cmStateEnums::MODULE_LIBRARY));
}

std::string cmGeneratorTarget::GetSupportDirectory() const
{
  std::string dir = cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(),
                             "/CMakeFiles/", this->GetName());
#if defined(__VMS)
  dir += "_dir";
#else
  dir += ".dir";
#endif
  return dir;
}

bool cmGeneratorTarget::IsLinkable() const
{
  return (this->GetType() == cmStateEnums::STATIC_LIBRARY ||
          this->GetType() == cmStateEnums::SHARED_LIBRARY ||
          this->GetType() == cmStateEnums::MODULE_LIBRARY ||
          this->GetType() == cmStateEnums::UNKNOWN_LIBRARY ||
          this->GetType() == cmStateEnums::OBJECT_LIBRARY ||
          this->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
          this->IsExecutableWithExports());
}

bool cmGeneratorTarget::HasLinkDependencyFile(std::string const& config) const
{
  if (this->GetType() != cmStateEnums::EXECUTABLE &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY) {
    return false;
  }

  if (this->Target->GetProperty("LINK_DEPENDS_NO_SHARED").IsOn()) {
    // Do not use the linker dependency file because it includes shared
    // libraries as well
    return false;
  }

  const std::string depsUseLinker{ "CMAKE_LINK_DEPENDS_USE_LINKER" };
  auto linkLanguage = this->GetLinkerLanguage(config);
  const std::string langDepsUseLinker{ cmStrCat("CMAKE_", linkLanguage,
                                                "_LINK_DEPENDS_USE_LINKER") };

  return (!this->Makefile->IsDefinitionSet(depsUseLinker) ||
          this->Makefile->IsOn(depsUseLinker)) &&
    this->Makefile->IsOn(langDepsUseLinker);
}

bool cmGeneratorTarget::IsFrameworkOnApple() const
{
  return this->Target->IsFrameworkOnApple();
}

bool cmGeneratorTarget::IsImportedFrameworkFolderOnApple(
  const std::string& config) const
{
  if (this->IsApple() && this->IsImported() &&
      (this->GetType() == cmStateEnums::STATIC_LIBRARY ||
       this->GetType() == cmStateEnums::SHARED_LIBRARY ||
       this->GetType() == cmStateEnums::UNKNOWN_LIBRARY)) {
    std::string cfg = config;
    if (cfg.empty() && this->GetGlobalGenerator()->IsXcode()) {
      // FIXME(#25515): Remove the need for this workaround.
      // The Xcode generator queries include directories without any
      // specific configuration.  Pick one in case this target does
      // not set either IMPORTED_LOCATION or IMPORTED_CONFIGURATIONS.
      cfg =
        this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig)[0];
    }
    return cmSystemTools::IsPathToFramework(this->GetLocation(cfg));
  }

  return false;
}

bool cmGeneratorTarget::IsAppBundleOnApple() const
{
  return this->Target->IsAppBundleOnApple();
}

bool cmGeneratorTarget::IsXCTestOnApple() const
{
  return (this->IsCFBundleOnApple() && this->GetPropertyAsBool("XCTEST"));
}

bool cmGeneratorTarget::IsCFBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::MODULE_LIBRARY && this->IsApple() &&
          this->GetPropertyAsBool("BUNDLE"));
}

cmGeneratorTarget::ManagedType cmGeneratorTarget::CheckManagedType(
  std::string const& propval) const
{
  // The type of the managed assembly (mixed unmanaged C++ and C++/CLI,
  // or only C++/CLI) does only depend on whether the property is an empty
  // string or contains any value at all. In Visual Studio generators
  // this propval is prepended with /clr[:] which results in:
  //
  // 1. propval does not exist: no /clr flag, unmanaged target, has import
  //                            lib
  // 2. empty propval:          add /clr as flag, mixed unmanaged/managed
  //                            target, has import lib
  // 3. netcore propval:        add /clr:netcore as flag, mixed
  //                            unmanaged/managed target, has import lib.
  // 4. any value (safe,pure):  add /clr:[propval] as flag, target with
  //                            managed code only, no import lib
  if (propval.empty() || propval == "netcore") {
    return ManagedType::Mixed;
  }
  return ManagedType::Managed;
}

cmGeneratorTarget::ManagedType cmGeneratorTarget::GetManagedType(
  const std::string& config) const
{
  // Only libraries and executables can be managed targets.
  if (this->GetType() > cmStateEnums::SHARED_LIBRARY) {
    return ManagedType::Undefined;
  }

  if (this->GetType() == cmStateEnums::STATIC_LIBRARY) {
    return ManagedType::Native;
  }

  // Check imported target.
  if (this->IsImported()) {
    if (cmGeneratorTarget::ImportInfo const* info =
          this->GetImportInfo(config)) {
      return info->Managed;
    }
    return ManagedType::Undefined;
  }

  // Check for explicitly set clr target property.
  if (cmValue clr = this->GetProperty("COMMON_LANGUAGE_RUNTIME")) {
    return this->CheckManagedType(*clr);
  }

  // C# targets are always managed. This language specific check
  // is added to avoid that the COMMON_LANGUAGE_RUNTIME target property
  // has to be set manually for C# targets.
  return this->IsCSharpOnly() ? ManagedType::Managed : ManagedType::Native;
}

bool cmGeneratorTarget::AddHeaderSetVerification()
{
  if (!this->GetPropertyAsBool("VERIFY_INTERFACE_HEADER_SETS")) {
    return true;
  }

  if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::UNKNOWN_LIBRARY &&
      this->GetType() != cmStateEnums::OBJECT_LIBRARY &&
      this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      !this->IsExecutableWithExports()) {
    return true;
  }

  auto verifyValue = this->GetProperty("INTERFACE_HEADER_SETS_TO_VERIFY");
  const bool all = verifyValue.IsEmpty();
  std::set<std::string> verifySet;
  if (!all) {
    cmList verifyList{ verifyValue };
    verifySet.insert(verifyList.begin(), verifyList.end());
  }

  cmTarget* verifyTarget = nullptr;
  cmTarget* allVerifyTarget =
    this->GlobalGenerator->GetMakefiles().front()->FindTargetToUse(
      "all_verify_interface_header_sets", true);

  auto interfaceFileSetEntries = this->Target->GetInterfaceHeaderSetsEntries();

  std::set<cmFileSet*> fileSets;
  for (auto const& entry : interfaceFileSetEntries) {
    for (auto const& name : cmList{ entry.Value }) {
      if (all || verifySet.count(name)) {
        fileSets.insert(this->Target->GetFileSet(name));
        verifySet.erase(name);
      }
    }
  }
  if (!verifySet.empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Property INTERFACE_HEADER_SETS_TO_VERIFY of target \"",
               this->GetName(),
               "\" contained the following header sets that are nonexistent "
               "or not INTERFACE:\n  ",
               cmJoin(verifySet, "\n  ")));
    return false;
  }

  cm::optional<std::set<std::string>> languages;
  for (auto* fileSet : fileSets) {
    auto dirCges = fileSet->CompileDirectoryEntries();
    auto fileCges = fileSet->CompileFileEntries();

    static auto const contextSensitive =
      [](const std::unique_ptr<cmCompiledGeneratorExpression>& cge) {
        return cge->GetHadContextSensitiveCondition();
      };
    bool dirCgesContextSensitive = false;
    bool fileCgesContextSensitive = false;

    std::vector<std::string> dirs;
    std::map<std::string, std::vector<std::string>> filesPerDir;
    bool first = true;
    for (auto const& config : this->Makefile->GetGeneratorConfigs(
           cmMakefile::GeneratorConfigQuery::IncludeEmptyConfig)) {
      if (first || dirCgesContextSensitive) {
        dirs = fileSet->EvaluateDirectoryEntries(dirCges, this->LocalGenerator,
                                                 config, this);
        dirCgesContextSensitive =
          std::any_of(dirCges.begin(), dirCges.end(), contextSensitive);
      }
      if (first || fileCgesContextSensitive) {
        filesPerDir.clear();
        for (auto const& fileCge : fileCges) {
          fileSet->EvaluateFileEntry(dirs, filesPerDir, fileCge,
                                     this->LocalGenerator, config, this);
          if (fileCge->GetHadContextSensitiveCondition()) {
            fileCgesContextSensitive = true;
          }
        }
      }

      for (auto const& files : filesPerDir) {
        for (auto const& file : files.second) {
          std::string filename = this->GenerateHeaderSetVerificationFile(
            *this->Makefile->GetOrCreateSource(file), files.first, languages);
          if (filename.empty()) {
            continue;
          }

          if (!verifyTarget) {
            {
              cmMakefile::PolicyPushPop polScope(this->Makefile);
              this->Makefile->SetPolicy(cmPolicies::CMP0119, cmPolicies::NEW);
              verifyTarget = this->Makefile->AddLibrary(
                cmStrCat(this->GetName(), "_verify_interface_header_sets"),
                cmStateEnums::OBJECT_LIBRARY, {}, true);
            }

            verifyTarget->AddLinkLibrary(
              *this->Makefile, this->GetName(),
              cmTargetLinkLibraryType::GENERAL_LibraryType);
            verifyTarget->SetProperty("AUTOMOC", "OFF");
            verifyTarget->SetProperty("AUTORCC", "OFF");
            verifyTarget->SetProperty("AUTOUIC", "OFF");
            verifyTarget->SetProperty("DISABLE_PRECOMPILE_HEADERS", "ON");
            verifyTarget->SetProperty("UNITY_BUILD", "OFF");
            verifyTarget->SetProperty("CXX_SCAN_FOR_MODULES", "OFF");
            cm::optional<std::map<std::string, cmValue>>
              perConfigCompileDefinitions;
            verifyTarget->FinalizeTargetConfiguration(
              this->Makefile->GetCompileDefinitionsEntries(),
              perConfigCompileDefinitions);

            if (!allVerifyTarget) {
              allVerifyTarget = this->GlobalGenerator->GetMakefiles()
                                  .front()
                                  ->AddNewUtilityTarget(
                                    "all_verify_interface_header_sets", true);
            }

            allVerifyTarget->AddUtility(verifyTarget->GetName(), false);
          }

          if (fileCgesContextSensitive) {
            filename = cmStrCat("$<$<CONFIG:", config, ">:", filename, ">");
          }
          verifyTarget->AddSource(filename);
        }
      }

      if (!dirCgesContextSensitive && !fileCgesContextSensitive) {
        break;
      }
      first = false;
    }
  }

  if (verifyTarget) {
    this->LocalGenerator->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(verifyTarget, this->LocalGenerator));
  }

  return true;
}

std::string cmGeneratorTarget::GenerateHeaderSetVerificationFile(
  cmSourceFile& source, const std::string& dir,
  cm::optional<std::set<std::string>>& languages) const
{
  std::string extension;
  std::string language = source.GetOrDetermineLanguage();

  if (source.GetPropertyAsBool("SKIP_LINTING")) {
    return std::string{};
  }

  if (language.empty()) {
    if (!languages) {
      languages.emplace();
      for (auto const& tgtSource : this->GetAllConfigSources()) {
        auto const& tgtSourceLanguage =
          tgtSource.Source->GetOrDetermineLanguage();
        if (tgtSourceLanguage == "CXX") {
          languages->insert("CXX");
          break; // C++ overrides everything else, so we don't need to keep
                 // checking.
        }
        if (tgtSourceLanguage == "C") {
          languages->insert("C");
        }
      }

      if (languages->empty()) {
        std::vector<std::string> languagesVector;
        this->GlobalGenerator->GetEnabledLanguages(languagesVector);
        languages->insert(languagesVector.begin(), languagesVector.end());
      }
    }

    if (languages->count("CXX")) {
      language = "CXX";
    } else if (languages->count("C")) {
      language = "C";
    }
  }

  if (language == "C") {
    extension = ".c";
  } else if (language == "CXX") {
    extension = ".cxx";
  } else {
    return "";
  }

  std::string headerFilename = dir;
  if (!headerFilename.empty()) {
    headerFilename += '/';
  }
  headerFilename += source.GetLocation().GetName();

  auto filename = cmStrCat(
    this->LocalGenerator->GetCurrentBinaryDirectory(), '/', this->GetName(),
    "_verify_interface_header_sets/", headerFilename, extension);
  auto* verificationSource = this->Makefile->GetOrCreateSource(filename);
  verificationSource->SetProperty("LANGUAGE", language);

  cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(filename));

  cmGeneratedFileStream fout(filename);
  fout.SetCopyIfDifferent(true);
  // The IWYU "associated" pragma tells include-what-you-use to
  // consider the headerFile as part of the entire language
  // unit within include-what-you-use and as a result allows
  // one to get IWYU advice for headers.
  fout << "#include <" << headerFilename << "> // IWYU pragma: associated\n";
  fout.close();

  return filename;
}

std::string cmGeneratorTarget::GetImportedXcFrameworkPath(
  const std::string& config) const
{
  if (!(this->IsApple() && this->IsImported() &&
        (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
         this->GetType() == cmStateEnums::STATIC_LIBRARY ||
         this->GetType() == cmStateEnums::UNKNOWN_LIBRARY))) {
    return {};
  }

  std::string desiredConfig = config;
  if (config.empty()) {
    desiredConfig = "NOCONFIG";
  }

  std::string result;

  cmValue loc = nullptr;
  cmValue imp = nullptr;
  std::string suffix;

  if (this->Target->GetMappedConfig(desiredConfig, loc, imp, suffix)) {
    if (loc) {
      result = *loc;
    } else {
      std::string impProp = cmStrCat("IMPORTED_LOCATION", suffix);
      if (cmValue configLocation = this->GetProperty(impProp)) {
        result = *configLocation;
      } else if (cmValue location = this->GetProperty("IMPORTED_LOCATION")) {
        result = *location;
      }
    }

    if (cmSystemTools::IsPathToXcFramework(result)) {
      return result;
    }
  }

  return {};
}

bool cmGeneratorTarget::HaveFortranSources(std::string const& config) const
{
  auto sources = this->GetSourceFiles(config);
  bool const have_direct = std::any_of(
    sources.begin(), sources.end(), [](BT<cmSourceFile*> const& sf) -> bool {
      return sf.Value->GetLanguage() == "Fortran"_s;
    });
  bool have_via_target_objects = false;
  if (!have_direct) {
    auto const sourceObjectLibraries = this->GetSourceObjectLibraries(config);
    have_via_target_objects =
      std::any_of(sourceObjectLibraries.begin(), sourceObjectLibraries.end(),
                  [&config](cmGeneratorTarget const* tgt) -> bool {
                    return tgt->HaveFortranSources(config);
                  });
  }
  return have_direct || have_via_target_objects;
}

bool cmGeneratorTarget::HaveFortranSources() const
{
  auto sources = this->GetAllConfigSources();
  bool const have_direct = std::any_of(
    sources.begin(), sources.end(), [](AllConfigSource const& sf) -> bool {
      return sf.Source->GetLanguage() == "Fortran"_s;
    });
  bool have_via_target_objects = false;
  if (!have_direct) {
    std::vector<std::string> configs =
      this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
    for (auto const& config : configs) {
      auto const sourceObjectLibraries =
        this->GetSourceObjectLibraries(config);
      have_via_target_objects =
        std::any_of(sourceObjectLibraries.begin(), sourceObjectLibraries.end(),
                    [&config](cmGeneratorTarget const* tgt) -> bool {
                      return tgt->HaveFortranSources(config);
                    });
      if (have_via_target_objects) {
        break;
      }
    }
  }
  return have_direct || have_via_target_objects;
}

bool cmGeneratorTarget::HaveCxx20ModuleSources(std::string* errorMessage) const
{
  auto const& fs_names = this->Target->GetAllFileSetNames();
  return std::any_of(
    fs_names.begin(), fs_names.end(),
    [this, errorMessage](std::string const& name) -> bool {
      auto const* file_set = this->Target->GetFileSet(name);
      if (!file_set) {
        auto message = cmStrCat("Target \"", this->Target->GetName(),
                                "\" is tracked to have file set \"", name,
                                "\", but it was not found.");
        if (errorMessage) {
          *errorMessage = std::move(message);
        } else {
          this->Makefile->IssueMessage(MessageType::INTERNAL_ERROR, message);
        }
        return false;
      }

      auto const& fs_type = file_set->GetType();
      return fs_type == "CXX_MODULES"_s;
    });
}

cmGeneratorTarget::Cxx20SupportLevel cmGeneratorTarget::HaveCxxModuleSupport(
  std::string const& config) const
{
  auto const* state = this->Makefile->GetState();
  if (!state->GetLanguageEnabled("CXX")) {
    return Cxx20SupportLevel::MissingCxx;
  }

  cmValue standardDefault =
    this->Makefile->GetDefinition("CMAKE_CXX_STANDARD_DEFAULT");
  if (!standardDefault || standardDefault->empty()) {
    // We do not know any meaningful C++ standard levels for this compiler.
    return Cxx20SupportLevel::NoCxx20;
  }

  cmStandardLevelResolver standardResolver(this->Makefile);
  cmStandardLevel const cxxStd20 =
    *standardResolver.LanguageStandardLevel("CXX", "20");
  cm::optional<cmStandardLevel> explicitLevel =
    this->GetExplicitStandardLevel("CXX", config);
  if (!explicitLevel || *explicitLevel < cxxStd20) {
    return Cxx20SupportLevel::NoCxx20;
  }

  cmValue scandepRule =
    this->Makefile->GetDefinition("CMAKE_CXX_SCANDEP_SOURCE");
  if (!scandepRule) {
    return Cxx20SupportLevel::MissingRule;
  }
  return Cxx20SupportLevel::Supported;
}

void cmGeneratorTarget::CheckCxxModuleStatus(std::string const& config) const
{
  bool haveScannableSources = false;

  // Check for `CXX_MODULE*` file sets and a lack of support.
  if (this->HaveCxx20ModuleSources()) {
    haveScannableSources = true;
  }

  if (!haveScannableSources) {
    // Check to see if there are regular sources that have requested scanning.
    auto sources = this->GetSourceFiles(config);
    for (auto const& source : sources) {
      auto const* sf = source.Value;
      auto const& lang = sf->GetLanguage();
      if (lang != "CXX"_s) {
        continue;
      }
      // Ignore sources which do not need dyndep.
      if (this->NeedDyndepForSource(lang, config, sf)) {
        haveScannableSources = true;
      }
    }
  }

  // If there isn't anything scannable, ignore it.
  if (!haveScannableSources) {
    return;
  }

  // If the generator doesn't support modules at all, error that we have
  // sources that require the support.
  if (!this->GetGlobalGenerator()->CheckCxxModuleSupport(
        cmGlobalGenerator::CxxModuleSupportQuery::Expected)) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("The target named \"", this->GetName(),
               "\" has C++ sources that may use modules, but modules are not "
               "supported by this generator:\n  ",
               this->GetGlobalGenerator()->GetName(), '\n',
               "Modules are supported only by Ninja, Ninja Multi-Config, "
               "and Visual Studio generators for VS 17.4 and newer.  "
               "See the cmake-cxxmodules(7) manual for details.  "
               "Use the CMAKE_CXX_SCAN_FOR_MODULES variable to enable or "
               "disable scanning."));
    return;
  }

  switch (this->HaveCxxModuleSupport(config)) {
    case cmGeneratorTarget::Cxx20SupportLevel::MissingCxx:
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("The target named \"", this->GetName(),
                 "\" has C++ sources that use modules, but the \"CXX\" "
                 "language has not been enabled."));
      break;
    case cmGeneratorTarget::Cxx20SupportLevel::NoCxx20: {
      cmStandardLevelResolver standardResolver(this->Makefile);
      auto effStandard =
        standardResolver.GetEffectiveStandard(this, "CXX", config);
      if (effStandard.empty()) {
        effStandard = "; no C++ standard found";
      } else {
        effStandard = cmStrCat("; found \"cxx_std_", effStandard, '"');
      }
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(
          "The target named \"", this->GetName(),
          "\" has C++ sources that use modules, but does not include "
          "\"cxx_std_20\" (or newer) among its `target_compile_features`",
          effStandard, '.'));
    } break;
    case cmGeneratorTarget::Cxx20SupportLevel::MissingRule: {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("The target named \"", this->GetName(),
                 "\" has C++ sources that may use modules, but the compiler "
                 "does not provide a way to discover the import graph "
                 "dependencies.  See the cmake-cxxmodules(7) manual for "
                 "details.  Use the CMAKE_CXX_SCAN_FOR_MODULES variable to "
                 "enable or disable scanning."));
    } break;
    case cmGeneratorTarget::Cxx20SupportLevel::Supported:
      // All is well.
      break;
  }
}

bool cmGeneratorTarget::NeedCxxModuleSupport(std::string const& lang,
                                             std::string const& config) const
{
  if (lang != "CXX"_s) {
    return false;
  }
  return this->HaveCxxModuleSupport(config) == Cxx20SupportLevel::Supported &&
    this->GetGlobalGenerator()->CheckCxxModuleSupport(
      cmGlobalGenerator::CxxModuleSupportQuery::Inspect);
}

bool cmGeneratorTarget::NeedDyndep(std::string const& lang,
                                   std::string const& config) const
{
  return lang == "Fortran"_s || this->NeedCxxModuleSupport(lang, config);
}

cmFileSet const* cmGeneratorTarget::GetFileSetForSource(
  std::string const& config, cmSourceFile const* sf) const
{
  this->BuildFileSetInfoCache(config);

  auto const& path = sf->GetFullPath();
  auto const& per_config = this->Configs[config];

  auto const fsit = per_config.FileSetCache.find(path);
  if (fsit == per_config.FileSetCache.end()) {
    return nullptr;
  }
  return fsit->second;
}

bool cmGeneratorTarget::NeedDyndepForSource(std::string const& lang,
                                            std::string const& config,
                                            cmSourceFile const* sf) const
{
  // Fortran always needs to be scanned.
  if (lang == "Fortran"_s) {
    return true;
  }
  // Only C++ code needs scanned otherwise.
  if (lang != "CXX"_s) {
    return false;
  }

  // Any file in `CXX_MODULES` file sets need scanned (it being `CXX` is
  // enforced elsewhere).
  auto const* fs = this->GetFileSetForSource(config, sf);
  if (fs && fs->GetType() == "CXX_MODULES"_s) {
    return true;
  }

  auto targetDyndep = this->NeedCxxDyndep(config);
  if (targetDyndep == CxxModuleSupport::Unavailable) {
    return false;
  }
  auto const sfProp = sf->GetProperty("CXX_SCAN_FOR_MODULES");
  if (sfProp.IsSet()) {
    return sfProp.IsOn();
  }
  return targetDyndep == CxxModuleSupport::Enabled;
}

cmGeneratorTarget::CxxModuleSupport cmGeneratorTarget::NeedCxxDyndep(
  std::string const& config) const
{
  bool haveRule = false;
  switch (this->HaveCxxModuleSupport(config)) {
    case Cxx20SupportLevel::MissingCxx:
    case Cxx20SupportLevel::NoCxx20:
      return CxxModuleSupport::Unavailable;
    case Cxx20SupportLevel::MissingRule:
      break;
    case Cxx20SupportLevel::Supported:
      haveRule = true;
      break;
  }
  bool haveGeneratorSupport =
    this->GetGlobalGenerator()->CheckCxxModuleSupport(
      cmGlobalGenerator::CxxModuleSupportQuery::Inspect);
  auto const tgtProp = this->GetProperty("CXX_SCAN_FOR_MODULES");
  if (tgtProp.IsSet()) {
    return tgtProp.IsOn() ? CxxModuleSupport::Enabled
                          : CxxModuleSupport::Disabled;
  }

  CxxModuleSupport policyAnswer = CxxModuleSupport::Unavailable;
  switch (this->GetPolicyStatusCMP0155()) {
    case cmPolicies::WARN:
    case cmPolicies::OLD:
      // The OLD behavior is to not scan the source.
      policyAnswer = CxxModuleSupport::Disabled;
      break;
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::NEW:
      // The NEW behavior is to scan the source if the compiler supports
      // scanning and the generator supports it.
      if (haveRule && haveGeneratorSupport) {
        policyAnswer = CxxModuleSupport::Enabled;
      } else {
        policyAnswer = CxxModuleSupport::Disabled;
      }
      break;
  }
  return policyAnswer;
}

void cmGeneratorTarget::BuildFileSetInfoCache(std::string const& config) const
{
  auto& per_config = this->Configs[config];

  if (per_config.BuiltFileSetCache) {
    return;
  }

  auto const* tgt = this->Target;

  for (auto const& name : tgt->GetAllFileSetNames()) {
    auto const* file_set = tgt->GetFileSet(name);
    if (!file_set) {
      tgt->GetMakefile()->IssueMessage(
        MessageType::INTERNAL_ERROR,
        cmStrCat("Target \"", tgt->GetName(),
                 "\" is tracked to have file set \"", name,
                 "\", but it was not found."));
      continue;
    }

    auto fileEntries = file_set->CompileFileEntries();
    auto directoryEntries = file_set->CompileDirectoryEntries();
    auto directories = file_set->EvaluateDirectoryEntries(
      directoryEntries, this->LocalGenerator, config, this);

    std::map<std::string, std::vector<std::string>> files;
    for (auto const& entry : fileEntries) {
      file_set->EvaluateFileEntry(directories, files, entry,
                                  this->LocalGenerator, config, this);
    }

    for (auto const& it : files) {
      for (auto const& filename : it.second) {
        auto collapsedFile = cmSystemTools::CollapseFullPath(filename);
        per_config.FileSetCache[collapsedFile] = file_set;
      }
    }
  }

  per_config.BuiltFileSetCache = true;
}
