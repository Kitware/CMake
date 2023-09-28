/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
#include "cmUuid.h"
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

namespace {
std::string GetSLNFile(cmLocalGenerator* root)
{
  return cmStrCat(root->GetCurrentBinaryDirectory(), '/',
                  root->GetProjectName(), ".sln");
}
}

cmGlobalVisualStudio7Generator::cmGlobalVisualStudio7Generator(
  cmake* cm, std::string const& platformInGeneratorName)
  : cmGlobalVisualStudioGenerator(cm, platformInGeneratorName)
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

const std::string& cmGlobalVisualStudio7Generator::GetIntelProjectVersion()
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
    sscanf(intelVersion.c_str(), "%u", &intelVersionNumber);
    if (intelVersionNumber >= 11) {
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

const char* cmGlobalVisualStudio7Generator::ExternalProjectType(
  const std::string& location)
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
  const std::string& makeProgram, const std::string& projectName,
  const std::string& /*projectDir*/,
  std::vector<std::string> const& targetNames, const std::string& config,
  int /*jobs*/, bool /*verbose*/, const cmBuildOptions& /*buildOptions*/,
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
  const bool requiresOutputForward =
    (makeProgramLower.find("vcexpress") != std::string::npos);
  std::vector<GeneratedMakeCommand> makeCommands;

  std::vector<std::string> realTargetNames = targetNames;
  if (targetNames.empty() ||
      ((targetNames.size() == 1) && targetNames.front().empty())) {
    realTargetNames = { "ALL_BUILD" };
  }
  for (const auto& tname : realTargetNames) {
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

void cmGlobalVisualStudio7Generator::Generate()
{
  // first do the superclass method
  this->cmGlobalVisualStudioGenerator::Generate();

  // Now write out the DSW
  this->OutputSLNFile();
  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  if (!cmSystemTools::GetErrorOccurredFlag() &&
      !this->LocalGenerators.empty()) {
    this->CallVisualStudioMacro(MacroReload,
                                GetSLNFile(this->LocalGenerators[0].get()));
  }

  if (this->Version == VSVersion::VS9 &&
      !this->CMakeInstance->GetIsInTryCompile()) {
    std::string cmakeWarnVS9;
    if (cmValue cached = this->CMakeInstance->GetState()->GetCacheEntryValue(
          "CMAKE_WARN_VS9")) {
      this->CMakeInstance->MarkCliAsUsed("CMAKE_WARN_VS9");
      cmakeWarnVS9 = *cached;
    } else {
      cmSystemTools::GetEnv("CMAKE_WARN_VS9", cmakeWarnVS9);
    }
    if (cmakeWarnVS9.empty() || !cmIsOff(cmakeWarnVS9)) {
      this->CMakeInstance->IssueMessage(
        MessageType::WARNING,
        "The \"Visual Studio 9 2008\" generator is deprecated "
        "and will be removed in a future version of CMake."
        "\n"
        "Add CMAKE_WARN_VS9=OFF to the cache to disable this warning.");
    }
  }

  if (this->Version == VSVersion::VS12 &&
      !this->CMakeInstance->GetIsInTryCompile()) {
    std::string cmakeWarnVS12;
    if (cmValue cached = this->CMakeInstance->GetState()->GetCacheEntryValue(
          "CMAKE_WARN_VS12")) {
      this->CMakeInstance->MarkCliAsUsed("CMAKE_WARN_VS12");
      cmakeWarnVS12 = *cached;
    } else {
      cmSystemTools::GetEnv("CMAKE_WARN_VS12", cmakeWarnVS12);
    }
    if (cmakeWarnVS12.empty() || !cmIsOff(cmakeWarnVS12)) {
      this->CMakeInstance->IssueMessage(
        MessageType::WARNING,
        "The \"Visual Studio 12 2013\" generator is deprecated "
        "and will be removed in a future version of CMake."
        "\n"
        "Add CMAKE_WARN_VS12=OFF to the cache to disable this warning.");
    }
  }
}

void cmGlobalVisualStudio7Generator::OutputSLNFile(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  if (generators.empty()) {
    return;
  }
  this->CurrentProject = root->GetProjectName();
  std::string fname = GetSLNFile(root);
  cmGeneratedFileStream fout(fname);
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }
  this->WriteSLNFile(fout, root, generators);
  if (fout.Close()) {
    this->FileReplacedDuringGenerate(fname);
  }
}

// output the SLN file
void cmGlobalVisualStudio7Generator::OutputSLNFile()
{
  for (auto& it : this->ProjectMap) {
    this->OutputSLNFile(it.second[0], it.second);
  }
}

void cmGlobalVisualStudio7Generator::WriteTargetConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs,
  OrderedTargetDependSet const& projectTargets)
{
  // loop over again and write out configurations for each target
  // in the solution
  for (cmGeneratorTarget const* target : projectTargets) {
    if (!this->IsInSolution(target)) {
      continue;
    }
    cmValue expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if (expath) {
      std::set<std::string> allConfigurations(configs.begin(), configs.end());
      cmValue mapping = target->GetProperty("VS_PLATFORM_MAPPING");
      this->WriteProjectConfigurations(fout, target->GetName(), *target,
                                       configs, allConfigurations,
                                       mapping ? *mapping : "");
    } else {
      const std::set<std::string>& configsPartOfDefaultBuild =
        this->IsPartOfDefaultBuild(configs, projectTargets, target);
      cmValue vcprojName = target->GetProperty("GENERATOR_FILE_NAME");
      if (vcprojName) {
        std::string mapping;

        // On VS 19 and above, always map .NET SDK projects to "Any CPU".
        if (target->IsDotNetSdkTarget() &&
            this->GetVersion() >= VSVersion::VS16 &&
            !cmGlobalVisualStudio7Generator::IsReservedTarget(
              target->GetName())) {
          mapping = "Any CPU";
        }
        this->WriteProjectConfigurations(fout, *vcprojName, *target, configs,
                                         configsPartOfDefaultBuild, mapping);
      }
    }
  }
}

void cmGlobalVisualStudio7Generator::WriteTargetsToSolution(
  std::ostream& fout, cmLocalGenerator* root,
  OrderedTargetDependSet const& projectTargets)
{
  VisualStudioFolders.clear();

  std::vector<std::string> configs =
    root->GetMakefile()->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);

  for (cmGeneratorTarget const* target : projectTargets) {
    if (!this->IsInSolution(target)) {
      continue;
    }
    bool written = false;

    for (auto const& c : configs) {
      target->CheckCxxModuleStatus(c);
    }

    // handle external vc project files
    cmValue expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if (expath) {
      std::string project = target->GetName();
      std::string const& location = *expath;

      this->WriteExternalProject(fout, project, location,
                                 target->GetProperty("VS_PROJECT_TYPE"),
                                 target->GetUtilities());
      written = true;
    } else {
      cmValue vcprojName = target->GetProperty("GENERATOR_FILE_NAME");
      if (vcprojName) {
        cmLocalGenerator* lg = target->GetLocalGenerator();
        std::string dir = lg->GetCurrentBinaryDirectory();
        dir = root->MaybeRelativeToCurBinDir(dir);
        if (dir == "."_s) {
          dir.clear(); // msbuild cannot handle ".\" prefix
        }
        this->WriteProject(fout, *vcprojName, dir, target);
        written = true;
      }
    }

    // Create "solution folder" information from FOLDER target property
    //
    if (written && this->UseFolderProperty()) {
      const std::string targetFolder = target->GetEffectiveFolderName();
      if (!targetFolder.empty()) {
        std::vector<std::string> tokens =
          cmSystemTools::SplitString(targetFolder, '/', false);

        std::string cumulativePath;

        for (std::string const& iter : tokens) {
          if (iter.empty()) {
            continue;
          }

          if (cumulativePath.empty()) {
            cumulativePath = cmStrCat("CMAKE_FOLDER_GUID_", iter);
          } else {
            VisualStudioFolders[cumulativePath].insert(
              cmStrCat(cumulativePath, '/', iter));

            cumulativePath = cmStrCat(cumulativePath, '/', iter);
          }
        }

        if (!cumulativePath.empty()) {
          VisualStudioFolders[cumulativePath].insert(target->GetName());
        }
      }
    }
  }
}

void cmGlobalVisualStudio7Generator::WriteFolders(std::ostream& fout)
{
  cm::string_view const prefix = "CMAKE_FOLDER_GUID_";
  std::string guidProjectTypeFolder = "2150E333-8FDC-42A3-9474-1A3956D46DE8";
  for (auto const& iter : VisualStudioFolders) {
    std::string fullName = iter.first;
    std::string guid = this->GetGUID(fullName);

    std::replace(fullName.begin(), fullName.end(), '/', '\\');
    if (cmHasPrefix(fullName, prefix)) {
      fullName = fullName.substr(prefix.size());
    }

    std::string nameOnly = cmSystemTools::GetFilenameName(fullName);

    fout << "Project(\"{" << guidProjectTypeFolder << "}\") = \"" << nameOnly
         << "\", \"" << fullName << "\", \"{" << guid << "}\"\nEndProject\n";
  }
}

void cmGlobalVisualStudio7Generator::WriteFoldersContent(std::ostream& fout)
{
  for (auto const& iter : VisualStudioFolders) {
    std::string key(iter.first);
    std::string guidParent(this->GetGUID(key));

    for (std::string const& it : iter.second) {
      std::string const& value(it);
      std::string guid(this->GetGUID(value));

      fout << "\t\t{" << guid << "} = {" << guidParent << "}\n";
    }
  }
}

std::string cmGlobalVisualStudio7Generator::ConvertToSolutionPath(
  const std::string& path)
{
  // Convert to backslashes.  Do not use ConvertToOutputPath because
  // we will add quoting ourselves, and we know these projects always
  // use windows slashes.
  std::string d = path;
  std::string::size_type pos = 0;
  while ((pos = d.find('/', pos)) != std::string::npos) {
    d[pos++] = '\\';
  }
  return d;
}

void cmGlobalVisualStudio7Generator::WriteSLNGlobalSections(
  std::ostream& fout, cmLocalGenerator* root)
{
  std::string const guid =
    this->GetGUID(cmStrCat(root->GetProjectName(), ".sln"));
  bool extensibilityGlobalsOverridden = false;
  bool extensibilityAddInsOverridden = false;
  const std::vector<std::string> propKeys =
    root->GetMakefile()->GetPropertyKeys();
  for (std::string const& it : propKeys) {
    if (cmHasLiteralPrefix(it, "VS_GLOBAL_SECTION_")) {
      std::string sectionType;
      std::string name = it.substr(18);
      if (cmHasLiteralPrefix(name, "PRE_")) {
        name = name.substr(4);
        sectionType = "preSolution";
      } else if (cmHasLiteralPrefix(name, "POST_")) {
        name = name.substr(5);
        sectionType = "postSolution";
      } else {
        continue;
      }
      if (!name.empty()) {
        bool addGuid = false;
        if (name == "ExtensibilityGlobals"_s &&
            sectionType == "postSolution"_s) {
          addGuid = true;
          extensibilityGlobalsOverridden = true;
        } else if (name == "ExtensibilityAddIns"_s &&
                   sectionType == "postSolution"_s) {
          extensibilityAddInsOverridden = true;
        }
        fout << "\tGlobalSection(" << name << ") = " << sectionType << '\n';
        cmValue p = root->GetMakefile()->GetProperty(it);
        cmList keyValuePairs{ *p };
        for (std::string const& itPair : keyValuePairs) {
          const std::string::size_type posEqual = itPair.find('=');
          if (posEqual != std::string::npos) {
            const std::string key =
              cmTrimWhitespace(itPair.substr(0, posEqual));
            const std::string value =
              cmTrimWhitespace(itPair.substr(posEqual + 1));
            fout << "\t\t" << key << " = " << value << '\n';
            if (key == "SolutionGuid"_s) {
              addGuid = false;
            }
          }
        }
        if (addGuid) {
          fout << "\t\tSolutionGuid = {" << guid << "}\n";
        }
        fout << "\tEndGlobalSection\n";
      }
    }
  }
  if (!extensibilityGlobalsOverridden) {
    fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
         << "\t\tSolutionGuid = {" << guid << "}\n"
         << "\tEndGlobalSection\n";
  }
  if (!extensibilityAddInsOverridden) {
    fout << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
         << "\tEndGlobalSection\n";
  }
}

// Standard end of dsw file
void cmGlobalVisualStudio7Generator::WriteSLNFooter(std::ostream& fout)
{
  fout << "EndGlobal\n";
}

std::string cmGlobalVisualStudio7Generator::WriteUtilityDepend(
  cmGeneratorTarget const* target)
{
  std::vector<std::string> configs =
    target->Target->GetMakefile()->GetGeneratorConfigs(
      cmMakefile::ExcludeEmptyConfig);
  std::string pname = cmStrCat(target->GetName(), "_UTILITY");
  std::string fname =
    cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(), '/',
             pname, ".vcproj");
  cmGeneratedFileStream fout(fname);
  fout.SetCopyIfDifferent(true);
  std::string guid = this->GetGUID(pname);

  /* clang-format off */
  fout <<
    R"(<?xml version="1.0" encoding = ")"
    << this->Encoding() << "\"?>\n"
    "<VisualStudioProject\n"
    "\tProjectType=\"Visual C++\"\n"
    "\tVersion=\"" << this->GetIDEVersion() << "0\"\n"
    "\tName=\"" << pname << "\"\n"
    "\tProjectGUID=\"{" << guid << "}\"\n"
    "\tKeyword=\"Win32Proj\">\n"
    "\t<Platforms><Platform Name=\"Win32\"/></Platforms>\n"
    "\t<Configurations>\n"
    ;
  /* clang-format on */
  for (std::string const& i : configs) {
    /* clang-format off */
    fout <<
      "\t\t<Configuration\n"
      "\t\t\tName=\"" << i << "|Win32\"\n"
      "\t\t\tOutputDirectory=\"" << i << "\"\n"
      "\t\t\tIntermediateDirectory=\"" << pname << ".dir\\" << i << "\"\n"
      "\t\t\tConfigurationType=\"10\"\n"
      "\t\t\tUseOfMFC=\"0\"\n"
      "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"\n"
      "\t\t\tCharacterSet=\"2\">\n"
      "\t\t</Configuration>\n"
      ;
    /* clang-format on */
  }
  /* clang-format off */
  fout <<
    "\t</Configurations>\n"
    "\t<Files></Files>\n"
    "\t<Globals></Globals>\n"
    "</VisualStudioProject>\n"
    ;
  /* clang-format on */

  if (fout.Close()) {
    this->FileReplacedDuringGenerate(fname);
  }
  return pname;
}

std::string cmGlobalVisualStudio7Generator::GetGUID(std::string const& name)
{
  std::string const& guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  if (cmValue storedGUID =
        this->CMakeInstance->GetCacheDefinition(guidStoreName)) {
    return *storedGUID;
  }
  // Compute a GUID that is deterministic but unique to the build tree.
  std::string input =
    cmStrCat(this->CMakeInstance->GetState()->GetBinaryDirectory(), '|', name);

  cmUuid uuidGenerator;

  std::vector<unsigned char> uuidNamespace;
  uuidGenerator.StringToBinary("ee30c4be-5192-4fb0-b335-722a2dffe760",
                               uuidNamespace);

  std::string guid = uuidGenerator.FromMd5(uuidNamespace, input);

  return cmSystemTools::UpperCase(guid);
}

void cmGlobalVisualStudio7Generator::AppendDirectoryForConfig(
  const std::string& prefix, const std::string& config,
  const std::string& suffix, std::string& dir)
{
  if (!config.empty()) {
    dir += cmStrCat(prefix, config, suffix);
  }
}

std::set<std::string> cmGlobalVisualStudio7Generator::IsPartOfDefaultBuild(
  std::vector<std::string> const& configs,
  OrderedTargetDependSet const& projectTargets,
  cmGeneratorTarget const* target)
{
  std::set<std::string> activeConfigs;
  // if it is a utility target then only make it part of the
  // default build if another target depends on it
  int type = target->GetType();
  if (type == cmStateEnums::GLOBAL_TARGET) {
    std::vector<std::string> targetNames;
    targetNames.push_back("INSTALL");
    targetNames.push_back("PACKAGE");
    for (std::string const& t : targetNames) {
      // check if target <t> is part of default build
      if (target->GetName() == t) {
        const std::string propertyName =
          cmStrCat("CMAKE_VS_INCLUDE_", t, "_TO_DEFAULT_BUILD");
        // inspect CMAKE_VS_INCLUDE_<t>_TO_DEFAULT_BUILD properties
        for (std::string const& i : configs) {
          cmValue propertyValue =
            target->Target->GetMakefile()->GetDefinition(propertyName);
          if (propertyValue &&
              cmIsOn(cmGeneratorExpression::Evaluate(
                *propertyValue, target->GetLocalGenerator(), i))) {
            activeConfigs.insert(i);
          }
        }
      }
    }
    return activeConfigs;
  }
  if (type == cmStateEnums::UTILITY &&
      !this->IsDependedOn(projectTargets, target)) {
    return activeConfigs;
  }
  // inspect EXCLUDE_FROM_DEFAULT_BUILD[_<CONFIG>] properties
  for (std::string const& i : configs) {
    if (cmIsOff(target->GetFeature("EXCLUDE_FROM_DEFAULT_BUILD", i))) {
      activeConfigs.insert(i);
    }
  }
  return activeConfigs;
}

bool cmGlobalVisualStudio7Generator::IsDependedOn(
  OrderedTargetDependSet const& projectTargets, cmGeneratorTarget const* gtIn)
{
  return std::any_of(projectTargets.begin(), projectTargets.end(),
                     [this, gtIn](cmTargetDepend const& l) {
                       TargetDependSet const& tgtdeps =
                         this->GetTargetDirectDepends(l);
                       return tgtdeps.count(gtIn);
                     });
}

std::string cmGlobalVisualStudio7Generator::Encoding()
{
  return "UTF-8";
}
