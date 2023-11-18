/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommonTargetGenerator.h"

#include <algorithm>
#include <sstream>
#include <type_traits>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalCommonGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
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
#include "cmSystemTools.h"
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

cmCommonTargetGenerator::LinkedTargetDirs
cmCommonTargetGenerator::GetLinkedTargetDirectories(
  const std::string& lang, const std::string& config) const
{
  LinkedTargetDirs dirs;
  std::set<cmGeneratorTarget const*> forward_emitted;
  std::set<cmGeneratorTarget const*> direct_emitted;
  cmGlobalCommonGenerator* const gg = this->GlobalCommonGenerator;

  enum class Forwarding
  {
    Yes,
    No
  };

  if (cmComputeLinkInformation* cli =
        this->GeneratorTarget->GetLinkInformation(config)) {
    auto addLinkedTarget =
      [this, &lang, &config, &dirs, &direct_emitted, &forward_emitted,
       gg](cmGeneratorTarget const* linkee, Forwarding forward) {
        if (linkee &&
            !linkee->IsImported()
            // Skip targets that build after this one in a static lib cycle.
            && gg->TargetOrderIndexLess(linkee, this->GeneratorTarget)
            // We can ignore the INTERFACE_LIBRARY items because
            // Target->GetLinkInformation already processed their
            // link interface and they don't have any output themselves.
            && (linkee->GetType() != cmStateEnums::INTERFACE_LIBRARY
                // Synthesized targets may have relevant rules.
                || linkee->IsSynthetic()) &&
            ((lang == "CXX"_s && linkee->HaveCxx20ModuleSources()) ||
             (lang == "Fortran"_s && linkee->HaveFortranSources(config)))) {
          cmLocalGenerator* lg = linkee->GetLocalGenerator();
          std::string di = cmStrCat(lg->GetCurrentBinaryDirectory(), '/',
                                    lg->GetTargetDirectory(linkee));
          if (lg->GetGlobalGenerator()->IsMultiConfig()) {
            di = cmStrCat(di, '/', config);
          }
          if (forward == Forwarding::Yes &&
              forward_emitted.insert(linkee).second) {
            dirs.Forward.push_back(di);
          }
          if (direct_emitted.insert(linkee).second) {
            dirs.Direct.emplace_back(di);
          }
        }
      };
    for (auto const& item : cli->GetItems()) {
      if (item.Target) {
        addLinkedTarget(item.Target, Forwarding::No);
      } else if (item.ObjectSource && lang == "Fortran"_s
                 /* Object source files do not have a language associated with
                    them. */
                 /* && item.ObjectSource->GetLanguage() == "Fortran"_s*/) {
        // Fortran modules provided by `$<TARGET_OBJECTS>` as linked items
        // should be collated for use in this target.
        addLinkedTarget(this->LocalCommonGenerator->FindGeneratorTargetToUse(
                          item.ObjectSource->GetObjectLibrary()),
                        Forwarding::Yes);
      }
    }
    for (cmGeneratorTarget const* target : cli->GetExternalObjectTargets()) {
      addLinkedTarget(target, Forwarding::No);
    }
    if (lang == "Fortran"_s) {
      // Fortran modules provided by `$<TARGET_OBJECTS>` as sources should be
      // collated for use in this target.
      for (cmGeneratorTarget const* target :
           this->GeneratorTarget->GetSourceObjectLibraries(config)) {
        addLinkedTarget(target, Forwarding::Yes);
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
  std::string manifestFlag =
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
  if (this->GeneratorTarget->IsAIX()) {
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

std::string cmCommonTargetGenerator::GetCompilerLauncher(
  std::string const& lang, std::string const& config)
{
  std::string compilerLauncher;
  if (lang == "C" || lang == "CXX" || lang == "Fortran" || lang == "CUDA" ||
      lang == "HIP" || lang == "ISPC" || lang == "OBJC" || lang == "OBJCXX") {
    std::string const clauncher_prop = cmStrCat(lang, "_COMPILER_LAUNCHER");
    cmValue clauncher = this->GeneratorTarget->GetProperty(clauncher_prop);
    std::string const evaluatedClauncher = cmGeneratorExpression::Evaluate(
      *clauncher, this->GeneratorTarget->GetLocalGenerator(), config,
      this->GeneratorTarget, nullptr, this->GeneratorTarget, lang);
    if (!evaluatedClauncher.empty()) {
      compilerLauncher = evaluatedClauncher;
    }
  }
  return compilerLauncher;
}

std::string cmCommonTargetGenerator::GenerateCodeCheckRules(
  cmSourceFile const& source, std::string& compilerLauncher,
  std::string const& cmakeCmd, std::string const& config,
  std::function<std::string(std::string const&)> const& pathConverter)
{
  auto const lang = source.GetLanguage();
  std::string tidy;
  std::string iwyu;
  std::string cpplint;
  std::string cppcheck;

  auto evaluateProp = [&](std::string const& prop) -> std::string {
    auto const value = this->GeneratorTarget->GetProperty(prop);
    if (!value) {
      return std::string{};
    }
    auto evaluatedProp = cmGeneratorExpression::Evaluate(
      *value, this->GeneratorTarget->GetLocalGenerator(), config,
      this->GeneratorTarget, nullptr, this->GeneratorTarget, lang);
    return evaluatedProp;
  };
  std::string const tidy_prop = cmStrCat(lang, "_CLANG_TIDY");
  tidy = evaluateProp(tidy_prop);

  if (lang == "C" || lang == "CXX") {
    std::string const iwyu_prop = cmStrCat(lang, "_INCLUDE_WHAT_YOU_USE");
    iwyu = evaluateProp(iwyu_prop);

    std::string const cpplint_prop = cmStrCat(lang, "_CPPLINT");
    cpplint = evaluateProp(cpplint_prop);

    std::string const cppcheck_prop = cmStrCat(lang, "_CPPCHECK");
    cppcheck = evaluateProp(cppcheck_prop);
  }
  if (cmNonempty(iwyu) || cmNonempty(tidy) || cmNonempty(cpplint) ||
      cmNonempty(cppcheck)) {
    std::string code_check = cmakeCmd + " -E __run_co_compile";
    if (!compilerLauncher.empty()) {
      // In __run_co_compile case the launcher command is supplied
      // via --launcher=<maybe-list> and consumed
      code_check += " --launcher=";
      code_check += this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(
        compilerLauncher);
      compilerLauncher.clear();
    }
    if (cmNonempty(iwyu)) {
      code_check += " --iwyu=";

      // Only add --driver-mode if it is not already specified, as adding
      // it unconditionally might override a user-specified driver-mode
      if (iwyu.find("--driver-mode=") == std::string::npos) {
        cmValue const p = this->Makefile->GetDefinition(
          cmStrCat("CMAKE_", lang, "_INCLUDE_WHAT_YOU_USE_DRIVER_MODE"));
        std::string driverMode;

        if (cmNonempty(p)) {
          driverMode = *p;
        } else {
          driverMode = lang == "C" ? "gcc" : "g++";
        }

        code_check +=
          this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(
            cmStrCat(iwyu, ";--driver-mode=", driverMode));
      } else {
        code_check +=
          this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(iwyu);
      }
    }
    if (cmNonempty(tidy)) {
      code_check += " --tidy=";
      cmValue const p = this->Makefile->GetDefinition(
        "CMAKE_" + lang + "_CLANG_TIDY_DRIVER_MODE");
      std::string driverMode;
      if (cmNonempty(p)) {
        driverMode = *p;
      } else {
        driverMode = lang == "C" ? "gcc" : "g++";
      }

      auto const generatorName = this->GeneratorTarget->GetLocalGenerator()
                                   ->GetGlobalGenerator()
                                   ->GetName();
      auto const clangTidyExportFixedDir =
        this->GeneratorTarget->GetClangTidyExportFixesDirectory(lang);
      auto fixesFile = this->GetClangTidyReplacementsFilePath(
        clangTidyExportFixedDir, source, config);
      std::string exportFixes;
      if (!clangTidyExportFixedDir.empty()) {
        this->GlobalCommonGenerator->AddClangTidyExportFixesDir(
          clangTidyExportFixedDir);
      }
      if (generatorName.find("Make") != std::string::npos) {
        if (!clangTidyExportFixedDir.empty()) {
          this->GlobalCommonGenerator->AddClangTidyExportFixesFile(fixesFile);
          cmSystemTools::MakeDirectory(
            cmSystemTools::GetFilenamePath(fixesFile));
          fixesFile = this->GeneratorTarget->GetLocalGenerator()
                        ->MaybeRelativeToCurBinDir(fixesFile);
          exportFixes = cmStrCat(";--export-fixes=", fixesFile);
        }
        code_check +=
          this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(
            cmStrCat(tidy, ";--extra-arg-before=--driver-mode=", driverMode,
                     exportFixes));
      } else if (generatorName.find("Ninja") != std::string::npos) {
        if (!clangTidyExportFixedDir.empty()) {
          this->GlobalCommonGenerator->AddClangTidyExportFixesFile(fixesFile);
          cmSystemTools::MakeDirectory(
            cmSystemTools::GetFilenamePath(fixesFile));
          if (!pathConverter) {
            fixesFile = pathConverter(fixesFile);
          }
          exportFixes = cmStrCat(";--export-fixes=", fixesFile);
        }
        code_check +=
          this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(
            cmStrCat(tidy, ";--extra-arg-before=--driver-mode=", driverMode,
                     exportFixes));
      }
    }
    if (cmNonempty(cpplint)) {
      code_check += " --cpplint=";
      code_check +=
        this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(cpplint);
    }
    if (cmNonempty(cppcheck)) {
      code_check += " --cppcheck=";
      code_check +=
        this->GeneratorTarget->GetLocalGenerator()->EscapeForShell(cppcheck);
    }
    if (cmNonempty(tidy) || (cmNonempty(cpplint)) || (cmNonempty(cppcheck))) {
      code_check += " --source=";
      code_check +=
        this->GeneratorTarget->GetLocalGenerator()->ConvertToOutputFormat(
          source.GetFullPath(), cmOutputConverter::SHELL);
    }
    code_check += " -- ";
    return code_check;
  }
  return "";
}

std::string cmCommonTargetGenerator::GetLinkerLauncher(
  const std::string& config)
{
  std::string lang = this->GeneratorTarget->GetLinkerLanguage(config);
  std::string propName = lang + "_LINKER_LAUNCHER";
  cmValue launcherProp = this->GeneratorTarget->GetProperty(propName);
  if (cmNonempty(launcherProp)) {
    cmGeneratorExpressionDAGChecker dagChecker(this->GeneratorTarget, propName,
                                               nullptr, nullptr);
    std::string evaluatedLinklauncher = cmGeneratorExpression::Evaluate(
      *launcherProp, this->LocalCommonGenerator, config, this->GeneratorTarget,
      &dagChecker, this->GeneratorTarget, lang);
    // Convert ;-delimited list to single string
    cmList args{ evaluatedLinklauncher, cmList::EmptyElements::Yes };
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
