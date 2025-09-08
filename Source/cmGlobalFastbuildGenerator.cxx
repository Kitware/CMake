/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmGlobalFastbuildGenerator.h"

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <queue>
#include <sstream>

#include <cm/memory>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmFastbuildLinkLineComputer.h"
#include "cmFastbuildTargetGenerator.h" // IWYU pragma: keep
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobCacheEntry.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmList.h"
#include "cmLocalFastbuildGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

#if defined(_WIN32)
#  include <future>

#  include <objbase.h>
#  include <shellapi.h>

#endif

class cmLinkLineComputer;

#define FASTBUILD_REBUILD_BFF_TARGET_NAME "rebuild-bff"
#define FASTBUILD_GLOB_CHECK_TARGET "glob-check"
#define FASTBUILD_ENV_VAR_NAME "LocalEnv"

// IDE support
#define FASTBUILD_XCODE_BASE_PATH "XCode/Projects"
#define FASTBUILD_VS_BASE_PATH "VisualStudio/Projects"

#define FASTBUILD_IDE_VS_COMMAND_PREFIX "cd ^$(SolutionDir).. && "
#define FASTBUILD_IDE_BUILD_ARGS " -ide -cache -summary -dist "

constexpr auto FASTBUILD_CAPTURE_SYSTEM_ENV =
  "CMAKE_FASTBUILD_CAPTURE_SYSTEM_ENV";
constexpr auto FASTBUILD_ENV_OVERRIDES = "CMAKE_FASTBUILD_ENV_OVERRIDES";

// Inherits from "CMAKE_FASTBUILD_VERBOSE_GENERATOR" env variable.
constexpr auto FASTBUILD_VERBOSE_GENERATOR =
  "CMAKE_FASTBUILD_VERBOSE_GENERATOR";
constexpr auto FASTBUILD_CACHE_PATH = "CMAKE_FASTBUILD_CACHE_PATH";
// Compiler settings.
constexpr auto FASTBUILD_COMPILER_EXTRA_FILES =
  "CMAKE_FASTBUILD_COMPILER_EXTRA_FILES";
constexpr auto FASTBUILD_USE_LIGHTCACHE = "CMAKE_FASTBUILD_USE_LIGHTCACHE";

template <class T>
FastbuildAliasNode generateAlias(std::string const& name, char const* postfix,
                                 T const& nodes)
{
  FastbuildAliasNode alias;
  alias.Name = name + postfix;
  for (auto const& node : nodes) {
    alias.PreBuildDependencies.emplace(node.Name);
  }
  return alias;
}

void FastbuildTarget::GenerateAliases()
{
  // -deps
  this->DependenciesAlias.Name =
    this->Name + FASTBUILD_DEPS_ARTIFACTS_ALIAS_POSTFIX;
  for (auto const& dep : this->PreBuildDependencies) {
    if (dep.Type != FastbuildTargetDepType::ORDER_ONLY) {
      this->DependenciesAlias.PreBuildDependencies.emplace(dep);
    }
  }
  // Build.
  if (!this->LinkerNode.empty() &&
      this->LinkerNode[0].Type != FastbuildLinkerNode::NONE) {
    this->BuildAlias = generateAlias(this->Name, FASTBUILD_BUILD_ALIAS_POSTFIX,
                                     this->LinkerNode);
  }

  // PRE/POST/REST
  if (!this->PreBuildExecNodes.PreBuildDependencies.empty()) {
    this->PreBuildExecNodes.Name =
      this->Name + FASTBUILD_PRE_BUILD_ALIAS_POSTFIX;
  }
  if (!this->PreLinkExecNodes.Nodes.empty()) {
    this->PreLinkExecNodes.Alias =
      generateAlias(this->Name, FASTBUILD_PRE_LINK_ALIAS_POSTFIX,
                    this->PreLinkExecNodes.Nodes);
  }
  if (!this->PostBuildExecNodes.Alias.PreBuildDependencies.empty()) {
    this->PostBuildExecNodes.Alias.Name =
      this->Name + FASTBUILD_POST_BUILD_ALIAS_POSTFIX;
  }
  if (!this->ExecNodes.PreBuildDependencies.empty()) {
    this->ExecNodes.Name = this->Name + FASTBUILD_CUSTOM_COMMAND_ALIAS_POSTFIX;
  }

  // -all.
  FastbuildAliasNode allAlias;
  allAlias.Name = this->Name + FASTBUILD_ALL_ALIAS_POSTFIX;

  auto const addToAll = [&allAlias](FastbuildAliasNode const& node) {
    if (!node.PreBuildDependencies.empty()) {
      allAlias.PreBuildDependencies.emplace(node.Name);
    }
  };
  // Add "-pre" nodes first, so we wait for all generations when building
  // "-all".
  for (FastbuildAliasNode const& execs : {
         PreBuildExecNodes,
         PreLinkExecNodes.Alias,
       }) {
    addToAll(execs);
  }

  // -build.
  addToAll(this->BuildAlias);
  for (FastbuildAliasNode const& node : AliasNodes) {
    addToAll(node);
  }

  for (FastbuildAliasNode const& execs : { ExecNodes }) {
    addToAll(execs);
  }

  bool const hasStampExeProperty = !this->LinkerNode.empty() &&
    (this->LinkerNode[0].Type == FastbuildLinkerNode::EXECUTABLE ||
     this->LinkerNode[0].Type == FastbuildLinkerNode::SHARED_LIBRARY);
  // If we have .LinkerStampExe - we will use it to execute POST_BUILD steps,
  // so don't add them to "-all".
  if (!hasStampExeProperty) {
    addToAll(this->PostBuildExecNodes.Alias);
  }

  // Tested in "RunCMake.VerifyHeaderSets" test.
  if (allAlias.PreBuildDependencies.empty()) {
    addToAll(DependenciesAlias);
  }

  for (auto const& objectList : this->ObjectListNodes) {
    allAlias.PreBuildDependencies.emplace(objectList.Name);
  }

  for (auto const& linkerNode : this->LinkerNode) {
    allAlias.PreBuildDependencies.emplace(linkerNode.Name);
  }

  // Absolutely empty target. But we still should be able to "build" it.
  if (allAlias.PreBuildDependencies.empty()) {
    allAlias.PreBuildDependencies.emplace(FASTBUILD_NOOP_FILE_NAME);
  }

  AliasNodes.emplace_back(std::move(allAlias));
  // In case we want to build the target by name, but we don't have any output.
  // OR we have two arches and real output does not match target name (it can
  // match if lipo produces such file).
  // RunCMake.CMakePackage / RunCMake.XcFramework
  if ((this->LinkerNode.size() > 1 && this->RealOutput != this->Name) ||
      this->RealOutput.empty()) {
    FastbuildAliasNode alias;
    alias.Name = this->Name;
    FastbuildTargetDep dep{ this->Name };
    dep.Type = FastbuildTargetDepType::ALL;
    alias.PreBuildDependencies = { std::move(dep) };
    AliasNodes.emplace_back(std::move(alias));
  }

  // Link artifacts (should not be added to all
  // since on Windows it might contain Import Lib and FASTBuild doesn't know
  // how to create it, so "-all" will fail).
  AliasNodes.emplace_back(generateAlias(
    this->Name, FASTBUILD_OBJECTS_ALIAS_POSTFIX, this->ObjectListNodes));

  for (auto const& linkerNode : this->LinkerNode) {
    if (linkerNode.Type == FastbuildLinkerNode::SHARED_LIBRARY ||
        linkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY ||
        linkerNode.Type == FastbuildLinkerNode::EXECUTABLE) {
      std::string name = FASTBUILD_LINK_ARTIFACTS_ALIAS_POSTFIX;
      if (!linkerNode.Arch.empty()) {
        name += cmStrCat('-', linkerNode.Arch);
      }
#ifdef _WIN32
      // On Windows DLL and Executables must be linked via Import Lib file
      // (.lib).
      if (linkerNode.Type == FastbuildLinkerNode::SHARED_LIBRARY ||
          linkerNode.Type == FastbuildLinkerNode::EXECUTABLE) {
        FastbuildAliasNode linkAlias;
        linkAlias.Name = this->Name + FASTBUILD_LINK_ARTIFACTS_ALIAS_POSTFIX;
        linkAlias.PreBuildDependencies.emplace(
          FASTBUILD_DOLLAR_TAG "TargetOutputImplib" FASTBUILD_DOLLAR_TAG);
        AliasNodes.emplace_back(std::move(linkAlias));
        continue;
      }
#endif
      AliasNodes.emplace_back(
        generateAlias(this->Name, name.c_str(),
                      std::vector<FastbuildLinkerNode>{ linkerNode }));
    }
  }
}

cmGlobalFastbuildGenerator::cmGlobalFastbuildGenerator(cmake* cm)
  : cmGlobalCommonGenerator(cm)
  , BuildFileStream(nullptr)
{
#ifdef _WIN32
  cm->GetState()->SetWindowsShell(true);
#endif
  this->FindMakeProgramFile = "CMakeFastbuildFindMake.cmake";
  cm->GetState()->SetFastbuildMake(true);
  cm->GetState()->SetIsGeneratorMultiConfig(false);
}

void cmGlobalFastbuildGenerator::ProcessEnvironment()
{
  bool const CaptureSystemEnv =
    !this->GetGlobalSetting(FASTBUILD_CAPTURE_SYSTEM_ENV).IsSet() ||
    this->GetGlobalSetting(FASTBUILD_CAPTURE_SYSTEM_ENV).IsOn();
  // On Windows environment is needed for MSVC, but preserve ability to discard
  // it from the generated file if requested.
  if (CaptureSystemEnv) {
    LocalEnvironment = cmSystemTools::GetEnvironmentVariables();
  }
  // FASTBuild strips off "-isysroot" command line option (see :
  // https://github.com/fastbuild/fastbuild/issues/1066).
  // If 'SDK_ROOT' is not set via env and '-isysroot' is absent, AppleClang
  // seems to use MacOS SDK by default (even though FBuild flattens includes
  // before compiling). It breaks cross-compilation for iOS. Tested in
  // "RunCMake.Framework" test.
  std::string const osxRoot = this->GetSafeGlobalSetting("CMAKE_OSX_SYSROOT");
  if (!osxRoot.empty()) {
    LocalEnvironment.emplace_back("SDKROOT=" + osxRoot);
  }

  auto const EnvOverrides =
    this->GetSafeGlobalSetting(FASTBUILD_ENV_OVERRIDES);

  if (!EnvOverrides.empty()) {
    auto const overrideEnvVar = [this](std::string const& prefix,
                                       std::string val) {
      auto const iter =
        std::find_if(LocalEnvironment.begin(), LocalEnvironment.end(),
                     [&prefix](std::string const& value) {
                       return cmSystemTools::StringStartsWith(value.c_str(),
                                                              prefix.c_str());
                     });
      if (iter != LocalEnvironment.end()) {
        *iter = std::move(val);
      } else {
        LocalEnvironment.emplace_back(std::move(val));
      }
    };
    for (auto& val : cmList{ EnvOverrides }) {
      auto const pos = val.find('=');
      if (pos != std::string::npos && ((pos + 1) < val.size())) {
        overrideEnvVar(val.substr(0, pos + 1), std::move(val));
      }
    }
  }

  // Empty strings are not allowed.
  LocalEnvironment.erase(
    std::remove_if(LocalEnvironment.begin(), LocalEnvironment.end(),
                   [](std::string const& s) { return s.empty(); }),
    LocalEnvironment.end());
}

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalFastbuildGenerator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(
    new cmGlobalGeneratorSimpleFactory<cmGlobalFastbuildGenerator>());
}

void cmGlobalFastbuildGenerator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
  for (std::string const& l : lang) {
    if (l == "NONE") {
      continue;
    }
    this->ResolveLanguageCompiler(l, mf, optional);
  }
}

bool cmGlobalFastbuildGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (!cmGlobalGenerator::FindMakeProgram(mf)) {
    return false;
  }
  if (auto fastbuildCommand = mf->GetDefinition("CMAKE_MAKE_PROGRAM")) {
    this->FastbuildCommand = *fastbuildCommand;
    std::vector<std::string> command;
    command.push_back(this->FastbuildCommand);
    command.emplace_back("-version");
    std::string version;
    std::string error;
    if (!cmSystemTools::RunSingleCommand(command, &version, &error, nullptr,
                                         nullptr,
                                         cmSystemTools::OUTPUT_NONE)) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       "Running\n '" + cmJoin(command, "' '") +
                         "'\n"
                         "failed with:\n " +
                         error);
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    cmsys::RegularExpression versionRegex(R"(^FASTBuild v([0-9]+\.[0-9]+))");
    versionRegex.find(version);
    this->FastbuildVersion = versionRegex.match(1);
  }
  return true;
}

std::unique_ptr<cmLocalGenerator>
cmGlobalFastbuildGenerator::CreateLocalGenerator(cmMakefile* makefile)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalFastbuildGenerator>(this, makefile));
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalFastbuildGenerator::GenerateBuildCommand(
  std::string const& makeProgram, std::string const& /*projectName*/,
  std::string const& projectDir, std::vector<std::string> const& targetNames,
  std::string const& /*config*/, int /*jobs*/, bool verbose,
  cmBuildOptions /*buildOptions*/, std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand;
  this->FastbuildCommand = this->SelectMakeProgram(makeProgram);
  makeCommand.Add(this->FastbuildCommand);
  // A build command for fastbuild looks like this:
  // fbuild.exe [make-options] [-config projectName.bff] <target>

  std::string configFile = cmStrCat(projectDir, '/', FASTBUILD_BUILD_FILE);

  // Push in the make options
  makeCommand.Add(makeOptions.begin(), makeOptions.end());

  if (!configFile.empty()) {
    makeCommand.Add("-config", configFile);
  }
  // Tested in "RunCMake.SymlinkTrees" test.
  makeCommand.Add("-continueafterdbmove");

  // Tested in RunCMake.LinkWhatYouUse on Linux. (We need to see output of
  // LinkerStampExe process).
  // In general, it might be useful to see output of external processes
  // regardless of their outcome.
  makeCommand.Add("-showcmdoutput");

  // Add the target-config to the command
  for (auto const& tname : targetNames) {
    if (!tname.empty()) {
      makeCommand.Add(tname);
    }
  }
  if (verbose) {
    makeCommand.Add("-verbose");
  }

  // Make "rebuild-bff" target up-to-date before running the build.

  std::string output;
  ExecuteFastbuildTarget(projectDir, FASTBUILD_REBUILD_BFF_TARGET_NAME, output,
                         { "-why" });

  // If fbuild.bff was re-generated we need to "restat" it.
  if (output.find("Need to build") != std::string::npos) {
    // Let the user know that re-generation happened (and why it
    // happened).
    cmSystemTools::Stdout(output);
    // FASTBuild will consider the target out-of-date in case some of the
    // inputs have changes after re-generation which might happen if, for
    // example, configuration depends on some files generated during
    // the configuration itself.
    AskCMakeToMakeRebuildBFFUpToDate(projectDir);
  }

  return { std::move(makeCommand) };
}

void cmGlobalFastbuildGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir =
    cmStrCat(gt->GetSupportDirectory(), '/', this->GetCMakeCFGIntDir(), '/');
  gt->ObjectDirectory = std::move(dir);
}

void cmGlobalFastbuildGenerator::AppendDirectoryForConfig(
  std::string const& prefix, std::string const& config,
  std::string const& suffix, std::string& dir)
{
  if (!config.empty() && this->IsMultiConfig()) {
    dir += cmStrCat(prefix, config, suffix);
  }
}

cmDocumentationEntry cmGlobalFastbuildGenerator::GetDocumentation()
{
  return { cmGlobalFastbuildGenerator::GetActualName(),
           "Generates build.bff files." };
}

void cmGlobalFastbuildGenerator::Generate()
{
  // Check minimum Fastbuild version.
  if (cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
                                    this->FastbuildVersion,
                                    RequiredFastbuildVersion())) {
    std::ostringstream msg;
    msg << "The detected version of Fastbuild (" << this->FastbuildVersion;
    msg << ") is less than the version of Fastbuild required by CMake (";
    msg << this->RequiredFastbuildVersion() << ").";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return;
  }
  this->ProcessEnvironment();

  this->OpenBuildFileStream();

  this->WriteSettings();
  this->WriteEnvironment();

  // Execute the standard generate process
  cmGlobalGenerator::Generate();

  // Write compilers
  this->WriteCompilers();

  this->WriteTargets();

  this->CloseBuildFileStream();

  if (cmSystemTools::GetErrorOccurredFlag()) {
    return;
  }

  this->RemoveUnknownClangTidyExportFixesFiles();

  if (this->GetCMakeInstance()->GetRegenerateDuringBuild()) {
    return;
  }
  // TODO: figure out how to skip this in TryCompile
  //  Make "rebuild-bff" target up-to-date after the generation.
  //  This is actually a noop, it just asks CMake to touch the generated file
  //  so FASTBuild would consider the target as up-to-date.
  AskCMakeToMakeRebuildBFFUpToDate(
    this->GetCMakeInstance()->GetHomeOutputDirectory());
}

void cmGlobalFastbuildGenerator::AskCMakeToMakeRebuildBFFUpToDate(
  std::string const& workingDir) const
{
  // "restat" the generated build file.
  // The idea here is to mimic what Ninja's "restat" command does.
  // We need to make the "rebuild.bff" target up-to-date, so the regeneration
  // will only be triggered when CMake files have actually changed.
  // Tested in "RunCMake.Configure" test.
  cmsys::ofstream{
    cmStrCat(workingDir, '/', FASTBUILD_RESTAT_FILE).c_str(),
    std::ios::out | std::ios::binary
  } << cmStrCat(workingDir, '/', FASTBUILD_BUILD_FILE);
  std::string output;
  ExecuteFastbuildTarget(workingDir, FASTBUILD_REBUILD_BFF_TARGET_NAME,
                         output);
}

void cmGlobalFastbuildGenerator::ExecuteFastbuildTarget(
  std::string const& dir, std::string const& target, std::string& output,
  std::vector<std::string> const& fbuildOptions) const
{
  std::vector<std::string> command;

  command.emplace_back(this->FastbuildCommand);
  command.emplace_back("-config");
  std::string const file = cmStrCat(dir, '/', FASTBUILD_BUILD_FILE);
  command.emplace_back(file);
  command.emplace_back(target);
  if (!fbuildOptions.empty()) {
    command.emplace_back(cmJoin(fbuildOptions, " "));
  }

  int retVal = 0;
  if (!cmSystemTools::RunSingleCommand(command, &output, nullptr, &retVal,
                                       dir.c_str(),
                                       cmSystemTools::OUTPUT_NONE) ||
      retVal != 0) {
    cmSystemTools::Error(cmStrCat("Failed to run FASTBuild command:\n  '",
                                  cmJoin(command, "' '"), "'\nOutput:\n",
                                  output));
    cmSystemTools::Stdout(output);
    std::exit(retVal);
  }
}

void cmGlobalFastbuildGenerator::WriteSettings()
{
  // Define some placeholder
  WriteDivider();
  *this->BuildFileStream << "// Helper variables\n\n";

  WriteVariable("FB_INPUT_1_PLACEHOLDER", Quote("\"%1\""));
  WriteVariable("FB_INPUT_1_0_PLACEHOLDER", Quote("\"%1[0]\""));
  WriteVariable("FB_INPUT_1_1_PLACEHOLDER", Quote("\"%1[1]\""));
  WriteVariable("FB_INPUT_2_PLACEHOLDER", Quote("\"%2\""));
  WriteVariable("FB_INPUT_3_PLACEHOLDER", Quote("\"%3\""));

  std::string cacheDir;
  // If explicitly set from CMake.
  auto val = this->GetSafeGlobalSetting(FASTBUILD_CACHE_PATH);
  if (!val.empty()) {
    cacheDir = std::move(val);
  } else {
    // Default value.
    cacheDir =
      this->GetCMakeInstance()->GetHomeOutputDirectory() + "/fbuild.cache";
  }
  cmSystemTools::ConvertToOutputSlashes(cacheDir);

  WriteDivider();
  *this->BuildFileStream << "// Settings\n\n";

  WriteCommand("Settings");
  *this->BuildFileStream << "{\n";
  if (!cacheDir.empty()) {
    WriteVariable("CachePath", Quote(cacheDir), 1);
  }
  // Concurrency groups.
  WriteStruct(
    FASTBUILD_UTIL_CONCURRENCY_GROUP_NAME,
    { { "ConcurrencyGroupName", Quote(FASTBUILD_UTIL_CONCURRENCY_GROUP_NAME) },
      { "ConcurrencyLimit", "1" } },
    1);

  WriteArray("ConcurrencyGroups",
             { "." FASTBUILD_UTIL_CONCURRENCY_GROUP_NAME }, 1);

  *this->BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteEnvironment()
{
  if (!LocalEnvironment.empty()) {
    WriteArray(FASTBUILD_ENV_VAR_NAME, Wrap(LocalEnvironment), 0);
  }
}

void cmGlobalFastbuildGenerator::WriteDivider()
{
  *this->BuildFileStream << "// ======================================"
                            "=======================================\n";
}

void cmGlobalFastbuildGenerator::Indent(int count)
{
  for (int i = 0; i < count; ++i) {
    *this->BuildFileStream << "  ";
  }
}

void cmGlobalFastbuildGenerator::WriteComment(std::string const& comment,
                                              int indent)
{
  if (comment.empty()) {
    return;
  }

  std::string::size_type lpos = 0;
  std::string::size_type rpos;
  *this->BuildFileStream << "\n";
  Indent(indent);
  *this->BuildFileStream << "/////////////////////////////////////////////\n";
  while ((rpos = comment.find('\n', lpos)) != std::string::npos) {
    Indent(indent);
    *this->BuildFileStream << "// " << comment.substr(lpos, rpos - lpos)
                           << "\n";
    lpos = rpos + 1;
  }
  Indent(indent);
  *this->BuildFileStream << "// " << comment.substr(lpos) << "\n\n";
}

void cmGlobalFastbuildGenerator::WriteVariable(std::string const& key,
                                               std::string const& value,
                                               int indent)
{
  WriteVariable(key, value, "=", indent);
}

void cmGlobalFastbuildGenerator::WriteVariable(std::string const& key,
                                               std::string const& value,
                                               std::string const& op,
                                               int indent)
{
  Indent(indent);
  *this->BuildFileStream << "." << key << " " + op + " " << value << "\n";
}

void cmGlobalFastbuildGenerator::WriteCommand(std::string const& command,
                                              std::string const& value,
                                              int indent)
{
  Indent(indent);
  *this->BuildFileStream << command;
  if (!value.empty()) {
    *this->BuildFileStream << "(" << value << ")";
  }
  *this->BuildFileStream << "\n";
}

void cmGlobalFastbuildGenerator::WriteArray(
  std::string const& key, std::vector<std::string> const& values, int indent)
{
  WriteArray(key, values, "=", indent);
}

void cmGlobalFastbuildGenerator::WriteArray(
  std::string const& key, std::vector<std::string> const& values,
  std::string const& op, int indent)
{
  WriteVariable(key, "", op, indent);
  Indent(indent);
  *this->BuildFileStream << "{\n";
  char const* sep = "";
  for (std::string const& value : values) {
    *this->BuildFileStream << sep;
    sep = ",\n";
    Indent(indent + 1);
    *this->BuildFileStream << value;
  }
  *this->BuildFileStream << "\n";
  Indent(indent);
  *this->BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteStruct(
  std::string const& name,
  std::vector<std::pair<std::string, std::string>> const& variables,
  int indent)
{
  WriteVariable(name, "", "=", indent);
  Indent(indent);
  *this->BuildFileStream << "[\n";
  for (auto const& val : variables) {
    auto const& key = val.first;
    auto const& value = val.second;
    WriteVariable(key, value, "=", indent + 1);
  }
  Indent(indent);
  *this->BuildFileStream << "]\n";
}

std::string cmGlobalFastbuildGenerator::Quote(std::string const& str,
                                              std::string const& quotation)
{
  std::string result = str;
  cmSystemTools::ReplaceString(result, quotation, "^" + quotation);
  cmSystemTools::ReplaceString(result, FASTBUILD_DOLLAR_TAG, "$");
  return quotation + result + quotation;
}
std::string cmGlobalFastbuildGenerator::QuoteIfHasSpaces(std::string str)
{
  if (str.find(' ') != std::string::npos) {
    return '"' + str + '"';
  }
  return str;
}

struct WrapHelper
{
  std::string Prefix;
  std::string Suffix;
  bool EscapeDollar;

  std::string operator()(std::string in)
  {
    // If we have ^ in env variable - need to escape it.
    cmSystemTools::ReplaceString(in, "^", "^^");
    // Those all are considered as line ends by FASTBuild.
    cmSystemTools::ReplaceString(in, "\n", "\\n");
    cmSystemTools::ReplaceString(in, "\r", "\\r");
    // Escaping of single quotes tested in "RunCMake.CompilerArgs" test.
    cmSystemTools::ReplaceString(in, "'", "^'");
    std::string result = Prefix + in + Suffix;
    if (EscapeDollar) {
      cmSystemTools::ReplaceString(result, "$", "^$");
      cmSystemTools::ReplaceString(result, FASTBUILD_DOLLAR_TAG, "$");
    }
    return result;
  }
  std::string operator()(FastbuildTargetDep const& in)
  {
    std::string res = in.Name;
    if (in.Type == FastbuildTargetDepType::ALL) {
      res += FASTBUILD_ALL_ALIAS_POSTFIX;
    } else if (in.Type == FastbuildTargetDepType::BUILD) {
      res += FASTBUILD_BUILD_ALIAS_POSTFIX;
    }

    return (*this)(std::move(res));
  }
};
template <class T>
std::vector<std::string> cmGlobalFastbuildGenerator::Wrap(
  T const& in, std::string const& prefix, std::string const& suffix,
  bool const escape_dollar)
{

  std::vector<std::string> result;

  WrapHelper helper = { prefix, suffix, escape_dollar };

  std::transform(in.begin(), in.end(), std::back_inserter(result), helper);

  return result;
}

void cmGlobalFastbuildGenerator::TopologicalSort(
  std::vector<FastbuildTargetPtrT>& nodes)
{
  std::unordered_map<std::string, int> inDegree;
  std::unordered_map<std::string, std::set<std::string>> reverseDeps;
  std::unordered_map<std::string, std::size_t> originalIndex;

  // Track original positions
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    auto const& node = nodes[i];
    inDegree[node->Name] = 0;
    originalIndex[node->Name] = i;
  }

  // Build reverse dependency graph and in-degree map
  for (auto const& node : nodes) {
    for (auto const& dep : node->PreBuildDependencies) {
      if (inDegree.count(dep.Name)) {
        reverseDeps[dep.Name].insert(node->Name);
        ++inDegree[node->Name];
      }
    }
  }

  // Min-heap based on original position
  auto const cmp = [&](std::string const& a, std::string const& b) {
    return originalIndex[a] > originalIndex[b];
  };
  std::priority_queue<std::string, std::vector<std::string>, decltype(cmp)>
    zeroInDegree(cmp);

  for (auto const& val : inDegree) {
    auto const& degree = val.second;
    auto const& name = val.first;
    if (degree == 0) {
      zeroInDegree.push(name);
    }
  }

  std::vector<std::string> sorted;
  while (!zeroInDegree.empty()) {
    std::string node = zeroInDegree.top();
    zeroInDegree.pop();
    sorted.push_back(node);
    for (auto const& dep : reverseDeps[node]) {
      if (--inDegree[dep] == 0) {
        zeroInDegree.push(dep);
      }
    }
  }

  if (sorted.size() != nodes.size()) {
    cmSystemTools::Error("Failed to sort (Cyclic dependency)");
    cmSystemTools::Error(cmStrCat("Sorted size: ", sorted.size()));
    cmSystemTools::Error(cmStrCat("nodes size: ", nodes.size()));
    for (auto const& node : nodes) {
      cmSystemTools::Error("Node: " + node->Name);
      for (auto const& dep : reverseDeps[node->Name]) {
        cmSystemTools::Error("\tReverse dep: " + dep);
      }
      for (auto const& child : node->PreBuildDependencies) {
        cmSystemTools::Error("\tChild: " + child.Name);
      }
    }
    for (auto const& node : sorted) {
      cmSystemTools::Error("Sorted: " + node);
    }
    for (auto const& node : nodes) {
      cmSystemTools::Error("In node: " + node->Name);
    }
  }

  // Reconstruct sorted nodes
  std::vector<FastbuildTargetPtrT> result;
  for (auto const& name : sorted) {
    auto it = std::find_if(
      nodes.begin(), nodes.end(), [&name](FastbuildTargetPtrT const& node) {
        return node /* the node might be in moved-from state*/ &&
          node->Name == name;
      });
    if (it != nodes.end()) {
      result.emplace_back(std::move(*it));
    }
  }

  std::swap(result, nodes);
}

void cmGlobalFastbuildGenerator::WriteDisclaimer()
{
  *this->BuildFileStream << "// CMAKE generated file: DO NOT EDIT!\n"
                         << "// Generated by \"" << this->GetName() << "\""
                         << " Generator, CMake Version "
                         << cmVersion::GetMajorVersion() << "."
                         << cmVersion::GetMinorVersion() << "\n\n";
}

void cmGlobalFastbuildGenerator::OpenBuildFileStream()
{
  // Compute Fastbuild's build file path.
  std::string buildFilePath =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  buildFilePath += "/";
  buildFilePath += FASTBUILD_BUILD_FILE;

  // Get a stream where to generate things.
  if (!this->BuildFileStream) {
    this->BuildFileStream = cm::make_unique<cmGeneratedFileStream>(
      buildFilePath, false, this->GetMakefileEncoding());
    if (!this->BuildFileStream) {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return;
    }
  }

  // Write the do not edit header.
  this->WriteDisclaimer();

  // Write a comment about this file.
  *this->BuildFileStream
    << "// This file contains all the build statements\n\n";
}

void cmGlobalFastbuildGenerator::CloseBuildFileStream()
{
  if (this->BuildFileStream) {
    this->BuildFileStream.reset();
  } else {
    cmSystemTools::Error("Build file stream was not open.");
  }
}

void cmGlobalFastbuildGenerator::WriteCompilers()
{
  WriteDivider();
  *this->BuildFileStream << "// Compilers\n\n";
  for (auto const& val : Compilers) {
    auto const& compilerDef = val.second;

    std::string fastbuildFamily = "custom";

    if (compilerDef.Language == "C" || compilerDef.Language == "CXX" ||
        compilerDef.Language == "CUDA") {
      std::map<std::string, std::string> compilerIdToFastbuildFamily = {
        { "MSVC", "msvc" },        { "Clang", "clang" },
        { "AppleClang", "clang" }, { "GNU", "gcc" },
        { "NVIDIA", "cuda-nvcc" }, { "Clang-cl", "clang-cl" },
      };
      auto ft = compilerIdToFastbuildFamily.find(compilerDef.CmakeCompilerID);
      if (ft != compilerIdToFastbuildFamily.end()) {
        fastbuildFamily = ft->second;
      }
    }

    std::string compilerPath = compilerDef.Executable;

    // Write out the compiler that has been configured
    WriteCommand("Compiler", Quote(compilerDef.Name));
    *this->BuildFileStream << "{\n";
    for (auto const& extra : compilerDef.ExtraVariables) {
      auto const& extraKey = extra.first;
      auto const& extraVal = extra.second;
      WriteVariable(extraKey, Quote(extraVal), 1);
    }
    WriteVariable("Executable", Quote(compilerPath), 1);
    WriteVariable("CompilerFamily", Quote(fastbuildFamily), 1);
    if (compilerDef.UseLightCache) {
      WriteVariable("UseLightCache_Experimental", "true", 1);
    }
    if (fastbuildFamily == "clang") {
      WriteVariable("ClangRewriteIncludes", "false", 1);
    }

    if (compilerDef.DontUseEnv) {
      LogMessage("Not using system environment");
    } else {
      WriteVariable("Environment", "." FASTBUILD_ENV_VAR_NAME, 1);
    }
    if (!compilerDef.ExtraFiles.empty()) {
      // Do not escape '$' sign, CMAKE_${LANG}_FASTBUILD_EXTRA_FILES might
      // contain FB variables to be expanded (we do use some internally).
      // Besides a path cannot contain a '$'
      WriteArray("ExtraFiles", Wrap(compilerDef.ExtraFiles, "'", "'", false),
                 1);
    }
    *this->BuildFileStream << "}\n";

    auto const compilerId = compilerDef.Name;
    WriteVariable(compilerId, Quote(compilerDef.Name));
    *this->BuildFileStream << "\n";
  }
  // We need this because the Library command needs a compiler
  // even if don't compile anything
  if (!this->Compilers.empty()) {
    WriteVariable("Compiler_dummy",
                  Quote(this->Compilers.begin()->second.Name));
  }
}

void cmGlobalFastbuildGenerator::AddCompiler(std::string const& language,
                                             cmMakefile* mf)
{
  if (this->Compilers.find(FASTBUILD_COMPILER_PREFIX + language) !=
      this->Compilers.end()) {
    return;
  }

  // Calculate the root location of the compiler
  std::string const variableString = "CMAKE_" + language + "_COMPILER";
  std::string const compilerLocation = mf->GetSafeDefinition(variableString);
  if (compilerLocation.empty()) {
    return;
  }

  // Calculate the i18n number.
  std::string i18nNum = "1033";

  // Add the language to the compiler's name
  FastbuildCompiler compilerDef;
  compilerDef.ExtraVariables["Root"] =
    cmSystemTools::GetFilenamePath(compilerLocation);
  compilerDef.Name = FASTBUILD_COMPILER_PREFIX + language;
  compilerDef.Executable = compilerLocation;
  compilerDef.CmakeCompilerID =
    mf->GetSafeDefinition("CMAKE_" + language + "_COMPILER_ID");
  if (compilerDef.CmakeCompilerID == "Clang" &&
      mf->GetSafeDefinition("CMAKE_" + language +
                            "_COMPILER_FRONTEND_VARIANT") == "MSVC") {
    compilerDef.CmakeCompilerID = "Clang-cl";
  }

  compilerDef.CmakeCompilerVersion =
    mf->GetSafeDefinition("CMAKE_" + language + "_COMPILER_VERSION");
  compilerDef.Language = language;

  if (compilerDef.CmakeCompilerID == "MSVC" &&
      cmIsOn(mf->GetSafeDefinition(FASTBUILD_USE_LIGHTCACHE)) &&
      (language == "C" || language == "CXX")) {
    compilerDef.UseLightCache = true;
  }
  cmExpandList(mf->GetSafeDefinition(FASTBUILD_COMPILER_EXTRA_FILES),
               compilerDef.ExtraFiles);

  // If FASTBUILD_COMPILER_EXTRA_FILES is not set - automatically add extra
  // files based on compiler (see
  // https://fastbuild.org/docs/functions/compiler.html)
  if (compilerDef.ExtraFiles.empty() &&
      (language == "C" || language == "CXX") &&
      compilerDef.CmakeCompilerID == "MSVC") {
    // https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html

    // Visual Studio 17 (19.30 to 19.39)
    // TODO

    // Visual Studio 16 (19.20 to 19.29)
    if (cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                      compilerDef.CmakeCompilerVersion,
                                      "19.20")) {
      compilerDef.ExtraFiles.push_back("$Root$/c1.dll");
      compilerDef.ExtraFiles.push_back("$Root$/c1xx.dll");
      compilerDef.ExtraFiles.push_back("$Root$/c2.dll");
      compilerDef.ExtraFiles.push_back(
        "$Root$/atlprov.dll"); // Only needed if using ATL
      compilerDef.ExtraFiles.push_back("$Root$/msobj140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdb140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdbcore.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdbsrv.exe");
      compilerDef.ExtraFiles.push_back("$Root$/mspft140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/msvcp140.dll");
      compilerDef.ExtraFiles.push_back(
        "$Root$/msvcp140_atomic_wait.dll"); // Required circa 16.8.3
                                            // (14.28.29333)
      compilerDef.ExtraFiles.push_back(
        "$Root$/tbbmalloc.dll"); // Required as of 16.2 (14.22.27905)
      compilerDef.ExtraFiles.push_back("$Root$/vcruntime140.dll");
      compilerDef.ExtraFiles.push_back(
        "$Root$/vcruntime140_1.dll"); // Required as of 16.5.1 (14.25.28610)
      compilerDef.ExtraFiles.push_back("$Root$/" + i18nNum + "/clui.dll");
      compilerDef.ExtraFiles.push_back(
        "$Root$/" + i18nNum + "/mspft140ui.dll"); // Localized messages for
                                                  // static analysis
    }
    // Visual Studio 15 (19.10 to 19.19)
    else if (cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                           compilerDef.CmakeCompilerVersion,
                                           "19.10")) {
      compilerDef.ExtraFiles.push_back("$Root$/c1.dll");
      compilerDef.ExtraFiles.push_back("$Root$/c1xx.dll");
      compilerDef.ExtraFiles.push_back("$Root$/c2.dll");
      compilerDef.ExtraFiles.push_back(
        "$Root$/atlprov.dll"); // Only needed if using ATL
      compilerDef.ExtraFiles.push_back("$Root$/msobj140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdb140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdbcore.dll");
      compilerDef.ExtraFiles.push_back("$Root$/mspdbsrv.exe");
      compilerDef.ExtraFiles.push_back("$Root$/mspft140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/msvcp140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/vcruntime140.dll");
      compilerDef.ExtraFiles.push_back("$Root$/" + i18nNum + "/clui.dll");
    }
  }
  // TODO: Handle Intel compiler

  this->Compilers[compilerDef.Name] = std::move(compilerDef);
}

void cmGlobalFastbuildGenerator::AddLauncher(std::string const& prefix,
                                             std::string const& launcher,
                                             std::string const& language,
                                             std::string const& args)
{
  if (this->Compilers.find(prefix + language) != this->Compilers.end()) {
    return;
  }
  LogMessage("Launcher: " + launcher);
  LogMessage("Launcher args: " + args);
  FastbuildCompiler compilerDef;
  compilerDef.Name = prefix + language;
  compilerDef.Args = args;
  if (cmSystemTools::FileIsFullPath(launcher)) {
    compilerDef.Executable = launcher;
  } else {
    // FASTBuild needs an absolute path to the executable.
    compilerDef.Executable = cmSystemTools::FindProgram(launcher);
    if (compilerDef.Executable.empty()) {
      cmSystemTools::Error("Failed to find path to " + launcher);
      return;
    }
  }
  // When CTest is used as a launcher, there is an interesting env variable
  // "CTEST_LAUNCH_LOGS" which is set by parent CTest process and is expected
  // to be read from global (sic!) env by the launched CTest process. So we
  // will need to make this global env available for CTest executable used as a
  // "launcher". Tested in RunCMake.ctest_labels_for_subprojects test..
  compilerDef.DontUseEnv = true;
  this->Compilers[compilerDef.Name] = std::move(compilerDef);
}

std::string cmGlobalFastbuildGenerator::ConvertToFastbuildPath(
  std::string const& path) const
{
  cmLocalGenerator const* root = LocalGenerators[0].get();
  return root->MaybeRelativeToWorkDir(cmSystemTools::FileIsFullPath(path)
                                        ? cmSystemTools::CollapseFullPath(path)
                                        : path);
}

std::unique_ptr<cmLinkLineComputer>
cmGlobalFastbuildGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter,
  cmStateDirectory const& /* stateDir */) const
{
  return cm::make_unique<cmFastbuildLinkLineComputer>(
    outputConverter,
    this->LocalGenerators[0]->GetStateSnapshot().GetDirectory(), this);
}

void cmGlobalFastbuildGenerator::WriteExec(FastbuildExecNode const& Exec,
                                           int indent)
{
  auto const identPlus1 = indent + 1;
  WriteCommand("Exec", Exec.Name.empty() ? std::string{} : Quote(Exec.Name),
               indent);
  Indent(indent);
  *BuildFileStream << "{\n";
  {
    if (!Exec.PreBuildDependencies.empty()) {
      WriteArray("PreBuildDependencies", Wrap(Exec.PreBuildDependencies),
                 identPlus1);
    }
    WriteVariable("ExecExecutable", Quote(Exec.ExecExecutable), identPlus1);
    if (!Exec.ExecArguments.empty()) {
      WriteVariable("ExecArguments", Quote(Exec.ExecArguments), identPlus1);
    }
    if (!Exec.ExecWorkingDir.empty()) {
      WriteVariable("ExecWorkingDir", Quote(Exec.ExecWorkingDir), identPlus1);
    }
    if (!Exec.ExecInput.empty()) {
      WriteArray("ExecInput", Wrap(Exec.ExecInput), identPlus1);
    }
    if (Exec.ExecUseStdOutAsOutput) {
      WriteVariable("ExecUseStdOutAsOutput", "true", identPlus1);
    }
    if (!Exec.ExecInputPath.empty()) {
      WriteArray("ExecInputPath", Wrap(Exec.ExecInputPath), identPlus1);
    }
    if (!Exec.ExecInputPattern.empty()) {
      WriteArray("ExecInputPattern", Wrap(Exec.ExecInputPattern), identPlus1);
    }
    WriteVariable("ExecAlwaysShowOutput", "true", identPlus1);
    WriteVariable("ExecOutput", Quote(Exec.ExecOutput), identPlus1);
    WriteVariable("ExecAlways", Exec.ExecAlways ? "true" : "false",
                  identPlus1);
    if (!Exec.ConcurrencyGroupName.empty()) {
      WriteVariable("ConcurrencyGroupName", Quote(Exec.ConcurrencyGroupName),
                    identPlus1);
    }
  }
  Indent(indent);
  *BuildFileStream << "}\n";
  WriteAlias(Exec.OutputsAlias);
  WriteAlias(Exec.ByproductsAlias);
}

void cmGlobalFastbuildGenerator::WriteObjectList(
  FastbuildObjectListNode const& ObjectList, bool allowDistribution)
{
  WriteCommand("ObjectList", Quote(ObjectList.Name), 1);
  Indent(1);
  *BuildFileStream << "{\n";
  {
    if (!allowDistribution) {
      WriteVariable("AllowDistribution", "false", 2);
    }
    if (!ObjectList.PreBuildDependencies.empty()) {
      WriteArray("PreBuildDependencies", Wrap(ObjectList.PreBuildDependencies),
                 2);
    }
    WriteVariable("Compiler", ObjectList.Compiler, 2);
    // If only PCH output is present - this node reuses existing PCH.
    if (!ObjectList.PCHOutputFile.empty()) {
      WriteVariable("PCHOutputFile", Quote(ObjectList.PCHOutputFile), 2);
    }
    // If PCHInputFile and PCHOptions are present  - this node creates PCH.
    if (!ObjectList.PCHInputFile.empty() && !ObjectList.PCHOptions.empty()) {
      WriteVariable("PCHInputFile", Quote(ObjectList.PCHInputFile), 2);
      WriteVariable("PCHOptions", Quote(ObjectList.PCHOptions), 2);
    }
    WriteVariable("CompilerOptions", Quote(ObjectList.CompilerOptions), 2);
    WriteVariable("CompilerOutputPath", Quote(ObjectList.CompilerOutputPath),
                  2);
    WriteVariable("CompilerOutputExtension",
                  Quote(ObjectList.CompilerOutputExtension), 2);
    WriteVariable("CompilerOutputKeepBaseExtension", "true", 2);
    WriteArray("CompilerInputFiles", Wrap(ObjectList.CompilerInputFiles), 2);
    if (!ObjectList.AllowCaching) {
      WriteVariable("AllowCaching", "false", 2);
    }
    if (!ObjectList.AllowDistribution) {
      WriteVariable("AllowDistribution", "false", 2);
    }
    if (ObjectList.Hidden) {
      WriteVariable("Hidden", "true", 2);
    }
  }
  Indent(1);
  *BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteLinker(
  FastbuildLinkerNode const& LinkerNode, bool allowDistribution)
{
  WriteCommand(
    LinkerNode.Type == FastbuildLinkerNode::EXECUTABLE         ? "Executable"
      : LinkerNode.Type == FastbuildLinkerNode::SHARED_LIBRARY ? "DLL"
                                                               : "Library",
    LinkerNode.Name != LinkerNode.LinkerOutput ? Quote(LinkerNode.Name) : "",
    1);
  Indent(1);
  *BuildFileStream << "{\n";
  {
    if (!LinkerNode.PreBuildDependencies.empty()) {
      WriteArray("PreBuildDependencies", Wrap(LinkerNode.PreBuildDependencies),
                 2);
    }
    if (!allowDistribution) {
      WriteVariable("AllowDistribution", "false", 2);
    }

    if (!LinkerNode.Compiler.empty()) {
      WriteVariable("Compiler", LinkerNode.Compiler, 2);
      WriteVariable("CompilerOptions", Quote(LinkerNode.CompilerOptions), 2);
      WriteVariable("CompilerOutputPath", Quote("."), 2);
    }
    if (!LocalEnvironment.empty()) {
      WriteVariable("Environment", "." FASTBUILD_ENV_VAR_NAME, 2);
    }

    WriteVariable(LinkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY
                    ? "Librarian"
                    : "Linker",
                  Quote(LinkerNode.Linker), 2);

    WriteVariable(LinkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY
                    ? "LibrarianOptions"
                    : "LinkerOptions",
                  Quote(LinkerNode.LinkerOptions), 2);

    WriteVariable(LinkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY
                    ? "LibrarianOutput"
                    : "LinkerOutput",
                  Quote(LinkerNode.LinkerOutput), 2);

    if (!LinkerNode.LibrarianAdditionalInputs.empty()) {
      WriteArray(LinkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY
                   ? "LibrarianAdditionalInputs"
                   : "Libraries",
                 Wrap(LinkerNode.LibrarianAdditionalInputs), 2);
    }
    if (!LinkerNode.Libraries2.empty()) {
      WriteArray("Libraries2", Wrap(LinkerNode.Libraries2), 2);
    }
    if (!LinkerNode.LibrarianAdditionalInputs.empty()) {

      if (!LinkerNode.LinkerType.empty()) {
        WriteVariable("LinkerType", Quote(LinkerNode.LinkerType), 2);
      }
    }
    if (LinkerNode.Type == FastbuildLinkerNode::EXECUTABLE ||
        LinkerNode.Type == FastbuildLinkerNode::SHARED_LIBRARY) {
      WriteVariable("LinkerLinkObjects",
                    LinkerNode.LinkerLinkObjects ? "true" : "false", 2);

      if (!LinkerNode.LinkerStampExe.empty()) {
        WriteVariable("LinkerStampExe", Quote(LinkerNode.LinkerStampExe), 2);
        if (!LinkerNode.LinkerStampExeArgs.empty()) {
          WriteVariable("LinkerStampExeArgs",
                        Quote(LinkerNode.LinkerStampExeArgs), 2);
        }
      }
    }
    Indent(1);
    *BuildFileStream << "}\n";
  }
}

void cmGlobalFastbuildGenerator::WriteAlias(FastbuildAliasNode const& Alias,
                                            int indent)
{
  if (Alias.PreBuildDependencies.empty()) {
    return;
  }
  auto const identPlus1 = indent + 1;
  WriteCommand("Alias", Quote(Alias.Name), indent);
  Indent(indent);
  *BuildFileStream << "{\n";
  WriteArray("Targets", Wrap(Alias.PreBuildDependencies), identPlus1);
  if (Alias.Hidden) {
    WriteVariable("Hidden", "true", identPlus1);
  }
  Indent(indent);
  *BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteCopy(FastbuildCopyNode const& Copy)
{
  cmGlobalFastbuildGenerator::WriteCommand(
    Copy.CopyDir ? "CopyDir" : "Copy",
    cmGlobalFastbuildGenerator::Quote(Copy.Name), 1);
  cmGlobalFastbuildGenerator::Indent(1);

  *BuildFileStream << "{\n";
  WriteVariable("PreBuildDependencies",
                cmGlobalFastbuildGenerator::Quote(Copy.PreBuildDependencies),
                2);
  WriteVariable(Copy.CopyDir ? "SourcePaths" : "Source",
                cmGlobalFastbuildGenerator::Quote(Copy.Source), 2);
  WriteVariable("Dest", cmGlobalFastbuildGenerator::Quote(Copy.Dest), 2);
  cmGlobalFastbuildGenerator::Indent(1);
  *BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteTarget(FastbuildTarget const& target)
{
  for (auto const& val : target.Variables) {
    auto const& key = val.first;
    auto const& value = val.second;
    WriteVariable(key, cmGlobalFastbuildGenerator::Quote(value), 1);
  }
  // add_custom_commands(...)
  for (auto const& alias : { target.ExecNodes }) {
    this->WriteAlias(alias);
  }

  // -deps Alias.
  this->WriteAlias(target.DependenciesAlias);

  // PRE_BUILD.
  for (auto const& alias : { target.PreBuildExecNodes }) {
    this->WriteAlias(alias);
  }

  // Copy commands.

  for (FastbuildCopyNode const& node : target.CopyNodes) {
    this->WriteCopy(node);
  }

  // Objects.
  for (FastbuildObjectListNode const& objectList : target.ObjectListNodes) {
    this->WriteObjectList(objectList, target.AllowDistribution);
  }

  if (!target.PreLinkExecNodes.Nodes.empty()) {
    for (auto const& exec : target.PreLinkExecNodes.Nodes) {
      this->WriteExec(exec);
    }
    this->WriteAlias(target.PreLinkExecNodes.Alias);
  }

  // Libraries / executables.
  if (!target.LinkerNode.empty()) {
    for (auto const& linkerNode : target.LinkerNode) {
      this->WriteLinker(linkerNode, target.AllowDistribution);
    }
  }

  // -build.
  this->WriteAlias(target.BuildAlias);

  if (!target.PostBuildExecNodes.Nodes.empty()) {
    for (auto const& exec : target.PostBuildExecNodes.Nodes) {
      this->WriteExec(exec);
    }
    this->WriteAlias(target.PostBuildExecNodes.Alias);
  }

  // Aliases (if any).
  for (FastbuildAliasNode const& alias : target.AliasNodes) {
    this->WriteAlias(alias);
  }
}
void cmGlobalFastbuildGenerator::WriteIDEProjects()
{
  for (auto const& proj : IDEProjects) {
    // VS
    auto const& VSProj = proj.second.first;
    WriteCommand("VCXProject", Quote(VSProj.Alias));
    *this->BuildFileStream << "{\n";

    WriteVariable("ProjectOutput", Quote(VSProj.ProjectOutput), 1);
    WriteIDEProjectConfig(VSProj.ProjectConfigs);
    WriteVSBuildCommands();
    WriteIDEProjectCommon(VSProj);
    *this->BuildFileStream << "}\n\n";

    // XCode
    // On Windows Xcode project's nested paths can become too long.
#if !defined(_WIN32)
    auto const& XCodeProj = proj.second.second;
    WriteCommand("XCodeProject", Quote(XCodeProj.Alias), 0);
    *this->BuildFileStream << "{\n";
    WriteVariable("ProjectOutput", Quote(XCodeProj.ProjectOutput), 1);
    WriteIDEProjectConfig(XCodeProj.ProjectConfigs);
    WriteXCodeBuildCommands();

    WriteIDEProjectCommon(XCodeProj);
    *this->BuildFileStream << "}\n\n";
#endif
  }

  this->WriteSolution();
#if !defined(_WIN32)
  this->WriteXCodeTopLevelProject();
#endif
}

void cmGlobalFastbuildGenerator::WriteVSBuildCommands()
{
  WriteVariable("ProjectBuildCommand",
                Quote(FASTBUILD_IDE_VS_COMMAND_PREFIX +
                      this->FastbuildCommand +
                      FASTBUILD_IDE_BUILD_ARGS " ^$(ProjectName)"),
                1);
  WriteVariable("ProjectRebuildCommand",
                Quote(FASTBUILD_IDE_VS_COMMAND_PREFIX +
                      this->FastbuildCommand +
                      FASTBUILD_IDE_BUILD_ARGS "-clean ^$(ProjectName)"),
                1);
  WriteVariable("ProjectCleanCommand",
                Quote(FASTBUILD_IDE_VS_COMMAND_PREFIX +
                      this->FastbuildCommand + " -ide clean"),
                1);
}
void cmGlobalFastbuildGenerator::WriteXCodeBuildCommands()
{
  WriteVariable("XCodeBuildToolPath", Quote(this->FastbuildCommand), 1);
  WriteVariable("XCodeBuildToolArgs",
                Quote(FASTBUILD_IDE_BUILD_ARGS "^$(FASTBUILD_TARGET)"), 1);
  WriteVariable("XCodeBuildWorkingDir",
                Quote(this->CMakeInstance->GetHomeOutputDirectory()), 1);
}

void cmGlobalFastbuildGenerator::WriteIDEProjectCommon(
  IDEProjectCommon const& project)
{
  WriteVariable("ProjectBasePath", Quote(project.ProjectBasePath), 1);
  // So Fastbuild will pick up files relative to CMakeLists.txt
  WriteVariable("ProjectInputPaths", Quote(project.ProjectBasePath), 1);
}

void cmGlobalFastbuildGenerator::WriteIDEProjectConfig(
  std::vector<IDEProjectConfig> const& configs, std::string const& keyName)
{
  std::vector<std::string> allConfigVariables;
  for (auto const& config : configs) {
    std::string configName = "Config" + config.Config;
    WriteVariable(configName, "", 1);
    Indent(1);
    *this->BuildFileStream << "[\n";
    WriteVariable("Config", Quote(config.Config), 2);
    if (!config.Target.empty()) {
      WriteVariable("Target", Quote(config.Target), 2);
    }
    if (!config.Platform.empty()) {
      WriteVariable("Platform", Quote(config.Platform), 2);
    }
    Indent(1);
    *this->BuildFileStream << "]\n";
    allConfigVariables.emplace_back(std::move(configName));
  }
  WriteArray(keyName, Wrap(allConfigVariables, ".", ""), 1);
}

void cmGlobalFastbuildGenerator::AddTargetAll()
{
  FastbuildAliasNode allAliasNode;
  allAliasNode.Name = FASTBUILD_ALL_TARGET_NAME;

  FastbuildTarget allTarget;
  allTarget.Name = allTarget.Name = FASTBUILD_ALL_TARGET_NAME;
  allTarget.IsGlobal = true;

  for (auto const& targetBase : FastbuildTargets) {
    if (targetBase->Type == FastbuildTargetType::LINK) {
      auto const& target = static_cast<FastbuildTarget const&>(*targetBase);
      // Add non-global and non-excluded targets to "all"
      if (!target.IsGlobal && !target.ExcludeFromAll) {
        allAliasNode.PreBuildDependencies.emplace(target.Name +
                                                  FASTBUILD_ALL_ALIAS_POSTFIX);
        allTarget.PreBuildDependencies.emplace(target.Name);
      }
    } else if (targetBase->Type == FastbuildTargetType::ALIAS) {
      auto const& target = static_cast<FastbuildAliasNode const&>(*targetBase);
      if (!target.ExcludeFromAll) {
        allAliasNode.PreBuildDependencies.emplace(target.Name);
      }
    }
  }
  if (allAliasNode.PreBuildDependencies.empty()) {
    allAliasNode.PreBuildDependencies.emplace(FASTBUILD_NOOP_FILE_NAME);
  }
  allTarget.AliasNodes.emplace_back(std::move(allAliasNode));
  this->AddTarget(std::move(allTarget));
}

void cmGlobalFastbuildGenerator::AddGlobCheckExec()
{
  // Tested in "RunCMake.file" test.
  std::string const globScript =
    this->GetCMakeInstance()->GetGlobVerifyScript();
  if (!globScript.empty()) {

    FastbuildExecNode globCheck;
    globCheck.Name = FASTBUILD_GLOB_CHECK_TARGET;
    globCheck.ExecExecutable = cmSystemTools::GetCMakeCommand();
    globCheck.ExecArguments = "-P " FASTBUILD_1_INPUT_PLACEHOLDER;
    globCheck.ExecInput = { this->ConvertToFastbuildPath(globScript) };
    globCheck.ExecAlways = false;
    globCheck.ExecUseStdOutAsOutput = false;
    auto const cache = this->GetCMakeInstance()->GetGlobCacheEntries();
    for (auto const& entry : cache) {
      auto path = cmSystemTools::GetFilenamePath(entry.Expression);
      auto expression = cmSystemTools::GetFilenameName(entry.Expression);
      if (std::find(globCheck.ExecInputPath.begin(),
                    globCheck.ExecInputPath.end(),
                    path) == globCheck.ExecInputPath.end()) {
        globCheck.ExecInputPath.emplace_back(std::move(path));
      }
      if (std::find(globCheck.ExecInputPattern.begin(),
                    globCheck.ExecInputPattern.end(),
                    expression) == globCheck.ExecInputPattern.end()) {
        globCheck.ExecInputPattern.emplace_back(std::move(expression));
      }
    }
    globCheck.ExecOutput = this->ConvertToFastbuildPath(
      this->GetCMakeInstance()->GetGlobVerifyStamp());
    this->AddTarget(std::move(globCheck));
  }
}
void cmGlobalFastbuildGenerator::WriteSolution()
{
  std::string const solutionName = LocalGenerators[0]->GetProjectName();
  std::vector<std::string> VSProjects;

  for (auto const& IDEProj : IDEProjects) {
    auto const VSProj = IDEProj.second.first;
    VSProjects.emplace_back(VSProj.Alias);
  }

  WriteCommand("VSSolution", Quote("solution"));
  *this->BuildFileStream << "{\n";

  WriteVariable("SolutionOutput",
                Quote(cmJoin({ "VisualStudio", solutionName + ".sln" }, "/")),
                1);

  auto const& configs = IDEProjects.begin()->second.first.ProjectConfigs;
  WriteIDEProjectConfig(configs, "SolutionConfigs");
  WriteArray("SolutionProjects", Wrap(VSProjects), 1);

  *this->BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteXCodeTopLevelProject()
{
  std::string const projectName = LocalGenerators[0]->GetProjectName();
  std::vector<std::string> XCodeProjects;
  for (auto const& IDEProj : IDEProjects) {
    auto const XCodeProj = IDEProj.second.second;
    XCodeProjects.emplace_back(XCodeProj.Alias);
  }

  WriteCommand("XCodeProject", Quote("xcode"));
  *this->BuildFileStream << "{\n";

  WriteVariable(
    "ProjectOutput",
    Quote(
      cmJoin({ "XCode", projectName + ".xcodeproj", "project.pbxproj" }, "/")),
    1);
  WriteVariable("ProjectBasePath", Quote(FASTBUILD_XCODE_BASE_PATH), 1);

  auto const& configs = IDEProjects.begin()->second.second.ProjectConfigs;
  WriteIDEProjectConfig(configs);
  WriteArray("ProjectFiles", Wrap(XCodeProjects), 1);

  *this->BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::LogMessage(std::string const& m) const
{
  static bool const verbose = GlobalSettingIsOn(FASTBUILD_VERBOSE_GENERATOR) ||
    cmSystemTools::HasEnv(FASTBUILD_VERBOSE_GENERATOR);
  if (verbose) {
    cmSystemTools::Message(m);
  }
}
void cmGlobalFastbuildGenerator::AddFileToClean(std::string const& file)
{
  AllFilesToClean.insert(file);
}

std::string cmGlobalFastbuildGenerator::GetExternalShellExecutable()
{
  // FindProgram is expensive - touches filesystem and makes syscalls, so cache
  // it.
  static std::string const cached =
#ifdef _WIN32
    cmSystemTools::FindProgram(
      "cmd.exe", std::vector<std::string>{ "C:\\Windows\\System32" });
#else
    cmSystemTools::FindProgram("sh", std::vector<std::string>{ "/bin" });
#endif
  return cached;
}

void cmGlobalFastbuildGenerator::WriteTargetRebuildBFF()
{
  std::vector<std::string> implicitDeps;
  for (auto& lg : LocalGenerators) {
    std::vector<std::string> const& lf = lg->GetMakefile()->GetListFiles();
    for (auto const& dep : lf) {
      implicitDeps.push_back(this->ConvertToFastbuildPath(dep));
    }
  }
  auto const* cmake = this->GetCMakeInstance();
  std::string outDir = cmake->GetHomeOutputDirectory() + '/';

  implicitDeps.push_back(outDir + "CMakeCache.txt");

  FastbuildExecNode rebuildBFF;
  rebuildBFF.Name = FASTBUILD_REBUILD_BFF_TARGET_NAME;
  if (!this->GetCMakeInstance()->GetGlobVerifyScript().empty()) {
    implicitDeps.emplace_back(this->GetCMakeInstance()->GetGlobVerifyStamp());
  }

  std::sort(implicitDeps.begin(), implicitDeps.end());
  implicitDeps.erase(std::unique(implicitDeps.begin(), implicitDeps.end()),
                     implicitDeps.end());
  std::string args =
    cmStrCat("--regenerate-during-build",
             (this->GetCMakeInstance()->GetIgnoreCompileWarningAsError()
                ? " --compile-no-warning-as-error"
                : ""),
             (this->GetCMakeInstance()->GetIgnoreLinkWarningAsError()
                ? " --link-no-warning-as-error"
                : ""),
             " -S", QuoteIfHasSpaces(cmake->GetHomeDirectory()), " -B",
             QuoteIfHasSpaces(cmake->GetHomeOutputDirectory()));

  rebuildBFF.ExecArguments = std::move(args);
  rebuildBFF.ExecInput = implicitDeps;
  rebuildBFF.ExecExecutable = cmSystemTools::GetCMakeCommand();
  rebuildBFF.ExecWorkingDir = outDir;
  rebuildBFF.ExecOutput = outDir + FASTBUILD_BUILD_FILE;
  this->WriteExec(rebuildBFF, 0);
}

void cmGlobalFastbuildGenerator::WriteTargetClean()
{
  WriteCommand("RemoveDir", Quote(FASTBUILD_CLEAN_TARGET_NAME));
  *BuildFileStream << "{\n";

  WriteVariable("RemoveDirs", "false", 1);
  WriteVariable("RemovePathsRecurse", "false", 1);

  std::vector<std::string> removePatterns{
    "*" FASTBUILD_DUMMY_OUTPUT_EXTENSION
  };
  std::vector<std::string> langs;
  this->GetEnabledLanguages(langs);
  for (auto const& lang : langs) {
    auto const extension = this->GetSafeGlobalSetting(
      cmStrCat("CMAKE_", lang, "_OUTPUT_EXTENSION"));
    if (!extension.empty() &&
        std::find(removePatterns.begin(), removePatterns.end(),
                  '*' + extension) == removePatterns.end()) {
      removePatterns.emplace_back('*' + extension);
    }
  }
  auto const PchExtension = this->GetSafeGlobalSetting("CMAKE_PCH_EXTENSION");
  if (!PchExtension.empty()) {
    removePatterns.emplace_back('*' + PchExtension);
  }
  auto const buildDir = this->GetCMakeInstance()->GetHomeOutputDirectory();
  if (!AllFilesToClean.empty()) {
    for (auto const& file : AllFilesToClean) {
      auto const path = cmSystemTools::GetFilenamePath(file);
      // Since we don't recurse (to not delete unrelated .obj), need to add
      // full path to the file.
      // Tested in "ExternalOBJ" test.
      if (!path.empty()) {
        AllFoldersToClean.emplace(cmStrCat(buildDir, '/', path));
      }
      removePatterns.emplace_back('*' + cmSystemTools::GetFilenameName(file));
    }
  }

  WriteArray("RemovePatterns", Wrap(removePatterns), 1);

  std::vector<std::string> tmp;

  std::move(AllFilesToKeep.begin(), AllFilesToKeep.end(),
            std::back_inserter(tmp));
  if (!tmp.empty()) {
    WriteArray("RemoveExcludeFiles", Wrap(tmp), 1);
    tmp.clear();
  }

  tmp.emplace_back(buildDir);
  std::move(AllFoldersToClean.begin(), AllFoldersToClean.end(),
            std::back_inserter(tmp));
  WriteArray("RemovePaths", Wrap(tmp), 1);

  *BuildFileStream << "}\n";
}

void cmGlobalFastbuildGenerator::WriteTargets()
{
  std::string const outputDir = this->CMakeInstance->GetHomeOutputDirectory();
  LogMessage("GetHomeOutputDirectory: " + outputDir);
  // Noop file that 'all' can alias to if we don't have any other targets...
  // The exact location of the "noop" file is verified in one of the tests in
  // "RunCMake.CMakePresetsPackage" test suite.
  cmSystemTools::Touch(cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(),
                                '/', FASTBUILD_NOOP_FILE_NAME),
                       true);
  // Add "all" utility target before sorting, so we can correctly sort
  // targets that depend on it
  AddTargetAll();
  TopologicalSort(FastbuildTargets);

  AddGlobCheckExec();

  for (auto const& targetBase : FastbuildTargets) {
    this->WriteComment("Target definition: " + targetBase->Name);
    // Target start.
    *BuildFileStream << "{\n";

    if (targetBase->Type == FastbuildTargetType::EXEC) {
      this->WriteExec(static_cast<FastbuildExecNode const&>(*targetBase));
    } else if (targetBase->Type == FastbuildTargetType::ALIAS) {
      this->WriteAlias(static_cast<FastbuildAliasNode const&>(*targetBase));
      // CustomCommandByproducts test.
      FastbuildAliasNode alias;
      alias.Name = targetBase->Name + FASTBUILD_ALL_ALIAS_POSTFIX;
      alias.PreBuildDependencies.emplace(targetBase->Name);
      this->WriteAlias(alias);
    } else if (targetBase->Type == FastbuildTargetType::LINK) {
      auto const& target = static_cast<FastbuildTarget const&>(*targetBase);
      this->WriteTarget(target);
    }
    // Target end.
    *BuildFileStream << "}\n";
  }

  if (!this->GetCMakeInstance()->GetIsInTryCompile()) {
    if (!IDEProjects.empty()) {
      this->WriteIDEProjects();
    }
  }
  this->WriteTargetClean();
  this->WriteTargetRebuildBFF();
}

std::string cmGlobalFastbuildGenerator::GetTargetName(
  cmGeneratorTarget const* GeneratorTarget) const
{
  std::string targetName =
    GeneratorTarget->GetLocalGenerator()->GetCurrentBinaryDirectory();
  targetName += "/";
  targetName += GeneratorTarget->GetName();
  targetName = this->ConvertToFastbuildPath(targetName);
  return targetName;
}

cm::optional<FastbuildTarget>
cmGlobalFastbuildGenerator::GetTargetByOutputName(
  std::string const& output) const
{
  for (auto const& targetBase : FastbuildTargets) {
    if (targetBase->Type == FastbuildTargetType::LINK) {
      auto const& target = static_cast<FastbuildTarget const&>(*targetBase);
      if (std::any_of(target.LinkerNode.begin(), target.LinkerNode.end(),
                      [&output](FastbuildLinkerNode const& target_) {
                        return target_.LinkerOutput == output;
                      })) {
        return target;
      }
    }
  }
  return cm::nullopt;
}

void cmGlobalFastbuildGenerator::AddIDEProject(FastbuildTarget const& target,
                                               std::string const& config)
{
  auto const& configs = GetConfigNames();
  if (std::find(configs.begin(), configs.end(), config) == configs.end()) {
    LogMessage("Config " + config + " doesn't exist, IDE projest for " +
               target.Name + " won't be generated");
    return;
  }
  auto& IDEProject = IDEProjects[target.BaseName];
  // VS
  auto& VSProject = IDEProject.first;
  VSProject.Alias = target.BaseName + "-vcxproj";
  VSProject.ProjectOutput =
    cmStrCat("VisualStudio/Projects/", target.BaseName + ".vcxproj");
  VSProject.ProjectBasePath = target.BasePath;
  // XCode
  auto& XCodeProject = IDEProject.second;
  XCodeProject.Alias = target.BaseName + "-xcodeproj";
  XCodeProject.ProjectOutput = cmStrCat(
    "XCode/Projects/", target.BaseName + ".xcodeproj/project.pbxproj");
  XCodeProject.ProjectBasePath = target.BasePath;

  IDEProjectConfig VSConfig;
  VSConfig.Platform = "X64";
  IDEProjectConfig XCodeConfig;
  VSConfig.Target = XCodeConfig.Target = target.Name;
  VSConfig.Config = XCodeConfig.Config = config.empty() ? "DEFAULT" : config;

  VSProject.ProjectConfigs.emplace_back(std::move(VSConfig));
  XCodeProject.ProjectConfigs.emplace_back(std::move(XCodeConfig));
}

bool cmGlobalFastbuildGenerator::IsExcluded(cmGeneratorTarget* target)
{
  return cmGlobalGenerator::IsExcluded(LocalGenerators[0].get(), target);
}
std::vector<std::string> const& cmGlobalFastbuildGenerator::GetConfigNames()
  const
{
  return static_cast<cmLocalFastbuildGenerator const*>(
           this->LocalGenerators.front().get())
    ->GetConfigNames();
}

bool cmGlobalFastbuildGenerator::Open(std::string const& bindir,
                                      std::string const& projectName,
                                      bool dryRun)
{
#ifdef _WIN32
  std::string sln = bindir + "/VisualStudio/" + projectName + ".sln";

  if (dryRun) {
    return cmSystemTools::FileExists(sln, true);
  }

  sln = cmSystemTools::ConvertToOutputPath(sln);

  auto OpenSolution = [](std::string pathToSolution) {
    HRESULT comInitialized =
      CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(comInitialized)) {
      return false;
    }

    HINSTANCE hi = ShellExecuteA(NULL, "open", pathToSolution.c_str(), NULL,
                                 NULL, SW_SHOWNORMAL);

    CoUninitialize();

    return reinterpret_cast<intptr_t>(hi) > 32;
  };

  return std::async(std::launch::async, OpenSolution, sln).get();
#else
  return cmGlobalCommonGenerator::Open(bindir, projectName, dryRun);
#endif
}
