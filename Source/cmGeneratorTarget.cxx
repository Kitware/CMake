/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorTarget.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmCustomCommandLines.h"
#include "cmFileTimes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionNode.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPropertyMap.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmTargetPropertyComputer.h"
#include "cmake.h"

class cmMessenger;

template <>
const char* cmTargetPropertyComputer::GetSources<cmGeneratorTarget>(
  cmGeneratorTarget const* tgt, cmMessenger* /* messenger */,
  cmListFileBacktrace const& /* context */)
{
  return tgt->GetSourcesProperty().c_str();
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

class cmGeneratorTarget::TargetPropertyEntry
{
protected:
  static cmLinkImplItem NoLinkImplItem;

public:
  TargetPropertyEntry(cmLinkImplItem const& item)
    : LinkImplItem(item)
  {
  }
  virtual ~TargetPropertyEntry() = default;

  virtual const std::string& Evaluate(
    cmLocalGenerator* lg, const std::string& config,
    cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker,
    std::string const& language) const = 0;

  virtual cmListFileBacktrace GetBacktrace() const = 0;
  virtual std::string const& GetInput() const = 0;
  virtual bool GetHadContextSensitiveCondition() const { return false; }

  cmLinkImplItem const& LinkImplItem;
};
cmLinkImplItem cmGeneratorTarget::TargetPropertyEntry::NoLinkImplItem;

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

class TargetPropertyEntryString : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryString(std::string propertyValue,
                            cmListFileBacktrace backtrace,
                            cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , PropertyValue(std::move(propertyValue))
    , Backtrace(std::move(backtrace))
  {
  }

  const std::string& Evaluate(cmLocalGenerator*, const std::string&,
                              cmGeneratorTarget const*,
                              cmGeneratorExpressionDAGChecker*,
                              std::string const&) const override
  {
    return this->PropertyValue;
  }

  cmListFileBacktrace GetBacktrace() const override { return this->Backtrace; }
  std::string const& GetInput() const override { return this->PropertyValue; }

private:
  std::string PropertyValue;
  cmListFileBacktrace Backtrace;
};

std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>
CreateTargetPropertyEntry(
  const std::string& propertyValue,
  cmListFileBacktrace backtrace = cmListFileBacktrace(),
  bool evaluateForBuildsystem = false)
{
  if (cmGeneratorExpression::Find(propertyValue) != std::string::npos) {
    cmGeneratorExpression ge(std::move(backtrace));
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(propertyValue);
    cge->SetEvaluateForBuildsystem(evaluateForBuildsystem);
    return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
      cm::make_unique<TargetPropertyEntryGenex>(std::move(cge)));
  }

  return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
    cm::make_unique<TargetPropertyEntryString>(propertyValue,
                                               std::move(backtrace)));
}

void CreatePropertyGeneratorExpressions(
  cmStringRange entries, cmBacktraceRange backtraces,
  std::vector<std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>>& items,
  bool evaluateForBuildsystem = false)
{
  auto btIt = backtraces.begin();
  for (auto it = entries.begin(); it != entries.end(); ++it, ++btIt) {
    items.push_back(
      CreateTargetPropertyEntry(*it, *btIt, evaluateForBuildsystem));
  }
}

namespace {
// Represent a target property entry after evaluating generator expressions
// and splitting up lists.
struct EvaluatedTargetPropertyEntry
{
  EvaluatedTargetPropertyEntry(cmLinkImplItem const& item,
                               cmListFileBacktrace bt)
    : LinkImplItem(item)
    , Backtrace(std::move(bt))
  {
  }

  // Move-only.
  EvaluatedTargetPropertyEntry(EvaluatedTargetPropertyEntry&&) = default;
  EvaluatedTargetPropertyEntry(EvaluatedTargetPropertyEntry const&) = delete;
  EvaluatedTargetPropertyEntry& operator=(EvaluatedTargetPropertyEntry&&) =
    delete;
  EvaluatedTargetPropertyEntry& operator=(
    EvaluatedTargetPropertyEntry const&) = delete;

  cmLinkImplItem const& LinkImplItem;
  cmListFileBacktrace Backtrace;
  std::vector<std::string> Values;
  bool ContextDependent = false;
};

EvaluatedTargetPropertyEntry EvaluateTargetPropertyEntry(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget::TargetPropertyEntry& entry)
{
  EvaluatedTargetPropertyEntry ee(entry.LinkImplItem, entry.GetBacktrace());
  cmExpandList(entry.Evaluate(thisTarget->GetLocalGenerator(), config,
                              thisTarget, dagChecker, lang),
               ee.Values);
  if (entry.GetHadContextSensitiveCondition()) {
    ee.ContextDependent = true;
  }
  return ee;
}

std::vector<EvaluatedTargetPropertyEntry> EvaluateTargetPropertyEntries(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>> const&
    in)
{
  std::vector<EvaluatedTargetPropertyEntry> out;
  out.reserve(in.size());
  for (auto& entry : in) {
    out.emplace_back(EvaluateTargetPropertyEntry(thisTarget, config, lang,
                                                 dagChecker, *entry));
  }
  return out;
}
}

cmGeneratorTarget::cmGeneratorTarget(cmTarget* t, cmLocalGenerator* lg)
  : Target(t)
  , FortranModuleDirectoryCreated(false)
  , SourceFileFlagsConstructed(false)
  , PolicyWarnedCMP0022(false)
  , PolicyReportedCMP0069(false)
  , DebugIncludesDone(false)
  , DebugCompileOptionsDone(false)
  , DebugCompileFeaturesDone(false)
  , DebugCompileDefinitionsDone(false)
  , DebugLinkOptionsDone(false)
  , DebugLinkDirectoriesDone(false)
  , DebugPrecompileHeadersDone(false)
  , DebugSourcesDone(false)
  , LinkImplementationLanguageIsContextDependent(true)
  , UtilityItemsDone(false)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = lg;
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();

  this->GlobalGenerator->ComputeTargetObjectDirectory(this);

  CreatePropertyGeneratorExpressions(t->GetIncludeDirectoriesEntries(),
                                     t->GetIncludeDirectoriesBacktraces(),
                                     this->IncludeDirectoriesEntries);

  CreatePropertyGeneratorExpressions(t->GetCompileOptionsEntries(),
                                     t->GetCompileOptionsBacktraces(),
                                     this->CompileOptionsEntries);

  CreatePropertyGeneratorExpressions(t->GetCompileFeaturesEntries(),
                                     t->GetCompileFeaturesBacktraces(),
                                     this->CompileFeaturesEntries);

  CreatePropertyGeneratorExpressions(t->GetCompileDefinitionsEntries(),
                                     t->GetCompileDefinitionsBacktraces(),
                                     this->CompileDefinitionsEntries);

  CreatePropertyGeneratorExpressions(t->GetLinkOptionsEntries(),
                                     t->GetLinkOptionsBacktraces(),
                                     this->LinkOptionsEntries);

  CreatePropertyGeneratorExpressions(t->GetLinkDirectoriesEntries(),
                                     t->GetLinkDirectoriesBacktraces(),
                                     this->LinkDirectoriesEntries);

  CreatePropertyGeneratorExpressions(t->GetPrecompileHeadersEntries(),
                                     t->GetPrecompileHeadersBacktraces(),
                                     this->PrecompileHeadersEntries);

  CreatePropertyGeneratorExpressions(t->GetSourceEntries(),
                                     t->GetSourceBacktraces(),
                                     this->SourceEntries, true);

  this->PolicyMap = t->GetPolicyMap();
}

cmGeneratorTarget::~cmGeneratorTarget() = default;

const std::string& cmGeneratorTarget::GetSourcesProperty() const
{
  std::vector<std::string> values;
  for (auto& se : this->SourceEntries) {
    values.push_back(se->GetInput());
  }
  static std::string value;
  value.clear();
  value = cmJoin(values, ";");
  return value;
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
  const char* exportName = this->GetProperty("EXPORT_NAME");

  if (exportName && *exportName) {
    if (!cmGeneratorExpression::IsValidTargetName(exportName)) {
      std::ostringstream e;
      e << "EXPORT_NAME property \"" << exportName << "\" for \""
        << this->GetName() << "\": is not valid.";
      cmSystemTools::Error(e.str());
      return "";
    }
    return exportName;
  }
  return this->GetName();
}

const char* cmGeneratorTarget::GetProperty(const std::string& prop) const
{
  if (!cmTargetPropertyComputer::PassesWhitelist(
        this->GetType(), prop, this->Makefile->GetMessenger(),
        this->GetBacktrace())) {
    return nullptr;
  }
  if (const char* result = cmTargetPropertyComputer::GetProperty(
        this, prop, this->Makefile->GetMessenger(), this->GetBacktrace())) {
    return result;
  }
  if (cmSystemTools::GetFatalErrorOccured()) {
    return nullptr;
  }
  return this->Target->GetProperty(prop);
}

const char* cmGeneratorTarget::GetSafeProperty(const std::string& prop) const
{
  const char* ret = this->GetProperty(prop);
  if (!ret) {
    return "";
  }
  return ret;
}

const char* cmGeneratorTarget::GetOutputTargetType(
  cmStateEnums::ArtifactType artifact) const
{
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
        // For non-DLL platforms shared libraries are treated as
        // library targets.
        return "LIBRARY";
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
      if (const char* outNameProp = this->GetProperty(p)) {
        outName = outNameProp;
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
    const char* prefix = this->GetFilePrefixInternal(config, artifact);

    return prefix ? prefix : std::string();
  }

  std::string prefix;
  std::string suffix;
  std::string base;
  this->GetFullNameInternal(config, artifact, prefix, base, suffix);
  return prefix;
}
std::string cmGeneratorTarget::GetFileSuffix(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    const char* suffix = this->GetFileSuffixInternal(config, artifact);

    return suffix ? suffix : std::string();
  }

  std::string prefix;
  std::string suffix;
  std::string base;
  this->GetFullNameInternal(config, artifact, prefix, base, suffix);
  return suffix;
}

std::string cmGeneratorTarget::GetFilePostfix(const std::string& config) const
{
  const char* postfix = nullptr;
  if (!config.empty()) {
    std::string configProp =
      cmStrCat(cmSystemTools::UpperCase(config), "_POSTFIX");
    postfix = this->GetProperty(configProp);
    // Mac application bundles and frameworks have no postfix.
    if (!this->IsImported() && postfix &&
        (this->IsAppBundleOnApple() || this->IsFrameworkOnApple())) {
      postfix = nullptr;
    }
  }
  return postfix ? postfix : std::string();
}

const char* cmGeneratorTarget::GetFilePrefixInternal(
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
  const char* targetPrefix =
    (isImportedLibraryArtifact ? this->GetProperty("IMPORT_PREFIX")
                               : this->GetProperty("PREFIX"));

  if (!targetPrefix) {
    const char* prefixVar = this->Target->GetPrefixVariableInternal(artifact);
    if (!language.empty() && prefixVar && *prefixVar) {
      std::string langPrefix = prefixVar + std::string("_") + language;
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
const char* cmGeneratorTarget::GetFileSuffixInternal(
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
  const char* targetSuffix =
    (isImportedLibraryArtifact ? this->GetProperty("IMPORT_SUFFIX")
                               : this->GetProperty("SUFFIX"));

  if (!targetSuffix) {
    const char* suffixVar = this->Target->GetSuffixVariableInternal(artifact);
    if (!language.empty() && suffixVar && *suffixVar) {
      std::string langSuffix = suffixVar + std::string("_") + language;
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
  this->LinkImplementationLanguageIsContextDependent = true;
  this->Objects.clear();
  this->VisitedConfigsForObjects.clear();
}

void cmGeneratorTarget::AddSourceCommon(const std::string& src, bool before)
{
  this->SourceEntries.insert(
    before ? this->SourceEntries.begin() : this->SourceEntries.end(),
    CreateTargetPropertyEntry(src, this->Makefile->GetBacktrace(), true));
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
  this->Target->InsertInclude(src, this->Makefile->GetBacktrace(), before);
  this->IncludeDirectoriesEntries.insert(
    before ? this->IncludeDirectoriesEntries.begin()
           : this->IncludeDirectoriesEntries.end(),
    CreateTargetPropertyEntry(src, this->Makefile->GetBacktrace(), true));
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
                             std::vector<std::string>& result,
                             bool excludeImported, std::string const& language)
{
  if (const char* dirs =
        depTgt->GetProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES")) {
    cmExpandList(cmGeneratorExpression::Evaluate(dirs, lg, config, headTarget,
                                                 dagChecker, depTgt, language),
                 result);
  }
  if (!depTgt->IsImported() || excludeImported) {
    return;
  }

  if (const char* dirs =
        depTgt->GetProperty("INTERFACE_INCLUDE_DIRECTORIES")) {
    cmExpandList(cmGeneratorExpression::Evaluate(dirs, lg, config, headTarget,
                                                 dagChecker, depTgt, language),
                 result);
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
  auto const& configs = this->Makefile->GetGeneratorConfigs();
  std::set<std::string> configSet(configs.begin(), configs.end());
  if (configSet == this->VisitedConfigsForObjects) {
    return;
  }

  for (std::string const& c : configs) {
    std::vector<cmSourceFile const*> sourceFiles;
    this->GetObjectSources(sourceFiles, c);
  }
}

const char* cmGeneratorTarget::GetFeature(const std::string& feature,
                                          const std::string& config) const
{
  if (!config.empty()) {
    std::string featureConfig =
      cmStrCat(feature, '_', cmSystemTools::UpperCase(config));
    if (const char* value = this->GetProperty(featureConfig)) {
      return value;
    }
  }
  if (const char* value = this->GetProperty(feature)) {
    return value;
  }
  return this->LocalGenerator->GetFeature(feature, config);
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
  const char* feature = "INTERPROCEDURAL_OPTIMIZATION";
  const bool result = cmIsOn(this->GetFeature(feature, config));

  if (!result) {
    // 'INTERPROCEDURAL_OPTIMIZATION' is off, no need to check policies
    return false;
  }

  if (lang != "C" && lang != "CXX" && lang != "Fortran") {
    // We do not define IPO behavior for other languages.
    return false;
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
  static std::string extension;
  const bool has_ptx_extension =
    this->GetPropertyAsBool("CUDA_PTX_COMPILATION");
  if (has_ptx_extension) {
    extension = ".ptx";
    return extension.c_str();
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

void cmGeneratorTarget::GetExpectedResxHeaders(std::set<std::string>& headers,
                                               const std::string& config) const
{
  KindedSources const& kinded = this->GetKindedSources(config);
  headers = kinded.ExpectedResxHeaders;
}

void cmGeneratorTarget::GetResxSources(std::vector<cmSourceFile const*>& data,
                                       const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindResx);
}

void cmGeneratorTarget::GetAppManifest(std::vector<cmSourceFile const*>& data,
                                       const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindAppManifest);
}

void cmGeneratorTarget::GetManifests(std::vector<cmSourceFile const*>& data,
                                     const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindManifest);
}

void cmGeneratorTarget::GetCertificates(std::vector<cmSourceFile const*>& data,
                                        const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindCertificate);
}

void cmGeneratorTarget::GetExpectedXamlHeaders(std::set<std::string>& headers,
                                               const std::string& config) const
{
  KindedSources const& kinded = this->GetKindedSources(config);
  headers = kinded.ExpectedXamlHeaders;
}

void cmGeneratorTarget::GetExpectedXamlSources(std::set<std::string>& srcs,
                                               const std::string& config) const
{
  KindedSources const& kinded = this->GetKindedSources(config);
  srcs = kinded.ExpectedXamlSources;
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

void cmGeneratorTarget::GetXamlSources(std::vector<cmSourceFile const*>& data,
                                       const std::string& config) const
{
  IMPLEMENT_VISIT(SourceKindXaml);
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

bool cmGeneratorTarget::IsImported() const
{
  return this->Target->IsImported();
}

bool cmGeneratorTarget::IsImportedGloballyVisible() const
{
  return this->Target->IsImportedGloballyVisible();
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
  location = this->GetDirectory();
  const char* cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if (cfgid && strcmp(cfgid, ".") != 0) {
    location += "/";
    location += cfgid;
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
  assert(this->GetType() != cmStateEnums::INTERFACE_LIBRARY);
  std::string config_upper;
  if (!config.empty()) {
    config_upper = cmSystemTools::UpperCase(config);
  }

  using IncludeCacheType = std::map<std::string, std::vector<std::string>>;
  auto iter = this->SystemIncludesCache.find(config_upper);

  if (iter == this->SystemIncludesCache.end()) {
    cmGeneratorExpressionDAGChecker dagChecker(
      this, "SYSTEM_INCLUDE_DIRECTORIES", nullptr, nullptr);

    bool excludeImported = this->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED");

    std::vector<std::string> result;
    for (std::string const& it : this->Target->GetSystemIncludeDirectories()) {
      cmExpandList(cmGeneratorExpression::Evaluate(it, this->LocalGenerator,
                                                   config, this, &dagChecker,
                                                   nullptr, language),
                   result);
    }

    std::vector<cmGeneratorTarget const*> const& deps =
      this->GetLinkImplementationClosure(config);
    for (cmGeneratorTarget const* dep : deps) {
      handleSystemIncludesDep(this->LocalGenerator, dep, config, this,
                              &dagChecker, result, excludeImported, language);
    }

    std::for_each(result.begin(), result.end(),
                  cmSystemTools::ConvertToUnixSlashes);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    IncludeCacheType::value_type entry(config_upper, result);
    iter = this->SystemIncludesCache.insert(entry).first;
  }

  return std::binary_search(iter->second.begin(), iter->second.end(), dir);
}

bool cmGeneratorTarget::GetPropertyAsBool(const std::string& prop) const
{
  return this->Target->GetPropertyAsBool(prop);
}

bool cmGeneratorTarget::MaybeHaveInterfaceProperty(
  std::string const& prop, cmGeneratorExpressionContext* context,
  bool usage_requirements_only) const
{
  std::string const key = prop + '@' + context->Config;
  auto i = this->MaybeInterfacePropertyExists.find(key);
  if (i == this->MaybeInterfacePropertyExists.end()) {
    // Insert an entry now in case there is a cycle.
    i = this->MaybeInterfacePropertyExists.emplace(key, false).first;
    bool& maybeInterfaceProp = i->second;

    // If this target itself has a non-empty property value, we are done.
    const char* p = this->GetProperty(prop);
    maybeInterfaceProp = p && *p;

    // Otherwise, recurse to interface dependencies.
    if (!maybeInterfaceProp) {
      cmGeneratorTarget const* headTarget =
        context->HeadTarget ? context->HeadTarget : this;
      if (cmLinkInterfaceLibraries const* iface =
            this->GetLinkInterfaceLibraries(context->Config, headTarget,
                                            usage_requirements_only)) {
        if (iface->HadHeadSensitiveCondition) {
          // With a different head target we may get to a library with
          // this interface property.
          maybeInterfaceProp = true;
        } else {
          // The transitive interface libraries do not depend on the
          // head target, so we can follow them.
          for (cmLinkItem const& lib : iface->Libraries) {
            if (lib.Target &&
                lib.Target->MaybeHaveInterfaceProperty(
                  prop, context, usage_requirements_only)) {
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
  bool usage_requirements_only) const
{
  std::string result;

  // If the property does not appear transitively at all, we are done.
  if (!this->MaybeHaveInterfaceProperty(prop, context,
                                        usage_requirements_only)) {
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
      return result;
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      // No error. We have already seen this transitive property.
      return result;
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
  }

  cmGeneratorTarget const* headTarget =
    context->HeadTarget ? context->HeadTarget : this;

  if (const char* p = this->GetProperty(prop)) {
    result = cmGeneratorExpressionNode::EvaluateDependentExpression(
      p, context->LG, context, headTarget, &dagChecker, this);
  }

  if (cmLinkInterfaceLibraries const* iface = this->GetLinkInterfaceLibraries(
        context->Config, headTarget, usage_requirements_only)) {
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
                                                usage_requirements_only));
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
void AddInterfaceEntries(cmGeneratorTarget const* headTarget,
                         std::string const& config, std::string const& prop,
                         std::string const& lang,
                         cmGeneratorExpressionDAGChecker* dagChecker,
                         std::vector<EvaluatedTargetPropertyEntry>& entries,
                         bool usage_requirements_only = true)
{
  if (cmLinkImplementationLibraries const* impl =
        headTarget->GetLinkImplementationLibraries(config)) {
    for (cmLinkImplItem const& lib : impl->Libraries) {
      if (lib.Target) {
        EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
        // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in our
        // caller's property and hand-evaluate it as if it were compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cmGeneratorExpressionContext context(
          headTarget->GetLocalGenerator(), config, false, headTarget,
          headTarget, true, lib.Backtrace, lang);
        cmExpandList(lib.Target->EvaluateInterfaceProperty(
                       prop, &context, dagChecker, usage_requirements_only),
                     ee.Values);
        ee.ContextDependent = context.HadContextSensitiveCondition;
        entries.emplace_back(std::move(ee));
      }
    }
  }
}

void AddObjectEntries(cmGeneratorTarget const* headTarget,
                      std::string const& config,
                      cmGeneratorExpressionDAGChecker* dagChecker,
                      std::vector<EvaluatedTargetPropertyEntry>& entries)
{
  if (cmLinkImplementationLibraries const* impl =
        headTarget->GetLinkImplementationLibraries(config)) {
    for (cmLinkImplItem const& lib : impl->Libraries) {
      if (lib.Target &&
          lib.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        std::string uniqueName =
          headTarget->GetGlobalGenerator()->IndexGeneratorTargetUniquely(
            lib.Target);
        std::string genex = "$<TARGET_OBJECTS:" + std::move(uniqueName) + ">";
        cmGeneratorExpression ge(lib.Backtrace);
        std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(genex);
        cge->SetEvaluateForBuildsystem(true);

        EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
        cmExpandList(cge->Evaluate(headTarget->GetLocalGenerator(), config,
                                   headTarget, dagChecker),
                     ee.Values);
        if (cge->GetHadContextSensitiveCondition()) {
          ee.ContextDependent = true;
        }
        entries.emplace_back(std::move(ee));
      }
    }
  }
}

bool processSources(cmGeneratorTarget const* tgt,
                    std::vector<EvaluatedTargetPropertyEntry>& entries,
                    std::vector<BT<std::string>>& srcs,
                    std::unordered_set<std::string>& uniqueSrcs,
                    bool debugSources)
{
  cmMakefile* mf = tgt->Target->GetMakefile();

  bool contextDependent = false;

  for (EvaluatedTargetPropertyEntry& entry : entries) {
    if (entry.ContextDependent) {
      contextDependent = true;
    }

    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();

    for (std::string& src : entry.Values) {
      cmSourceFile* sf = mf->GetOrCreateSource(src);
      std::string e;
      std::string fullPath = sf->ResolveFullPath(&e);
      if (fullPath.empty()) {
        if (!e.empty()) {
          cmake* cm = tgt->GetLocalGenerator()->GetCMakeInstance();
          cm->IssueMessage(MessageType::FATAL_ERROR, e, tgt->GetBacktrace());
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
  assert(this->GetType() != cmStateEnums::INTERFACE_LIBRARY);

  if (!this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    // At configure-time, this method can be called as part of getting the
    // LOCATION property or to export() a file to be include()d.  However
    // there is no cmGeneratorTarget at configure-time, so search the SOURCES
    // for TARGET_OBJECTS instead for backwards compatibility with OLD
    // behavior of CMP0024 and CMP0026 only.

    cmStringRange sourceEntries = this->Target->GetSourceEntries();
    for (std::string const& entry : sourceEntries) {
      std::vector<std::string> items = cmExpandedList(entry);
      for (std::string const& item : items) {
        if (cmHasLiteralPrefix(item, "$<TARGET_OBJECTS:") &&
            item.back() == '>') {
          continue;
        }
        files.emplace_back(item);
      }
    }
    return files;
  }

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugSources =
    !this->DebugSourcesDone && cmContains(debugProperties, "SOURCES");

  if (this->LocalGenerator->GetGlobalGenerator()->GetConfigureDoneCMP0026()) {
    this->DebugSourcesDone = true;
  }

  cmGeneratorExpressionDAGChecker dagChecker(this, "SOURCES", nullptr,
                                             nullptr);

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, std::string(), &dagChecker,
                                  this->SourceEntries);

  std::unordered_set<std::string> uniqueSrcs;
  bool contextDependentDirectSources =
    processSources(this, entries, files, uniqueSrcs, debugSources);

  // Collect INTERFACE_SOURCES of all direct link-dependencies.
  std::vector<EvaluatedTargetPropertyEntry> linkInterfaceSourcesEntries;
  AddInterfaceEntries(this, config, "INTERFACE_SOURCES", std::string(),
                      &dagChecker, linkInterfaceSourcesEntries);
  std::vector<std::string>::size_type numFilesBefore = files.size();
  bool contextDependentInterfaceSources = processSources(
    this, linkInterfaceSourcesEntries, files, uniqueSrcs, debugSources);

  // Collect TARGET_OBJECTS of direct object link-dependencies.
  bool contextDependentObjects = false;
  std::vector<std::string>::size_type numFilesBefore2 = files.size();
  if (this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    std::vector<EvaluatedTargetPropertyEntry> linkObjectsEntries;
    AddObjectEntries(this, config, &dagChecker, linkObjectsEntries);
    contextDependentObjects = processSources(this, linkObjectsEntries, files,
                                             uniqueSrcs, debugSources);
  }

  if (!contextDependentDirectSources &&
      !(contextDependentInterfaceSources && numFilesBefore < files.size()) &&
      !(contextDependentObjects && numFilesBefore2 < files.size())) {
    this->LinkImplementationLanguageIsContextDependent = false;
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
  // If we already processed one configuration and found no dependenc
  // on configuration then always use the one result.
  if (!this->LinkImplementationLanguageIsContextDependent) {
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
    if (sf->GetCustomCommand()) {
      kind = SourceKindCustomCommand;
    } else if (this->Target->GetType() == cmStateEnums::UTILITY) {
      kind = SourceKindExtra;
    } else if (this->IsSourceFilePartOfUnityBatch(sf->ResolveFullPath())) {
      kind = SourceKindUnityBatched;
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
      // Build and save the name of the corresponding .h file
      // This relationship will be used later when building the project files.
      // Both names would have been auto generated from Visual Studio
      // where the user supplied the file name and Visual Studio
      // appended the suffix.
      std::string resx = sf->ResolveFullPath();
      std::string hFileName = resx.substr(0, resx.find_last_of('.')) + ".h";
      files.ExpectedResxHeaders.insert(hFileName);
    } else if (ext == "appxmanifest") {
      kind = SourceKindAppManifest;
    } else if (ext == "manifest") {
      kind = SourceKindManifest;
    } else if (ext == "pfx") {
      kind = SourceKindCertificate;
    } else if (ext == "xaml") {
      kind = SourceKindXaml;
      // Build and save the name of the corresponding .h and .cpp file
      // This relationship will be used later when building the project files.
      // Both names would have been auto generated from Visual Studio
      // where the user supplied the file name and Visual Studio
      // appended the suffix.
      std::string xaml = sf->ResolveFullPath();
      std::string hFileName = xaml + ".h";
      std::string cppFileName = xaml + ".cpp";
      files.ExpectedXamlHeaders.insert(hFileName);
      files.ExpectedXamlSources.insert(cppFileName);
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
  std::vector<std::string> configs;
  this->Makefile->GetConfigurations(configs);

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
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, cmStateEnums::RuntimeBinaryArtifact,
                            prefix, base, suffix);

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config);
  std::string configProp = cmStrCat("COMPILE_PDB_NAME_", configUpper);
  const char* config_name = this->GetProperty(configProp);
  if (config_name && *config_name) {
    return prefix + config_name + ".pdb";
  }

  const char* name = this->GetProperty("COMPILE_PDB_NAME");
  if (name && *name) {
    return prefix + name + ".pdb";
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
  bool is_ninja =
    this->LocalGenerator->GetGlobalGenerator()->GetName() == "Ninja";

  if (have_rpath && is_ninja) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      "The install of the " << this->GetName() << " target requires "
      "changing an RPATH from the build tree, but this is not supported "
      "with the Ninja generator unless on an ELF-based platform.  The "
      "CMAKE_BUILD_WITH_INSTALL_RPATH variable may be set to avoid this "
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

#if defined(CMAKE_USE_ELF_PARSER)
  // Enable if the rpath flag uses a separator and the target uses ELF
  // binaries.
  std::string ll = this->GetLinkerLanguage(config);
  if (!ll.empty()) {
    std::string sepVar =
      cmStrCat("CMAKE_SHARED_LIBRARY_RUNTIME_", ll, "_FLAG_SEP");
    const char* sep = this->Makefile->GetDefinition(sepVar);
    if (sep && *sep) {
      // TODO: Add ELF check to ABI detection and get rid of
      // CMAKE_EXECUTABLE_FORMAT.
      if (const char* fmt =
            this->Makefile->GetDefinition("CMAKE_EXECUTABLE_FORMAT")) {
        return strcmp(fmt, "ELF") == 0;
      }
    }
  }
#endif
  static_cast<void>(config);
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
  bool install_name_is_rpath = false;
  bool macosx_rpath = false;

  if (!this->IsImported()) {
    if (this->GetType() != cmStateEnums::SHARED_LIBRARY) {
      return false;
    }
    const char* install_name = this->GetProperty("INSTALL_NAME_DIR");
    bool use_install_name = this->MacOSXUseInstallNameDir();
    if (install_name && use_install_name &&
        std::string(install_name) == "@rpath") {
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
        if (info->SOName.find("@rpath/") == 0) {
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

  const char* macosx_rpath_str = this->GetProperty("MACOSX_RPATH");
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
  const char* build_with_install_name =
    this->GetProperty("BUILD_WITH_INSTALL_NAME_DIR");
  if (build_with_install_name) {
    return cmIsOn(build_with_install_name);
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

std::string cmGeneratorTarget::GetSOName(const std::string& config) const
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
      if (info->SOName.find("@rpath/") == 0) {
        return info->SOName.substr(6);
      }
      return info->SOName;
    }
    return "";
  }
  // Compute the soname that will be built.
  return this->GetLibraryNames(config).SharedObject;
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
  const char* ext = this->GetProperty("BUNDLE_EXTENSION");
  if (!ext) {
    ext = "app";
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

bool cmGeneratorTarget::IsBundleOnApple() const
{
  return this->IsFrameworkOnApple() || this->IsAppBundleOnApple() ||
    this->IsCFBundleOnApple();
}

std::string cmGeneratorTarget::GetCFBundleDirectory(
  const std::string& config, BundleDirectoryLevel level) const
{
  std::string fpath = cmStrCat(
    this->GetOutputName(config, cmStateEnums::RuntimeBinaryArtifact), '.');
  const char* ext = this->GetProperty("BUNDLE_EXTENSION");
  if (!ext) {
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
  const char* ext = this->GetProperty("BUNDLE_EXTENSION");
  if (!ext) {
    ext = "framework";
  }
  fpath += ext;
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
    const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");

    if (this->CanGenerateInstallNameDir(INSTALL_NAME_FOR_INSTALL)) {
      if (install_name_dir && *install_name_dir) {
        dir = install_name_dir;
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
    if (const char* custom_export_name = this->GetProperty("DEFINE_SYMBOL")) {
      this->ExportMacro = custom_export_name;
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
                               cmGeneratorTarget const* head)
    : Config(std::move(config))
    , Languages(languages)
    , HeadTarget(head)
    , Target(target)
  {
    this->Visited.insert(target);
  }

  void Visit(cmLinkItem const& item)
  {
    if (!item.Target) {
      if (item.AsStr().find("::") != std::string::npos) {
        bool noMessage = false;
        MessageType messageType = MessageType::FATAL_ERROR;
        std::ostringstream e;
        switch (this->Target->GetLocalGenerator()->GetPolicyStatus(
          cmPolicies::CMP0028)) {
          case cmPolicies::WARN: {
            e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0028) << "\n";
            messageType = MessageType::AUTHOR_WARNING;
          } break;
          case cmPolicies::OLD:
            noMessage = true;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::NEW:
            // Issue the fatal message.
            break;
        }

        if (!noMessage) {
          e << "Target \"" << this->Target->GetName()
            << "\" links to target \"" << item.AsStr()
            << "\" but the target was not found.  Perhaps a find_package() "
               "call is missing for an IMPORTED target, or an ALIAS target is "
               "missing?";
          this->Target->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
            messageType, e.str(), this->Target->GetBacktrace());
        }
      }
      return;
    }
    if (!this->Visited.insert(item.Target).second) {
      return;
    }
    cmLinkInterface const* iface =
      item.Target->GetLinkInterface(this->Config, this->HeadTarget);
    if (!iface) {
      return;
    }

    for (std::string const& language : iface->Languages) {
      this->Languages.insert(language);
    }

    for (cmLinkItem const& lib : iface->Libraries) {
      this->Visit(lib);
    }
  }

private:
  std::string Config;
  std::unordered_set<std::string>& Languages;
  cmGeneratorTarget const* HeadTarget;
  const cmGeneratorTarget* Target;
  std::set<cmGeneratorTarget const*> Visited;
};

cmGeneratorTarget::LinkClosure const* cmGeneratorTarget::GetLinkClosure(
  const std::string& config) const
{
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
  int Preference;
  cmGeneratorTarget const* Target;
  cmGlobalGenerator* GG;
  std::set<std::string> Preferred;

public:
  cmTargetSelectLinker(cmGeneratorTarget const* target)
    : Preference(0)
    , Target(target)
  {
    this->GG = this->Target->GetLocalGenerator()->GetGlobalGenerator();
  }
  void Consider(const char* lang)
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

void cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc) const
{
  // Get languages built in this target.
  std::unordered_set<std::string> languages;
  cmLinkImplementation const* impl = this->GetLinkImplementation(config);
  assert(impl);
  for (std::string const& li : impl->Languages) {
    languages.insert(li);
  }

  // Add interface languages from linked targets.
  cmTargetCollectLinkLanguages cll(this, config, languages, this);
  for (cmLinkImplItem const& lib : impl->Libraries) {
    cll.Visit(lib);
  }

  // Store the transitive closure of languages.
  for (std::string const& lang : languages) {
    lc.Languages.push_back(lang);
  }

  // Choose the language whose linker should be used.
  if (this->GetProperty("HAS_CXX")) {
    lc.LinkerLanguage = "CXX";
  } else if (const char* linkerLang = this->GetProperty("LINKER_LANGUAGE")) {
    lc.LinkerLanguage = linkerLang;
  } else {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for (std::string const& l : impl->Languages) {
      tsl.Consider(l.c_str());
    }

    // Now consider languages that propagate from linked targets.
    for (std::string const& lang : languages) {
      std::string propagates =
        "CMAKE_" + lang + "_LINKER_PREFERENCE_PROPAGATES";
      if (this->Makefile->IsOn(propagates)) {
        tsl.Consider(lang.c_str());
      }
    }

    lc.LinkerLanguage = tsl.Choose();
  }
}

void cmGeneratorTarget::GetFullNameComponents(
  std::string& prefix, std::string& base, std::string& suffix,
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  this->GetFullNameInternal(config, artifact, prefix, base, suffix);
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

  const char* targetFolder = this->GetProperty("FOLDER");
  if (targetFolder) {
    effectiveFolder += targetFolder;
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

void processILibs(const std::string& config,
                  cmGeneratorTarget const* headTarget, cmLinkItem const& item,
                  cmGlobalGenerator* gg,
                  std::vector<cmGeneratorTarget const*>& tgts,
                  std::set<cmGeneratorTarget const*>& emitted)
{
  if (item.Target && emitted.insert(item.Target).second) {
    tgts.push_back(item.Target);
    if (cmLinkInterfaceLibraries const* iface =
          item.Target->GetLinkInterfaceLibraries(config, headTarget, true)) {
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
  LinkImplClosure& tgts = this->LinkImplClosureMap[config];
  if (!tgts.Done) {
    tgts.Done = true;
    std::set<cmGeneratorTarget const*> emitted;

    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibraries(config);

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
  void FollowCommandDepends(cmCustomCommand const& cc,
                            const std::string& config,
                            std::set<std::string>& emitted);
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
  if (target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
    std::set<cmSourceFile*> emitted;
    std::vector<std::string> const& configs =
      this->Makefile->GetGeneratorConfigs();
    for (std::string const& c : configs) {
      std::vector<cmSourceFile*> sources;
      this->GeneratorTarget->GetSourceFiles(sources, c);
      for (cmSourceFile* sf : sources) {
        const std::set<cmGeneratorTarget const*> tgts =
          this->GlobalGenerator->GetFilenameTargetDepends(sf);
        if (cmContains(tgts, this->GeneratorTarget)) {
          std::ostringstream e;
          e << "Evaluation output file\n  \"" << sf->ResolveFullPath()
            << "\"\ndepends on the sources of a target it is used in.  This "
               "is a dependency loop and is not allowed.";
          this->GeneratorTarget->LocalGenerator->IssueMessage(
            MessageType::FATAL_ERROR, e.str());
          return;
        }
        if (emitted.insert(sf).second &&
            this->SourcesQueued.insert(sf).second) {
          this->SourceQueue.push(sf);
        }
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
    if (const char* additionalDeps = sf->GetProperty("OBJECT_DEPENDS")) {
      std::vector<std::string> objDeps = cmExpandedList(additionalDeps);
      for (std::string& objDep : objDeps) {
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
    cmSourcesWithOutput sources = this->Makefile->GetSourcesWithOutput(name);
    // If we failed to find a target or source and we have a relative path, it
    // might be a valid source if made relative to the current binary
    // directory.
    if (!sources.Target && !sources.Source &&
        !cmSystemTools::FileIsFullPath(name)) {
      auto fullname =
        cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/', name);
      fullname = cmSystemTools::CollapseFullPath(
        fullname, this->Makefile->GetHomeOutputDirectory());
      sources = this->Makefile->GetSourcesWithOutput(fullname);
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
      this->GeneratorTarget->Target->AddUtility(util, false);
      return true;
    }
  }

  // The dependency does not name a target built in this project.
  return false;
}

void cmTargetTraceDependencies::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Transform command names that reference targets built in this
  // project to corresponding target-level dependencies.
  cmGeneratorExpression ge(cc.GetBacktrace());

  // Add target-level dependencies referenced by generator expressions.
  std::set<cmGeneratorTarget*> targets;

  for (cmCustomCommandLine const& cCmdLine : cc.GetCommandLines()) {
    std::string const& command = cCmdLine.front();
    // Check for a target with this name.
    if (cmGeneratorTarget* t =
          this->LocalGenerator->FindGeneratorTargetToUse(command)) {
      if (t->GetType() == cmStateEnums::EXECUTABLE) {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->GeneratorTarget->Target->AddUtility(command, true);
      }
    }

    // Check for target references in generator expressions.
    for (std::string const& cl : cCmdLine) {
      const std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(cl);
      cge->SetQuiet(true);
      cge->Evaluate(this->GeneratorTarget->GetLocalGenerator(), "");
      std::set<cmGeneratorTarget*> geTargets = cge->GetTargets();
      targets.insert(geTargets.begin(), geTargets.end());
    }
  }

  for (cmGeneratorTarget* target : targets) {
    this->GeneratorTarget->Target->AddUtility(target->GetName(), true);
  }

  // Queue the custom command dependencies.
  std::set<std::string> emitted;
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs();
  for (std::string const& conf : configs) {
    this->FollowCommandDepends(cc, conf, emitted);
  }
}

void cmTargetTraceDependencies::FollowCommandDepends(
  cmCustomCommand const& cc, const std::string& config,
  std::set<std::string>& emitted)
{
  cmCustomCommandGenerator ccg(cc, config,
                               this->GeneratorTarget->LocalGenerator);

  const std::vector<std::string>& depends = ccg.GetDepends();

  for (std::string const& dep : depends) {
    if (emitted.insert(dep).second) {
      if (!this->IsUtility(dep)) {
        // The dependency does not name a target and may be a file we
        // know how to generate.  Queue it.
        this->FollowName(dep);
      }
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

void cmGeneratorTarget::GetAppleArchs(const std::string& config,
                                      std::vector<std::string>& archVec) const
{
  const char* archs = nullptr;
  if (!config.empty()) {
    std::string defVarName =
      cmStrCat("OSX_ARCHITECTURES_", cmSystemTools::UpperCase(config));
    archs = this->GetProperty(defVarName);
  }
  if (!archs) {
    archs = this->GetProperty("OSX_ARCHITECTURES");
  }
  if (archs) {
    cmExpandList(std::string(archs), archVec);
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

namespace {
void processIncludeDirectories(
  cmGeneratorTarget const* tgt,
  std::vector<EvaluatedTargetPropertyEntry>& entries,
  std::vector<BT<std::string>>& includes,
  std::unordered_set<std::string>& uniqueIncludes, bool debugIncludes)
{
  for (EvaluatedTargetPropertyEntry& entry : entries) {
    cmLinkImplItem const& item = entry.LinkImplItem;
    std::string const& targetName = item.AsStr();
    bool const fromImported = item.Target && item.Target->IsImported();
    bool const checkCMP0027 = item.FromGenex;

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
  std::vector<BT<std::string>> includes;
  std::unordered_set<std::string> uniqueIncludes;

  cmGeneratorExpressionDAGChecker dagChecker(this, "INCLUDE_DIRECTORIES",
                                             nullptr, nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugIncludes = !this->DebugIncludesDone &&
    cmContains(debugProperties, "INCLUDE_DIRECTORIES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugIncludesDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, lang, &dagChecker,
                                  this->IncludeDirectoriesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_INCLUDE_DIRECTORIES", lang,
                      &dagChecker, entries);

  if (this->Makefile->IsOn("APPLE")) {
    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibraries(config);
    for (cmLinkImplItem const& lib : impl->Libraries) {
      std::string libDir = cmSystemTools::CollapseFullPath(
        lib.AsStr(), this->Makefile->GetHomeOutputDirectory());

      static cmsys::RegularExpression frameworkCheck(
        "(.*\\.framework)(/Versions/[^/]+)?/[^/]+$");
      if (!frameworkCheck.find(libDir)) {
        continue;
      }

      libDir = frameworkCheck.match(1);

      EvaluatedTargetPropertyEntry ee(lib, cmListFileBacktrace());
      ee.Values.emplace_back(std::move(libDir));
      entries.emplace_back(std::move(ee));
    }
  }

  processIncludeDirectories(this, entries, includes, uniqueIncludes,
                            debugIncludes);

  return includes;
}

enum class OptionsParse
{
  None,
  Shell
};

namespace {
void processOptions(cmGeneratorTarget const* tgt,
                    std::vector<EvaluatedTargetPropertyEntry> const& entries,
                    std::vector<BT<std::string>>& options,
                    std::unordered_set<std::string>& uniqueOptions,
                    bool debugOptions, const char* logName, OptionsParse parse)
{
  for (EvaluatedTargetPropertyEntry const& entry : entries) {
    std::string usedOptions;
    for (std::string const& opt : entry.Values) {
      if (uniqueOptions.insert(opt).second) {
        if (parse == OptionsParse::Shell &&
            cmHasLiteralPrefix(opt, "SHELL:")) {
          std::vector<std::string> tmp;
          cmSystemTools::ParseUnixCommandLine(opt.c_str() + 6, tmp);
          for (std::string& o : tmp) {
            options.emplace_back(std::move(o), entry.Backtrace);
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
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_OPTIONS", nullptr,
                                             nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugOptions = !this->DebugCompileOptionsDone &&
    cmContains(debugProperties, "COMPILE_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileOptionsDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, language, &dagChecker,
                                  this->CompileOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_OPTIONS", language,
                      &dagChecker, entries);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "compile options", OptionsParse::Shell);

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

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugFeatures = !this->DebugCompileFeaturesDone &&
    cmContains(debugProperties, "COMPILE_FEATURES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileFeaturesDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, std::string(), &dagChecker,
                                  this->CompileFeaturesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_FEATURES",
                      std::string(), &dagChecker, entries);

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
  std::vector<BT<std::string>> list;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "COMPILE_DEFINITIONS",
                                             nullptr, nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugDefines = !this->DebugCompileDefinitionsDone &&
    cmContains(debugProperties, "COMPILE_DEFINITIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugCompileDefinitionsDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, language, &dagChecker,
                                  this->CompileDefinitionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_COMPILE_DEFINITIONS", language,
                      &dagChecker, entries);

  if (!config.empty()) {
    std::string configPropName =
      "COMPILE_DEFINITIONS_" + cmSystemTools::UpperCase(config);
    const char* configProp = this->GetProperty(configPropName);
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
            CreateTargetPropertyEntry(configProp);
          entries.emplace_back(EvaluateTargetPropertyEntry(
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

  return list;
}

std::vector<BT<std::string>> cmGeneratorTarget::GetPrecompileHeaders(
  const std::string& config, const std::string& language) const
{
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "PRECOMPILE_HEADERS",
                                             nullptr, nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugDefines = !this->DebugPrecompileHeadersDone &&
    std::find(debugProperties.begin(), debugProperties.end(),
              "PRECOMPILE_HEADERS") != debugProperties.end();

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugPrecompileHeadersDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, language, &dagChecker,
                                  this->PrecompileHeadersEntries);

  AddInterfaceEntries(this, config, "INTERFACE_PRECOMPILE_HEADERS", language,
                      &dagChecker, entries);

  std::vector<BT<std::string>> list;
  processOptions(this, entries, list, uniqueOptions, debugDefines,
                 "precompile headers", OptionsParse::None);

  return list;
}

std::string cmGeneratorTarget::GetPchHeader(const std::string& config,
                                            const std::string& language) const
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }

  if (this->GetPropertyAsBool("DISABLE_PRECOMPILE_HEADERS")) {
    return std::string();
  }
  const cmGeneratorTarget* generatorTarget = this;
  const char* pchReuseFrom =
    generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");

  const auto inserted =
    this->PchHeaders.insert(std::make_pair(language + config, ""));
  if (inserted.second) {
    const std::vector<BT<std::string>> headers =
      this->GetPrecompileHeaders(config, language);
    if (headers.empty() && !pchReuseFrom) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    if (pchReuseFrom) {
      generatorTarget =
        this->GetGlobalGenerator()->FindGeneratorTarget(pchReuseFrom);
    }

    filename = cmStrCat(
      generatorTarget->LocalGenerator->GetCurrentBinaryDirectory(), "/");

    const std::map<std::string, std::string> languageToExtension = {
      { "C", ".h" },
      { "CXX", ".hxx" },
      { "OBJC", ".objc.h" },
      { "OBJCXX", ".objcxx.hxx" }
    };

    filename =
      cmStrCat(filename, "CMakeFiles/", generatorTarget->GetName(), ".dir");

    if (this->GetGlobalGenerator()->IsMultiConfig()) {
      filename = cmStrCat(filename, "/", config);
    }

    filename =
      cmStrCat(filename, "/cmake_pch", languageToExtension.at(language));

    const std::string filename_tmp = cmStrCat(filename, ".tmp");
    if (!pchReuseFrom) {
      auto pchPrologue = this->Makefile->GetDefinition("CMAKE_PCH_PROLOGUE");
      auto pchEpilogue = this->Makefile->GetDefinition("CMAKE_PCH_EPILOGUE");

      std::string firstHeaderOnDisk;
      {
        cmGeneratedFileStream file(
          filename_tmp, false,
          this->GetGlobalGenerator()->GetMakefileEncoding());
        file << "/* generated by CMake */\n\n";
        if (pchPrologue) {
          file << pchPrologue << "\n";
        }
        if (this->GetGlobalGenerator()->IsXcode()) {
          file << "#ifndef CMAKE_SKIP_PRECOMPILE_HEADERS\n";
        }
        if (language == "CXX") {
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
        if (language == "CXX") {
          file << "#endif // __cplusplus\n";
        }
        if (this->GetGlobalGenerator()->IsXcode()) {
          file << "#endif // CMAKE_SKIP_PRECOMPILE_HEADERS\n";
        }
        if (pchEpilogue) {
          file << pchEpilogue << "\n";
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
                                            const std::string& language) const
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }
  const auto inserted =
    this->PchSources.insert(std::make_pair(language + config, ""));
  if (inserted.second) {
    const std::string pchHeader = this->GetPchHeader(config, language);
    if (pchHeader.empty()) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    const cmGeneratorTarget* generatorTarget = this;
    const char* pchReuseFrom =
      generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");
    if (pchReuseFrom) {
      generatorTarget =
        this->GetGlobalGenerator()->FindGeneratorTarget(pchReuseFrom);
    }

    filename =
      cmStrCat(generatorTarget->LocalGenerator->GetCurrentBinaryDirectory(),
               "/CMakeFiles/", generatorTarget->GetName(), ".dir/cmake_pch");

    // For GCC the source extension will be tranformed into .h[xx].gch
    if (!this->Makefile->IsOn("CMAKE_LINK_PCH")) {
      const std::map<std::string, std::string> languageToExtension = {
        { "C", ".h.c" },
        { "CXX", ".hxx.cxx" },
        { "OBJC", ".objc.h.m" },
        { "OBJCXX", ".objcxx.hxx.mm" }
      };

      filename += languageToExtension.at(language);
    } else {
      const std::map<std::string, std::string> languageToExtension = {
        { "C", ".c" }, { "CXX", ".cxx" }, { "OBJC", ".m" }, { "OBJCXX", ".mm" }
      };

      filename += languageToExtension.at(language);
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
                                                const std::string& language)
{
  if (language != "C" && language != "CXX" && language != "OBJC" &&
      language != "OBJCXX") {
    return std::string();
  }
  const auto inserted =
    this->PchObjectFiles.insert(std::make_pair(language + config, ""));
  if (inserted.second) {
    const std::string pchSource = this->GetPchSource(config, language);
    if (pchSource.empty()) {
      return std::string();
    }
    std::string& filename = inserted.first->second;

    auto pchSf = this->Makefile->GetOrCreateSource(
      pchSource, false, cmSourceFileLocationKind::Known);

    filename = cmStrCat(this->ObjectDirectory, this->GetObjectName(pchSf));
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchFile(const std::string& config,
                                          const std::string& language)
{
  const auto inserted =
    this->PchFiles.insert(std::make_pair(language + config, ""));
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
      const char* pchReuseFrom =
        generatorTarget->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");
      if (pchReuseFrom) {
        generatorTarget =
          this->GetGlobalGenerator()->FindGeneratorTarget(pchReuseFrom);
      }

      const std::string pchFileObject =
        generatorTarget->GetPchFileObject(config, language);
      if (!pchExtension.empty()) {
        pchFile = replaceExtension(pchFileObject, pchExtension);
      }
    } else {
      pchFile = this->GetPchHeader(config, language);
      pchFile += pchExtension;
    }
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchCreateCompileOptions(
  const std::string& config, const std::string& language)
{
  const auto inserted = this->PchCreateCompileOptions.insert(
    std::make_pair(language + config, ""));
  if (inserted.second) {
    std::string& createOptionList = inserted.first->second;

    const std::string createOptVar =
      cmStrCat("CMAKE_", language, "_COMPILE_OPTIONS_CREATE_PCH");
    createOptionList = this->Makefile->GetSafeDefinition(createOptVar);

    const std::string pchHeader = this->GetPchHeader(config, language);
    const std::string pchFile = this->GetPchFile(config, language);

    cmSystemTools::ReplaceString(createOptionList, "<PCH_HEADER>", pchHeader);
    cmSystemTools::ReplaceString(createOptionList, "<PCH_FILE>", pchFile);
  }
  return inserted.first->second;
}

std::string cmGeneratorTarget::GetPchUseCompileOptions(
  const std::string& config, const std::string& language)
{
  const auto inserted =
    this->PchUseCompileOptions.insert(std::make_pair(language + config, ""));
  if (inserted.second) {
    std::string& useOptionList = inserted.first->second;

    const std::string useOptVar =
      cmStrCat("CMAKE_", language, "_COMPILE_OPTIONS_USE_PCH");
    useOptionList = this->Makefile->GetSafeDefinition(useOptVar);

    const std::string pchHeader = this->GetPchHeader(config, language);
    const std::string pchFile = this->GetPchFile(config, language);

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
  std::vector<BT<std::string>> tmp = this->GetLinkOptions(config, language);
  result.reserve(tmp.size());
  for (BT<std::string>& v : tmp) {
    result.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmGeneratorTarget::GetLinkOptions(
  std::string const& config, std::string const& language) const
{
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueOptions;

  cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_OPTIONS", nullptr,
                                             nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugOptions =
    !this->DebugLinkOptionsDone && cmContains(debugProperties, "LINK_OPTIONS");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkOptionsDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, language, &dagChecker,
                                  this->LinkOptionsEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_OPTIONS", language,
                      &dagChecker, entries,
                      this->GetPolicyStatusCMP0099() != cmPolicies::NEW);

  processOptions(this, entries, result, uniqueOptions, debugOptions,
                 "link options", OptionsParse::Shell);

  // Last step: replace "LINKER:" prefixed elements by
  // actual linker wrapper
  const std::string wrapper(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language + "_LINKER_WRAPPER_FLAG"));
  std::vector<std::string> wrapperFlag = cmExpandedList(wrapper);
  const std::string wrapperSep(this->Makefile->GetSafeDefinition(
    "CMAKE_" + language + "_LINKER_WRAPPER_FLAG_SEP"));
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

    std::vector<BT<std::string>> options;
    if (wrapperFlag.empty()) {
      // nothing specified, insert elements as is
      options.reserve(linkerOptions.size());
      for (std::string& o : linkerOptions) {
        options.emplace_back(std::move(o), bt);
      }
    } else {
      if (!wrapperSep.empty()) {
        if (concatFlagAndArgs) {
          // insert flag elements except last one
          for (auto i = wrapperFlag.begin(); i != wrapperFlag.end() - 1; ++i) {
            options.emplace_back(*i, bt);
          }
          // concatenate last flag element and all LINKER list values
          // in one option
          options.emplace_back(
            wrapperFlag.back() + cmJoin(linkerOptions, wrapperSep), bt);
        } else {
          for (std::string const& i : wrapperFlag) {
            options.emplace_back(i, bt);
          }
          // concatenate all LINKER list values in one option
          options.emplace_back(cmJoin(linkerOptions, wrapperSep), bt);
        }
      } else {
        // prefix each element of LINKER list with wrapper
        if (concatFlagAndArgs) {
          std::transform(linkerOptions.begin(), linkerOptions.end(),
                         linkerOptions.begin(),
                         [&wrapperFlag](std::string const& o) -> std::string {
                           return wrapperFlag.back() + o;
                         });
        }
        for (std::string& o : linkerOptions) {
          for (auto i = wrapperFlag.begin(),
                    e = concatFlagAndArgs ? wrapperFlag.end() - 1
                                          : wrapperFlag.end();
               i != e; ++i) {
            options.emplace_back(*i, bt);
          }
          options.emplace_back(std::move(o), bt);
        }
      }
    }
    result.insert(entry, options.begin(), options.end());
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

  std::vector<EvaluatedTargetPropertyEntry> entries;
  if (const char* linkOptions = this->GetProperty("STATIC_LIBRARY_OPTIONS")) {
    std::vector<std::string> options = cmExpandedList(linkOptions);
    for (const auto& option : options) {
      std::unique_ptr<TargetPropertyEntry> entry =
        CreateTargetPropertyEntry(option);
      entries.emplace_back(EvaluateTargetPropertyEntry(this, config, language,
                                                       &dagChecker, *entry));
    }
  }
  processOptions(this, entries, result, uniqueOptions, false,
                 "static library link options", OptionsParse::Shell);

  return result;
}

namespace {
void processLinkDirectories(cmGeneratorTarget const* tgt,
                            std::vector<EvaluatedTargetPropertyEntry>& entries,
                            std::vector<BT<std::string>>& directories,
                            std::unordered_set<std::string>& uniqueDirectories,
                            bool debugDirectories)
{
  for (EvaluatedTargetPropertyEntry& entry : entries) {
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
  std::vector<BT<std::string>> result;
  std::unordered_set<std::string> uniqueDirectories;

  cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_DIRECTORIES", nullptr,
                                             nullptr);

  std::vector<std::string> debugProperties;
  const char* debugProp =
    this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugDirectories = !this->DebugLinkDirectoriesDone &&
    cmContains(debugProperties, "LINK_DIRECTORIES");

  if (this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    this->DebugLinkDirectoriesDone = true;
  }

  std::vector<EvaluatedTargetPropertyEntry> entries =
    EvaluateTargetPropertyEntries(this, config, language, &dagChecker,
                                  this->LinkDirectoriesEntries);

  AddInterfaceEntries(this, config, "INTERFACE_LINK_DIRECTORIES", language,
                      &dagChecker, entries,
                      this->GetPolicyStatusCMP0099() != cmPolicies::NEW);

  processLinkDirectories(this, entries, result, uniqueDirectories,
                         debugDirectories);

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

  std::vector<EvaluatedTargetPropertyEntry> entries;
  if (const char* linkDepends = this->GetProperty("LINK_DEPENDS")) {
    std::vector<std::string> depends = cmExpandedList(linkDepends);
    for (const auto& depend : depends) {
      std::unique_ptr<TargetPropertyEntry> entry =
        CreateTargetPropertyEntry(depend);
      entries.emplace_back(EvaluateTargetPropertyEntry(this, config, language,
                                                       &dagChecker, *entry));
    }
  }
  AddInterfaceEntries(this, config, "INTERFACE_LINK_DEPENDS", language,
                      &dagChecker, entries,
                      this->GetPolicyStatusCMP0099() != cmPolicies::NEW);

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
  if (!targetNames.ImportLibrary.empty()) {
    f =
      cmStrCat(this->GetDirectory(config, cmStateEnums::ImportLibraryArtifact),
               '/', targetNames.ImportLibrary);
    gg->AddToManifest(f);
  }
}

bool cmGeneratorTarget::ComputeCompileFeatures(std::string const& config) const
{
  std::vector<BT<std::string>> features = this->GetCompileFeatures(config);
  for (BT<std::string> const& f : features) {
    if (!this->Makefile->AddRequiredTargetFeature(this->Target, f.Value)) {
      return false;
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
      fpath += this->GetFullName(config, cmStateEnums::ImportLibraryArtifact);
      break;
  }
  return fpath;
}

std::string cmGeneratorTarget::NormalGetRealName(
  const std::string& config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if (this->IsImported()) {
    std::string msg = cmStrCat("NormalGetRealName called on imported target: ",
                               this->GetName());
    this->LocalGenerator->IssueMessage(MessageType::INTERNAL_ERROR, msg);
  }

  if (this->GetType() == cmStateEnums::EXECUTABLE) {
    // Compute the real name that will be built.
    return this->GetExecutableNames(config).Real;
  }
  // Compute the real name that will be built.
  return this->GetLibraryNames(config).Real;
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
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
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
  std::string prefix;
  std::string suffix;
  this->GetFullNameInternal(config, cmStateEnums::RuntimeBinaryArtifact,
                            prefix, targetNames.Base, suffix);

  // The library name.
  targetNames.Output = prefix + targetNames.Base + suffix;

  if (this->IsFrameworkOnApple()) {
    targetNames.Real = prefix;
    if (!this->Makefile->PlatformIsAppleEmbedded()) {
      targetNames.Real += "Versions/";
      targetNames.Real += this->GetFrameworkVersion();
      targetNames.Real += "/";
    }
    targetNames.Real += targetNames.Base;
    targetNames.SharedObject = targetNames.Real;
  } else {
    // The library's soname.
    this->ComputeVersionedName(targetNames.SharedObject, prefix,
                               targetNames.Base, suffix, targetNames.Output,
                               soversion);

    // The library's real name on disk.
    this->ComputeVersionedName(targetNames.Real, prefix, targetNames.Base,
                               suffix, targetNames.Output, version);
  }

  // The import library name.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GetType() == cmStateEnums::MODULE_LIBRARY) {
    targetNames.ImportLibrary =
      this->GetFullNameInternal(config, cmStateEnums::ImportLibraryArtifact);
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
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if (this->GetType() != cmStateEnums::EXECUTABLE ||
      this->Makefile->IsOn("XCODE")) {
    version = nullptr;
  }
#endif

  // Get the components of the executable name.
  std::string prefix;
  std::string suffix;
  this->GetFullNameInternal(config, cmStateEnums::RuntimeBinaryArtifact,
                            prefix, targetNames.Base, suffix);

  // The executable name.
  targetNames.Output = prefix + targetNames.Base + suffix;

// The executable's real name on disk.
#if defined(__CYGWIN__)
  targetNames.Real = prefix + targetNames.Base;
#else
  targetNames.Real = targetNames.Output;
#endif
  if (version) {
    targetNames.Real += "-";
    targetNames.Real += version;
  }
#if defined(__CYGWIN__)
  targetNames.Real += suffix;
#endif

  // The import library name.
  targetNames.ImportLibrary =
    this->GetFullNameInternal(config, cmStateEnums::ImportLibraryArtifact);

  // The program database file name.
  targetNames.PDB = this->GetPDBName(config);

  return targetNames;
}

std::string cmGeneratorTarget::GetFullNameInternal(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, artifact, prefix, base, suffix);
  return prefix + base + suffix;
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

void cmGeneratorTarget::GetFullNameInternal(
  const std::string& config, cmStateEnums::ArtifactType artifact,
  std::string& outPrefix, std::string& outBase, std::string& outSuffix) const
{
  // Use just the target name for non-main target types.
  if (this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    outPrefix.clear();
    outBase = this->GetName();
    outSuffix.clear();
    return;
  }

  const bool isImportedLibraryArtifact =
    (artifact == cmStateEnums::ImportLibraryArtifact);

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if (isImportedLibraryArtifact && !this->NeedImportLibraryName(config)) {
    outPrefix.clear();
    outBase.clear();
    outSuffix.clear();
    return;
  }

  // retrieve prefix and suffix
  std::string ll = this->GetLinkerLanguage(config);
  const char* targetPrefix = this->GetFilePrefixInternal(config, artifact, ll);
  const char* targetSuffix = this->GetFileSuffixInternal(config, artifact, ll);

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::EXECUTABLE) {
    artifact = cmStateEnums::RuntimeBinaryArtifact;
  }

  // Compute the full name for main target types.
  const std::string configPostfix = this->GetFilePostfix(config);

  // frameworks have directory prefix but no suffix
  std::string fw_prefix;
  if (this->IsFrameworkOnApple()) {
    fw_prefix =
      cmStrCat(this->GetFrameworkDirectory(config, ContentLevel), '/');
    targetPrefix = fw_prefix.c_str();
    targetSuffix = nullptr;
  }

  if (this->IsCFBundleOnApple()) {
    fw_prefix = cmStrCat(this->GetCFBundleDirectory(config, FullLevel), '/');
    targetPrefix = fw_prefix.c_str();
    targetSuffix = nullptr;
  }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix ? targetPrefix : "";

  // Append the target name or property-specified name.
  outBase += this->GetOutputName(config, artifact);

  // Append the per-configuration postfix.
  outBase += configPostfix;

  // Name shared libraries with their version number on some platforms.
  if (const char* soversion = this->GetProperty("SOVERSION")) {
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY &&
        !isImportedLibraryArtifact &&
        this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION")) {
      outBase += "-";
      outBase += soversion;
    }
  }

  // Append the suffix.
  outSuffix = targetSuffix ? targetSuffix : "";
}

std::string cmGeneratorTarget::GetLinkerLanguage(
  const std::string& config) const
{
  return this->GetLinkClosure(config)->LinkerLanguage;
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
    if (const char* outName = this->GetProperty(p)) {
      base = outName;
      break;
    }
  }
  return base;
}

std::string cmGeneratorTarget::GetPDBName(const std::string& config) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, cmStateEnums::RuntimeBinaryArtifact,
                            prefix, base, suffix);

  std::vector<std::string> props;
  std::string configUpper = cmSystemTools::UpperCase(config);
  if (!configUpper.empty()) {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
  }

  // PDB_NAME
  props.emplace_back("PDB_NAME");

  for (std::string const& p : props) {
    if (const char* outName = this->GetProperty(p)) {
      base = outName;
      break;
    }
  }
  return prefix + base + ".pdb";
}

std::string cmGeneratorTarget::GetObjectDirectory(
  std::string const& config) const
{
  std::string obj_dir =
    this->GlobalGenerator->ExpandCFGIntDir(this->ObjectDirectory, config);
#if defined(__APPLE__)
  // find and replace $(PROJECT_NAME) xcode placeholder
  const std::string projectName = this->LocalGenerator->GetProjectName();
  cmSystemTools::ReplaceString(obj_dir, "$(PROJECT_NAME)", projectName);
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
    if (const char* location = sf->GetProperty("MACOSX_PACKAGE_LOCATION")) {
      flags.MacFolder = location;
      const bool stripResources =
        this->GlobalGenerator->ShouldStripResourcePath(this->Makefile);
      if (strcmp(location, "Resources") == 0) {
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        if (stripResources) {
          flags.MacFolder = "";
        }
      } else if (cmHasLiteralPrefix(location, "Resources/")) {
        flags.Type = cmGeneratorTarget::SourceFileTypeDeepResource;
        if (stripResources) {
          flags.MacFolder += strlen("Resources/");
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
  if (const char* files = this->GetProperty("PUBLIC_HEADER")) {
    std::vector<std::string> relFiles = cmExpandedList(files);
    for (std::string const& relFile : relFiles) {
      if (cmSourceFile* sf = this->Makefile->GetSource(relFile)) {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmGeneratorTarget::SourceFileTypePublicHeader;
      }
    }
  }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if (const char* files = this->GetProperty("PRIVATE_HEADER")) {
    std::vector<std::string> relFiles = cmExpandedList(files);
    for (std::string const& relFile : relFiles) {
      if (cmSourceFile* sf = this->Makefile->GetSource(relFile)) {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmGeneratorTarget::SourceFileTypePrivateHeader;
      }
    }
  }

  // Mark sources listed as resources.
  if (const char* files = this->GetProperty("RESOURCE")) {
    std::vector<std::string> relFiles = cmExpandedList(files);
    for (std::string const& relFile : relFiles) {
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
  if (const char* prop = li->GetProperty("COMPATIBLE_INTERFACE_" #X)) {       \
    std::vector<std::string> props;                                           \
    cmExpandList(prop, props);                                                \
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
  const char* prop = dependee->GetProperty(propName);
  if (!prop) {
    return;
  }

  std::vector<std::string> props = cmExpandedList(prop);
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
      if (cmSystemTools::GetErrorOccuredFlag()) {
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
    if (!dep.Target) {
      continue;
    }

    checkPropertyConsistency<bool>(this, dep.Target, strBool, emittedBools,
                                   config, BoolType, nullptr);
    if (cmSystemTools::GetErrorOccuredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strString,
                                          emittedStrings, config, StringType,
                                          nullptr);
    if (cmSystemTools::GetErrorOccuredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMin,
                                          emittedMinNumbers, config,
                                          NumberMinType, nullptr);
    if (cmSystemTools::GetErrorOccuredFlag()) {
      return;
    }
    checkPropertyConsistency<const char*>(this, dep.Target, strNumMax,
                                          emittedMaxNumbers, config,
                                          NumberMaxType, nullptr);
    if (cmSystemTools::GetErrorOccuredFlag()) {
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
      << "\".  This is not allowed. A property may only require compatibility "
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
std::string valueAsString<std::nullptr_t>(std::nullptr_t /*unused*/)
{
  return "(unset)";
}

std::string compatibilityType(CompatibleType t)
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

std::string compatibilityAgree(CompatibleType t, bool dominant)
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

  const char* value = tgt->GetProperty(prop);
  return cmIsOn(genexInterpreter->Evaluate(value, prop));
}

template <>
const char* getTypedProperty<const char*>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  const char* value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return value;
  }

  return genexInterpreter->Evaluate(value, prop).c_str();
}

template <>
std::string getTypedProperty<std::string>(
  cmGeneratorTarget const* tgt, const std::string& prop,
  cmGeneratorExpressionInterpreter* genexInterpreter)
{
  const char* value = tgt->GetProperty(prop);

  if (genexInterpreter == nullptr) {
    return valueAsString(value);
  }

  return genexInterpreter->Evaluate(value, prop);
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

std::pair<bool, const char*> consistentStringProperty(const char* lhs,
                                                      const char* rhs)
{
  const bool b = strcmp(lhs, rhs) == 0;
  return { b, b ? lhs : nullptr };
}

std::pair<bool, std::string> consistentStringProperty(const std::string& lhs,
                                                      const std::string& rhs)
{
  const bool b = lhs == rhs;
  return { b, b ? lhs : valueAsString(nullptr) };
}

std::pair<bool, const char*> consistentNumberProperty(const char* lhs,
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

std::pair<bool, std::string> consistentProperty(const std::string& lhs,
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
  const bool explicitlySet = cmContains(headPropKeys, p);

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

    const bool ifaceIsSet = cmContains(propKeys, interfaceProperty);
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

  if (const char* version = this->GetProperty(property)) {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    int parsed_patch;
    switch (sscanf(version, "%d.%d.%d", &parsed_major, &parsed_minor,
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

std::string cmGeneratorTarget::CreateFortranModuleDirectory(
  std::string const& working_dir) const
{
  std::string mod_dir;
  std::string target_mod_dir;
  if (const char* prop = this->GetProperty("Fortran_MODULE_DIRECTORY")) {
    target_mod_dir = prop;
  } else {
    std::string const& default_mod_dir =
      this->LocalGenerator->GetCurrentBinaryDirectory();
    if (default_mod_dir != working_dir) {
      target_mod_dir = default_mod_dir;
    }
  }
  const char* moddir_flag =
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

std::string cmGeneratorTarget::GetFrameworkVersion() const
{
  assert(this->GetType() != cmStateEnums::INTERFACE_LIBRARY);

  if (const char* fversion = this->GetProperty("FRAMEWORK_VERSION")) {
    return fversion;
  }
  if (const char* tversion = this->GetProperty("VERSION")) {
    return tversion;
  }
  return "A";
}

void cmGeneratorTarget::ComputeVersionedName(std::string& vName,
                                             std::string const& prefix,
                                             std::string const& base,
                                             std::string const& suffix,
                                             std::string const& name,
                                             const char* version) const
{
  vName = this->Makefile->IsOn("APPLE") ? (prefix + base) : name;
  if (version) {
    vName += ".";
    vName += version;
  }
  vName += this->Makefile->IsOn("APPLE") ? suffix : std::string();
}

std::vector<std::string> cmGeneratorTarget::GetPropertyKeys() const
{
  return this->Target->GetProperties().GetKeys();
}

void cmGeneratorTarget::ReportPropertyOrigin(
  const std::string& p, const std::string& result, const std::string& report,
  const std::string& compatibilityType) const
{
  std::vector<std::string> debugProperties;
  const char* debugProp = this->Target->GetMakefile()->GetDefinition(
    "CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp) {
    cmExpandList(debugProp, debugProperties);
  }

  bool debugOrigin =
    !this->DebugCompatiblePropertiesDone[p] && cmContains(debugProperties, p);

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
    cmDirectoryId const dirId = n.substr(sizeof(CMAKE_DIRECTORY_ID_SEP) - 1);
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

void cmGeneratorTarget::LookupLinkItems(std::vector<std::string> const& names,
                                        cmListFileBacktrace const& bt,
                                        std::vector<cmLinkItem>& items) const
{
  cmLocalGenerator const* lg = this->LocalGenerator;
  for (std::string const& n : names) {
    if (this->IsLinkLookupScope(n, lg)) {
      continue;
    }

    std::string name = this->CheckCMP0004(n);
    if (name == this->GetName() || name.empty()) {
      continue;
    }
    items.push_back(this->ResolveLinkItem(name, bt, lg));
  }
}

void cmGeneratorTarget::ExpandLinkItems(
  std::string const& prop, std::string const& value, std::string const& config,
  cmGeneratorTarget const* headTarget, bool usage_requirements_only,
  std::vector<cmLinkItem>& items, bool& hadHeadSensitiveCondition) const
{
  // Keep this logic in sync with ComputeLinkImplementationLibraries.
  cmGeneratorExpression ge;
  cmGeneratorExpressionDAGChecker dagChecker(this, prop, nullptr, nullptr);
  // The $<LINK_ONLY> expression may be in a link interface to specify private
  // link dependencies that are otherwise excluded from usage requirements.
  if (usage_requirements_only) {
    dagChecker.SetTransitivePropertiesOnly();
  }
  std::vector<std::string> libs;
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
  cmExpandList(
    cge->Evaluate(this->LocalGenerator, config, headTarget, &dagChecker, this),
    libs);
  this->LookupLinkItems(libs, cge->GetBacktrace(), items);
  hadHeadSensitiveCondition = cge->GetHadHeadSensitiveCondition();
}

cmLinkInterface const* cmGeneratorTarget::GetLinkInterface(
  const std::string& config, cmGeneratorTarget const* head) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, false);
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
  // then return the one we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    return &hm.begin()->second;
  }

  cmOptionalLinkInterface& iface = hm[head];
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head, false);
  }
  if (!iface.AllDone) {
    iface.AllDone = true;
    if (iface.Exists) {
      this->ComputeLinkInterface(config, iface, head);
    }
  }

  return iface.Exists ? &iface : nullptr;
}

void cmGeneratorTarget::ComputeLinkInterface(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget) const
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
        cmLinkImplementation const* impl = this->GetLinkImplementation(config);
        for (cmLinkImplItem const& lib : impl->Libraries) {
          if (emitted.insert(lib).second) {
            if (lib.Target) {
              // This is a runtime dependency on another shared library.
              if (lib.Target->GetType() == cmStateEnums::SHARED_LIBRARY) {
                iface.SharedDeps.push_back(lib);
              }
            } else {
              // TODO: Recognize shared library file names.  Perhaps this
              // should be moved to cmComputeLinkInformation, but that creates
              // a chicken-and-egg problem since this list is needed for its
              // construction.
            }
          }
        }
      }
    }
  } else if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN ||
             this->GetPolicyStatusCMP0022() == cmPolicies::OLD) {
    // The link implementation is the default link interface.
    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibrariesInternal(config, headTarget);
    iface.ImplementationIsInterface = true;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
  }

  if (this->LinkLanguagePropagatesToDependents()) {
    // Targets using this archive need its language runtime libraries.
    if (cmLinkImplementation const* impl =
          this->GetLinkImplementation(config)) {
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
    if (const char* config_reps = this->GetProperty(propName)) {
      sscanf(config_reps, "%u", &iface.Multiplicity);
    } else if (const char* reps =
                 this->GetProperty("LINK_INTERFACE_MULTIPLICITY")) {
      sscanf(reps, "%u", &iface.Multiplicity);
    }
  }
}

const cmLinkInterfaceLibraries* cmGeneratorTarget::GetLinkInterfaceLibraries(
  const std::string& config, cmGeneratorTarget const* head,
  bool usage_requirements_only) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, usage_requirements_only);
  }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if (this->GetType() == cmStateEnums::EXECUTABLE &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup any existing link interface for this configuration.
  std::string CONFIG = cmSystemTools::UpperCase(config);
  cmHeadToLinkInterfaceMap& hm =
    (usage_requirements_only
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then return the one we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    return &hm.begin()->second;
  }

  cmOptionalLinkInterface& iface = hm[head];
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head,
                                        usage_requirements_only);
  }

  return iface.Exists ? &iface : nullptr;
}

std::string cmGeneratorTarget::GetDirectory(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  if (this->IsImported()) {
    // Return the directory from which the target is imported.
    return cmSystemTools::GetFilenamePath(
      this->Target->ImportedGetFullPath(config, artifact));
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
  const char* propertyName = nullptr;
  std::string propertyNameStr = targetTypeName;
  if (!propertyNameStr.empty()) {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
  }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(conf);
  const char* configProp = nullptr;
  std::string configPropStr = targetTypeName;
  if (!configPropStr.empty()) {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
  }

  // Select an output directory.
  if (const char* config_outdir = this->GetProperty(configProp)) {
    // Use the user-specified per-configuration output directory.
    out = cmGeneratorExpression::Evaluate(config_outdir, this->LocalGenerator,
                                          config);

    // Skip per-configuration subdirectory.
    conf.clear();
  } else if (const char* outdir = this->GetProperty(propertyName)) {
    // Use the user-specified output directory.
    out =
      cmGeneratorExpression::Evaluate(outdir, this->LocalGenerator, config);

    // Skip per-configuration subdirectory if the value contained a
    // generator expression.
    if (out != outdir) {
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
  const char* propertyName = nullptr;
  std::string propertyNameStr = kind;
  if (!propertyNameStr.empty()) {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
  }
  std::string conf = config;

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(conf);
  const char* configProp = nullptr;
  std::string configPropStr = kind;
  if (!configPropStr.empty()) {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
  }

  // Select an output directory.
  if (const char* config_outdir = this->GetProperty(configProp)) {
    // Use the user-specified per-configuration output directory.
    out = cmGeneratorExpression::Evaluate(config_outdir, this->LocalGenerator,
                                          config);

    // Skip per-configuration subdirectory.
    conf.clear();
  } else if (const char* outdir = this->GetProperty(propertyName)) {
    // Use the user-specified output directory.
    out =
      cmGeneratorExpression::Evaluate(outdir, this->LocalGenerator, config);

    // Skip per-configuration subdirectory if the value contained a
    // generator expression.
    if (out != outdir) {
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
  const char* value = this->GetProperty(prop);
  if (!value) {
    return false;
  }

  rpath = cmGeneratorExpression::Evaluate(value, this->LocalGenerator, config);

  return true;
}

void cmGeneratorTarget::ComputeLinkInterfaceLibraries(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget, bool usage_requirements_only) const
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
  const char* explicitLibraries = nullptr;
  std::string linkIfaceProp;
  bool const cmp0022NEW = (this->GetPolicyStatusCMP0022() != cmPolicies::OLD &&
                           this->GetPolicyStatusCMP0022() != cmPolicies::WARN);
  if (cmp0022NEW) {
    // CMP0022 NEW behavior is to use INTERFACE_LINK_LIBRARIES.
    linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
    explicitLibraries = this->GetProperty(linkIfaceProp);
  } else if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
             this->IsExecutableWithExports()) {
    // CMP0022 OLD behavior is to use LINK_INTERFACE_LIBRARIES if set on a
    // shared lib or executable.

    // Lookup the per-configuration property.
    linkIfaceProp = cmStrCat("LINK_INTERFACE_LIBRARIES", suffix);
    explicitLibraries = this->GetProperty(linkIfaceProp);

    // If not set, try the generic property.
    if (!explicitLibraries) {
      linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
      explicitLibraries = this->GetProperty(linkIfaceProp);
    }
  }

  if (explicitLibraries &&
      this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
      !this->PolicyWarnedCMP0022) {
    // Compare the explicitly set old link interface properties to the
    // preferred new link interface property one and warn if different.
    const char* newExplicitLibraries =
      this->GetProperty("INTERFACE_LINK_LIBRARIES");
    if (newExplicitLibraries &&
        strcmp(newExplicitLibraries, explicitLibraries) != 0) {
      std::ostringstream w;
      /* clang-format off */
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0022) << "\n"
        "Target \"" << this->GetName() << "\" has an "
        "INTERFACE_LINK_LIBRARIES property which differs from its " <<
        linkIfaceProp << " properties."
        "\n"
        "INTERFACE_LINK_LIBRARIES:\n"
        "  " << newExplicitLibraries << "\n" <<
        linkIfaceProp << ":\n"
        "  " << explicitLibraries << "\n";
      /* clang-format on */
      this->LocalGenerator->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
      this->PolicyWarnedCMP0022 = true;
    }
  }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  if (!explicitLibraries &&
      (this->GetType() == cmStateEnums::EXECUTABLE ||
       (this->GetType() == cmStateEnums::MODULE_LIBRARY))) {
    return;
  }
  iface.Exists = true;
  iface.Explicit = cmp0022NEW || explicitLibraries != nullptr;

  if (explicitLibraries) {
    // The interface libraries have been explicitly set.
    this->ExpandLinkItems(linkIfaceProp, explicitLibraries, config, headTarget,
                          usage_requirements_only, iface.Libraries,
                          iface.HadHeadSensitiveCondition);
  } else if (!cmp0022NEW)
  // If CMP0022 is NEW then the plain tll signature sets the
  // INTERFACE_LINK_LIBRARIES, so if we get here then the project
  // cleared the property explicitly and we should not fall back
  // to the link implementation.
  {
    // The link implementation is the default link interface.
    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibrariesInternal(config, headTarget);
    iface.Libraries.insert(iface.Libraries.end(), impl->Libraries.begin(),
                           impl->Libraries.end());
    if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
        !this->PolicyWarnedCMP0022 && !usage_requirements_only) {
      // Compare the link implementation fallback link interface to the
      // preferred new link interface property and warn if different.
      std::vector<cmLinkItem> ifaceLibs;
      static const std::string newProp = "INTERFACE_LINK_LIBRARIES";
      if (const char* newExplicitLibraries = this->GetProperty(newProp)) {
        bool hadHeadSensitiveConditionDummy = false;
        this->ExpandLinkItems(newProp, newExplicitLibraries, config,
                              headTarget, usage_requirements_only, ifaceLibs,
                              hadHeadSensitiveConditionDummy);
      }
      if (ifaceLibs != iface.Libraries) {
        std::string oldLibraries = cmJoin(impl->Libraries, ";");
        std::string newLibraries = cmJoin(ifaceLibs, ";");
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

const cmLinkInterface* cmGeneratorTarget::GetImportLinkInterface(
  const std::string& config, cmGeneratorTarget const* headTarget,
  bool usage_requirements_only) const
{
  cmGeneratorTarget::ImportInfo const* info = this->GetImportInfo(config);
  if (!info) {
    return nullptr;
  }

  std::string CONFIG = cmSystemTools::UpperCase(config);
  cmHeadToLinkInterfaceMap& hm =
    (usage_requirements_only
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then return the one we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    return &hm.begin()->second;
  }

  cmOptionalLinkInterface& iface = hm[headTarget];
  if (!iface.AllDone) {
    iface.AllDone = true;
    iface.Multiplicity = info->Multiplicity;
    cmExpandList(info->Languages, iface.Languages);
    this->ExpandLinkItems(info->LibrariesProp, info->Libraries, config,
                          headTarget, usage_requirements_only, iface.Libraries,
                          iface.HadHeadSensitiveCondition);
    std::vector<std::string> deps = cmExpandedList(info->SharedDeps);
    this->LookupLinkItems(deps, cmListFileBacktrace(), iface.SharedDeps);
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

  const char* loc = nullptr;
  const char* imp = nullptr;
  std::string suffix;
  if (!this->Target->GetMappedConfig(desired_config, &loc, &imp, suffix)) {
    return;
  }

  // Get the link interface.
  {
    std::string linkProp = "INTERFACE_LINK_LIBRARIES";
    const char* propertyLibs = this->GetProperty(linkProp);

    if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
      if (!propertyLibs) {
        linkProp = cmStrCat("IMPORTED_LINK_INTERFACE_LIBRARIES", suffix);
        propertyLibs = this->GetProperty(linkProp);
      }

      if (!propertyLibs) {
        linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
        propertyLibs = this->GetProperty(linkProp);
      }
    }
    if (propertyLibs) {
      info.LibrariesProp = linkProp;
      info.Libraries = propertyLibs;
    }
  }
  if (this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    if (loc) {
      info.LibName = loc;
    }
    return;
  }

  // A provided configuration has been chosen.  Load the
  // configuration's properties.

  // Get the location.
  if (loc) {
    info.Location = loc;
  } else {
    std::string impProp = cmStrCat("IMPORTED_LOCATION", suffix);
    if (const char* config_location = this->GetProperty(impProp)) {
      info.Location = config_location;
    } else if (const char* location = this->GetProperty("IMPORTED_LOCATION")) {
      info.Location = location;
    }
  }

  // Get the soname.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY) {
    std::string soProp = cmStrCat("IMPORTED_SONAME", suffix);
    if (const char* config_soname = this->GetProperty(soProp)) {
      info.SOName = config_soname;
    } else if (const char* soname = this->GetProperty("IMPORTED_SONAME")) {
      info.SOName = soname;
    }
  }

  // Get the "no-soname" mark.
  if (this->GetType() == cmStateEnums::SHARED_LIBRARY) {
    std::string soProp = cmStrCat("IMPORTED_NO_SONAME", suffix);
    if (const char* config_no_soname = this->GetProperty(soProp)) {
      info.NoSOName = cmIsOn(config_no_soname);
    } else if (const char* no_soname =
                 this->GetProperty("IMPORTED_NO_SONAME")) {
      info.NoSOName = cmIsOn(no_soname);
    }
  }

  // Get the import library.
  if (imp) {
    info.ImportLibrary = imp;
  } else if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
             this->IsExecutableWithExports()) {
    std::string impProp = cmStrCat("IMPORTED_IMPLIB", suffix);
    if (const char* config_implib = this->GetProperty(impProp)) {
      info.ImportLibrary = config_implib;
    } else if (const char* implib = this->GetProperty("IMPORTED_IMPLIB")) {
      info.ImportLibrary = implib;
    }
  }

  // Get the link dependencies.
  {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_DEPENDENT_LIBRARIES", suffix);
    if (const char* config_libs = this->GetProperty(linkProp)) {
      info.SharedDeps = config_libs;
    } else if (const char* libs =
                 this->GetProperty("IMPORTED_LINK_DEPENDENT_LIBRARIES")) {
      info.SharedDeps = libs;
    }
  }

  // Get the link languages.
  if (this->LinkLanguagePropagatesToDependents()) {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_INTERFACE_LANGUAGES", suffix);
    if (const char* config_libs = this->GetProperty(linkProp)) {
      info.Languages = config_libs;
    } else if (const char* libs =
                 this->GetProperty("IMPORTED_LINK_INTERFACE_LANGUAGES")) {
      info.Languages = libs;
    }
  }

  // Get information if target is managed assembly.
  {
    std::string linkProp = "IMPORTED_COMMON_LANGUAGE_RUNTIME";
    if (auto pc = this->GetProperty(linkProp + suffix)) {
      info.Managed = this->CheckManagedType(pc);
    } else if (auto p = this->GetProperty(linkProp)) {
      info.Managed = this->CheckManagedType(p);
    }
  }

  // Get the cyclic repetition count.
  if (this->GetType() == cmStateEnums::STATIC_LIBRARY) {
    std::string linkProp =
      cmStrCat("IMPORTED_LINK_INTERFACE_MULTIPLICITY", suffix);
    if (const char* config_reps = this->GetProperty(linkProp)) {
      sscanf(config_reps, "%u", &info.Multiplicity);
    } else if (const char* reps =
                 this->GetProperty("IMPORTED_LINK_INTERFACE_MULTIPLICITY")) {
      sscanf(reps, "%u", &info.Multiplicity);
    }
  }
}

cmHeadToLinkInterfaceMap& cmGeneratorTarget::GetHeadToLinkInterfaceMap(
  const std::string& config) const
{
  std::string CONFIG = cmSystemTools::UpperCase(config);
  return this->LinkInterfaceMap[CONFIG];
}

cmHeadToLinkInterfaceMap&
cmGeneratorTarget::GetHeadToLinkInterfaceUsageRequirementsMap(
  const std::string& config) const
{
  std::string CONFIG = cmSystemTools::UpperCase(config);
  return this->LinkInterfaceUsageRequirementsOnlyMap[CONFIG];
}

const cmLinkImplementation* cmGeneratorTarget::GetLinkImplementation(
  const std::string& config) const
{
  // There is no link implementation for imported targets.
  if (this->IsImported()) {
    return nullptr;
  }

  std::string CONFIG = cmSystemTools::UpperCase(config);
  cmOptionalLinkImplementation& impl = this->LinkImplMap[CONFIG][this];
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, this);
  }
  if (!impl.LanguagesDone) {
    impl.LanguagesDone = true;
    this->ComputeLinkImplementationLanguages(config, impl);
  }
  return &impl;
}

bool cmGeneratorTarget::GetConfigCommonSourceFiles(
  std::vector<cmSourceFile*>& files) const
{
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs();

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
  cmStringRange rng = this->Target->GetSourceEntries();
  for (std::string const& entry : rng) {
    std::vector<std::string> files = cmExpandedList(entry);
    for (std::string const& li : files) {
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
  const char* deprecation = this->GetProperty("DEPRECATION");
  return deprecation && *deprecation;
}

std::string cmGeneratorTarget::GetDeprecation() const
{
  // find DEPRECATION property
  if (const char* deprecation = this->GetProperty("DEPRECATION")) {
    return deprecation;
  }
  return std::string();
}

void cmGeneratorTarget::GetLanguages(std::set<std::string>& languages,
                                     const std::string& config) const
{
  std::vector<cmSourceFile*> sourceFiles;
  this->GetSourceFiles(sourceFiles, config);
  for (cmSourceFile* src : sourceFiles) {
    const std::string& lang = src->GetOrDetermineLanguage();
    if (!lang.empty()) {
      languages.insert(lang);
    }
  }

  std::vector<cmGeneratorTarget*> objectLibraries;
  std::vector<cmSourceFile const*> externalObjects;
  if (!this->GlobalGenerator->GetConfigureDoneCMP0026()) {
    std::vector<cmGeneratorTarget*> objectTargets;
    this->GetObjectLibrariesCMP0026(objectTargets);
    objectLibraries.reserve(objectTargets.size());
    for (cmGeneratorTarget* gt : objectTargets) {
      objectLibraries.push_back(gt);
    }
  } else {
    this->GetExternalObjects(externalObjects, config);
    for (cmSourceFile const* extObj : externalObjects) {
      std::string objLib = extObj->GetObjectLibrary();
      if (cmGeneratorTarget* tgt =
            this->LocalGenerator->FindGeneratorTargetToUse(objLib)) {
        auto const objLibIt =
          std::find_if(objectLibraries.cbegin(), objectLibraries.cend(),
                       [tgt](cmGeneratorTarget* t) { return t == tgt; });
        if (objectLibraries.cend() == objLibIt) {
          objectLibraries.push_back(tgt);
        }
      }
    }
  }
  for (cmGeneratorTarget* objLib : objectLibraries) {
    objLib->GetLanguages(languages, config);
  }
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
  const char* linkLang = this->GetProperty("LINKER_LANGUAGE");
  if (linkLang && *linkLang) {
    languages.insert(linkLang);
  }
  return languages.size() == 1 && languages.count("CSharp") > 0;
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
        this->GetLinkImplementationLibraries(config)) {
    return !impl->Libraries.empty();
  }
  return false;
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibraries(
  const std::string& config) const
{
  return this->GetLinkImplementationLibrariesInternal(config, this);
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibrariesInternal(
  const std::string& config, cmGeneratorTarget const* head) const
{
  // There is no link implementation for imported targets.
  if (this->IsImported()) {
    return nullptr;
  }

  // Populate the link implementation libraries for this configuration.
  std::string CONFIG = cmSystemTools::UpperCase(config);
  HeadToLinkImplementationMap& hm = this->LinkImplMap[CONFIG];

  // If the link implementation does not depend on the head target
  // then return the one we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    return &hm.begin()->second;
  }

  cmOptionalLinkImplementation& impl = hm[head];
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, head);
  }
  return &impl;
}

bool cmGeneratorTarget::IsNullImpliedByLinkLibraries(
  const std::string& p) const
{
  return cmContains(this->LinkImplicitNullProperties, p);
}

void cmGeneratorTarget::ComputeLinkImplementationLibraries(
  const std::string& config, cmOptionalLinkImplementation& impl,
  cmGeneratorTarget const* head) const
{
  cmLocalGenerator const* lg = this->LocalGenerator;
  cmStringRange entryRange = this->Target->GetLinkImplementationEntries();
  cmBacktraceRange btRange = this->Target->GetLinkImplementationBacktraces();
  cmBacktraceRange::const_iterator btIt = btRange.begin();
  // Collect libraries directly linked in this configuration.
  for (cmStringRange::const_iterator le = entryRange.begin(),
                                     end = entryRange.end();
       le != end; ++le, ++btIt) {
    std::vector<std::string> llibs;
    // Keep this logic in sync with ExpandLinkItems.
    cmGeneratorExpressionDAGChecker dagChecker(this, "LINK_LIBRARIES", nullptr,
                                               nullptr);
    cmGeneratorExpression ge(*btIt);
    std::unique_ptr<cmCompiledGeneratorExpression> const cge = ge.Parse(*le);
    std::string const& evaluated =
      cge->Evaluate(this->LocalGenerator, config, head, &dagChecker);
    cmExpandList(evaluated, llibs);
    if (cge->GetHadHeadSensitiveCondition()) {
      impl.HadHeadSensitiveCondition = true;
    }

    for (std::string const& lib : llibs) {
      if (this->IsLinkLookupScope(lib, lg)) {
        continue;
      }

      // Skip entries that resolve to the target itself or are empty.
      std::string name = this->CheckCMP0004(lib);
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
      impl.Libraries.emplace_back(this->ResolveLinkItem(name, *btIt, lg),
                                  evaluated != *le);
    }

    std::set<std::string> const& seenProps = cge->GetSeenTargetProperties();
    for (std::string const& sp : seenProps) {
      if (!this->GetProperty(sp)) {
        this->LinkImplicitNullProperties.insert(sp);
      }
    }
    cge->GetMaxLanguageStandard(this, this->MaxLanguageStandards);
  }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  cmTargetLinkLibraryType linkType =
    CMP0003_ComputeLinkType(config, debugConfigs);
  cmTarget::LinkLibraryVectorType const& oldllibs =
    this->Target->GetOriginalLinkLibraries();
  for (cmTarget::LibraryID const& oldllib : oldllibs) {
    if (oldllib.second != GENERAL_LibraryType && oldllib.second != linkType) {
      std::string name = this->CheckCMP0004(oldllib.first);
      if (name == this->GetName() || name.empty()) {
        continue;
      }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(
        this->ResolveLinkItem(name, cmListFileBacktrace()));
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
  std::string const& name, cmListFileBacktrace const& bt) const
{
  return this->ResolveLinkItem(name, bt, this->LocalGenerator);
}

cmLinkItem cmGeneratorTarget::ResolveLinkItem(std::string const& name,
                                              cmListFileBacktrace const& bt,
                                              cmLocalGenerator const* lg) const
{
  TargetOrString resolved = this->ResolveTargetReference(name, lg);

  if (!resolved.Target) {
    return cmLinkItem(resolved.String, false, bt);
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
    return cmLinkItem(resolved.Target->GetName(), false, bt);
  }

  return cmLinkItem(resolved.Target, false, bt);
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

bool cmGeneratorTarget::IsExecutableWithExports() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

bool cmGeneratorTarget::HasImportLibrary(std::string const& config) const
{
  return (this->IsDLLPlatform() &&
          (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->IsExecutableWithExports()) &&
          // Assemblies which have only managed code do not have
          // import libraries.
          this->GetManagedType(config) != ManagedType::Managed) ||
    (this->Target->IsAIX() && this->IsExecutableWithExports());
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

bool cmGeneratorTarget::IsFrameworkOnApple() const
{
  return ((this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->GetType() == cmStateEnums::STATIC_LIBRARY) &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

bool cmGeneratorTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

bool cmGeneratorTarget::IsXCTestOnApple() const
{
  return (this->IsCFBundleOnApple() && this->GetPropertyAsBool("XCTEST"));
}

bool cmGeneratorTarget::IsCFBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::MODULE_LIBRARY &&
          this->Makefile->IsOn("APPLE") && this->GetPropertyAsBool("BUNDLE"));
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
  // 3. any value (safe,pure):  add /clr:[propval] as flag, target with
  //                            managed code only, no import lib
  return propval.empty() ? ManagedType::Mixed : ManagedType::Managed;
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
  if (auto* clr = this->GetProperty("COMMON_LANGUAGE_RUNTIME")) {
    return this->CheckManagedType(clr);
  }

  // C# targets are always managed. This language specific check
  // is added to avoid that the COMMON_LANGUAGE_RUNTIME target property
  // has to be set manually for C# targets.
  return this->IsCSharpOnly() ? ManagedType::Managed : ManagedType::Native;
}
