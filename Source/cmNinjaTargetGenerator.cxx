/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaTargetGenerator.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iterator>
#include <map>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmDyndepCollation.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNinjaNormalTargetGenerator.h"
#include "cmNinjaUtilityTargetGenerator.h"
#include "cmOutputConverter.h"
#include "cmRange.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

std::unique_ptr<cmNinjaTargetGenerator> cmNinjaTargetGenerator::New(
  cmGeneratorTarget* target)
{
  switch (target->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::OBJECT_LIBRARY:
      return cm::make_unique<cmNinjaNormalTargetGenerator>(target);

    case cmStateEnums::UTILITY:
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::GLOBAL_TARGET:
      return cm::make_unique<cmNinjaUtilityTargetGenerator>(target);

    default:
      return std::unique_ptr<cmNinjaTargetGenerator>();
  }
}

cmNinjaTargetGenerator::cmNinjaTargetGenerator(cmGeneratorTarget* target)
  : cmCommonTargetGenerator(target)
  , OSXBundleGenerator(nullptr)
  , LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(target->GetLocalGenerator()))
{
  for (auto const& fileConfig :
       target->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig)) {
    this->Configs[fileConfig].MacOSXContentGenerator =
      cm::make_unique<MacOSXContentGeneratorType>(this, fileConfig);
  }
}

cmNinjaTargetGenerator::~cmNinjaTargetGenerator() = default;

cmGeneratedFileStream& cmNinjaTargetGenerator::GetImplFileStream(
  const std::string& config) const
{
  return *this->GetGlobalGenerator()->GetImplFileStream(config);
}

cmGeneratedFileStream& cmNinjaTargetGenerator::GetCommonFileStream() const
{
  return *this->GetGlobalGenerator()->GetCommonFileStream();
}

cmGeneratedFileStream& cmNinjaTargetGenerator::GetRulesFileStream() const
{
  return *this->GetGlobalGenerator()->GetRulesFileStream();
}

cmGlobalNinjaGenerator* cmNinjaTargetGenerator::GetGlobalGenerator() const
{
  return this->LocalGenerator->GetGlobalNinjaGenerator();
}

std::string cmNinjaTargetGenerator::LanguageCompilerRule(
  const std::string& lang, const std::string& config,
  WithScanning withScanning) const
{
  return cmStrCat(
    lang, "_COMPILER__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    withScanning == WithScanning::Yes ? "_scanned_" : "_unscanned_", config);
}

std::string cmNinjaTargetGenerator::LanguagePreprocessAndScanRule(
  std::string const& lang, const std::string& config) const
{
  return cmStrCat(
    lang, "_PREPROCESS_SCAN__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

std::string cmNinjaTargetGenerator::LanguageScanRule(
  std::string const& lang, const std::string& config) const
{
  return cmStrCat(
    lang, "_SCAN__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

bool cmNinjaTargetGenerator::NeedExplicitPreprocessing(
  std::string const& lang) const
{
  return lang == "Fortran";
}

bool cmNinjaTargetGenerator::CompileWithDefines(std::string const& lang) const
{
  return this->Makefile->IsOn(
    cmStrCat("CMAKE_", lang, "_COMPILE_WITH_DEFINES"));
}

std::string cmNinjaTargetGenerator::LanguageDyndepRule(
  const std::string& lang, const std::string& config) const
{
  return cmStrCat(
    lang, "_DYNDEP__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

std::string cmNinjaTargetGenerator::OrderDependsTargetForTarget(
  const std::string& config)
{
  return this->GetGlobalGenerator()->OrderDependsTargetForTarget(
    this->GeneratorTarget, config);
}

// TODO: Most of the code is picked up from
// void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink),
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
// Refactor it.
std::string cmNinjaTargetGenerator::ComputeFlagsForObject(
  cmSourceFile const* source, const std::string& language,
  const std::string& config)
{
  std::unordered_map<std::string, std::string> pchSources;
  std::vector<std::string> architectures =
    this->GeneratorTarget->GetAppleArchs(config, language);
  if (architectures.empty()) {
    architectures.emplace_back();
  }

  std::string filterArch;
  for (const std::string& arch : architectures) {
    const std::string pchSource =
      this->GeneratorTarget->GetPchSource(config, language, arch);
    if (pchSource == source->GetFullPath()) {
      filterArch = arch;
    }
    if (!pchSource.empty()) {
      pchSources.insert(std::make_pair(pchSource, arch));
    }
  }

  std::string flags;
  // Explicitly add the explicit language flag before any other flag
  // so user flags can override it.
  this->GeneratorTarget->AddExplicitLanguageFlags(flags, *source);

  if (!flags.empty()) {
    flags += " ";
  }
  flags += this->GetFlags(language, config, filterArch);

  // Add Fortran format flags.
  if (language == "Fortran") {
    this->AppendFortranFormatFlags(flags, *source);
    this->AppendFortranPreprocessFlags(flags, *source,
                                       PreprocessFlagsRequired::NO);
  }

  // Add source file specific flags.
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, language);

  const std::string COMPILE_FLAGS("COMPILE_FLAGS");
  if (cmValue cflags = source->GetProperty(COMPILE_FLAGS)) {
    this->LocalGenerator->AppendFlags(
      flags, genexInterpreter.Evaluate(*cflags, COMPILE_FLAGS));
  }

  const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
  if (cmValue coptions = source->GetProperty(COMPILE_OPTIONS)) {
    this->LocalGenerator->AppendCompileOptions(
      flags, genexInterpreter.Evaluate(*coptions, COMPILE_OPTIONS));
  }

  // Add precompile headers compile options.
  if (!pchSources.empty() && !source->GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    std::string pchOptions;
    auto pchIt = pchSources.find(source->GetFullPath());
    if (pchIt != pchSources.end()) {
      pchOptions = this->GeneratorTarget->GetPchCreateCompileOptions(
        config, language, pchIt->second);
    } else {
      pchOptions =
        this->GeneratorTarget->GetPchUseCompileOptions(config, language);
    }

    this->LocalGenerator->AppendCompileOptions(
      flags, genexInterpreter.Evaluate(pchOptions, COMPILE_OPTIONS));
  }

  auto const* fs = this->GeneratorTarget->GetFileSetForSource(config, source);
  if (fs && fs->GetType() == "CXX_MODULES"_s) {
    if (source->GetLanguage() != "CXX"_s) {
      this->GetMakefile()->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", this->GeneratorTarget->Target->GetName(),
                 "\" contains the source\n  ", source->GetFullPath(),
                 "\nin a file set of type \"", fs->GetType(),
                 R"(" but the source is not classified as a "CXX" source.)"));
    }
  }

  return flags;
}

void cmNinjaTargetGenerator::AddIncludeFlags(std::string& languageFlags,
                                             std::string const& language,
                                             const std::string& config)
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              language, config);
  // Add include directory flags.
  std::string includeFlags = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, language, config, false);
  if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
    std::replace(includeFlags.begin(), includeFlags.end(), '\\', '/');
  }

  this->LocalGenerator->AppendFlags(languageFlags, includeFlags);
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string cmNinjaTargetGenerator::ComputeDefines(cmSourceFile const* source,
                                                   const std::string& language,
                                                   const std::string& config)
{
  std::set<std::string> defines;
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, language);

  // Seriously??
  if (this->GetGlobalGenerator()->IsMultiConfig()) {
    defines.insert(cmStrCat("CMAKE_INTDIR=\"", config, '"'));
  }

  const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
  if (cmValue compile_defs = source->GetProperty(COMPILE_DEFINITIONS)) {
    this->LocalGenerator->AppendDefines(
      defines, genexInterpreter.Evaluate(*compile_defs, COMPILE_DEFINITIONS));
  }

  std::string defPropName =
    cmStrCat("COMPILE_DEFINITIONS_", cmSystemTools::UpperCase(config));
  if (cmValue config_compile_defs = source->GetProperty(defPropName)) {
    this->LocalGenerator->AppendDefines(
      defines,
      genexInterpreter.Evaluate(*config_compile_defs, COMPILE_DEFINITIONS));
  }

  std::string definesString = this->GetDefines(language, config);
  this->LocalGenerator->JoinDefines(defines, definesString, language);

  return definesString;
}

std::string cmNinjaTargetGenerator::ComputeIncludes(
  cmSourceFile const* source, const std::string& language,
  const std::string& config)
{
  std::vector<std::string> includes;
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->LocalGenerator, config, this->GeneratorTarget, language);

  const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
  if (cmValue cincludes = source->GetProperty(INCLUDE_DIRECTORIES)) {
    this->LocalGenerator->AppendIncludeDirectories(
      includes, genexInterpreter.Evaluate(*cincludes, INCLUDE_DIRECTORIES),
      *source);
  }

  std::string includesString = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, language, config, false);
  this->LocalGenerator->AppendFlags(includesString,
                                    this->GetIncludes(language, config));

  return includesString;
}

cmNinjaDeps cmNinjaTargetGenerator::ComputeLinkDeps(
  const std::string& linkLanguage, const std::string& config,
  bool ignoreType) const
{
  // Static libraries never depend on other targets for linking.
  if (!ignoreType &&
      (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
       this->GeneratorTarget->GetType() == cmStateEnums::OBJECT_LIBRARY)) {
    return cmNinjaDeps();
  }

  cmComputeLinkInformation* cli =
    this->GeneratorTarget->GetLinkInformation(config);
  if (!cli) {
    return cmNinjaDeps();
  }

  const std::vector<std::string>& deps = cli->GetDepends();
  cmNinjaDeps result(deps.size());
  std::transform(deps.begin(), deps.end(), result.begin(),
                 this->MapToNinjaPath());

  // Add a dependency on the link definitions file, if any.
  if (cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
        this->GeneratorTarget->GetModuleDefinitionInfo(config)) {
    for (cmSourceFile const* src : mdi->Sources) {
      result.push_back(this->ConvertToNinjaPath(src->GetFullPath()));
    }
  }

  // Add a dependency on user-specified manifest files, if any.
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, config);
  for (cmSourceFile const* manifest_src : manifest_srcs) {
    result.push_back(this->ConvertToNinjaPath(manifest_src->GetFullPath()));
  }

  // Add user-specified dependencies.
  std::vector<std::string> linkDeps;
  this->GeneratorTarget->GetLinkDepends(linkDeps, config, linkLanguage);
  std::transform(linkDeps.begin(), linkDeps.end(), std::back_inserter(result),
                 this->MapToNinjaPath());

  return result;
}

std::string cmNinjaTargetGenerator::GetCompiledSourceNinjaPath(
  cmSourceFile const* source) const
{
  // Pass source files to the compiler by absolute path.
  return this->ConvertToNinjaAbsPath(source->GetFullPath());
}

std::string cmNinjaTargetGenerator::GetObjectFilePath(
  cmSourceFile const* source, const std::string& config) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += '/';
  }
  std::string const& objectName = this->GeneratorTarget->GetObjectName(source);
  path += cmStrCat(
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
    this->GetGlobalGenerator()->ConfigDirectory(config), '/', objectName);
  return path;
}

std::string cmNinjaTargetGenerator::GetClangTidyReplacementsFilePath(
  std::string const& directory, cmSourceFile const& source,
  std::string const& config) const
{
  auto path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += '/';
  }
  path = cmStrCat(directory, '/', path);
  auto const& objectName = this->GeneratorTarget->GetObjectName(&source);
  path =
    cmStrCat(std::move(path),
             this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
             this->GetGlobalGenerator()->ConfigDirectory(config), '/',
             objectName, ".yaml");
  return path;
}

std::string cmNinjaTargetGenerator::GetPreprocessedFilePath(
  cmSourceFile const* source, const std::string& config) const
{
  // Choose an extension to compile already-preprocessed source.
  std::string ppExt = source->GetExtension();
  if (cmHasLiteralPrefix(ppExt, "F")) {
    // Some Fortran compilers automatically enable preprocessing for
    // upper-case extensions.  Since the source is already preprocessed,
    // use a lower-case extension.
    ppExt = cmSystemTools::LowerCase(ppExt);
  }
  if (ppExt == "fpp") {
    // Some Fortran compilers automatically enable preprocessing for
    // the ".fpp" extension.  Since the source is already preprocessed,
    // use the ".f" extension.
    ppExt = "f";
  }

  // Take the object file name and replace the extension.
  std::string const& objName = this->GeneratorTarget->GetObjectName(source);
  std::string const& objExt =
    this->GetGlobalGenerator()->GetLanguageOutputExtension(*source);
  assert(objName.size() >= objExt.size());
  std::string const ppName =
    cmStrCat(objName.substr(0, objName.size() - objExt.size()), "-pp.", ppExt);

  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += '/';
  }
  path +=
    cmStrCat(this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
             this->GetGlobalGenerator()->ConfigDirectory(config), '/', ppName);
  return path;
}

std::string cmNinjaTargetGenerator::GetDyndepFilePath(
  std::string const& lang, const std::string& config) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty()) {
    path += '/';
  }
  path += cmStrCat(
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
    this->GetGlobalGenerator()->ConfigDirectory(config), '/', lang, ".dd");
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetDependInfoPath(
  std::string const& lang, const std::string& config) const
{
  std::string path =
    cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/',
             this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
             this->GetGlobalGenerator()->ConfigDirectory(config), '/', lang,
             "DependInfo.json");
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetOutputDir(
  const std::string& config) const
{
  std::string dir = this->GeneratorTarget->GetDirectory(config);
  return this->ConvertToNinjaPath(dir);
}

std::string cmNinjaTargetGenerator::GetTargetFilePath(
  const std::string& name, const std::string& config) const
{
  std::string path = this->GetTargetOutputDir(config);
  if (path.empty() || path == ".") {
    return name;
  }
  path += cmStrCat('/', name);
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetName() const
{
  return this->GeneratorTarget->GetName();
}

bool cmNinjaTargetGenerator::SetMsvcTargetPdbVariable(
  cmNinjaVars& vars, const std::string& config) const
{
  cmMakefile* mf = this->GetMakefile();
  if (mf->GetDefinition("MSVC_C_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CXX_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CUDA_ARCHITECTURE_ID")) {
    std::string pdbPath;
    std::string compilePdbPath = this->ComputeTargetCompilePDB(config);
    if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE ||
        this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
      pdbPath = cmStrCat(this->GeneratorTarget->GetPDBDirectory(config), '/',
                         this->GeneratorTarget->GetPDBName(config));
    }

    vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
      this->ConvertToNinjaPath(pdbPath), cmOutputConverter::SHELL);
    vars["TARGET_COMPILE_PDB"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->ConvertToNinjaPath(compilePdbPath), cmOutputConverter::SHELL);

    this->EnsureParentDirectoryExists(pdbPath);
    this->EnsureParentDirectoryExists(compilePdbPath);
    return true;
  }
  return false;
}

void cmNinjaTargetGenerator::WriteLanguageRules(const std::string& language,
                                                const std::string& config)
{
#ifdef NINJA_GEN_VERBOSE_FILES
  this->GetRulesFileStream() << "# Rules for language " << language << "\n\n";
#endif
  this->WriteCompileRule(language, config);
}

namespace {
// Create the command to run the dependency scanner
std::string GetScanCommand(cm::string_view cmakeCmd, cm::string_view tdi,
                           cm::string_view lang, cm::string_view srcFile,
                           cm::string_view ddiFile)
{
  return cmStrCat(cmakeCmd, " -E cmake_ninja_depends --tdi=", tdi,
                  " --lang=", lang, " --src=", srcFile, " --out=$out",
                  " --dep=$DEP_FILE --obj=$OBJ_FILE --ddi=", ddiFile);
}

// Helper function to create dependency scanning rule that may or may
// not perform explicit preprocessing too.
cmNinjaRule GetScanRule(
  std::string const& ruleName, std::string const& ppFileName,
  std::string const& deptype,
  cmRulePlaceholderExpander::RuleVariables const& vars,
  const std::string& responseFlag, const std::string& flags,
  cmRulePlaceholderExpander* const rulePlaceholderExpander,
  cmLocalNinjaGenerator* generator, std::vector<std::string> scanCmds,
  const std::string& outputConfig)
{
  cmNinjaRule rule(ruleName);
  // Scanning always uses a depfile for preprocessor dependencies.
  if (deptype == "msvc"_s) {
    rule.DepType = deptype;
    rule.DepFile.clear();
  } else {
    rule.DepType.clear(); // no deps= for multiple outputs
    rule.DepFile = "$DEP_FILE";
  }

  cmRulePlaceholderExpander::RuleVariables scanVars;
  scanVars.CMTargetName = vars.CMTargetName;
  scanVars.CMTargetType = vars.CMTargetType;
  scanVars.Language = vars.Language;
  scanVars.Object = "$OBJ_FILE";
  scanVars.PreprocessedSource = ppFileName.c_str();
  scanVars.DynDepFile = "$DYNDEP_INTERMEDIATE_FILE";
  scanVars.DependencyFile = rule.DepFile.c_str();
  scanVars.DependencyTarget = "$out";

  // Scanning needs the same preprocessor settings as direct compilation would.
  scanVars.Source = vars.Source;
  scanVars.Defines = vars.Defines;
  scanVars.Includes = vars.Includes;

  // Scanning needs the compilation flags too.
  std::string scanFlags = flags;

  // If using a response file, move defines, includes, and flags into it.
  if (!responseFlag.empty()) {
    rule.RspFile = "$RSP_FILE";
    rule.RspContent =
      cmStrCat(' ', scanVars.Defines, ' ', scanVars.Includes, ' ', scanFlags);
    scanFlags = cmStrCat(responseFlag, rule.RspFile);
    scanVars.Defines = "";
    scanVars.Includes = "";
  }

  scanVars.Flags = scanFlags.c_str();

  // Rule for scanning a source file.
  for (std::string& scanCmd : scanCmds) {
    rulePlaceholderExpander->ExpandRuleVariables(generator, scanCmd, scanVars);
  }
  rule.Command =
    generator->BuildCommandLine(scanCmds, outputConfig, outputConfig);

  return rule;
}
}

void cmNinjaTargetGenerator::WriteCompileRule(const std::string& lang,
                                              const std::string& config)
{
  // For some cases we scan to dynamically discover dependencies.
  bool const needDyndep = this->GetGeneratorTarget()->NeedDyndep(lang, config);

  if (needDyndep) {
    this->WriteCompileRule(lang, config, WithScanning::Yes);
  }
  this->WriteCompileRule(lang, config, WithScanning::No);
}

void cmNinjaTargetGenerator::WriteCompileRule(const std::string& lang,
                                              const std::string& config,
                                              WithScanning withScanning)
{
  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
  vars.CMTargetType =
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()).c_str();
  vars.Language = lang.c_str();
  vars.Source = "$in";
  vars.Object = "$out";
  vars.Defines = "$DEFINES";
  vars.Includes = "$INCLUDES";
  vars.TargetPDB = "$TARGET_PDB";
  vars.TargetCompilePDB = "$TARGET_COMPILE_PDB";
  vars.ObjectDir = "$OBJECT_DIR";
  vars.ObjectFileDir = "$OBJECT_FILE_DIR";
  vars.CudaCompileMode = "$CUDA_COMPILE_MODE";
  vars.ISPCHeader = "$ISPC_HEADER_FILE";

  cmMakefile* mf = this->GetMakefile();

  // For some cases we scan to dynamically discover dependencies.
  bool const compilationPreprocesses = !this->NeedExplicitPreprocessing(lang);

  std::string flags = "$FLAGS";

  std::string responseFlag;
  bool const lang_supports_response = lang != "RC";
  if (lang_supports_response && this->ForceResponseFile()) {
    std::string const responseFlagVar =
      cmStrCat("CMAKE_", lang, "_RESPONSE_FILE_FLAG");
    responseFlag = this->Makefile->GetSafeDefinition(responseFlagVar);
    if (responseFlag.empty() && lang != "CUDA") {
      responseFlag = "@";
    }
  }
  std::string const modmapFormatVar =
    cmStrCat("CMAKE_EXPERIMENTAL_", lang, "_MODULE_MAP_FORMAT");
  std::string const modmapFormat =
    this->Makefile->GetSafeDefinition(modmapFormatVar);

  auto rulePlaceholderExpander =
    this->GetLocalGenerator()->CreateRulePlaceholderExpander();

  std::string const tdi = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(this->GetTargetDependInfoPath(lang, config)),
    cmLocalGenerator::SHELL);

  std::string launcher;
  std::string val = this->GetLocalGenerator()->GetRuleLauncher(
    this->GetGeneratorTarget(), "RULE_LAUNCH_COMPILE", config);
  if (cmNonempty(val)) {
    launcher = cmStrCat(val, ' ');
  }

  std::string const cmakeCmd =
    this->GetLocalGenerator()->ConvertToOutputFormat(
      cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);

  if (withScanning == WithScanning::Yes) {
    const auto& scanDepType = this->GetMakefile()->GetSafeDefinition(
      cmStrCat("CMAKE_EXPERIMENTAL_", lang, "_SCANDEP_DEPFILE_FORMAT"));

    // Rule to scan dependencies of sources that need preprocessing.
    {
      cmList scanCommands;
      std::string scanRuleName;
      std::string ppFileName;
      if (compilationPreprocesses) {
        scanRuleName = this->LanguageScanRule(lang, config);
        ppFileName = "$PREPROCESSED_OUTPUT_FILE";
        std::string const& scanCommand = mf->GetRequiredDefinition(
          cmStrCat("CMAKE_EXPERIMENTAL_", lang, "_SCANDEP_SOURCE"));
        scanCommands.assign(scanCommand);
        for (auto& i : scanCommands) {
          i = cmStrCat(launcher, i);
        }
      } else {
        scanRuleName = this->LanguagePreprocessAndScanRule(lang, config);
        ppFileName = "$out";
        std::string const& ppCommmand = mf->GetRequiredDefinition(
          cmStrCat("CMAKE_", lang, "_PREPROCESS_SOURCE"));
        scanCommands.assign(ppCommmand);
        for (auto& i : scanCommands) {
          i = cmStrCat(launcher, i);
        }
        scanCommands.emplace_back(GetScanCommand(cmakeCmd, tdi, lang, "$out",
                                                 "$DYNDEP_INTERMEDIATE_FILE"));
      }

      auto scanRule = GetScanRule(
        scanRuleName, ppFileName, scanDepType, vars, responseFlag, flags,
        rulePlaceholderExpander.get(), this->GetLocalGenerator(),
        std::move(scanCommands), config);

      scanRule.Comment =
        cmStrCat("Rule for generating ", lang, " dependencies.");
      if (compilationPreprocesses) {
        scanRule.Description =
          cmStrCat("Scanning $in for ", lang, " dependencies");
      } else {
        scanRule.Description =
          cmStrCat("Building ", lang, " preprocessed $out");
      }

      this->GetGlobalGenerator()->AddRule(scanRule);
    }

    if (!compilationPreprocesses) {
      // Compilation will not preprocess, so it does not need the defines
      // unless the compiler wants them for some other purpose.
      if (!this->CompileWithDefines(lang)) {
        vars.Defines = "";
      }

      // Rule to scan dependencies of sources that do not need preprocessing.
      std::string const& scanRuleName = this->LanguageScanRule(lang, config);
      std::vector<std::string> scanCommands;
      scanCommands.emplace_back(
        GetScanCommand(cmakeCmd, tdi, lang, "$in", "$out"));

      auto scanRule =
        GetScanRule(scanRuleName, "", scanDepType, vars, "", flags,
                    rulePlaceholderExpander.get(), this->GetLocalGenerator(),
                    std::move(scanCommands), config);

      // Write the rule for generating dependencies for the given language.
      scanRule.Comment = cmStrCat("Rule for generating ", lang,
                                  " dependencies on non-preprocessed files.");
      scanRule.Description =
        cmStrCat("Generating ", lang, " dependencies for $in");

      this->GetGlobalGenerator()->AddRule(scanRule);
    }

    // Write the rule for ninja dyndep file generation.
    cmNinjaRule rule(this->LanguageDyndepRule(lang, config));
    // Command line length is almost always limited -> use response file for
    // dyndep rules
    rule.RspFile = "$out.rsp";
    rule.RspContent = "$in";

    // Run CMake dependency scanner on the source file (using the preprocessed
    // source if that was performed).
    std::string ddModmapArg;
    if (!modmapFormat.empty()) {
      ddModmapArg += cmStrCat(" --modmapfmt=", modmapFormat);
    }
    {
      std::vector<std::string> ddCmds;
      {
        std::string ccmd = cmStrCat(
          cmakeCmd, " -E cmake_ninja_dyndep --tdi=", tdi, " --lang=", lang,
          ddModmapArg, " --dd=$out @", rule.RspFile);
        ddCmds.emplace_back(std::move(ccmd));
      }
      rule.Command =
        this->GetLocalGenerator()->BuildCommandLine(ddCmds, config, config);
    }
    rule.Comment =
      cmStrCat("Rule to generate ninja dyndep files for ", lang, '.');
    rule.Description = cmStrCat("Generating ", lang, " dyndep file $out");
    this->GetGlobalGenerator()->AddRule(rule);
  }

  cmNinjaRule rule(this->LanguageCompilerRule(lang, config, withScanning));
  // If using a response file, move defines, includes, and flags into it.
  if (!responseFlag.empty()) {
    rule.RspFile = "$RSP_FILE";
    rule.RspContent =
      cmStrCat(' ', vars.Defines, ' ', vars.Includes, ' ', flags);
    flags = cmStrCat(responseFlag, rule.RspFile);
    vars.Defines = "";
    vars.Includes = "";
  }

  // Tell ninja dependency format so all deps can be loaded into a database
  std::string cldeps;
  if (!compilationPreprocesses) {
    // The compiler will not do preprocessing, so it has no such dependencies.
  } else if (mf->IsOn(cmStrCat("CMAKE_NINJA_CMCLDEPS_", lang))) {
    // For the MS resource compiler we need cmcldeps, but skip dependencies
    // for source-file try_compile cases because they are always fresh.
    if (!mf->GetIsSourceFileTryCompile()) {
      rule.DepType = "gcc";
      rule.DepFile = "$DEP_FILE";
      cmValue d = mf->GetDefinition("CMAKE_C_COMPILER");
      const std::string cl =
        d ? *d : mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
      std::string cmcldepsPath;
      cmSystemTools::GetShortPath(cmSystemTools::GetCMClDepsCommand(),
                                  cmcldepsPath);
      cldeps = cmStrCat(cmcldepsPath, ' ', lang, ' ', vars.Source,
                        " $DEP_FILE $out \"",
                        mf->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX"),
                        "\" \"", cl, "\" ");
    }
  } else {
    const auto& depType = this->GetMakefile()->GetSafeDefinition(
      cmStrCat("CMAKE_", lang, "_DEPFILE_FORMAT"));
    if (depType == "msvc"_s) {
      rule.DepType = "msvc";
      rule.DepFile.clear();
    } else {
      rule.DepType = "gcc";
      rule.DepFile = "$DEP_FILE";
    }
    vars.DependencyFile = rule.DepFile.c_str();
    vars.DependencyTarget = "$out";

    const std::string flagsName = cmStrCat("CMAKE_DEPFILE_FLAGS_", lang);
    std::string depfileFlags = mf->GetSafeDefinition(flagsName);
    if (!depfileFlags.empty()) {
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   depfileFlags, vars);
      flags += cmStrCat(' ', depfileFlags);
    }
  }

  if (withScanning == WithScanning::Yes && !modmapFormat.empty()) {
    std::string modmapFlags = mf->GetRequiredDefinition(
      cmStrCat("CMAKE_EXPERIMENTAL_", lang, "_MODULE_MAP_FLAG"));
    cmSystemTools::ReplaceString(modmapFlags, "<MODULE_MAP_FILE>",
                                 "$DYNDEP_MODULE_MAP_FILE");
    flags += cmStrCat(' ', modmapFlags);
  }

  vars.Flags = flags.c_str();
  vars.DependencyFile = rule.DepFile.c_str();

  std::string cudaCompileMode;
  if (lang == "CUDA") {
    if (this->GeneratorTarget->GetPropertyAsBool(
          "CUDA_SEPARABLE_COMPILATION")) {
      const std::string& rdcFlag =
        this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_RDC_FLAG");
      cudaCompileMode = cmStrCat(cudaCompileMode, rdcFlag, " ");
    }
    static std::array<cm::string_view, 4> const compileModes{
      { "PTX"_s, "CUBIN"_s, "FATBIN"_s, "OPTIX"_s }
    };
    bool useNormalCompileMode = true;
    for (cm::string_view mode : compileModes) {
      auto propName = cmStrCat("CUDA_", mode, "_COMPILATION");
      auto defName = cmStrCat("_CMAKE_CUDA_", mode, "_FLAG");
      if (this->GeneratorTarget->GetPropertyAsBool(propName)) {
        const std::string& flag =
          this->Makefile->GetRequiredDefinition(defName);
        cudaCompileMode = cmStrCat(cudaCompileMode, flag);
        useNormalCompileMode = false;
        break;
      }
    }
    if (useNormalCompileMode) {
      const std::string& wholeFlag =
        this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_WHOLE_FLAG");
      cudaCompileMode = cmStrCat(cudaCompileMode, wholeFlag);
    }
    vars.CudaCompileMode = cudaCompileMode.c_str();
  }

  // Rule for compiling object file.
  const std::string cmdVar = cmStrCat("CMAKE_", lang, "_COMPILE_OBJECT");
  const std::string& compileCmd = mf->GetRequiredDefinition(cmdVar);
  cmList compileCmds(compileCmd);

  if (!compileCmds.empty()) {
    compileCmds.front().insert(0, "${CODE_CHECK}");
    compileCmds.front().insert(0, "${LAUNCHER}");
  }

  if (!compileCmds.empty()) {
    compileCmds.front().insert(0, cldeps);
  }

  const auto& extraCommands = this->GetMakefile()->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_DEPENDS_EXTRA_COMMANDS"));
  if (!extraCommands.empty()) {
    compileCmds.append(extraCommands);
  }

  for (auto& i : compileCmds) {
    i = cmStrCat(launcher, i);
    rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(), i,
                                                 vars);
  }

  rule.Command =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds, config, config);

  // Write the rule for compiling file of the given language.
  rule.Comment = cmStrCat("Rule for compiling ", lang, " files.");
  rule.Description = cmStrCat("Building ", lang, " object $out");
  this->GetGlobalGenerator()->AddRule(rule);
}

void cmNinjaTargetGenerator::WriteObjectBuildStatements(
  const std::string& config, const std::string& fileConfig,
  bool firstForConfig)
{
  this->GeneratorTarget->CheckCxxModuleStatus(config);

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetImplFileStream(fileConfig));
  this->GetImplFileStream(fileConfig)
    << "# Object build statements for "
    << cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType())
    << " target " << this->GetTargetName() << "\n\n";

  {
    std::vector<cmSourceFile const*> customCommands;
    this->GeneratorTarget->GetCustomCommands(customCommands, config);
    for (cmSourceFile const* sf : customCommands) {
      cmCustomCommand const* cc = sf->GetCustomCommand();
      this->GetLocalGenerator()->AddCustomCommandTarget(
        cc, this->GetGeneratorTarget());
      // Record the custom commands for this target. The container is used
      // in WriteObjectBuildStatement when called in a loop below.
      this->Configs[config].CustomCommands.push_back(cc);
    }
  }
  {
    std::vector<cmSourceFile const*> headerSources;
    this->GeneratorTarget->GetHeaderSources(headerSources, config);
    this->OSXBundleGenerator->GenerateMacOSXContentStatements(
      headerSources, this->Configs[fileConfig].MacOSXContentGenerator.get(),
      config);
  }
  {
    std::vector<cmSourceFile const*> extraSources;
    this->GeneratorTarget->GetExtraSources(extraSources, config);
    this->OSXBundleGenerator->GenerateMacOSXContentStatements(
      extraSources, this->Configs[fileConfig].MacOSXContentGenerator.get(),
      config);
  }
  if (firstForConfig) {
    cmValue pchExtension =
      this->GetMakefile()->GetDefinition("CMAKE_PCH_EXTENSION");

    std::vector<cmSourceFile const*> externalObjects;
    this->GeneratorTarget->GetExternalObjects(externalObjects, config);
    for (cmSourceFile const* sf : externalObjects) {
      auto objectFileName = this->GetGlobalGenerator()->ExpandCFGIntDir(
        this->ConvertToNinjaPath(sf->GetFullPath()), config);
      if (!cmHasSuffix(objectFileName, pchExtension)) {
        this->Configs[config].Objects.push_back(objectFileName);
      }
    }
  }

  {
    cmNinjaBuild build("phony");
    build.Comment =
      cmStrCat("Order-only phony target for ", this->GetTargetName());
    build.Outputs.push_back(this->OrderDependsTargetForTarget(config));

    cmNinjaDeps& orderOnlyDeps = build.OrderOnlyDeps;
    this->GetLocalGenerator()->AppendTargetDepends(
      this->GeneratorTarget, orderOnlyDeps, config, fileConfig,
      DependOnTargetOrdering);

    // Add order-only dependencies on other files associated with the target.
    cm::append(orderOnlyDeps, this->Configs[config].ExtraFiles);

    // Add order-only dependencies on custom command outputs.
    for (cmCustomCommand const* cc : this->Configs[config].CustomCommands) {
      cmCustomCommandGenerator ccg(*cc, config, this->GetLocalGenerator());
      const std::vector<std::string>& ccoutputs = ccg.GetOutputs();
      const std::vector<std::string>& ccbyproducts = ccg.GetByproducts();
      std::transform(ccoutputs.begin(), ccoutputs.end(),
                     std::back_inserter(orderOnlyDeps),
                     this->MapToNinjaPath());
      std::transform(ccbyproducts.begin(), ccbyproducts.end(),
                     std::back_inserter(orderOnlyDeps),
                     this->MapToNinjaPath());
    }

    std::sort(orderOnlyDeps.begin(), orderOnlyDeps.end());
    orderOnlyDeps.erase(
      std::unique(orderOnlyDeps.begin(), orderOnlyDeps.end()),
      orderOnlyDeps.end());

    // The phony target must depend on at least one input or ninja will explain
    // that "output ... of phony edge with no inputs doesn't exist" and
    // consider the phony output "dirty".
    if (orderOnlyDeps.empty()) {
      // Any path that always exists will work here.  It would be nice to
      // use just "." but that is not supported by Ninja < 1.7.
      std::string tgtDir = cmStrCat(
        this->LocalGenerator->GetCurrentBinaryDirectory(), '/',
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget));
      orderOnlyDeps.push_back(this->ConvertToNinjaPath(tgtDir));
    }

    this->GetGlobalGenerator()->WriteBuild(this->GetImplFileStream(fileConfig),
                                           build);
  }

  {
    std::vector<cmSourceFile const*> objectSources;
    this->GeneratorTarget->GetObjectSources(objectSources, config);

    for (cmSourceFile const* sf : objectSources) {
      this->WriteObjectBuildStatement(sf, config, fileConfig, firstForConfig);
    }
  }

  for (auto const& langScanningFiles : this->Configs[config].ScanningInfo) {
    std::string const& language = langScanningFiles.first;
    std::vector<ScanningFiles> const& scanningFiles = langScanningFiles.second;

    cmNinjaBuild build(this->LanguageDyndepRule(language, config));
    build.Outputs.push_back(this->GetDyndepFilePath(language, config));
    build.ImplicitOuts.push_back(
      cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/',
               this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget),
               this->GetGlobalGenerator()->ConfigDirectory(config), '/',
               language, "Modules.json"));
    for (auto const& scanFiles : scanningFiles) {
      if (!scanFiles.ScanningOutput.empty()) {
        build.ExplicitDeps.push_back(scanFiles.ScanningOutput);
      }
      if (!scanFiles.ModuleMapFile.empty()) {
        build.ImplicitOuts.push_back(scanFiles.ModuleMapFile);
      }
    }

    this->WriteTargetDependInfo(language, config);

    for (std::string const& l :
         this->GetLinkedTargetDirectories(language, config)) {
      build.ImplicitDeps.push_back(cmStrCat(l, '/', language, "Modules.json"));
    }

    this->GetGlobalGenerator()->WriteBuild(this->GetImplFileStream(fileConfig),
                                           build);
  }

  this->GetImplFileStream(fileConfig) << "\n";
}

void cmNinjaTargetGenerator::GenerateSwiftOutputFileMap(
  const std::string& config, std::string& flags)
{
  if (this->Configs[config].SwiftOutputMap.empty()) {
    return;
  }

  std::string const targetSwiftDepsPath = [this, config]() -> std::string {
    cmGeneratorTarget const* target = this->GeneratorTarget;
    if (cmValue name = target->GetProperty("Swift_DEPENDENCIES_FILE")) {
      return *name;
    }
    return this->ConvertToNinjaPath(cmStrCat(target->GetSupportDirectory(),
                                             '/', config, '/',
                                             target->GetName(), ".swiftdeps"));
  }();

  std::string mapFilePath =
    cmStrCat(this->GeneratorTarget->GetSupportDirectory(), '/', config, '/',
             "output-file-map.json");

  // build the global target dependencies
  // https://github.com/apple/swift/blob/master/docs/Driver.md#output-file-maps
  Json::Value deps(Json::objectValue);
  deps["swift-dependencies"] = targetSwiftDepsPath;
  this->Configs[config].SwiftOutputMap[""] = deps;

  cmGeneratedFileStream output(mapFilePath);
  output << this->Configs[config].SwiftOutputMap;

  // Add flag
  this->LocalGenerator->AppendFlags(flags, "-output-file-map");
  this->LocalGenerator->AppendFlagEscape(flags,
                                         ConvertToNinjaPath(mapFilePath));
}

namespace {
cmNinjaBuild GetScanBuildStatement(const std::string& ruleName,
                                   const std::string& ppFileName,
                                   bool compilePP, bool compilePPWithDefines,
                                   bool compilationPreprocesses,
                                   cmNinjaBuild& objBuild, cmNinjaVars& vars,
                                   const std::string& objectFileName,
                                   cmLocalGenerator* lg)
{
  cmNinjaBuild scanBuild(ruleName);

  scanBuild.RspFile = "$out.rsp";

  if (compilePP) {
    // Move compilation dependencies to the scan/preprocessing build statement.
    std::swap(scanBuild.ExplicitDeps, objBuild.ExplicitDeps);
    std::swap(scanBuild.ImplicitDeps, objBuild.ImplicitDeps);
    std::swap(scanBuild.OrderOnlyDeps, objBuild.OrderOnlyDeps);
    std::swap(scanBuild.Variables["IN_ABS"], vars["IN_ABS"]);

    // The actual compilation will now use the preprocessed source.
    objBuild.ExplicitDeps.push_back(ppFileName);
  } else {
    // Copy compilation dependencies to the scan/preprocessing build statement.
    scanBuild.ExplicitDeps = objBuild.ExplicitDeps;
    scanBuild.ImplicitDeps = objBuild.ImplicitDeps;
    scanBuild.OrderOnlyDeps = objBuild.OrderOnlyDeps;
    scanBuild.Variables["IN_ABS"] = vars["IN_ABS"];
  }

  // Scanning and compilation generally use the same flags.
  scanBuild.Variables["FLAGS"] = vars["FLAGS"];

  if (compilePP && !compilePPWithDefines) {
    // Move preprocessor definitions to the scan/preprocessor build statement.
    std::swap(scanBuild.Variables["DEFINES"], vars["DEFINES"]);
  } else {
    // Copy preprocessor definitions to the scan/preprocessor build statement.
    scanBuild.Variables["DEFINES"] = vars["DEFINES"];
  }

  // Copy include directories to the preprocessor build statement.  The
  // Fortran compilation build statement still needs them for the INCLUDE
  // directive.
  scanBuild.Variables["INCLUDES"] = vars["INCLUDES"];

  // Tell dependency scanner the object file that will result from
  // compiling the source.
  scanBuild.Variables["OBJ_FILE"] = objectFileName;

  // Tell dependency scanner where to store dyndep intermediate results.
  std::string const& ddiFile = cmStrCat(objectFileName, ".ddi");
  scanBuild.Variables["DYNDEP_INTERMEDIATE_FILE"] = ddiFile;

  // Outputs of the scan/preprocessor build statement.
  if (compilePP) {
    scanBuild.Outputs.push_back(ppFileName);
    scanBuild.ImplicitOuts.push_back(ddiFile);
  } else {
    scanBuild.Outputs.push_back(ddiFile);
    scanBuild.Variables["PREPROCESSED_OUTPUT_FILE"] = ppFileName;
    if (!compilationPreprocesses) {
      // Compilation does not preprocess and we are not compiling an
      // already-preprocessed source.  Make compilation depend on the scan
      // results to honor implicit dependencies discovered during scanning
      // (such as Fortran INCLUDE directives).
      objBuild.ImplicitDeps.emplace_back(ddiFile);
    }
  }

  // Scanning always provides a depfile for preprocessor dependencies. This
  // variable is unused in `msvc`-deptype scanners.
  std::string const& depFileName = cmStrCat(scanBuild.Outputs.front(), ".d");
  scanBuild.Variables["DEP_FILE"] =
    lg->ConvertToOutputFormat(depFileName, cmOutputConverter::SHELL);
  if (compilePP) {
    // The actual compilation does not need a depfile because it
    // depends on the already-preprocessed source.
    vars.erase("DEP_FILE");
  }

  return scanBuild;
}
}

void cmNinjaTargetGenerator::WriteObjectBuildStatement(
  cmSourceFile const* source, const std::string& config,
  const std::string& fileConfig, bool firstForConfig)
{
  std::string const language = source->GetLanguage();
  std::string const sourceFilePath = this->GetCompiledSourceNinjaPath(source);
  std::string const objectDir = this->ConvertToNinjaPath(
    cmStrCat(this->GeneratorTarget->GetSupportDirectory(),
             this->GetGlobalGenerator()->ConfigDirectory(config)));
  std::string const objectFileName =
    this->ConvertToNinjaPath(this->GetObjectFilePath(source, config));
  std::string const objectFileDir =
    cmSystemTools::GetFilenamePath(objectFileName);

  std::string cmakeVarLang = cmStrCat("CMAKE_", language);

  // build response file name
  std::string cmakeLinkVar = cmStrCat(cmakeVarLang, "_RESPONSE_FILE_FLAG");

  cmValue flag = this->GetMakefile()->GetDefinition(cmakeLinkVar);

  bool const lang_supports_response =
    !(language == "RC" || (language == "CUDA" && !flag));
  int const commandLineLengthLimit =
    ((lang_supports_response && this->ForceResponseFile())) ? -1 : 0;
  bool const needDyndep =
    this->GeneratorTarget->NeedDyndepForSource(language, config, source);

  cmNinjaBuild objBuild(this->LanguageCompilerRule(
    language, config, needDyndep ? WithScanning::Yes : WithScanning::No));
  cmNinjaVars& vars = objBuild.Variables;
  vars["FLAGS"] = this->ComputeFlagsForObject(source, language, config);
  vars["DEFINES"] = this->ComputeDefines(source, language, config);
  vars["INCLUDES"] = this->ComputeIncludes(source, language, config);

  auto compilerLauncher = this->GetCompilerLauncher(language, config);

  cmValue const skipCodeCheck = source->GetProperty("SKIP_LINTING");
  if (!skipCodeCheck.IsOn()) {
    auto const cmakeCmd = this->GetLocalGenerator()->ConvertToOutputFormat(
      cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
    vars["CODE_CHECK"] =
      this->GenerateCodeCheckRules(*source, compilerLauncher, cmakeCmd, config,
                                   [this](const std::string& path) {
                                     return this->ConvertToNinjaPath(path);
                                   });
  }

  // If compiler launcher was specified and not consumed above, it
  // goes to the beginning of the command line.
  if (!compilerLauncher.empty()) {
    cmList args{ compilerLauncher, cmList::EmptyElements::Yes };
    if (!args.empty()) {
      args[0] = this->LocalGenerator->ConvertToOutputFormat(
        args[0], cmOutputConverter::SHELL);
      for (std::string& i : cmMakeRange(args.begin() + 1, args.end())) {
        i = this->LocalGenerator->EscapeForShell(i);
      }
      vars["LAUNCHER"] = args.join(" ") + " ";
    }
  }

  if (this->GetMakefile()->GetSafeDefinition(
        cmStrCat("CMAKE_", language, "_DEPFILE_FORMAT")) != "msvc"_s) {
    bool replaceExt(false);
    if (!language.empty()) {
      std::string repVar =
        cmStrCat("CMAKE_", language, "_DEPFILE_EXTENSION_REPLACE");
      replaceExt = this->Makefile->IsOn(repVar);
    }
    if (!replaceExt) {
      // use original code
      vars["DEP_FILE"] = this->GetLocalGenerator()->ConvertToOutputFormat(
        cmStrCat(objectFileName, ".d"), cmOutputConverter::SHELL);
    } else {
      // Replace the original source file extension with the
      // depend file extension.
      std::string dependFileName = cmStrCat(
        cmSystemTools::GetFilenameWithoutLastExtension(objectFileName), ".d");
      vars["DEP_FILE"] = this->GetLocalGenerator()->ConvertToOutputFormat(
        cmStrCat(objectFileDir, '/', dependFileName),
        cmOutputConverter::SHELL);
    }
  }

  if (firstForConfig) {
    this->ExportObjectCompileCommand(
      language, sourceFilePath, objectDir, objectFileName, objectFileDir,
      vars["FLAGS"], vars["DEFINES"], vars["INCLUDES"], config);
  }

  objBuild.Outputs.push_back(objectFileName);
  if (firstForConfig) {
    cmValue pchExtension =
      this->GetMakefile()->GetDefinition("CMAKE_PCH_EXTENSION");
    if (!cmHasSuffix(objectFileName, pchExtension)) {
      // Add this object to the list of object files.
      this->Configs[config].Objects.push_back(objectFileName);
    }
  }

  objBuild.ExplicitDeps.push_back(sourceFilePath);

  // Add precompile headers dependencies
  std::vector<std::string> depList;

  std::vector<std::string> architectures =
    this->GeneratorTarget->GetAppleArchs(config, language);
  if (architectures.empty()) {
    architectures.emplace_back();
  }

  std::unordered_set<std::string> pchSources;
  for (const std::string& arch : architectures) {
    const std::string pchSource =
      this->GeneratorTarget->GetPchSource(config, language, arch);

    if (!pchSource.empty()) {
      pchSources.insert(pchSource);
    }
  }

  if (!pchSources.empty() && !source->GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    for (const std::string& arch : architectures) {
      depList.push_back(
        this->GeneratorTarget->GetPchHeader(config, language, arch));
      if (pchSources.find(source->GetFullPath()) == pchSources.end()) {
        depList.push_back(
          this->GeneratorTarget->GetPchFile(config, language, arch));
      }
    }
  }

  if (cmValue objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    cmList objDepList{ *objectDeps };
    std::copy(objDepList.begin(), objDepList.end(),
              std::back_inserter(depList));
  }

  if (!depList.empty()) {
    for (std::string& odi : depList) {
      if (cmSystemTools::FileIsFullPath(odi)) {
        odi = cmSystemTools::CollapseFullPath(odi);
      }
    }
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(objBuild.ImplicitDeps),
                   this->MapToNinjaPath());
  }

  objBuild.OrderOnlyDeps.push_back(this->OrderDependsTargetForTarget(config));

  // If the source file is GENERATED and does not have a custom command
  // (either attached to this source file or another one), assume that one of
  // the target dependencies, OBJECT_DEPENDS or header file custom commands
  // will rebuild the file.
  if (source->GetIsGenerated() &&
      !source->GetPropertyAsBool("__CMAKE_GENERATED_BY_CMAKE") &&
      !source->GetCustomCommand() &&
      !this->GetGlobalGenerator()->HasCustomCommandOutput(sourceFilePath)) {
    this->GetGlobalGenerator()->AddAssumedSourceDependencies(
      sourceFilePath, objBuild.OrderOnlyDeps);
  }

  // For some cases we scan to dynamically discover dependencies.
  bool const compilationPreprocesses =
    !this->NeedExplicitPreprocessing(language);

  std::string modmapFormat;
  if (needDyndep) {
    std::string const modmapFormatVar =
      cmStrCat("CMAKE_EXPERIMENTAL_", language, "_MODULE_MAP_FORMAT");
    modmapFormat = this->Makefile->GetSafeDefinition(modmapFormatVar);
  }

  if (needDyndep) {
    // If source/target has preprocessing turned off, we still need to
    // generate an explicit dependency step
    const auto srcpp = source->GetSafeProperty("Fortran_PREPROCESS");
    cmOutputConverter::FortranPreprocess preprocess =
      cmOutputConverter::GetFortranPreprocess(srcpp);
    if (preprocess == cmOutputConverter::FortranPreprocess::Unset) {
      const auto& tgtpp =
        this->GeneratorTarget->GetSafeProperty("Fortran_PREPROCESS");
      preprocess = cmOutputConverter::GetFortranPreprocess(tgtpp);
    }

    bool const compilePP = !compilationPreprocesses &&
      (preprocess != cmOutputConverter::FortranPreprocess::NotNeeded);
    bool const compilePPWithDefines =
      compilePP && this->CompileWithDefines(language);

    std::string scanRuleName;
    std::string ppFileName;
    if (compilePP) {
      scanRuleName = this->LanguagePreprocessAndScanRule(language, config);
      ppFileName = this->ConvertToNinjaPath(
        this->GetPreprocessedFilePath(source, config));
    } else {
      scanRuleName = this->LanguageScanRule(language, config);
      ppFileName = cmStrCat(objectFileName, ".ddi.i");
    }

    cmNinjaBuild ppBuild = GetScanBuildStatement(
      scanRuleName, ppFileName, compilePP, compilePPWithDefines,
      compilationPreprocesses, objBuild, vars, objectFileName,
      this->LocalGenerator);

    if (compilePP) {
      // In case compilation requires flags that are incompatible with
      // preprocessing, include them here.
      std::string const& postFlag = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_", language, "_POSTPROCESS_FLAG"));
      this->LocalGenerator->AppendFlags(vars["FLAGS"], postFlag);

      // Prepend source file's original directory as an include directory
      // so e.g. Fortran INCLUDE statements can look for files in it.
      std::vector<std::string> sourceDirectory;
      sourceDirectory.push_back(
        cmSystemTools::GetParentDirectory(source->GetFullPath()));

      std::string sourceDirectoryFlag = this->LocalGenerator->GetIncludeFlags(
        sourceDirectory, this->GeneratorTarget, language, config, false);

      vars["INCLUDES"] = cmStrCat(sourceDirectoryFlag, ' ', vars["INCLUDES"]);
    }

    ScanningFiles scanningFiles;

    if (firstForConfig) {
      scanningFiles.ScanningOutput = cmStrCat(objectFileName, ".ddi");
    }

    this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetGeneratorTarget(),
                               ppBuild.Variables);

    this->GetGlobalGenerator()->WriteBuild(this->GetImplFileStream(fileConfig),
                                           ppBuild, commandLineLengthLimit);

    std::string const dyndep = this->GetDyndepFilePath(language, config);
    objBuild.OrderOnlyDeps.push_back(dyndep);
    vars["dyndep"] = dyndep;

    if (!modmapFormat.empty()) {
      // XXX(modmap): If changing this path construction, change
      // `cmGlobalNinjaGenerator::WriteDyndep` to expect the corresponding file
      // path.
      std::string ddModmapFile = cmStrCat(objectFileName, ".modmap");
      vars["DYNDEP_MODULE_MAP_FILE"] = ddModmapFile;
      objBuild.OrderOnlyDeps.push_back(ddModmapFile);
      scanningFiles.ModuleMapFile = std::move(ddModmapFile);
    }

    if (!scanningFiles.IsEmpty()) {
      this->Configs[config].ScanningInfo[language].emplace_back(scanningFiles);
    }
  }

  this->EnsureParentDirectoryExists(objectFileName);

  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectDir, cmOutputConverter::SHELL);
  vars["OBJECT_FILE_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectFileDir, cmOutputConverter::SHELL);

  this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetGeneratorTarget(),
                             vars);

  if (!pchSources.empty() && !source->GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    auto pchIt = pchSources.find(source->GetFullPath());
    if (pchIt != pchSources.end()) {
      this->addPoolNinjaVariable("JOB_POOL_PRECOMPILE_HEADER",
                                 this->GetGeneratorTarget(), vars);
    }
  }

  this->SetMsvcTargetPdbVariable(vars, config);

  objBuild.RspFile = cmStrCat(objectFileName, ".rsp");

  if (language == "ISPC") {
    std::string const& objectName =
      this->GeneratorTarget->GetObjectName(source);
    std::string ispcSource =
      cmSystemTools::GetFilenameWithoutLastExtension(objectName);
    ispcSource = cmSystemTools::GetFilenameWithoutLastExtension(ispcSource);

    cmValue ispcSuffixProp =
      this->GeneratorTarget->GetProperty("ISPC_HEADER_SUFFIX");
    assert(ispcSuffixProp);

    std::string ispcHeaderDirectory =
      this->GeneratorTarget->GetObjectDirectory(config);
    if (cmValue prop =
          this->GeneratorTarget->GetProperty("ISPC_HEADER_DIRECTORY")) {
      ispcHeaderDirectory =
        cmStrCat(this->LocalGenerator->GetBinaryDirectory(), '/', *prop);
    }

    std::string ispcHeader =
      cmStrCat(ispcHeaderDirectory, '/', ispcSource, *ispcSuffixProp);
    ispcHeader = this->ConvertToNinjaPath(ispcHeader);

    // Make sure ninja knows what command generates the header
    objBuild.ImplicitOuts.push_back(ispcHeader);

    // Make sure ninja knows how to clean the generated header
    this->GetGlobalGenerator()->AddAdditionalCleanFile(ispcHeader, config);

    auto ispcSuffixes =
      detail::ComputeISPCObjectSuffixes(this->GeneratorTarget);
    if (ispcSuffixes.size() > 1) {
      std::string rootObjectDir =
        this->GeneratorTarget->GetObjectDirectory(config);
      auto ispcSideEfffectObjects = detail::ComputeISPCExtraObjects(
        objectName, rootObjectDir, ispcSuffixes);

      for (auto sideEffect : ispcSideEfffectObjects) {
        sideEffect = this->ConvertToNinjaPath(sideEffect);
        objBuild.ImplicitOuts.emplace_back(sideEffect);
        this->GetGlobalGenerator()->AddAdditionalCleanFile(sideEffect, config);
      }
    }

    vars["ISPC_HEADER_FILE"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        ispcHeader, cmOutputConverter::SHELL);
  } else {
    auto headers = this->GeneratorTarget->GetGeneratedISPCHeaders(config);
    if (!headers.empty()) {
      std::transform(headers.begin(), headers.end(), headers.begin(),
                     this->MapToNinjaPath());
      objBuild.OrderOnlyDeps.insert(objBuild.OrderOnlyDeps.end(),
                                    headers.begin(), headers.end());
    }
  }

  if (language == "Swift") {
    this->EmitSwiftDependencyInfo(source, config);
  } else {
    this->GetGlobalGenerator()->WriteBuild(this->GetImplFileStream(fileConfig),
                                           objBuild, commandLineLengthLimit);
  }

  if (cmValue objectOutputs = source->GetProperty("OBJECT_OUTPUTS")) {
    std::string evaluatedObjectOutputs = cmGeneratorExpression::Evaluate(
      *objectOutputs, this->LocalGenerator, config);

    if (!evaluatedObjectOutputs.empty()) {
      cmNinjaBuild build("phony");
      build.Comment = "Additional output files.";
      build.Outputs = cmList{ evaluatedObjectOutputs }.data();
      std::transform(build.Outputs.begin(), build.Outputs.end(),
                     build.Outputs.begin(), this->MapToNinjaPath());
      build.ExplicitDeps = objBuild.Outputs;
      this->GetGlobalGenerator()->WriteBuild(
        this->GetImplFileStream(fileConfig), build);
    }
  }
}

void cmNinjaTargetGenerator::WriteTargetDependInfo(std::string const& lang,
                                                   const std::string& config)
{
  Json::Value tdi(Json::objectValue);
  tdi["language"] = lang;
  tdi["compiler-id"] = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILER_ID"));
  tdi["compiler-simulate-id"] = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_SIMULATE_ID"));
  tdi["compiler-frontend-variant"] = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILER_FRONTEND_VARIANT"));

  std::string mod_dir;
  if (lang == "Fortran") {
    mod_dir = this->GeneratorTarget->GetFortranModuleDirectory(
      this->Makefile->GetHomeOutputDirectory());
  } else if (lang == "CXX") {
    mod_dir = this->GetGlobalGenerator()->ExpandCFGIntDir(
      cmSystemTools::CollapseFullPath(this->GeneratorTarget->ObjectDirectory),
      config);
  }
  if (mod_dir.empty()) {
    mod_dir = this->Makefile->GetCurrentBinaryDirectory();
  }
  tdi["module-dir"] = mod_dir;

  if (lang == "Fortran") {
    tdi["submodule-sep"] =
      this->Makefile->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_SEP");
    tdi["submodule-ext"] =
      this->Makefile->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_EXT");
  } else if (lang == "CXX") {
    // No extra information necessary.
  }

  tdi["dir-cur-bld"] = this->Makefile->GetCurrentBinaryDirectory();
  tdi["dir-cur-src"] = this->Makefile->GetCurrentSourceDirectory();
  tdi["dir-top-bld"] = this->Makefile->GetHomeOutputDirectory();
  tdi["dir-top-src"] = this->Makefile->GetHomeDirectory();

  Json::Value& tdi_include_dirs = tdi["include-dirs"] = Json::arrayValue;
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              lang, config);
  for (std::string const& i : includes) {
    // Convert the include directories the same way we do for -I flags.
    // See upstream ninja issue 1251.
    tdi_include_dirs.append(this->ConvertToNinjaPath(i));
  }

  Json::Value& tdi_linked_target_dirs = tdi["linked-target-dirs"] =
    Json::arrayValue;
  for (std::string const& l : this->GetLinkedTargetDirectories(lang, config)) {
    tdi_linked_target_dirs.append(l);
  }

  cmDyndepGeneratorCallbacks cb;
  cb.ObjectFilePath = [this](cmSourceFile const* sf, std::string const& cnf) {
    return this->GetObjectFilePath(sf, cnf);
  };

#if !defined(CMAKE_BOOTSTRAP)
  cmDyndepCollation::AddCollationInformation(tdi, this->GeneratorTarget,
                                             config, cb);
#endif

  std::string const tdin = this->GetTargetDependInfoPath(lang, config);
  cmGeneratedFileStream tdif(tdin);
  tdif << tdi;
}

void cmNinjaTargetGenerator::EmitSwiftDependencyInfo(
  cmSourceFile const* source, const std::string& config)
{
  std::string const sourceFilePath = this->GetCompiledSourceNinjaPath(source);
  std::string const objectFilePath =
    this->ConvertToNinjaPath(this->GetObjectFilePath(source, config));
  std::string const swiftDepsPath = [source, objectFilePath]() -> std::string {
    if (cmValue name = source->GetProperty("Swift_DEPENDENCIES_FILE")) {
      return *name;
    }
    return cmStrCat(objectFilePath, ".swiftdeps");
  }();
  std::string const swiftDiaPath = [source, objectFilePath]() -> std::string {
    if (cmValue name = source->GetProperty("Swift_DIAGNOSTICS_FILE")) {
      return *name;
    }
    return cmStrCat(objectFilePath, ".dia");
  }();
  std::string const makeDepsPath = [this, source, config]() -> std::string {
    cmLocalNinjaGenerator const* local = this->GetLocalGenerator();
    std::string const objectFileName =
      this->ConvertToNinjaPath(this->GetObjectFilePath(source, config));
    std::string const objectFileDir =
      cmSystemTools::GetFilenamePath(objectFileName);

    if (this->Makefile->IsOn("CMAKE_Swift_DEPFLE_EXTNSION_REPLACE")) {
      std::string dependFileName = cmStrCat(
        cmSystemTools::GetFilenameWithoutLastExtension(objectFileName), ".d");
      return local->ConvertToOutputFormat(
        cmStrCat(objectFileDir, '/', dependFileName),
        cmOutputConverter::SHELL);
    }
    return local->ConvertToOutputFormat(cmStrCat(objectFileName, ".d"),
                                        cmOutputConverter::SHELL);
  }();

  // build the source file mapping
  // https://github.com/apple/swift/blob/master/docs/Driver.md#output-file-maps
  Json::Value entry = Json::Value(Json::objectValue);
  entry["object"] = objectFilePath;
  entry["dependencies"] = makeDepsPath;
  entry["swift-dependencies"] = swiftDepsPath;
  entry["diagnostics"] = swiftDiaPath;
  this->Configs[config].SwiftOutputMap[sourceFilePath] = entry;
}

void cmNinjaTargetGenerator::ExportObjectCompileCommand(
  std::string const& language, std::string const& sourceFileName,
  std::string const& objectDir, std::string const& objectFileName,
  std::string const& objectFileDir, std::string const& flags,
  std::string const& defines, std::string const& includes,
  std::string const& outputConfig)
{
  if (!this->GeneratorTarget->GetPropertyAsBool("EXPORT_COMPILE_COMMANDS")) {
    return;
  }

  cmRulePlaceholderExpander::RuleVariables compileObjectVars;
  compileObjectVars.Language = language.c_str();

  std::string escapedSourceFileName = sourceFileName;

  if (!cmSystemTools::FileIsFullPath(sourceFileName)) {
    escapedSourceFileName =
      cmSystemTools::CollapseFullPath(escapedSourceFileName,
                                      this->GetGlobalGenerator()
                                        ->GetCMakeInstance()
                                        ->GetHomeOutputDirectory());
  }

  escapedSourceFileName = this->LocalGenerator->ConvertToOutputFormat(
    escapedSourceFileName, cmOutputConverter::SHELL);

  compileObjectVars.Source = escapedSourceFileName.c_str();
  compileObjectVars.Object = objectFileName.c_str();
  compileObjectVars.ObjectDir = objectDir.c_str();
  compileObjectVars.ObjectFileDir = objectFileDir.c_str();
  compileObjectVars.Flags = flags.c_str();
  compileObjectVars.Defines = defines.c_str();
  compileObjectVars.Includes = includes.c_str();

  // Rule for compiling object file.
  std::string cudaCompileMode;
  if (language == "CUDA") {
    if (this->GeneratorTarget->GetPropertyAsBool(
          "CUDA_SEPARABLE_COMPILATION")) {
      const std::string& rdcFlag =
        this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_RDC_FLAG");
      cudaCompileMode = cmStrCat(cudaCompileMode, rdcFlag, " ");
    }
    static std::array<cm::string_view, 4> const compileModes{
      { "PTX"_s, "CUBIN"_s, "FATBIN"_s, "OPTIX"_s }
    };
    bool useNormalCompileMode = true;
    for (cm::string_view mode : compileModes) {
      auto propName = cmStrCat("CUDA_", mode, "_COMPILATION");
      auto defName = cmStrCat("_CMAKE_CUDA_", mode, "_FLAG");
      if (this->GeneratorTarget->GetPropertyAsBool(propName)) {
        const std::string& flag =
          this->Makefile->GetRequiredDefinition(defName);
        cudaCompileMode = cmStrCat(cudaCompileMode, flag);
        useNormalCompileMode = false;
        break;
      }
    }
    if (useNormalCompileMode) {
      const std::string& wholeFlag =
        this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_WHOLE_FLAG");
      cudaCompileMode = cmStrCat(cudaCompileMode, wholeFlag);
    }
    compileObjectVars.CudaCompileMode = cudaCompileMode.c_str();
  }

  const std::string cmdVar = cmStrCat("CMAKE_", language, "_COMPILE_OBJECT");
  const std::string& compileCmd =
    this->Makefile->GetRequiredDefinition(cmdVar);
  cmList compileCmds(compileCmd);

  auto rulePlaceholderExpander =
    this->GetLocalGenerator()->CreateRulePlaceholderExpander();

  for (auto& i : compileCmds) {
    // no launcher for CMAKE_EXPORT_COMPILE_COMMANDS
    rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(), i,
                                                 compileObjectVars);
  }

  std::string cmdLine = this->GetLocalGenerator()->BuildCommandLine(
    compileCmds, outputConfig, outputConfig);

  this->GetGlobalGenerator()->AddCXXCompileCommand(cmdLine, sourceFileName,
                                                   objectFileName);
}

void cmNinjaTargetGenerator::AdditionalCleanFiles(const std::string& config)
{
  if (cmValue prop_value =
        this->GeneratorTarget->GetProperty("ADDITIONAL_CLEAN_FILES")) {
    cmLocalNinjaGenerator* lg = this->LocalGenerator;
    cmList cleanFiles(cmGeneratorExpression::Evaluate(*prop_value, lg, config,
                                                      this->GeneratorTarget));
    std::string const& binaryDir = lg->GetCurrentBinaryDirectory();
    cmGlobalNinjaGenerator* gg = lg->GetGlobalNinjaGenerator();
    for (auto const& cleanFile : cleanFiles) {
      // Support relative paths
      gg->AddAdditionalCleanFile(
        cmSystemTools::CollapseFullPath(cleanFile, binaryDir), config);
    }
  }
}

cmNinjaDeps cmNinjaTargetGenerator::GetObjects(const std::string& config) const
{
  auto const it = this->Configs.find(config);
  if (it != this->Configs.end()) {
    return it->second.Objects;
  }
  return {};
}

void cmNinjaTargetGenerator::EnsureDirectoryExists(
  const std::string& path) const
{
  if (cmSystemTools::FileIsFullPath(path)) {
    cmSystemTools::MakeDirectory(path);
  } else {
    cmGlobalNinjaGenerator* gg = this->GetGlobalGenerator();
    std::string fullPath = gg->GetCMakeInstance()->GetHomeOutputDirectory();
    // Also ensures there is a trailing slash.
    gg->StripNinjaOutputPathPrefixAsSuffix(fullPath);
    fullPath += path;
    cmSystemTools::MakeDirectory(fullPath);
  }
}

void cmNinjaTargetGenerator::EnsureParentDirectoryExists(
  const std::string& path) const
{
  this->EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path));
}

void cmNinjaTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile const& source, const char* pkgloc, const std::string& config)
{
  // Skip OS X content when not building a Framework or Bundle.
  if (!this->Generator->GetGeneratorTarget()->IsBundleOnApple()) {
    return;
  }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc,
                                                                    config);

  // Reject files that collide with files from the Ninja file's native config.
  if (config != this->FileConfig) {
    std::string nativeMacdir =
      this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(
        pkgloc, this->FileConfig);
    if (macdir == nativeMacdir) {
      return;
    }
  }

  // Get the input file location.
  std::string input = source.GetFullPath();
  input = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(input);

  // Get the output file location.
  std::string output =
    cmStrCat(macdir, '/', cmSystemTools::GetFilenameName(input));
  output = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(output);

  // Write a build statement to copy the content into the bundle.
  this->Generator->GetGlobalGenerator()->WriteMacOSXContentBuild(
    input, output, this->FileConfig);

  // Add as a dependency to the target so that it gets called.
  this->Generator->Configs[config].ExtraFiles.push_back(std::move(output));
}

void cmNinjaTargetGenerator::addPoolNinjaVariable(
  const std::string& pool_property, cmGeneratorTarget* target,
  cmNinjaVars& vars)
{
  cmValue pool = target->GetProperty(pool_property);
  if (pool) {
    vars["pool"] = *pool;
  }
}

bool cmNinjaTargetGenerator::ForceResponseFile()
{
  static std::string const forceRspFile = "CMAKE_NINJA_FORCE_RESPONSE_FILE";
  return (this->GetMakefile()->IsDefinitionSet(forceRspFile) ||
          cmSystemTools::HasEnv(forceRspFile));
}
