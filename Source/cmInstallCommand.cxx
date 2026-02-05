/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallCommand.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/Glob.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCMakePath.h"
#include "cmExecutionStatus.h"
#include "cmExperimental.h"
#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmInstallAndroidMKExportGenerator.h"
#include "cmInstallCMakeConfigExportGenerator.h"
#include "cmInstallCommandArguments.h"
#include "cmInstallCxxModuleBmiGenerator.h"
#include "cmInstallDirectoryGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallGetRuntimeDependenciesGenerator.h"
#include "cmInstallImportedRuntimeArtifactsGenerator.h"
#include "cmInstallPackageInfoExportGenerator.h"
#include "cmInstallRuntimeDependencySet.h"
#include "cmInstallRuntimeDependencySetGenerator.h"
#include "cmInstallSbomExportGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPackageInfoArguments.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmSbomArguments.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"
#include "cmValue.h"

class cmListFileBacktrace;

namespace {

struct RuntimeDependenciesArgs
{
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Directories;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PreIncludeRegexes;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PreExcludeRegexes;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PostIncludeRegexes;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PostExcludeRegexes;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PostIncludeFiles;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> PostExcludeFiles;
};

auto const RuntimeDependenciesArgHelper =
  cmArgumentParser<RuntimeDependenciesArgs>{}
    .Bind("DIRECTORIES"_s, &RuntimeDependenciesArgs::Directories)
    .Bind("PRE_INCLUDE_REGEXES"_s, &RuntimeDependenciesArgs::PreIncludeRegexes)
    .Bind("PRE_EXCLUDE_REGEXES"_s, &RuntimeDependenciesArgs::PreExcludeRegexes)
    .Bind("POST_INCLUDE_REGEXES"_s,
          &RuntimeDependenciesArgs::PostIncludeRegexes)
    .Bind("POST_EXCLUDE_REGEXES"_s,
          &RuntimeDependenciesArgs::PostExcludeRegexes)
    .Bind("POST_INCLUDE_FILES"_s, &RuntimeDependenciesArgs::PostIncludeFiles)
    .Bind("POST_EXCLUDE_FILES"_s, &RuntimeDependenciesArgs::PostExcludeFiles);

class Helper
{
public:
  Helper(cmExecutionStatus& status)
    : Status(status)
    , Makefile(&status.GetMakefile())
  {
    this->DefaultComponentName = this->Makefile->GetSafeDefinition(
      "CMAKE_INSTALL_DEFAULT_COMPONENT_NAME");
    if (this->DefaultComponentName.empty()) {
      this->DefaultComponentName = "Unspecified";
    }
  }

  void SetError(std::string const& err) { this->Status.SetError(err); }

  bool MakeFilesFullPath(char const* modeName,
                         std::vector<std::string> const& relFiles,
                         std::vector<std::string>& absFiles);
  bool MakeFilesFullPath(char const* modeName, std::string const& basePath,
                         std::vector<std::string> const& relFiles,
                         std::vector<std::string>& absFiles);

  std::string GetDestination(cmInstallCommandArguments const* args,
                             std::string const& varName,
                             std::string const& guess) const;
  std::string GetRuntimeDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetSbinDestination(cmInstallCommandArguments const* args) const;
  std::string GetArchiveDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetLibraryDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetCxxModulesBmiDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetIncludeDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetSysconfDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetSharedStateDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetLocalStateDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetRunStateDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetDataRootDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetDataDestination(cmInstallCommandArguments const* args) const;
  std::string GetInfoDestination(cmInstallCommandArguments const* args) const;
  std::string GetLocaleDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetManDestination(cmInstallCommandArguments const* args) const;
  std::string GetDocDestination(cmInstallCommandArguments const* args) const;
  std::string GetProgramExecutablesDestination(
    cmInstallCommandArguments const* args) const;
  std::string GetDestinationForType(cmInstallCommandArguments const* args,
                                    std::string const& type) const;

  cmExecutionStatus& Status;
  cmMakefile* Makefile;
  std::string DefaultComponentName;
};

std::unique_ptr<cmInstallTargetGenerator> CreateInstallTargetGenerator(
  cmTarget& target, cmInstallCommandArguments const& args, bool impLib,
  cmListFileBacktrace const& backtrace, std::string const& destination,
  bool forceOpt = false, bool namelink = false)
{
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(target.GetMakefile());
  target.SetHaveInstallRule(true);
  std::string const& component =
    namelink ? args.GetNamelinkComponent() : args.GetComponent();
  auto g = cm::make_unique<cmInstallTargetGenerator>(
    target.GetName(), destination, impLib, args.GetPermissions(),
    args.GetConfigurations(), component, message, args.GetExcludeFromAll(),
    args.GetOptional() || forceOpt, backtrace);
  target.AddInstallGenerator(g.get());
  return g;
}

std::unique_ptr<cmInstallTargetGenerator> CreateInstallTargetGenerator(
  cmTarget& target, cmInstallCommandArguments const& args, bool impLib,
  cmListFileBacktrace const& backtrace, bool forceOpt = false,
  bool namelink = false)
{
  return CreateInstallTargetGenerator(target, args, impLib, backtrace,
                                      args.GetDestination(), forceOpt,
                                      namelink);
}

std::unique_ptr<cmInstallFilesGenerator> CreateInstallFilesGenerator(
  cmMakefile* mf, std::vector<std::string> const& absFiles,
  cmInstallCommandArguments const& args, bool programs,
  std::string const& destination)
{
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(mf);
  return cm::make_unique<cmInstallFilesGenerator>(
    absFiles, destination, programs, args.GetPermissions(),
    args.GetConfigurations(), args.GetComponent(), message,
    args.GetExcludeFromAll(), args.GetRename(), args.GetOptional(),
    mf->GetBacktrace());
}

std::unique_ptr<cmInstallFilesGenerator> CreateInstallFilesGenerator(
  cmMakefile* mf, std::vector<std::string> const& absFiles,
  cmInstallCommandArguments const& args, bool programs)
{
  return CreateInstallFilesGenerator(mf, absFiles, args, programs,
                                     args.GetDestination());
}

std::unique_ptr<cmInstallFileSetGenerator> CreateInstallFileSetGenerator(
  Helper& helper, cmTarget& target, cmFileSetDestinations dests,
  cmInstallCommandFileSetArguments const& args)
{
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);
  return cm::make_unique<cmInstallFileSetGenerator>(
    target.GetName(), args.GetFileSet(), std::move(dests),
    args.GetPermissions(), args.GetConfigurations(), args.GetComponent(),
    message, args.GetExcludeFromAll(), args.GetOptional(),
    helper.Makefile->GetBacktrace());
}

void AddInstallRuntimeDependenciesGenerator(
  Helper& helper, cmInstallRuntimeDependencySet* runtimeDependencySet,
  cmInstallCommandArguments const& runtimeArgs,
  cmInstallCommandArguments const& libraryArgs,
  cmInstallCommandArguments const& frameworkArgs,
  RuntimeDependenciesArgs runtimeDependenciesArgs, bool& installsRuntime,
  bool& installsLibrary, bool& installsFramework)
{
  bool dllPlatform =
    !helper.Makefile->GetSafeDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX").empty();
  bool apple =
    helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME") == "Darwin";
  auto const& runtimeDependenciesArgsRef =
    dllPlatform ? runtimeArgs : libraryArgs;
  std::vector<std::string> configurations =
    runtimeDependenciesArgsRef.GetConfigurations();
  if (apple) {
    std::copy(frameworkArgs.GetConfigurations().begin(),
              frameworkArgs.GetConfigurations().end(),
              std::back_inserter(configurations));
  }

  // Create file(GET_RUNTIME_DEPENDENCIES) generator.
  auto getRuntimeDependenciesGenerator =
    cm::make_unique<cmInstallGetRuntimeDependenciesGenerator>(
      runtimeDependencySet, std::move(runtimeDependenciesArgs.Directories),
      std::move(runtimeDependenciesArgs.PreIncludeRegexes),
      std::move(runtimeDependenciesArgs.PreExcludeRegexes),
      std::move(runtimeDependenciesArgs.PostIncludeRegexes),
      std::move(runtimeDependenciesArgs.PostExcludeRegexes),
      std::move(runtimeDependenciesArgs.PostIncludeFiles),
      std::move(runtimeDependenciesArgs.PostExcludeFiles),
      runtimeDependenciesArgsRef.GetComponent(),
      apple ? frameworkArgs.GetComponent() : "", true, "_CMAKE_DEPS",
      "_CMAKE_RPATH", configurations,
      cmInstallGenerator::SelectMessageLevel(helper.Makefile),
      runtimeDependenciesArgsRef.GetExcludeFromAll() &&
        (apple ? frameworkArgs.GetExcludeFromAll() : true),
      helper.Makefile->GetBacktrace(),
      helper.Makefile->GetPolicyStatus(cmPolicies::CMP0207));
  helper.Makefile->AddInstallGenerator(
    std::move(getRuntimeDependenciesGenerator));

  // Create the library dependencies generator.
  auto libraryRuntimeDependenciesGenerator =
    cm::make_unique<cmInstallRuntimeDependencySetGenerator>(
      cmInstallRuntimeDependencySetGenerator::DependencyType::Library,
      runtimeDependencySet, std::vector<std::string>{}, true, std::string{},
      true, "_CMAKE_DEPS", "_CMAKE_RPATH", "_CMAKE_TMP",
      dllPlatform ? helper.GetRuntimeDestination(&runtimeArgs)
                  : helper.GetLibraryDestination(&libraryArgs),
      runtimeDependenciesArgsRef.GetConfigurations(),
      runtimeDependenciesArgsRef.GetComponent(),
      runtimeDependenciesArgsRef.GetPermissions(),
      cmInstallGenerator::SelectMessageLevel(helper.Makefile),
      runtimeDependenciesArgsRef.GetExcludeFromAll(),
      helper.Makefile->GetBacktrace());
  helper.Makefile->AddInstallGenerator(
    std::move(libraryRuntimeDependenciesGenerator));
  if (dllPlatform) {
    installsRuntime = true;
  } else {
    installsLibrary = true;
  }

  if (apple) {
    // Create the framework dependencies generator.
    auto frameworkRuntimeDependenciesGenerator =
      cm::make_unique<cmInstallRuntimeDependencySetGenerator>(
        cmInstallRuntimeDependencySetGenerator::DependencyType::Framework,
        runtimeDependencySet, std::vector<std::string>{}, true, std::string{},
        true, "_CMAKE_DEPS", "_CMAKE_RPATH", "_CMAKE_TMP",
        frameworkArgs.GetDestination(), frameworkArgs.GetConfigurations(),
        frameworkArgs.GetComponent(), frameworkArgs.GetPermissions(),
        cmInstallGenerator::SelectMessageLevel(helper.Makefile),
        frameworkArgs.GetExcludeFromAll(), helper.Makefile->GetBacktrace());
    helper.Makefile->AddInstallGenerator(
      std::move(frameworkRuntimeDependenciesGenerator));
    installsFramework = true;
  }
}

std::set<std::string> const allowedTypes{
  "BIN",         "SBIN",       "LIB",      "INCLUDE", "SYSCONF",
  "SHAREDSTATE", "LOCALSTATE", "RUNSTATE", "DATA",    "INFO",
  "LOCALE",      "MAN",        "DOC",      "LIBEXEC",
};

template <typename T>
bool AddBundleExecutable(Helper& helper,
                         cmInstallRuntimeDependencySet* runtimeDependencySet,
                         T&& bundleExecutable)
{
  if (!runtimeDependencySet->AddBundleExecutable(bundleExecutable)) {
    helper.SetError(
      "A runtime dependency set may only have one bundle executable.");
    return false;
  }
  return true;
}

bool HandleScriptMode(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  Helper helper(status);

  std::string component = helper.DefaultComponentName;
  int componentCount = 0;
  bool doing_script = false;
  bool doing_code = false;
  bool exclude_from_all = false;
  bool all_components = false;

  // Scan the args once for COMPONENT. Only allow one.
  //
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "COMPONENT" && i + 1 < args.size()) {
      ++componentCount;
      ++i;
      component = args[i];
    }
    if (args[i] == "EXCLUDE_FROM_ALL") {
      exclude_from_all = true;
    } else if (args[i] == "ALL_COMPONENTS") {
      all_components = true;
    }
  }

  if (componentCount > 1) {
    status.SetError("given more than one COMPONENT for the SCRIPT or CODE "
                    "signature of the INSTALL command. "
                    "Use multiple INSTALL commands with one COMPONENT each.");
    return false;
  }

  if (all_components && componentCount == 1) {
    status.SetError("ALL_COMPONENTS and COMPONENT are mutually exclusive");
    return false;
  }

  // Scan the args again, this time adding install generators each time we
  // encounter a SCRIPT or CODE arg:
  //
  for (std::string const& arg : args) {
    if (arg == "SCRIPT") {
      doing_script = true;
      doing_code = false;
    } else if (arg == "CODE") {
      doing_script = false;
      doing_code = true;
    } else if (arg == "COMPONENT") {
      doing_script = false;
      doing_code = false;
    } else if (doing_script) {
      doing_script = false;
      std::string script = arg;
      if (!cmHasLiteralPrefix(script, "$<INSTALL_PREFIX>")) {
        if (!cmSystemTools::FileIsFullPath(script)) {
          script =
            cmStrCat(helper.Makefile->GetCurrentSourceDirectory(), '/', arg);
        }
        if (cmSystemTools::FileIsDirectory(script)) {
          status.SetError("given a directory as value of SCRIPT argument.");
          return false;
        }
      }
      helper.Makefile->AddInstallGenerator(
        cm::make_unique<cmInstallScriptGenerator>(
          script, false, component, exclude_from_all, all_components,
          helper.Makefile->GetBacktrace()));
    } else if (doing_code) {
      doing_code = false;
      std::string const& code = arg;
      helper.Makefile->AddInstallGenerator(
        cm::make_unique<cmInstallScriptGenerator>(
          code, true, component, exclude_from_all, all_components,
          helper.Makefile->GetBacktrace()));
    }
  }

  if (doing_script) {
    status.SetError("given no value for SCRIPT argument.");
    return false;
  }
  if (doing_code) {
    status.SetError("given no value for CODE argument.");
    return false;
  }

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(component);

  return true;
}

bool HandleTargetsMode(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the TARGETS mode.
  std::vector<cmTarget*> targets;

  struct ArgVectors
  {
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Archive;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Library;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Runtime;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Object;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Framework;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Bundle;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Includes;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PrivateHeader;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PublicHeader;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Resource;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> CxxModulesBmi;
    std::vector<std::vector<std::string>> FileSets;
  };

  static auto const argHelper =
    cmArgumentParser<ArgVectors>{}
      .Bind("ARCHIVE"_s, &ArgVectors::Archive)
      .Bind("LIBRARY"_s, &ArgVectors::Library)
      .Bind("RUNTIME"_s, &ArgVectors::Runtime)
      .Bind("OBJECTS"_s, &ArgVectors::Object)
      .Bind("FRAMEWORK"_s, &ArgVectors::Framework)
      .Bind("BUNDLE"_s, &ArgVectors::Bundle)
      .Bind("INCLUDES"_s, &ArgVectors::Includes)
      .Bind("PRIVATE_HEADER"_s, &ArgVectors::PrivateHeader)
      .Bind("PUBLIC_HEADER"_s, &ArgVectors::PublicHeader)
      .Bind("RESOURCE"_s, &ArgVectors::Resource)
      .Bind("FILE_SET"_s, &ArgVectors::FileSets)
      .Bind("CXX_MODULES_BMI"_s, &ArgVectors::CxxModulesBmi);

  std::vector<std::string> genericArgVector;
  ArgVectors const argVectors = argHelper.Parse(args, &genericArgVector);

  // now parse the generic args (i.e. the ones not specialized on LIBRARY/
  // ARCHIVE, RUNTIME etc. (see above)
  // These generic args also contain the targets and the export stuff
  ArgumentParser::MaybeEmpty<std::vector<std::string>> targetList;
  std::string exports;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>>
    runtimeDependenciesArgVector;
  std::string runtimeDependencySetArg;
  std::vector<std::string> unknownArgs;
  cmInstallCommandArguments genericArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  genericArgs.Bind("TARGETS"_s, targetList);
  genericArgs.Bind("EXPORT"_s, exports);
  genericArgs.Bind("RUNTIME_DEPENDENCIES"_s, runtimeDependenciesArgVector);
  genericArgs.Bind("RUNTIME_DEPENDENCY_SET"_s, runtimeDependencySetArg);
  genericArgs.Parse(genericArgVector, &unknownArgs);
  bool success = genericArgs.Finalize();

  RuntimeDependenciesArgs runtimeDependenciesArgs =
    runtimeDependenciesArgVector
    ? RuntimeDependenciesArgHelper.Parse(*runtimeDependenciesArgVector,
                                         &unknownArgs)
    : RuntimeDependenciesArgs();

  cmInstallCommandArguments archiveArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments libraryArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments runtimeArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments objectArgs(helper.DefaultComponentName,
                                       *helper.Makefile);
  cmInstallCommandArguments frameworkArgs(helper.DefaultComponentName,
                                          *helper.Makefile);
  cmInstallCommandArguments bundleArgs(helper.DefaultComponentName,
                                       *helper.Makefile);
  cmInstallCommandArguments privateHeaderArgs(helper.DefaultComponentName,
                                              *helper.Makefile);
  cmInstallCommandArguments publicHeaderArgs(helper.DefaultComponentName,
                                             *helper.Makefile);
  cmInstallCommandArguments resourceArgs(helper.DefaultComponentName,
                                         *helper.Makefile);
  cmInstallCommandIncludesArgument includesArgs;
  std::vector<cmInstallCommandFileSetArguments> fileSetArgs(
    argVectors.FileSets.size(),
    { cmInstallCommandFileSetArguments(helper.DefaultComponentName,
                                       *helper.Makefile) });
  cmInstallCommandArguments cxxModuleBmiArgs(helper.DefaultComponentName,
                                             *helper.Makefile);

  // now parse the args for specific parts of the target (e.g. LIBRARY,
  // RUNTIME, ARCHIVE etc.
  archiveArgs.Parse(argVectors.Archive, &unknownArgs);
  libraryArgs.Parse(argVectors.Library, &unknownArgs);
  runtimeArgs.Parse(argVectors.Runtime, &unknownArgs);
  objectArgs.Parse(argVectors.Object, &unknownArgs);
  frameworkArgs.Parse(argVectors.Framework, &unknownArgs);
  bundleArgs.Parse(argVectors.Bundle, &unknownArgs);
  privateHeaderArgs.Parse(argVectors.PrivateHeader, &unknownArgs);
  publicHeaderArgs.Parse(argVectors.PublicHeader, &unknownArgs);
  resourceArgs.Parse(argVectors.Resource, &unknownArgs);
  includesArgs.Parse(&argVectors.Includes, &unknownArgs);
  cxxModuleBmiArgs.Parse(argVectors.CxxModulesBmi, &unknownArgs);
  for (std::size_t i = 0; i < argVectors.FileSets.size(); i++) {
    // We have to create a separate object for the parsing because
    // cmArgumentParser<void>::Bind() binds to a specific address, but the
    // objects in the vector can move around. So we parse in an object with a
    // fixed address and then copy the data into the vector.
    cmInstallCommandFileSetArguments fileSetArg(helper.DefaultComponentName,
                                                *helper.Makefile);
    fileSetArg.Parse(argVectors.FileSets[i], &unknownArgs);
    fileSetArgs[i] = std::move(fileSetArg);
  }

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat("TARGETS given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  // apply generic args
  archiveArgs.SetGenericArguments(&genericArgs);
  libraryArgs.SetGenericArguments(&genericArgs);
  runtimeArgs.SetGenericArguments(&genericArgs);
  objectArgs.SetGenericArguments(&genericArgs);
  frameworkArgs.SetGenericArguments(&genericArgs);
  bundleArgs.SetGenericArguments(&genericArgs);
  privateHeaderArgs.SetGenericArguments(&genericArgs);
  publicHeaderArgs.SetGenericArguments(&genericArgs);
  resourceArgs.SetGenericArguments(&genericArgs);
  for (auto& fileSetArg : fileSetArgs) {
    fileSetArg.SetGenericArguments(&genericArgs);
  }
  cxxModuleBmiArgs.SetGenericArguments(&genericArgs);

  success = success && archiveArgs.Finalize();
  success = success && libraryArgs.Finalize();
  success = success && runtimeArgs.Finalize();
  success = success && objectArgs.Finalize();
  success = success && frameworkArgs.Finalize();
  success = success && bundleArgs.Finalize();
  success = success && privateHeaderArgs.Finalize();
  success = success && publicHeaderArgs.Finalize();
  success = success && resourceArgs.Finalize();
  success = success && cxxModuleBmiArgs.Finalize();
  for (auto& fileSetArg : fileSetArgs) {
    success = success && fileSetArg.Finalize();
  }

  if (!success) {
    return false;
  }

  // Enforce argument rules too complex to specify for the
  // general-purpose parser.
  if (runtimeArgs.GetNamelinkOnly() || objectArgs.GetNamelinkOnly() ||
      frameworkArgs.GetNamelinkOnly() || bundleArgs.GetNamelinkOnly() ||
      privateHeaderArgs.GetNamelinkOnly() ||
      publicHeaderArgs.GetNamelinkOnly() || resourceArgs.GetNamelinkOnly() ||
      std::any_of(fileSetArgs.begin(), fileSetArgs.end(),
                  [](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return fileSetArg.GetNamelinkOnly(); }) ||
      cxxModuleBmiArgs.GetNamelinkOnly()) {
    status.SetError(
      "TARGETS given NAMELINK_ONLY option not in LIBRARY or ARCHIVE group.  "
      "The NAMELINK_ONLY option may be specified only following LIBRARY or "
      "ARCHIVE.");
    return false;
  }
  if (runtimeArgs.GetNamelinkSkip() || objectArgs.GetNamelinkSkip() ||
      frameworkArgs.GetNamelinkSkip() || bundleArgs.GetNamelinkSkip() ||
      privateHeaderArgs.GetNamelinkSkip() ||
      publicHeaderArgs.GetNamelinkSkip() || resourceArgs.GetNamelinkSkip() ||
      std::any_of(fileSetArgs.begin(), fileSetArgs.end(),
                  [](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return fileSetArg.GetNamelinkSkip(); }) ||
      cxxModuleBmiArgs.GetNamelinkSkip()) {
    status.SetError(
      "TARGETS given NAMELINK_SKIP option not in LIBRARY or ARCHIVE group.  "
      "The NAMELINK_SKIP option may be specified only following LIBRARY or "
      "ARCHIVE.");
    return false;
  }
  if (runtimeArgs.HasNamelinkComponent() ||
      objectArgs.HasNamelinkComponent() ||
      frameworkArgs.HasNamelinkComponent() ||
      bundleArgs.HasNamelinkComponent() ||
      privateHeaderArgs.HasNamelinkComponent() ||
      publicHeaderArgs.HasNamelinkComponent() ||
      resourceArgs.HasNamelinkComponent() ||
      std::any_of(fileSetArgs.begin(), fileSetArgs.end(),
                  [](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return fileSetArg.HasNamelinkComponent(); }) ||
      cxxModuleBmiArgs.HasNamelinkComponent()) {
    status.SetError(
      "TARGETS given NAMELINK_COMPONENT option not in LIBRARY or ARCHIVE "
      "group.  The NAMELINK_COMPONENT option may be specified only following "
      "LIBRARY or ARCHIVE.");
    return false;
  }
  if (libraryArgs.GetNamelinkOnly() && libraryArgs.GetNamelinkSkip()) {
    status.SetError("TARGETS given NAMELINK_ONLY and NAMELINK_SKIP.  "
                    "At most one of these two options may be specified.");
    return false;
  }
  if (!genericArgs.GetType().empty() || !archiveArgs.GetType().empty() ||
      !libraryArgs.GetType().empty() || !runtimeArgs.GetType().empty() ||
      !objectArgs.GetType().empty() || !frameworkArgs.GetType().empty() ||
      !bundleArgs.GetType().empty() || !privateHeaderArgs.GetType().empty() ||
      !publicHeaderArgs.GetType().empty() || !resourceArgs.GetType().empty() ||
      std::any_of(fileSetArgs.begin(), fileSetArgs.end(),
                  [](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return !fileSetArg.GetType().empty(); }) ||
      !cxxModuleBmiArgs.GetType().empty()) {
    status.SetError(
      "TARGETS given TYPE option. The TYPE option may only be specified in "
      " install(FILES) and install(DIRECTORIES).");
    return false;
  }
  if (std::any_of(fileSetArgs.begin(), fileSetArgs.end(),
                  [](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return fileSetArg.GetFileSet().empty(); })) {
    status.SetError("TARGETS given FILE_SET option without file set name.");
    return false;
  }

  cmInstallRuntimeDependencySet* runtimeDependencySet = nullptr;
  if (runtimeDependenciesArgVector) {
    if (!runtimeDependencySetArg.empty()) {
      status.SetError("TARGETS cannot have both RUNTIME_DEPENDENCIES and "
                      "RUNTIME_DEPENDENCY_SET.");
      return false;
    }
    auto system = helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
    if (!cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
          system)) {
      status.SetError(
        cmStrCat("TARGETS RUNTIME_DEPENDENCIES is not supported on system \"",
                 system, '"'));
      return false;
    }
    if (helper.Makefile->IsOn("CMAKE_CROSSCOMPILING")) {
      status.SetError("TARGETS RUNTIME_DEPENDENCIES is not supported "
                      "when cross-compiling.");
      return false;
    }
    if (helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME") ==
          "Darwin" &&
        frameworkArgs.GetDestination().empty()) {
      status.SetError(
        "TARGETS RUNTIME_DEPENDENCIES given no FRAMEWORK DESTINATION");
      return false;
    }
    runtimeDependencySet = helper.Makefile->GetGlobalGenerator()
                             ->CreateAnonymousRuntimeDependencySet();
  } else if (!runtimeDependencySetArg.empty()) {
    auto system = helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
    if (!cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
          system)) {
      status.SetError(cmStrCat(
        "TARGETS RUNTIME_DEPENDENCY_SET is not supported on system \"", system,
        '"'));
      return false;
    }
    runtimeDependencySet =
      helper.Makefile->GetGlobalGenerator()->GetNamedRuntimeDependencySet(
        runtimeDependencySetArg);
  }

  // Select the mode for installing symlinks to versioned shared libraries.
  cmInstallTargetGenerator::NamelinkModeType namelinkMode =
    cmInstallTargetGenerator::NamelinkModeNone;
  if (libraryArgs.GetNamelinkOnly()) {
    namelinkMode = cmInstallTargetGenerator::NamelinkModeOnly;
  } else if (libraryArgs.GetNamelinkSkip()) {
    namelinkMode = cmInstallTargetGenerator::NamelinkModeSkip;
  }
  // Select the mode for installing symlinks to versioned imported libraries.
  cmInstallTargetGenerator::NamelinkModeType importlinkMode =
    cmInstallTargetGenerator::NamelinkModeNone;
  if (archiveArgs.GetNamelinkOnly()) {
    importlinkMode = cmInstallTargetGenerator::NamelinkModeOnly;
  } else if (archiveArgs.GetNamelinkSkip()) {
    importlinkMode = cmInstallTargetGenerator::NamelinkModeSkip;
  }

  // Check if there is something to do.
  if (targetList.empty()) {
    return true;
  }

  for (std::string const& tgt : targetList) {

    if (helper.Makefile->IsAlias(tgt)) {
      status.SetError(
        cmStrCat("TARGETS given target \"", tgt, "\" which is an alias."));
      return false;
    }
    // Lookup this target in the current directory.
    cmTarget* target = helper.Makefile->FindLocalNonAliasTarget(tgt);
    if (!target) {
      // If no local target has been found, find it in the global scope.
      cmTarget* const global_target =
        helper.Makefile->GetGlobalGenerator()->FindTarget(
          tgt, { cmStateEnums::TargetDomain::NATIVE });
      if (global_target && !global_target->IsImported()) {
        target = global_target;
      }
    }
    if (target) {
      // Found the target.  Check its type.
      if (target->GetType() != cmStateEnums::EXECUTABLE &&
          target->GetType() != cmStateEnums::STATIC_LIBRARY &&
          target->GetType() != cmStateEnums::SHARED_LIBRARY &&
          target->GetType() != cmStateEnums::MODULE_LIBRARY &&
          target->GetType() != cmStateEnums::OBJECT_LIBRARY &&
          target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          cmStrCat("TARGETS given target \"", tgt,
                   "\" which is not an executable, library, or module."));
        return false;
      }
      // Store the target in the list to be installed.
      targets.push_back(target);
    } else {
      // Did not find the target.
      status.SetError(
        cmStrCat("TARGETS given target \"", tgt, "\" which does not exist."));
      return false;
    }
  }

  // Keep track of whether we will be performing an installation of
  // any files of the given type.
  bool installsArchive = false;
  bool installsLibrary = false;
  bool installsNamelink = false;
  bool installsImportlink = false;
  bool installsRuntime = false;
  bool installsObject = false;
  bool installsFramework = false;
  bool installsBundle = false;
  bool installsPrivateHeader = false;
  bool installsPublicHeader = false;
  bool installsResource = false;
  std::vector<bool> installsFileSet(fileSetArgs.size(), false);
  bool installsCxxModuleBmi = false;

  // Generate install script code to install the given targets.
  for (cmTarget* ti : targets) {
    // Handle each target type.
    cmTarget& target = *ti;
    std::unique_ptr<cmInstallTargetGenerator> archiveGenerator;
    std::unique_ptr<cmInstallTargetGenerator> libraryGenerator;
    std::unique_ptr<cmInstallTargetGenerator> namelinkGenerator;
    std::unique_ptr<cmInstallTargetGenerator> importlinkGenerator;
    std::unique_ptr<cmInstallTargetGenerator> runtimeGenerator;
    std::unique_ptr<cmInstallTargetGenerator> objectGenerator;
    std::unique_ptr<cmInstallTargetGenerator> frameworkGenerator;
    std::unique_ptr<cmInstallTargetGenerator> bundleGenerator;
    std::unique_ptr<cmInstallFilesGenerator> privateHeaderGenerator;
    std::unique_ptr<cmInstallFilesGenerator> publicHeaderGenerator;
    std::unique_ptr<cmInstallFilesGenerator> resourceGenerator;
    std::vector<std::unique_ptr<cmInstallFileSetGenerator>> fileSetGenerators;
    std::unique_ptr<cmInstallCxxModuleBmiGenerator> cxxModuleBmiGenerator;

    // Avoid selecting default destinations for PUBLIC_HEADER and
    // PRIVATE_HEADER if any artifacts are specified.
    bool artifactsSpecified = false;

    // Track whether this is a namelink-only rule.
    bool namelinkOnly = false;

    auto addTargetExport = [&]() -> bool {
      // Add this install rule to an export if one was specified.
      if (!exports.empty()) {
        auto interfaceFileSets = target.GetAllInterfaceFileSets();
        if (std::any_of(
              interfaceFileSets.begin(), interfaceFileSets.end(),
              [=](std::string const& name) -> bool {
                return !std::any_of(
                  fileSetArgs.begin(), fileSetArgs.end(),
                  [=](cmInstallCommandFileSetArguments const& fileSetArg)
                    -> bool { return fileSetArg.GetFileSet() == name; });
              })) {
          status.SetError(cmStrCat("TARGETS target ", target.GetName(),
                                   " is exported but not all of its interface "
                                   "file sets are installed"));
          return false;
        }

        auto te = cm::make_unique<cmTargetExport>();
        te->TargetName = target.GetName();
        te->ArchiveGenerator = archiveGenerator.get();
        te->BundleGenerator = bundleGenerator.get();
        te->FrameworkGenerator = frameworkGenerator.get();
        te->HeaderGenerator = publicHeaderGenerator.get();
        te->LibraryGenerator = libraryGenerator.get();
        te->RuntimeGenerator = runtimeGenerator.get();
        te->ObjectsGenerator = objectGenerator.get();
        for (auto const& gen : fileSetGenerators) {
          te->FileSetGenerators[gen->GetFileSetName()] = gen.get();
        }
        te->CxxModuleBmiGenerator = cxxModuleBmiGenerator.get();
        target.AddInstallIncludeDirectories(
          *te, cmMakeRange(includesArgs.GetIncludeDirs()));
        te->NamelinkOnly = namelinkOnly;
        helper.Makefile->GetGlobalGenerator()
          ->GetExportSets()[exports]
          .AddTargetExport(std::move(te));
      }
      return true;
    };

    switch (target.GetType()) {
      case cmStateEnums::SHARED_LIBRARY: {
        // Shared libraries are handled differently on DLL and non-DLL
        // platforms.  All windows platforms are DLL platforms including
        // cygwin.  Currently no other platform is a DLL platform.
        if (target.IsDLLPlatform()) {
          // When in namelink only mode skip all libraries on Windows.
          if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
            namelinkOnly = true;
            if (!addTargetExport()) {
              return false;
            }
            continue;
          }

          // This is a DLL platform.
          if (!archiveArgs.GetDestination().empty()) {
            // The import library uses the ARCHIVE properties.
            archiveGenerator = CreateInstallTargetGenerator(
              target, archiveArgs, true, helper.Makefile->GetBacktrace());
            artifactsSpecified = true;
          }
          if (!runtimeArgs.GetDestination().empty()) {
            // The DLL uses the RUNTIME properties.
            runtimeGenerator = CreateInstallTargetGenerator(
              target, runtimeArgs, false, helper.Makefile->GetBacktrace());
            artifactsSpecified = true;
          }
          if (!archiveGenerator && !runtimeGenerator) {
            archiveGenerator = CreateInstallTargetGenerator(
              target, archiveArgs, true, helper.Makefile->GetBacktrace(),
              helper.GetArchiveDestination(nullptr));
            runtimeGenerator = CreateInstallTargetGenerator(
              target, runtimeArgs, false, helper.Makefile->GetBacktrace(),
              helper.GetRuntimeDestination(nullptr));
          }
          if (runtimeDependencySet && runtimeGenerator) {
            runtimeDependencySet->AddLibrary(runtimeGenerator.get());
          }
        } else {
          // This is a non-DLL platform.
          // If it is marked with FRAMEWORK property use the FRAMEWORK set of
          // INSTALL properties. Otherwise, use the LIBRARY properties.
          if (target.IsFrameworkOnApple()) {
            // When in namelink only mode skip frameworks.
            if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
              namelinkOnly = true;
              if (!addTargetExport()) {
                return false;
              }
              continue;
            }

            // Use the FRAMEWORK properties.
            if (!frameworkArgs.GetDestination().empty()) {
              frameworkGenerator = CreateInstallTargetGenerator(
                target, frameworkArgs, false, helper.Makefile->GetBacktrace());
            } else {
              status.SetError(
                cmStrCat("TARGETS given no FRAMEWORK DESTINATION for shared "
                         "library FRAMEWORK target \"",
                         target.GetName(), "\"."));
              return false;
            }
          } else {
            // The shared library uses the LIBRARY properties.
            if (!libraryArgs.GetDestination().empty()) {
              artifactsSpecified = true;
            }
            if (namelinkMode != cmInstallTargetGenerator::NamelinkModeOnly) {
              libraryGenerator = CreateInstallTargetGenerator(
                target, libraryArgs, false, helper.Makefile->GetBacktrace(),
                helper.GetLibraryDestination(&libraryArgs));
              libraryGenerator->SetNamelinkMode(
                cmInstallTargetGenerator::NamelinkModeSkip);
            }
            if (namelinkMode != cmInstallTargetGenerator::NamelinkModeSkip) {
              namelinkGenerator = CreateInstallTargetGenerator(
                target, libraryArgs, false, helper.Makefile->GetBacktrace(),
                helper.GetLibraryDestination(&libraryArgs), false, true);
              namelinkGenerator->SetNamelinkMode(
                cmInstallTargetGenerator::NamelinkModeOnly);
            }
            namelinkOnly =
              (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly);

            if (target.GetMakefile()->PlatformSupportsAppleTextStubs() &&
                target.IsSharedLibraryWithExports()) {
              // Apple .tbd files use the ARCHIVE properties
              if (!archiveArgs.GetDestination().empty()) {
                artifactsSpecified = true;
              }
              if (importlinkMode !=
                  cmInstallTargetGenerator::NamelinkModeOnly) {
                archiveGenerator = CreateInstallTargetGenerator(
                  target, archiveArgs, true, helper.Makefile->GetBacktrace(),
                  helper.GetLibraryDestination(&archiveArgs));
                archiveGenerator->SetImportlinkMode(
                  cmInstallTargetGenerator::NamelinkModeSkip);
              }
              if (importlinkMode !=
                  cmInstallTargetGenerator::NamelinkModeSkip) {
                importlinkGenerator = CreateInstallTargetGenerator(
                  target, archiveArgs, true, helper.Makefile->GetBacktrace(),
                  helper.GetLibraryDestination(&archiveArgs), false, true);
                importlinkGenerator->SetImportlinkMode(
                  cmInstallTargetGenerator::NamelinkModeOnly);
              }
              namelinkOnly =
                (importlinkMode == cmInstallTargetGenerator::NamelinkModeOnly);
            }
          }
          if (runtimeDependencySet && libraryGenerator) {
            runtimeDependencySet->AddLibrary(libraryGenerator.get());
          }
        }
      } break;
      case cmStateEnums::STATIC_LIBRARY: {
        // If it is marked with FRAMEWORK property use the FRAMEWORK set of
        // INSTALL properties. Otherwise, use the LIBRARY properties.
        if (target.IsFrameworkOnApple()) {
          // When in namelink only mode skip frameworks.
          if (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly) {
            namelinkOnly = true;
            if (!addTargetExport()) {
              return false;
            }
            continue;
          }

          // Use the FRAMEWORK properties.
          if (!frameworkArgs.GetDestination().empty()) {
            frameworkGenerator = CreateInstallTargetGenerator(
              target, frameworkArgs, false, helper.Makefile->GetBacktrace());
          } else {
            status.SetError(
              cmStrCat("TARGETS given no FRAMEWORK DESTINATION for static "
                       "library FRAMEWORK target \"",
                       target.GetName(), "\"."));
            return false;
          }
        } else {
          // Static libraries use ARCHIVE properties.
          if (!archiveArgs.GetDestination().empty()) {
            artifactsSpecified = true;
          }
          archiveGenerator = CreateInstallTargetGenerator(
            target, archiveArgs, false, helper.Makefile->GetBacktrace(),
            helper.GetArchiveDestination(&archiveArgs));
        }
      } break;
      case cmStateEnums::MODULE_LIBRARY: {
        // Modules use LIBRARY properties.
        if (!libraryArgs.GetDestination().empty()) {
          libraryGenerator = CreateInstallTargetGenerator(
            target, libraryArgs, false, helper.Makefile->GetBacktrace());
          libraryGenerator->SetNamelinkMode(namelinkMode);
          namelinkOnly =
            (namelinkMode == cmInstallTargetGenerator::NamelinkModeOnly);
          if (runtimeDependencySet) {
            runtimeDependencySet->AddModule(libraryGenerator.get());
          }
        } else {
          status.SetError(
            cmStrCat("TARGETS given no LIBRARY DESTINATION for module "
                     "target \"",
                     target.GetName(), "\"."));
          return false;
        }
      } break;
      case cmStateEnums::OBJECT_LIBRARY: {
        // Objects use OBJECT properties.
        if (!objectArgs.GetDestination().empty()) {
          // Verify that we know where the objects are to install them.
          std::string reason;
          if (!target.HasKnownObjectFileLocation(&reason)) {
            status.SetError(
              cmStrCat("TARGETS given OBJECT library \"", target.GetName(),
                       "\" whose objects may not be installed", reason, '.'));
            return false;
          }

          objectGenerator = CreateInstallTargetGenerator(
            target, objectArgs, false, helper.Makefile->GetBacktrace());
        } else {
          // Installing an OBJECT library without a destination transforms
          // it to an INTERFACE library.  It installs no files but can be
          // exported.
        }
      } break;
      case cmStateEnums::EXECUTABLE: {
        if (target.IsAppBundleOnApple()) {
          // Application bundles use the BUNDLE properties.
          if (!bundleArgs.GetDestination().empty()) {
            bundleGenerator = CreateInstallTargetGenerator(
              target, bundleArgs, false, helper.Makefile->GetBacktrace());
          }
          if (!bundleGenerator) {
            status.SetError(cmStrCat("TARGETS given no BUNDLE DESTINATION for "
                                     "MACOSX_BUNDLE executable target \"",
                                     target.GetName(), "\"."));
            return false;
          }
          if (runtimeDependencySet) {
            if (!AddBundleExecutable(helper, runtimeDependencySet,
                                     bundleGenerator.get())) {
              return false;
            }
          }
        } else {
          // Executables use the RUNTIME properties.
          if (!runtimeArgs.GetDestination().empty()) {
            artifactsSpecified = true;
          }
          runtimeGenerator = CreateInstallTargetGenerator(
            target, runtimeArgs, false, helper.Makefile->GetBacktrace(),
            helper.GetRuntimeDestination(&runtimeArgs));
          if (runtimeDependencySet) {
            runtimeDependencySet->AddExecutable(runtimeGenerator.get());
          }
        }

        // On DLL platforms an executable may also have an import
        // library.  Install it to the archive destination if it
        // exists.
        if ((target.IsDLLPlatform() || target.IsAIX()) &&
            !archiveArgs.GetDestination().empty() &&
            target.IsExecutableWithExports()) {
          // The import library uses the ARCHIVE properties.
          artifactsSpecified = true;
          archiveGenerator = CreateInstallTargetGenerator(
            target, archiveArgs, true, helper.Makefile->GetBacktrace(), true);
        }
      } break;
      case cmStateEnums::INTERFACE_LIBRARY:
        // Nothing to do. An INTERFACE_LIBRARY can be installed, but the
        // only effect of that is to make it exportable. It installs no
        // other files itself.
      default:
        // This should never happen due to the above type check.
        // Ignore the case.
        break;
    }

    // These well-known sets of files are installed *automatically* for
    // FRAMEWORK SHARED library targets on the Mac as part of installing the
    // FRAMEWORK.  For other target types or on other platforms, they are not
    // installed automatically and so we need to create install files
    // generators for them.
    bool createInstallGeneratorsForTargetFileSets = true;

    if (target.IsFrameworkOnApple()) {
      createInstallGeneratorsForTargetFileSets = false;
    }

    if (createInstallGeneratorsForTargetFileSets && !namelinkOnly) {
      cmValue files = target.GetProperty("PRIVATE_HEADER");
      if (cmNonempty(files)) {
        cmList relFiles{ *files };
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("PRIVATE_HEADER", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!artifactsSpecified ||
            !privateHeaderArgs.GetDestination().empty()) {
          privateHeaderGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, privateHeaderArgs, false,
            helper.GetIncludeDestination(&privateHeaderArgs));
        } else {
          std::ostringstream e;
          e << "Target " << target.GetName() << " has "
            << "PRIVATE_HEADER files but no PRIVATE_HEADER DESTINATION.";
          helper.Makefile->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        }
      }

      files = target.GetProperty("PUBLIC_HEADER");
      if (cmNonempty(files)) {
        cmList relFiles{ *files };
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("PUBLIC_HEADER", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!artifactsSpecified ||
            !publicHeaderArgs.GetDestination().empty()) {
          publicHeaderGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, publicHeaderArgs, false,
            helper.GetIncludeDestination(&publicHeaderArgs));
        } else {
          std::ostringstream e;
          e << "Target " << target.GetName() << " has "
            << "PUBLIC_HEADER files but no PUBLIC_HEADER DESTINATION.";
          helper.Makefile->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        }
      }

      files = target.GetProperty("RESOURCE");
      if (cmNonempty(files)) {
        cmList relFiles{ *files };
        std::vector<std::string> absFiles;
        if (!helper.MakeFilesFullPath("RESOURCE", relFiles, absFiles)) {
          return false;
        }

        // Create the files install generator.
        if (!resourceArgs.GetDestination().empty()) {
          resourceGenerator = CreateInstallFilesGenerator(
            helper.Makefile, absFiles, resourceArgs, false);
        } else if (!target.IsAppBundleOnApple()) {
          helper.Makefile->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmStrCat("Target ", target.GetName(),
                     " has RESOURCE files but no RESOURCE DESTINATION."));
        }
      }
    }

    if (!namelinkOnly) {
      for (std::size_t i = 0; i < fileSetArgs.size(); i++) {
        cmFileSetDestinations dests;
        dests.Headers = helper.GetIncludeDestination(&fileSetArgs[i]);
        dests.CXXModules = fileSetArgs[i].GetDestination();
        fileSetGenerators.push_back(CreateInstallFileSetGenerator(
          helper, target, std::move(dests), fileSetArgs[i]));
        installsFileSet[i] = true;
      }
    }

    if (!cxxModuleBmiArgs.GetDestination().empty()) {
      cxxModuleBmiGenerator = cm::make_unique<cmInstallCxxModuleBmiGenerator>(
        target.GetName(),
        helper.GetCxxModulesBmiDestination(&cxxModuleBmiArgs),
        cxxModuleBmiArgs.GetPermissions(),
        cxxModuleBmiArgs.GetConfigurations(), cxxModuleBmiArgs.GetComponent(),
        cmInstallGenerator::SelectMessageLevel(target.GetMakefile()),
        cxxModuleBmiArgs.GetExcludeFromAll(), cxxModuleBmiArgs.GetOptional(),
        helper.Makefile->GetBacktrace());
      target.SetHaveInstallRule(true);
    }

    // Add this install rule to an export if one was specified.
    if (!addTargetExport()) {
      return false;
    }

    // Keep track of whether we're installing anything in each category
    installsArchive = installsArchive || archiveGenerator;
    installsLibrary = installsLibrary || libraryGenerator;
    installsNamelink = installsNamelink || namelinkGenerator;
    installsImportlink = installsImportlink || importlinkGenerator;
    installsRuntime = installsRuntime || runtimeGenerator;
    installsObject = installsObject || objectGenerator;
    installsFramework = installsFramework || frameworkGenerator;
    installsBundle = installsBundle || bundleGenerator;
    installsPrivateHeader = installsPrivateHeader || privateHeaderGenerator;
    installsPublicHeader = installsPublicHeader || publicHeaderGenerator;
    installsResource = installsResource || resourceGenerator;
    installsCxxModuleBmi = installsCxxModuleBmi || cxxModuleBmiGenerator;

    helper.Makefile->AddInstallGenerator(std::move(archiveGenerator));
    helper.Makefile->AddInstallGenerator(std::move(libraryGenerator));
    helper.Makefile->AddInstallGenerator(std::move(namelinkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(importlinkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(runtimeGenerator));
    helper.Makefile->AddInstallGenerator(std::move(objectGenerator));
    helper.Makefile->AddInstallGenerator(std::move(frameworkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(bundleGenerator));
    helper.Makefile->AddInstallGenerator(std::move(privateHeaderGenerator));
    helper.Makefile->AddInstallGenerator(std::move(publicHeaderGenerator));
    helper.Makefile->AddInstallGenerator(std::move(resourceGenerator));
    for (auto& gen : fileSetGenerators) {
      helper.Makefile->AddInstallGenerator(std::move(gen));
    }
    helper.Makefile->AddInstallGenerator(std::move(cxxModuleBmiGenerator));
  }

  if (runtimeDependenciesArgVector && !runtimeDependencySet->Empty()) {
    AddInstallRuntimeDependenciesGenerator(
      helper, runtimeDependencySet, runtimeArgs, libraryArgs, frameworkArgs,
      std::move(runtimeDependenciesArgs), installsRuntime, installsLibrary,
      installsFramework);
  }

  // Tell the global generator about any installation component names
  // specified
  if (installsArchive) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      archiveArgs.GetComponent());
  }
  if (installsLibrary) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetComponent());
  }
  if (installsNamelink) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetNamelinkComponent());
  }
  if (installsImportlink) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      archiveArgs.GetNamelinkComponent());
  }
  if (installsRuntime) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      runtimeArgs.GetComponent());
  }
  if (installsObject) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      objectArgs.GetComponent());
  }
  if (installsFramework) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      frameworkArgs.GetComponent());
  }
  if (installsBundle) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      bundleArgs.GetComponent());
  }
  if (installsPrivateHeader) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      privateHeaderArgs.GetComponent());
  }
  if (installsPublicHeader) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      publicHeaderArgs.GetComponent());
  }
  if (installsResource) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      resourceArgs.GetComponent());
  }
  for (std::size_t i = 0; i < fileSetArgs.size(); i++) {
    if (installsFileSet[i]) {
      helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
        fileSetArgs[i].GetComponent());
    }
  }
  if (installsCxxModuleBmi) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      cxxModuleBmiArgs.GetComponent());
  }

  return true;
}

bool HandleImportedRuntimeArtifactsMode(std::vector<std::string> const& args,
                                        cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the IMPORTED_RUNTIME_ARTIFACTS mode.
  std::vector<cmTarget*> targets;

  struct ArgVectors
  {
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Library;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Runtime;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Framework;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Bundle;
  };

  static auto const argHelper = cmArgumentParser<ArgVectors>{}
                                  .Bind("LIBRARY"_s, &ArgVectors::Library)
                                  .Bind("RUNTIME"_s, &ArgVectors::Runtime)
                                  .Bind("FRAMEWORK"_s, &ArgVectors::Framework)
                                  .Bind("BUNDLE"_s, &ArgVectors::Bundle);

  std::vector<std::string> genericArgVector;
  ArgVectors const argVectors = argHelper.Parse(args, &genericArgVector);

  // now parse the generic args (i.e. the ones not specialized on LIBRARY,
  // RUNTIME etc. (see above)
  ArgumentParser::MaybeEmpty<std::vector<std::string>> targetList;
  std::string runtimeDependencySetArg;
  std::vector<std::string> unknownArgs;
  cmInstallCommandArguments genericArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  genericArgs.Bind("IMPORTED_RUNTIME_ARTIFACTS"_s, targetList)
    .Bind("RUNTIME_DEPENDENCY_SET"_s, runtimeDependencySetArg);
  genericArgs.Parse(genericArgVector, &unknownArgs);
  bool success = genericArgs.Finalize();

  cmInstallCommandArguments libraryArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments runtimeArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments frameworkArgs(helper.DefaultComponentName,
                                          *helper.Makefile);
  cmInstallCommandArguments bundleArgs(helper.DefaultComponentName,
                                       *helper.Makefile);

  // now parse the args for specific parts of the target (e.g. LIBRARY,
  // RUNTIME etc.
  libraryArgs.Parse(argVectors.Library, &unknownArgs);
  runtimeArgs.Parse(argVectors.Runtime, &unknownArgs);
  frameworkArgs.Parse(argVectors.Framework, &unknownArgs);
  bundleArgs.Parse(argVectors.Bundle, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given unknown argument \"",
               unknownArgs[0], "\"."));
    return false;
  }

  // apply generic args
  libraryArgs.SetGenericArguments(&genericArgs);
  runtimeArgs.SetGenericArguments(&genericArgs);
  frameworkArgs.SetGenericArguments(&genericArgs);
  bundleArgs.SetGenericArguments(&genericArgs);

  success = success && libraryArgs.Finalize();
  success = success && runtimeArgs.Finalize();
  success = success && frameworkArgs.Finalize();
  success = success && bundleArgs.Finalize();

  if (!success) {
    return false;
  }

  cmInstallRuntimeDependencySet* runtimeDependencySet = nullptr;
  if (!runtimeDependencySetArg.empty()) {
    auto system = helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
    if (!cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
          system)) {
      status.SetError(
        cmStrCat("IMPORTED_RUNTIME_ARTIFACTS RUNTIME_DEPENDENCY_SET is not "
                 "supported on system \"",
                 system, '"'));
      return false;
    }
    runtimeDependencySet =
      helper.Makefile->GetGlobalGenerator()->GetNamedRuntimeDependencySet(
        runtimeDependencySetArg);
  }

  // Check if there is something to do.
  if (targetList.empty()) {
    return true;
  }

  for (std::string const& tgt : targetList) {
    if (helper.Makefile->IsAlias(tgt)) {
      status.SetError(cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given target \"",
                               tgt, "\" which is an alias."));
      return false;
    }
    // Lookup this target in the current directory.
    cmTarget* target = helper.Makefile->FindTargetToUse(tgt);
    if (!target || !target->IsImported()) {
      // If no local target has been found, find it in the global scope.
      cmTarget* const global_target =
        helper.Makefile->GetGlobalGenerator()->FindTarget(
          tgt, { cmStateEnums::TargetDomain::NATIVE });
      if (global_target && global_target->IsImported()) {
        target = global_target;
      }
    }
    if (target) {
      // Found the target.  Check its type.
      if (target->GetType() != cmStateEnums::EXECUTABLE &&
          target->GetType() != cmStateEnums::SHARED_LIBRARY &&
          target->GetType() != cmStateEnums::MODULE_LIBRARY) {
        status.SetError(
          cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given target \"", tgt,
                   "\" which is not an executable, library, or module."));
        return false;
      }
      // Store the target in the list to be installed.
      targets.push_back(target);
    } else {
      // Did not find the target.
      status.SetError(cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given target \"",
                               tgt, "\" which does not exist."));
      return false;
    }
  }

  // Keep track of whether we will be performing an installation of
  // any files of the given type.
  bool installsLibrary = false;
  bool installsRuntime = false;
  bool installsFramework = false;
  bool installsBundle = false;

  auto const createInstallGenerator =
    [helper](cmTarget& target, cmInstallCommandArguments const& typeArgs,
             std::string const& destination)
    -> std::unique_ptr<cmInstallImportedRuntimeArtifactsGenerator> {
    return cm::make_unique<cmInstallImportedRuntimeArtifactsGenerator>(
      target.GetName(), destination, typeArgs.GetPermissions(),
      typeArgs.GetConfigurations(), typeArgs.GetComponent(),
      cmInstallGenerator::SelectMessageLevel(helper.Makefile),
      typeArgs.GetExcludeFromAll(), typeArgs.GetOptional(),
      helper.Makefile->GetBacktrace());
  };

  // Generate install script code to install the given targets.
  for (cmTarget* ti : targets) {
    // Handle each target type.
    cmTarget& target = *ti;
    std::unique_ptr<cmInstallImportedRuntimeArtifactsGenerator>
      libraryGenerator;
    std::unique_ptr<cmInstallImportedRuntimeArtifactsGenerator>
      runtimeGenerator;
    std::unique_ptr<cmInstallImportedRuntimeArtifactsGenerator>
      frameworkGenerator;
    std::unique_ptr<cmInstallImportedRuntimeArtifactsGenerator>
      bundleGenerator;

    switch (target.GetType()) {
      case cmStateEnums::SHARED_LIBRARY:
        if (target.IsDLLPlatform()) {
          runtimeGenerator = createInstallGenerator(
            target, runtimeArgs, helper.GetRuntimeDestination(&runtimeArgs));
          if (runtimeDependencySet) {
            runtimeDependencySet->AddLibrary(runtimeGenerator.get());
          }
        } else if (target.IsFrameworkOnApple()) {
          if (frameworkArgs.GetDestination().empty()) {
            status.SetError(cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given no "
                                     "FRAMEWORK DESTINATION for shared "
                                     "library FRAMEWORK target \"",
                                     target.GetName(), "\"."));
            return false;
          }
          frameworkGenerator = createInstallGenerator(
            target, frameworkArgs, frameworkArgs.GetDestination());
          if (runtimeDependencySet) {
            runtimeDependencySet->AddLibrary(frameworkGenerator.get());
          }
        } else {
          libraryGenerator = createInstallGenerator(
            target, libraryArgs, helper.GetLibraryDestination(&libraryArgs));
          if (runtimeDependencySet) {
            runtimeDependencySet->AddLibrary(libraryGenerator.get());
          }
        }
        break;
      case cmStateEnums::MODULE_LIBRARY:
        libraryGenerator = createInstallGenerator(
          target, libraryArgs, helper.GetLibraryDestination(&libraryArgs));
        if (runtimeDependencySet) {
          runtimeDependencySet->AddModule(libraryGenerator.get());
        }
        break;
      case cmStateEnums::EXECUTABLE:
        if (target.IsAppBundleOnApple()) {
          if (bundleArgs.GetDestination().empty()) {
            status.SetError(
              cmStrCat("IMPORTED_RUNTIME_ARTIFACTS given no BUNDLE "
                       "DESTINATION for MACOSX_BUNDLE executable target \"",
                       target.GetName(), "\"."));
            return false;
          }
          bundleGenerator = createInstallGenerator(
            target, bundleArgs, bundleArgs.GetDestination());
          if (runtimeDependencySet) {
            if (!AddBundleExecutable(helper, runtimeDependencySet,
                                     bundleGenerator.get())) {
              return false;
            }
          }
        } else {
          runtimeGenerator = createInstallGenerator(
            target, runtimeArgs, helper.GetRuntimeDestination(&runtimeArgs));
          if (runtimeDependencySet) {
            runtimeDependencySet->AddExecutable(runtimeGenerator.get());
          }
        }
        break;
      default:
        assert(false && "This should never happen");
        break;
    }

    // Keep track of whether we're installing anything in each category
    installsLibrary = installsLibrary || libraryGenerator;
    installsRuntime = installsRuntime || runtimeGenerator;
    installsFramework = installsFramework || frameworkGenerator;
    installsBundle = installsBundle || bundleGenerator;

    helper.Makefile->AddInstallGenerator(std::move(libraryGenerator));
    helper.Makefile->AddInstallGenerator(std::move(runtimeGenerator));
    helper.Makefile->AddInstallGenerator(std::move(frameworkGenerator));
    helper.Makefile->AddInstallGenerator(std::move(bundleGenerator));
  }

  // Tell the global generator about any installation component names
  // specified
  if (installsLibrary) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetComponent());
  }
  if (installsRuntime) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      runtimeArgs.GetComponent());
  }
  if (installsFramework) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      frameworkArgs.GetComponent());
  }
  if (installsBundle) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      bundleArgs.GetComponent());
  }

  return true;
}

bool HandleFilesMode(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the FILES mode.
  bool programs = (args[0] == "PROGRAMS");
  cmInstallCommandArguments ica(helper.DefaultComponentName, *helper.Makefile);
  ArgumentParser::MaybeEmpty<std::vector<std::string>> files;
  ica.Bind(programs ? "PROGRAMS"_s : "FILES"_s, files);
  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  std::string type = ica.GetType();
  if (!type.empty() && allowedTypes.count(type) == 0) {
    status.SetError(
      cmStrCat(args[0], " given non-type \"", type, "\" with TYPE argument."));
    return false;
  }

  std::vector<std::string> const& filesVector = files;

  // Check if there is something to do.
  if (filesVector.empty()) {
    return true;
  }

  if (!ica.GetRename().empty() && filesVector.size() > 1) {
    // The rename option works only with one file.
    status.SetError(
      cmStrCat(args[0], " given RENAME option with more than one file."));
    return false;
  }

  std::vector<std::string> absFiles;
  if (!helper.MakeFilesFullPath(args[0].c_str(), filesVector, absFiles)) {
    return false;
  }

  cmGlobalGenerator* gg = helper.Makefile->GetGlobalGenerator();
  for (std::string const& file : filesVector) {
    if (gg->IsExportedTargetsFile(file)) {
      helper.Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("The file\n  ", file,
                 "\n"
                 "was generated by the export() command.  "
                 "It may not be installed with the install() command.  "
                 "Use the install(EXPORT) mechanism instead.  "
                 "See the cmake-packages(7) manual for more."));
      return false;
    }
  }

  if (!ica.Finalize()) {
    return false;
  }

  if (!type.empty() && !ica.GetDestination().empty()) {
    status.SetError(cmStrCat(args[0],
                             " given both TYPE and DESTINATION arguments. "
                             "You may only specify one."));
    return false;
  }

  std::string destination = helper.GetDestinationForType(&ica, type);
  if (destination.empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Create the files install generator.
  helper.Makefile->AddInstallGenerator(CreateInstallFilesGenerator(
    helper.Makefile, absFiles, ica, programs, destination));

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());

  return true;
}

bool HandleDirectoryMode(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  Helper helper(status);

  enum Doing
  {
    DoingNone,
    DoingDirs,
    DoingDestination,
    DoingPattern,
    DoingRegex,
    DoingPermsFile,
    DoingPermsDir,
    DoingPermsMatch,
    DoingConfigurations,
    DoingComponent,
    DoingType
  };
  Doing doing = DoingDirs;
  bool in_match_mode = false;
  bool optional = false;
  bool exclude_from_all = false;
  bool message_never = false;
  std::vector<std::string> dirs;
  cm::optional<std::string> destination;
  std::string permissions_file;
  std::string permissions_dir;
  std::vector<std::string> configurations;
  std::string component = helper.DefaultComponentName;
  std::string literal_args;
  std::string type;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "DESTINATION") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the destination property.
      doing = DoingDestination;
    } else if (args[i] == "TYPE") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the type.
      doing = DoingType;
    } else if (args[i] == "OPTIONAL") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Mark the rule as optional.
      optional = true;
      doing = DoingNone;
    } else if (args[i] == "MESSAGE_NEVER") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Mark the rule as quiet.
      message_never = true;
      doing = DoingNone;
    } else if (args[i] == "PATTERN") {
      // Switch to a new pattern match rule.
      doing = DoingPattern;
      in_match_mode = true;
    } else if (args[i] == "REGEX") {
      // Switch to a new regex match rule.
      doing = DoingRegex;
      in_match_mode = true;
    } else if (args[i] == "EXCLUDE") {
      // Add this property to the current match rule.
      if (!in_match_mode || doing == DoingPattern || doing == DoingRegex) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" before a PATTERN or REGEX is given."));
        return false;
      }
      literal_args += " EXCLUDE";
      doing = DoingNone;
    } else if (args[i] == "PERMISSIONS") {
      if (!in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" before a PATTERN or REGEX is given."));
        return false;
      }

      // Switch to setting the current match permissions property.
      literal_args += " PERMISSIONS";
      doing = DoingPermsMatch;
    } else if (args[i] == "FILE_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the file permissions property.
      doing = DoingPermsFile;
    } else if (args[i] == "DIRECTORY_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the directory permissions property.
      doing = DoingPermsDir;
    } else if (args[i] == "USE_SOURCE_PERMISSIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Add this option literally.
      literal_args += " USE_SOURCE_PERMISSIONS";
      doing = DoingNone;
    } else if (args[i] == "FILES_MATCHING") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Add this option literally.
      literal_args += " FILES_MATCHING";
      doing = DoingNone;
    } else if (args[i] == "CONFIGURATIONS") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the configurations property.
      doing = DoingConfigurations;
    } else if (args[i] == "COMPONENT") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }

      // Switch to setting the component property.
      doing = DoingComponent;
    } else if (args[i] == "EXCLUDE_FROM_ALL") {
      if (in_match_mode) {
        status.SetError(cmStrCat(args[0], " does not allow \"", args[i],
                                 "\" after PATTERN or REGEX."));
        return false;
      }
      exclude_from_all = true;
      doing = DoingNone;
    } else if (doing == DoingDirs) {
      // Convert this directory to a full path.
      std::string dir = args[i];
      std::string::size_type gpos = cmGeneratorExpression::Find(dir);
      if (gpos != 0 && !cmSystemTools::FileIsFullPath(dir)) {
        dir =
          cmStrCat(helper.Makefile->GetCurrentSourceDirectory(), '/', args[i]);
      }

      // Make sure the name is a directory.
      if (cmSystemTools::FileExists(dir, true)) {
        status.SetError(cmStrCat(args[0], " given non-directory \"", args[i],
                                 "\" to install."));
        return false;
      }

      // Store the directory for installation.
      dirs.push_back(std::move(dir));
    } else if (doing == DoingConfigurations) {
      configurations.push_back(args[i]);
    } else if (doing == DoingDestination) {
      // A trailing slash is meaningful for this form, but normalization
      // preserves it if present
      switch (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0177)) {
        case cmPolicies::NEW:
          destination = cmCMakePath(args[i]).Normal().String();
          break;
        case cmPolicies::WARN:
          // We can't be certain if a warning is appropriate if there are any
          // generator expressions
          if (cmGeneratorExpression::Find(args[i]) == cm::string_view::npos &&
              args[i] != cmCMakePath(args[i]).Normal().String()) {
            status.GetMakefile().IssueMessage(
              MessageType::AUTHOR_WARNING,
              cmPolicies::GetPolicyWarning(cmPolicies::CMP0177));
          }
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          destination = args[i];
          break;
      }
      doing = DoingNone;
    } else if (doing == DoingType) {
      if (allowedTypes.count(args[i]) == 0) {
        status.SetError(cmStrCat(args[0], " given non-type \"", args[i],
                                 "\" with TYPE argument."));
        return false;
      }

      type = args[i];
      doing = DoingNone;
    } else if (doing == DoingPattern) {
      // Convert the pattern to a regular expression.  Require a
      // leading slash and trailing end-of-string in the matched
      // string to make sure the pattern matches only whole file
      // names.
      literal_args += " REGEX \"/";
      std::string regex = cmsys::Glob::PatternToRegex(args[i], false);
      cmSystemTools::ReplaceString(regex, "\\", "\\\\");
      literal_args += regex;
      literal_args += "$\"";
      doing = DoingNone;
    } else if (doing == DoingRegex) {
      literal_args += " REGEX \"";
// Match rules are case-insensitive on some platforms.
#if defined(_WIN32) || defined(__APPLE__)
      std::string regex = cmSystemTools::LowerCase(args[i]);
#else
      std::string regex = args[i];
#endif
      cmSystemTools::ReplaceString(regex, "\\", "\\\\");
      literal_args += regex;
      literal_args += "\"";
      doing = DoingNone;
    } else if (doing == DoingComponent) {
      component = args[i];
      doing = DoingNone;
    } else if (doing == DoingPermsFile) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       permissions_file)) {
        status.SetError(cmStrCat(args[0], " given invalid file permission \"",
                                 args[i], "\"."));
        return false;
      }
    } else if (doing == DoingPermsDir) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       permissions_dir)) {
        status.SetError(cmStrCat(
          args[0], " given invalid directory permission \"", args[i], "\"."));
        return false;
      }
    } else if (doing == DoingPermsMatch) {
      // Check the requested permission.
      if (!cmInstallCommandArguments::CheckPermissions(args[i],
                                                       literal_args)) {
        status.SetError(
          cmStrCat(args[0], " given invalid permission \"", args[i], "\"."));
        return false;
      }
    } else {
      // Unknown argument.
      status.SetError(
        cmStrCat(args[0], " given unknown argument \"", args[i], "\"."));
      return false;
    }
  }

  // Support installing an empty directory.
  if (dirs.empty() && destination.has_value()) {
    dirs.emplace_back();
  }

  // Check if there is something to do.
  if (dirs.empty()) {
    return true;
  }
  if (!destination.has_value()) {
    if (type.empty()) {
      // A destination is required.
      status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
      return false;
    }
    destination = helper.GetDestinationForType(nullptr, type);
  } else if (!type.empty()) {
    status.SetError(cmStrCat(args[0],
                             " given both TYPE and DESTINATION "
                             "arguments. You may only specify one."));
    return false;
  }

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile, message_never);

  // Create the directory install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallDirectoryGenerator>(
      dirs, *destination, permissions_file, permissions_dir, configurations,
      component, message, exclude_from_all, literal_args, optional,
      helper.Makefile->GetBacktrace()));

  // Tell the global generator about any installation component names
  // specified.
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(component);

  return true;
}

bool HandleExportAndroidMKMode(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
#ifndef CMAKE_BOOTSTRAP
  Helper helper(status);

  // This is the EXPORT mode.
  cmInstallCommandArguments ica(helper.DefaultComponentName, *helper.Makefile);

  std::string exp;
  std::string name_space;
  bool exportOld = false;
  std::string filename;

  ica.Bind("EXPORT_ANDROID_MK"_s, exp);
  ica.Bind("NAMESPACE"_s, name_space);
  ica.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, exportOld);
  ica.Bind("FILE"_s, filename);

  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  // Make sure there is a destination.
  if (ica.GetDestination().empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Check the file name.
  std::string fname = filename;
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(cmStrCat(args[0], " given invalid export file name \"",
                             fname,
                             "\".  The FILE argument may not contain a path.  "
                             "Specify the path in the DESTINATION argument."));
    return false;
  }

  // Check the file extension.
  if (!fname.empty() &&
      cmSystemTools::GetFilenameLastExtension(fname) != ".mk") {
    status.SetError(cmStrCat(
      args[0], " given invalid export file name \"", fname,
      R"(".  The FILE argument must specify a name ending in ".mk".)"));
    return false;
  }
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(
      cmStrCat(args[0], " given export name \"", exp,
               "\".  "
               "This name cannot be safely converted to a file name.  "
               "Specify a different export name or use the FILE option to set "
               "a file name explicitly."));
    return false;
  }
  // Use the default name
  if (fname.empty()) {
    fname = "Android.mk";
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exp];

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Tell the global generator about any installation component names
  // specified
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallAndroidMKExportGenerator>(
      &exportSet, ica.GetDestination(), ica.GetPermissions(),
      ica.GetConfigurations(), ica.GetComponent(), message,
      ica.GetExcludeFromAll(), std::move(fname), std::move(name_space),
      helper.Makefile->GetBacktrace()));

  return true;
#else
  static_cast<void>(args);
  status.SetError("EXPORT_ANDROID_MK not supported in bootstrap cmake");
  return false;
#endif
}

#ifndef CMAKE_BOOTSTRAP
cm::optional<cm::string_view> MatchExport(cm::string_view directive,
                                          std::string const& exportName)
{
  std::string::size_type const l = exportName.size();
  if (directive.substr(0, l) == exportName) {
    if (directive.size() > l && directive[l] == ':') {
      return directive.substr(l + 1);
    }
  }
  return cm::nullopt;
}

void AssignValue(std::string& dest, std::string const& value)
{
  dest = value;
}

void AssignValue(std::vector<std::string>& dest, std::string const& value)
{
  dest = cmList{ value }.data();
}

template <typename T>
void GetExportArgumentFromVariable(cmMakefile const* makefile,
                                   cmExportSet const& exportSet,
                                   cm::string_view suffix, T& variable)
{
  std::string const& name =
    cmStrCat(exportSet.GetName(), "_EXPORT_PACKAGE_INFO_"_s, suffix);
  if (cmValue const& value = makefile->GetDefinition(name)) {
    std::string realValue;
    makefile->ConfigureString(value, realValue, true, false);
    AssignValue(variable, realValue);
  }
}

bool HandleMappedPackageInfo(
  cmExportSet& exportSet, cm::string_view directive, Helper& helper,
  cmInstallCommandArguments const& installCommandArgs,
  cmExecutionStatus& status, cmInstallGenerator::MessageLevel message,
  std::string const& cxxModulesDirectory)
{
  cmPackageInfoArguments arguments;

  // Extract information from the directive.
  std::string::size_type const n = directive.find('/');
  if (n != std::string::npos) {
    arguments.PackageName = std::string{ directive.substr(0, n) };
    directive = directive.substr(n + 1);

    if (!directive.empty() && directive[0] == 'l') {
      arguments.LowerCase = true;
      directive = directive.substr(1);
    }

    if (!directive.empty() && directive[0] == 'a') {
      std::string::size_type const d = directive.find('/');
      if (d != std::string::npos) {
        arguments.Appendix = std::string{ directive.substr(1, d - 1) };
        directive = directive.substr(d);
      } else {
        arguments.Appendix = std::string{ directive.substr(1) };
        directive = {};
      }

      if (arguments.Appendix.empty()) {
        status.SetError(cmStrCat(
          "CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO given APPENDIX "
          R"(directive for export ")"_s,
          exportSet.GetName(), R"(", but no appendix name was provided.)"_s));
        return false;
      }
    }

    if (!directive.empty()) {
      if (directive[0] != '/') {
        status.SetError(
          cmStrCat("CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO given unrecognized "
                   R"(directive  ")"_s,
                   directive, R"(".)"_s));
        return false;
      }

      directive = directive.substr(1);
    }
  } else {
    arguments.PackageName = std::string{ directive };
    directive = {};
  }

  if (arguments.PackageName.empty()) {
    status.SetError(
      cmStrCat("CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO missing package name "
               R"(for export ")"_s,
               exportSet.GetName(), R"(".)"_s));
    return false;
  }

  // Build destination.
  std::string dest = std::string{ directive };
  if (dest.empty()) {
    if (helper.Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME") == "Windows") {
      dest = std::string{ "cps"_s };
    } else {
      dest = cmStrCat(helper.GetLibraryDestination(nullptr), "/cps/",
                      arguments.GetPackageDirName());
    }
  }

  if (arguments.Appendix.empty()) {
    // Get additional export information from variables.
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "VERSION"_s, arguments.Version);
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "COMPAT_VERSION"_s, arguments.VersionCompat);
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "VERSION_SCHEMA"_s, arguments.VersionSchema);
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "LICENSE"_s, arguments.License);
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "DEFAULT_LICENSE"_s,
      arguments.DefaultLicense);
    GetExportArgumentFromVariable( // BR
      helper.Makefile, exportSet, "DEFAULT_CONFIGURATIONS"_s,
      arguments.DefaultConfigs);
  }

  // Sanity-check export information.
  if (!arguments.Check(status)) {
    return false;
  }

  // Create the package info generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallPackageInfoExportGenerator>(
      &exportSet, dest, installCommandArgs.GetPermissions(),
      installCommandArgs.GetConfigurations(),
      installCommandArgs.GetComponent(), message,
      installCommandArgs.GetExcludeFromAll(), std::move(arguments),
      cxxModulesDirectory, helper.Makefile->GetBacktrace()));

  return true;
}
#endif

bool HandleExportMode(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  Helper helper(status);

  // This is the EXPORT mode.
  cmInstallCommandArguments ica(helper.DefaultComponentName, *helper.Makefile);

  std::string exp;
  std::string name_space;
  bool exportOld = false;
  std::string filename;
  std::string cxx_modules_directory;
  bool exportPackageDependencies = false;

  ica.Bind("EXPORT"_s, exp);
  ica.Bind("NAMESPACE"_s, name_space);
  ica.Bind("EXPORT_LINK_INTERFACE_LIBRARIES"_s, exportOld);
  ica.Bind("FILE"_s, filename);
  ica.Bind("CXX_MODULES_DIRECTORY"_s, cxx_modules_directory);

  if (cmExperimental::HasSupportEnabled(
        status.GetMakefile(),
        cmExperimental::Feature::ExportPackageDependencies)) {
    ica.Bind("EXPORT_PACKAGE_DEPENDENCIES"_s, exportPackageDependencies);
  }

  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  if (!unknownArgs.empty()) {
    // Unknown argument.
    status.SetError(
      cmStrCat(args[0], " given unknown argument \"", unknownArgs[0], "\"."));
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  if (exp.empty()) {
    status.SetError(cmStrCat(args[0], " missing EXPORT."));
    return false;
  }

  // Make sure there is a destination.
  if (ica.GetDestination().empty()) {
    // A destination is required.
    status.SetError(cmStrCat(args[0], " given no DESTINATION!"));
    return false;
  }

  // Check the file name.
  std::string fname = filename;
  if (fname.find_first_of(":/\\") != std::string::npos) {
    status.SetError(cmStrCat(args[0], " given invalid export file name \"",
                             fname,
                             "\".  "
                             "The FILE argument may not contain a path.  "
                             "Specify the path in the DESTINATION argument."));
    return false;
  }

  // Check the file extension.
  if (!fname.empty() &&
      cmSystemTools::GetFilenameLastExtension(fname) != ".cmake") {
    status.SetError(
      cmStrCat(args[0], " given invalid export file name \"", fname,
               "\".  "
               "The FILE argument must specify a name ending in \".cmake\"."));
    return false;
  }

  // Construct the file name.
  if (fname.empty()) {
    fname = cmStrCat(exp, ".cmake");

    if (fname.find_first_of(":/\\") != std::string::npos) {
      status.SetError(cmStrCat(
        args[0], " given export name \"", exp,
        "\".  "
        "This name cannot be safely converted to a file name.  "
        "Specify a different export name or use the FILE option to set "
        "a file name explicitly."));
      return false;
    }
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exp];

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Tell the global generator about any installation component names
  // specified
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());

#ifndef CMAKE_BOOTSTRAP
  // Check if PACKAGE_INFO export has been requested for this export set.
  if (cmExperimental::HasSupportEnabled(
        status.GetMakefile(), cmExperimental::Feature::ExportPackageInfo) &&
      cmExperimental::HasSupportEnabled(
        status.GetMakefile(), cmExperimental::Feature::MappedPackageInfo)) {
    if (cmValue const& piExports = helper.Makefile->GetDefinition(
          "CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO")) {
      for (auto const& pie : cmList{ piExports }) {
        cm::optional<cm::string_view> const directive = MatchExport(pie, exp);
        if (directive) {
          if (!HandleMappedPackageInfo(exportSet, *directive, helper, ica,
                                       status, message,
                                       cxx_modules_directory)) {
            return false;
          }
        }
      }
    }
  }
#endif

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallCMakeConfigExportGenerator>(
      &exportSet, ica.GetDestination(), ica.GetPermissions(),
      ica.GetConfigurations(), ica.GetComponent(), message,
      ica.GetExcludeFromAll(), std::move(fname), std::move(name_space),
      std::move(cxx_modules_directory), exportOld, exportPackageDependencies,
      helper.Makefile->GetBacktrace()));

  return true;
}

bool HandlePackageInfoMode(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
#ifndef CMAKE_BOOTSTRAP
  if (!cmExperimental::HasSupportEnabled(
        status.GetMakefile(), cmExperimental::Feature::ExportPackageInfo)) {
    status.SetError("does not recognize sub-command PACKAGE_INFO");
    return false;
  }

  Helper helper(status);

  // This is the PACKAGE_INFO mode.
  cmInstallCommandArguments ica(helper.DefaultComponentName, *helper.Makefile);

  cmPackageInfoArguments arguments;
  ArgumentParser::NonEmpty<std::string> exportName;
  ArgumentParser::NonEmpty<std::string> cxxModulesDirectory;

  arguments.Bind(ica);
  ica.Bind("EXPORT"_s, exportName);
  ica.Bind("CXX_MODULES_DIRECTORY"_s, cxxModulesDirectory);

  std::vector<std::string> unknownArgs;
  ArgumentParser::ParseResult result = ica.Parse(args, &unknownArgs);

  if (!result.Check(args[0], &unknownArgs, status)) {
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  if (exportName.empty()) {
    status.SetError(cmStrCat(args[0], " missing EXPORT."));
    return false;
  }

  if (!arguments.Check(status) || !arguments.SetMetadataFromProject(status)) {
    return false;
  }

  // Get or construct the destination path.
  std::string dest = ica.GetDestination();
  if (dest.empty()) {
    if (helper.Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME") == "Windows") {
      dest = std::string{ "cps"_s };
    } else {
      dest = cmStrCat(helper.GetLibraryDestination(nullptr), "/cps/",
                      arguments.GetPackageDirName());
    }
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exportName];

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Tell the global generator about any installation component names
  // specified
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallPackageInfoExportGenerator>(
      &exportSet, dest, ica.GetPermissions(), ica.GetConfigurations(),
      ica.GetComponent(), message, ica.GetExcludeFromAll(),
      std::move(arguments), std::move(cxxModulesDirectory),
      helper.Makefile->GetBacktrace()));

  return true;
#else
  static_cast<void>(args);
  status.SetError("PACKAGE_INFO not supported in bootstrap cmake");
  return false;
#endif
}

bool HandleRuntimeDependencySetMode(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  Helper helper(status);

  auto system = helper.Makefile->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
  if (!cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
        system)) {
    status.SetError(cmStrCat(
      "RUNTIME_DEPENDENCY_SET is not supported on system \"", system, '"'));
    return false;
  }

  // This is the RUNTIME_DEPENDENCY_SET mode.
  cmInstallRuntimeDependencySet* runtimeDependencySet;

  struct ArgVectors
  {
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Library;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Runtime;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Framework;
  };

  static auto const argHelper = cmArgumentParser<ArgVectors>{}
                                  .Bind("LIBRARY"_s, &ArgVectors::Library)
                                  .Bind("RUNTIME"_s, &ArgVectors::Runtime)
                                  .Bind("FRAMEWORK"_s, &ArgVectors::Framework);

  std::vector<std::string> genericArgVector;
  ArgVectors const argVectors = argHelper.Parse(args, &genericArgVector);

  // now parse the generic args (i.e. the ones not specialized on LIBRARY,
  // RUNTIME, FRAMEWORK etc. (see above)
  // These generic args also contain the runtime dependency set
  std::string runtimeDependencySetArg;
  std::vector<std::string> runtimeDependencyArgVector;
  cmInstallCommandArguments genericArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  genericArgs.Bind("RUNTIME_DEPENDENCY_SET"_s, runtimeDependencySetArg);
  genericArgs.Parse(genericArgVector, &runtimeDependencyArgVector);
  bool success = genericArgs.Finalize();

  cmInstallCommandArguments libraryArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments runtimeArgs(helper.DefaultComponentName,
                                        *helper.Makefile);
  cmInstallCommandArguments frameworkArgs(helper.DefaultComponentName,
                                          *helper.Makefile);

  // Now also parse the file(GET_RUNTIME_DEPENDENCY) args
  std::vector<std::string> unknownArgs;
  auto runtimeDependencyArgs = RuntimeDependenciesArgHelper.Parse(
    runtimeDependencyArgVector, &unknownArgs);

  // now parse the args for specific parts of the target (e.g. LIBRARY,
  // RUNTIME, FRAMEWORK etc.
  libraryArgs.Parse(argVectors.Library, &unknownArgs);
  runtimeArgs.Parse(argVectors.Runtime, &unknownArgs);
  frameworkArgs.Parse(argVectors.Framework, &unknownArgs);

  libraryArgs.SetGenericArguments(&genericArgs);
  runtimeArgs.SetGenericArguments(&genericArgs);
  frameworkArgs.SetGenericArguments(&genericArgs);

  success = success && libraryArgs.Finalize();
  success = success && runtimeArgs.Finalize();
  success = success && frameworkArgs.Finalize();

  if (!success) {
    return false;
  }

  if (!unknownArgs.empty()) {
    helper.SetError(
      cmStrCat("RUNTIME_DEPENDENCY_SET given unknown argument \"",
               unknownArgs.front(), "\"."));
    return false;
  }

  if (runtimeDependencySetArg.empty()) {
    helper.SetError(
      "RUNTIME_DEPENDENCY_SET not given a runtime dependency set.");
    return false;
  }

  runtimeDependencySet =
    helper.Makefile->GetGlobalGenerator()->GetNamedRuntimeDependencySet(
      runtimeDependencySetArg);

  bool installsRuntime = false;
  bool installsLibrary = false;
  bool installsFramework = false;

  AddInstallRuntimeDependenciesGenerator(
    helper, runtimeDependencySet, runtimeArgs, libraryArgs, frameworkArgs,
    std::move(runtimeDependencyArgs), installsRuntime, installsLibrary,
    installsFramework);

  // Tell the global generator about any installation component names
  // specified
  if (installsLibrary) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      libraryArgs.GetComponent());
  }
  if (installsRuntime) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      runtimeArgs.GetComponent());
  }
  if (installsFramework) {
    helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
      frameworkArgs.GetComponent());
  }

  return true;
}

bool HandleSbomMode(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
#ifndef CMAKE_BOOTSTRAP
  if (!cmExperimental::HasSupportEnabled(
        status.GetMakefile(), cmExperimental::Feature::GenerateSbom)) {
    status.SetError("does not recognize sub-command SBOM");
    return false;
  }

  Helper helper(status);
  cmInstallCommandArguments ica(helper.DefaultComponentName, *helper.Makefile);

  cmSbomArguments arguments;
  ArgumentParser::NonEmpty<std::string> exportName;
  ArgumentParser::NonEmpty<std::string> cxxModulesDirectory;

  arguments.Bind(ica);
  ica.Bind("EXPORT"_s, exportName);
  // ica.Bind("CXX_MODULES_DIRECTORY"_s, cxxModulesDirectory); TODO?

  std::vector<std::string> unknownArgs;
  ica.Parse(args, &unknownArgs);

  ArgumentParser::ParseResult result = ica.Parse(args, &unknownArgs);
  if (!result.Check(args[0], &unknownArgs, status)) {
    return false;
  }

  if (!ica.Finalize()) {
    return false;
  }

  if (arguments.PackageName.empty()) {
    // TODO: Fix our use of the parser to enforce this.
    status.SetError(cmStrCat(args[0], " missing SBOM name."));
    return false;
  }

  if (exportName.empty()) {
    status.SetError(cmStrCat(args[0], " missing EXPORT."));
    return false;
  }

  if (!arguments.Check(status) || !arguments.SetMetadataFromProject(status)) {
    return false;
  }

  // Get or construct the destination path.
  std::string dest = ica.GetDestination();
  if (dest.empty()) {
    if (helper.Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME") == "Windows") {
      dest = std::string{ "/sbom/"_s };
    } else {
      dest = cmStrCat(helper.GetLibraryDestination(nullptr), "/sbom/",
                      arguments.GetPackageDirName());
    }
  }

  cmExportSet& exportSet =
    helper.Makefile->GetGlobalGenerator()->GetExportSets()[exportName];

  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(helper.Makefile);

  // Tell the global generator about any installation component names
  // specified
  helper.Makefile->GetGlobalGenerator()->AddInstallComponent(
    ica.GetComponent());
  helper.Makefile->SetExplicitlyGeneratesSbom(true);

  // Create the export install generator.
  helper.Makefile->AddInstallGenerator(
    cm::make_unique<cmInstallSbomExportGenerator>(
      &exportSet, dest, ica.GetPermissions(), ica.GetConfigurations(),
      ica.GetComponent(), message, ica.GetExcludeFromAll(),
      std::move(arguments), std::move(cxxModulesDirectory),
      helper.Makefile->GetBacktrace()));

  return true;
#else
  static_cast<void>(args);
  status.SetError("SBOM not supported in bootstrap cmake");
  return false;
#endif
}

bool Helper::MakeFilesFullPath(char const* modeName,
                               std::vector<std::string> const& relFiles,
                               std::vector<std::string>& absFiles)
{
  return this->MakeFilesFullPath(
    modeName, this->Makefile->GetCurrentSourceDirectory(), relFiles, absFiles);
}

bool Helper::MakeFilesFullPath(char const* modeName,
                               std::string const& basePath,
                               std::vector<std::string> const& relFiles,
                               std::vector<std::string>& absFiles)
{
  for (std::string const& relFile : relFiles) {
    std::string file = relFile;
    std::string::size_type gpos = cmGeneratorExpression::Find(file);
    if (gpos != 0 && !cmSystemTools::FileIsFullPath(file)) {
      file = cmStrCat(basePath, '/', relFile);
    }

    // Make sure the file is not a directory.
    if (gpos == std::string::npos && !cmSystemTools::FileIsSymlink(file) &&
        cmSystemTools::FileIsDirectory(file)) {
      this->SetError(
        cmStrCat(modeName, " given directory \"", relFile, "\" to install."));
      return false;
    }
    // Store the file for installation.
    absFiles.push_back(std::move(file));
  }
  return true;
}

std::string Helper::GetDestination(cmInstallCommandArguments const* args,
                                   std::string const& varName,
                                   std::string const& guess) const
{
  if (args && !args->GetDestination().empty()) {
    return args->GetDestination();
  }
  std::string val = this->Makefile->GetSafeDefinition(varName);
  if (!val.empty()) {
    return val;
  }
  return guess;
}

std::string Helper::GetRuntimeDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_BINDIR", "bin");
}

std::string Helper::GetSbinDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_SBINDIR", "sbin");
}

std::string Helper::GetArchiveDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string Helper::GetLibraryDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_LIBDIR", "lib");
}

std::string Helper::GetCxxModulesBmiDestination(
  cmInstallCommandArguments const* args) const
{
  if (args) {
    return args->GetDestination();
  }
  return {};
}

std::string Helper::GetIncludeDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_INCLUDEDIR", "include");
}

std::string Helper::GetSysconfDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_SYSCONFDIR", "etc");
}

std::string Helper::GetSharedStateDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_SHAREDSTATEDIR", "com");
}

std::string Helper::GetLocalStateDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_LOCALSTATEDIR", "var");
}

std::string Helper::GetRunStateDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_RUNSTATEDIR",
                              this->GetLocalStateDestination(nullptr) +
                                "/run");
}

std::string Helper::GetDataRootDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_DATAROOTDIR", "share");
}

std::string Helper::GetDataDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_DATADIR",
                              this->GetDataRootDestination(nullptr));
}

std::string Helper::GetInfoDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_INFODIR",
                              this->GetDataRootDestination(nullptr) + "/info");
}

std::string Helper::GetLocaleDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_LOCALEDIR",
                              this->GetDataRootDestination(nullptr) +
                                "/locale");
}

std::string Helper::GetManDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_MANDIR",
                              this->GetDataRootDestination(nullptr) + "/man");
}

std::string Helper::GetDocDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_DOCDIR",
                              this->GetDataRootDestination(nullptr) + "/doc");
}

std::string Helper::GetProgramExecutablesDestination(
  cmInstallCommandArguments const* args) const
{
  return this->GetDestination(args, "CMAKE_INSTALL_LIBEXECDIR", "libexec");
}

std::string Helper::GetDestinationForType(
  cmInstallCommandArguments const* args, std::string const& type) const
{
  if (args && !args->GetDestination().empty()) {
    return args->GetDestination();
  }
  if (type == "BIN") {
    return this->GetRuntimeDestination(nullptr);
  }
  if (type == "SBIN") {
    return this->GetSbinDestination(nullptr);
  }
  if (type == "SYSCONF") {
    return this->GetSysconfDestination(nullptr);
  }
  if (type == "SHAREDSTATE") {
    return this->GetSharedStateDestination(nullptr);
  }
  if (type == "LOCALSTATE") {
    return this->GetLocalStateDestination(nullptr);
  }
  if (type == "RUNSTATE") {
    return this->GetRunStateDestination(nullptr);
  }
  if (type == "LIB") {
    return this->GetLibraryDestination(nullptr);
  }
  if (type == "INCLUDE") {
    return this->GetIncludeDestination(nullptr);
  }
  if (type == "DATA") {
    return this->GetDataDestination(nullptr);
  }
  if (type == "INFO") {
    return this->GetInfoDestination(nullptr);
  }
  if (type == "LOCALE") {
    return this->GetLocaleDestination(nullptr);
  }
  if (type == "MAN") {
    return this->GetManDestination(nullptr);
  }
  if (type == "DOC") {
    return this->GetDocDestination(nullptr);
  }
  if (type == "LIBEXEC") {
    return this->GetProgramExecutablesDestination(nullptr);
  }
  return "";
}

} // namespace

bool cmInstallCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  // Allow calling with no arguments so that arguments may be built up
  // using a variable that may be left empty.
  if (args.empty()) {
    return true;
  }

  // Enable the install target.
  status.GetMakefile().GetGlobalGenerator()->EnableInstallTarget();

  static cmSubcommandTable const subcommand{
    { "SCRIPT"_s, HandleScriptMode },
    { "CODE"_s, HandleScriptMode },
    { "TARGETS"_s, HandleTargetsMode },
    { "IMPORTED_RUNTIME_ARTIFACTS"_s, HandleImportedRuntimeArtifactsMode },
    { "FILES"_s, HandleFilesMode },
    { "PROGRAMS"_s, HandleFilesMode },
    { "DIRECTORY"_s, HandleDirectoryMode },
    { "EXPORT"_s, HandleExportMode },
    { "EXPORT_ANDROID_MK"_s, HandleExportAndroidMKMode },
    { "PACKAGE_INFO"_s, HandlePackageInfoMode },
    { "RUNTIME_DEPENDENCY_SET"_s, HandleRuntimeDependencySetMode },
    { "SBOM"_s, HandleSbomMode }
  };

  return subcommand(args[0], args, status);
}
