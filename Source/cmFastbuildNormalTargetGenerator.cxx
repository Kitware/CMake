/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmFastbuildNormalTargetGenerator.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"

#include "cmCommonTargetGenerator.h"
#include "cmCryptoHash.h"
#include "cmFastbuildTargetGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalCommonGenerator.h"
#include "cmGlobalFastbuildGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalFastbuildGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"
#include "cmObjectLocation.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

std::string const COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
std::string const COMPILE_OPTIONS("COMPILE_OPTIONS");
std::string const COMPILE_FLAGS("COMPILE_FLAGS");
std::string const CMAKE_LANGUAGE("CMAKE");
std::string const INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");

std::string const CMAKE_UNITY_BUILD("CMAKE_UNITY_BUILD");
std::string const CMAKE_UNITY_BUILD_BATCH_SIZE("CMAKE_UNITY_BUILD_BATCH_SIZE");
std::string const UNITY_BUILD("UNITY_BUILD");
std::string const UNITY_BUILD_BATCH_SIZE("UNITY_BUILD_BATCH_SIZE");
std::string const SKIP_UNITY_BUILD_INCLUSION("SKIP_UNITY_BUILD_INCLUSION");
std::string const UNITY_GROUP("UNITY_GROUP");

#ifdef _WIN32
char const kPATH_SLASH = '\\';
#else
char const kPATH_SLASH = '/';
#endif

} // anonymous namespace

cmFastbuildNormalTargetGenerator::cmFastbuildNormalTargetGenerator(
  cmGeneratorTarget* gt, std::string configParam)
  : cmFastbuildTargetGenerator(gt, std::move(configParam))
  , RulePlaceholderExpander(
      this->LocalCommonGenerator->CreateRulePlaceholderExpander())
  , ObjectOutDir(this->GetGlobalGenerator()->ConvertToFastbuildPath(
      this->GeneratorTarget->GetObjectDirectory(Config)))
  , Languages(GetLanguages())
  , CompileObjectCmakeRules(GetCompileObjectCommand())
  , CudaCompileMode(this->GetCudaCompileMode())
{

  LogMessage(cmStrCat("objectOutDir: ", ObjectOutDir));
  this->OSXBundleGenerator = cm::make_unique<cmOSXBundleGenerator>(gt);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);

  // Quotes to account for potential spaces.
  RulePlaceholderExpander->SetTargetImpLib(
    "\"" FASTBUILD_DOLLAR_TAG "TargetOutputImplib" FASTBUILD_DOLLAR_TAG "\"");
  for (auto const& lang : Languages) {
    TargetIncludesByLanguage[lang] = this->GetIncludes(lang, Config);
    LogMessage(cmStrCat("targetIncludes for lang ", lang, " = ",
                        TargetIncludesByLanguage[lang]));

    for (auto const& arch : this->GetArches()) {
      auto& flags = CompileFlagsByLangAndArch[std::make_pair(lang, arch)];
      this->LocalCommonGenerator->GetTargetCompileFlags(
        this->GeneratorTarget, Config, lang, flags, arch);
      LogMessage(
        cmStrCat("Lang: ", lang, ", arch: ", arch, ", flags: ", flags));
    }
  }
}

std::string cmFastbuildNormalTargetGenerator::DetectCompilerFlags(
  cmSourceFile const& srcFile, std::string const& arch)
{
  std::string const language = srcFile.GetLanguage();
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->GetLocalGenerator(), Config, this->GeneratorTarget, language);

  std::vector<std::string> sourceIncludesVec;
  if (cmValue cincludes = srcFile.GetProperty(INCLUDE_DIRECTORIES)) {
    this->LocalGenerator->AppendIncludeDirectories(
      sourceIncludesVec,
      genexInterpreter.Evaluate(*cincludes, INCLUDE_DIRECTORIES), srcFile);
  }
  std::string sourceIncludesStr = this->LocalGenerator->GetIncludeFlags(
    sourceIncludesVec, this->GeneratorTarget, language, Config, false);
  LogMessage(cmStrCat("sourceIncludes = ", sourceIncludesStr));

  std::string compileFlags =
    CompileFlagsByLangAndArch[std::make_pair(language, arch)];
  this->GeneratorTarget->AddExplicitLanguageFlags(compileFlags, srcFile);

  if (cmValue const cflags = srcFile.GetProperty(COMPILE_FLAGS)) {
    this->LocalGenerator->AppendFlags(
      compileFlags, genexInterpreter.Evaluate(*cflags, COMPILE_FLAGS));
  }

  if (cmValue const coptions = srcFile.GetProperty(COMPILE_OPTIONS)) {
    this->LocalGenerator->AppendCompileOptions(
      compileFlags, genexInterpreter.Evaluate(*coptions, COMPILE_OPTIONS));
  }
  // Source includes take precedence over target includes.
  this->LocalGenerator->AppendFlags(compileFlags, sourceIncludesStr);
  this->LocalGenerator->AppendFlags(compileFlags,
                                    TargetIncludesByLanguage[language]);

  if (language == "Fortran") {
    this->AppendFortranFormatFlags(compileFlags, srcFile);
    this->AppendFortranPreprocessFlags(compileFlags, srcFile);
  }

  LogMessage(cmStrCat("compileFlags = ", compileFlags));
  return compileFlags;
}

void cmFastbuildNormalTargetGenerator::SplitLinkerFromArgs(
  std::string const& command, std::string& outLinkerExecutable,
  std::string& outLinkerArgs) const
{
#ifdef _WIN32
  std::vector<std::string> args;
  std::string tmp;
  cmSystemTools::SplitProgramFromArgs(command, tmp, outLinkerArgs);
  // cmLocalGenerator::GetStaticLibraryFlags seems to add empty quotes when
  // appending "STATIC_LIBRARY_FLAGS_DEBUG"...
  cmSystemTools::ReplaceString(outLinkerArgs, "\"\"", "");
  cmSystemTools::ParseWindowsCommandLine(command.c_str(), args);
  outLinkerExecutable = std::move(args[0]);
#else
  cmSystemTools::SplitProgramFromArgs(command, outLinkerExecutable,
                                      outLinkerArgs);
#endif
}

void cmFastbuildNormalTargetGenerator::GetLinkerExecutableAndArgs(
  std::string const& command, std::string& outLinkerExecutable,
  std::string& outLinkerArgs)
{
  if (command.empty()) {
    return;
  }

  LogMessage("Link Command: " + command);

  auto const& compilers = this->GetGlobalGenerator()->Compilers;
  auto const linkerLauncherVarName = FASTBUILD_LINKER_LAUNCHER_PREFIX +
    this->GeneratorTarget->GetLinkerLanguage(Config);
  auto const iter = compilers.find(linkerLauncherVarName);
  // Tested in "RunCMake.LinkerLauncher" test.
  if (iter != compilers.end()) {
    LogMessage("Linker launcher: " + iter->first);
    outLinkerExecutable = iter->second.Executable;
    outLinkerArgs = cmStrCat(iter->second.Args, ' ', command);
  } else {
    SplitLinkerFromArgs(command, outLinkerExecutable, outLinkerArgs);
  }
  LogMessage("Linker Exe: " + outLinkerExecutable);
  LogMessage("Linker args: " + outLinkerArgs);
}

bool cmFastbuildNormalTargetGenerator::DetectBaseLinkerCommand(
  std::string& command, std::string const& arch,
  cmGeneratorTarget::Names const& targetNames)
{
  std::string const linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(Config);
  if (linkLanguage.empty()) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: " +
                         this->GeneratorTarget->GetName());
    return false;
  }
  LogMessage("linkLanguage: " + linkLanguage);

  std::string linkLibs;
  std::string targetFlags;
  std::string linkFlags;
  std::string frameworkPath;
  // Tested in "RunCMake.StandardLinkDirectories" test.
  std::string linkPath;

  std::unique_ptr<cmLinkLineComputer> const linkLineComputer =
    this->GetGlobalGenerator()->CreateLinkLineComputer(
      this->LocalGenerator,
      this->GetLocalGenerator()->GetStateSnapshot().GetDirectory());

  this->LocalCommonGenerator->GetTargetFlags(
    linkLineComputer.get(), Config, linkLibs, targetFlags, linkFlags,
    frameworkPath, linkPath, this->GeneratorTarget);

  // cmLocalGenerator::GetStaticLibraryFlags seems to add empty quotes when
  // appending "STATIC_LIBRARY_FLAGS_DEBUG"...
  cmSystemTools::ReplaceString(linkFlags, "\"\"", "");
  LogMessage("linkLibs: " + linkLibs);
  LogMessage("targetFlags: " + targetFlags);
  LogMessage("linkFlags: " + linkFlags);
  LogMessage("frameworkPath: " + frameworkPath);
  LogMessage("linkPath: " + linkPath);

  LogMessage("MANIFESTS: " + this->GetManifests(Config));

  cmComputeLinkInformation* linkInfo =
    this->GeneratorTarget->GetLinkInformation(Config);
  if (!linkInfo) {
    return false;
  }

  // Tested in "RunCMake.RuntimePath" test.
  std::string const rpath = linkLineComputer->ComputeRPath(*linkInfo);
  LogMessage("RPath: " + rpath);

  if (!linkFlags.empty()) {
    linkFlags += " ";
  }
  linkFlags += cmJoin({ rpath, frameworkPath, linkPath }, " ");

  cmStateEnums::TargetType const targetType = this->GeneratorTarget->GetType();
  // Add OS X version flags, if any.
  if (targetType == cmStateEnums::SHARED_LIBRARY ||
      targetType == cmStateEnums::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "COMPATIBILITY", true);
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "CURRENT", false);
  }
  // Add Arch flags to link flags for binaries
  if (targetType == cmStateEnums::SHARED_LIBRARY ||
      targetType == cmStateEnums::MODULE_LIBRARY ||
      targetType == cmStateEnums::EXECUTABLE) {
    this->LocalCommonGenerator->AddArchitectureFlags(
      linkFlags, this->GeneratorTarget, linkLanguage, Config, arch);
    this->UseLWYU = this->GetLocalGenerator()->AppendLWYUFlags(
      linkFlags, this->GetGeneratorTarget(), linkLanguage);
  }

  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GeneratorTarget->GetName().c_str();
  vars.CMTargetType = cmState::GetTargetTypeName(targetType).c_str();
  vars.Config = Config.c_str();
  vars.Language = linkLanguage.c_str();
  std::string const manifests =
    cmJoin(this->GetManifestsAsFastbuildPath(), " ");
  vars.Manifests = manifests.c_str();

  std::string const stdLibString = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", linkLanguage, "_STANDARD_LIBRARIES"));

  LogMessage(cmStrCat("Target type: ", this->GeneratorTarget->GetType()));
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE ||
      this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    vars.Objects = FASTBUILD_1_0_INPUT_PLACEHOLDER;
    vars.LinkLibraries = stdLibString.c_str();
  } else {
    vars.Objects = FASTBUILD_1_INPUT_PLACEHOLDER;
  }

  vars.ObjectDir = FASTBUILD_DOLLAR_TAG "TargetOutDir" FASTBUILD_DOLLAR_TAG;
  vars.Target = FASTBUILD_2_INPUT_PLACEHOLDER;

  std::string install_dir;
  std::string target_so_name;
  if (this->GeneratorTarget->HasSOName(Config)) {
    vars.SONameFlag = this->Makefile->GetSONameFlag(
      this->GeneratorTarget->GetLinkerLanguage(Config));
    target_so_name =
      cmGlobalFastbuildGenerator::QuoteIfHasSpaces(targetNames.SharedObject);
    vars.TargetSOName = target_so_name.c_str();
    // Tested in "RunCMake.RuntimePath / RunCMake.INSTALL_NAME_DIR"
    // tests.
    install_dir = this->LocalGenerator->ConvertToOutputFormat(
      this->GeneratorTarget->GetInstallNameDirForBuildTree(Config),
      cmOutputConverter::SHELL);
    vars.TargetInstallNameDir = install_dir.c_str();
  } else {
    vars.TargetSOName = "";
  }
  vars.TargetPDB = FASTBUILD_DOLLAR_TAG "LinkerPDB" FASTBUILD_DOLLAR_TAG;

  // Setup the target version.
  std::string targetVersionMajor;
  std::string targetVersionMinor;
  {
    std::ostringstream majorStream;
    std::ostringstream minorStream;
    int major;
    int minor;
    this->GeneratorTarget->GetTargetVersion(major, minor);
    majorStream << major;
    minorStream << minor;
    targetVersionMajor = majorStream.str();
    targetVersionMinor = minorStream.str();
  }
  vars.TargetVersionMajor = targetVersionMajor.c_str();
  vars.TargetVersionMinor = targetVersionMinor.c_str();

  vars.Defines =
    FASTBUILD_DOLLAR_TAG "CompileDefineFlags" FASTBUILD_DOLLAR_TAG;
  vars.Flags = targetFlags.c_str();
  vars.LinkFlags = linkFlags.c_str();
  vars.LanguageCompileFlags = "";
  std::string const linker = this->GeneratorTarget->GetLinkerTool(Config);
  vars.Linker = linker.c_str();
  std::string const targetSupportPath = this->ConvertToFastbuildPath(
    this->GetGeneratorTarget()->GetCMFSupportDirectory());
  vars.TargetSupportDir = targetSupportPath.c_str();

  LogMessage("linkFlags: " + linkFlags);
  LogMessage("linker: " + linker);

  std::string linkRule = GetLinkCommand();
  ApplyLinkRuleLauncher(linkRule);
  RulePlaceholderExpander->ExpandRuleVariables(
    dynamic_cast<cmLocalFastbuildGenerator*>(this->LocalCommonGenerator),
    linkRule, vars);

  command = std::move(linkRule);
  LogMessage(cmStrCat("Expanded link command: ", command));
  return true;
}

void cmFastbuildNormalTargetGenerator::ApplyLinkRuleLauncher(
  std::string& command)
{
  std::string const val = this->GetLocalGenerator()->GetRuleLauncher(
    this->GetGeneratorTarget(), "RULE_LAUNCH_LINK", Config);
  if (cmNonempty(val)) {
    LogMessage("RULE_LAUNCH_LINK: " + val);
    command = cmStrCat(val, ' ', command);
  }
}

void cmFastbuildNormalTargetGenerator::ApplyLWYUToLinkerCommand(
  FastbuildLinkerNode& linkerNode)
{
  cmValue const lwyuCheck =
    this->Makefile->GetDefinition("CMAKE_LINK_WHAT_YOU_USE_CHECK");
  if (this->UseLWYU && lwyuCheck) {
    LogMessage("UseLWYU=true");
    std::string args = " -E __run_co_compile --lwyu=";
    args += this->GetLocalGenerator()->EscapeForShell(*lwyuCheck);

    args += cmStrCat(
      " --source=",
      this->ConvertToFastbuildPath(this->GetGeneratorTarget()->GetFullPath(
        Config, cmStateEnums::RuntimeBinaryArtifact,
        /*realname=*/true)));

    LogMessage("LWUY args: " + args);
    linkerNode.LinkerStampExe = cmSystemTools::GetCMakeCommand();
    linkerNode.LinkerStampExeArgs = std::move(args);
  }
}

std::string cmFastbuildNormalTargetGenerator::ComputeDefines(
  cmSourceFile const& srcFile)
{
  std::string const language = srcFile.GetLanguage();
  std::set<std::string> defines;
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->GetLocalGenerator(), Config, this->GeneratorTarget, language);

  if (auto compile_defs = srcFile.GetProperty(COMPILE_DEFINITIONS)) {
    this->GetLocalGenerator()->AppendDefines(
      defines, genexInterpreter.Evaluate(*compile_defs, COMPILE_DEFINITIONS));
  }

  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(Config);
  if (auto config_compile_defs = srcFile.GetProperty(defPropName)) {
    this->GetLocalGenerator()->AppendDefines(
      defines,
      genexInterpreter.Evaluate(*config_compile_defs, COMPILE_DEFINITIONS));
  }

  std::string definesString = this->GetDefines(language, Config);
  LogMessage(cmStrCat("TARGET DEFINES = ", definesString));
  this->GetLocalGenerator()->JoinDefines(defines, definesString, language);

  LogMessage(cmStrCat("DEFINES = ", definesString));
  return definesString;
}

void cmFastbuildNormalTargetGenerator::ComputePCH(
  cmSourceFile const& srcFile, FastbuildObjectListNode& node,
  std::set<std::string>& createdPCH)
{
  if (srcFile.GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    return;
  }
  // We have already computed PCH for this node.
  if (!node.PCHOptions.empty() || !node.PCHInputFile.empty() ||
      !node.PCHOutputFile.empty()) {
    return;
  }
  std::string const language = srcFile.GetLanguage();
  cmGeneratorExpressionInterpreter genexInterpreter(
    this->GetLocalGenerator(), Config, this->GeneratorTarget, language);

  //.cxx
  std::string const pchSource =
    this->GeneratorTarget->GetPchSource(Config, language);
  //.hxx
  std::string const pchHeader =
    this->GeneratorTarget->GetPchHeader(Config, language);
  //.pch
  std::string const pchFile =
    this->GeneratorTarget->GetPchFile(Config, language);

  if (pchHeader.empty() || pchFile.empty()) {
    return;
  }
  // In "RunCMake.GenEx-TARGET_PROPERTY" test we call set
  // CMAKE_PCH_EXTENSION="", so pchHeader becomes same as pchFile...
  if (pchHeader == pchFile) {
    LogMessage("pchHeader == pchFile > skipping");
    LogMessage("pchHeader: " + pchHeader);
    LogMessage("pchFile: " + pchFile);
    return;
  }

  node.PCHOutputFile =
    this->GetGlobalGenerator()->ConvertToFastbuildPath(pchFile);
  // Tell the ObjectList how to use PCH.
  std::string const pchUseOption =
    this->GeneratorTarget->GetPchUseCompileOptions(Config, language);
  LogMessage(cmStrCat("pchUseOption: ", pchUseOption));

  std::string origCompileOptions = node.CompilerOptions;
  for (auto const& opt :
       cmList{ genexInterpreter.Evaluate(pchUseOption, COMPILE_OPTIONS) }) {
    node.CompilerOptions += " ";
    node.CompilerOptions += opt;
  }

  if (!createdPCH.emplace(node.PCHOutputFile).second) {
    LogMessage(node.PCHOutputFile + " is already created by this target");
    return;
  }

  // Short circuit if the PCH has already been created by another target.
  if (!this->GeneratorTarget->GetSafeProperty("PRECOMPILE_HEADERS_REUSE_FROM")
         .empty()) {
    LogMessage(cmStrCat("PCH: ", node.PCHOutputFile,
                        " already created by another target"));
    return;
  }

  node.PCHInputFile =
    this->GetGlobalGenerator()->ConvertToFastbuildPath(pchSource);

  std::string const pchCreateOptions =
    this->GeneratorTarget->GetPchCreateCompileOptions(Config, language);
  LogMessage(cmStrCat("pchCreateOptions: ", pchCreateOptions));
  char const* sep = "";
  for (auto const& opt : cmList{
         genexInterpreter.Evaluate(pchCreateOptions, COMPILE_OPTIONS) }) {
    node.PCHOptions += sep;
    node.PCHOptions += opt;
    sep = " ";
  }

  // Reuse compiler options for PCH options.
  node.PCHOptions += origCompileOptions;
  if (this->Makefile->GetSafeDefinition("CMAKE_" + language +
                                        "_COMPILER_ID") == "MSVC") {
    cmSystemTools::ReplaceString(node.PCHOptions,
                                 FASTBUILD_2_INPUT_PLACEHOLDER,
                                 FASTBUILD_3_INPUT_PLACEHOLDER);
  }

  LogMessage("PCH Source: " + pchSource);
  LogMessage("node.PCHInputFile: " + node.PCHInputFile);
  LogMessage("node.PCHOutputFile: " + node.PCHOutputFile);
  LogMessage("node.PCHOptions: " + node.PCHOptions);
  LogMessage("node.CompilerOptions: " + node.CompilerOptions);
}

void cmFastbuildNormalTargetGenerator::EnsureDirectoryExists(
  std::string const& path) const
{
  if (cmSystemTools::FileIsFullPath(path.c_str())) {
    cmSystemTools::MakeDirectory(path.c_str());
  } else {
    auto* gg = this->GetGlobalGenerator();
    std::string fullPath = gg->GetCMakeInstance()->GetHomeOutputDirectory();
    // Also ensures there is a trailing slash.
    fullPath += path;
    cmSystemTools::MakeDirectory(fullPath);
  }
}

void cmFastbuildNormalTargetGenerator::EnsureParentDirectoryExists(
  std::string const& path) const
{
  this->EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path));
}

std::vector<std::string>
cmFastbuildNormalTargetGenerator::GetManifestsAsFastbuildPath() const
{
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, Config);
  std::vector<std::string> manifests;
  manifests.reserve(manifest_srcs.size());
  for (auto& manifest_src : manifest_srcs) {
    std::string str = this->ConvertToFastbuildPath(
      cmSystemTools::ConvertToOutputPath(manifest_src->GetFullPath()));
    LogMessage("Manifest: " + str);
    manifests.emplace_back(std::move(str));
  }

  return manifests;
}

void cmFastbuildNormalTargetGenerator::GenerateModuleDefinitionInfo(
  FastbuildTarget& target) const
{
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    GeneratorTarget->GetModuleDefinitionInfo(Config);
  if (mdi && mdi->DefFileGenerated) {
    FastbuildExecNode execNode;
    execNode.Name = target.Name + "-def-files";
    execNode.ExecExecutable = cmSystemTools::GetCMakeCommand();
    execNode.ExecArguments =
      cmStrCat("-E __create_def ", FASTBUILD_2_INPUT_PLACEHOLDER, ' ',
               FASTBUILD_1_INPUT_PLACEHOLDER);
    std::string const obj_list_file = mdi->DefFile + ".objs";

    auto const nm_executable = GetMakefile()->GetDefinition("CMAKE_NM");
    if (!nm_executable.IsEmpty()) {
      execNode.ExecArguments += " --nm=";
      execNode.ExecArguments += ConvertToFastbuildPath(*nm_executable);
    }
    execNode.ExecOutput = ConvertToFastbuildPath(mdi->DefFile);
    execNode.ExecInput.push_back(ConvertToFastbuildPath(obj_list_file));

    // RunCMake.AutoExportDll
    for (auto const& objList : target.ObjectListNodes) {
      execNode.PreBuildDependencies.emplace(objList.Name);
    }
    // Tested in "RunCMake.AutoExportDll" / "ModuleDefinition" tests.
    for (auto& linkerNode : target.LinkerNode) {
      linkerNode.Libraries2.emplace_back(execNode.Name);
    }

    target.PreLinkExecNodes.Nodes.emplace_back(std::move(execNode));

    // create a list of obj files for the -E __create_def to read
    cmGeneratedFileStream fout(obj_list_file);
    // Since we generate this file once during configuration, we should not
    // remove it when "clean" is built.
    // Tested in "RunCMake.AutoExportDll" / "ModuleDefinition" tests.
    this->GetGlobalGenerator()->AllFilesToKeep.insert(obj_list_file);

    if (mdi->WindowsExportAllSymbols) {
      std::vector<cmSourceFile const*> objectSources;
      GeneratorTarget->GetObjectSources(objectSources, Config);
      std::map<cmSourceFile const*, cmObjectLocations> mapping;
      for (cmSourceFile const* it : objectSources) {
        mapping[it];
      }
      GeneratorTarget->LocalGenerator->ComputeObjectFilenames(mapping, Config,
                                                              GeneratorTarget);

      std::vector<std::string> objs;
      for (cmSourceFile const* it : objectSources) {
        auto const& v = mapping[it];
        LogMessage("Obj source : " + v.LongLoc.GetPath());
        std::string objFile = this->ConvertToFastbuildPath(
          GeneratorTarget->GetObjectDirectory(Config) + v.LongLoc.GetPath());
        objFile = cmSystemTools::ConvertToOutputPath(objFile);
        LogMessage("objFile path: " + objFile);
        objs.push_back(objFile);
      }

      std::vector<cmSourceFile const*> externalObjectSources;
      GeneratorTarget->GetExternalObjects(externalObjectSources, Config);
      for (cmSourceFile const* it : externalObjectSources) {
        objs.push_back(cmSystemTools::ConvertToOutputPath(
          this->ConvertToFastbuildPath(it->GetFullPath())));
      }

      for (std::string const& objFile : objs) {
        if (cmHasLiteralSuffix(objFile, ".obj")) {
          fout << objFile << "\n";
        }
      }
    }
    for (cmSourceFile const* src : mdi->Sources) {
      fout << src->GetFullPath() << "\n";
    }
  }
}

void cmFastbuildNormalTargetGenerator::AddPrebuildDeps(
  FastbuildTarget& target) const
{
  // All ObjectLists should wait for PRE_BUILD.
  for (FastbuildObjectListNode& node : target.ObjectListNodes) {
    if (!target.PreBuildExecNodes.Name.empty()) {
      node.PreBuildDependencies.emplace(target.PreBuildExecNodes.Name);
    }
    if (!target.ExecNodes.Name.empty()) {
      node.PreBuildDependencies.emplace(target.ExecNodes.Name);
    }
  }
  for (auto& linkerNode : target.LinkerNode) {
    // Wait for 'PRE_BUILD' custom commands.
    if (!target.PreBuildExecNodes.Name.empty()) {
      linkerNode.PreBuildDependencies.emplace(target.PreBuildExecNodes.Name);
    }

    // Wait for regular custom commands.
    if (!target.ExecNodes.Name.empty()) {
      linkerNode.PreBuildDependencies.emplace(target.ExecNodes.Name);
    }
    // All targets that we depend on must be prebuilt.
    if (!target.DependenciesAlias.PreBuildDependencies.empty()) {
      linkerNode.PreBuildDependencies.emplace(target.DependenciesAlias.Name);
    }
  }
}

std::set<std::string> cmFastbuildNormalTargetGenerator::GetLanguages()
{
  std::set<std::string> result;
  this->GetGeneratorTarget()->GetLanguages(result, Config);
  for (std::string const& lang : result) {
    this->GetGlobalGenerator()->AddCompiler(lang, this->GetMakefile());
  }
  LogMessage("Languages: " + cmJoin(result, ", "));
  return result;
}

std::unordered_map<std::string, std::string>
cmFastbuildNormalTargetGenerator::GetCompileObjectCommand() const
{
  std::unordered_map<std::string, std::string> result;
  result.reserve(Languages.size());
  for (std::string const& lang : Languages) {
    std::vector<std::string> commands;
    std::string cmakeVar;
    cmakeVar = "CMAKE_";
    cmakeVar += lang;
    cmakeVar += "_COMPILE_OBJECT";

    std::string cmakeValue =
      LocalCommonGenerator->GetMakefile()->GetSafeDefinition(cmakeVar);

    LogMessage(cmakeVar.append(" = ").append(cmakeValue));

    result[lang] = std::move(cmakeValue);
  }
  return result;
}
std::string cmFastbuildNormalTargetGenerator::GetCudaCompileMode() const
{
  if (Languages.find("CUDA") == Languages.end()) {
    return {};
  }
  // TODO: unify it with makefile / ninja generators.
  std::string cudaCompileMode;
  if (this->GeneratorTarget->GetPropertyAsBool("CUDA_SEPARABLE_COMPILATION")) {
    std::string const& rdcFlag =
      this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_RDC_FLAG");
    cudaCompileMode = cmStrCat(cudaCompileMode, rdcFlag, ' ');
  }
  static std::array<cm::string_view, 4> const compileModes{
    { "PTX"_s, "CUBIN"_s, "FATBIN"_s, "OPTIX"_s }
  };
  bool useNormalCompileMode = true;
  for (cm::string_view mode : compileModes) {
    auto propName = cmStrCat("CUDA_", mode, "_COMPILATION");
    auto defName = cmStrCat("_CMAKE_CUDA_", mode, "_FLAG");
    if (this->GeneratorTarget->GetPropertyAsBool(propName)) {
      std::string const& flag = this->Makefile->GetRequiredDefinition(defName);
      cudaCompileMode = cmStrCat(cudaCompileMode, flag);
      useNormalCompileMode = false;
      break;
    }
  }
  if (useNormalCompileMode) {
    std::string const& wholeFlag =
      this->Makefile->GetRequiredDefinition("_CMAKE_CUDA_WHOLE_FLAG");
    cudaCompileMode = cmStrCat(cudaCompileMode, wholeFlag);
  }
  return cudaCompileMode;
}

std::string cmFastbuildNormalTargetGenerator::GetLinkCommand() const
{
  std::string const& linkLanguage = GeneratorTarget->GetLinkerLanguage(Config);
  std::string linkCmdVar =
    GeneratorTarget->GetCreateRuleVariable(linkLanguage, Config);
  std::string res = this->Makefile->GetSafeDefinition(linkCmdVar);
  if (res.empty() &&
      this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    linkCmdVar = linkCmdVar =
      cmStrCat("CMAKE_", linkLanguage, "_ARCHIVE_CREATE");
    res = this->Makefile->GetSafeDefinition(linkCmdVar);
  }
  LogMessage("Link rule: " + cmStrCat(linkCmdVar, " = ", res));
  return res;
}

void cmFastbuildNormalTargetGenerator::AddCompilerLaunchersForLanguages()
{
  // General rule for all languages.
  std::string const launchCompile = this->GetLocalGenerator()->GetRuleLauncher(
    this->GetGeneratorTarget(), "RULE_LAUNCH_COMPILE", Config);
  // See if we need to use a compiler launcher like ccache or distcc
  for (std::string const& language : Languages) {
    std::string const compilerLauncher =
      cmCommonTargetGenerator::GetCompilerLauncher(language, Config);
    LogMessage("compilerLauncher: " + compilerLauncher);
    std::vector<std::string> expanded;
    cmExpandList(compilerLauncher, expanded);

    if (!expanded.empty()) {
      std::string const exe = expanded[0];
      expanded.erase(expanded.begin());
      this->GetGlobalGenerator()->AddLauncher(FASTBUILD_LAUNCHER_PREFIX, exe,
                                              language, cmJoin(expanded, " "));
    } else if (!launchCompile.empty()) {
      std::string exe;
      std::string args;
      cmSystemTools::SplitProgramFromArgs(launchCompile, exe, args);
      this->GetGlobalGenerator()->AddLauncher(FASTBUILD_LAUNCHER_PREFIX, exe,
                                              language, args);
    }
  }
}
void cmFastbuildNormalTargetGenerator::AddLinkerLauncher()
{
  std::string const linkerLauncher =
    cmCommonTargetGenerator::GetLinkerLauncher(Config);
  std::vector<std::string> args;
#ifdef _WIN32
  cmSystemTools::ParseWindowsCommandLine(linkerLauncher.c_str(), args);
#else
  cmSystemTools::ParseUnixCommandLine(linkerLauncher.c_str(), args);
#endif
  if (!args.empty()) {
    std::string const exe = std::move(args[0]);
    args.erase(args.begin());
    this->GetGlobalGenerator()->AddLauncher(
      FASTBUILD_LINKER_LAUNCHER_PREFIX, exe,
      this->GeneratorTarget->GetLinkerLanguage(Config), cmJoin(args, " "));
  }
}
void cmFastbuildNormalTargetGenerator::AddCMakeLauncher()
{
  // Add CMake launcher (might be used for static analysis).
  this->GetGlobalGenerator()->AddLauncher(FASTBUILD_LAUNCHER_PREFIX,
                                          cmSystemTools::GetCMakeCommand(),
                                          CMAKE_LANGUAGE, "");
}

void cmFastbuildNormalTargetGenerator::ComputePaths(
  FastbuildTarget& target) const
{
  std::string const objPath = GetGeneratorTarget()->GetSupportDirectory();
  EnsureDirectoryExists(objPath);
  target.Variables["TargetOutDir"] =
    cmSystemTools::ConvertToOutputPath(this->ConvertToFastbuildPath(objPath));

  if (GeneratorTarget->GetType() <= cmStateEnums::MODULE_LIBRARY) {
    std::string const pdbDir = GeneratorTarget->GetPDBDirectory(Config);
    LogMessage("GetPDBDirectory: " + pdbDir);
    EnsureDirectoryExists(pdbDir);
    std::string const linkerPDB =
      cmStrCat(pdbDir, '/', this->GeneratorTarget->GetPDBName(Config));

    if (!linkerPDB.empty()) {
      target.Variables["LinkerPDB"] = cmSystemTools::ConvertToOutputPath(
        this->ConvertToFastbuildPath(linkerPDB));
    }
  }
  std::string const compilerPDB = this->ComputeTargetCompilePDB(this->Config);
  if (!compilerPDB.empty()) {
    LogMessage("ComputeTargetCompilePDB: " + compilerPDB);
    std::string compilerPDBArg = cmSystemTools::ConvertToOutputPath(
      this->ConvertToFastbuildPath(compilerPDB));
    if (cmHasSuffix(compilerPDB, '/')) {
      // The compiler will choose the .pdb file name.
      this->EnsureDirectoryExists(compilerPDB);
      // ConvertToFastbuildPath dropped the trailing slash.  Add it back.
      // We do this after ConvertToOutputPath so that we can use a forward
      // slash in the case that the argument is quoted.
      if (cmHasSuffix(compilerPDBArg, '"')) {
        // A quoted trailing backslash requires escaping, e.g., `/Fd"dir\\"`,
        // but fbuild does not parse such arguments correctly as of 1.15.
        // Always use a forward slash.
        compilerPDBArg.insert(compilerPDBArg.size() - 1, 1, '/');
      } else {
        // An unquoted trailing slash or backslash is fine.
        compilerPDBArg.push_back(kPATH_SLASH);
      }
    } else {
      // We have an explicit .pdb path with file name.
      this->EnsureParentDirectoryExists(compilerPDB);
    }
    target.Variables["CompilerPDB"] = std::move(compilerPDBArg);
  }
  std::string const impLibFullPath =
    GeneratorTarget->GetFullPath(Config, cmStateEnums::ImportLibraryArtifact);
  std::string impLibFile = ConvertToFastbuildPath(impLibFullPath);
  cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(impLibFullPath));
  if (!impLibFile.empty()) {
    cmSystemTools::ConvertToOutputSlashes(impLibFile);
    target.Variables["TargetOutputImplib"] = std::move(impLibFile);
  }
}

void cmFastbuildNormalTargetGenerator::Generate()
{
  this->GeneratorTarget->CheckCxxModuleStatus(Config);

  FastbuildTarget fastbuildTarget;
  auto const addUtilDepToTarget = [&fastbuildTarget](std::string depName) {
    FastbuildTargetDep dep{ depName };
    dep.Type = FastbuildTargetDepType::UTIL;
    fastbuildTarget.PreBuildDependencies.emplace(std::move(dep));
  };

  fastbuildTarget.Name = GetTargetName();
  fastbuildTarget.BaseName = this->GeneratorTarget->GetName();

  LogMessage("<-------------->");
  LogMessage("Generate target: " + fastbuildTarget.Name);
  LogMessage("Config: " + Config);

  LogMessage("Deps: ");
  for (cmTargetDepend const& dep : TargetDirectDependencies) {
    auto const tname = dep->GetName();
    LogMessage(tname);
    FastbuildTargetDep targetDep{ tname };
    if (dep->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      targetDep.Type = FastbuildTargetDepType::ORDER_ONLY;
    }
    fastbuildTarget.PreBuildDependencies.emplace(std::move(targetDep));
  }

  ComputePaths(fastbuildTarget);
  AddCompilerLaunchersForLanguages();
  AddLinkerLauncher();
  AddCMakeLauncher();

  for (auto& cc : GenerateCommands(FastbuildBuildStep::PRE_BUILD).Nodes) {
    fastbuildTarget.PreBuildExecNodes.PreBuildDependencies.emplace(cc.Name);
    addUtilDepToTarget(cc.Name);
    this->GetGlobalGenerator()->AddTarget(std::move(cc));
  }
  for (auto& cc : GenerateCommands(FastbuildBuildStep::PRE_LINK).Nodes) {
    cc.PreBuildDependencies.emplace(fastbuildTarget.Name +
                                    FASTBUILD_DEPS_ARTIFACTS_ALIAS_POSTFIX);
    fastbuildTarget.PreLinkExecNodes.Nodes.emplace_back(std::move(cc));
  }
  for (auto& cc : GenerateCommands(FastbuildBuildStep::REST).Nodes) {
    fastbuildTarget.ExecNodes.PreBuildDependencies.emplace(cc.Name);
    this->GetGlobalGenerator()->AddTarget(std::move(cc));
  }
  for (auto& cc : GenerateCommands(FastbuildBuildStep::POST_BUILD).Nodes) {
    fastbuildTarget.PostBuildExecNodes.Alias.PreBuildDependencies.emplace(
      cc.Name);
    fastbuildTarget.PostBuildExecNodes.Nodes.emplace_back(std::move(cc));
  }

  GenerateObjects(fastbuildTarget);

  std::vector<std::string> objectDepends;
  AddObjectDependencies(fastbuildTarget, objectDepends);

  GenerateCudaDeviceLink(fastbuildTarget);

  GenerateLink(fastbuildTarget, objectDepends);

  if (fastbuildTarget.LinkerNode.size() > 1) {
    if (!this->GeneratorTarget->IsApple()) {
      cmSystemTools::Error(
        "Can't handle more than 1 arch on non-Apple target");
      return;
    }
    AddLipoCommand(fastbuildTarget);
  }
  fastbuildTarget.CopyNodes = std::move(this->CopyNodes);

  // Generate symlink commands if real output name differs from "expected".
  for (auto& symlink : GetSymlinkExecs()) {
    fastbuildTarget.PostBuildExecNodes.Alias.PreBuildDependencies.emplace(
      symlink.Name);
    fastbuildTarget.PostBuildExecNodes.Nodes.emplace_back(std::move(symlink));
  }
  {
    auto appleTextStubCommand = GetAppleTextStubCommand();
    if (!appleTextStubCommand.Name.empty()) {
      fastbuildTarget.PostBuildExecNodes.Alias.PreBuildDependencies.emplace(
        appleTextStubCommand.Name);
      fastbuildTarget.PostBuildExecNodes.Nodes.emplace_back(
        std::move(appleTextStubCommand));
    }
  }

  AddPrebuildDeps(fastbuildTarget);

  fastbuildTarget.IsGlobal =
    GeneratorTarget->GetType() == cmStateEnums::GLOBAL_TARGET;
  fastbuildTarget.ExcludeFromAll =
    this->GetGlobalGenerator()->IsExcluded(GeneratorTarget);
  if (GeneratorTarget->GetPropertyAsBool("DONT_DISTRIBUTE")) {
    fastbuildTarget.AllowDistribution = false;
  }

  GenerateModuleDefinitionInfo(fastbuildTarget);
  // Needs to be called after we've added all PRE-LINK steps (like creation of
  // .def files on Windows).
  AddLinkerNodeDependencies(fastbuildTarget);

  // Must be called after "GenerateObjects", since it also adds Prebuild deps
  // to it.
  // Also after "GenerateModuleDefinitionInfo", since uses PreLinkExecNodes.

  fastbuildTarget.GenerateAliases();
  if (!fastbuildTarget.ExecNodes.PreBuildDependencies.empty()) {
    fastbuildTarget.DependenciesAlias.PreBuildDependencies.emplace(
      fastbuildTarget.ExecNodes.Name);
  }

  fastbuildTarget.Hidden = false;

  fastbuildTarget.BasePath = this->GetMakefile()->GetCurrentSourceDirectory();

  this->GetGlobalGenerator()->AddIDEProject(fastbuildTarget, Config);

  AddStampExeIfApplicable(fastbuildTarget);

  // size 1 means that it's not a multi-arch lib (which can only be the case on
  // Darwin).
  if (fastbuildTarget.LinkerNode.size() == 1 &&
      fastbuildTarget.LinkerNode[0].Type ==
        FastbuildLinkerNode::STATIC_LIBRARY &&
      !fastbuildTarget.PostBuildExecNodes.Nodes.empty()) {
    ProcessPostBuildForStaticLib(fastbuildTarget);
  }

  AdditionalCleanFiles();

  if (!fastbuildTarget.DependenciesAlias.PreBuildDependencies.empty()) {
    for (FastbuildObjectListNode& objListNode :
         fastbuildTarget.ObjectListNodes) {
      objListNode.PreBuildDependencies.emplace(
        fastbuildTarget.DependenciesAlias.Name);
    }
    for (auto& linkerNode : fastbuildTarget.LinkerNode) {
      linkerNode.PreBuildDependencies.emplace(
        fastbuildTarget.DependenciesAlias.Name);
    }
  }

  this->GetGlobalGenerator()->AddTarget(std::move(fastbuildTarget));
}

void cmFastbuildNormalTargetGenerator::ProcessManifests(
  FastbuildLinkerNode& linkerNode) const
{
  if (this->GetGlobalGenerator()->GetCMakeInstance()->GetIsInTryCompile()) {
    return;
  }
  auto manifests = this->GetManifestsAsFastbuildPath();
  if (manifests.empty()) {
    return;
  }
  // Manifests should always be in .Libraries2, so we re-link when needed.
  // Tested in RunCMake.BuildDepends
  for (auto const& manifest : manifests) {
    linkerNode.Libraries2.emplace_back(manifest);
  }

  if (this->Makefile->GetSafeDefinition("CMAKE_C_COMPILER_ID") != "MSVC") {
    return;
  }

  for (auto const& manifest : manifests) {
    linkerNode.LinkerOptions =
      cmStrCat("/MANIFESTINPUT:", manifest, ' ', linkerNode.LinkerOptions);
  }
  // /MANIFESTINPUT only works with /MANIFEST:EMBED
  linkerNode.LinkerOptions =
    cmStrCat("/MANIFEST:EMBED ", linkerNode.LinkerOptions);
}

void cmFastbuildNormalTargetGenerator::AddStampExeIfApplicable(
  FastbuildTarget& fastbuildTarget) const
{
  LogMessage("AddStampExeIfApplicable(...)");
  if (fastbuildTarget.LinkerNode.empty() ||
      (fastbuildTarget.LinkerNode[0].Type != FastbuildLinkerNode::EXECUTABLE &&
       fastbuildTarget.LinkerNode[0].Type !=
         FastbuildLinkerNode::SHARED_LIBRARY)) {
    return;
  }
  // File which executes all POST_BUILD steps.
  // We use it in .LinkerStampExeArgs in order to run POST_BUILD steps after
  // the compilation.
  if (!fastbuildTarget.PostBuildExecNodes.Nodes.empty()) {
    std::string const AllPostBuildExecsScriptFile =
      cmStrCat(this->Makefile->GetHomeOutputDirectory(), "/CMakeFiles/",
               fastbuildTarget.Name,
               "-all-postbuild-commands" FASTBUILD_SCRIPT_FILE_EXTENSION);

    CollapseAllExecsIntoOneScriptfile(
      AllPostBuildExecsScriptFile, fastbuildTarget.PostBuildExecNodes.Nodes);
    auto& linkerNode = fastbuildTarget.LinkerNode.back();
    // On macOS, a target may have multiple linker nodes (e.g., for different
    // architectures). In that case, add the POST_BUILD step to only one node
    // to avoid running lipo multiple times.
    linkerNode.LinkerStampExe =
      cmGlobalFastbuildGenerator::GetExternalShellExecutable();
    linkerNode.LinkerStampExeArgs = FASTBUILD_SCRIPT_FILE_ARG;
    linkerNode.LinkerStampExeArgs +=
      cmGlobalFastbuildGenerator::QuoteIfHasSpaces(
        AllPostBuildExecsScriptFile);

  } else {
    LogMessage("No POST_BUILD steps for target: " + fastbuildTarget.Name);
  }
}

void cmFastbuildNormalTargetGenerator::ProcessPostBuildForStaticLib(
  FastbuildTarget& fastbuildTarget) const
{
  // "Library" nodes do not have "LinkerStampExe" property, so we need to be
  // clever here: create an alias that will refer to the binary as well as to
  // all post-build steps. Also, make sure that post-build steps depend on the
  // binary itself.
  LogMessage("ProcessPostBuildForStaticLib(...)");
  FastbuildAliasNode alias;
  alias.Name = std::move(fastbuildTarget.LinkerNode[0].Name);
  for (FastbuildExecNode& postBuildExec :
       fastbuildTarget.PostBuildExecNodes.Nodes) {
    postBuildExec.PreBuildDependencies.emplace(
      fastbuildTarget.LinkerNode[0].LinkerOutput);
    alias.PreBuildDependencies.emplace(postBuildExec.Name);
  }
  fastbuildTarget.AliasNodes.emplace_back(std::move(alias));
}

void cmFastbuildNormalTargetGenerator::CollapseAllExecsIntoOneScriptfile(
  std::string const& scriptFileName,
  std::vector<FastbuildExecNode> const& execs) const
{
  cmsys::ofstream scriptFile(scriptFileName.c_str());
  if (!scriptFile.is_open()) {
    cmSystemTools::Error("Failed to open: " + scriptFileName);
    return;
  }
  LogMessage("Writing collapsed Execs to " + scriptFileName);
  auto const shell = cmGlobalFastbuildGenerator::GetExternalShellExecutable();
  for (auto const& exec : execs) {
    if (exec.ScriptFile.empty()) {
      scriptFile << cmSystemTools::ConvertToOutputPath(exec.ExecExecutable)
                 << " " << exec.ExecArguments << '\n';
    } else {
#if defined(_WIN32)
      scriptFile << "call "
                 << cmSystemTools::ConvertToWindowsOutputPath(exec.ScriptFile)
                 << '\n';
#else
      scriptFile << cmSystemTools::ConvertToOutputPath(shell) << " "
                 << cmSystemTools::ConvertToOutputPath(exec.ScriptFile)
                 << '\n';
#endif
    }
  }
}

std::string cmFastbuildNormalTargetGenerator::ComputeCodeCheckOptions(
  cmSourceFile const& srcFile)
{
  cmValue const srcSkipCodeCheckVal = srcFile.GetProperty("SKIP_LINTING");
  bool const skipCodeCheck = srcSkipCodeCheckVal.IsSet()
    ? srcSkipCodeCheckVal.IsOn()
    : this->GetGeneratorTarget()->GetPropertyAsBool("SKIP_LINTING");

  if (skipCodeCheck) {
    return {};
  }
  std::string compilerLauncher;
  std::string staticCheckRule = this->GenerateCodeCheckRules(
    srcFile, compilerLauncher, "", Config, nullptr);
  LogMessage(cmStrCat("CodeCheck: ", staticCheckRule));
  return staticCheckRule;
}

void cmFastbuildNormalTargetGenerator::ComputeCompilerAndOptions(
  std::string const& compilerOptions, std::string const& staticCheckOptions,
  std::string const& language, FastbuildObjectListNode& outObjectList)
{
  auto& compilers = this->GetGlobalGenerator()->Compilers;
  auto const compilerIter =
    compilers.find(FASTBUILD_COMPILER_PREFIX + language);
  auto const launcherIter =
    compilers.find(FASTBUILD_LAUNCHER_PREFIX + language);
  if (!staticCheckOptions.empty()) {
    // If we want to run static checks - use CMake as a launcher.
    // Tested in "RunCMake.ClangTidy", "RunCMake.IncludeWhatYouUse",
    // "RunCMake.Cpplint", "RunCMake.Cppcheck", "RunCMake.MultiLint" tests.
    outObjectList.Compiler = "." FASTBUILD_LAUNCHER_PREFIX + CMAKE_LANGUAGE;
    outObjectList.CompilerOptions = staticCheckOptions;
    // Add compile command which will be passed to the static analyzer via
    // dash-dash.
    if (compilerIter != compilers.end()) {
      // Wrap in quotes to account for potential spaces in the path.
      outObjectList.CompilerOptions +=
        cmGlobalFastbuildGenerator::QuoteIfHasSpaces(
          compilerIter->second.Executable);
      outObjectList.CompilerOptions += compilerOptions;
    }
  } else if (launcherIter != compilers.end()) {
    // Tested in "RunCMake.CompilerLauncher" test.
    outObjectList.Compiler = "." + launcherIter->first;
    outObjectList.CompilerOptions = launcherIter->second.Args;

    auto vars = cmFastbuildNormalTargetGenerator::ComputeRuleVariables();
    vars.Language = language.c_str();
    std::string const targetSupportPath = this->ConvertToFastbuildPath(
      this->GetGeneratorTarget()->GetCMFSupportDirectory());
    vars.TargetSupportDir = targetSupportPath.c_str();
    RulePlaceholderExpander->ExpandRuleVariables(
      LocalCommonGenerator, outObjectList.CompilerOptions, vars);

    // Add compiler executable explicitly to the compile options.
    if (compilerIter != compilers.end()) {
      outObjectList.CompilerOptions += " ";
      // Wrap in quotes to account for potential spaces in the path.
      outObjectList.CompilerOptions +=
        cmGlobalFastbuildGenerator::QuoteIfHasSpaces(
          compilerIter->second.Executable);
      outObjectList.CompilerOptions += compilerOptions;
    }
  } else if (compilerIter != compilers.end()) {
    outObjectList.Compiler = "." + compilerIter->first;
    outObjectList.CompilerOptions = compilerOptions;
  }
  LogMessage(cmStrCat(".Compiler = ", outObjectList.Compiler));
  LogMessage(cmStrCat(".CompilerOptions = ", outObjectList.CompilerOptions));
}

cmRulePlaceholderExpander::RuleVariables
cmFastbuildNormalTargetGenerator::ComputeRuleVariables() const
{
  cmRulePlaceholderExpander::RuleVariables compileObjectVars;
  compileObjectVars.CMTargetName = GeneratorTarget->GetName().c_str();
  compileObjectVars.CMTargetType =
    cmState::GetTargetTypeName(GeneratorTarget->GetType()).c_str();
  compileObjectVars.Source = FASTBUILD_1_INPUT_PLACEHOLDER;
  compileObjectVars.Object = FASTBUILD_2_INPUT_PLACEHOLDER;
  compileObjectVars.ObjectDir =
    FASTBUILD_DOLLAR_TAG "TargetOutDir" FASTBUILD_DOLLAR_TAG;
  compileObjectVars.ObjectFileDir = "";
  compileObjectVars.Flags = "";
  compileObjectVars.Includes = "";
  compileObjectVars.Defines = "";
  compileObjectVars.Includes = "";
  compileObjectVars.TargetCompilePDB =
    FASTBUILD_DOLLAR_TAG "CompilerPDB" FASTBUILD_DOLLAR_TAG;
  compileObjectVars.Config = Config.c_str();
  return compileObjectVars;
}

std::vector<std::string> cmFastbuildNormalTargetGenerator::GetSourceProperty(
  cmSourceFile const& srcFile, std::string const& prop) const
{
  std::vector<std::string> res;
  if (cmValue val = srcFile.GetProperty(prop)) {
    cmExpandList(*val, res);
    return GetGlobalGenerator()->ConvertToFastbuildPath(res);
  }
  return res;
}

void cmFastbuildNormalTargetGenerator::AppendExtraResources(
  std::set<std::string>& deps) const
{
  // Generate Fastbuild's "Copy" commands to copy resources.
  auto const generateCopyCommands =
    [this](std::vector<cmSourceFile const*>& frameworkDeps) {
      this->OSXBundleGenerator->GenerateMacOSXContentStatements(
        frameworkDeps, this->MacOSXContentGenerator.get(), Config);
    };

  std::vector<cmSourceFile const*> headerSources;
  this->GeneratorTarget->GetHeaderSources(headerSources, Config);
  generateCopyCommands(headerSources);

  std::vector<cmSourceFile const*> extraSources;
  this->GeneratorTarget->GetExtraSources(extraSources, Config);
  generateCopyCommands(extraSources);

  std::vector<cmSourceFile const*> externalObjects;
  this->GeneratorTarget->GetExternalObjects(externalObjects, Config);
  generateCopyCommands(externalObjects);

  for (FastbuildCopyNode const& node : this->CopyNodes) {
    LogMessage("Adding resource: " + node.Name);
    deps.emplace(node.Name);
  }
}

std::string cmFastbuildNormalTargetGenerator::GetCompileOptions(
  cmSourceFile const& srcFile, std::string const& arch)
{
  std::string const language = srcFile.GetLanguage();
  cmRulePlaceholderExpander::RuleVariables compileObjectVars =
    ComputeRuleVariables();
  std::string const compilerFlags = DetectCompilerFlags(srcFile, arch);
  std::string const compilerDefines = ComputeDefines(srcFile);
  compileObjectVars.Flags = compilerFlags.c_str();
  compileObjectVars.Defines = compilerDefines.c_str();
  compileObjectVars.Language = language.c_str();
  if (language == "CUDA") {
    compileObjectVars.CudaCompileMode = this->CudaCompileMode.c_str();
  }

  std::string rule = CompileObjectCmakeRules.at(language);
  RulePlaceholderExpander->ExpandRuleVariables(LocalCommonGenerator, rule,
                                               compileObjectVars);

  std::string compilerExecutable;
  // Remove the compiler from .CompilerOptions, since it would be set as
  // .Compiler in Fastbuild.
  // See https://www.fastbuild.org/docs/functions/objectlist.html for a
  // reference.
  std::string options;
  if (!cmSystemTools::SplitProgramFromArgs(rule, compilerExecutable,
                                           options)) {
    cmSystemTools::Error(cmStrCat("Failed to split compiler options: ", rule));
  }
  LogMessage("Expanded compile options = " + options);
  LogMessage("Compiler executable = " + compilerExecutable);
  return options;
}

std::vector<std::string> cmFastbuildNormalTargetGenerator::GetArches() const
{
  auto arches = this->GetGeneratorTarget()->GetAppleArchs(Config, {});
  // Don't add any arch-specific logic if arch is only one.
  if (arches.empty() || arches.size() == 1) {
    arches.clear();
    arches.emplace_back();
  }
  return arches;
}

void cmFastbuildNormalTargetGenerator::GetCudaDeviceLinkLinkerAndArgs(
  std::string& linker, std::string& args) const
{
  std::string linkCmd =
    this->GetMakefile()->GetDefinition("CMAKE_CUDA_DEVICE_LINK_"
                                       "LIBRARY");
  auto vars = ComputeRuleVariables();
  vars.Language = "CUDA";
  vars.Objects = FASTBUILD_1_INPUT_PLACEHOLDER;
  vars.Target = FASTBUILD_2_INPUT_PLACEHOLDER;
  std::unique_ptr<cmLinkLineDeviceComputer> linkLineComputer(
    new cmLinkLineDeviceComputer(
      this->LocalGenerator,
      this->LocalGenerator->GetStateSnapshot().GetDirectory()));
  std::string linkLibs;
  std::string targetFlags;
  std::string linkFlags;
  std::string frameworkPath;
  std::string linkPath;
  // So that the call to "GetTargetFlags" does not pollute "LinkLibs" and
  // "LinkFlags" with unneeded values.
  std::string dummyLinkLibs;
  std::string dummyLinkFlags;
  this->LocalCommonGenerator->GetDeviceLinkFlags(
    *linkLineComputer, Config, linkLibs, linkFlags, frameworkPath, linkPath,
    this->GeneratorTarget);
  this->LocalCommonGenerator->GetTargetFlags(
    linkLineComputer.get(), Config, dummyLinkLibs, targetFlags, dummyLinkFlags,
    frameworkPath, linkPath, this->GeneratorTarget);
  vars.LanguageCompileFlags = "";
  vars.LinkFlags = linkFlags.c_str();
  vars.LinkLibraries = linkLibs.c_str();
  vars.LanguageCompileFlags = targetFlags.c_str();
  this->RulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                     linkCmd, vars);
  SplitLinkerFromArgs(linkCmd, linker, args);
}

void cmFastbuildNormalTargetGenerator::GenerateCudaDeviceLink(
  FastbuildTarget& target) const
{
  auto const arches = this->GetArches();
  if (!requireDeviceLinking(*this->GeneratorTarget, *this->GetLocalGenerator(),
                            Config)) {
    return;
  }
  LogMessage("GenerateCudaDeviceLink(...)");
  for (auto const& arch : arches) {
    std::string linker;
    std::string args;
    GetCudaDeviceLinkLinkerAndArgs(linker, args);

    FastbuildLinkerNode deviceLinkNode;
    deviceLinkNode.Name = cmStrCat(target.Name, "_cuda_device_link");
    deviceLinkNode.Type = FastbuildLinkerNode::SHARED_LIBRARY;
    deviceLinkNode.Linker = std::move(linker);
    deviceLinkNode.LinkerOptions = std::move(args);
    // Output
    deviceLinkNode.LinkerOutput = this->ConvertToFastbuildPath(cmStrCat(
      FASTBUILD_DOLLAR_TAG "TargetOutDi"
                           "r" FASTBUILD_DOLLAR_TAG "/cmake_device_link",
      (args.empty() ? "" : "_" + arch),
      this->Makefile->GetSafeDefinition("CMAKE_CUDA_OUTPUT_"
                                        "EXTENSION")));

    // Input
    for (auto const& objList : target.ObjectListNodes) {
      deviceLinkNode.LibrarianAdditionalInputs.push_back(objList.Name);
    }
    target.CudaDeviceLinkNode.emplace_back(std::move(deviceLinkNode));
  }
  LogMessage("GenerateCudaDeviceLink end");
}

void cmFastbuildNormalTargetGenerator::GenerateObjects(FastbuildTarget& target)
{
  this->GetGlobalGenerator()->AllFoldersToClean.insert(ObjectOutDir);

  std::map<std::string, FastbuildObjectListNode> nodesPermutations;

  cmCryptoHash hash(cmCryptoHash::AlgoSHA256);

  std::vector<cmSourceFile const*> objectSources;
  GeneratorTarget->GetObjectSources(objectSources, Config);

  std::set<std::string> createdPCH;

  // Directory level.
  bool useUnity =
    GeneratorTarget->GetLocalGenerator()->GetMakefile()->IsDefinitionSet(
      CMAKE_UNITY_BUILD);
  // Check if explicitly disabled for this target.
  auto const targetProp = GeneratorTarget->GetProperty(UNITY_BUILD);
  if (targetProp.IsSet() && targetProp.IsOff()) {
    useUnity = false;
  }

  // List of sources isolated from the unity build if enabled.
  std::set<std::string> isolatedFromUnity;

  // Mapping from unity group (if any) to sources belonging to that group.
  std::map<std::string, std::vector<std::string>> sourcesWithGroups;

  for (cmSourceFile const* source : objectSources) {

    cmSourceFile const& srcFile = *source;
    std::string const pathToFile = srcFile.GetFullPath();
    if (useUnity) {
      // Check if the source should be added to "UnityInputIsolatedFiles".
      if (srcFile.GetPropertyAsBool(SKIP_UNITY_BUILD_INCLUSION)) {
        isolatedFromUnity.emplace(pathToFile);
      }
      std::string const perFileUnityGroup =
        srcFile.GetSafeProperty(UNITY_GROUP);
      if (!perFileUnityGroup.empty()) {
        sourcesWithGroups[perFileUnityGroup].emplace_back(pathToFile);
      }
    }

    this->GetGlobalGenerator()->AddFileToClean(cmStrCat(
      ObjectOutDir, '/', this->GeneratorTarget->GetObjectName(source)));

    // Do not generate separate node for PCH source file.
    if (this->GeneratorTarget->GetPchSource(Config, srcFile.GetLanguage()) ==
        pathToFile) {
      continue;
    }

    std::string const language = srcFile.GetLanguage();
    LogMessage(
      cmStrCat("Source file: ", this->ConvertToFastbuildPath(pathToFile)));
    LogMessage("Language: " + language);

    std::string const staticCheckOptions = ComputeCodeCheckOptions(srcFile);

    auto const isDisabled = [this](char const* prop) {
      auto const propValue = this->GeneratorTarget->GetProperty(prop);
      return propValue && propValue.IsOff();
    };
    bool const disableCaching = isDisabled("FASTBUILD_CACHING");
    bool const disableDistribution = isDisabled("FASTBUILD_DISTRIBUTION");

    for (auto const& arch : this->GetArches()) {
      std::string const compileOptions = GetCompileOptions(srcFile, arch);

      std::string objOutDirWithPossibleSubdir = ObjectOutDir;

      // If object should be placed in some subdir in the output
      // path. Tested in "SourceGroups" test.
      auto const subdir = cmSystemTools::GetFilenamePath(
        this->GeneratorTarget->GetObjectName(source));
      if (!subdir.empty()) {
        objOutDirWithPossibleSubdir += "/";
        objOutDirWithPossibleSubdir += subdir;
      }

      std::string const objectListHash = hash.HashString(cmStrCat(
        compileOptions, staticCheckOptions, objOutDirWithPossibleSubdir,
        // If file does not need PCH - it must be in another ObjectList.
        srcFile.GetProperty("SKIP_PRECOMPILE_HEADERS"),
        srcFile.GetLanguage()));

      LogMessage("ObjectList Hash: " + objectListHash);

      FastbuildObjectListNode& objectListNode =
        nodesPermutations[objectListHash];

      // Absolute path needed in "RunCMake.SymlinkTrees" test.
      objectListNode.CompilerInputFiles.push_back(pathToFile);

      std::vector<std::string> const outputs =
        GetSourceProperty(srcFile, "OBJECT_OUTPUTS");
      objectListNode.ObjectOutputs.insert(outputs.begin(), outputs.end());

      std::vector<std::string> const depends =
        GetSourceProperty(srcFile, "OBJECT_DEPENDS");
      objectListNode.ObjectDepends.insert(depends.begin(), depends.end());

      // We have already computed properties that are computed below.
      // (.CompilerOptions, .PCH*, etc.). Short circuit this iteration.
      if (!objectListNode.CompilerOptions.empty()) {
        continue;
      }
      if (disableCaching) {
        objectListNode.AllowCaching = false;
      }
      if (disableDistribution) {
        objectListNode.AllowDistribution = false;
      }

      objectListNode.CompilerOutputPath = objOutDirWithPossibleSubdir;
      LogMessage(cmStrCat("Output path: ", objectListNode.CompilerOutputPath));

      ComputeCompilerAndOptions(compileOptions, staticCheckOptions, language,
                                objectListNode);
      ComputePCH(*source, objectListNode, createdPCH);

      objectListNode.Name = cmStrCat(this->GetName(), '_', language, "_Objs");
      // TODO: Ask cmake the output objects and group by extension instead
      // of doing this
      if (language == "RC") {
        objectListNode.CompilerOutputExtension = ".res";
      } else {
        if (!arch.empty()) {
          objectListNode.CompilerOutputExtension = cmStrCat('.', arch);
          objectListNode.arch = arch;
        }
        char const* customExt =
          this->GeneratorTarget->GetCustomObjectExtension();

        objectListNode.CompilerOutputExtension +=
          this->GetMakefile()->GetSafeDefinition(
            cmStrCat("CMAKE_", language, "_OUTPUT_EXTENSION"));
        // Tested in "CudaOnly.ExportPTX" test.
        if (customExt) {
          objectListNode.CompilerOutputExtension += customExt;
        }
      }
    }
  }

  int groupNameCount = 0;

  for (auto& val : nodesPermutations) {
    auto& objectListNode = val.second;
    objectListNode.Name = cmStrCat(objectListNode.Name, '_', ++groupNameCount);
    LogMessage(cmStrCat("ObjectList name: ", objectListNode.Name));
  }
  std::vector<FastbuildObjectListNode>& objects = target.ObjectListNodes;
  objects.reserve(nodesPermutations.size());
  for (auto& val : nodesPermutations) {
    auto& node = val.second;
    if (!node.PCHInputFile.empty()) {
      // Node that produces PCH should be the first one, since other nodes
      // might reuse this PCH.
      // Note: we might have several such nodes for different languages.
      objects.insert(objects.begin(), std::move(node));
    } else {
      objects.emplace_back(std::move(node));
    }
  }
  if (useUnity) {
    target.UnityNodes =
      GenerateUnity(objects, isolatedFromUnity, sourcesWithGroups);
  }
}

FastbuildUnityNode cmFastbuildNormalTargetGenerator::GetOneUnity(
  std::set<std::string> const& isolatedFiles, std::vector<std::string>& files,
  int unitySize) const
{
  FastbuildUnityNode result;
  for (auto iter = files.begin(); iter != files.end();) {
    std::string pathToFile = std::move(*iter);
    iter = files.erase(iter);
    // This source must be isolated
    if (isolatedFiles.find(pathToFile) != isolatedFiles.end()) {
      result.UnityInputFiles.emplace_back(pathToFile);
      result.UnityInputIsolatedFiles.emplace_back(std::move(pathToFile));
    } else {
      result.UnityInputFiles.emplace_back(std::move(pathToFile));
    }
    if (int(result.UnityInputFiles.size() -
            result.UnityInputIsolatedFiles.size()) == unitySize) {
      break;
    }
  }
  return result;
}
int cmFastbuildNormalTargetGenerator::GetUnityBatchSize() const
{
  int unitySize = 8;
  try {
    auto const perTargetSize =
      GeneratorTarget->GetSafeProperty(UNITY_BUILD_BATCH_SIZE);
    if (!perTargetSize.empty()) {
      unitySize = std::stoi(perTargetSize);
    }
    // Per-directory level.
    else {
      unitySize = std::stoi(
        GeneratorTarget->GetLocalGenerator()->GetMakefile()->GetDefinition(
          CMAKE_UNITY_BUILD_BATCH_SIZE));
    }
  } catch (...) {
    return unitySize;
  }
  return unitySize;
}

std::vector<FastbuildUnityNode>
cmFastbuildNormalTargetGenerator::GenerateUnity(
  std::vector<FastbuildObjectListNode>& objects,
  std::set<std::string> const& isolatedSources,
  std::map<std::string, std::vector<std::string>> const& sourcesWithGroups)
{
  int const unitySize = GetUnityBatchSize();
  // Unity of size less than 2 doesn't make sense.
  if (unitySize < 2) {
    return {};
  }

  int unityNumber = 0;
  int unityGroupNumber = 0;
  std::vector<FastbuildUnityNode> result;

  for (FastbuildObjectListNode& obj : objects) {
    // Don't use unity for only 1 file.
    if (obj.CompilerInputFiles.size() < 2) {
      continue;
    }
    std::string const ext =
      cmSystemTools::GetFilenameExtension(obj.CompilerInputFiles[0]);
    // Process groups.
    auto groupedNode = GenerateGroupedUnityNode(
      obj.CompilerInputFiles, sourcesWithGroups, unityGroupNumber);
    // We have at least 2 sources in the group.
    if (groupedNode.UnityInputFiles.size() > 1) {
      groupedNode.UnityOutputPath = obj.CompilerOutputPath;
      obj.CompilerInputUnity.emplace_back(groupedNode.Name);
      groupedNode.UnityOutputPattern = cmStrCat(groupedNode.Name, ext);
      result.emplace_back(std::move(groupedNode));
    }
    // General unity batching of the remaining (non-grouped) sources.
    while (!obj.CompilerInputFiles.empty()) {
      FastbuildUnityNode node =
        GetOneUnity(isolatedSources, obj.CompilerInputFiles, unitySize);
      node.Name = cmStrCat(this->GetName(), "_Unity_", ++unityNumber);
      node.UnityOutputPath = obj.CompilerOutputPath;
      node.UnityOutputPattern = cmStrCat(node.Name, ext);

      // Unity group of size 1 doesn't make sense - just isolate the source.
      if (groupedNode.UnityInputFiles.size() == 1) {
        node.UnityInputIsolatedFiles.emplace_back(
          groupedNode.UnityInputFiles[0]);
        node.UnityInputFiles.emplace_back(
          std::move(groupedNode.UnityInputFiles[0]));
        // Clear so we don't enter here on the next iteration.
        groupedNode.UnityInputFiles.clear();
      }

      // We've got only 1 file left. No need to create a Unity node for it,
      // just return it back to the ObjectList and exit.
      if (node.UnityInputFiles.size() == 1) {
        obj.CompilerInputFiles.emplace_back(
          std::move(node.UnityInputFiles[0]));
        break;
      }

      obj.CompilerInputUnity.emplace_back(node.Name);
      result.emplace_back(std::move(node));
    }
  }
  return result;
}

FastbuildUnityNode cmFastbuildNormalTargetGenerator::GenerateGroupedUnityNode(
  std::vector<std::string>& inputFiles,
  std::map<std::string, std::vector<std::string>> const& sourcesWithGroups,
  int& groupId)
{
  std::vector<FastbuildUnityNode> result;
  for (auto const& item : sourcesWithGroups) {
    auto const& group = item.first;
    auto const& sources = item.second;
    FastbuildUnityNode node;
    // Check if any of the sources belong to this group.
    for (auto const& source : sources) {
      auto const iter =
        std::find(inputFiles.begin(), inputFiles.end(), source);
      if (iter == inputFiles.end()) {
        continue;
      }
      node.Name =
        cmStrCat(this->GetName(), "_Unity_Group_", group, '_', ++groupId);
      node.UnityInputFiles.emplace_back(source);

      // Remove from the general batching.
      inputFiles.erase(
        std::remove(inputFiles.begin(), inputFiles.end(), source),
        inputFiles.end());
    }
    if (!node.UnityInputFiles.empty()) {
      // The unity group belongs to the ObjectLists that we're processing.
      // We've grouped all the sources we could from the current ObjectList.
      return node;
    }
  }
  return {};
}

std::string cmFastbuildNormalTargetGenerator::ResolveIfAlias(
  std::string const& targetName) const
{
  LogMessage("targetName: " + targetName);
  std::map<std::string, std::string> const aliases =
    this->Makefile->GetAliasTargets();
  auto const iter = aliases.find(targetName);
  if (iter != aliases.end()) {
    LogMessage("Non alias name: " + iter->second);
    return iter->second;
  }
  return targetName;
}

void cmFastbuildNormalTargetGenerator::AppendExternalObject(
  FastbuildLinkerNode& linkerNode, std::set<std::string>& linkedDeps) const
{
  // Different aspects of this logic exercised in "ObjectLibrary" and
  // "ExportImport" test. When making changes here - verify that both of those
  // tests are still passing.
  LogMessage("AppendExternalObject(...)");
  std::vector<cmSourceFile const*> extObjects;
  this->GeneratorTarget->GetExternalObjects(extObjects, Config);
  for (cmSourceFile const* src : extObjects) {

    std::string const pathToObj =
      this->ConvertToFastbuildPath(src->GetFullPath());
    LogMessage("EXT OBJ: " + pathToObj);
    std::string const objLibName = ResolveIfAlias(src->GetObjectLibrary());
    LogMessage("GetObjectLibrary: " + objLibName);
    // Tested in "ExternalOBJ" test.
    cmTarget const* target =
      this->GlobalCommonGenerator->FindTarget(objLibName);
    if (objLibName.empty()) {
      linkerNode.LibrarianAdditionalInputs.emplace_back(pathToObj);
    }
    // We know how to generate this target and haven't added this dependency
    // yet.
    else if (target) {
      if (!linkedDeps.emplace(objLibName + FASTBUILD_OBJECTS_ALIAS_POSTFIX)
             .second) {
        LogMessage("Object Target: " + objLibName +
                   FASTBUILD_OBJECTS_ALIAS_POSTFIX " already linked");
        continue;
      }
      linkerNode.LibrarianAdditionalInputs.emplace_back(
        objLibName + FASTBUILD_OBJECTS_ALIAS_POSTFIX);
    } else if (linkedDeps.emplace(pathToObj).second) {
      LogMessage("Adding obj dep : " + pathToObj);
      linkerNode.LibrarianAdditionalInputs.emplace_back(pathToObj);
    }
  }
}

void cmFastbuildNormalTargetGenerator::AppendExeToLink(
  FastbuildLinkerNode& linkerNode,
  cmComputeLinkInformation::Item const& item) const
{
  std::string const decorated =
    item.GetFormattedItem(this->ConvertToFastbuildPath(item.Value.Value))
      .Value;
  LogMessage("Linking to executable : " + decorated);
  // Tested in "InterfaceLinkLibrariesDirect" and "Plugin" test.
  linkerNode.LinkerOptions +=
    (" " + cmGlobalFastbuildGenerator::QuoteIfHasSpaces(decorated));
}

std::string cmFastbuildNormalTargetGenerator::GetImportedLoc(
  cmComputeLinkInformation::Item const& item) const
{
  // Link to import library when possible.
  // Tested in "StagingPrefix" test on Windows/MSVC.
  cmStateEnums::ArtifactType const artifact =
    item.Target->HasImportLibrary(Config)
    ? cmStateEnums::ImportLibraryArtifact
    : cmStateEnums::RuntimeBinaryArtifact;

  std::string importedLoc = this->ConvertToFastbuildPath(
    item.Target->GetFullPath(Config, artifact, true));
  LogMessage("ImportedGetLocation: " + importedLoc);
  return importedLoc;
}

void cmFastbuildNormalTargetGenerator::AppendTargetDep(
  FastbuildLinkerNode& linkerNode, std::set<std::string>& linkedObjects,
  cmComputeLinkInformation::Item const& item) const
{
  LogMessage("AppendTargetDep(...)");
  cmStateEnums::TargetType const depType = item.Target->GetType();
  LogMessage("Link dep type: " + std::to_string(depType));
  LogMessage("Target name: " + item.Target->GetName());
  auto const resolvedTargetName = ResolveIfAlias(item.Target->GetName());
  LogMessage("Resolved: " + resolvedTargetName);
  if (depType == cmStateEnums::INTERFACE_LIBRARY) {
    return;
  }
  std::string const feature = item.GetFeatureName();

  if (item.Target->IsImported()) {

    if (feature == "FRAMEWORK") {
      // Use just framework's name. The exact path where to look for the
      // framework will be provided from "frameworkPath" in
      // "cmFastbuildNormalTargetGenerator::DetectBaseLinkerCommand(...)".
      // Tested in "RunCMake.Framework - ImportedFrameworkConsumption".
      std::string const decorated =
        item.GetFormattedItem(item.Value.Value).Value;
      LogMessage(
        cmStrCat("Adding framework dep <", decorated, "> to command line"));
      linkerNode.LinkerOptions += (" " + decorated);
      return;
    }
    if (depType == cmStateEnums::UNKNOWN_LIBRARY) {
      LogMessage("Unknown library -- adding to LibrarianAdditionalInputs or "
                 "Libraries2");
      if (UsingCommandLine) {
        AppendCommandLineDep(linkerNode, item);
      } else {
        AppendLinkDep(linkerNode, GetImportedLoc(item));
      }
      return;
    }
    // Tested in "ExportImport" test.
    if (depType == cmStateEnums::EXECUTABLE) {
      AppendExeToLink(linkerNode, item);
      return;
    }
    // Skip exported objects.
    // Tested in "ExportImport" test.
    if (depType == cmStateEnums::OBJECT_LIBRARY) {
      LogMessage("target : " + item.Target->GetName() +
                 " already linked... Skipping");
      return;
    }
    // Tested in "ExportImport" test.
    cmList const list{ GetImportedLoc(item) };
    for (std::string const& linkDep : list) {
      AppendLinkDep(linkerNode, linkDep);
    }
  } else {
    if (depType == cmStateEnums::SHARED_LIBRARY &&
        this->GeneratorTarget->GetPropertyAsBool("LINK_DEPENDS_NO_SHARED")) {
      // It moves the dep outside of FASTBuild control, so the binary won't
      // be re-built if the shared lib has changed.
      // Tested in "BuildDepends" test.
      LogMessage(
        cmStrCat("LINK_DEPENDS_NO_SHARED is set on the target, adding dep",
                 item.Value.Value, " as is"));
      linkerNode.LinkerOptions +=
        (" " + cmGlobalFastbuildGenerator::QuoteIfHasSpaces(item.Value.Value));
      return;
    }
    // Just add path to binary artifact to command line (except for OBJECT
    // libraries which we will link directly).
    if (UsingCommandLine && depType != cmStateEnums::OBJECT_LIBRARY) {
      AppendCommandLineDep(linkerNode, item);
      return;
    }
    // This dep has a special way of linking to it (e.g.
    // "CMAKE_LINK_LIBRARY_USING_<FEATURE>").
    bool const isFeature = !feature.empty() && feature != "DEFAULT";
    if (isFeature) {
      std::string const decorated =
        item.GetFormattedItem(this->ConvertToFastbuildPath(item.Value.Value))
          .Value;
      LogMessage("Prepending with feature: " + decorated);
      linkerNode.LinkerOptions += (" " + decorated);
    }

    std::string dep = resolvedTargetName +
      (depType == cmStateEnums::OBJECT_LIBRARY
         ? FASTBUILD_OBJECTS_ALIAS_POSTFIX
         : FASTBUILD_LINK_ARTIFACTS_ALIAS_POSTFIX);
    if (!linkerNode.Arch.empty()) {
      dep += cmStrCat('-', linkerNode.Arch);
    }
    // If we have a special way of linking the dep, we can't have it in
    // ".Libraries" (since there might be multiple such deps, but
    // FASTBuild expands ".Libraries" as a continuous array, so we can't
    // inject any properties in between). Tested in
    // "RunCMake.target_link_libraries-LINK_LIBRARY" test.
    if (isFeature) {
      LogMessage(cmStrCat("AppendTargetDep: ", dep, " as prebuild"));
      linkerNode.PreBuildDependencies.emplace(dep);
      return;
    }

    if (depType != cmStateEnums::OBJECT_LIBRARY ||
        linkedObjects.emplace(dep).second) {
      AppendLinkDep(linkerNode, dep);
    }
    AppendTransitivelyLinkedObjects(*item.Target, linkedObjects);
  }
}

void cmFastbuildNormalTargetGenerator::AppendPrebuildDeps(
  FastbuildLinkerNode& linkerNode,
  cmComputeLinkInformation::Item const& item) const
{
  if (!item.Target->IsImported()) {
    return;
  }
  // In "RunCMake.FileAPI" imported object library "imported_object_lib" is
  // added w/o import location...
  if (item.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    return;
  }
  cmList const list{ GetImportedLoc(item) };
  for (std::string const& linkDep : list) {
    // In case we know how to generate this file (needed for proper
    // sorting by deps). Tested in "RunCMake.target_link_libraries-ALIAS"
    // test.
    auto fastbuildTarget =
      this->GetGlobalGenerator()->GetTargetByOutputName(linkDep);
    std::string fastbuildTargetName;
    if (fastbuildTarget) {
      fastbuildTargetName = std::move(fastbuildTarget->Name);
    }
    if (!fastbuildTargetName.empty()) {
      LogMessage("Adding dep to " + fastbuildTargetName);
      linkerNode.PreBuildDependencies.insert(std::move(fastbuildTargetName));
    } else {
      if (!cmIsNOTFOUND(linkDep)) {
        LogMessage(cmStrCat("Adding dep ", linkDep, " for sorting"));
        linkerNode.PreBuildDependencies.insert(linkDep);
      }
    }
  }
}

void cmFastbuildNormalTargetGenerator::AppendTransitivelyLinkedObjects(
  cmGeneratorTarget const& target, std::set<std::string>& linkedObjects) const
{
  std::vector<std::string> objs;
  // Consider that all those object are now linked as well.
  // Tested in "ExportImport" test.
  target.GetTargetObjectNames(Config, objs);
  for (std::string const& obj : objs) {
    std::string const pathToObj = this->ConvertToFastbuildPath(
      cmStrCat(target.GetObjectDirectory(Config), '/', obj));
    linkedObjects.insert(pathToObj);
  }
  // Object libs should not be propagated transitively. It's especially
  // important for LinkObjRHSObject2 test where the absence of the propagation
  // is tested.
  for (auto const& linkedTarget :
       target.Target->GetLinkImplementationEntries()) {
    auto objAlias = linkedTarget.Value + FASTBUILD_OBJECTS_ALIAS_POSTFIX;
    LogMessage("Object target is linked transitively " + objAlias);
    linkedObjects.emplace(std::move(objAlias));
  }
}

void cmFastbuildNormalTargetGenerator::AppendCommandLineDep(
  FastbuildLinkerNode& linkerNode,
  cmComputeLinkInformation::Item const& item) const
{
  LogMessage("AppendCommandLineDep(...)");
  // Tested in:
  // "LinkDirectory" (TargetType::EXECUTABLE),
  // "ObjC.simple-build-test" (TargetType::SHARED_LIBRARY),
  // "XCTest" (TargetType::MODULE_LIBRARY) tests.

  std::string formatted;
  if (item.Target && item.Target->IsImported()) {
    formatted = GetImportedLoc(item);
  } else {
    formatted = item.GetFormattedItem(item.Value.Value).Value;
  }
  formatted = this->ConvertToFastbuildPath(formatted);

  LogMessage(
    cmStrCat("Unknown link dep: ", formatted, ", adding to command line"));

  // Only add real artifacts to .Libraries2, otherwise Fastbuild will always
  // consider the target out-of-date (since its input doesn't exist).
  if (item.IsPath == cmComputeLinkInformation::ItemIsPath::Yes &&
      item.GetFeatureName() == "DEFAULT") {
    linkerNode.LinkerOptions +=
      (" " + cmGlobalFastbuildGenerator::QuoteIfHasSpaces(formatted));
    AppendToLibraries2IfApplicable(linkerNode, std::move(formatted));
  } else {
    // It's some link option, not a path.
    linkerNode.LinkerOptions += (" " + formatted);
  }
}

void cmFastbuildNormalTargetGenerator::AppendToLibraries2IfApplicable(
  FastbuildLinkerNode& linkerNode, std::string dep) const
{
  // Strings like "-framework Cocoa" in .Libraries2 node will always make the
  // target out-of-date (since it never exists).
  if (this->GeneratorTarget->IsApple() &&
      cmSystemTools::StringStartsWith(dep, "-framework")) {
    LogMessage(cmStrCat("Not adding framework: ", dep, " to .Libraries2"));
    return;
  }

  auto const target = this->GetGlobalGenerator()->GetTargetByOutputName(dep);
  // Fastbuild doesn't support executables in .Libraries2, though we can use
  // Executables via "-bundle_loader" on Apple.
  if (this->GeneratorTarget->IsApple() && target &&
      !target->LinkerNode.empty() &&
      target->LinkerNode[0].Type == FastbuildLinkerNode::EXECUTABLE) {
    LogMessage(cmStrCat("Not adding DLL/Executable(", linkerNode.Name,
                        " to .Libraries2"));
    return;
  }

  // Adding to .Libraries2 for tracking.
  LogMessage(cmStrCat("Adding ", dep, " .Libraries2"));
  linkerNode.Libraries2.emplace_back(std::move(dep));
}

void cmFastbuildNormalTargetGenerator::AppendLINK_DEPENDS(
  FastbuildLinkerNode& linkerNode) const
{
  // LINK_DEPENDS and such.
  // Tested in "BuildDepends" test.
  for (std::string const& lang : Languages) {
    for (BT<std::string> const& dep :
         this->GeneratorTarget->GetLinkDepends(Config, lang)) {
      // We can't add "LINK_DEPENDS" to .PreBuildDependencies, since FASTBuild
      // only forces such targets to be built and doesn't force re-linking if
      // they've changed.
      linkerNode.Libraries2.emplace_back(
        this->ConvertToFastbuildPath(dep.Value));
    }
  }
}

void cmFastbuildNormalTargetGenerator::AppendLinkDep(
  FastbuildLinkerNode& linkerNode, std::string dep) const
{
  LogMessage(cmStrCat("AppendLinkDep: ", dep,
                      " to .LibrarianAdditionalInputs/.Libraries"));
  linkerNode.LibrarianAdditionalInputs.emplace_back(std::move(dep));
}

void cmFastbuildNormalTargetGenerator::AppendDirectObjectLibs(
  FastbuildLinkerNode& linkerNode, std::set<std::string>& linkedObjects)
{
  auto const srcs = this->GeneratorTarget->GetSourceFiles(Config);
  for (auto const& entry : srcs) {
    auto const objLib = entry.Value->GetObjectLibrary();
    auto const objPath = entry.Value->GetFullPath();
    LogMessage("Source obj entry: " + objPath);
    if (!objLib.empty()) {
      auto* const objTarget =
        this->LocalGenerator->FindGeneratorTargetToUse(objLib);
      if (objTarget) {
        LogMessage("Imported: " + std::to_string(objTarget->IsImported()));
        std::string fastbuildTarget;
        // If target is imported - we don't have it in our build file, so can't
        // refer to it by name. Use file path to the object then.
        // Tested in "ExportImport" test.
        if (objTarget->IsImported()) {
          fastbuildTarget = entry.Value->GetFullPath();
        } else {
          // Mark all target objects as linked.
          linkedObjects.emplace(this->ConvertToFastbuildPath(objPath));
          fastbuildTarget =
            objTarget->GetName() + FASTBUILD_OBJECTS_ALIAS_POSTFIX;
        }
        if (linkedObjects.emplace(fastbuildTarget).second) {
          LogMessage("Adding object target: " + fastbuildTarget);
          linkerNode.LibrarianAdditionalInputs.emplace_back(
            std::move(fastbuildTarget));
        }
      }
    }
  }
}

void cmFastbuildNormalTargetGenerator::AppendLinkDeps(
  std::set<FastbuildTargetDep>& preBuildDeps, FastbuildLinkerNode& linkerNode,
  FastbuildLinkerNode& cudaDeviceLinkLinkerNode)
{
  std::set<std::string> linkedObjects;
  cmComputeLinkInformation const* linkInfo =
    this->GeneratorTarget->GetLinkInformation(Config);
  if (!linkInfo) {
    return;
  }

  UsingCommandLine = false;
  AppendLINK_DEPENDS(linkerNode);
  // Object libs that are linked directly to target (e.g.
  // add_executable(test_exe archiveObjs)
  AppendDirectObjectLibs(linkerNode, linkedObjects);
  std::size_t numberOfDirectlyLinkedObjects =
    linkerNode.LibrarianAdditionalInputs.size();
  // target_link_libraries.
  cmComputeLinkInformation::ItemVector const items = linkInfo->GetItems();

  LogMessage(cmStrCat("Link items size: ", items.size()));
  for (cmComputeLinkInformation::Item const& item : items) {
    std::string const feature = item.GetFeatureName();
    LogMessage("GetFeatureName: " + feature);
    if (!feature.empty()) {
      LogMessage("GetFormattedItem: " +
                 item.GetFormattedItem(item.Value.Value).Value);
    }
    // We're linked to `$<TARGET_OBJECTS>`.
    // Static libs transitively propagate such deps, see:
    // https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries-via-target-objects
    if (item.ObjectSource &&
        linkerNode.Type != FastbuildLinkerNode::STATIC_LIBRARY) {
      // Tested in "ObjectLibrary" test.
      auto libName = item.ObjectSource->GetObjectLibrary();
      std::string dep = libName + FASTBUILD_OBJECTS_ALIAS_POSTFIX;
      if (linkedObjects.emplace(dep).second) {
        FastbuildTargetDep targetDep{ std::move(libName) };
        targetDep.Type = FastbuildTargetDepType::ORDER_ONLY;
        preBuildDeps.emplace(std::move(targetDep));
        linkerNode.LibrarianAdditionalInputs.emplace_back(std::move(dep));
      }
    } else if (linkerNode.Type == FastbuildLinkerNode::STATIC_LIBRARY) {
      LogMessage(cmStrCat("Skipping linking to STATIC_LIBRARY (",
                          linkerNode.Name, ')'));
      continue;
    }
    // We're linked to exact target.
    else if (item.Target) {
      AppendTargetDep(linkerNode, linkedObjects, item);
      AppendPrebuildDeps(linkerNode, item);
      if (!item.Target->IsImported() &&
          item.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        ++numberOfDirectlyLinkedObjects;
        cudaDeviceLinkLinkerNode.LibrarianAdditionalInputs.emplace_back(
          cmStrCat(item.Target->GetName(), FASTBUILD_OBJECTS_ALIAS_POSTFIX));
      }

    } else {
      AppendCommandLineDep(linkerNode, item);
      UsingCommandLine = true;
    }
  }
  AppendExternalObject(linkerNode, linkedObjects);

  if (!cudaDeviceLinkLinkerNode.Name.empty()) {
    linkerNode.LibrarianAdditionalInputs.push_back(
      cudaDeviceLinkLinkerNode.Name);
    // CUDA device-link stub needs to go AFTER direct object dependencies, but
    // BEFORE all other dependencies. Needed for the correct left-to-right
    // symbols resolution on Linux.
    std::swap(
      linkerNode.LibrarianAdditionalInputs[numberOfDirectlyLinkedObjects],
      linkerNode.LibrarianAdditionalInputs.back());
  }
}

void cmFastbuildNormalTargetGenerator::AddLipoCommand(FastbuildTarget& target)
{
  static auto const lipo = cmSystemTools::FindProgram("lipo");
  LogMessage("found lipo at " + lipo);
  FastbuildExecNode exec;
  exec.ExecExecutable = lipo;
  exec.ExecOutput = target.RealOutput;
  if (exec.ExecOutput != target.Name) {
    exec.Name = target.Name;
  }
  for (auto const& ArchSpecificTarget : target.LinkerNode) {
    exec.ExecInput.emplace_back(ArchSpecificTarget.LinkerOutput);
  }
  exec.ExecArguments += cmStrCat("-create -output ", target.RealOutput, " ",
                                 cmJoin(exec.ExecInput, " "));
  target.PostBuildExecNodes.Alias.PreBuildDependencies.emplace(
    exec.ExecOutput);
  target.PostBuildExecNodes.Nodes.emplace_back(std::move(exec));
}

void cmFastbuildNormalTargetGenerator::GenerateLink(
  FastbuildTarget& target, std::vector<std::string> const& objectDepends)
{
  std::string const targetName = this->GetTargetName();
  cmGeneratorTarget::Names const targetNames = DetectOutput();
  LogMessage("targetNames.Real: " + targetNames.Real);
  LogMessage("targetNames.ImportOutput: " + targetNames.ImportOutput);
  LogMessage("targetNames.SharedObject: " + targetNames.SharedObject);
  LogMessage("targetNames.Base: " + targetNames.Base);

  std::vector<std::string> allNodes;
  auto const arches = this->GetArches();
  for (std::size_t i = 0; i < arches.size(); ++i) {
    auto const& arch = arches[i];
    FastbuildLinkerNode linkerNode;
    ProcessManifests(linkerNode);
    // Objects built by the current target.
    for (auto const& objectList : target.ObjectListNodes) {
      if (objectList.arch.empty() || objectList.arch == arch) {
        linkerNode.LibrarianAdditionalInputs.push_back(objectList.Name);
      }
    }

    // Detection of the link command as follows:
    auto const type = this->GeneratorTarget->GetType();
    switch (type) {
      case cmStateEnums::EXECUTABLE: {
        LogMessage("Generating EXECUTABLE");
        linkerNode.Type = FastbuildLinkerNode::EXECUTABLE;
        break;
      }
      case cmStateEnums::MODULE_LIBRARY: {
        LogMessage("Generating MODULE_LIBRARY");
        linkerNode.Type = FastbuildLinkerNode::SHARED_LIBRARY;
        break;
      }
      case cmStateEnums::SHARED_LIBRARY: {
        LogMessage("Generating SHARED_LIBRARY");
        linkerNode.Type = FastbuildLinkerNode::SHARED_LIBRARY;
        break;
      }
      case cmStateEnums::STATIC_LIBRARY: {
        LogMessage("Generating STATIC_LIBRARY");
        linkerNode.Type = FastbuildLinkerNode::STATIC_LIBRARY;
        break;
      }
      case cmStateEnums::OBJECT_LIBRARY: {
        LogMessage("Generating OBJECT_LIBRARY");
        return;
      }
      default: {
        LogMessage("Skipping GenerateLink");
        return;
      }
    }

    std::string const targetOutput =
      ConvertToFastbuildPath(GeneratorTarget->GetFullPath(Config));
    std::string targetOutputReal = ConvertToFastbuildPath(
      GeneratorTarget->GetFullPath(Config, cmStateEnums::RuntimeBinaryArtifact,
                                   /*realname=*/true));
    LogMessage("targetOutput: " + targetOutput);
    LogMessage("targetOutputReal: " + targetOutputReal);

    std::string const output =
      cmSystemTools::GetFilenameName(targetNames.Output);
    std::string const outputReal =
      cmSystemTools::GetFilenameName(targetNames.Real);
    // Generate "Copy" nodes for copying Framework / Bundle resources.
    AppendExtraResources(linkerNode.PreBuildDependencies);

    if (type == cmStateEnums::EXECUTABLE ||
        type == cmStateEnums::SHARED_LIBRARY) {
      // Tested in "RunCMake.BuildDepends" test (we need to rebuild when
      // manifest  changes).
      std::copy(objectDepends.begin(), objectDepends.end(),
                std::back_inserter(linkerNode.Libraries2));
    }

    if (GeneratorTarget->IsAppBundleOnApple()) {
      // Create the app bundle
      std::string outpath = GeneratorTarget->GetDirectory(Config);
      this->OSXBundleGenerator->CreateAppBundle(targetNames.Output, outpath,
                                                Config);
      targetOutputReal = cmStrCat(outpath, '/', outputReal);
      targetOutputReal = this->ConvertToFastbuildPath(targetOutputReal);
    } else if (GeneratorTarget->IsFrameworkOnApple()) {
      // Create the library framework.
      this->OSXBundleGenerator->CreateFramework(
        targetNames.Output, GeneratorTarget->GetDirectory(Config), Config);
    } else if (GeneratorTarget->IsCFBundleOnApple()) {
      // Create the core foundation bundle.
      this->OSXBundleGenerator->CreateCFBundle(
        targetNames.Output, GeneratorTarget->GetDirectory(Config), Config);
    }

    std::string linkCmd;
    if (!DetectBaseLinkerCommand(linkCmd, arch, targetNames)) {
      LogMessage("No linker command detected");
      return;
    }

    std::string executable;
    std::string linkerOptions;
    std::string linkerType = "auto";

    GetLinkerExecutableAndArgs(linkCmd, executable, linkerOptions);

    linkerNode.Compiler = ".Compiler_dummy";
    linkerNode.CompilerOptions = " ";

    linkerNode.Name = targetName;
    linkerNode.LinkerOutput = targetOutputReal;
    this->GetGlobalGenerator()->AddFileToClean(linkerNode.LinkerOutput);
    target.RealOutput = targetOutputReal;
    if (!arch.empty()) {
      linkerNode.Name += cmStrCat('-', arch);
      linkerNode.LinkerOutput += cmStrCat('.', arch);
      linkerNode.Arch = arch;
    }
    linkerNode.Linker = executable;
    linkerNode.LinkerType = linkerType;
    linkerNode.LinkerOptions += linkerOptions;

    // Check if we have CUDA device link stub for this target.
    FastbuildLinkerNode dummyCudaDeviceLinkNode;
    AppendLinkDeps(target.PreBuildDependencies, linkerNode,
                   target.CudaDeviceLinkNode.size() > i
                     ? target.CudaDeviceLinkNode[i]
                     : dummyCudaDeviceLinkNode);
    ApplyLWYUToLinkerCommand(linkerNode);

    // On macOS, only the last LinkerNode performs lipo in POST_BUILD.
    // Make it depend on all previous nodes to ensure correct execution order.
    if (i == arches.size() - 1) {
      for (auto& prevNode : allNodes) {
        linkerNode.PreBuildDependencies.emplace(std::move(prevNode));
      }
    } else {
      allNodes.emplace_back(linkerNode.Name);
    }
    if (!target.ObjectListNodes.empty()) {
      // Just reuse any of compiler options mainly for the correct IDE project
      // generation.
      linkerNode.CompilerOptions = target.ObjectListNodes[0].CompilerOptions;
    }
    target.LinkerNode.emplace_back(std::move(linkerNode));
  }
}

std::vector<FastbuildExecNode>
cmFastbuildNormalTargetGenerator::GetSymlinkExecs() const
{
  std::vector<FastbuildExecNode> res;
  cmGeneratorTarget::Names const targetNames = DetectOutput();
  LogMessage("targetNames.Real: " + targetNames.Real);
  LogMessage("targetNames.ImportOutput: " + targetNames.ImportOutput);
  LogMessage("targetNames.SharedObject: " + targetNames.SharedObject);
  LogMessage("targetNames.Base: " + targetNames.Base);

  std::string const targetOutput =
    ConvertToFastbuildPath(GeneratorTarget->GetFullPath(Config));
  std::string const targetOutputReal = ConvertToFastbuildPath(
    GeneratorTarget->GetFullPath(Config, cmStateEnums::RuntimeBinaryArtifact,
                                 /*realname=*/true));
  LogMessage("targetOutput: " + targetOutput);

  LogMessage("targetOutputReal: " + targetOutputReal);

  if (targetOutput != targetOutputReal &&
      !GeneratorTarget->IsFrameworkOnApple()) {
    auto const generateSymlinkCommand = [&](std::string const& from,
                                            std::string const& to) {
      if (from.empty() || to.empty() || from == to) {
        return;
      }
      LogMessage(cmStrCat("Symlinking ", from, " -> ", to));
      FastbuildExecNode postBuildExecNode;
      postBuildExecNode.Name = "cmake_symlink_" + to;
      postBuildExecNode.ExecOutput =
        cmJoin({ GeneratorTarget->GetDirectory(Config), to }, "/");
      postBuildExecNode.ExecExecutable = cmSystemTools::GetCMakeCommand();
      postBuildExecNode.ExecArguments = cmStrCat(
        "-E cmake_symlink_executable ",
        cmGlobalFastbuildGenerator::QuoteIfHasSpaces(from), ' ',
        cmGlobalFastbuildGenerator::QuoteIfHasSpaces(
          this->ConvertToFastbuildPath(postBuildExecNode.ExecOutput)));
      res.emplace_back(std::move(postBuildExecNode));
    };
    generateSymlinkCommand(targetNames.Real, targetNames.Output);
    generateSymlinkCommand(targetNames.Real, targetNames.SharedObject);
    generateSymlinkCommand(targetNames.ImportReal, targetNames.ImportOutput);
  }
  return res;
}
