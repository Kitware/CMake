/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio8Generator.h"

#include <algorithm>
#include <functional>
#include <ostream>
#include <utility>

#include <cm/memory>
#include <cmext/algorithm>
#include <cmext/memory>

#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmCustomCommandTypes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudio7Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
#include "cmVisualStudioGeneratorOptions.h"
#include "cmake.h"

struct cmIDEFlagTable;

cmGlobalVisualStudio8Generator::cmGlobalVisualStudio8Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio71Generator(cm, platformInGeneratorName)
{
  this->ProjectConfigurationSectionName = "ProjectConfigurationPlatforms";
  this->Name = name;
  this->ExtraFlagTable =
    cmGlobalVisualStudio8Generator::GetExtraFlagTableVS8();
}

std::string cmGlobalVisualStudio8Generator::FindDevEnvCommand()
{
  // First look for VCExpress.
  std::string vsxcmd;
  std::string vsxkey =
    cmStrCat(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VCExpress\)",
             this->GetIDEVersion(), ";InstallDir");
  if (cmSystemTools::ReadRegistryValue(vsxkey, vsxcmd,
                                       cmSystemTools::KeyWOW64_32)) {
    cmSystemTools::ConvertToUnixSlashes(vsxcmd);
    vsxcmd += "/VCExpress.exe";
    return vsxcmd;
  }
  // Now look for devenv.
  return this->cmGlobalVisualStudio71Generator::FindDevEnvCommand();
}

void cmGlobalVisualStudio8Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  for (std::string const& it : lang) {
    if (it == "ASM_MASM") {
      this->MasmEnabled = true;
    }
  }
  this->AddPlatformDefinitions(mf);
  cmGlobalVisualStudio7Generator::EnableLanguage(lang, mf, optional);
}

void cmGlobalVisualStudio8Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  if (this->TargetsWindowsCE()) {
    mf->AddDefinition("CMAKE_VS_WINCE_VERSION", this->WindowsCEVersion);
  }
}

bool cmGlobalVisualStudio8Generator::SetGeneratorPlatform(std::string const& p,
                                                          cmMakefile* mf)
{
  if (this->PlatformInGeneratorName) {
    // This is an old-style generator name that contains the platform name.
    // No explicit platform specification is supported, so pass it through
    // to our base class implementation, which errors on non-empty platforms.
    return this->cmGlobalVisualStudio7Generator::SetGeneratorPlatform(p, mf);
  }

  if (!this->ParseGeneratorPlatform(p, mf)) {
    return false;
  }

  // FIXME: Add CMAKE_GENERATOR_PLATFORM field to set the framework.
  // For now, just report the generator's default, if any.
  if (cm::optional<std::string> const& targetFrameworkVersion =
        this->GetTargetFrameworkVersion()) {
    mf->AddDefinition("CMAKE_VS_TARGET_FRAMEWORK_VERSION",
                      *targetFrameworkVersion);
  }
  if (cm::optional<std::string> const& targetFrameworkIdentifier =
        this->GetTargetFrameworkIdentifier()) {
    mf->AddDefinition("CMAKE_VS_TARGET_FRAMEWORK_IDENTIFIER",
                      *targetFrameworkIdentifier);
  }
  if (cm::optional<std::string> const& targetFrameworkTargetsVersion =
        this->GetTargetFrameworkTargetsVersion()) {
    mf->AddDefinition("CMAKE_VS_TARGET_FRAMEWORK_TARGETS_VERSION",
                      *targetFrameworkTargetsVersion);
  }

  // The generator name does not contain the platform name, and so supports
  // explicit platform specification.  We handled that above, so pass an
  // empty platform name to our base class implementation so it does not error.
  return this->cmGlobalVisualStudio7Generator::SetGeneratorPlatform("", mf);
}

bool cmGlobalVisualStudio8Generator::ParseGeneratorPlatform(
  std::string const& p, cmMakefile* mf)
{
  this->GeneratorPlatform.clear();

  std::vector<std::string> const fields = cmTokenize(p, ",");
  auto fi = fields.begin();
  if (fi == fields.end()) {
    return true;
  }

  // The first field may be the VS platform.
  if (fi->find('=') == fi->npos) {
    this->GeneratorPlatform = *fi;
    ++fi;
  }

  std::set<std::string> handled;

  // The rest of the fields must be key=value pairs.
  for (; fi != fields.end(); ++fi) {
    std::string::size_type pos = fi->find('=');
    if (pos == fi->npos) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given platform specification\n"
        "  " << p << "\n"
        "that contains a field after the first ',' with no '='."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    std::string const key = fi->substr(0, pos);
    std::string const value = fi->substr(pos + 1);
    if (!handled.insert(key).second) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given platform specification\n"
        "  " << p << "\n"
        "that contains duplicate field key '" << key << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    if (!this->ProcessGeneratorPlatformField(key, value)) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given platform specification\n"
        "  " << p << "\n"
        "that contains invalid field '" << *fi << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  return true;
}

bool cmGlobalVisualStudio8Generator::ProcessGeneratorPlatformField(
  std::string const& key, std::string const& value)
{
  static_cast<void>(key);
  static_cast<void>(value);
  return false;
}

cm::optional<std::string> const&
cmGlobalVisualStudio8Generator::GetTargetFrameworkVersion() const
{
  return this->DefaultTargetFrameworkVersion;
}

cm::optional<std::string> const&
cmGlobalVisualStudio8Generator::GetTargetFrameworkIdentifier() const
{
  return this->DefaultTargetFrameworkIdentifier;
}

cm::optional<std::string> const&
cmGlobalVisualStudio8Generator::GetTargetFrameworkTargetsVersion() const
{
  return this->DefaultTargetFrameworkTargetsVersion;
}

std::string cmGlobalVisualStudio8Generator::GetGenerateStampList()
{
  return "generate.stamp.list";
}

void cmGlobalVisualStudio8Generator::Configure()
{
  this->cmGlobalVisualStudio7Generator::Configure();
}

bool cmGlobalVisualStudio8Generator::UseFolderProperty() const
{
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  return IsExpressEdition() ? false : cmGlobalGenerator::UseFolderProperty();
}

bool cmGlobalVisualStudio8Generator::AddCheckTarget()
{
  // Add a special target on which all other targets depend that
  // checks the build system and optionally re-runs CMake.
  // Skip the target if no regeneration is to be done.
  if (this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    return false;
  }

  std::vector<std::unique_ptr<cmLocalGenerator>> const& generators =
    this->LocalGenerators;
  auto& lg =
    cm::static_reference_cast<cmLocalVisualStudio7Generator>(generators[0]);

  auto cc = cm::make_unique<cmCustomCommand>();
  cmTarget* tgt = lg.AddUtilityCommand(CMAKE_CHECK_BUILD_SYSTEM_TARGET, false,
                                       std::move(cc));

  // Collect the input files used to generate all targets in this
  // project.
  std::vector<std::string> listFiles;
  for (const auto& gen : generators) {
    cm::append(listFiles, gen->GetMakefile()->GetListFiles());
  }
  // Sort the list of input files and remove duplicates.
  std::sort(listFiles.begin(), listFiles.end(), std::less<std::string>());
  auto new_end = std::unique(listFiles.begin(), listFiles.end());
  listFiles.erase(new_end, listFiles.end());

  auto ptr = cm::make_unique<cmGeneratorTarget>(tgt, &lg);
  auto* gt = ptr.get();
  lg.AddGeneratorTarget(std::move(ptr));

  // Organize in the "predefined targets" folder:
  //
  if (this->UseFolderProperty()) {
    tgt->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
  }

  // Create a list of all stamp files for this project.
  std::vector<std::string> stamps;
  std::string stampList = cmStrCat(
    "CMakeFiles/", cmGlobalVisualStudio8Generator::GetGenerateStampList());
  {
    std::string stampListFile =
      cmStrCat(generators[0]->GetMakefile()->GetCurrentBinaryDirectory(), '/',
               stampList);
    std::string stampFile;
    cmGeneratedFileStream fout(stampListFile);
    for (const auto& gi : generators) {
      stampFile = cmStrCat(gi->GetMakefile()->GetCurrentBinaryDirectory(),
                           "/CMakeFiles/generate.stamp");
      fout << stampFile << '\n';
      stamps.push_back(stampFile);
    }
  }

  // Add a custom rule to re-run CMake if any input files changed.
  {
    // The custom rule runs cmake so set UTF-8 pipes.
    bool stdPipesUTF8 = true;

    // Add a custom prebuild target to run the VerifyGlobs script.
    cmake* cm = this->GetCMakeInstance();
    if (cm->DoWriteGlobVerifyTarget()) {
      cmCustomCommandLines verifyCommandLines = cmMakeSingleCommandLine(
        { cmSystemTools::GetCMakeCommand(), "-P", cm->GetGlobVerifyScript() });
      std::vector<std::string> byproducts;
      byproducts.push_back(cm->GetGlobVerifyStamp());

      cc = cm::make_unique<cmCustomCommand>();
      cc->SetByproducts(byproducts);
      cc->SetCommandLines(verifyCommandLines);
      cc->SetComment("Checking File Globs");
      cc->SetStdPipesUTF8(stdPipesUTF8);
      lg.AddCustomCommandToTarget(CMAKE_CHECK_BUILD_SYSTEM_TARGET,
                                  cmCustomCommandType::PRE_BUILD,
                                  std::move(cc));

      // Ensure ZERO_CHECK always runs in Visual Studio using MSBuild,
      // otherwise the prebuild command will not be run.
      tgt->SetProperty("VS_GLOBAL_DisableFastUpToDateCheck", "true");
      listFiles.push_back(cm->GetGlobVerifyStamp());
    }

    // Create a rule to re-run CMake.
    std::string argS = cmStrCat("-S", lg.GetSourceDirectory());
    std::string argB = cmStrCat("-B", lg.GetBinaryDirectory());
    std::string const sln =
      cmStrCat(lg.GetBinaryDirectory(), '/', lg.GetProjectName(), ".sln");
    cmCustomCommandLines commandLines = cmMakeSingleCommandLine(
      { cmSystemTools::GetCMakeCommand(), argS, argB, "--check-stamp-list",
        stampList, "--vs-solution-file", sln });
    if (cm->GetIgnoreWarningAsError()) {
      commandLines[0].emplace_back("--compile-no-warning-as-error");
    }

    // Add the rule.  Note that we cannot use the CMakeLists.txt
    // file as the main dependency because it would get
    // overwritten by the CreateVCProjBuildRule.
    // (this could be avoided with per-target source files)
    cc = cm::make_unique<cmCustomCommand>();
    cc->SetOutputs(stamps);
    cc->SetDepends(listFiles);
    cc->SetCommandLines(commandLines);
    cc->SetComment("Checking Build System");
    cc->SetEscapeOldStyle(false);
    cc->SetStdPipesUTF8(stdPipesUTF8);
    if (cmSourceFile* file =
          lg.AddCustomCommandToOutput(std::move(cc), true)) {
      gt->AddSource(file->ResolveFullPath());
    } else {
      cmSystemTools::Error(cmStrCat("Error adding rule for ", stamps[0]));
    }
  }

  return true;
}

void cmGlobalVisualStudio8Generator::AddExtraIDETargets()
{
  cmGlobalVisualStudio7Generator::AddExtraIDETargets();
  if (this->AddCheckTarget()) {
    for (auto& LocalGenerator : this->LocalGenerators) {
      const auto& tgts = LocalGenerator->GetGeneratorTargets();
      // All targets depend on the build-system check target.
      for (const auto& ti : tgts) {
        if (ti->GetName() != CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
          ti->Target->AddUtility(CMAKE_CHECK_BUILD_SYSTEM_TARGET, false);
        }
      }
    }
  }
}

void cmGlobalVisualStudio8Generator::WriteSolutionConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs)
{
  fout << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
  for (std::string const& i : configs) {
    fout << "\t\t" << i << '|' << this->GetPlatformName() << " = " << i << '|'
         << this->GetPlatformName() << '\n';
  }
  fout << "\tEndGlobalSection\n";
}

void cmGlobalVisualStudio8Generator::WriteProjectConfigurations(
  std::ostream& fout, const std::string& name, cmGeneratorTarget const& target,
  std::vector<std::string> const& configs,
  const std::set<std::string>& configsPartOfDefaultBuild,
  std::string const& platformMapping)
{
  std::string guid = this->GetGUID(name);
  for (std::string const& i : configs) {
    cmList mapConfig;
    const char* dstConfig = i.c_str();
    if (target.GetProperty("EXTERNAL_MSPROJECT")) {
      if (cmValue m = target.GetProperty(
            cmStrCat("MAP_IMPORTED_CONFIG_", cmSystemTools::UpperCase(i)))) {
        mapConfig.assign(*m);
        if (!mapConfig.empty()) {
          dstConfig = mapConfig[0].c_str();
        }
      }
    }
    fout << "\t\t{" << guid << "}." << i << '|' << this->GetPlatformName()
         << ".ActiveCfg = " << dstConfig << '|'
         << (!platformMapping.empty() ? platformMapping
                                      : this->GetPlatformName())
         << '\n';
    auto ci = configsPartOfDefaultBuild.find(i);
    if (!(ci == configsPartOfDefaultBuild.end())) {
      fout << "\t\t{" << guid << "}." << i << '|' << this->GetPlatformName()
           << ".Build.0 = " << dstConfig << '|'
           << (!platformMapping.empty() ? platformMapping
                                        : this->GetPlatformName())
           << '\n';
    }
    if (this->NeedsDeploy(target, dstConfig)) {
      fout << "\t\t{" << guid << "}." << i << '|' << this->GetPlatformName()
           << ".Deploy.0 = " << dstConfig << '|'
           << (!platformMapping.empty() ? platformMapping
                                        : this->GetPlatformName())
           << '\n';
    }
  }
}

bool cmGlobalVisualStudio8Generator::NeedsDeploy(
  cmGeneratorTarget const& target, const char* config) const
{
  cmStateEnums::TargetType const type = target.GetType();
  if (type != cmStateEnums::EXECUTABLE &&
      type != cmStateEnums::SHARED_LIBRARY) {
    // deployment only valid on executables and shared libraries.
    return false;
  }

  if (cmValue prop = target.GetProperty("VS_SOLUTION_DEPLOY")) {
    // If set, it dictates behavior
    return cmIsOn(
      cmGeneratorExpression::Evaluate(*prop, target.LocalGenerator, config));
  }

  // To be deprecated, disable deployment even if target supports it.
  if (cmValue prop = target.GetProperty("VS_NO_SOLUTION_DEPLOY")) {
    if (cmIsOn(cmGeneratorExpression::Evaluate(*prop, target.LocalGenerator,
                                               config))) {
      // If true, always disable deployment
      return false;
    }
  }

  // Legacy behavior, enabled deployment based on 'hard-coded' target
  // platforms.
  return this->TargetSystemSupportsDeployment();
}

bool cmGlobalVisualStudio8Generator::TargetSystemSupportsDeployment() const
{
  return this->TargetsWindowsCE();
}

bool cmGlobalVisualStudio8Generator::ComputeTargetDepends()
{
  // Skip over the cmGlobalVisualStudioGenerator implementation!
  // We do not need the support that VS <= 7.1 needs.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  return this->cmGlobalGenerator::ComputeTargetDepends();
}

void cmGlobalVisualStudio8Generator::WriteProjectDepends(
  std::ostream& fout, const std::string&, const std::string&,
  cmGeneratorTarget const* gt)
{
  TargetDependSet const& unordered = this->GetTargetDirectDepends(gt);
  OrderedTargetDependSet depends(unordered, std::string());
  for (cmTargetDepend const& i : depends) {
    if (!this->IsInSolution(i)) {
      continue;
    }
    std::string guid = this->GetGUID(i->GetName());
    fout << "\t\t{" << guid << "} = {" << guid << "}\n";
  }
}

bool cmGlobalVisualStudio8Generator::NeedLinkLibraryDependencies(
  cmGeneratorTarget* target)
{
  // Look for utility dependencies that magically link.
  auto const& utilities = target->GetUtilities();
  return std::any_of(
    utilities.begin(), utilities.end(),
    [target](BT<std::pair<std::string, bool>> const& ui) {
      if (cmGeneratorTarget* depTarget =
            target->GetLocalGenerator()->FindGeneratorTargetToUse(
              ui.Value.first)) {
        if (depTarget->IsInBuildSystem() &&
            depTarget->GetProperty("EXTERNAL_MSPROJECT")) {
          // This utility dependency names an external .vcproj target.
          // We use LinkLibraryDependencies="true" to link to it without
          // predicting the .lib file location or name.
          return true;
        }
      }
      return false;
    });
}

static cmVS7FlagTable cmVS8ExtraFlagTable[] = {
  { "CallingConvention", "Gd", "cdecl", "0", 0 },
  { "CallingConvention", "Gr", "fastcall", "1", 0 },
  { "CallingConvention", "Gz", "stdcall", "2", 0 },

  { "Detect64BitPortabilityProblems", "Wp64",
    "Detect 64Bit Portability Problems", "true", 0 },
  { "ErrorReporting", "errorReport:prompt", "Report immediately", "1", 0 },
  { "ErrorReporting", "errorReport:queue", "Queue for next login", "2", 0 },
  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  { "UsePrecompiledHeader", "Yu", "Use Precompiled Header", "2",
    cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue },
  { "PrecompiledHeaderThrough", "Yu", "Precompiled Header Name", "",
    cmVS7FlagTable::UserValueRequired },
  { "UsePrecompiledHeader", "Y-", "Don't use precompiled header", "0", 0 },
  // There is no YX option in the VS8 IDE.

  // Exception handling mode.  If no entries match, it will be FALSE.
  { "ExceptionHandling", "GX", "enable c++ exceptions", "1", 0 },
  { "ExceptionHandling", "EHsc", "enable c++ exceptions", "1", 0 },
  { "ExceptionHandling", "EHa", "enable SEH exceptions", "2", 0 },

  { "EnablePREfast", "analyze", "", "true", 0 },
  { "EnablePREfast", "analyze-", "", "false", 0 },

  // Language options
  { "TreatWChar_tAsBuiltInType", "Zc:wchar_t", "wchar_t is a built-in type",
    "true", 0 },
  { "TreatWChar_tAsBuiltInType", "Zc:wchar_t-",
    "wchar_t is not a built-in type", "false", 0 },

  { "", "", "", "", 0 }
};
cmIDEFlagTable const* cmGlobalVisualStudio8Generator::GetExtraFlagTableVS8()
{
  return cmVS8ExtraFlagTable;
}
