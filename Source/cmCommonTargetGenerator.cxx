/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommonTargetGenerator.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalCommonGenerator.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"

cmCommonTargetGenerator::cmCommonTargetGenerator(cmGeneratorTarget* gt)
  : GeneratorTarget(gt)
  , Makefile(gt->Makefile)
  , LocalCommonGenerator(
      static_cast<cmLocalCommonGenerator*>(gt->LocalGenerator))
  , GlobalCommonGenerator(static_cast<cmGlobalCommonGenerator*>(
      gt->LocalGenerator->GetGlobalGenerator()))
  , ConfigNames(this->LocalCommonGenerator->GetConfigNames())
{
}

cmCommonTargetGenerator::~cmCommonTargetGenerator() = default;

std::vector<std::string> const& cmCommonTargetGenerator::GetConfigNames() const
{
  return this->ConfigNames;
}

cmValue cmCommonTargetGenerator::GetFeature(const std::string& feature,
                                            const std::string& config)
{
  return this->GeneratorTarget->GetFeature(feature, config);
}

void cmCommonTargetGenerator::AppendFortranFormatFlags(
  std::string& flags, cmSourceFile const& source)
{
  const std::string srcfmt = source.GetSafeProperty("Fortran_FORMAT");
  cmOutputConverter::FortranFormat format =
    cmOutputConverter::GetFortranFormat(srcfmt);
  if (format == cmOutputConverter::FortranFormatNone) {
    std::string const& tgtfmt =
      this->GeneratorTarget->GetSafeProperty("Fortran_FORMAT");
    format = cmOutputConverter::GetFortranFormat(tgtfmt);
  }
  const char* var = nullptr;
  switch (format) {
    case cmOutputConverter::FortranFormatFixed:
      var = "CMAKE_Fortran_FORMAT_FIXED_FLAG";
      break;
    case cmOutputConverter::FortranFormatFree:
      var = "CMAKE_Fortran_FORMAT_FREE_FLAG";
      break;
    default:
      break;
  }
  if (var) {
    this->LocalCommonGenerator->AppendFlags(
      flags, this->Makefile->GetSafeDefinition(var));
  }
}

void cmCommonTargetGenerator::AppendFortranPreprocessFlags(
  std::string& flags, cmSourceFile const& source,
  PreprocessFlagsRequired requires_pp)
{
  const std::string srcpp = source.GetSafeProperty("Fortran_PREPROCESS");
  cmOutputConverter::FortranPreprocess preprocess =
    cmOutputConverter::GetFortranPreprocess(srcpp);
  if (preprocess == cmOutputConverter::FortranPreprocess::Unset) {
    std::string const& tgtpp =
      this->GeneratorTarget->GetSafeProperty("Fortran_PREPROCESS");
    preprocess = cmOutputConverter::GetFortranPreprocess(tgtpp);
  }
  const char* var = nullptr;
  switch (preprocess) {
    case cmOutputConverter::FortranPreprocess::Needed:
      if (requires_pp == PreprocessFlagsRequired::YES) {
        var = "CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_ON";
      }
      break;
    case cmOutputConverter::FortranPreprocess::NotNeeded:
      var = "CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_OFF";
      break;
    default:
      break;
  }
  if (var) {
    this->LocalCommonGenerator->AppendCompileOptions(
      flags, this->Makefile->GetSafeDefinition(var));
  }
}

std::string cmCommonTargetGenerator::GetFlags(const std::string& l,
                                              const std::string& config,
                                              const std::string& arch)
{
  const std::string key = config + arch;

  auto i = this->Configs[key].FlagsByLanguage.find(l);
  if (i == this->Configs[key].FlagsByLanguage.end()) {
    std::string flags;

    this->LocalCommonGenerator->GetTargetCompileFlags(this->GeneratorTarget,
                                                      config, l, flags, arch);

    ByLanguageMap::value_type entry(l, flags);
    i = this->Configs[key].FlagsByLanguage.insert(entry).first;
  }
  return i->second;
}

std::string cmCommonTargetGenerator::GetDefines(const std::string& l,
                                                const std::string& config)
{
  auto i = this->Configs[config].DefinesByLanguage.find(l);
  if (i == this->Configs[config].DefinesByLanguage.end()) {
    std::set<std::string> defines;
    this->LocalCommonGenerator->GetTargetDefines(this->GeneratorTarget, config,
                                                 l, defines);

    std::string definesString;
    this->LocalCommonGenerator->JoinDefines(defines, definesString, l);

    ByLanguageMap::value_type entry(l, definesString);
    i = this->Configs[config].DefinesByLanguage.insert(entry).first;
  }
  return i->second;
}

std::string cmCommonTargetGenerator::GetIncludes(std::string const& l,
                                                 const std::string& config)
{
  auto i = this->Configs[config].IncludesByLanguage.find(l);
  if (i == this->Configs[config].IncludesByLanguage.end()) {
    std::string includes;
    this->AddIncludeFlags(includes, l, config);
    ByLanguageMap::value_type entry(l, includes);
    i = this->Configs[config].IncludesByLanguage.insert(entry).first;
  }
  return i->second;
}

std::vector<std::string> cmCommonTargetGenerator::GetLinkedTargetDirectories(
  const std::string& config) const
{
  std::vector<std::string> dirs;
  std::set<cmGeneratorTarget const*> emitted;
  if (cmComputeLinkInformation* cli =
        this->GeneratorTarget->GetLinkInformation(config)) {
    cmComputeLinkInformation::ItemVector const& items = cli->GetItems();
    for (auto const& item : items) {
      cmGeneratorTarget const* linkee = item.Target;
      if (linkee &&
          !linkee->IsImported()
          // We can ignore the INTERFACE_LIBRARY items because
          // Target->GetLinkInformation already processed their
          // link interface and they don't have any output themselves.
          && linkee->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
          emitted.insert(linkee).second) {
        cmLocalGenerator* lg = linkee->GetLocalGenerator();
        std::string di = cmStrCat(lg->GetCurrentBinaryDirectory(), '/',
                                  lg->GetTargetDirectory(linkee));
        dirs.push_back(std::move(di));
      }
    }
  }
  return dirs;
}

std::string cmCommonTargetGenerator::ComputeTargetCompilePDB(
  const std::string& config) const
{
  std::string compilePdbPath;
  if (this->GeneratorTarget->GetType() > cmStateEnums::OBJECT_LIBRARY) {
    return compilePdbPath;
  }

  compilePdbPath = this->GeneratorTarget->GetCompilePDBPath(config);
  if (compilePdbPath.empty()) {
    // Match VS default: `$(IntDir)vc$(PlatformToolsetVersion).pdb`.
    // A trailing slash tells the toolchain to add its default file name.
    compilePdbPath = this->GeneratorTarget->GetSupportDirectory();
    if (this->GlobalCommonGenerator->IsMultiConfig()) {
      compilePdbPath += "/";
      compilePdbPath += config;
    }
    compilePdbPath += "/";
    if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
      // Match VS default for static libs: `$(IntDir)$(ProjectName).pdb`.
      compilePdbPath += this->GeneratorTarget->GetName();
      compilePdbPath += ".pdb";
    }
  }

  return compilePdbPath;
}

std::string cmCommonTargetGenerator::GetManifests(const std::string& config)
{
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, config);

  std::vector<std::string> manifests;
  manifests.reserve(manifest_srcs.size());

  std::string lang = this->GeneratorTarget->GetLinkerLanguage(config);
  std::string const& manifestFlag =
    this->Makefile->GetDefinition("CMAKE_" + lang + "_LINKER_MANIFEST_FLAG");
  for (cmSourceFile const* manifest_src : manifest_srcs) {
    manifests.push_back(manifestFlag +
                        this->LocalCommonGenerator->ConvertToOutputFormat(
                          this->LocalCommonGenerator->MaybeRelativeToWorkDir(
                            manifest_src->GetFullPath()),
                          cmOutputConverter::SHELL));
  }

  return cmJoin(manifests, " ");
}

std::string cmCommonTargetGenerator::GetAIXExports(std::string const&)
{
  std::string aixExports;
  if (this->GeneratorTarget->Target->IsAIX()) {
    if (cmValue exportAll =
          this->GeneratorTarget->GetProperty("AIX_EXPORT_ALL_SYMBOLS")) {
      if (cmIsOff(*exportAll)) {
        aixExports = "-n";
      }
    }
  }
  return aixExports;
}

void cmCommonTargetGenerator::AppendOSXVerFlag(std::string& flags,
                                               const std::string& lang,
                                               const char* name, bool so)
{
  // Lookup the flag to specify the version.
  std::string fvar = cmStrCat("CMAKE_", lang, "_OSX_", name, "_VERSION_FLAG");
  cmValue flag = this->Makefile->GetDefinition(fvar);

  // Skip if no such flag.
  if (!flag) {
    return;
  }

  // Lookup the target version information.
  int major;
  int minor;
  int patch;
  std::string prop = cmStrCat("MACHO_", name, "_VERSION");
  std::string fallback_prop = so ? "SOVERSION" : "VERSION";
  this->GeneratorTarget->GetTargetVersionFallback(prop, fallback_prop, major,
                                                  minor, patch);
  if (major > 0 || minor > 0 || patch > 0) {
    // Append the flag since a non-zero version is specified.
    std::ostringstream vflag;
    vflag << *flag << major << "." << minor << "." << patch;
    this->LocalCommonGenerator->AppendFlags(flags, vflag.str());
  }
}

std::string cmCommonTargetGenerator::GetLinkerLauncher(
  const std::string& config)
{
  std::string lang = this->GeneratorTarget->GetLinkerLanguage(config);
  cmValue launcherProp =
    this->GeneratorTarget->GetProperty(lang + "_LINKER_LAUNCHER");
  if (cmNonempty(launcherProp)) {
    // Convert ;-delimited list to single string
    std::vector<std::string> args = cmExpandedList(*launcherProp, true);
    if (!args.empty()) {
      args[0] = this->LocalCommonGenerator->ConvertToOutputFormat(
        args[0], cmOutputConverter::SHELL);
      for (std::string& i : cmMakeRange(args.begin() + 1, args.end())) {
        i = this->LocalCommonGenerator->EscapeForShell(i);
      }
      return cmJoin(args, " ");
    }
  }
  return std::string();
}

bool cmCommonTargetGenerator::HaveRequiredLanguages(
  const std::vector<cmSourceFile const*>& sources,
  std::set<std::string>& languagesNeeded) const
{
  for (cmSourceFile const* sf : sources) {
    languagesNeeded.insert(sf->GetLanguage());
  }

  auto* makefile = this->Makefile;
  auto* state = makefile->GetState();
  auto unary = [&state, &makefile](const std::string& lang) -> bool {
    const bool valid = state->GetLanguageEnabled(lang);
    if (!valid) {
      makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("The language ", lang,
                 " was requested for compilation but was not enabled."
                 " To enable a language it needs to be specified in a"
                 " 'project' or 'enable_language' command in the root"
                 " CMakeLists.txt"));
    }
    return valid;
  };
  return std::all_of(languagesNeeded.cbegin(), languagesNeeded.cend(), unary);
}
