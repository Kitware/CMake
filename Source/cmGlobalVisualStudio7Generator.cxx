/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio7Generator.h"

#include <algorithm>
#include <cstdio>
#include <ostream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/string_view>

#include <windows.h>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmVisualStudioGeneratorOptions.h"
#include "cmake.h"

static cmVS7FlagTable cmVS7ExtraFlagTable[] = {
  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  { "UsePrecompiledHeader", "YX", "Automatically Generate", "2",
    cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue },
  { "PrecompiledHeaderThrough", "YX", "Precompiled Header Name", "",
    cmVS7FlagTable::UserValueRequired },
  { "UsePrecompiledHeader", "Yu", "Use Precompiled Header", "3",
    cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue },
  { "PrecompiledHeaderThrough", "Yu", "Precompiled Header Name", "",
    cmVS7FlagTable::UserValueRequired },
  { "UsePrecompiledHeader", "Y-", "Don't use precompiled header", "0", 0 },
  { "WholeProgramOptimization", "LTCG", "WholeProgramOptimization", "true",
    0 },

  // Exception handling mode.  If no entries match, it will be FALSE.
  { "ExceptionHandling", "GX", "enable c++ exceptions", "true", 0 },
  { "ExceptionHandling", "EHsc", "enable c++ exceptions", "true", 0 },
  // The EHa option does not have an IDE setting.  Let it go to false,
  // and have EHa passed on the command line by leaving out the table
  // entry.

  { "", "", "", "", 0 }
};

cmGlobalVisualStudio7Generator::cmGlobalVisualStudio7Generator(cmake* cm)
  : cmGlobalVisualStudioGenerator(cm)
{
  this->DevEnvCommandInitialized = false;
  this->MarmasmEnabled = false;
  this->MasmEnabled = false;
  this->NasmEnabled = false;
  this->ExtraFlagTable = cmVS7ExtraFlagTable;
}

cmGlobalVisualStudio7Generator::~cmGlobalVisualStudio7Generator() = default;

// Package GUID of Intel Visual Fortran plugin to VS IDE
#define CM_INTEL_PLUGIN_GUID "{B68A201D-CB9B-47AF-A52F-7EEC72E217E4}"

std::string const& cmGlobalVisualStudio7Generator::GetIntelProjectVersion()
{
  if (this->IntelProjectVersion.empty()) {
    // Compute the version of the Intel plugin to the VS IDE.
    // If the key does not exist then use a default guess.
    std::string intelVersion;
    std::string vskey =
      cmStrCat(this->GetRegistryBase(),
               "\\Packages\\" CM_INTEL_PLUGIN_GUID ";ProductVersion");
    cmSystemTools::ReadRegistryValue(vskey, intelVersion,
                                     cmSystemTools::KeyWOW64_32);
    unsigned int intelVersionNumber = ~0u;
    if (sscanf(intelVersion.c_str(), "%u", &intelVersionNumber) != 1 ||
        intelVersionNumber >= 11) {
      // Default to latest known project file version.
      intelVersion = "11.0";
    } else if (intelVersionNumber == 10) {
      // Version 10.x actually uses 9.10 in project files!
      intelVersion = "9.10";
    } else {
      // Version <= 9: use ProductVersion from registry.
    }
    this->IntelProjectVersion = intelVersion;
  }
  return this->IntelProjectVersion;
}

void cmGlobalVisualStudio7Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("CMAKE_GENERATOR_RC", "rc");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  mf->InitCMAKE_CONFIGURATION_TYPES("Debug;Release;MinSizeRel;RelWithDebInfo");

  // Create list of configurations requested by user's cache, if any.
  this->cmGlobalVisualStudioGenerator::EnableLanguage(lang, mf, optional);

  // if this environment variable is set, then copy it to
  // a static cache entry.  It will be used by
  // cmLocalGenerator::ConstructScript, to add an extra PATH
  // to all custom commands.   This is because the VS IDE
  // does not use the environment it is run in, and this allows
  // for running commands and using dll's that the IDE environment
  // does not know about.
  std::string extraPath;
  if (cmSystemTools::GetEnv("CMAKE_MSVCIDE_RUN_PATH", extraPath)) {
    mf->AddCacheDefinition("CMAKE_MSVCIDE_RUN_PATH", extraPath,
                           "Saved environment variable CMAKE_MSVCIDE_RUN_PATH",
                           cmStateEnums::STATIC);
  }
}

bool cmGlobalVisualStudio7Generator::FindMakeProgram(cmMakefile* mf)
{
  if (!this->cmGlobalVisualStudioGenerator::FindMakeProgram(mf)) {
    return false;
  }
  mf->AddDefinition("CMAKE_VS_DEVENV_COMMAND", this->GetDevEnvCommand());
  return true;
}

std::string const& cmGlobalVisualStudio7Generator::GetDevEnvCommand()
{
  if (!this->DevEnvCommandInitialized) {
    this->DevEnvCommandInitialized = true;
    this->DevEnvCommand = this->FindDevEnvCommand();
  }
  return this->DevEnvCommand;
}

std::string cmGlobalVisualStudio7Generator::FindDevEnvCommand()
{
  std::string vscmd;
  std::string vskey;

  // Search in standard location.
  vskey = cmStrCat(this->GetRegistryBase(), ";InstallDir");
  if (cmSystemTools::ReadRegistryValue(vskey, vscmd,
                                       cmSystemTools::KeyWOW64_32)) {
    cmSystemTools::ConvertToUnixSlashes(vscmd);
    vscmd += "/devenv.com";
    if (cmSystemTools::FileExists(vscmd, true)) {
      return vscmd;
    }
  }

  // Search where VS15Preview places it.
  vskey =
    cmStrCat(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\SxS\VS7;)",
             this->GetIDEVersion());
  if (cmSystemTools::ReadRegistryValue(vskey, vscmd,
                                       cmSystemTools::KeyWOW64_32)) {
    cmSystemTools::ConvertToUnixSlashes(vscmd);
    vscmd += "/Common7/IDE/devenv.com";
    if (cmSystemTools::FileExists(vscmd, true)) {
      return vscmd;
    }
  }

  vscmd = "devenv.com";
  return vscmd;
}

char const* cmGlobalVisualStudio7Generator::ExternalProjectType(
  std::string const& location)
{
  std::string extension = cmSystemTools::GetFilenameLastExtension(location);
  if (extension == ".vbproj"_s) {
    return "F184B08F-C81C-45F6-A57F-5ABD9991F28F";
  }
  if (extension == ".csproj"_s) {
    return "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC";
  }
  if (extension == ".fsproj"_s) {
    return "F2A71F9B-5D33-465A-A702-920D77279786";
  }
  if (extension == ".vdproj"_s) {
    return "54435603-DBB4-11D2-8724-00A0C9A8B90C";
  }
  if (extension == ".dbproj"_s) {
    return "C8D11400-126E-41CD-887F-60BD40844F9E";
  }
  if (extension == ".wixproj"_s) {
    return "930C7802-8A8C-48F9-8165-68863BCCD9DD";
  }
  if (extension == ".pyproj"_s) {
    return "888888A0-9F3D-457C-B088-3A5042F75D52";
  }
  return "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942";
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalVisualStudio7Generator::GenerateBuildCommand(
  std::string const& makeProgram, std::string const& projectName,
  std::string const& /*projectDir*/,
  std::vector<std::string> const& targetNames, std::string const& config,
  int /*jobs*/, bool /*verbose*/, cmBuildOptions /*buildOptions*/,
  std::vector<std::string> const& makeOptions)
{
  // Select the caller- or user-preferred make program, else devenv.
  std::string makeProgramSelected =
    this->SelectMakeProgram(makeProgram, this->GetDevEnvCommand());

  // Ignore the above preference if it is msbuild.
  // Assume any other value is either a devenv or
  // command-line compatible with devenv.
  std::string makeProgramLower = makeProgramSelected;
  cmSystemTools::LowerCase(makeProgramLower);
  if (makeProgramLower.find("msbuild") != std::string::npos) {
    makeProgramSelected = this->GetDevEnvCommand();
  }

  // Workaround to convince VCExpress.exe to produce output.
  bool const requiresOutputForward =
    (makeProgramLower.find("vcexpress") != std::string::npos);
  std::vector<GeneratedMakeCommand> makeCommands;

  std::vector<std::string> realTargetNames = targetNames;
  if (targetNames.empty() ||
      ((targetNames.size() == 1) && targetNames.front().empty())) {
    realTargetNames = { "ALL_BUILD" };
  }
  for (auto const& tname : realTargetNames) {
    std::string realTarget;
    if (!tname.empty()) {
      realTarget = tname;
    } else {
      continue;
    }
    bool clean = false;
    if (realTarget == "clean"_s) {
      clean = true;
      realTarget = "ALL_BUILD";
    }
    GeneratedMakeCommand makeCommand;
    makeCommand.RequiresOutputForward = requiresOutputForward;
    makeCommand.Add(makeProgramSelected);
    makeCommand.Add(cmStrCat(projectName, ".sln"));
    makeCommand.Add((clean ? "/clean" : "/build"));
    makeCommand.Add((config.empty() ? "Debug" : config));
    makeCommand.Add("/project");
    makeCommand.Add(realTarget);
    makeCommand.Add(makeOptions.begin(), makeOptions.end());
    makeCommands.emplace_back(std::move(makeCommand));
  }
  return makeCommands;
}

//! Create a local generator appropriate to this Global Generator
std::unique_ptr<cmLocalGenerator>
cmGlobalVisualStudio7Generator::CreateLocalGenerator(cmMakefile* mf)
{
  auto lg = cm::make_unique<cmLocalVisualStudio7Generator>(this, mf);
  return std::unique_ptr<cmLocalGenerator>(std::move(lg));
}

#if !defined(CMAKE_BOOTSTRAP)
Json::Value cmGlobalVisualStudio7Generator::GetJson() const
{
  Json::Value generator = this->cmGlobalVisualStudioGenerator::GetJson();
  generator["platform"] = this->GetPlatformName();
  return generator;
}
#endif

bool cmGlobalVisualStudio7Generator::SetSystemName(std::string const& s,
                                                   cmMakefile* mf)
{
  mf->AddDefinition("CMAKE_VS_INTEL_Fortran_PROJECT_VERSION",
                    this->GetIntelProjectVersion());
  return this->cmGlobalVisualStudioGenerator::SetSystemName(s, mf);
}

void cmGlobalVisualStudio7Generator::AppendDirectoryForConfig(
  std::string const& prefix, std::string const& config,
  std::string const& suffix, std::string& dir)
{
  if (!config.empty()) {
    dir += cmStrCat(prefix, config, suffix);
  }
}

std::string cmGlobalVisualStudio7Generator::Encoding()
{
  return "UTF-8";
}
