/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalXCodeGenerator.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmComputeLinkInformation.h"
#include "cmCryptoHash.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmCustomCommandLines.h"
#include "cmCustomCommandTypes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmXCode21Object.h"
#include "cmXCodeObject.h"
#include "cmXCodeScheme.h"
#include "cmXMLWriter.h"
#include "cmXcFramework.h"
#include "cmake.h"

#if !defined(CMAKE_BOOTSTRAP) && defined(__APPLE__)
#  include <CoreFoundation/CoreFoundation.h>
#  if !TARGET_OS_IPHONE
#    define HAVE_APPLICATION_SERVICES
#    include <ApplicationServices/ApplicationServices.h>
#  endif
#endif

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmXMLParser.h"

// parse the xml file storing the installed version of Xcode on
// the machine
class cmXcodeVersionParser : public cmXMLParser
{
public:
  cmXcodeVersionParser()
    : Version("1.5")
  {
  }
  void StartElement(const std::string&, const char**) override
  {
    this->Data = "";
  }
  void EndElement(const std::string& name) override
  {
    if (name == "key"_s) {
      this->Key = this->Data;
    } else if (name == "string"_s) {
      if (this->Key == "CFBundleShortVersionString"_s) {
        this->Version = this->Data;
      }
    }
  }
  void CharacterDataHandler(const char* data, int length) override
  {
    this->Data.append(data, length);
  }
  std::string Version;
  std::string Key;
  std::string Data;
};
#endif

// Builds either an object list or a space-separated string from the
// given inputs.
class cmGlobalXCodeGenerator::BuildObjectListOrString
{
  cmGlobalXCodeGenerator* Generator;
  cmXCodeObject* Group = nullptr;
  bool Empty = true;
  std::string String;

public:
  BuildObjectListOrString(cmGlobalXCodeGenerator* gen, bool buildObjectList)
    : Generator(gen)
  {
    if (buildObjectList) {
      this->Group = this->Generator->CreateObject(cmXCodeObject::OBJECT_LIST);
    }
  }

  bool IsEmpty() const { return this->Empty; }

  void Add(const std::string& newString)
  {
    this->Empty = false;

    if (this->Group) {
      this->Group->AddObject(this->Generator->CreateString(newString));
    } else {
      this->String += newString;
      this->String += ' ';
    }
  }

  const std::string& GetString() const { return this->String; }

  cmXCodeObject* CreateList()
  {
    if (this->Group) {
      return this->Group;
    }
    return this->Generator->CreateString(this->String);
  }
};

class cmGlobalXCodeGenerator::Factory : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override;

  cmDocumentationEntry GetDocumentation() const override
  {
    return cmGlobalXCodeGenerator::GetDocumentation();
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(cmGlobalXCodeGenerator::GetActualName());
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    return std::vector<std::string>();
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return false; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    return std::vector<std::string>();
  }

  std::string GetDefaultPlatformName() const override { return std::string(); }
};

cmGlobalXCodeGenerator::cmGlobalXCodeGenerator(
  cmake* cm, std::string const& version_string, unsigned int version_number)
  : cmGlobalGenerator(cm)
{
  this->VersionString = version_string;
  this->XcodeVersion = version_number;
  if (this->XcodeVersion >= 120) {
    this->XcodeBuildSystem = BuildSystem::Twelve;
  } else {
    this->XcodeBuildSystem = BuildSystem::One;
  }

  this->RootObject = nullptr;
  this->MainGroupChildren = nullptr;
  this->FrameworkGroup = nullptr;
  this->CurrentMakefile = nullptr;
  this->CurrentLocalGenerator = nullptr;
  this->XcodeBuildCommandInitialized = false;

  this->ObjectDirArchDefault = "$(CURRENT_ARCH)";
  this->ObjectDirArch = this->ObjectDirArchDefault;

  cm->GetState()->SetIsGeneratorMultiConfig(true);
}

std::unique_ptr<cmGlobalGeneratorFactory> cmGlobalXCodeGenerator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

std::unique_ptr<cmGlobalGenerator>
cmGlobalXCodeGenerator::Factory::CreateGlobalGenerator(const std::string& name,
                                                       bool /*allowArch*/,
                                                       cmake* cm) const
{
  if (name != GetActualName()) {
    return std::unique_ptr<cmGlobalGenerator>();
  }
#if !defined(CMAKE_BOOTSTRAP)
  cmXcodeVersionParser parser;
  std::string versionFile;
  {
    std::string out;
    bool commandResult = cmSystemTools::RunSingleCommand(
      "xcode-select --print-path", &out, nullptr, nullptr, nullptr,
      cmSystemTools::OUTPUT_NONE);
    if (commandResult) {
      std::string::size_type pos = out.find(".app/");
      if (pos != std::string::npos) {
        versionFile =
          cmStrCat(out.substr(0, pos + 5), "Contents/version.plist");
      }
    }
  }
  if (!versionFile.empty() && cmSystemTools::FileExists(versionFile)) {
    parser.ParseFile(versionFile.c_str());
  } else if (cmSystemTools::FileExists(
               "/Applications/Xcode.app/Contents/version.plist")) {
    parser.ParseFile("/Applications/Xcode.app/Contents/version.plist");
  } else {
    parser.ParseFile(
      "/Developer/Applications/Xcode.app/Contents/version.plist");
  }
  std::string const& version_string = parser.Version;

  // Compute an integer form of the version number.
  unsigned int v[2] = { 0, 0 };
  sscanf(version_string.c_str(), "%u.%u", &v[0], &v[1]);
  unsigned int version_number = 10 * v[0] + v[1];

  if (version_number < 50) {
    cm->IssueMessage(MessageType::FATAL_ERROR,
                     cmStrCat("Xcode ", version_string, " not supported."));
    return std::unique_ptr<cmGlobalGenerator>();
  }

  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalXCodeGenerator>(cm, version_string,
                                            version_number));
#else
  std::cerr << "CMake should be built with cmake to use Xcode, "
               "default to Xcode 1.5\n";
  return std::unique_ptr<cmGlobalGenerator>(
    cm::make_unique<cmGlobalXCodeGenerator>(cm));
#endif
}

bool cmGlobalXCodeGenerator::FindMakeProgram(cmMakefile* mf)
{
  // The Xcode generator knows how to lookup its build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM", this->GetXcodeBuildCommand());
  }
  return true;
}

std::string const& cmGlobalXCodeGenerator::GetXcodeBuildCommand()
{
  if (!this->XcodeBuildCommandInitialized) {
    this->XcodeBuildCommandInitialized = true;
    this->XcodeBuildCommand = this->FindXcodeBuildCommand();
  }
  return this->XcodeBuildCommand;
}

std::string cmGlobalXCodeGenerator::FindXcodeBuildCommand()
{
  std::string makeProgram = cmSystemTools::FindProgram("xcodebuild");
  if (makeProgram.empty()) {
    makeProgram = "xcodebuild";
  }
  return makeProgram;
}

bool cmGlobalXCodeGenerator::SetSystemName(std::string const& s,
                                           cmMakefile* mf)
{
  this->SystemName = s;
  return this->cmGlobalGenerator::SetSystemName(s, mf);
}

namespace {
cm::string_view cmXcodeBuildSystemString(cmGlobalXCodeGenerator::BuildSystem b)
{
  switch (b) {
    case cmGlobalXCodeGenerator::BuildSystem::One:
      return "1"_s;
    case cmGlobalXCodeGenerator::BuildSystem::Twelve:
      return "12"_s;
  }
  return {};
}
}

bool cmGlobalXCodeGenerator::SetGeneratorToolset(std::string const& ts,
                                                 bool build, cmMakefile* mf)
{
  if (!this->ParseGeneratorToolset(ts, mf)) {
    return false;
  }
  if (build) {
    return true;
  }
  if (!this->GeneratorToolset.empty()) {
    mf->AddDefinition("CMAKE_XCODE_PLATFORM_TOOLSET", this->GeneratorToolset);
  }
  mf->AddDefinition("CMAKE_XCODE_BUILD_SYSTEM",
                    cmXcodeBuildSystemString(this->XcodeBuildSystem));
  return true;
}

bool cmGlobalXCodeGenerator::ParseGeneratorToolset(std::string const& ts,
                                                   cmMakefile* mf)
{
  std::vector<std::string> const fields = cmTokenize(ts, ",");
  auto fi = fields.cbegin();
  if (fi == fields.cend()) {
    return true;
  }

  // The first field may be the Xcode GCC_VERSION.
  if (fi->find('=') == fi->npos) {
    this->GeneratorToolset = *fi;
    ++fi;
  }

  std::unordered_set<std::string> handled;

  // The rest of the fields must be key=value pairs.
  for (; fi != fields.cend(); ++fi) {
    std::string::size_type pos = fi->find('=');
    if (pos == fi->npos) {
      /* clang-format off */
      std::string const& e = cmStrCat(
        "Generator\n"
        "  ", this->GetName(), "\n"
        "given toolset specification\n"
        "  ", ts, "\n"
        "that contains a field after the first ',' with no '='."
        );
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e);
      return false;
    }
    std::string const key = fi->substr(0, pos);
    std::string const value = fi->substr(pos + 1);
    if (!handled.insert(key).second) {
      /* clang-format off */
      std::string const& e = cmStrCat(
        "Generator\n"
        "  ", this->GetName(), "\n"
        "given toolset specification\n"
        "  ", ts, "\n"
        "that contains duplicate field key '", key, "'."
        );
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e);
      return false;
    }
    if (!this->ProcessGeneratorToolsetField(key, value, mf)) {
      return false;
    }
  }

  return true;
}

bool cmGlobalXCodeGenerator::ProcessGeneratorToolsetField(
  std::string const& key, std::string const& value, cmMakefile* mf)
{
  if (key == "buildsystem"_s) {
    if (value == "1"_s) {
      this->XcodeBuildSystem = BuildSystem::One;
    } else if (value == "12"_s) {
      this->XcodeBuildSystem = BuildSystem::Twelve;
    } else {
      /* clang-format off */
      std::string const& e = cmStrCat(
        "Generator\n"
        "  ",  this->GetName(), "\n"
        "toolset specification field\n"
        "  buildsystem=", value, "\n"
        "value is unknown.  It must be '1' or '12'."
        );
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e);
      return false;
    }
    if ((this->XcodeBuildSystem == BuildSystem::Twelve &&
         this->XcodeVersion < 120) ||
        (this->XcodeBuildSystem == BuildSystem::One &&
         this->XcodeVersion >= 140)) {
      /* clang-format off */
      std::string const& e = cmStrCat(
        "Generator\n"
        "  ",  this->GetName(), "\n"
        "toolset specification field\n"
        "  buildsystem=", value, "\n"
        "is not allowed with Xcode ", this->VersionString, '.'
        );
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e);
      return false;
    }
    return true;
  }
  /* clang-format off */
  std::string const& e = cmStrCat(
    "Generator\n"
    "  ", this->GetName(), "\n"
    "given toolset specification that contains invalid field '", key, "'."
    );
  /* clang-format on */
  mf->IssueMessage(MessageType::FATAL_ERROR, e);
  return false;
}

void cmGlobalXCodeGenerator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("XCODE", "1");
  mf->AddDefinition("XCODE_VERSION", this->VersionString);
  mf->InitCMAKE_CONFIGURATION_TYPES("Debug;Release;MinSizeRel;RelWithDebInfo");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
  this->ComputeArchitectures(mf);
}

bool cmGlobalXCodeGenerator::Open(const std::string& bindir,
                                  const std::string& projectName, bool dryRun)
{
  bool ret = false;

#ifdef HAVE_APPLICATION_SERVICES
  std::string url = cmStrCat(bindir, '/', projectName, ".xcodeproj");

  if (dryRun) {
    return cmSystemTools::FileExists(url, false);
  }

  CFStringRef cfStr = CFStringCreateWithCString(
    kCFAllocatorDefault, url.c_str(), kCFStringEncodingUTF8);
  if (cfStr) {
    CFURLRef cfUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfStr,
                                                   kCFURLPOSIXPathStyle, true);
    if (cfUrl) {
      OSStatus err = LSOpenCFURLRef(cfUrl, nullptr);
      ret = err == noErr;
      CFRelease(cfUrl);
    }
    CFRelease(cfStr);
  }
#else
  (void)bindir;
  (void)projectName;
  (void)dryRun;
#endif

  return ret;
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalXCodeGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& /*projectDir*/,
  std::vector<std::string> const& targetNames, const std::string& config,
  int jobs, bool /*verbose*/, const cmBuildOptions& /*buildOptions*/,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand;
  // now build the test
  makeCommand.Add(
    this->SelectMakeProgram(makeProgram, this->GetXcodeBuildCommand()));

  if (!projectName.empty()) {
    makeCommand.Add("-project");
    std::string projectArg = cmStrCat(projectName, ".xcodeproj");
    makeCommand.Add(projectArg);
  }
  if (cm::contains(targetNames, "clean")) {
    makeCommand.Add("clean");
    makeCommand.Add("-target", "ALL_BUILD");
  } else {
    makeCommand.Add("build");
    if (targetNames.empty() ||
        ((targetNames.size() == 1) && targetNames.front().empty())) {
      makeCommand.Add("-target", "ALL_BUILD");
    } else {
      for (const auto& tname : targetNames) {
        if (!tname.empty()) {
          makeCommand.Add("-target", tname);
        }
      }
    }
  }

  if ((this->XcodeBuildSystem >= BuildSystem::Twelve) ||
      (jobs != cmake::NO_BUILD_PARALLEL_LEVEL)) {
    makeCommand.Add("-parallelizeTargets");
  }
  makeCommand.Add("-configuration", (config.empty() ? "Debug" : config));

  if ((jobs != cmake::NO_BUILD_PARALLEL_LEVEL) &&
      (jobs != cmake::DEFAULT_BUILD_PARALLEL_LEVEL)) {
    makeCommand.Add("-jobs", std::to_string(jobs));
  }

  if (this->XcodeVersion >= 70) {
    makeCommand.Add("-hideShellScriptEnvironment");
  }
  makeCommand.Add(makeOptions.begin(), makeOptions.end());
  return { std::move(makeCommand) };
}

//! Create a local generator appropriate to this Global Generator
std::unique_ptr<cmLocalGenerator> cmGlobalXCodeGenerator::CreateLocalGenerator(
  cmMakefile* mf)
{
  std::unique_ptr<cmLocalGenerator> lg(
    cm::make_unique<cmLocalXCodeGenerator>(this, mf));
  if (this->XcodeBuildSystem >= BuildSystem::Twelve) {
    // For this build system variant we generate custom commands as
    // shell scripts directly rather than inside Makefiles.
    // FIXME: Rename or refactor this option for clarity.
    lg->SetLinkScriptShell(true);
  }
  return lg;
}

void cmGlobalXCodeGenerator::AddExtraIDETargets()
{
  // make sure extra targets are added before calling
  // the parent generate which will call trace depends
  for (auto keyVal : this->ProjectMap) {
    cmLocalGenerator* root = keyVal.second[0];
    this->SetGenerationRoot(root);
    // add ALL_BUILD, INSTALL, etc
    this->AddExtraTargets(root, keyVal.second);
  }
}

void cmGlobalXCodeGenerator::Generate()
{
  this->cmGlobalGenerator::Generate();
  if (cmSystemTools::GetErrorOccurredFlag()) {
    return;
  }

  for (auto keyVal : this->ProjectMap) {
    cmLocalGenerator* root = keyVal.second[0];

    bool generateTopLevelProjectOnly =
      root->GetMakefile()->IsOn("CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY");

    if (generateTopLevelProjectOnly) {
      cmStateSnapshot snp = root->GetStateSnapshot();
      if (snp.GetBuildsystemDirectoryParent().IsValid()) {
        continue;
      }
    }

    // cache the enabled languages for source file type queries
    this->GetEnabledLanguages(this->EnabledLangs);

    this->SetGenerationRoot(root);
    // now create the project
    this->OutputXCodeProject(root, keyVal.second);
  }
}

void cmGlobalXCodeGenerator::SetGenerationRoot(cmLocalGenerator* root)
{
  this->CurrentProject = root->GetProjectName();
  this->SetCurrentLocalGenerator(root);
  this->CurrentRootGenerator = root;
  this->CurrentXCodeHackMakefile =
    cmStrCat(root->GetCurrentBinaryDirectory(), "/CMakeScripts");
  cmSystemTools::MakeDirectory(this->CurrentXCodeHackMakefile);
  this->CurrentXCodeHackMakefile += "/XCODE_DEPEND_HELPER.make";
}

std::string cmGlobalXCodeGenerator::PostBuildMakeTarget(
  std::string const& tName, std::string const& configName)
{
  std::string target = tName;
  std::replace(target.begin(), target.end(), ' ', '_');
  std::string out = cmStrCat("PostBuild.", target, '.', configName);
  return out;
}

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

void cmGlobalXCodeGenerator::AddExtraTargets(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& gens)
{
  // Add ALL_BUILD
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetCommandLines(
    cmMakeSingleCommandLine({ "echo", "Build all projects" }));
  cmTarget* allbuild =
    root->AddUtilityCommand("ALL_BUILD", true, std::move(cc));

  // Add xcconfig files to ALL_BUILD sources
  for (auto& config : this->CurrentConfigurationTypes) {
    auto xcconfig = cmGeneratorExpression::Evaluate(
      this->CurrentMakefile->GetSafeDefinition("CMAKE_XCODE_XCCONFIG"),
      this->CurrentLocalGenerator, config);
    if (!xcconfig.empty()) {
      allbuild->AddSource(xcconfig);
    }
  }

  root->AddGeneratorTarget(cm::make_unique<cmGeneratorTarget>(allbuild, root));

  // Add XCODE depend helper
  std::string legacyDependHelperDir = root->GetCurrentBinaryDirectory();
  cmCustomCommandLines legacyDependHelperCommandLines;
  if (this->XcodeBuildSystem == BuildSystem::One) {
    legacyDependHelperCommandLines = cmMakeSingleCommandLine(
      { "make", "-C", legacyDependHelperDir, "-f",
        this->CurrentXCodeHackMakefile, "OBJDIR=$(OBJDIR)",
        /* placeholder, see below */ "" });
  }

  // Add ZERO_CHECK
  bool regenerate = !this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION");
  bool generateTopLevelProjectOnly =
    root->GetMakefile()->IsOn("CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY");
  bool isTopLevel =
    !root->GetStateSnapshot().GetBuildsystemDirectoryParent().IsValid();
  bool isGenerateProject = isTopLevel || !generateTopLevelProjectOnly;
  if (regenerate && isGenerateProject) {
    this->CreateReRunCMakeFile(root, gens);
    std::string file =
      this->ConvertToRelativeForMake(this->CurrentReRunCMakeMakefile);
    cmSystemTools::ReplaceString(file, "\\ ", " ");
    cc = cm::make_unique<cmCustomCommand>();
    cc->SetCommandLines(cmMakeSingleCommandLine({ "make", "-f", file }));
    cmTarget* check = root->AddUtilityCommand(CMAKE_CHECK_BUILD_SYSTEM_TARGET,
                                              true, std::move(cc));

    root->AddGeneratorTarget(cm::make_unique<cmGeneratorTarget>(check, root));
  }

  // now make the allbuild depend on all the non-utility targets
  // in the project
  for (auto& gen : gens) {
    for (const auto& target : gen->GetGeneratorTargets()) {
      if (target->GetType() == cmStateEnums::GLOBAL_TARGET) {
        continue;
      }

      if (regenerate &&
          (target->GetName() != CMAKE_CHECK_BUILD_SYSTEM_TARGET)) {
        target->Target->AddUtility(CMAKE_CHECK_BUILD_SYSTEM_TARGET, false);
      }

      // make all exe, shared libs and modules
      // run the depend check makefile as a post build rule
      // this will make sure that when the next target is built
      // things are up-to-date
      if (this->XcodeBuildSystem == BuildSystem::One && isGenerateProject &&
          target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        legacyDependHelperCommandLines.front().back() = // fill placeholder
          this->PostBuildMakeTarget(target->GetName(), "$(CONFIGURATION)");
        cc = cm::make_unique<cmCustomCommand>();
        cc->SetCommandLines(legacyDependHelperCommandLines);
        cc->SetComment("Depend check for xcode");
        cc->SetWorkingDirectory(legacyDependHelperDir.c_str());
        gen->AddCustomCommandToTarget(
          target->GetName(), cmCustomCommandType::POST_BUILD, std::move(cc),
          cmObjectLibraryCommands::Accept);
      }

      if (!this->IsExcluded(gens[0], target.get())) {
        allbuild->AddUtility(target->GetName(), false);
      }
    }
  }
}

void cmGlobalXCodeGenerator::CreateReRunCMakeFile(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*> const& gens)
{
  std::vector<std::string> lfiles;
  for (auto* gen : gens) {
    cm::append(lfiles, gen->GetMakefile()->GetListFiles());
  }

  // sort the array
  std::sort(lfiles.begin(), lfiles.end());
  lfiles.erase(std::unique(lfiles.begin(), lfiles.end()), lfiles.end());

  cmake* cm = this->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    lfiles.emplace_back(cm->GetGlobVerifyStamp());
  }

  this->CurrentReRunCMakeMakefile =
    cmStrCat(root->GetCurrentBinaryDirectory(), "/CMakeScripts");
  cmSystemTools::MakeDirectory(this->CurrentReRunCMakeMakefile);
  this->CurrentReRunCMakeMakefile += "/ReRunCMake.make";
  cmGeneratedFileStream makefileStream(this->CurrentReRunCMakeMakefile);
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n\n"

                    "TARGETS:= \n"
                    "empty:= \n"
                    "space:= $(empty) $(empty)\n"
                    "spaceplus:= $(empty)\\ $(empty)\n\n";

  for (const auto& lfile : lfiles) {
    makefileStream << "TARGETS += $(subst $(space),$(spaceplus),$(wildcard "
                   << this->ConvertToRelativeForMake(lfile) << "))\n";
  }
  makefileStream << '\n';

  std::string checkCache =
    cmStrCat(root->GetBinaryDirectory(), "/CMakeFiles/cmake.check_cache");

  if (cm->DoWriteGlobVerifyTarget()) {
    makefileStream << ".NOTPARALLEL:\n\n"
                      ".PHONY: all VERIFY_GLOBS\n\n"
                      "all: VERIFY_GLOBS "
                   << this->ConvertToRelativeForMake(checkCache)
                   << "\n\n"
                      "VERIFY_GLOBS:\n"
                      "\t"
                   << this->ConvertToRelativeForMake(
                        cmSystemTools::GetCMakeCommand())
                   << " -P "
                   << this->ConvertToRelativeForMake(cm->GetGlobVerifyScript())
                   << "\n\n";
  }

  makefileStream << this->ConvertToRelativeForMake(checkCache)
                 << ": $(TARGETS)\n";
  makefileStream
    << '\t' << this->ConvertToRelativeForMake(cmSystemTools::GetCMakeCommand())
    << " -S" << this->ConvertToRelativeForMake(root->GetSourceDirectory())
    << " -B" << this->ConvertToRelativeForMake(root->GetBinaryDirectory())
    << (cm->GetIgnoreWarningAsError() ? " --compile-no-warning-as-error" : "")
    << '\n';
}

static bool objectIdLessThan(const std::unique_ptr<cmXCodeObject>& l,
                             const std::unique_ptr<cmXCodeObject>& r)
{
  return l->GetId() < r->GetId();
}

void cmGlobalXCodeGenerator::SortXCodeObjects()
{
  std::sort(this->XCodeObjects.begin(), this->XCodeObjects.end(),
            objectIdLessThan);
}

void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  this->TargetDoneSet.clear();
  this->XCodeObjects.clear();
  this->XCodeObjectIDs.clear();
  this->XCodeObjectMap.clear();
  this->GroupMap.clear();
  this->GroupNameMap.clear();
  this->TargetGroup.clear();
  this->FileRefs.clear();
  this->ExternalLibRefs.clear();
  this->EmbeddedLibRefs.clear();
  this->FileRefToBuildFileMap.clear();
  this->FileRefToEmbedBuildFileMap.clear();
  this->CommandsVisited.clear();
}

void cmGlobalXCodeGenerator::addObject(std::unique_ptr<cmXCodeObject> obj)
{
  if (obj->GetType() == cmXCodeObject::OBJECT) {
    const std::string& id = obj->GetId();

    // If this is a duplicate id, it's an error:
    //
    if (this->XCodeObjectIDs.count(id)) {
      cmSystemTools::Error(
        "Xcode generator: duplicate object ids not allowed");
    }

    this->XCodeObjectIDs.insert(id);
  }

  this->XCodeObjects.push_back(std::move(obj));
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateObject(
  cmXCodeObject::PBXType ptype, cm::string_view key)
{
  auto obj = cm::make_unique<cmXCode21Object>(ptype, cmXCodeObject::OBJECT,
                                              this->GetObjectId(ptype, key));
  auto* ptr = obj.get();
  this->addObject(std::move(obj));
  return ptr;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  auto obj = cm::make_unique<cmXCodeObject>(
    cmXCodeObject::None, type,
    "Temporary cmake object, should not be referred to in Xcode file");
  auto* ptr = obj.get();
  this->addObject(std::move(obj));
  return ptr;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateString(const std::string& s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateObjectReference(
  cmXCodeObject* ref)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::OBJECT_REF);
  obj->SetObject(ref);
  return obj;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateFlatClone(cmXCodeObject* orig)
{
  cmXCodeObject* obj = this->CreateObject(orig->GetType());
  obj->CopyAttributes(orig);
  return obj;
}

static std::string GetGroupMapKeyFromPath(cmGeneratorTarget* target,
                                          const std::string& fullpath)
{
  return cmStrCat(target->GetName(), '-', fullpath);
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateXCodeBuildFileFromPath(
  const std::string& fullpath, cmGeneratorTarget* target,
  const std::string& lang, cmSourceFile* sf)
{
  // Using a map and the full path guarantees that we will always get the same
  // fileRef object for any given full path. Same goes for the buildFile
  // object.
  cmXCodeObject* fileRef =
    this->CreateXCodeFileReferenceFromPath(fullpath, target, lang, sf);
  if (fileRef) {
    auto it = this->FileRefToBuildFileMap.find(fileRef);
    if (it == this->FileRefToBuildFileMap.end()) {
      cmXCodeObject* buildFile =
        this->CreateObject(cmXCodeObject::PBXBuildFile);
      buildFile->SetComment(fileRef->GetComment());
      buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));
      this->FileRefToBuildFileMap[fileRef] = buildFile;
      return buildFile;
    }
    return it->second;
  }
  return nullptr;
}

class XCodeGeneratorExpressionInterpreter
  : public cmGeneratorExpressionInterpreter
{
public:
  XCodeGeneratorExpressionInterpreter(cmSourceFile* sourceFile,
                                      cmLocalGenerator* localGenerator,
                                      cmGeneratorTarget* headTarget,
                                      const std::string& lang)
    : cmGeneratorExpressionInterpreter(
        localGenerator, "NO-PER-CONFIG-SUPPORT-IN-XCODE", headTarget, lang)
    , SourceFile(sourceFile)
  {
  }

  XCodeGeneratorExpressionInterpreter(
    XCodeGeneratorExpressionInterpreter const&) = delete;
  XCodeGeneratorExpressionInterpreter& operator=(
    XCodeGeneratorExpressionInterpreter const&) = delete;

  const std::string& Evaluate(const char* expression,
                              const std::string& property)
  {
    return this->Evaluate(std::string(expression ? expression : ""), property);
  }

  const std::string& Evaluate(const std::string& expression,
                              const std::string& property)
  {
    const std::string& processed =
      this->cmGeneratorExpressionInterpreter::Evaluate(expression, property);
    if (this->CompiledGeneratorExpression->GetHadContextSensitiveCondition()) {
      std::ostringstream e;
      /* clang-format off */
      e <<
          "Xcode does not support per-config per-source " << property << ":\n"
          "  " << expression << "\n"
          "specified for source:\n"
          "  " << this->SourceFile->ResolveFullPath() << '\n';
      /* clang-format on */
      this->LocalGenerator->IssueMessage(MessageType::FATAL_ERROR, e.str());
    }

    return processed;
  }

private:
  cmSourceFile* SourceFile = nullptr;
};

cmXCodeObject* cmGlobalXCodeGenerator::CreateXCodeSourceFile(
  cmLocalGenerator* lg, cmSourceFile* sf, cmGeneratorTarget* gtgt)
{
  std::string lang = this->CurrentLocalGenerator->GetSourceFileLanguage(*sf);

  XCodeGeneratorExpressionInterpreter genexInterpreter(sf, lg, gtgt, lang);

  // Add flags from target and source file properties.
  std::string flags;
  std::string const& srcfmt = sf->GetSafeProperty("Fortran_FORMAT");
  switch (cmOutputConverter::GetFortranFormat(srcfmt)) {
    case cmOutputConverter::FortranFormatFixed:
      flags = cmStrCat("-fixed ", flags);
      break;
    case cmOutputConverter::FortranFormatFree:
      flags = cmStrCat("-free ", flags);
      break;
    default:
      break;
  }

  // Explicitly add the explicit language flag before any other flag
  // so user flags can override it.
  gtgt->AddExplicitLanguageFlags(flags, *sf);

  const std::string COMPILE_FLAGS("COMPILE_FLAGS");
  if (cmValue cflags = sf->GetProperty(COMPILE_FLAGS)) {
    lg->AppendFlags(flags, genexInterpreter.Evaluate(*cflags, COMPILE_FLAGS));
  }
  const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
  if (cmValue coptions = sf->GetProperty(COMPILE_OPTIONS)) {
    lg->AppendCompileOptions(
      flags, genexInterpreter.Evaluate(*coptions, COMPILE_OPTIONS));
  }

  // Add per-source definitions.
  BuildObjectListOrString flagsBuild(this, false);
  const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
  if (cmValue compile_defs = sf->GetProperty(COMPILE_DEFINITIONS)) {
    this->AppendDefines(
      flagsBuild,
      genexInterpreter.Evaluate(*compile_defs, COMPILE_DEFINITIONS).c_str(),
      true);
  }

  if (sf->GetPropertyAsBool("SKIP_PRECOMPILE_HEADERS")) {
    this->AppendDefines(flagsBuild, "CMAKE_SKIP_PRECOMPILE_HEADERS", true);
  }

  if (!flagsBuild.IsEmpty()) {
    if (!flags.empty()) {
      flags += ' ';
    }
    flags += flagsBuild.GetString();
  }

  // Add per-source include directories.
  std::vector<std::string> includes;
  const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
  if (cmValue cincludes = sf->GetProperty(INCLUDE_DIRECTORIES)) {
    lg->AppendIncludeDirectories(
      includes, genexInterpreter.Evaluate(*cincludes, INCLUDE_DIRECTORIES),
      *sf);
  }
  lg->AppendFlags(flags,
                  lg->GetIncludeFlags(includes, gtgt, lang, std::string()));

  cmXCodeObject* buildFile =
    this->CreateXCodeBuildFileFromPath(sf->ResolveFullPath(), gtgt, lang, sf);

  cmXCodeObject* settings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  settings->AddAttributeIfNotEmpty("COMPILER_FLAGS",
                                   this->CreateString(flags));

  cmGeneratorTarget::SourceFileFlags tsFlags =
    gtgt->GetTargetSourceFileFlags(sf);

  cmXCodeObject* attrs = this->CreateObject(cmXCodeObject::OBJECT_LIST);

  // Is this a "private" or "public" framework header file?
  // Set the ATTRIBUTES attribute appropriately...
  //
  if (gtgt->IsFrameworkOnApple()) {
    if (tsFlags.Type == cmGeneratorTarget::SourceFileTypePrivateHeader) {
      attrs->AddObject(this->CreateString("Private"));
    } else if (tsFlags.Type == cmGeneratorTarget::SourceFileTypePublicHeader) {
      attrs->AddObject(this->CreateString("Public"));
    }
  }

  // Add user-specified file attributes.
  cmValue extraFileAttributes = sf->GetProperty("XCODE_FILE_ATTRIBUTES");
  if (extraFileAttributes) {
    // Expand the list of attributes.
    cmList attributes{ *extraFileAttributes };

    // Store the attributes.
    for (const auto& attribute : attributes) {
      attrs->AddObject(this->CreateString(attribute));
    }
  }

  settings->AddAttributeIfNotEmpty("ATTRIBUTES", attrs);

  if (buildFile) {
    buildFile->AddAttributeIfNotEmpty("settings", settings);
  }
  return buildFile;
}

void cmGlobalXCodeGenerator::AddXCodeProjBuildRule(
  cmGeneratorTarget* target, std::vector<cmSourceFile*>& sources) const
{
  std::string listfile =
    cmStrCat(target->GetLocalGenerator()->GetCurrentSourceDirectory(),
             "/CMakeLists.txt");
  cmSourceFile* srcCMakeLists = target->Makefile->GetOrCreateSource(
    listfile, false, cmSourceFileLocationKind::Known);
  if (!cm::contains(sources, srcCMakeLists)) {
    sources.push_back(srcCMakeLists);
  }
}

namespace {

bool IsLinkPhaseLibraryExtension(const std::string& fileExt)
{
  // Empty file extension is a special case for paths to framework's
  // internal binary which could be MyFw.framework/Versions/*/MyFw
  return (fileExt == ".framework"_s || fileExt == ".xcframework"_s ||
          fileExt == ".a"_s || fileExt == ".o"_s || fileExt == ".dylib"_s ||
          fileExt == ".tbd"_s || fileExt.empty());
}
bool IsLibraryType(const std::string& fileType)
{
  return (fileType == "wrapper.framework"_s ||
          fileType == "wrapper.xcframework"_s || fileType == "archive.ar"_s ||
          fileType == "compiled.mach-o.objfile"_s ||
          fileType == "compiled.mach-o.dylib"_s ||
          fileType == "compiled.mach-o.executable"_s ||
          fileType == "sourcecode.text-based-dylib-definition"_s);
}

std::string GetDirectoryValueFromFileExtension(const std::string& dirExt)
{
  std::string ext = cmSystemTools::LowerCase(dirExt);
  if (ext == "framework"_s) {
    return "wrapper.framework";
  }
  if (ext == "xcframework"_s) {
    return "wrapper.xcframework";
  }
  if (ext == "xcassets"_s) {
    return "folder.assetcatalog";
  }
  return "folder";
}

std::string GetSourcecodeValueFromFileExtension(
  const std::string& _ext, const std::string& lang,
  bool& keepLastKnownFileType, const std::vector<std::string>& enabled_langs)
{
  std::string ext = cmSystemTools::LowerCase(_ext);
  std::string sourcecode = "sourcecode";

  if (ext == "o"_s) {
    keepLastKnownFileType = true;
    sourcecode = "compiled.mach-o.objfile";
  } else if (ext == "xctest"_s) {
    sourcecode = "wrapper.cfbundle";
  } else if (ext == "xib"_s) {
    keepLastKnownFileType = true;
    sourcecode = "file.xib";
  } else if (ext == "storyboard"_s) {
    keepLastKnownFileType = true;
    sourcecode = "file.storyboard";
    // NOLINTNEXTLINE(bugprone-branch-clone)
  } else if (ext == "mm"_s && !cm::contains(enabled_langs, "OBJCXX")) {
    sourcecode += ".cpp.objcpp";
    // NOLINTNEXTLINE(bugprone-branch-clone)
  } else if (ext == "m"_s && !cm::contains(enabled_langs, "OBJC")) {
    sourcecode += ".c.objc";
  } else if (ext == "swift"_s) {
    sourcecode += ".swift";
  } else if (ext == "plist"_s) {
    sourcecode += ".text.plist";
  } else if (ext == "h"_s) {
    sourcecode += ".c.h";
  } else if (ext == "hxx"_s || ext == "hpp"_s || ext == "txx"_s ||
             ext == "pch"_s || ext == "hh"_s || ext == "inl"_s) {
    sourcecode += ".cpp.h";
  } else if (ext == "png"_s || ext == "gif"_s || ext == "jpg"_s) {
    keepLastKnownFileType = true;
    sourcecode = "image";
  } else if (ext == "txt"_s) {
    sourcecode += ".text";
  } else if (lang == "CXX"_s) {
    sourcecode += ".cpp.cpp";
  } else if (lang == "C"_s) {
    sourcecode += ".c.c";
  } else if (lang == "OBJCXX"_s) {
    sourcecode += ".cpp.objcpp";
  } else if (lang == "OBJC"_s) {
    sourcecode += ".c.objc";
  } else if (lang == "Fortran"_s) {
    sourcecode += ".fortran.f90";
  } else if (lang == "ASM"_s) {
    sourcecode += ".asm";
  } else if (ext == "metal"_s) {
    sourcecode += ".metal";
  } else if (ext == "mig"_s) {
    sourcecode += ".mig";
  } else if (ext == "tbd"_s) {
    sourcecode += ".text-based-dylib-definition";
  } else if (ext == "a"_s) {
    keepLastKnownFileType = true;
    sourcecode = "archive.ar";
  } else if (ext == "dylib"_s) {
    keepLastKnownFileType = true;
    sourcecode = "compiled.mach-o.dylib";
  } else if (ext == "framework"_s) {
    keepLastKnownFileType = true;
    sourcecode = "wrapper.framework";
  } else if (ext == "xcassets"_s) {
    keepLastKnownFileType = true;
    sourcecode = "folder.assetcatalog";
  } else if (ext == "xcconfig"_s) {
    keepLastKnownFileType = true;
    sourcecode = "text.xcconfig";
  }
  // else
  //  {
  //  // Already specialized above or we leave sourcecode == "sourcecode"
  //  // which is probably the most correct choice. Extensionless headers,
  //  // for example... Or file types unknown to Xcode that do not map to a
  //  // valid explicitFileType value.
  //  }

  return sourcecode;
}

template <class T>
std::string GetTargetObjectDirArch(T const& target,
                                   const std::string& defaultVal)
{
  cmList archs{ target.GetSafeProperty("OSX_ARCHITECTURES") };
  if (archs.size() > 1) {
    return "$(CURRENT_ARCH)";
  }
  if (archs.size() == 1) {
    return archs.front();
  }
  return defaultVal;
}

} // anonymous

// Extracts the framework directory, if path matches the framework syntax
// otherwise returns the path untouched
std::string cmGlobalXCodeGenerator::GetLibraryOrFrameworkPath(
  const std::string& path) const
{
  auto fwDescriptor = this->SplitFrameworkPath(path);
  if (fwDescriptor) {
    return fwDescriptor->GetFrameworkPath();
  }

  return path;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateXCodeFileReferenceFromPath(
  const std::string& fullpath, cmGeneratorTarget* target,
  const std::string& lang, cmSourceFile* sf)
{
  bool useLastKnownFileType = false;
  std::string fileType;
  if (sf) {
    if (cmValue e = sf->GetProperty("XCODE_EXPLICIT_FILE_TYPE")) {
      fileType = *e;
    } else if (cmValue l = sf->GetProperty("XCODE_LAST_KNOWN_FILE_TYPE")) {
      useLastKnownFileType = true;
      fileType = *l;
    }
  }
  // Make a copy so that we can override it later
  std::string path = cmSystemTools::CollapseFullPath(fullpath);
  // Compute the extension without leading '.'.
  std::string ext = cmSystemTools::GetFilenameLastExtension(path);
  if (!ext.empty()) {
    ext = ext.substr(1);
  }
  if (fileType.empty()) {
    path = this->GetLibraryOrFrameworkPath(path);
    ext = cmSystemTools::GetFilenameLastExtension(path);
    if (!ext.empty()) {
      ext = ext.substr(1);
    }
    // If fullpath references a directory, then we need to specify
    // lastKnownFileType as folder in order for Xcode to be able to
    // open the contents of the folder.
    // (Xcode 4.6 does not like explicitFileType=folder).
    if (cmSystemTools::FileIsDirectory(path)) {
      fileType = GetDirectoryValueFromFileExtension(ext);
      useLastKnownFileType = true;
    } else {
      if (ext.empty() && !sf) {
        // Special case for executable or library without extension
        // that is not a source file. We can't tell which without reading
        // its Mach-O header, but the file might not exist yet, so we
        // have to pick one here.
        useLastKnownFileType = true;
        fileType = "compiled.mach-o.executable";
      } else {
        fileType = GetSourcecodeValueFromFileExtension(
          ext, lang, useLastKnownFileType, this->EnabledLangs);
      }
    }
  }

  std::string key = GetGroupMapKeyFromPath(target, path);
  cmXCodeObject* fileRef = this->FileRefs[key];
  if (!fileRef) {
    fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
    fileRef->SetComment(path);
    this->FileRefs[key] = fileRef;
  }
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  fileRef->AddAttribute(useLastKnownFileType ? "lastKnownFileType"
                                             : "explicitFileType",
                        this->CreateString(fileType));
  // Store the file path relative to the top of the source tree.
  if (!IsLibraryType(fileType)) {
    path = this->RelativeToSource(path);
  }
  std::string name = cmSystemTools::GetFilenameName(path);
  const char* sourceTree =
    cmSystemTools::FileIsFullPath(path) ? "<absolute>" : "SOURCE_ROOT";
  fileRef->AddAttribute("name", this->CreateString(name));
  fileRef->AddAttribute("path", this->CreateString(path));
  fileRef->AddAttribute("sourceTree", this->CreateString(sourceTree));

  cmXCodeObject* group = this->GroupMap[key];
  if (!group) {
    if (IsLibraryType(fileType)) {
      group = this->FrameworkGroup;
    } else if (fileType == "folder") {
      group = this->ResourcesGroup;
    }
    if (group)
      this->GroupMap[key] = group;
  }

  if (!group) {
    cmSystemTools::Error(cmStrCat("Could not find a PBX group for ", key));
    return nullptr;
  }
  cmXCodeObject* children = group->GetAttribute("children");
  if (!children->HasObject(fileRef)) {
    children->AddObject(fileRef);
  }
  return fileRef;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateXCodeFileReference(
  cmSourceFile* sf, cmGeneratorTarget* target)
{
  std::string lang = this->CurrentLocalGenerator->GetSourceFileLanguage(*sf);

  return this->CreateXCodeFileReferenceFromPath(sf->ResolveFullPath(), target,
                                                lang, sf);
}

bool cmGlobalXCodeGenerator::SpecialTargetEmitted(std::string const& tname)
{
  if (tname == "ALL_BUILD"_s || tname == "install"_s || tname == "package"_s ||
      tname == "RUN_TESTS"_s || tname == CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
    if (this->TargetDoneSet.find(tname) != this->TargetDoneSet.end()) {
      return true;
    }
    this->TargetDoneSet.insert(tname);
    return false;
  }
  return false;
}

void cmGlobalXCodeGenerator::SetCurrentLocalGenerator(cmLocalGenerator* gen)
{
  this->CurrentLocalGenerator = gen;
  this->CurrentMakefile = gen->GetMakefile();

  // Select the current set of configuration types.
  this->CurrentConfigurationTypes =
    this->CurrentMakefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
}

struct cmSourceFilePathCompare
{
  bool operator()(cmSourceFile* l, cmSourceFile* r)
  {
    return l->ResolveFullPath() < r->ResolveFullPath();
  }
};

struct cmCompareTargets
{
  bool operator()(cmXCodeObject* l, cmXCodeObject* r) const
  {
    std::string const& a = l->GetTarget()->GetName();
    std::string const& b = r->GetTarget()->GetName();
    if (a == "ALL_BUILD"_s) {
      return true;
    }
    if (b == "ALL_BUILD"_s) {
      return false;
    }
    return a < b;
  }
};

bool cmGlobalXCodeGenerator::CreateXCodeTargets(
  cmLocalGenerator* gen, std::vector<cmXCodeObject*>& targets)
{
  this->SetCurrentLocalGenerator(gen);
  std::vector<cmGeneratorTarget*> gts =
    this->GetLocalGeneratorTargetsInOrder(gen);
  for (auto* gtgt : gts) {
    if (!this->CreateXCodeTarget(gtgt, targets)) {
      return false;
    }
  }
  std::sort(targets.begin(), targets.end(), cmCompareTargets());
  return true;
}

bool cmGlobalXCodeGenerator::CreateXCodeTarget(
  cmGeneratorTarget* gtgt, std::vector<cmXCodeObject*>& targets)
{
  std::string targetName = gtgt->GetName();

  // make sure ALL_BUILD, INSTALL, etc are only done once
  if (this->SpecialTargetEmitted(targetName)) {
    return true;
  }

  if (!gtgt->IsInBuildSystem()) {
    return true;
  }

  for (std::string const& configName : this->CurrentConfigurationTypes) {
    gtgt->CheckCxxModuleStatus(configName);
  }

  auto& gtgt_visited = this->CommandsVisited[gtgt];
  auto const& deps = this->GetTargetDirectDepends(gtgt);
  for (auto const& d : deps) {
    // Take the union of visited source files of custom commands so far.
    // ComputeTargetOrder ensures our dependencies already visited their
    // custom commands and updated CommandsVisited.
    auto& dep_visited = this->CommandsVisited[d];
    gtgt_visited.insert(dep_visited.begin(), dep_visited.end());
  }

  if (gtgt->GetType() == cmStateEnums::UTILITY ||
      gtgt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
      gtgt->GetType() == cmStateEnums::GLOBAL_TARGET) {
    cmXCodeObject* t = this->CreateUtilityTarget(gtgt);
    if (!t) {
      return false;
    }
    targets.push_back(t);
    return true;
  }

  // organize the sources
  std::vector<cmSourceFile*> commonSourceFiles;
  if (!gtgt->GetConfigCommonSourceFilesForXcode(commonSourceFiles)) {
    return false;
  }

  // Add CMakeLists.txt file for user convenience.
  this->AddXCodeProjBuildRule(gtgt, commonSourceFiles);

  // Add the Info.plist we are about to generate for an App Bundle.
  if (gtgt->GetPropertyAsBool("MACOSX_BUNDLE")) {
    std::string plist = this->ComputeInfoPListLocation(gtgt);
    cmSourceFile* sf = gtgt->Makefile->GetOrCreateSource(
      plist, true, cmSourceFileLocationKind::Known);
    commonSourceFiles.push_back(sf);
  }

  std::sort(commonSourceFiles.begin(), commonSourceFiles.end(),
            cmSourceFilePathCompare());

  gtgt->ComputeObjectMapping();

  std::vector<cmXCodeObject*> externalObjFiles;
  std::vector<cmXCodeObject*> headerFiles;
  std::vector<cmXCodeObject*> resourceFiles;
  std::vector<cmXCodeObject*> sourceFiles;
  for (auto* sourceFile : commonSourceFiles) {
    cmXCodeObject* xsf = this->CreateXCodeSourceFile(
      this->CurrentLocalGenerator, sourceFile, gtgt);
    cmXCodeObject* fr = xsf->GetAttribute("fileRef");
    cmXCodeObject* filetype =
      fr->GetObject()->GetAttribute("explicitFileType");
    if (!filetype) {
      filetype = fr->GetObject()->GetAttribute("lastKnownFileType");
    }

    cmGeneratorTarget::SourceFileFlags tsFlags =
      gtgt->GetTargetSourceFileFlags(sourceFile);

    if (filetype && filetype->GetString() == "compiled.mach-o.objfile"_s) {
      if (sourceFile->GetObjectLibrary().empty()) {
        externalObjFiles.push_back(xsf);
      }
    } else if (this->IsHeaderFile(sourceFile) ||
               (tsFlags.Type ==
                cmGeneratorTarget::SourceFileTypePrivateHeader) ||
               (tsFlags.Type ==
                cmGeneratorTarget::SourceFileTypePublicHeader)) {
      headerFiles.push_back(xsf);
    } else if (tsFlags.Type == cmGeneratorTarget::SourceFileTypeResource) {
      resourceFiles.push_back(xsf);
    } else if (!sourceFile->GetPropertyAsBool("HEADER_FILE_ONLY") &&
               !gtgt->IsSourceFilePartOfUnityBatch(
                 sourceFile->ResolveFullPath())) {
      // Include this file in the build if it has a known language
      // and has not been listed as an ignored extension for this
      // generator.
      if (!this->CurrentLocalGenerator->GetSourceFileLanguage(*sourceFile)
             .empty() &&
          !this->IgnoreFile(sourceFile->GetExtension().c_str())) {
        sourceFiles.push_back(xsf);
      }
    }
  }

  // some build phases only apply to bundles and/or frameworks
  bool isFrameworkTarget = gtgt->IsFrameworkOnApple();
  bool isBundleTarget = gtgt->GetPropertyAsBool("MACOSX_BUNDLE");
  bool isCFBundleTarget = gtgt->IsCFBundleOnApple();

  cmXCodeObject* buildFiles = nullptr;

  // create source build phase
  cmXCodeObject* sourceBuildPhase = nullptr;
  if (!sourceFiles.empty()) {
    sourceBuildPhase = this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
    sourceBuildPhase->SetComment("Sources");
    sourceBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (auto& sourceFile : sourceFiles) {
      buildFiles->AddObject(sourceFile);
    }
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
  }

  // create header build phase - only for framework targets
  cmXCodeObject* headerBuildPhase = nullptr;
  if (!headerFiles.empty() && isFrameworkTarget) {
    headerBuildPhase = this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->SetComment("Headers");
    headerBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (auto& headerFile : headerFiles) {
      buildFiles->AddObject(headerFile);
    }
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
  }

  // create resource build phase - only for framework or bundle targets
  cmXCodeObject* resourceBuildPhase = nullptr;
  if (!resourceFiles.empty() &&
      (isFrameworkTarget || isBundleTarget || isCFBundleTarget)) {
    resourceBuildPhase =
      this->CreateObject(cmXCodeObject::PBXResourcesBuildPhase);
    resourceBuildPhase->SetComment("Resources");
    resourceBuildPhase->AddAttribute("buildActionMask",
                                     this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (auto& resourceFile : resourceFiles) {
      buildFiles->AddObject(resourceFile);
    }
    resourceBuildPhase->AddAttribute("files", buildFiles);
    resourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                     this->CreateString("0"));
  }

  // create vector of "non-resource content file" build phases - only for
  // framework or bundle targets
  std::vector<cmXCodeObject*> contentBuildPhases;
  if (isFrameworkTarget || isBundleTarget || isCFBundleTarget) {
    using mapOfVectorOfSourceFiles =
      std::map<std::string, std::vector<cmSourceFile*>>;
    mapOfVectorOfSourceFiles bundleFiles;
    for (auto* sourceFile : commonSourceFiles) {
      cmGeneratorTarget::SourceFileFlags tsFlags =
        gtgt->GetTargetSourceFileFlags(sourceFile);
      if (tsFlags.Type == cmGeneratorTarget::SourceFileTypeMacContent) {
        bundleFiles[tsFlags.MacFolder].push_back(sourceFile);
      }
    }
    for (auto const& keySources : bundleFiles) {
      cmXCodeObject* copyFilesBuildPhase =
        this->CreateObject(cmXCodeObject::PBXCopyFilesBuildPhase);
      copyFilesBuildPhase->SetComment("Copy files");
      copyFilesBuildPhase->AddAttribute("buildActionMask",
                                        this->CreateString("2147483647"));
      copyFilesBuildPhase->AddAttribute("dstSubfolderSpec",
                                        this->CreateString("6"));
      std::ostringstream ostr;
      if (gtgt->IsFrameworkOnApple()) {
        // dstPath in frameworks is relative to Versions/<version>
        ostr << keySources.first;
      } else if (keySources.first != "MacOS"_s) {
        if (gtgt->Target->GetMakefile()->PlatformIsAppleEmbedded()) {
          ostr << keySources.first;
        } else {
          // dstPath in bundles is relative to Contents/MacOS
          ostr << "../" << keySources.first;
        }
      }
      copyFilesBuildPhase->AddAttribute("dstPath",
                                        this->CreateString(ostr.str()));
      copyFilesBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                        this->CreateString("0"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      copyFilesBuildPhase->AddAttribute("files", buildFiles);
      for (auto* sourceFile : keySources.second) {
        cmXCodeObject* xsf = this->CreateXCodeSourceFile(
          this->CurrentLocalGenerator, sourceFile, gtgt);
        buildFiles->AddObject(xsf);
      }
      contentBuildPhases.push_back(copyFilesBuildPhase);
    }
  }

  // create vector of "resource content file" build phases - only for
  // framework or bundle targets
  if (isFrameworkTarget || isBundleTarget || isCFBundleTarget) {
    using mapOfVectorOfSourceFiles =
      std::map<std::string, std::vector<cmSourceFile*>>;
    mapOfVectorOfSourceFiles bundleFiles;
    for (auto* sourceFile : commonSourceFiles) {
      cmGeneratorTarget::SourceFileFlags tsFlags =
        gtgt->GetTargetSourceFileFlags(sourceFile);
      if (tsFlags.Type == cmGeneratorTarget::SourceFileTypeDeepResource) {
        bundleFiles[tsFlags.MacFolder].push_back(sourceFile);
      }
    }
    for (auto const& keySources : bundleFiles) {
      cmXCodeObject* copyFilesBuildPhase =
        this->CreateObject(cmXCodeObject::PBXCopyFilesBuildPhase);
      copyFilesBuildPhase->SetComment("Copy files");
      copyFilesBuildPhase->AddAttribute("buildActionMask",
                                        this->CreateString("2147483647"));
      copyFilesBuildPhase->AddAttribute("dstSubfolderSpec",
                                        this->CreateString("7"));
      copyFilesBuildPhase->AddAttribute("dstPath",
                                        this->CreateString(keySources.first));
      copyFilesBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                        this->CreateString("0"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      copyFilesBuildPhase->AddAttribute("files", buildFiles);
      for (auto* sourceFile : keySources.second) {
        cmXCodeObject* xsf = this->CreateXCodeSourceFile(
          this->CurrentLocalGenerator, sourceFile, gtgt);
        buildFiles->AddObject(xsf);
      }
      contentBuildPhases.push_back(copyFilesBuildPhase);
    }
  }

  // Always create Link Binary With Libraries build phase
  cmXCodeObject* frameworkBuildPhase = nullptr;
  frameworkBuildPhase =
    this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
  frameworkBuildPhase->SetComment("Frameworks");
  frameworkBuildPhase->AddAttribute("buildActionMask",
                                    this->CreateString("2147483647"));
  buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  frameworkBuildPhase->AddAttribute("files", buildFiles);
  // Add all collected .o files to this build phase
  for (auto& externalObjFile : externalObjFiles) {
    buildFiles->AddObject(externalObjFile);
  }
  frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                    this->CreateString("0"));

  // create list of build phases and create the Xcode target
  cmXCodeObject* buildPhases = this->CreateObject(cmXCodeObject::OBJECT_LIST);

  this->CreateCustomCommands(buildPhases, sourceBuildPhase, headerBuildPhase,
                             resourceBuildPhase, contentBuildPhases,
                             frameworkBuildPhase, gtgt);

  targets.push_back(this->CreateXCodeTarget(gtgt, buildPhases));
  return true;
}

void cmGlobalXCodeGenerator::ForceLinkerLanguages()
{
  for (const auto& localGenerator : this->LocalGenerators) {
    // All targets depend on the build-system check target.
    for (const auto& tgt : localGenerator->GetGeneratorTargets()) {
      // This makes sure all targets link using the proper language.
      this->ForceLinkerLanguage(tgt.get());
    }
  }
}

void cmGlobalXCodeGenerator::ForceLinkerLanguage(cmGeneratorTarget* gtgt)
{
  // This matters only for targets that link.
  if (gtgt->GetType() != cmStateEnums::EXECUTABLE &&
      gtgt->GetType() != cmStateEnums::SHARED_LIBRARY &&
      gtgt->GetType() != cmStateEnums::MODULE_LIBRARY) {
    return;
  }

  std::string llang = gtgt->GetLinkerLanguage("NOCONFIG");
  if (llang.empty()) {
    return;
  }

  // If the language is compiled as a source trust Xcode to link with it.
  for (auto const& Language :
       gtgt
         ->GetLinkImplementation("NOCONFIG",
                                 cmGeneratorTarget::LinkInterfaceFor::Link)
         ->Languages) {
    if (Language == llang) {
      return;
    }
  }

  // Allow empty source file list for iOS Sticker packs
  if (const char* productType = GetTargetProductType(gtgt)) {
    if (strcmp(productType,
               "com.apple.product-type.app-extension.messages-sticker-pack") ==
        0) {
      return;
    }
  }

  // Add an empty source file to the target that compiles with the
  // linker language.  This should convince Xcode to choose the proper
  // language.
  cmMakefile* mf = gtgt->Target->GetMakefile();
  std::string fname = cmStrCat(
    gtgt->GetLocalGenerator()->GetCurrentBinaryDirectory(), "/CMakeFiles/",
    gtgt->GetName(), "-CMakeForceLinker.", cmSystemTools::LowerCase(llang));
  {
    cmGeneratedFileStream fout(fname);
    fout << '\n';
  }
  if (cmSourceFile* sf = mf->GetOrCreateSource(fname)) {
    sf->SetProperty("LANGUAGE", llang);
    sf->SetProperty("CXX_SCAN_FOR_MODULES", "0");
    gtgt->AddSource(fname);
  }
}

bool cmGlobalXCodeGenerator::IsHeaderFile(cmSourceFile* sf)
{
  return cm::contains(this->CMakeInstance->GetHeaderExtensions(),
                      sf->GetExtension());
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateLegacyRunScriptBuildPhase(
  const char* name, const char* name2, cmGeneratorTarget* target,
  const std::vector<cmCustomCommand>& commands)
{
  if (commands.empty() && strcmp(name, "CMake ReRun") != 0) {
    return nullptr;
  }
  cmXCodeObject* buildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  buildPhase->AddAttribute("name", this->CreateString(name));
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
  this->AddCommandsToBuildPhase(buildPhase, target, commands, name2);
  return buildPhase;
}

void cmGlobalXCodeGenerator::CreateCustomCommands(
  cmXCodeObject* buildPhases, cmXCodeObject* sourceBuildPhase,
  cmXCodeObject* headerBuildPhase, cmXCodeObject* resourceBuildPhase,
  std::vector<cmXCodeObject*> const& contentBuildPhases,
  cmXCodeObject* frameworkBuildPhase, cmGeneratorTarget* gtgt)
{
  std::vector<cmCustomCommand> const& prebuild = gtgt->GetPreBuildCommands();
  std::vector<cmCustomCommand> const& prelink = gtgt->GetPreLinkCommands();
  std::vector<cmCustomCommand> postbuild = gtgt->GetPostBuildCommands();

  if (gtgt->GetType() == cmStateEnums::SHARED_LIBRARY &&
      !gtgt->IsFrameworkOnApple()) {
    std::string str_file = cmStrCat("$<TARGET_FILE:", gtgt->GetName(), '>');
    std::string str_so_file =
      cmStrCat("$<TARGET_SONAME_FILE:", gtgt->GetName(), '>');
    std::string str_link_file =
      cmStrCat("$<TARGET_LINKER_LIBRARY_FILE:", gtgt->GetName(), '>');
    cmCustomCommandLines cmd = cmMakeSingleCommandLine(
      { cmSystemTools::GetCMakeCommand(), "-E", "cmake_symlink_library",
        str_file, str_so_file, str_link_file });

    cmCustomCommand command;
    command.SetCommandLines(cmd);
    command.SetComment("Creating symlinks");
    command.SetWorkingDirectory("");
    command.SetBacktrace(this->CurrentMakefile->GetBacktrace());
    command.SetStdPipesUTF8(true);

    postbuild.push_back(std::move(command));
  }

  if (gtgt->HasImportLibrary("") && !gtgt->IsFrameworkOnApple()) {
    // create symbolic links for .tbd file
    std::string file = cmStrCat("$<TARGET_IMPORT_FILE:", gtgt->GetName(), '>');
    std::string soFile =
      cmStrCat("$<TARGET_SONAME_IMPORT_FILE:", gtgt->GetName(), '>');
    std::string linkFile =
      cmStrCat("$<TARGET_LINKER_IMPORT_FILE:", gtgt->GetName(), '>');
    cmCustomCommandLines symlink_command = cmMakeSingleCommandLine(
      { cmSystemTools::GetCMakeCommand(), "-E", "cmake_symlink_library", file,
        soFile, linkFile });

    cmCustomCommand command;
    command.SetCommandLines(symlink_command);
    command.SetComment("Creating import symlinks");
    command.SetWorkingDirectory("");
    command.SetBacktrace(this->CurrentMakefile->GetBacktrace());
    command.SetStdPipesUTF8(true);

    postbuild.push_back(std::move(command));
  }

  cmXCodeObject* legacyCustomCommandsBuildPhase = nullptr;
  cmXCodeObject* preBuildPhase = nullptr;
  cmXCodeObject* preLinkPhase = nullptr;
  cmXCodeObject* postBuildPhase = nullptr;

  if (this->XcodeBuildSystem >= BuildSystem::Twelve) {
    // create prebuild phase
    preBuildPhase =
      this->CreateRunScriptBuildPhase("CMake PreBuild Rules", gtgt, prebuild);
    // create prelink phase
    preLinkPhase =
      this->CreateRunScriptBuildPhase("CMake PreLink Rules", gtgt, prelink);
    // create postbuild phase
    postBuildPhase = this->CreateRunScriptBuildPhase("CMake PostBuild Rules",
                                                     gtgt, postbuild);
  } else {
    std::vector<cmSourceFile*> classes;
    if (!gtgt->GetConfigCommonSourceFilesForXcode(classes)) {
      return;
    }
    // add all the sources
    std::vector<cmCustomCommand> commands;
    auto& visited = this->CommandsVisited[gtgt];
    for (auto* sourceFile : classes) {
      if (sourceFile->GetCustomCommand() &&
          visited.insert(sourceFile).second) {
        commands.push_back(*sourceFile->GetCustomCommand());
      }
    }
    // create custom commands phase
    legacyCustomCommandsBuildPhase = this->CreateLegacyRunScriptBuildPhase(
      "CMake Rules", "cmakeRulesBuildPhase", gtgt, commands);
    // create prebuild phase
    preBuildPhase = this->CreateLegacyRunScriptBuildPhase(
      "CMake PreBuild Rules", "preBuildCommands", gtgt, prebuild);
    // create prelink phase
    preLinkPhase = this->CreateLegacyRunScriptBuildPhase(
      "CMake PreLink Rules", "preLinkCommands", gtgt, prelink);
    // create postbuild phase
    postBuildPhase = this->CreateLegacyRunScriptBuildPhase(
      "CMake PostBuild Rules", "postBuildPhase", gtgt, postbuild);
  }

  // The order here is the order they will be built in.
  // The order "headers, resources, sources" mimics a native project generated
  // from an xcode template...
  //
  if (preBuildPhase) {
    buildPhases->AddObject(preBuildPhase);
  }
  if (legacyCustomCommandsBuildPhase) {
    buildPhases->AddObject(legacyCustomCommandsBuildPhase);
  }
  if (this->XcodeBuildSystem >= BuildSystem::Twelve) {
    this->CreateRunScriptBuildPhases(buildPhases, gtgt);
  }
  if (headerBuildPhase) {
    buildPhases->AddObject(headerBuildPhase);
  }
  if (resourceBuildPhase) {
    buildPhases->AddObject(resourceBuildPhase);
  }
  for (auto* obj : contentBuildPhases) {
    buildPhases->AddObject(obj);
  }
  if (sourceBuildPhase) {
    buildPhases->AddObject(sourceBuildPhase);
  }
  if (preLinkPhase) {
    buildPhases->AddObject(preLinkPhase);
  }
  if (frameworkBuildPhase) {
    buildPhases->AddObject(frameworkBuildPhase);
  }

  // When this build phase is present, it must be last. More build phases may
  // be added later for embedding things and they will insert themselves just
  // before this last build phase.
  if (postBuildPhase) {
    buildPhases->AddObject(postBuildPhase);
  }
}

void cmGlobalXCodeGenerator::CreateRunScriptBuildPhases(
  cmXCodeObject* buildPhases, cmGeneratorTarget const* gt)
{
  std::vector<cmSourceFile*> sources;
  if (!gt->GetConfigCommonSourceFilesForXcode(sources)) {
    return;
  }
  auto& visited = this->CommandsVisited[gt];
  for (auto* sf : sources) {
    this->CreateRunScriptBuildPhases(buildPhases, sf, gt, visited);
  }
}

void cmGlobalXCodeGenerator::CreateRunScriptBuildPhases(
  cmXCodeObject* buildPhases, cmSourceFile const* sf,
  cmGeneratorTarget const* gt, std::set<cmSourceFile const*>& visited)
{
  cmCustomCommand const* cc = sf->GetCustomCommand();
  if (cc && visited.insert(sf).second) {
    this->CustomCommandRoots[sf].insert(gt);
    if (std::vector<cmSourceFile*> const* depends = gt->GetSourceDepends(sf)) {
      for (cmSourceFile const* di : *depends) {
        this->CreateRunScriptBuildPhases(buildPhases, di, gt, visited);
      }
    }
    cmXCodeObject* buildPhase = this->CreateRunScriptBuildPhase(sf, gt, *cc);
    buildPhases->AddObject(buildPhase);
  }
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateRunScriptBuildPhase(
  cmSourceFile const* sf, cmGeneratorTarget const* gt,
  cmCustomCommand const& cc)
{
  std::set<std::string> allConfigInputs;
  std::set<std::string> allConfigOutputs;

  cmXCodeObject* buildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase,
                       cmStrCat(gt->GetName(), ':', sf->GetFullPath()));

  auto depfilesDirectory = cmStrCat(
    gt->GetLocalGenerator()->GetCurrentBinaryDirectory(), "/CMakeFiles/d/");
  auto depfilesPrefix = cmStrCat(depfilesDirectory, buildPhase->GetId(), '.');

  std::string shellScript = "set -e\n";
  for (std::string const& configName : this->CurrentConfigurationTypes) {
    cmCustomCommandGenerator ccg(
      cc, configName, this->CurrentLocalGenerator, true, {},
      [&depfilesPrefix](const std::string& config, const std::string&)
        -> std::string { return cmStrCat(depfilesPrefix, config, ".d"); });
    std::vector<std::string> realDepends;
    realDepends.reserve(ccg.GetDepends().size());
    for (auto const& d : ccg.GetDepends()) {
      std::string dep;
      if (this->CurrentLocalGenerator->GetRealDependency(d, configName, dep)) {
        realDepends.emplace_back(std::move(dep));
      }
    }

    allConfigInputs.insert(realDepends.begin(), realDepends.end());
    allConfigOutputs.insert(ccg.GetByproducts().begin(),
                            ccg.GetByproducts().end());
    allConfigOutputs.insert(ccg.GetOutputs().begin(), ccg.GetOutputs().end());

    shellScript =
      cmStrCat(shellScript, R"(if test "$CONFIGURATION" = ")", configName,
               "\"; then :\n", this->ConstructScript(ccg), "fi\n");
  }

  if (!cc.GetDepfile().empty()) {
    buildPhase->AddAttribute(
      "dependencyFile",
      this->CreateString(cmStrCat(depfilesDirectory, buildPhase->GetId(),
                                  ".$(CONFIGURATION).d")));
    // to avoid spurious errors during first build,  create empty dependency
    // files
    cmSystemTools::MakeDirectory(depfilesDirectory);
    for (std::string const& configName : this->CurrentConfigurationTypes) {
      auto file = cmStrCat(depfilesPrefix, configName, ".d");
      if (!cmSystemTools::FileExists(file)) {
        cmSystemTools::Touch(file, true);
      }
    }
  }

  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  {
    std::string name;
    if (!allConfigOutputs.empty()) {
      name = cmStrCat("Generate ",
                      this->RelativeToBinary(*allConfigOutputs.begin()));
    } else {
      name = sf->GetLocation().GetName();
    }
    buildPhase->AddAttribute("name", this->CreateString(name));
  }
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
  buildPhase->AddAttribute("shellScript", this->CreateString(shellScript));
  buildPhase->AddAttribute("showEnvVarsInLog", this->CreateString("0"));

  bool symbolic = false;
  {
    cmXCodeObject* inputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (std::string const& i : allConfigInputs) {
      inputPaths->AddUniqueObject(this->CreateString(i));
      if (!symbolic) {
        if (cmSourceFile* isf =
              gt->GetLocalGenerator()->GetMakefile()->GetSource(
                i, cmSourceFileLocationKind::Known)) {
          symbolic = isf->GetPropertyAsBool("SYMBOLIC");
        }
      }
    }
    buildPhase->AddAttribute("inputPaths", inputPaths);
  }
  {
    cmXCodeObject* outputPaths =
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (std::string const& o : allConfigOutputs) {
      outputPaths->AddUniqueObject(this->CreateString(o));
      if (!symbolic) {
        if (cmSourceFile* osf =
              gt->GetLocalGenerator()->GetMakefile()->GetSource(
                o, cmSourceFileLocationKind::Known)) {
          symbolic = osf->GetPropertyAsBool("SYMBOLIC");
        }
      }
    }
    buildPhase->AddAttribute("outputPaths", outputPaths);
  }
  if (symbolic) {
    buildPhase->AddAttribute("alwaysOutOfDate", this->CreateString("1"));
  }

  return buildPhase;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateRunScriptBuildPhase(
  std::string const& name, cmGeneratorTarget const* gt,
  std::vector<cmCustomCommand> const& commands)
{
  if (commands.empty()) {
    return nullptr;
  }

  std::set<std::string> allConfigOutputs;

  std::string shellScript = "set -e\n";
  for (std::string const& configName : this->CurrentConfigurationTypes) {
    shellScript = cmStrCat(shellScript, R"(if test "$CONFIGURATION" = ")",
                           configName, "\"; then :\n");
    for (cmCustomCommand const& cc : commands) {
      cmCustomCommandGenerator ccg(cc, configName,
                                   this->CurrentLocalGenerator);
      shellScript = cmStrCat(shellScript, this->ConstructScript(ccg));
      allConfigOutputs.insert(ccg.GetByproducts().begin(),
                              ccg.GetByproducts().end());
    }
    shellScript = cmStrCat(shellScript, "fi\n");
  }

  cmXCodeObject* buildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase,
                       cmStrCat(gt->GetName(), ':', name));
  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  buildPhase->AddAttribute("name", this->CreateString(name));
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
  buildPhase->AddAttribute("shellScript", this->CreateString(shellScript));
  buildPhase->AddAttribute("showEnvVarsInLog", this->CreateString("0"));
  {
    cmXCodeObject* outputPaths =
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for (std::string const& o : allConfigOutputs) {
      outputPaths->AddUniqueObject(this->CreateString(o));
    }
    buildPhase->AddAttribute("outputPaths", outputPaths);
  }
  buildPhase->AddAttribute("alwaysOutOfDate", this->CreateString("1"));

  return buildPhase;
}

namespace {
void ReplaceScriptVars(std::string& cmd)
{
  cmSystemTools::ReplaceString(cmd, "$(CONFIGURATION)", "$CONFIGURATION");
  cmSystemTools::ReplaceString(cmd, "$(EFFECTIVE_PLATFORM_NAME)",
                               "$EFFECTIVE_PLATFORM_NAME");
}
}

std::string cmGlobalXCodeGenerator::ConstructScript(
  cmCustomCommandGenerator const& ccg)
{
  std::string script;
  cmLocalGenerator* lg = this->CurrentLocalGenerator;
  std::string wd = ccg.GetWorkingDirectory();
  if (wd.empty()) {
    wd = lg->GetCurrentBinaryDirectory();
  }
  wd = lg->ConvertToOutputFormat(wd, cmOutputConverter::SHELL);
  ReplaceScriptVars(wd);
  script = cmStrCat(script, "  cd ", wd, '\n');
  for (unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c) {
    std::string cmd = ccg.GetCommand(c);
    if (cmd.empty()) {
      continue;
    }
    cmSystemTools::ReplaceString(cmd, "/./", "/");
    cmd = lg->ConvertToOutputFormat(cmd, cmOutputConverter::SHELL);
    ccg.AppendArguments(c, cmd);
    ReplaceScriptVars(cmd);
    script = cmStrCat(script, "  ", cmd, '\n');
  }
  return script;
}

// This function removes each occurrence of the flag and returns the last one
// (i.e., the dominant flag in GCC)
std::string cmGlobalXCodeGenerator::ExtractFlag(const char* flag,
                                                std::string& flags)
{
  std::string retFlag;
  std::string::size_type lastOccurancePos = flags.rfind(flag);
  bool saved = false;
  while (lastOccurancePos != std::string::npos) {
    // increment pos, we use lastOccurancePos to reduce search space on next
    // inc
    std::string::size_type pos = lastOccurancePos;
    if (pos == 0 || flags[pos - 1] == ' ') {
      while (pos < flags.size() && flags[pos] != ' ') {
        if (!saved) {
          retFlag += flags[pos];
        }
        flags[pos] = ' ';
        pos++;
      }
      saved = true;
    }
    // decrement lastOccurancePos while making sure we don't loop around
    // and become a very large positive number since size_type is unsigned
    lastOccurancePos = lastOccurancePos == 0 ? 0 : lastOccurancePos - 1;
    lastOccurancePos = flags.rfind(flag, lastOccurancePos);
  }
  return retFlag;
}

// This function removes each matching occurrence of the expression and
// returns the last one (i.e., the dominant flag in GCC)
std::string cmGlobalXCodeGenerator::ExtractFlagRegex(const char* exp,
                                                     int matchIndex,
                                                     std::string& flags)
{
  std::string retFlag;

  cmsys::RegularExpression regex(exp);
  assert(regex.is_valid());
  if (!regex.is_valid()) {
    return retFlag;
  }

  std::string::size_type offset = 0;

  while (regex.find(&flags[offset])) {
    const std::string::size_type startPos = offset + regex.start(matchIndex);
    const std::string::size_type endPos = offset + regex.end(matchIndex);
    const std::string::size_type size = endPos - startPos;

    offset = startPos + 1;

    retFlag.assign(flags, startPos, size);
    flags.replace(startPos, size, size, ' ');
  }

  return retFlag;
}

//----------------------------------------------------------------------------
// This function strips off Xcode attributes that do not target the current
// configuration
void cmGlobalXCodeGenerator::FilterConfigurationAttribute(
  std::string const& configName, std::string& attribute)
{
  // Handle [variant=<config>] condition explicitly here.
  std::string::size_type beginVariant = attribute.find("[variant=");
  if (beginVariant == std::string::npos) {
    // There is no variant in this attribute.
    return;
  }

  std::string::size_type endVariant = attribute.find(']', beginVariant + 9);
  if (endVariant == std::string::npos) {
    // There is no terminating bracket.
    return;
  }

  // Compare the variant to the configuration.
  std::string variant =
    attribute.substr(beginVariant + 9, endVariant - beginVariant - 9);
  if (variant == configName) {
    // The variant matches the configuration so use this
    // attribute but drop the [variant=<config>] condition.
    attribute.erase(beginVariant, endVariant - beginVariant + 1);
  } else {
    // The variant does not match the configuration so
    // do not use this attribute.
    attribute.clear();
  }
}

void cmGlobalXCodeGenerator::AddCommandsToBuildPhase(
  cmXCodeObject* buildphase, cmGeneratorTarget* target,
  std::vector<cmCustomCommand> const& commands, const char* name)
{
  std::string dir = cmStrCat(
    this->CurrentLocalGenerator->GetCurrentBinaryDirectory(), "/CMakeScripts");
  cmSystemTools::MakeDirectory(dir);
  std::string makefile =
    cmStrCat(dir, '/', target->GetName(), '_', name, ".make");

  for (const auto& currentConfig : this->CurrentConfigurationTypes) {
    this->CreateCustomRulesMakefile(makefile.c_str(), target, commands,
                                    currentConfig);
  }

  std::string cdir = this->CurrentLocalGenerator->GetCurrentBinaryDirectory();
  cdir = this->ConvertToRelativeForMake(cdir);
  std::string makecmd = cmStrCat(
    "make -C ", cdir, " -f ",
    this->ConvertToRelativeForMake(cmStrCat(makefile, "$CONFIGURATION")),
    " OBJDIR=$(basename \"$OBJECT_FILE_DIR_normal\") all");
  buildphase->AddAttribute("shellScript", this->CreateString(makecmd));
  buildphase->AddAttribute("showEnvVarsInLog", this->CreateString("0"));
}

void cmGlobalXCodeGenerator::CreateCustomRulesMakefile(
  const char* makefileBasename, cmGeneratorTarget* target,
  std::vector<cmCustomCommand> const& commands, const std::string& configName)
{
  std::string makefileName = cmStrCat(makefileBasename, configName);
  cmGeneratedFileStream makefileStream(makefileName);
  if (!makefileStream) {
    return;
  }
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n"
                    "# Custom rules for "
                 << target->GetName() << '\n';

  // disable the implicit rules
  makefileStream << ".SUFFIXES: "
                    "\n";

  // have all depend on all outputs
  makefileStream << "all: ";
  std::map<const cmCustomCommand*, std::string> tname;
  int count = 0;
  for (auto const& command : commands) {
    cmCustomCommandGenerator ccg(command, configName,
                                 this->CurrentLocalGenerator);
    if (ccg.GetNumberOfCommands() > 0) {
      const std::vector<std::string>& outputs = ccg.GetOutputs();
      if (!outputs.empty()) {
        for (auto const& output : outputs) {
          makefileStream << "\\\n\t" << this->ConvertToRelativeForMake(output);
        }
      } else {
        std::ostringstream str;
        str << "_buildpart_" << count++;
        tname[&ccg.GetCC()] = cmStrCat(target->GetName(), str.str());
        makefileStream << "\\\n\t" << tname[&ccg.GetCC()];
      }
    }
  }
  makefileStream << "\n\n";

  auto depfilesDirectory =
    cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(),
             "/CMakeFiles/d/");

  for (auto const& command : commands) {
    cmCustomCommandGenerator ccg(
      command, configName, this->CurrentLocalGenerator, true, {},
      [this, &depfilesDirectory](const std::string& config,
                                 const std::string& file) -> std::string {
        return cmStrCat(
          depfilesDirectory,
          this->GetObjectId(cmXCodeObject::PBXShellScriptBuildPhase, file),
          '.', config, ".d");
      });

    auto depfile = ccg.GetInternalDepfile();
    if (!depfile.empty()) {
      makefileStream << "include "
                     << cmSystemTools::ConvertToOutputPath(depfile) << "\n\n";

      cmSystemTools::MakeDirectory(depfilesDirectory);
      if (!cmSystemTools::FileExists(depfile)) {
        cmSystemTools::Touch(depfile, true);
      }
    }

    std::vector<std::string> realDepends;
    realDepends.reserve(ccg.GetDepends().size());
    for (auto const& d : ccg.GetDepends()) {
      std::string dep;
      if (this->CurrentLocalGenerator->GetRealDependency(d, configName, dep)) {
        realDepends.emplace_back(std::move(dep));
      }
    }

    if (ccg.GetNumberOfCommands() > 0) {
      makefileStream << '\n';
      const std::vector<std::string>& outputs = ccg.GetOutputs();
      if (!outputs.empty()) {
        // There is at least one output, start the rule for it
        const char* sep = "";
        for (auto const& output : outputs) {
          makefileStream << sep << this->ConvertToRelativeForMake(output);
          sep = " ";
        }
        makefileStream << ": ";
      } else {
        // There are no outputs.  Use the generated force rule name.
        makefileStream << tname[&ccg.GetCC()] << ": ";
      }
      for (auto const& dep : realDepends) {
        makefileStream << "\\\n" << this->ConvertToRelativeForMake(dep);
      }
      makefileStream << '\n';

      if (cm::optional<std::string> comment = ccg.GetComment()) {
        std::string echo_cmd =
          cmStrCat("echo ",
                   (this->CurrentLocalGenerator->EscapeForShell(
                     *comment, ccg.GetCC().GetEscapeAllowMakeVars())));
        makefileStream << '\t' << echo_cmd << '\n';
      }

      // Add each command line to the set of commands.
      for (unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c) {
        // Build the command line in a single string.
        std::string cmd2 = ccg.GetCommand(c);
        cmSystemTools::ReplaceString(cmd2, "/./", "/");
        cmd2 = this->ConvertToRelativeForMake(cmd2);
        std::string cmd;
        std::string wd = ccg.GetWorkingDirectory();
        if (!wd.empty()) {
          cmd += "cd ";
          cmd += this->ConvertToRelativeForMake(wd);
          cmd += " && ";
        }
        cmd += cmd2;
        ccg.AppendArguments(c, cmd);
        makefileStream << '\t' << cmd << '\n';
      }

      // Symbolic inputs are not expected to exist, so add dummy rules.
      for (auto const& dep : realDepends) {
        if (cmSourceFile* dsf =
              target->GetLocalGenerator()->GetMakefile()->GetSource(
                dep, cmSourceFileLocationKind::Known)) {
          if (dsf->GetPropertyAsBool("SYMBOLIC")) {
            makefileStream << this->ConvertToRelativeForMake(dep) << ":\n";
          }
        }
      }
    }
  }
}

void cmGlobalXCodeGenerator::AddPositionIndependentLinkAttribute(
  cmGeneratorTarget* target, cmXCodeObject* buildSettings,
  const std::string& configName)
{
  // For now, only EXECUTABLE is concerned
  if (target->GetType() != cmStateEnums::EXECUTABLE) {
    return;
  }

  const char* PICValue = target->GetLinkPIEProperty(configName);
  if (PICValue == nullptr) {
    // POSITION_INDEPENDENT_CODE is not set
    return;
  }

  buildSettings->AddAttribute(
    "LD_NO_PIE", this->CreateString(cmIsOn(PICValue) ? "NO" : "YES"));
}

void cmGlobalXCodeGenerator::CreateBuildSettings(cmGeneratorTarget* gtgt,
                                                 cmXCodeObject* buildSettings,
                                                 const std::string& configName)
{
  if (!gtgt->IsInBuildSystem()) {
    return;
  }

  std::string defFlags;
  bool shared = ((gtgt->GetType() == cmStateEnums::SHARED_LIBRARY) ||
                 (gtgt->GetType() == cmStateEnums::MODULE_LIBRARY));
  bool binary = ((gtgt->GetType() == cmStateEnums::OBJECT_LIBRARY) ||
                 (gtgt->GetType() == cmStateEnums::STATIC_LIBRARY) ||
                 (gtgt->GetType() == cmStateEnums::EXECUTABLE) || shared);

  // Compute the compilation flags for each language.
  std::set<std::string> languages;
  gtgt->GetLanguages(languages, configName);
  std::map<std::string, std::string> cflags;
  for (auto const& lang : languages) {
    std::string& flags = cflags[lang];

    // Add language-specific flags.
    this->CurrentLocalGenerator->AddLanguageFlags(
      flags, gtgt, cmBuildStep::Compile, lang, configName);

    if (gtgt->IsIPOEnabled(lang, configName)) {
      this->CurrentLocalGenerator->AppendFeatureOptions(flags, lang, "IPO");
    }

    // Add shared-library flags if needed.
    this->CurrentLocalGenerator->AddCMP0018Flags(flags, gtgt, lang,
                                                 configName);

    this->CurrentLocalGenerator->AddVisibilityPresetFlags(flags, gtgt, lang);

    this->CurrentLocalGenerator->AddCompileOptions(flags, gtgt, lang,
                                                   configName);
  }

  std::string llang = gtgt->GetLinkerLanguage(configName);
  if (binary && llang.empty()) {
    cmSystemTools::Error(
      cmStrCat("CMake can not determine linker language for target: ",
               gtgt->GetName()));
    return;
  }

  // Choose a language to use for target-wide preprocessor definitions.
  static const char* ppLangs[] = { "CXX", "C", "OBJCXX", "OBJC" };
  std::string langForPreprocessorDefinitions;
  if (cm::contains(ppLangs, llang)) {
    langForPreprocessorDefinitions = llang;
  } else {
    for (const char* l : ppLangs) {
      if (languages.count(l)) {
        langForPreprocessorDefinitions = l;
        break;
      }
    }
  }

  if (gtgt->IsIPOEnabled(llang, configName)) {
    const char* ltoValue =
      this->CurrentMakefile->IsOn("_CMAKE_LTO_THIN") ? "YES_THIN" : "YES";
    buildSettings->AddAttribute("LLVM_LTO", this->CreateString(ltoValue));
  }

  // Handle PIE linker configuration
  this->AddPositionIndependentLinkAttribute(gtgt, buildSettings, configName);

  // Add define flags
  this->CurrentLocalGenerator->AppendFlags(
    defFlags, this->CurrentMakefile->GetDefineFlags());

  // Add preprocessor definitions for this target and configuration.
  BuildObjectListOrString ppDefs(this, true);
  this->AppendDefines(
    ppDefs, "CMAKE_INTDIR=\"$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)\"");
  if (const std::string* exportMacro = gtgt->GetExportMacro()) {
    // Add the export symbol definition for shared library objects.
    this->AppendDefines(ppDefs, exportMacro->c_str());
  }
  std::vector<std::string> targetDefines;
  if (!langForPreprocessorDefinitions.empty()) {
    gtgt->GetCompileDefinitions(targetDefines, configName,
                                langForPreprocessorDefinitions);
  }
  this->AppendDefines(ppDefs, targetDefines);
  buildSettings->AddAttribute("GCC_PREPROCESSOR_DEFINITIONS",
                              ppDefs.CreateList());
  if (languages.count("Swift")) {
    // Swift uses a separate attribute for definitions.
    std::vector<std::string> targetSwiftDefines;
    gtgt->GetCompileDefinitions(targetSwiftDefines, configName, "Swift");
    // Remove the '=value' parts, as Swift does not support them.
    std::for_each(targetSwiftDefines.begin(), targetSwiftDefines.end(),
                  [](std::string& def) {
                    std::string::size_type pos = def.find('=');
                    if (pos != std::string::npos) {
                      def.erase(pos);
                    }
                  });
    if (this->XcodeVersion < 80) {
      std::string defineString;
      std::set<std::string> defines(targetSwiftDefines.begin(),
                                    targetSwiftDefines.end());
      this->CurrentLocalGenerator->JoinDefines(defines, defineString, "Swift");
      cflags["Swift"] += cmStrCat(' ', defineString);
    } else {
      BuildObjectListOrString swiftDefs(this, true);
      this->AppendDefines(swiftDefs, targetSwiftDefines);
      buildSettings->AddAttribute("SWIFT_ACTIVE_COMPILATION_CONDITIONS",
                                  swiftDefs.CreateList());
    }
  }

  std::string extraLinkOptionsVar;
  std::string extraLinkOptions;
  if (gtgt->GetType() == cmStateEnums::EXECUTABLE) {
    extraLinkOptionsVar = "CMAKE_EXE_LINKER_FLAGS";
  } else if (gtgt->GetType() == cmStateEnums::SHARED_LIBRARY) {
    extraLinkOptionsVar = "CMAKE_SHARED_LINKER_FLAGS";
  } else if (gtgt->GetType() == cmStateEnums::MODULE_LIBRARY) {
    extraLinkOptionsVar = "CMAKE_MODULE_LINKER_FLAGS";
  }
  if (!extraLinkOptionsVar.empty()) {
    this->CurrentLocalGenerator->AddConfigVariableFlags(
      extraLinkOptions, extraLinkOptionsVar, configName);
  }

  if (gtgt->GetType() == cmStateEnums::OBJECT_LIBRARY ||
      gtgt->GetType() == cmStateEnums::STATIC_LIBRARY) {
    this->CurrentLocalGenerator->GetStaticLibraryFlags(
      extraLinkOptions, configName, llang, gtgt);
  } else {
    cmValue targetLinkFlags = gtgt->GetProperty("LINK_FLAGS");
    if (targetLinkFlags) {
      this->CurrentLocalGenerator->AppendFlags(extraLinkOptions,
                                               *targetLinkFlags);
    }
    if (!configName.empty()) {
      std::string linkFlagsVar =
        cmStrCat("LINK_FLAGS_", cmSystemTools::UpperCase(configName));
      if (cmValue linkFlags = gtgt->GetProperty(linkFlagsVar)) {
        this->CurrentLocalGenerator->AppendFlags(extraLinkOptions, *linkFlags);
      }
    }
    std::vector<std::string> opts;
    gtgt->GetLinkOptions(opts, configName, llang);
    // LINK_OPTIONS are escaped.
    this->CurrentLocalGenerator->AppendCompileOptions(extraLinkOptions, opts);
  }

  // Set target-specific architectures.
  std::vector<std::string> archs =
    gtgt->GetAppleArchs(configName, cm::nullopt);

  if (!archs.empty()) {
    // Enable ARCHS attribute.
    buildSettings->AddAttribute("ONLY_ACTIVE_ARCH", this->CreateString("NO"));

    // Store ARCHS value.
    if (archs.size() == 1) {
      buildSettings->AddAttribute("ARCHS", this->CreateString(archs[0]));
    } else {
      cmXCodeObject* archObjects =
        this->CreateObject(cmXCodeObject::OBJECT_LIST);
      for (auto& arch : archs) {
        archObjects->AddObject(this->CreateString(arch));
      }
      buildSettings->AddAttribute("ARCHS", archObjects);
    }
  }

  // Get the product name components.
  cmGeneratorTarget::NameComponents const& components =
    gtgt->GetFullNameComponents(configName);

  cmValue version = gtgt->GetProperty("VERSION");
  cmValue soversion = gtgt->GetProperty("SOVERSION");
  if (!gtgt->HasSOName(configName) || gtgt->IsFrameworkOnApple()) {
    version = nullptr;
    soversion = nullptr;
  }
  if (version && !soversion) {
    soversion = version;
  }
  if (!version && soversion) {
    version = soversion;
  }

  std::string realName = components.base;
  std::string soName = components.base;
  if (version && soversion) {
    realName += '.';
    realName += *version;
    soName += '.';
    soName += *soversion;
  }

  if (gtgt->CanCompileSources()) {
    std::string const tmpDir =
      this->GetTargetTempDir(gtgt, this->GetCMakeCFGIntDir());
    buildSettings->AddAttribute("TARGET_TEMP_DIR", this->CreateString(tmpDir));

    std::string outDir;
    if (gtgt->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      // We cannot suppress the archive, so hide it with intermediate files.
      outDir = tmpDir;
    } else {
      outDir = gtgt->GetDirectory(configName);
    }
    buildSettings->AddAttribute("CONFIGURATION_BUILD_DIR",
                                this->CreateString(outDir));
  }

  // Set attributes to specify the proper name for the target.
  std::string pndir = this->CurrentLocalGenerator->GetCurrentBinaryDirectory();
  if (gtgt->GetType() == cmStateEnums::STATIC_LIBRARY ||
      gtgt->GetType() == cmStateEnums::SHARED_LIBRARY ||
      gtgt->GetType() == cmStateEnums::MODULE_LIBRARY ||
      gtgt->GetType() == cmStateEnums::EXECUTABLE) {
    std::string prefix = components.prefix;
    if (gtgt->IsFrameworkOnApple() || gtgt->IsCFBundleOnApple()) {
      prefix = "";
    }

    buildSettings->AddAttribute("EXECUTABLE_PREFIX",
                                this->CreateString(prefix));
    buildSettings->AddAttribute("EXECUTABLE_SUFFIX",
                                this->CreateString(components.suffix));
  }

  // Store the product name for all target types.
  buildSettings->AddAttribute("PRODUCT_NAME", this->CreateString(realName));

  // Handle settings for each target type.
  switch (gtgt->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      if (gtgt->GetPropertyAsBool("FRAMEWORK")) {
        std::string fw_version = gtgt->GetFrameworkVersion();
        buildSettings->AddAttribute("FRAMEWORK_VERSION",
                                    this->CreateString(fw_version));
        cmValue ext = gtgt->GetProperty("BUNDLE_EXTENSION");
        if (ext) {
          buildSettings->AddAttribute("WRAPPER_EXTENSION",
                                      this->CreateString(*ext));
        }

        std::string plist = this->ComputeInfoPListLocation(gtgt);
        // Xcode will create the final version of Info.plist at build time,
        // so let it replace the framework name. This avoids creating
        // a per-configuration Info.plist file.
        this->CurrentLocalGenerator->GenerateFrameworkInfoPList(
          gtgt, "$(EXECUTABLE_NAME)", plist);
        buildSettings->AddAttribute("INFOPLIST_FILE",
                                    this->CreateString(plist));
        buildSettings->AddAttribute("MACH_O_TYPE",
                                    this->CreateString("staticlib"));
      } else {
        buildSettings->AddAttribute("LIBRARY_STYLE",
                                    this->CreateString("STATIC"));
      }
      break;

    case cmStateEnums::OBJECT_LIBRARY: {
      buildSettings->AddAttribute("LIBRARY_STYLE",
                                  this->CreateString("STATIC"));
      break;
    }

    case cmStateEnums::MODULE_LIBRARY: {
      buildSettings->AddAttribute("LIBRARY_STYLE",
                                  this->CreateString("BUNDLE"));
      if (gtgt->IsCFBundleOnApple()) {
        // It turns out that a BUNDLE is basically the same
        // in many ways as an application bundle, as far as
        // link flags go
        std::string createFlags = this->LookupFlags(
          "CMAKE_SHARED_MODULE_CREATE_", llang, "_FLAGS", "-bundle");
        if (!createFlags.empty()) {
          extraLinkOptions += ' ';
          extraLinkOptions += createFlags;
        }
        cmValue ext = gtgt->GetProperty("BUNDLE_EXTENSION");
        if (ext) {
          buildSettings->AddAttribute("WRAPPER_EXTENSION",
                                      this->CreateString(*ext));
        }
        std::string plist = this->ComputeInfoPListLocation(gtgt);
        // Xcode will create the final version of Info.plist at build time,
        // so let it replace the cfbundle name. This avoids creating
        // a per-configuration Info.plist file. The cfbundle plist
        // is very similar to the application bundle plist
        this->CurrentLocalGenerator->GenerateAppleInfoPList(
          gtgt, "$(EXECUTABLE_NAME)", plist);
        buildSettings->AddAttribute("INFOPLIST_FILE",
                                    this->CreateString(plist));
      } else {
        buildSettings->AddAttribute("MACH_O_TYPE",
                                    this->CreateString("mh_bundle"));
        buildSettings->AddAttribute("GCC_DYNAMIC_NO_PIC",
                                    this->CreateString("NO"));
        // Add the flags to create an executable.
        std::string createFlags =
          this->LookupFlags("CMAKE_", llang, "_LINK_FLAGS", "");
        if (!createFlags.empty()) {
          extraLinkOptions += ' ';
          extraLinkOptions += createFlags;
        }
      }
      break;
    }
    case cmStateEnums::SHARED_LIBRARY: {
      if (gtgt->GetPropertyAsBool("FRAMEWORK")) {
        std::string fw_version = gtgt->GetFrameworkVersion();
        buildSettings->AddAttribute("FRAMEWORK_VERSION",
                                    this->CreateString(fw_version));
        cmValue ext = gtgt->GetProperty("BUNDLE_EXTENSION");
        if (ext) {
          buildSettings->AddAttribute("WRAPPER_EXTENSION",
                                      this->CreateString(*ext));
        }

        std::string plist = this->ComputeInfoPListLocation(gtgt);
        // Xcode will create the final version of Info.plist at build time,
        // so let it replace the framework name. This avoids creating
        // a per-configuration Info.plist file.
        this->CurrentLocalGenerator->GenerateFrameworkInfoPList(
          gtgt, "$(EXECUTABLE_NAME)", plist);
        buildSettings->AddAttribute("INFOPLIST_FILE",
                                    this->CreateString(plist));
      } else {
        // Add the flags to create a shared library.
        std::string createFlags = this->LookupFlags(
          "CMAKE_SHARED_LIBRARY_CREATE_", llang, "_FLAGS", "-dynamiclib");
        if (!createFlags.empty()) {
          extraLinkOptions += ' ';
          extraLinkOptions += createFlags;
        }
      }

      buildSettings->AddAttribute("LIBRARY_STYLE",
                                  this->CreateString("DYNAMIC"));

      if (gtgt->HasImportLibrary(configName)) {
        // Request .tbd file generation
        buildSettings->AddAttribute("GENERATE_TEXT_BASED_STUBS",
                                    this->CreateString("YES"));
      }
      break;
    }
    case cmStateEnums::EXECUTABLE: {
      // Add the flags to create an executable.
      std::string createFlags =
        this->LookupFlags("CMAKE_", llang, "_LINK_FLAGS", "");
      if (!createFlags.empty()) {
        extraLinkOptions += ' ';
        extraLinkOptions += createFlags;
      }

      // Handle bundles and normal executables separately.
      if (gtgt->GetPropertyAsBool("MACOSX_BUNDLE")) {
        cmValue ext = gtgt->GetProperty("BUNDLE_EXTENSION");
        if (ext) {
          buildSettings->AddAttribute("WRAPPER_EXTENSION",
                                      this->CreateString(*ext));
        }
        std::string plist = this->ComputeInfoPListLocation(gtgt);
        // Xcode will create the final version of Info.plist at build time,
        // so let it replace the executable name.  This avoids creating
        // a per-configuration Info.plist file.
        this->CurrentLocalGenerator->GenerateAppleInfoPList(
          gtgt, "$(EXECUTABLE_NAME)", plist);
        buildSettings->AddAttribute("INFOPLIST_FILE",
                                    this->CreateString(plist));
      }
    } break;
    default:
      break;
  }

  BuildObjectListOrString dirs(this, true);
  BuildObjectListOrString fdirs(this, true);
  BuildObjectListOrString sysdirs(this, true);
  BuildObjectListOrString sysfdirs(this, true);
  const bool emitSystemIncludes = this->XcodeVersion >= 83;

  // Choose a language to use for target-wide include directories.
  std::string const& langForIncludes = llang;
  std::vector<std::string> includes;
  if (!langForIncludes.empty()) {
    this->CurrentLocalGenerator->GetIncludeDirectories(
      includes, gtgt, langForIncludes, configName);
  }
  std::set<std::string> emitted;
  emitted.insert("/System/Library/Frameworks");

  for (auto& include : includes) {
    if (this->NameResolvesToFramework(include)) {
      std::string frameworkDir = cmStrCat(include, "/../");
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir);
      if (emitted.insert(frameworkDir).second) {
        std::string incpath = this->XCodeEscapePath(frameworkDir);
        if (emitSystemIncludes &&
            gtgt->IsSystemIncludeDirectory(frameworkDir, configName,
                                           langForIncludes)) {
          sysfdirs.Add(incpath);
        } else {
          fdirs.Add(incpath);
        }
      }
    } else {
      std::string incpath = this->XCodeEscapePath(include);
      if (emitSystemIncludes &&
          gtgt->IsSystemIncludeDirectory(include, configName,
                                         langForIncludes)) {
        sysdirs.Add(incpath);
      } else {
        dirs.Add(incpath);
      }
    }
  }
  // Add framework search paths needed for linking.
  if (cmComputeLinkInformation* cli = gtgt->GetLinkInformation(configName)) {
    for (auto const& fwDir : cli->GetFrameworkPaths()) {
      if (emitted.insert(fwDir).second) {
        std::string incpath = this->XCodeEscapePath(fwDir);
        if (emitSystemIncludes &&
            gtgt->IsSystemIncludeDirectory(fwDir, configName,
                                           langForIncludes)) {
          sysfdirs.Add(incpath);
        } else {
          fdirs.Add(incpath);
        }
      }
    }
  }
  if (!fdirs.IsEmpty()) {
    buildSettings->AddAttribute("FRAMEWORK_SEARCH_PATHS", fdirs.CreateList());
  }
  if (!dirs.IsEmpty()) {
    buildSettings->AddAttribute("HEADER_SEARCH_PATHS", dirs.CreateList());
    if (languages.count("Swift")) {
      buildSettings->AddAttribute("SWIFT_INCLUDE_PATHS", dirs.CreateList());
    }
  }
  if (!sysfdirs.IsEmpty()) {
    buildSettings->AddAttribute("SYSTEM_FRAMEWORK_SEARCH_PATHS",
                                sysfdirs.CreateList());
  }
  if (!sysdirs.IsEmpty()) {
    buildSettings->AddAttribute("SYSTEM_HEADER_SEARCH_PATHS",
                                sysdirs.CreateList());
  }

  if (this->XcodeVersion >= 60 && !emitSystemIncludes) {
    // Add those per-language flags in addition to HEADER_SEARCH_PATHS to gain
    // system include directory awareness. We need to also keep on setting
    // HEADER_SEARCH_PATHS to work around a missing compile options flag for
    // GNU assembly files (#16449)
    for (auto const& language : languages) {
      std::string includeFlags = this->CurrentLocalGenerator->GetIncludeFlags(
        includes, gtgt, language, configName);

      if (!includeFlags.empty()) {
        cflags[language] += cmStrCat(' ', includeFlags);
      }
    }
  }

  bool same_gflags = true;
  std::map<std::string, std::string> gflags;
  std::string const* last_gflag = nullptr;
  std::string optLevel = "0";

  // Minimal map of flags to build settings.
  for (auto const& language : languages) {
    std::string& flags = cflags[language];
    std::string& gflag = gflags[language];
    std::string oflag =
      this->ExtractFlagRegex("(^| )(-Ofast|-Os|-O[0-9]*)( |$)", 2, flags);
    if (oflag.size() == 2) {
      optLevel = "1";
    } else if (oflag.size() > 2) {
      optLevel = oflag.substr(2);
    }
    gflag = this->ExtractFlag("-g", flags);
    // put back gdwarf-2 if used since there is no way
    // to represent it in the gui, but we still want debug yes
    if (gflag == "-gdwarf-2"_s) {
      flags += ' ';
      flags += gflag;
    }
    if (last_gflag && *last_gflag != gflag) {
      same_gflags = false;
    }
    last_gflag = &gflag;
  }

  const char* debugStr = "YES";
  if (!same_gflags) {
    // We can't set the Xcode flag differently depending on the language,
    // so put them back in this case.
    for (auto const& language : languages) {
      cflags[language] += ' ';
      cflags[language] += gflags[language];
    }
    debugStr = "NO";
  } else if (last_gflag && (last_gflag->empty() || *last_gflag == "-g0"_s)) {
    debugStr = "NO";
  }

  // extract C++ stdlib
  for (auto const& language : languages) {
    if (language != "CXX"_s && language != "OBJCXX"_s) {
      continue;
    }
    std::string& flags = cflags[language];

    auto stdlib =
      this->ExtractFlagRegex("(^| )(-stdlib=[^ ]+)( |$)", 2, flags);
    if (stdlib.size() > 8) {
      const auto cxxLibrary = stdlib.substr(8);
      if (language == "CXX"_s ||
          !buildSettings->GetAttribute("CLANG_CXX_LIBRARY")) {
        buildSettings->AddAttribute("CLANG_CXX_LIBRARY",
                                    this->CreateString(cxxLibrary));
      }
    }
  }

  buildSettings->AddAttribute("COMBINE_HIDPI_IMAGES",
                              this->CreateString("YES"));
  buildSettings->AddAttribute("GCC_GENERATE_DEBUGGING_SYMBOLS",
                              this->CreateString(debugStr));
  buildSettings->AddAttribute("GCC_OPTIMIZATION_LEVEL",
                              this->CreateString(optLevel));
  buildSettings->AddAttribute("GCC_SYMBOLS_PRIVATE_EXTERN",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("GCC_INLINES_ARE_PRIVATE_EXTERN",
                              this->CreateString("NO"));

  for (auto const& language : languages) {
    std::string flags = cmStrCat(cflags[language], ' ', defFlags);
    if (language == "CXX"_s || language == "OBJCXX"_s) {
      if (language == "CXX"_s ||
          !buildSettings->GetAttribute("OTHER_CPLUSPLUSFLAGS")) {
        buildSettings->AddAttribute("OTHER_CPLUSPLUSFLAGS",
                                    this->CreateString(flags));
      }
    } else if (language == "Fortran"_s) {
      buildSettings->AddAttribute("IFORT_OTHER_FLAGS",
                                  this->CreateString(flags));
    } else if (language == "C"_s || language == "OBJC"_s) {
      if (language == "C"_s || !buildSettings->GetAttribute("OTHER_CFLAGS")) {
        buildSettings->AddAttribute("OTHER_CFLAGS", this->CreateString(flags));
      }
    } else if (language == "Swift"_s) {
      buildSettings->AddAttribute("OTHER_SWIFT_FLAGS",
                                  this->CreateString(flags));
    }
  }

  // Add Fortran source format attribute if property is set.
  const char* format = nullptr;
  std::string const& tgtfmt = gtgt->GetSafeProperty("Fortran_FORMAT");
  switch (cmOutputConverter::GetFortranFormat(tgtfmt)) {
    case cmOutputConverter::FortranFormatFixed:
      format = "fixed";
      break;
    case cmOutputConverter::FortranFormatFree:
      format = "free";
      break;
    default:
      break;
  }
  if (format) {
    buildSettings->AddAttribute("IFORT_LANG_SRCFMT",
                                this->CreateString(format));
  }

  // Create the INSTALL_PATH attribute.
  std::string install_name_dir;
  if (gtgt->GetType() == cmStateEnums::SHARED_LIBRARY) {
    // Get the install_name directory for the build tree.
    install_name_dir = gtgt->GetInstallNameDirForBuildTree(configName);
    // Xcode doesn't create the correct install_name in some cases.
    // That is, if the INSTALL_PATH is empty, or if we have versioning
    // of dylib libraries, we want to specify the install_name.
    // This is done by adding a link flag to create an install_name
    // with just the library soname.
    std::string install_name;
    if (!install_name_dir.empty()) {
      // Convert to a path for the native build tool.
      cmSystemTools::ConvertToUnixSlashes(install_name_dir);
      install_name += install_name_dir;
      install_name += '/';
    }
    install_name += gtgt->GetSOName(configName);

    if ((realName != soName) || install_name_dir.empty()) {
      install_name_dir = "";
      extraLinkOptions += " -install_name ";
      extraLinkOptions += XCodeEscapePath(install_name);
    }
  }
  buildSettings->AddAttribute("INSTALL_PATH",
                              this->CreateString(install_name_dir));

  // Create the LD_RUNPATH_SEARCH_PATHS
  cmComputeLinkInformation* pcli = gtgt->GetLinkInformation(configName);
  if (pcli) {
    std::string search_paths;
    std::vector<std::string> runtimeDirs;
    pcli->GetRPath(runtimeDirs, false);
    // runpath dirs needs to be unique to prevent corruption
    std::set<std::string> unique_dirs;

    for (auto runpath : runtimeDirs) {
      runpath = this->ExpandCFGIntDir(runpath, configName);

      if (unique_dirs.find(runpath) == unique_dirs.end()) {
        unique_dirs.insert(runpath);
        if (!search_paths.empty()) {
          search_paths += ' ';
        }
        search_paths += this->XCodeEscapePath(runpath);
      }
    }
    if (!search_paths.empty()) {
      buildSettings->AddAttribute("LD_RUNPATH_SEARCH_PATHS",
                                  this->CreateString(search_paths));
    }
  }

  buildSettings->AddAttribute(this->GetTargetLinkFlagsVar(gtgt),
                              this->CreateString(extraLinkOptions));
  buildSettings->AddAttribute("OTHER_REZFLAGS", this->CreateString(""));
  buildSettings->AddAttribute("SECTORDER_FLAGS", this->CreateString(""));
  buildSettings->AddAttribute("ALWAYS_SEARCH_USER_PATHS",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("USE_HEADERMAP", this->CreateString("NO"));
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  group->AddObject(this->CreateString("$(inherited)"));
  buildSettings->AddAttribute("WARNING_CFLAGS", group);

  // Runtime version information.
  if (gtgt->GetType() == cmStateEnums::SHARED_LIBRARY) {
    int major;
    int minor;
    int patch;

    // MACHO_CURRENT_VERSION or VERSION -> current_version
    gtgt->GetTargetVersionFallback("MACHO_CURRENT_VERSION", "VERSION", major,
                                   minor, patch);
    std::ostringstream v;

    // Xcode always wants at least 1.0.0 or nothing
    if (!(major == 0 && minor == 0 && patch == 0)) {
      v << major << '.' << minor << '.' << patch;
    }
    buildSettings->AddAttribute("DYLIB_CURRENT_VERSION",
                                this->CreateString(v.str()));

    // MACHO_COMPATIBILITY_VERSION or SOVERSION -> compatibility_version
    gtgt->GetTargetVersionFallback("MACHO_COMPATIBILITY_VERSION", "SOVERSION",
                                   major, minor, patch);
    std::ostringstream vso;

    // Xcode always wants at least 1.0.0 or nothing
    if (!(major == 0 && minor == 0 && patch == 0)) {
      vso << major << '.' << minor << '.' << patch;
    }
    buildSettings->AddAttribute("DYLIB_COMPATIBILITY_VERSION",
                                this->CreateString(vso.str()));
  }

  // Precompile Headers
  std::string pchHeader = gtgt->GetPchHeader(configName, llang);
  if (!pchHeader.empty()) {
    buildSettings->AddAttribute("GCC_PREFIX_HEADER",
                                this->CreateString(pchHeader));
    buildSettings->AddAttribute("GCC_PRECOMPILE_PREFIX_HEADER",
                                this->CreateString("YES"));
  }

  // put this last so it can override existing settings
  // Convert "XCODE_ATTRIBUTE_*" properties directly.
  {
    for (auto const& prop : gtgt->GetPropertyKeys()) {
      if (cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
        std::string attribute = prop.substr(16);
        this->FilterConfigurationAttribute(configName, attribute);
        if (!attribute.empty()) {
          std::string const& pr = gtgt->GetSafeProperty(prop);
          std::string processed = cmGeneratorExpression::Evaluate(
            pr, this->CurrentLocalGenerator, configName);
          buildSettings->AddAttribute(attribute,
                                      this->CreateString(processed));
        }
      }
    }
  }
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateUtilityTarget(
  cmGeneratorTarget* gtgt)
{
  cmXCodeObject* shellBuildPhase = this->CreateObject(
    cmXCodeObject::PBXShellScriptBuildPhase, gtgt->GetName());
  shellBuildPhase->AddAttribute("buildActionMask",
                                this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("files", buildFiles);
  cmXCodeObject* inputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("inputPaths", inputPaths);
  cmXCodeObject* outputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("outputPaths", outputPaths);
  shellBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                this->CreateString("0"));
  shellBuildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
  shellBuildPhase->AddAttribute(
    "shellScript", this->CreateString("# shell script goes here\nexit 0"));
  shellBuildPhase->AddAttribute("showEnvVarsInLog", this->CreateString("0"));

  cmXCodeObject* target =
    this->CreateObject(cmXCodeObject::PBXAggregateTarget);
  target->SetComment(gtgt->GetName());
  cmXCodeObject* buildPhases = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  std::vector<cmXCodeObject*> emptyContentVector;
  this->CreateCustomCommands(buildPhases, nullptr, nullptr, nullptr,
                             emptyContentVector, nullptr, gtgt);
  target->AddAttribute("buildPhases", buildPhases);
  this->AddConfigurations(target, gtgt);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(gtgt->GetName()));
  target->AddAttribute("productName", this->CreateString(gtgt->GetName()));
  target->SetTarget(gtgt);
  this->XCodeObjectMap[gtgt] = target;

  // Add source files without build rules for editing convenience.
  if (gtgt->GetType() != cmStateEnums::GLOBAL_TARGET &&
      gtgt->GetName() != CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
    std::vector<cmSourceFile*> sources;
    if (!gtgt->GetConfigCommonSourceFilesForXcode(sources)) {
      return nullptr;
    }

    // Add CMakeLists.txt file for user convenience.
    this->AddXCodeProjBuildRule(gtgt, sources);

    for (auto* sourceFile : sources) {
      if (!sourceFile->GetIsGenerated()) {
        this->CreateXCodeFileReference(sourceFile, gtgt);
      }
    }
  }

  target->SetId(this->GetOrCreateId(gtgt->GetName(), target->GetId()));

  return target;
}

std::string cmGlobalXCodeGenerator::AddConfigurations(cmXCodeObject* target,
                                                      cmGeneratorTarget* gtgt)
{
  cmList const configList{ this->CurrentMakefile->GetRequiredDefinition(
    "CMAKE_CONFIGURATION_TYPES") };
  cmXCodeObject* configlist =
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  configlist->AddAttribute("buildConfigurations", buildConfigurations);
  std::string comment = cmStrCat("Build configuration list for ",
                                 cmXCodeObject::PBXTypeNames[target->GetIsA()],
                                 " \"", gtgt->GetName(), '"');
  configlist->SetComment(comment);
  target->AddAttribute("buildConfigurationList",
                       this->CreateObjectReference(configlist));
  for (auto const& i : configList) {
    cmXCodeObject* config =
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    buildConfigurations->AddObject(config);
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    this->CreateBuildSettings(gtgt, buildSettings, i);
    config->AddAttribute("name", this->CreateString(i));
    config->SetComment(i);
    config->AddAttribute("buildSettings", buildSettings);

    this->CreateTargetXCConfigSettings(gtgt, config, i);
  }
  if (!configList.empty()) {
    configlist->AddAttribute("defaultConfigurationName",
                             this->CreateString(configList[0]));
    configlist->AddAttribute("defaultConfigurationIsVisible",
                             this->CreateString("0"));
    return configList[0];
  }
  return "";
}

void cmGlobalXCodeGenerator::CreateGlobalXCConfigSettings(
  cmLocalGenerator* root, cmXCodeObject* config, const std::string& configName)
{
  auto xcconfig = cmGeneratorExpression::Evaluate(
    this->CurrentMakefile->GetSafeDefinition("CMAKE_XCODE_XCCONFIG"),
    this->CurrentLocalGenerator, configName);
  if (xcconfig.empty()) {
    return;
  }

  auto* sf = this->CurrentMakefile->GetSource(xcconfig);
  if (!sf) {
    cmSystemTools::Error(
      cmStrCat("sources for ALL_BUILD do not contain xcconfig file: '",
               xcconfig, "' (configuration: ", configName, ')'));
    return;
  }

  cmXCodeObject* fileRef = this->CreateXCodeFileReferenceFromPath(
    sf->ResolveFullPath(), root->FindGeneratorTargetToUse("ALL_BUILD"), "",
    sf);

  if (!fileRef) {
    // error is already reported by CreateXCodeFileReferenceFromPath
    return;
  }

  config->AddAttribute("baseConfigurationReference",
                       this->CreateObjectReference(fileRef));
}

void cmGlobalXCodeGenerator::CreateTargetXCConfigSettings(
  cmGeneratorTarget* target, cmXCodeObject* config,
  const std::string& configName)
{
  auto xcconfig =
    cmGeneratorExpression::Evaluate(target->GetSafeProperty("XCODE_XCCONFIG"),
                                    this->CurrentLocalGenerator, configName);
  if (xcconfig.empty()) {
    return;
  }

  auto* sf = target->Makefile->GetSource(xcconfig);
  if (!sf) {
    cmSystemTools::Error(cmStrCat("target sources for target ",
                                  target->Target->GetName(),
                                  " do not contain xcconfig file: '", xcconfig,
                                  "' (configuration: ", configName, ')'));
    return;
  }

  cmXCodeObject* fileRef = this->CreateXCodeFileReferenceFromPath(
    sf->ResolveFullPath(), target, "", sf);
  if (!fileRef) {
    // error is already reported by CreateXCodeFileReferenceFromPath
    return;
  }
  config->AddAttribute("baseConfigurationReference",
                       this->CreateObjectReference(fileRef));
}

const char* cmGlobalXCodeGenerator::GetTargetLinkFlagsVar(
  cmGeneratorTarget const* target) const
{
  if (this->XcodeVersion >= 60 &&
      (target->GetType() == cmStateEnums::STATIC_LIBRARY ||
       target->GetType() == cmStateEnums::OBJECT_LIBRARY)) {
    return "OTHER_LIBTOOLFLAGS";
  }
  return "OTHER_LDFLAGS";
}

const char* cmGlobalXCodeGenerator::GetTargetFileType(
  cmGeneratorTarget* target)
{
  if (cmValue e = target->GetProperty("XCODE_EXPLICIT_FILE_TYPE")) {
    return e->c_str();
  }

  switch (target->GetType()) {
    case cmStateEnums::OBJECT_LIBRARY:
      return "archive.ar";
    case cmStateEnums::STATIC_LIBRARY:
      return (target->GetPropertyAsBool("FRAMEWORK") ? "wrapper.framework"
                                                     : "archive.ar");
    case cmStateEnums::MODULE_LIBRARY:
      if (target->IsXCTestOnApple()) {
        return "wrapper.cfbundle";
      }
      if (target->IsCFBundleOnApple()) {
        return "wrapper.plug-in";
      }
      return "compiled.mach-o.executable";
    case cmStateEnums::SHARED_LIBRARY:
      return (target->GetPropertyAsBool("FRAMEWORK")
                ? "wrapper.framework"
                : "compiled.mach-o.dylib");
    case cmStateEnums::EXECUTABLE:
      return "compiled.mach-o.executable";
    default:
      break;
  }
  return nullptr;
}

const char* cmGlobalXCodeGenerator::GetTargetProductType(
  cmGeneratorTarget* target)
{
  if (cmValue e = target->GetProperty("XCODE_PRODUCT_TYPE")) {
    return e->c_str();
  }

  switch (target->GetType()) {
    case cmStateEnums::OBJECT_LIBRARY:
      return "com.apple.product-type.library.static";
    case cmStateEnums::STATIC_LIBRARY:
      return (target->GetPropertyAsBool("FRAMEWORK")
                ? "com.apple.product-type.framework"
                : "com.apple.product-type.library.static");
    case cmStateEnums::MODULE_LIBRARY:
      if (target->IsXCTestOnApple()) {
        return "com.apple.product-type.bundle.unit-test";
      } else if (target->IsCFBundleOnApple()) {
        return "com.apple.product-type.bundle";
      } else {
        return "com.apple.product-type.tool";
      }
    case cmStateEnums::SHARED_LIBRARY:
      return (target->GetPropertyAsBool("FRAMEWORK")
                ? "com.apple.product-type.framework"
                : "com.apple.product-type.library.dynamic");
    case cmStateEnums::EXECUTABLE:
      return (target->GetPropertyAsBool("MACOSX_BUNDLE")
                ? "com.apple.product-type.application"
                : "com.apple.product-type.tool");
    default:
      break;
  }
  return nullptr;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateXCodeTarget(
  cmGeneratorTarget* gtgt, cmXCodeObject* buildPhases)
{
  if (!gtgt->IsInBuildSystem()) {
    return nullptr;
  }
  cmXCodeObject* target = this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  std::string defConfig;
  defConfig = this->AddConfigurations(target, gtgt);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(gtgt->GetName()));
  target->AddAttribute("productName", this->CreateString(gtgt->GetName()));

  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  if (const char* fileType = this->GetTargetFileType(gtgt)) {
    fileRef->AddAttribute("explicitFileType", this->CreateString(fileType));
  }
  std::string fullName;
  if (gtgt->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    fullName = cmStrCat("lib", gtgt->GetName(), ".a");
  } else {
    fullName = gtgt->GetFullName(defConfig);
  }
  fileRef->AddAttribute("path", this->CreateString(fullName));
  fileRef->AddAttribute("sourceTree",
                        this->CreateString("BUILT_PRODUCTS_DIR"));
  fileRef->SetComment(gtgt->GetName());
  target->AddAttribute("productReference",
                       this->CreateObjectReference(fileRef));
  if (const char* productType = this->GetTargetProductType(gtgt)) {
    target->AddAttribute("productType", this->CreateString(productType));
  }
  target->SetTarget(gtgt);
  this->XCodeObjectMap[gtgt] = target;
  target->SetId(this->GetOrCreateId(gtgt->GetName(), target->GetId()));
  return target;
}

cmXCodeObject* cmGlobalXCodeGenerator::FindXCodeTarget(
  cmGeneratorTarget const* t)
{
  if (!t) {
    return nullptr;
  }

  auto const i = this->XCodeObjectMap.find(t);
  if (i == this->XCodeObjectMap.end()) {
    return nullptr;
  }
  return i->second;
}

std::string cmGlobalXCodeGenerator::GetObjectId(cmXCodeObject::PBXType ptype,
                                                cm::string_view key)
{
  std::string objectId;
  if (!key.empty()) {
    cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
    hash.Initialize();
    hash.Append(&ptype, sizeof(ptype));
    hash.Append(key);
    objectId = cmSystemTools::UpperCase(hash.FinalizeHex().substr(0, 24));
  } else {
    char cUuid[40] = { 0 };
    CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
    CFStringRef s = CFUUIDCreateString(kCFAllocatorDefault, uuid);
    CFStringGetCString(s, cUuid, sizeof(cUuid), kCFStringEncodingUTF8);
    objectId = cUuid;
    CFRelease(s);
    CFRelease(uuid);
    cmSystemTools::ReplaceString(objectId, "-", "");
    if (objectId.size() > 24) {
      objectId = objectId.substr(0, 24);
    }
  }
  return objectId;
}

std::string cmGlobalXCodeGenerator::GetOrCreateId(const std::string& name,
                                                  const std::string& id)
{
  std::string guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  cmValue storedGUID = this->CMakeInstance->GetCacheDefinition(guidStoreName);

  if (storedGUID) {
    return *storedGUID;
  }

  this->CMakeInstance->AddCacheEntry(
    guidStoreName, id, "Stored Xcode object GUID", cmStateEnums::INTERNAL);

  return id;
}

void cmGlobalXCodeGenerator::AddDependTarget(cmXCodeObject* target,
                                             cmXCodeObject* dependTarget)
{
  // This is called once for every edge in the target dependency graph.
  cmXCodeObject* container =
    this->CreateObject(cmXCodeObject::PBXContainerItemProxy);
  container->SetComment("PBXContainerItemProxy");
  container->AddAttribute("containerPortal",
                          this->CreateObjectReference(this->RootObject));
  container->AddAttribute("proxyType", this->CreateString("1"));
  container->AddAttribute("remoteGlobalIDString",
                          this->CreateObjectReference(dependTarget));
  container->AddAttribute(
    "remoteInfo", this->CreateString(dependTarget->GetTarget()->GetName()));
  cmXCodeObject* targetdep =
    this->CreateObject(cmXCodeObject::PBXTargetDependency);
  targetdep->SetComment("PBXTargetDependency");
  targetdep->AddAttribute("target", this->CreateObjectReference(dependTarget));
  targetdep->AddAttribute("targetProxy",
                          this->CreateObjectReference(container));

  cmXCodeObject* depends = target->GetAttribute("dependencies");
  if (!depends) {
    cmSystemTools::Error(
      "target does not have dependencies attribute error..");

  } else {
    depends->AddUniqueObject(targetdep);
  }
}

void cmGlobalXCodeGenerator::AppendOrAddBuildSetting(cmXCodeObject* settings,
                                                     const char* attribute,
                                                     cmXCodeObject* value)
{
  if (settings) {
    cmXCodeObject* attr = settings->GetAttribute(attribute);
    if (!attr) {
      settings->AddAttribute(attribute, value);
    } else {
      this->AppendBuildSettingAttribute(settings, attribute, attr, value);
    }
  }
}

void cmGlobalXCodeGenerator::AppendBuildSettingAttribute(
  cmXCodeObject* settings, const char* attribute, cmXCodeObject* attr,
  cmXCodeObject* value)
{
  if (value->GetType() != cmXCodeObject::OBJECT_LIST &&
      value->GetType() != cmXCodeObject::STRING) {
    cmSystemTools::Error(
      cmStrCat("Unsupported value type for appending: ", attribute));
    return;
  }
  if (attr->GetType() == cmXCodeObject::OBJECT_LIST) {
    if (value->GetType() == cmXCodeObject::OBJECT_LIST) {
      for (auto* obj : value->GetObjectList()) {
        attr->AddObject(obj);
      }
    } else {
      attr->AddObject(value);
    }
  } else if (attr->GetType() == cmXCodeObject::STRING) {
    if (value->GetType() == cmXCodeObject::OBJECT_LIST) {
      // Add old value as a list item to new object list
      // and replace the attribute with the new list
      value->PrependObject(attr);
      settings->AddAttribute(attribute, value);
    } else {
      std::string newValue =
        cmStrCat(attr->GetString(), ' ', value->GetString());
      attr->SetString(newValue);
    }
  } else {
    cmSystemTools::Error(
      cmStrCat("Unsupported attribute type for appending: ", attribute));
  }
}

void cmGlobalXCodeGenerator::AppendBuildSettingAttribute(
  cmXCodeObject* target, const char* attribute, cmXCodeObject* value,
  const std::string& configName)
{
  // There are multiple configurations.  Add the setting to the
  // buildSettings of the configuration name given.
  cmXCodeObject* configurationList =
    target->GetAttribute("buildConfigurationList")->GetObject();
  cmXCodeObject* buildConfigs =
    configurationList->GetAttribute("buildConfigurations");
  for (auto* obj : buildConfigs->GetObjectList()) {
    if (configName.empty() ||
        obj->GetAttribute("name")->GetString() == configName) {
      cmXCodeObject* settings = obj->GetAttribute("buildSettings");
      this->AppendOrAddBuildSetting(settings, attribute, value);
    }
  }
}

void cmGlobalXCodeGenerator::InheritBuildSettingAttribute(
  cmXCodeObject* target, const char* attribute)
{
  cmXCodeObject* configurationList =
    target->GetAttribute("buildConfigurationList")->GetObject();
  cmXCodeObject* buildConfigs =
    configurationList->GetAttribute("buildConfigurations");
  for (auto* obj : buildConfigs->GetObjectList()) {
    cmXCodeObject* settings = obj->GetAttribute("buildSettings");
    if (cmXCodeObject* attr = settings->GetAttribute(attribute)) {
      BuildObjectListOrString inherited(this, true);
      inherited.Add("$(inherited)");
      this->AppendBuildSettingAttribute(settings, attribute, attr,
                                        inherited.CreateList());
    }
  }
}

void cmGlobalXCodeGenerator::AddDependAndLinkInformation(cmXCodeObject* target)
{
  cmGeneratorTarget* gt = target->GetTarget();
  if (!gt) {
    cmSystemTools::Error("Error no target on xobject\n");
    return;
  }
  if (!gt->IsInBuildSystem()) {
    return;
  }

  // Add dependencies on other CMake targets.
  for (const auto& dep : this->GetTargetDirectDepends(gt)) {
    if (cmXCodeObject* dptarget = this->FindXCodeTarget(dep)) {
      this->AddDependTarget(target, dptarget);
    }
  }

  // Separate libraries into ones that can be linked using "Link Binary With
  // Libraries" build phase and the ones that can't. Only targets that build
  // Apple bundles (.app, .framework, .bundle), executables and dylibs can use
  // this feature and only targets that represent actual libraries (object,
  // static, dynamic or bundle, excluding executables) will be used. These are
  // limitations imposed by CMake use-cases - otherwise a lot of things break.
  // The rest will be linked using linker flags (OTHER_LDFLAGS setting in Xcode
  // project).
  std::map<std::string, std::vector<cmComputeLinkInformation::Item const*>>
    configItemMap;
  auto addToLinkerArguments =
    [&configItemMap](const std::string& configName,
                     cmComputeLinkInformation::Item const* libItemPtr) {
      auto& linkVector = configItemMap[configName];
      if (std::find_if(linkVector.begin(), linkVector.end(),
                       [libItemPtr](cmComputeLinkInformation::Item const* p) {
                         return p == libItemPtr;
                       }) == linkVector.end()) {
        linkVector.push_back(libItemPtr);
      }
    };
  std::vector<cmComputeLinkInformation::Item const*> linkPhaseTargetVector;
  std::map<std::string, std::vector<std::string>> targetConfigMap;
  using ConfigItemPair =
    std::pair<std::string, cmComputeLinkInformation::Item const*>;
  std::map<std::string, std::vector<ConfigItemPair>> targetItemMap;
  std::map<std::string, std::vector<std::string>> targetProductNameMap;
  bool useLinkPhase = false;
  bool forceLinkPhase = false;
  cmValue prop =
    target->GetTarget()->GetProperty("XCODE_LINK_BUILD_PHASE_MODE");
  if (prop) {
    if (*prop == "BUILT_ONLY"_s) {
      useLinkPhase = true;
    } else if (*prop == "KNOWN_LOCATION"_s) {
      useLinkPhase = true;
      forceLinkPhase = true;
    } else if (*prop != "NONE"_s) {
      cmSystemTools::Error(
        cmStrCat("Invalid value for XCODE_LINK_BUILD_PHASE_MODE: ", *prop));
      return;
    }
  }
  for (auto const& configName : this->CurrentConfigurationTypes) {
    cmComputeLinkInformation* cli = gt->GetLinkInformation(configName);
    if (!cli) {
      continue;
    }
    for (auto const& libItem : cli->GetItems()) {
      // Explicitly ignore OBJECT libraries as Xcode emulates them as static
      // libraries without an artifact. Avoid exposing this to the rest of
      // CMake's compilation model.
      if (libItem.Target &&
          libItem.Target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
        continue;
      }
      // We want to put only static libraries, dynamic libraries, frameworks
      // and bundles that are built from targets that are not imported in "Link
      // Binary With Libraries" build phase. Except if the target property
      // XCODE_LINK_BUILD_PHASE_MODE is KNOWN_LOCATION then all imported and
      // non-target libraries will be added as well.
      if (useLinkPhase &&
          (gt->GetType() == cmStateEnums::EXECUTABLE ||
           gt->GetType() == cmStateEnums::SHARED_LIBRARY ||
           gt->GetType() == cmStateEnums::MODULE_LIBRARY) &&
          ((libItem.Target &&
            (!libItem.Target->IsImported() || forceLinkPhase) &&
            (libItem.Target->GetType() == cmStateEnums::STATIC_LIBRARY ||
             libItem.Target->GetType() == cmStateEnums::SHARED_LIBRARY ||
             libItem.Target->GetType() == cmStateEnums::MODULE_LIBRARY ||
             libItem.Target->GetType() == cmStateEnums::UNKNOWN_LIBRARY)) ||
           (!libItem.Target &&
            libItem.IsPath == cmComputeLinkInformation::ItemIsPath::Yes &&
            forceLinkPhase))) {
        std::string libName;
        bool canUseLinkPhase = !libItem.HasFeature() ||
          libItem.GetFeatureName() == "__CMAKE_LINK_FRAMEWORK"_s ||
          libItem.GetFeatureName() == "FRAMEWORK"_s ||
          libItem.GetFeatureName() == "__CMAKE_LINK_XCFRAMEWORK"_s ||
          libItem.GetFeatureName() == "XCFRAMEWORK"_s ||
          libItem.GetFeatureName() == "WEAK_FRAMEWORK"_s ||
          libItem.GetFeatureName() == "WEAK_LIBRARY"_s;
        if (canUseLinkPhase) {
          if (libItem.Target) {
            if (libItem.Target->GetType() == cmStateEnums::UNKNOWN_LIBRARY) {
              canUseLinkPhase = canUseLinkPhase && forceLinkPhase;
            } else {
              // If a library target uses custom build output directory Xcode
              // won't pick it up so we have to resort back to linker flags,
              // but that's OK as long as the custom output dir is absolute
              // path.
              for (auto const& libConfigName :
                   this->CurrentConfigurationTypes) {
                canUseLinkPhase = canUseLinkPhase &&
                  libItem.Target->UsesDefaultOutputDir(
                    libConfigName, cmStateEnums::RuntimeBinaryArtifact);
              }
            }
            libName = libItem.Target->GetName();
          } else {
            libName = cmSystemTools::GetFilenameName(libItem.Value.Value);
            // We don't want all the possible files here, just standard
            // libraries
            const auto libExt = cmSystemTools::GetFilenameExtension(libName);
            if (!IsLinkPhaseLibraryExtension(libExt)) {
              canUseLinkPhase = false;
            }
          }
        }
        if (canUseLinkPhase) {
          // Add unique configuration name to target-config map for later
          // checks
          auto& configVector = targetConfigMap[libName];
          if (std::find(configVector.begin(), configVector.end(),
                        configName) == configVector.end()) {
            configVector.push_back(configName);
          }
          // Add a pair of config and item to target-item map
          auto& itemVector = targetItemMap[libName];
          itemVector.emplace_back(configName, &libItem);
          // Add product file-name to a lib-product map
          auto productName =
            cmSystemTools::GetFilenameName(libItem.Value.Value);
          auto& productVector = targetProductNameMap[libName];
          if (std::find(productVector.begin(), productVector.end(),
                        productName) == productVector.end()) {
            productVector.push_back(productName);
          }
          continue;
        }
      }
      // Add this library item to a regular linker flag list
      addToLinkerArguments(configName, &libItem);
    }
  }

  // Go through target library map and separate libraries that are linked
  // in all configurations and produce only single product, from the rest.
  // Only these will be linked through "Link Binary With Libraries" build
  // phase.
  for (auto const& targetLibConfigs : targetConfigMap) {
    // Add this library to "Link Binary With Libraries" build phase if it's
    // linked in all configurations and it has only one product name
    auto& itemVector = targetItemMap[targetLibConfigs.first];
    auto& productVector = targetProductNameMap[targetLibConfigs.first];
    if (targetLibConfigs.second == this->CurrentConfigurationTypes &&
        productVector.size() == 1) {
      // Add this library to "Link Binary With Libraries" list
      linkPhaseTargetVector.push_back(itemVector[0].second);
    } else {
      for (auto const& libItem : targetItemMap[targetLibConfigs.first]) {
        // Add this library item to a regular linker flag list
        addToLinkerArguments(libItem.first, libItem.second);
      }
    }
  }

  // Add libraries to "Link Binary With Libraries" build phase and collect
  // their search paths. Xcode does not support per-configuration linking
  // in this build phase so we don't have to do this for each configuration
  // separately.
  std::vector<std::string> linkSearchPaths;
  std::vector<std::string> frameworkSearchPaths;
  std::set<std::pair<cmXCodeObject*, std::string>> linkBuildFileSet;
  for (auto const& libItem : linkPhaseTargetVector) {
    // Add target output directory as a library search path
    std::string linkDir;
    if (libItem->Target) {
      linkDir = libItem->Target->GetLocationForBuild();
    } else {
      linkDir = libItem->Value.Value;
    }
    if (cmHasSuffix(libItem->GetFeatureName(), "FRAMEWORK"_s)) {
      auto fwDescriptor = this->SplitFrameworkPath(
        linkDir, cmGlobalGenerator::FrameworkFormat::Extended);
      if (fwDescriptor && !fwDescriptor->Directory.empty()) {
        linkDir = fwDescriptor->Directory;
        if (std::find(frameworkSearchPaths.begin(), frameworkSearchPaths.end(),
                      linkDir) == frameworkSearchPaths.end()) {
          frameworkSearchPaths.push_back(linkDir);
        }
      }
    } else {
      linkDir = cmSystemTools::GetParentDirectory(linkDir);
      if (std::find(linkSearchPaths.begin(), linkSearchPaths.end(), linkDir) ==
          linkSearchPaths.end()) {
        linkSearchPaths.push_back(linkDir);
      }
    }

    if (libItem->Target && !libItem->Target->IsImported()) {
      for (auto const& configName : this->CurrentConfigurationTypes) {
        target->AddDependTarget(configName, libItem->Target->GetName());
      }
    }
    // Get the library target
    auto* libTarget = FindXCodeTarget(libItem->Target);
    cmXCodeObject* buildFile;
    if (!libTarget) {
      if (libItem->IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
        // Get or create a direct file ref in the root project
        auto cleanPath = libItem->Value.Value;
        if (cmSystemTools::FileIsFullPath(cleanPath)) {
          // Some arguments are reported as paths, but they are actually not,
          // so we can't collapse them, and neither can we collapse relative
          // paths
          cleanPath = cmSystemTools::CollapseFullPath(cleanPath);
        }
        auto it = this->ExternalLibRefs.find(cleanPath);
        if (it == this->ExternalLibRefs.end()) {
          buildFile = CreateXCodeBuildFileFromPath(cleanPath, gt, "", nullptr);
          if (!buildFile) {
            // Add this library item back to a regular linker flag list
            for (const auto& conf : configItemMap) {
              addToLinkerArguments(conf.first, libItem);
            }
            continue;
          }
          this->ExternalLibRefs.emplace(cleanPath, buildFile);
        } else {
          buildFile = it->second;
        }
      } else {
        // Add this library item back to a regular linker flag list
        for (const auto& conf : configItemMap) {
          addToLinkerArguments(conf.first, libItem);
        }
        continue;
      }
    } else {
      // Add the target output file as a build reference for other targets
      // to link against
      auto* fileRefObject = libTarget->GetAttribute("productReference");
      if (!fileRefObject) {
        // Add this library item back to a regular linker flag list
        for (const auto& conf : configItemMap) {
          addToLinkerArguments(conf.first, libItem);
        }
        continue;
      }
      auto it = FileRefToBuildFileMap.find(fileRefObject);
      if (it == FileRefToBuildFileMap.end()) {
        buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
        buildFile->AddAttribute("fileRef", fileRefObject);
        FileRefToBuildFileMap[fileRefObject] = buildFile;
      } else {
        buildFile = it->second;
      }
    }
    // Add this reference to current target
    auto* buildPhases = target->GetAttribute("buildPhases");
    if (!buildPhases) {
      cmSystemTools::Error("Missing buildPhase of target");
      continue;
    }
    auto* frameworkBuildPhase =
      buildPhases->GetObject(cmXCodeObject::PBXFrameworksBuildPhase);
    if (!frameworkBuildPhase) {
      cmSystemTools::Error("Missing PBXFrameworksBuildPhase of buildPhase");
      continue;
    }
    auto* buildFiles = frameworkBuildPhase->GetAttribute("files");
    if (!buildFiles) {
      cmSystemTools::Error("Missing files of PBXFrameworksBuildPhase");
      continue;
    }
    if (buildFile) {
      if (cmHasPrefix(libItem->GetFeatureName(), "WEAK_"_s)) {
        auto key = std::make_pair(buildFile->GetAttribute("fileRef"),
                                  libItem->GetFeatureName());
        if (linkBuildFileSet.find(key) != linkBuildFileSet.end()) {
          continue;
        }
        linkBuildFileSet.insert(key);

        cmXCodeObject* buildObject =
          this->CreateObject(cmXCodeObject::PBXBuildFile);
        buildObject->AddAttribute("fileRef", key.first);
        // Add settings, ATTRIBUTES, Weak flag
        cmXCodeObject* settings =
          this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
        cmXCodeObject* attrs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
        attrs->AddObject(this->CreateString("Weak"));
        settings->AddAttribute("ATTRIBUTES", attrs);
        buildObject->AddAttribute("settings", settings);
        buildFile = buildObject;
      }
      if (!buildFiles->HasObject(buildFile)) {
        buildFiles->AddObject(buildFile);
      }
    }
  }

  // Loop over configuration types and set per-configuration info.
  for (auto const& configName : this->CurrentConfigurationTypes) {
    {
      // Add object library contents as link flags.
      BuildObjectListOrString libSearchPaths(this, true);
      std::vector<cmSourceFile const*> objs;
      gt->GetExternalObjects(objs, configName);
      for (auto const* sourceFile : objs) {
        if (sourceFile->GetObjectLibrary().empty()) {
          continue;
        }
        libSearchPaths.Add(this->XCodeEscapePath(sourceFile->GetFullPath()));
      }
      this->AppendBuildSettingAttribute(
        target, this->GetTargetLinkFlagsVar(gt), libSearchPaths.CreateList(),
        configName);
    }

    // Compute the link library and directory information.
    cmComputeLinkInformation* cli = gt->GetLinkInformation(configName);
    if (!cli) {
      continue;
    }

    // add .xcframework include paths
    {
      // Keep track of framework search paths we've already added or that are
      // part of the set of implicit search paths. We don't want to repeat
      // them and we also need to avoid hard-coding any SDK-specific paths.
      // This is essential for getting device-and-simulator builds to work,
      // otherwise we end up hard-coding a path to the wrong SDK for
      // SDK-provided frameworks that are added by their full path.
      std::set<std::string> emitted(cli->GetFrameworkPathsEmitted());
      BuildObjectListOrString includePaths(this, true);
      BuildObjectListOrString fwSearchPaths(this, true);
      for (auto const& libItem : configItemMap[configName]) {
        auto const& libName = *libItem;
        if (libName.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
          auto cleanPath = libName.Value.Value;
          if (cmSystemTools::FileIsFullPath(cleanPath)) {
            cleanPath = cmSystemTools::CollapseFullPath(cleanPath);
          }
          bool isXcFramework =
            cmHasSuffix(libName.GetFeatureName(), "XCFRAMEWORK"_s);
          if (isXcFramework) {
            auto plist = cmParseXcFrameworkPlist(
              cleanPath, *this->Makefiles.front(), libName.Value.Backtrace);
            if (!plist) {
              return;
            }
            if (auto const* library = plist->SelectSuitableLibrary(
                  *this->Makefiles.front(), libName.Value.Backtrace)) {
              auto libraryPath =
                cmStrCat(cleanPath, '/', library->LibraryIdentifier, '/',
                         library->LibraryPath);
              if (auto const fwDescriptor = this->SplitFrameworkPath(
                    libraryPath,
                    cmGlobalGenerator::FrameworkFormat::Relaxed)) {
                if (!fwDescriptor->Directory.empty() &&
                    emitted.insert(fwDescriptor->Directory).second) {
                  // This is a search path we had not added before and it
                  // isn't an implicit search path, so we need it
                  fwSearchPaths.Add(
                    this->XCodeEscapePath(fwDescriptor->Directory));
                }
              } else {
                if (!library->HeadersPath.empty()) {
                  includePaths.Add(this->XCodeEscapePath(
                    cmStrCat(cleanPath, '/', library->LibraryIdentifier, '/',
                             library->HeadersPath)));
                }
              }
            } else {
              return;
            }
          }
        }
      }
      if (!includePaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target, "HEADER_SEARCH_PATHS",
                                          includePaths.CreateList(),
                                          configName);
      }
      if (!fwSearchPaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target, "FRAMEWORK_SEARCH_PATHS",
                                          fwSearchPaths.CreateList(),
                                          configName);
      }
    }

    // Skip link information for object libraries.
    if (gt->GetType() == cmStateEnums::OBJECT_LIBRARY ||
        gt->GetType() == cmStateEnums::STATIC_LIBRARY) {
      continue;
    }

    // Add dependencies directly on library files.
    for (auto const& libDep : cli->GetDepends()) {
      target->AddDependLibrary(configName, libDep);
    }

    // add the library search paths
    {
      BuildObjectListOrString libSearchPaths(this, true);

      std::string linkDirs;
      for (auto const& libDir : cli->GetDirectories()) {
        if (!libDir.empty() && libDir != "/usr/lib"_s) {
          cmPolicies::PolicyStatus cmp0142 =
            target->GetTarget()->GetPolicyStatusCMP0142();
          if (cmp0142 == cmPolicies::OLD || cmp0142 == cmPolicies::WARN) {
            libSearchPaths.Add(this->XCodeEscapePath(cmStrCat(
              libDir, "/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)")));
          }
          libSearchPaths.Add(this->XCodeEscapePath(libDir));
        }
      }

      // Add previously collected paths where to look for libraries
      // that were added to "Link Binary With Libraries"
      for (auto& libDir : linkSearchPaths) {
        libSearchPaths.Add(this->XCodeEscapePath(libDir));
      }
      if (!libSearchPaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target, "LIBRARY_SEARCH_PATHS",
                                          libSearchPaths.CreateList(),
                                          configName);
      }
    }

    // add framework search paths
    {
      BuildObjectListOrString fwSearchPaths(this, true);
      // Add previously collected paths where to look for frameworks
      // that were added to "Link Binary With Libraries"
      for (auto& fwDir : frameworkSearchPaths) {
        fwSearchPaths.Add(this->XCodeEscapePath(fwDir));
      }
      if (!fwSearchPaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target, "FRAMEWORK_SEARCH_PATHS",
                                          fwSearchPaths.CreateList(),
                                          configName);
      }
    }

    // now add the left-over link libraries
    {
      // Keep track of framework search paths we've already added or that are
      // part of the set of implicit search paths. We don't want to repeat
      // them and we also need to avoid hard-coding any SDK-specific paths.
      // This is essential for getting device-and-simulator builds to work,
      // otherwise we end up hard-coding a path to the wrong SDK for
      // SDK-provided frameworks that are added by their full path.
      std::set<std::string> emitted(cli->GetFrameworkPathsEmitted());
      BuildObjectListOrString libPaths(this, true);
      BuildObjectListOrString fwSearchPaths(this, true);
      for (auto const& libItem : configItemMap[configName]) {
        auto const& libName = *libItem;
        if (libName.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
          auto cleanPath = libName.Value.Value;
          if (cmSystemTools::FileIsFullPath(cleanPath)) {
            cleanPath = cmSystemTools::CollapseFullPath(cleanPath);
          }
          bool isXcFramework =
            cmHasSuffix(libName.GetFeatureName(), "XCFRAMEWORK"_s);
          bool isFramework = !isXcFramework &&
            cmHasSuffix(libName.GetFeatureName(), "FRAMEWORK"_s);
          if (isFramework) {
            const auto fwDescriptor = this->SplitFrameworkPath(
              cleanPath, cmGlobalGenerator::FrameworkFormat::Extended);
            if (isFramework && !fwDescriptor->Directory.empty() &&
                emitted.insert(fwDescriptor->Directory).second) {
              // This is a search path we had not added before and it isn't
              // an implicit search path, so we need it
              fwSearchPaths.Add(
                this->XCodeEscapePath(fwDescriptor->Directory));
            }
            if (libName.GetFeatureName() == "__CMAKE_LINK_FRAMEWORK"_s) {
              // use the full path
              libPaths.Add(
                libName.GetFormattedItem(this->XCodeEscapePath(cleanPath))
                  .Value);
            } else {
              libPaths.Add(libName
                             .GetFormattedItem(this->XCodeEscapePath(
                               fwDescriptor->GetLinkName()))
                             .Value);
            }
          } else if (isXcFramework) {
            auto plist = cmParseXcFrameworkPlist(
              cleanPath, *this->Makefiles.front(), libName.Value.Backtrace);
            if (!plist) {
              return;
            }
            if (auto const* library = plist->SelectSuitableLibrary(
                  *this->Makefiles.front(), libName.Value.Backtrace)) {
              auto libraryPath =
                cmStrCat(cleanPath, '/', library->LibraryIdentifier, '/',
                         library->LibraryPath);
              if (auto const fwDescriptor = this->SplitFrameworkPath(
                    libraryPath,
                    cmGlobalGenerator::FrameworkFormat::Relaxed)) {
                libPaths.Add(cmStrCat(
                  "-framework ",
                  this->XCodeEscapePath(fwDescriptor->GetLinkName())));
              } else {
                libPaths.Add(
                  libName.GetFormattedItem(this->XCodeEscapePath(libraryPath))
                    .Value);
              }
            } else {
              return;
            }
          } else {
            libPaths.Add(
              libName.GetFormattedItem(this->XCodeEscapePath(cleanPath))
                .Value);
          }
          if ((!libName.Target || libName.Target->IsImported()) &&
              (isFramework || isXcFramework ||
               IsLinkPhaseLibraryExtension(cleanPath))) {
            // Create file reference for embedding
            auto it = this->ExternalLibRefs.find(cleanPath);
            if (it == this->ExternalLibRefs.end()) {
              auto* buildFile =
                this->CreateXCodeBuildFileFromPath(cleanPath, gt, "", nullptr);
              if (buildFile) {
                this->ExternalLibRefs.emplace(cleanPath, buildFile);
              }
            }
          }
        } else if (!libName.Target ||
                   libName.Target->GetType() !=
                     cmStateEnums::INTERFACE_LIBRARY) {
          libPaths.Add(libName.Value.Value);
        }
        if (libName.Target && !libName.Target->IsImported()) {
          target->AddDependTarget(configName, libName.Target->GetName());
        }
      }
      if (!libPaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target,
                                          this->GetTargetLinkFlagsVar(gt),
                                          libPaths.CreateList(), configName);
      }
      if (!fwSearchPaths.IsEmpty()) {
        this->AppendBuildSettingAttribute(target, "FRAMEWORK_SEARCH_PATHS",
                                          fwSearchPaths.CreateList(),
                                          configName);
      }
    }
  }
}

void cmGlobalXCodeGenerator::AddEmbeddedObjects(
  cmXCodeObject* target, const std::string& copyFilesBuildPhaseName,
  const std::string& embedPropertyName, const std::string& dstSubfolderSpec,
  int actionsOnByDefault, const std::string& defaultDstPath)
{
  cmGeneratorTarget* gt = target->GetTarget();
  if (!gt) {
    cmSystemTools::Error("Error no target on xobject\n");
    return;
  }
  if (!gt->IsInBuildSystem()) {
    return;
  }
  bool isFrameworkTarget = gt->IsFrameworkOnApple();
  bool isBundleTarget = gt->GetPropertyAsBool("MACOSX_BUNDLE");
  bool isCFBundleTarget = gt->IsCFBundleOnApple();
  if (!(isFrameworkTarget || isBundleTarget || isCFBundleTarget)) {
    return;
  }
  cmValue files = gt->GetProperty(embedPropertyName);
  if (!files) {
    return;
  }

  // Create an "Embedded Frameworks" build phase
  auto* copyFilesBuildPhase =
    this->CreateObject(cmXCodeObject::PBXCopyFilesBuildPhase);
  copyFilesBuildPhase->SetComment(copyFilesBuildPhaseName);
  copyFilesBuildPhase->AddAttribute("buildActionMask",
                                    this->CreateString("2147483647"));
  copyFilesBuildPhase->AddAttribute("dstSubfolderSpec",
                                    this->CreateString(dstSubfolderSpec));
  copyFilesBuildPhase->AddAttribute(
    "name", this->CreateString(copyFilesBuildPhaseName));
  if (cmValue fwEmbedPath =
        gt->GetProperty(cmStrCat(embedPropertyName, "_PATH"))) {
    copyFilesBuildPhase->AddAttribute("dstPath",
                                      this->CreateString(*fwEmbedPath));
  } else {
    copyFilesBuildPhase->AddAttribute("dstPath",
                                      this->CreateString(defaultDstPath));
  }
  copyFilesBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                    this->CreateString("0"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  // Collect all embedded frameworks and dylibs and add them to build phase
  cmList relFiles{ *files };
  for (std::string const& relFile : relFiles) {
    cmXCodeObject* buildFile{ nullptr };
    std::string filePath = relFile;
    auto* genTarget = this->FindGeneratorTarget(relFile);
    if (genTarget) {
      // This is a target - get it's product path reference
      auto* xcTarget = this->FindXCodeTarget(genTarget);
      if (!xcTarget) {
        cmSystemTools::Error(
          cmStrCat("Can not find a target for ", genTarget->GetName()));
        continue;
      }
      // Add the target output file as a build reference for other targets
      // to link against
      auto* fileRefObject = xcTarget->GetAttribute("productReference");
      if (!fileRefObject) {
        cmSystemTools::Error(cmStrCat("Target ", genTarget->GetName(),
                                      " is missing product reference"));
        continue;
      }
      auto it = this->FileRefToEmbedBuildFileMap.find(fileRefObject);
      if (it == this->FileRefToEmbedBuildFileMap.end()) {
        buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
        buildFile->AddAttribute("fileRef", fileRefObject);
        this->FileRefToEmbedBuildFileMap[fileRefObject] = buildFile;
      } else {
        buildFile = it->second;
      }
    } else if (cmSystemTools::IsPathToFramework(relFile) ||
               cmSystemTools::IsPathToMacOSSharedLibrary(relFile) ||
               cmSystemTools::FileIsDirectory(filePath)) {
      // This is a regular string path - create file reference
      auto it = this->EmbeddedLibRefs.find(relFile);
      if (it == this->EmbeddedLibRefs.end()) {
        cmXCodeObject* fileRef =
          this->CreateXCodeFileReferenceFromPath(relFile, gt, "", nullptr);
        if (fileRef) {
          buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
          buildFile->SetComment(fileRef->GetComment());
          buildFile->AddAttribute("fileRef",
                                  this->CreateObjectReference(fileRef));
        }
        if (!buildFile) {
          cmSystemTools::Error(
            cmStrCat("Can't create build file for ", relFile));
          continue;
        }
        this->EmbeddedLibRefs.emplace(filePath, buildFile);
      } else {
        buildFile = it->second;
      }
    }
    if (!buildFile) {
      cmSystemTools::Error(cmStrCat("Can't find a build file for ", relFile));
      continue;
    }
    // Set build file configuration
    cmXCodeObject* settings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    cmXCodeObject* attrs = this->CreateObject(cmXCodeObject::OBJECT_LIST);

    bool removeHeaders = actionsOnByDefault & RemoveHeadersOnCopyByDefault;
    if (auto prop = gt->GetProperty(
          cmStrCat(embedPropertyName, "_REMOVE_HEADERS_ON_COPY"))) {
      removeHeaders = cmIsOn(*prop);
    }
    if (removeHeaders) {
      attrs->AddObject(this->CreateString("RemoveHeadersOnCopy"));
    }

    bool codeSign = actionsOnByDefault & CodeSignOnCopyByDefault;
    if (auto prop =
          gt->GetProperty(cmStrCat(embedPropertyName, "_CODE_SIGN_ON_COPY"))) {
      codeSign = cmIsOn(*prop);
    }
    if (codeSign) {
      attrs->AddObject(this->CreateString("CodeSignOnCopy"));
    }

    settings->AddAttributeIfNotEmpty("ATTRIBUTES", attrs);
    buildFile->AddAttributeIfNotEmpty("settings", settings);
    if (!buildFiles->HasObject(buildFile)) {
      buildFiles->AddObject(buildFile);
    }
  }
  copyFilesBuildPhase->AddAttribute("files", buildFiles);
  auto* buildPhases = target->GetAttribute("buildPhases");
  // Embed-something build phases must be inserted before the post-build
  // command because that command is expected to be last
  buildPhases->InsertObject(buildPhases->GetObjectCount() - 1,
                            copyFilesBuildPhase);
}

void cmGlobalXCodeGenerator::AddEmbeddedFrameworks(cmXCodeObject* target)
{
  static auto const* const dstSubfolderSpec = "10";

  // Despite the name, by default Xcode uses "Embed Frameworks" build phase
  // for both frameworks and dynamic libraries
  this->AddEmbeddedObjects(target, "Embed Frameworks",
                           "XCODE_EMBED_FRAMEWORKS", dstSubfolderSpec,
                           NoActionOnCopyByDefault);
}

void cmGlobalXCodeGenerator::AddEmbeddedPlugIns(cmXCodeObject* target)
{
  static auto const* const dstSubfolderSpec = "13";

  this->AddEmbeddedObjects(target, "Embed PlugIns", "XCODE_EMBED_PLUGINS",
                           dstSubfolderSpec, NoActionOnCopyByDefault);
}

void cmGlobalXCodeGenerator::AddEmbeddedAppExtensions(cmXCodeObject* target)
{
  static auto const* const dstSubfolderSpec = "13";

  this->AddEmbeddedObjects(target, "Embed App Extensions",
                           "XCODE_EMBED_APP_EXTENSIONS", dstSubfolderSpec,
                           RemoveHeadersOnCopyByDefault);
}

void cmGlobalXCodeGenerator::AddEmbeddedExtensionKitExtensions(
  cmXCodeObject* target)
{
  static auto const* const dstSubfolderSpec = "16";

  this->AddEmbeddedObjects(target, "Embed App Extensions",
                           "XCODE_EMBED_EXTENSIONKIT_EXTENSIONS",
                           dstSubfolderSpec, RemoveHeadersOnCopyByDefault,
                           "$(EXTENSIONS_FOLDER_PATH)");
}

void cmGlobalXCodeGenerator::AddEmbeddedResources(cmXCodeObject* target)
{
  static const auto dstSubfolderSpec = "7";

  this->AddEmbeddedObjects(target, "Embed Resources", "XCODE_EMBED_RESOURCES",
                           dstSubfolderSpec, NoActionOnCopyByDefault);
}

bool cmGlobalXCodeGenerator::CreateGroups(
  std::vector<cmLocalGenerator*>& generators)
{
  for (auto& generator : generators) {
    cmMakefile* mf = generator->GetMakefile();
    std::vector<cmSourceGroup> sourceGroups = mf->GetSourceGroups();
    for (const auto& gtgt : generator->GetGeneratorTargets()) {
      // Same skipping logic here as in CreateXCodeTargets so that we do not
      // end up with (empty anyhow) ZERO_CHECK, install, or test source
      // groups:
      //
      if (!gtgt->IsInBuildSystem() ||
          gtgt->GetType() == cmStateEnums::GLOBAL_TARGET ||
          gtgt->GetName() == CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
        continue;
      }

      auto addSourceToGroup = [this, mf, &gtgt,
                               &sourceGroups](std::string const& source) {
        cmSourceGroup* sourceGroup = mf->FindSourceGroup(source, sourceGroups);
        cmXCodeObject* pbxgroup =
          this->CreateOrGetPBXGroup(gtgt.get(), sourceGroup);
        std::string key = GetGroupMapKeyFromPath(gtgt.get(), source);
        this->GroupMap[key] = pbxgroup;
      };

      // Put cmSourceFile instances in proper groups:
      for (auto const& si : gtgt->GetAllConfigSources()) {
        cmSourceFile const* sf = si.Source;
        if (!sf->GetObjectLibrary().empty()) {
          // Object library files go on the link line instead.
          continue;
        }
        addSourceToGroup(sf->GetFullPath());
      }

      // Add CMakeLists.txt file for user convenience.
      {
        std::string listfile =
          cmStrCat(gtgt->GetLocalGenerator()->GetCurrentSourceDirectory(),
                   "/CMakeLists.txt");
        cmSourceFile* sf = gtgt->Makefile->GetOrCreateSource(
          listfile, false, cmSourceFileLocationKind::Known);
        addSourceToGroup(sf->ResolveFullPath());
      }

      // Add the Info.plist we are about to generate for an App Bundle.
      if (gtgt->GetPropertyAsBool("MACOSX_BUNDLE")) {
        std::string plist = this->ComputeInfoPListLocation(gtgt.get());
        cmSourceFile* sf = gtgt->Makefile->GetOrCreateSource(
          plist, true, cmSourceFileLocationKind::Known);
        addSourceToGroup(sf->ResolveFullPath());
      }
    }
  }
  return true;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreatePBXGroup(cmXCodeObject* parent,
                                                      const std::string& name)
{
  cmXCodeObject* parentChildren = nullptr;
  if (parent) {
    parentChildren = parent->GetAttribute("children");
  }
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::PBXGroup);
  cmXCodeObject* groupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  group->AddAttribute("name", this->CreateString(name));
  group->AddAttribute("children", groupChildren);
  group->AddAttribute("sourceTree", this->CreateString("<group>"));
  if (parentChildren) {
    parentChildren->AddObject(group);
  }
  return group;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateOrGetPBXGroup(
  cmGeneratorTarget* gtgt, cmSourceGroup* sg)
{
  std::string s;
  std::string target;
  const std::string targetFolder = gtgt->GetEffectiveFolderName();
  if (!targetFolder.empty()) {
    target = cmStrCat(targetFolder, '/');
  }
  target += gtgt->GetName();
  s = cmStrCat(target, '/', sg->GetFullName());
  auto it = this->GroupNameMap.find(s);
  if (it != this->GroupNameMap.end()) {
    return it->second;
  }

  it = this->TargetGroup.find(target);
  cmXCodeObject* tgroup = nullptr;
  if (it != this->TargetGroup.end()) {
    tgroup = it->second;
  } else {
    std::vector<std::string> tgt_folders = cmTokenize(target, "/");
    std::string curr_tgt_folder;
    for (std::vector<std::string>::size_type i = 0; i < tgt_folders.size();
         i++) {
      if (i != 0) {
        curr_tgt_folder += '/';
      }
      curr_tgt_folder += tgt_folders[i];
      it = this->TargetGroup.find(curr_tgt_folder);
      if (it != this->TargetGroup.end()) {
        tgroup = it->second;
        continue;
      }
      tgroup = this->CreatePBXGroup(tgroup, tgt_folders[i]);
      this->TargetGroup[curr_tgt_folder] = tgroup;
      if (i == 0) {
        this->MainGroupChildren->AddObject(tgroup);
      }
    }
  }
  this->TargetGroup[target] = tgroup;

  // If it's the default source group (empty name) then put the source file
  // directly in the tgroup...
  //
  if (sg->GetFullName().empty()) {
    this->GroupNameMap[s] = tgroup;
    return tgroup;
  }

  // It's a recursive folder structure, let's find the real parent group
  if (sg->GetFullName() != sg->GetName()) {
    std::string curr_folder = cmStrCat(target, '/');
    for (auto const& folder : cmTokenize(sg->GetFullName(), "\\")) {
      curr_folder += folder;
      auto const i_folder = this->GroupNameMap.find(curr_folder);
      // Create new folder
      if (i_folder == this->GroupNameMap.end()) {
        cmXCodeObject* group = this->CreatePBXGroup(tgroup, folder);
        this->GroupNameMap[curr_folder] = group;
        tgroup = group;
      } else {
        tgroup = i_folder->second;
      }
      curr_folder += '\\';
    }
    return tgroup;
  }
  cmXCodeObject* group = this->CreatePBXGroup(tgroup, sg->GetName());
  this->GroupNameMap[s] = group;
  return group;
}

bool cmGlobalXCodeGenerator::CreateXCodeObjects(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  this->ClearXCodeObjects();
  this->RootObject = nullptr;
  this->MainGroupChildren = nullptr;
  this->FrameworkGroup = nullptr;
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for (const std::string& CurrentConfigurationType :
       this->CurrentConfigurationTypes) {
    cmXCodeObject* buildStyle =
      this->CreateObject(cmXCodeObject::PBXBuildStyle);
    const std::string& name = CurrentConfigurationType;
    buildStyle->AddAttribute("name", this->CreateString(name));
    buildStyle->SetComment(name);
    cmXCodeObject* sgroup = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    sgroup->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
    buildStyle->AddAttribute("buildSettings", sgroup);
    listObjs->AddObject(buildStyle);
  }

  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->MainGroupChildren = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", this->MainGroupChildren);
  mainGroup->AddAttribute("sourceTree", this->CreateString("<group>"));

  // now create the cmake groups
  if (!this->CreateGroups(generators)) {
    return false;
  }

  cmXCodeObject* productGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  productGroup->AddAttribute("name", this->CreateString("Products"));
  productGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  cmXCodeObject* productGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  productGroup->AddAttribute("children", productGroupChildren);
  this->MainGroupChildren->AddObject(productGroup);

  this->FrameworkGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->FrameworkGroup->AddAttribute("name", this->CreateString("Frameworks"));
  this->FrameworkGroup->AddAttribute("sourceTree",
                                     this->CreateString("<group>"));
  cmXCodeObject* frameworkGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  this->FrameworkGroup->AddAttribute("children", frameworkGroupChildren);
  this->MainGroupChildren->AddObject(this->FrameworkGroup);

  this->ResourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->ResourcesGroup->AddAttribute("name", this->CreateString("Resources"));
  this->ResourcesGroup->AddAttribute("sourceTree",
                                     this->CreateString("<group>"));
  cmXCodeObject* ResourcesGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  this->ResourcesGroup->AddAttribute("children", ResourcesGroupChildren);
  this->MainGroupChildren->AddObject(this->ResourcesGroup);

  this->RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  this->RootObject->SetComment("Project object");

  std::string project_id = cmStrCat("PROJECT_", root->GetProjectName());
  this->RootObject->SetId(
    this->GetOrCreateId(project_id, this->RootObject->GetId()));

  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  this->RootObject->AddAttribute("mainGroup",
                                 this->CreateObjectReference(mainGroup));
  this->RootObject->AddAttribute("buildSettings", group);
  this->RootObject->AddAttribute("buildStyles", listObjs);
  this->RootObject->AddAttribute("hasScannedForEncodings",
                                 this->CreateString("0"));
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("BuildIndependentTargetsInParallel",
                      this->CreateString("YES"));
  std::ostringstream v;
  v << std::setfill('0') << std::setw(4) << XcodeVersion * 10;
  group->AddAttribute("LastUpgradeCheck", this->CreateString(v.str()));
  this->RootObject->AddAttribute("attributes", group);
  this->RootObject->AddAttribute("compatibilityVersion",
                                 this->CreateString("Xcode 3.2"));
  // Point Xcode at the top of the source tree.
  {
    std::string pdir =
      this->RelativeToBinary(root->GetCurrentSourceDirectory());
    this->RootObject->AddAttribute("projectDirPath", this->CreateString(pdir));
    this->RootObject->AddAttribute("projectRoot", this->CreateString(""));
  }
  cmXCodeObject* configlist =
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  using Configs = std::vector<std::pair<std::string, cmXCodeObject*>>;
  Configs configs;
  std::string defaultConfigName;
  for (const auto& name : this->CurrentConfigurationTypes) {
    if (defaultConfigName.empty()) {
      defaultConfigName = name;
    }
    cmXCodeObject* config =
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    config->AddAttribute("name", this->CreateString(name));
    configs.emplace_back(name, config);
  }
  if (defaultConfigName.empty()) {
    defaultConfigName = "Debug";
  }
  for (auto& config : configs) {
    buildConfigurations->AddObject(config.second);
  }
  configlist->AddAttribute("buildConfigurations", buildConfigurations);

  std::string comment = cmStrCat("Build configuration list for PBXProject \"",
                                 this->CurrentProject, '"');
  configlist->SetComment(comment);
  configlist->AddAttribute("defaultConfigurationIsVisible",
                           this->CreateString("0"));
  configlist->AddAttribute("defaultConfigurationName",
                           this->CreateString(defaultConfigName));
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  cmValue sysroot = this->CurrentMakefile->GetDefinition("CMAKE_OSX_SYSROOT");
  cmValue deploymentTarget =
    this->CurrentMakefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
  if (sysroot) {
    buildSettings->AddAttribute("SDKROOT", this->CreateString(*sysroot));
  }
  // recompute this as it may have been changed since enable language
  this->ComputeArchitectures(this->CurrentMakefile);
  std::string const archs = cmJoin(this->Architectures, " ");
  if (archs.empty()) {
    // Tell Xcode to use NATIVE_ARCH instead of ARCHS.
    buildSettings->AddAttribute("ONLY_ACTIVE_ARCH", this->CreateString("YES"));
    // When targeting macOS, use only the host architecture.
    if (this->SystemName == "Darwin"_s &&
        (!cmNonempty(sysroot) ||
         cmSystemTools::LowerCase(*sysroot).find("macos") !=
           std::string::npos)) {
      buildSettings->AddAttribute("ARCHS",
                                  this->CreateString("$(NATIVE_ARCH_ACTUAL)"));
    }
  } else {
    // Tell Xcode to use ARCHS (ONLY_ACTIVE_ARCH defaults to NO).
    buildSettings->AddAttribute("ARCHS", this->CreateString(archs));
  }
  if (cmNonempty(deploymentTarget)) {
    buildSettings->AddAttribute(GetDeploymentPlatform(root->GetMakefile()),
                                this->CreateString(*deploymentTarget));
  }
  if (!this->GeneratorToolset.empty()) {
    buildSettings->AddAttribute("GCC_VERSION",
                                this->CreateString(this->GeneratorToolset));
  }
  if (this->GetLanguageEnabled("Swift")) {
    std::string swiftVersion;
    if (cmValue vers = this->CurrentMakefile->GetDefinition(
          "CMAKE_Swift_LANGUAGE_VERSION")) {
      swiftVersion = *vers;
    } else if (this->XcodeVersion >= 102) {
      swiftVersion = "4.0";
    } else if (this->XcodeVersion >= 83) {
      swiftVersion = "3.0";
    } else {
      swiftVersion = "2.3";
    }
    buildSettings->AddAttribute("SWIFT_VERSION",
                                this->CreateString(swiftVersion));
  }

  std::string const symroot = this->GetSymrootDir();
  buildSettings->AddAttribute("SYMROOT", this->CreateString(symroot));

  // Inside a try_compile project, do not require signing on any platform.
  if (this->CMakeInstance->GetIsInTryCompile()) {
    buildSettings->AddAttribute("CODE_SIGNING_ALLOWED",
                                this->CreateString("NO"));
  }
  auto debugConfigs = this->GetCMakeInstance()->GetDebugConfigs();
  std::set<std::string> debugConfigSet(debugConfigs.begin(),
                                       debugConfigs.end());

  for (auto& config : configs) {
    CreateGlobalXCConfigSettings(root, config.second, config.first);

    cmXCodeObject* buildSettingsForCfg = this->CreateFlatClone(buildSettings);

    if (debugConfigSet.count(cmSystemTools::UpperCase(config.first)) == 0) {
      buildSettingsForCfg->AddAttribute("SWIFT_COMPILATION_MODE",
                                        this->CreateString("wholemodule"));
    }

    // Put this last so it can override existing settings
    // Convert "CMAKE_XCODE_ATTRIBUTE_*" variables directly.
    for (const auto& var : this->CurrentMakefile->GetDefinitions()) {
      if (cmHasLiteralPrefix(var, "CMAKE_XCODE_ATTRIBUTE_")) {
        std::string attribute = var.substr(22);
        this->FilterConfigurationAttribute(config.first, attribute);
        if (!attribute.empty()) {
          std::string processed = cmGeneratorExpression::Evaluate(
            this->CurrentMakefile->GetSafeDefinition(var),
            this->CurrentLocalGenerator, config.first);
          buildSettingsForCfg->AddAttribute(attribute,
                                            this->CreateString(processed));
        }
      }
    }
    // store per-config buildSettings into configuration object
    config.second->AddAttribute("buildSettings", buildSettingsForCfg);
  }

  this->RootObject->AddAttribute("buildConfigurationList",
                                 this->CreateObjectReference(configlist));

  std::vector<cmXCodeObject*> targets;
  for (auto& generator : generators) {
    if (!this->CreateXCodeTargets(generator, targets)) {
      return false;
    }
    for (auto const& ccRoot : this->CustomCommandRoots) {
      if (ccRoot.second.size() > 1) {
        std::string e = "The custom command ";
        std::vector<std::string> const& outputs =
          ccRoot.first->GetCustomCommand()->GetOutputs();
        if (!outputs.empty()) {
          e = cmStrCat(e, "generating\n  ", outputs[0]);
        } else {
          e = cmStrCat(e, "driven by\n  ", ccRoot.first->GetFullPath());
        }
        e = cmStrCat(e, "\nis attached to multiple targets:");
        for (cmGeneratorTarget const* gt : ccRoot.second) {
          e = cmStrCat(e, "\n  ", gt->GetName());
        }
        e = cmStrCat(
          e,
          "\nbut none of these is a common dependency of the other(s).  "
          "This is not allowed by the Xcode \"new build system\".");
        generator->IssueMessage(MessageType::FATAL_ERROR, e);
        return false;
      }
    }
    this->CustomCommandRoots.clear();
  }
  // loop over all targets and add link and depend info
  for (auto* t : targets) {
    this->AddDependAndLinkInformation(t);
    this->AddEmbeddedFrameworks(t);
    this->AddEmbeddedPlugIns(t);
    this->AddEmbeddedAppExtensions(t);
    this->AddEmbeddedExtensionKitExtensions(t);
    this->AddEmbeddedResources(t);
    // Inherit project-wide values for any target-specific search paths.
    this->InheritBuildSettingAttribute(t, "HEADER_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "SYSTEM_HEADER_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "FRAMEWORK_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "SYSTEM_FRAMEWORK_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "LIBRARY_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "LD_RUNPATH_SEARCH_PATHS");
    this->InheritBuildSettingAttribute(t, "GCC_PREPROCESSOR_DEFINITIONS");
    this->InheritBuildSettingAttribute(t, "OTHER_CFLAGS");
    this->InheritBuildSettingAttribute(t, "OTHER_LDFLAGS");
    this->InheritBuildSettingAttribute(t, "OTHER_SWIFT_FLAGS");
    this->InheritBuildSettingAttribute(t,
                                       "SWIFT_ACTIVE_COMPILATION_CONDITIONS");
  }

  if (this->XcodeBuildSystem == BuildSystem::One) {
    this->CreateXCodeDependHackMakefile(targets);
  }
  // now add all targets to the root object
  cmXCodeObject* allTargets = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for (auto* t : targets) {
    allTargets->AddObject(t);
    cmXCodeObject* productRef = t->GetAttribute("productReference");
    if (productRef) {
      productGroupChildren->AddObject(productRef->GetObject());
    }
  }
  this->RootObject->AddAttribute("targets", allTargets);
  return true;
}

std::string cmGlobalXCodeGenerator::GetSymrootDir() const
{
  return cmStrCat(this->CMakeInstance->GetHomeOutputDirectory(), "/build");
}

std::string cmGlobalXCodeGenerator::GetTargetTempDir(
  cmGeneratorTarget const* gt, std::string const& configName) const
{
  // Use a path inside the SYMROOT.
  return cmStrCat(this->GetSymrootDir(), '/', gt->GetName(), ".build/",
                  configName);
}

void cmGlobalXCodeGenerator::ComputeArchitectures(cmMakefile* mf)
{
  this->Architectures.clear();
  cmList::append(this->Architectures,
                 mf->GetDefinition("CMAKE_OSX_ARCHITECTURES"));

  if (this->Architectures.empty()) {
    cmList::append(this->Architectures,
                   mf->GetDefinition("_CMAKE_APPLE_ARCHS_DEFAULT"));
  }

  if (this->Architectures.empty()) {
    // With no ARCHS we use ONLY_ACTIVE_ARCH and possibly a
    // platform-specific default ARCHS placeholder value.
    // Look up the arch that Xcode chooses in this case.
    if (cmValue arch = mf->GetDefinition("CMAKE_XCODE_ARCHS")) {
      this->ObjectDirArchDefault = *arch;
      // We expect only one arch but choose the first just in case.
      std::string::size_type pos = this->ObjectDirArchDefault.find(';');
      if (pos != std::string::npos) {
        this->ObjectDirArchDefault = this->ObjectDirArchDefault.substr(0, pos);
      }
    }
  }

  this->ComputeObjectDirArch(mf);
}

void cmGlobalXCodeGenerator::ComputeObjectDirArch(cmMakefile* mf)
{
  if (this->Architectures.size() > 1 || this->UseEffectivePlatformName(mf)) {
    this->ObjectDirArch = "$(CURRENT_ARCH)";
  } else if (!this->Architectures.empty()) {
    this->ObjectDirArch = this->Architectures[0];
  } else {
    this->ObjectDirArch = this->ObjectDirArchDefault;
  }
}

void cmGlobalXCodeGenerator::CreateXCodeDependHackMakefile(
  std::vector<cmXCodeObject*>& targets)
{
  cmGeneratedFileStream makefileStream(this->CurrentXCodeHackMakefile);
  if (!makefileStream) {
    cmSystemTools::Error(
      cmStrCat("Could not create ", this->CurrentXCodeHackMakefile));
    return;
  }
  makefileStream.SetCopyIfDifferent(true);
  // one more pass for external depend information not handled
  // correctly by xcode
  /* clang-format off */
  makefileStream << "# DO NOT EDIT\n"
                    "# This makefile makes sure all linkable targets are\n"
                    "# up-to-date with anything they link to\n"
    "default:\n"
    "\techo \"Do not invoke directly\"\n"
    "\n";
  /* clang-format on */

  std::set<std::string> dummyRules;

  // Write rules to help Xcode relink things at the right time.
  /* clang-format off */
  makefileStream <<
    "# Rules to remove targets that are older than anything to which they\n"
    "# link.  This forces Xcode to relink the targets from scratch.  It\n"
    "# does not seem to check these dependencies itself.\n";
  /* clang-format on */
  for (const auto& configName : this->CurrentConfigurationTypes) {
    for (auto* target : targets) {
      cmGeneratorTarget* gt = target->GetTarget();

      if (gt->GetType() == cmStateEnums::EXECUTABLE ||
          gt->GetType() == cmStateEnums::OBJECT_LIBRARY ||
          gt->GetType() == cmStateEnums::STATIC_LIBRARY ||
          gt->GetType() == cmStateEnums::SHARED_LIBRARY ||
          gt->GetType() == cmStateEnums::MODULE_LIBRARY) {
        // Declare an entry point for the target post-build phase.
        makefileStream << this->PostBuildMakeTarget(gt->GetName(), configName)
                       << ":\n";
      }

      if (gt->GetType() == cmStateEnums::EXECUTABLE ||
          gt->GetType() == cmStateEnums::STATIC_LIBRARY ||
          gt->GetType() == cmStateEnums::SHARED_LIBRARY ||
          gt->GetType() == cmStateEnums::MODULE_LIBRARY) {
        std::string tfull = gt->GetFullPath(configName);
        std::string trel = this->ConvertToRelativeForMake(tfull);

        // Add this target to the post-build phases of its dependencies.
        auto const y = target->GetDependTargets().find(configName);
        if (y != target->GetDependTargets().end()) {
          for (auto const& deptgt : y->second) {
            makefileStream << this->PostBuildMakeTarget(deptgt, configName)
                           << ": " << trel << '\n';
          }
        }

        std::vector<cmGeneratorTarget*> objlibs;
        gt->GetObjectLibrariesCMP0026(objlibs);
        for (auto* objLib : objlibs) {
          makefileStream << this->PostBuildMakeTarget(objLib->GetName(),
                                                      configName)
                         << ": " << trel << '\n';
        }

        // Create a rule for this target.
        makefileStream << trel << ':';

        // List dependencies if any exist.
        auto const x = target->GetDependLibraries().find(configName);
        if (x != target->GetDependLibraries().end()) {
          for (auto const& deplib : x->second) {
            std::string file = this->ConvertToRelativeForMake(deplib);
            makefileStream << "\\\n\t" << file;
            dummyRules.insert(file);
          }
        }

        for (auto* objLib : objlibs) {

          const std::string objLibName = objLib->GetName();
          std::string d = cmStrCat(this->GetTargetTempDir(gt, configName),
                                   "/lib", objLibName, ".a");

          std::string dependency = this->ConvertToRelativeForMake(d);
          makefileStream << "\\\n\t" << dependency;
          dummyRules.insert(dependency);
        }

        // Write the action to remove the target if it is out of date.
        makefileStream << "\n"
                          "\t/bin/rm -f "
                       << this->ConvertToRelativeForMake(tfull) << '\n';
        // if building for more than one architecture
        // then remove those executables as well
        if (this->Architectures.size() > 1) {
          std::string universal =
            cmStrCat(this->GetTargetTempDir(gt, configName), "/$(OBJDIR)/");
          for (const auto& architecture : this->Architectures) {
            std::string universalFile = cmStrCat(universal, architecture, '/',
                                                 gt->GetFullName(configName));
            makefileStream << "\t/bin/rm -f "
                           << this->ConvertToRelativeForMake(universalFile)
                           << '\n';
          }
        }
        makefileStream << "\n\n";
      }
    }
  }

  makefileStream << "\n\n"
                    "# For each target create a dummy rule"
                    "so the target does not have to exist\n";
  for (auto const& dummyRule : dummyRules) {
    makefileStream << dummyRule << ":\n";
  }
}

void cmGlobalXCodeGenerator::OutputXCodeProject(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  if (generators.empty()) {
    return;
  }
  if (!this->CreateXCodeObjects(root, generators)) {
    return;
  }
  std::string xcodeDir = cmStrCat(root->GetCurrentBinaryDirectory(), '/',
                                  root->GetProjectName(), ".xcodeproj");
  cmSystemTools::MakeDirectory(xcodeDir);
  std::string xcodeProjFile = cmStrCat(xcodeDir, "/project.pbxproj");
  cmGeneratedFileStream fout(xcodeProjFile);
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }
  this->WriteXCodePBXProj(fout, root, generators);

  bool hasGeneratedSchemes = this->OutputXCodeSharedSchemes(xcodeDir, root);
  this->OutputXCodeWorkspaceSettings(xcodeDir, hasGeneratedSchemes);

  this->ClearXCodeObjects();

  // Since this call may have created new cache entries, save the cache:
  //
  root->GetMakefile()->GetCMakeInstance()->SaveCache(
    root->GetBinaryDirectory());
}

bool cmGlobalXCodeGenerator::OutputXCodeSharedSchemes(
  const std::string& xcProjDir, cmLocalGenerator* root)
{
  // collect all tests for the targets
  std::map<std::string, cmXCodeScheme::TestObjects> testables;

  for (const auto& obj : this->XCodeObjects) {
    if (obj->GetType() != cmXCodeObject::OBJECT ||
        obj->GetIsA() != cmXCodeObject::PBXNativeTarget) {
      continue;
    }

    if (!obj->GetTarget()->IsXCTestOnApple()) {
      continue;
    }

    cmValue testee = obj->GetTarget()->GetProperty("XCTEST_TESTEE");
    if (!testee) {
      continue;
    }

    testables[*testee].push_back(obj.get());
  }

  // generate scheme
  bool ret = false;

  // Since the lowest available Xcode version for testing was 6.4,
  // I'm setting this as a limit then
  if (this->XcodeVersion >= 64) {
    for (const auto& obj : this->XCodeObjects) {
      if (obj->GetType() == cmXCodeObject::OBJECT &&
          (obj->GetIsA() == cmXCodeObject::PBXNativeTarget ||
           obj->GetIsA() == cmXCodeObject::PBXAggregateTarget) &&
          (root->GetMakefile()->GetCMakeInstance()->GetIsInTryCompile() ||
           obj->GetTarget()->GetPropertyAsBool("XCODE_GENERATE_SCHEME"))) {
        const std::string& targetName = obj->GetTarget()->GetName();
        cmXCodeScheme schm(root, obj.get(), testables[targetName],
                           this->CurrentConfigurationTypes,
                           this->XcodeVersion);
        schm.WriteXCodeSharedScheme(xcProjDir,
                                    this->RelativeToSource(xcProjDir));
        ret = true;
      }
    }
  }

  return ret;
}

void cmGlobalXCodeGenerator::OutputXCodeWorkspaceSettings(
  const std::string& xcProjDir, bool hasGeneratedSchemes)
{
  std::string xcodeSharedDataDir =
    cmStrCat(xcProjDir, "/project.xcworkspace/xcshareddata");
  cmSystemTools::MakeDirectory(xcodeSharedDataDir);

  std::string workspaceSettingsFile =
    cmStrCat(xcodeSharedDataDir, "/WorkspaceSettings.xcsettings");

  cmGeneratedFileStream fout(workspaceSettingsFile);
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }

  cmXMLWriter xout(fout);
  xout.StartDocument();
  xout.Doctype("plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\""
               "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");
  xout.StartElement("plist");
  xout.Attribute("version", "1.0");
  xout.StartElement("dict");
  if (this->XcodeVersion >= 100) {
    xout.Element("key", "BuildSystemType");
    switch (this->XcodeBuildSystem) {
      case BuildSystem::One:
        xout.Element("string", "Original");
        if (this->XcodeVersion >= 130) {
          xout.Element("key", "DisableBuildSystemDeprecationDiagnostic");
        } else {
          xout.Element("key", "DisableBuildSystemDeprecationWarning");
        }
        xout.Element("true");
        break;
      case BuildSystem::Twelve:
        xout.Element("string", "Latest");
        break;
    }
  }
  if (hasGeneratedSchemes) {
    xout.Element("key",
                 "IDEWorkspaceSharedSettings_AutocreateContextsIfNeeded");
    xout.Element("false");
  }
  xout.EndElement(); // dict
  xout.EndElement(); // plist
  xout.EndDocument();
}

void cmGlobalXCodeGenerator::WriteXCodePBXProj(std::ostream& fout,
                                               cmLocalGenerator*,
                                               std::vector<cmLocalGenerator*>&)
{
  SortXCodeObjects();

  fout << "// !$*UTF8*$!\n"
          "{\n";
  cmXCodeObject::Indent(1, fout);
  fout << "archiveVersion = 1;\n";
  cmXCodeObject::Indent(1, fout);
  fout << "classes = {\n";
  cmXCodeObject::Indent(1, fout);
  fout << "};\n";
  cmXCodeObject::Indent(1, fout);
  fout << "objectVersion = 46;\n";
  cmXCode21Object::PrintList(this->XCodeObjects, fout);
  cmXCodeObject::Indent(1, fout);
  fout << "rootObject = " << this->RootObject->GetId()
       << " /* Project object */;\n"
          "}\n";
}

const char* cmGlobalXCodeGenerator::GetCMakeCFGIntDir() const
{
  return "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)";
}

std::string cmGlobalXCodeGenerator::ExpandCFGIntDir(
  const std::string& str, const std::string& config) const
{
  std::string replace1 = "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)";
  std::string replace2 = "$(CONFIGURATION)";

  std::string tmp = str;
  for (std::string::size_type i = tmp.find(replace1); i != std::string::npos;
       i = tmp.find(replace1, i)) {
    tmp.replace(i, replace1.size(), config);
    i += config.size();
  }
  for (std::string::size_type i = tmp.find(replace2); i != std::string::npos;
       i = tmp.find(replace2, i)) {
    tmp.replace(i, replace2.size(), config);
    i += config.size();
  }
  return tmp;
}

cmDocumentationEntry cmGlobalXCodeGenerator::GetDocumentation()
{
  return { cmGlobalXCodeGenerator::GetActualName(),
           "Generate Xcode project files." };
}

std::string cmGlobalXCodeGenerator::ConvertToRelativeForMake(
  std::string const& p)
{
  return cmSystemTools::ConvertToOutputPath(p);
}

std::string cmGlobalXCodeGenerator::RelativeToSource(const std::string& p)
{
  std::string const& rootSrc =
    this->CurrentRootGenerator->GetCurrentSourceDirectory();
  if (cmSystemTools::IsSubDirectory(p, rootSrc)) {
    return cmSystemTools::ForceToRelativePath(rootSrc, p);
  }
  return p;
}

std::string cmGlobalXCodeGenerator::RelativeToBinary(const std::string& p)
{
  return this->CurrentRootGenerator->MaybeRelativeToCurBinDir(p);
}

std::string cmGlobalXCodeGenerator::XCodeEscapePath(const std::string& p)
{
  if (p.find_first_of(" []") != std::string::npos) {
    std::string t = cmStrCat('"', p, '"');
    return t;
  }
  return p;
}

void cmGlobalXCodeGenerator::AppendDirectoryForConfig(
  const std::string& prefix, const std::string& config,
  const std::string& suffix, std::string& dir)
{
  if (!config.empty()) {
    dir += prefix;
    dir += config;
    dir += suffix;
  }
}

std::string cmGlobalXCodeGenerator::LookupFlags(
  const std::string& varNamePrefix, const std::string& varNameLang,
  const std::string& varNameSuffix, const std::string& default_flags)
{
  if (!varNameLang.empty()) {
    std::string varName = cmStrCat(varNamePrefix, varNameLang, varNameSuffix);
    if (cmValue varValue = this->CurrentMakefile->GetDefinition(varName)) {
      if (!varValue->empty()) {
        return *varValue;
      }
    }
  }
  return default_flags;
}

void cmGlobalXCodeGenerator::AppendDefines(BuildObjectListOrString& defs,
                                           const char* defines_list,
                                           bool dflag)
{
  // Skip this if there are no definitions.
  if (!defines_list) {
    return;
  }

  // Expand the list of definitions.
  cmList defines{ defines_list };

  // Store the definitions in the string.
  this->AppendDefines(defs, defines, dflag);
}

void cmGlobalXCodeGenerator::AppendDefines(
  BuildObjectListOrString& defs, std::vector<std::string> const& defines,
  bool dflag)
{
  // GCC_PREPROCESSOR_DEFINITIONS is a space-separated list of definitions.
  std::string def;
  for (auto const& define : defines) {
    // Start with -D if requested.
    if (dflag && !cmHasLiteralPrefix(define, "-D")) {
      def = cmStrCat("-D", define);
    } else if (!dflag && cmHasLiteralPrefix(define, "-D")) {
      def = define.substr(2);
    } else {
      def = define;
    }

    // Append the flag with needed escapes.
    std::string tmp;
    this->AppendFlag(tmp, def);
    defs.Add(tmp);
  }
}

void cmGlobalXCodeGenerator::AppendFlag(std::string& flags,
                                        std::string const& flag) const
{
  // Short-circuit for an empty flag.
  if (flag.empty()) {
    return;
  }

  // Separate from previous flags.
  if (!flags.empty()) {
    flags += ' ';
  }

  // Check if the flag needs quoting.
  bool quoteFlag =
    flag.find_first_of("`~!@#$%^&*()+={}[]|:;\"'<>,.? ") != std::string::npos;

  // We escape a flag as follows:
  //   - Place each flag in single quotes ''
  //   - Escape a single quote as \'
  //   - Escape a backslash as \\ since it itself is an escape
  // Note that in the code below we need one more level of escapes for
  // C string syntax in this source file.
  //
  // The final level of escaping is done when the string is stored
  // into the project file by cmXCodeObject::PrintString.

  if (quoteFlag) {
    // Open single quote.
    flags += '\'';
  }

  // Flag value with escaped quotes and backslashes.
  for (auto c : flag) {
    if (c == '\'') {
      flags += "'\\''";
    } else if (c == '\\') {
      flags += "\\\\";
    } else {
      flags += c;
    }
  }

  if (quoteFlag) {
    // Close single quote.
    flags += '\'';
  }
}

std::string cmGlobalXCodeGenerator::ComputeInfoPListLocation(
  cmGeneratorTarget* target)
{
  std::string plist =
    cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(),
             "/CMakeFiles/", target->GetName(), ".dir/Info.plist");
  return plist;
}

// Return true if the generated build tree may contain multiple builds.
// i.e. "Can I build Debug and Release in the same tree?"
bool cmGlobalXCodeGenerator::IsMultiConfig() const
{
  // Newer Xcode versions are multi config:
  return true;
}

bool cmGlobalXCodeGenerator::HasKnownObjectFileLocation(
  cmTarget const& target, std::string* reason) const
{
  auto objectDirArch = GetTargetObjectDirArch(target, this->ObjectDirArch);

  if (objectDirArch.find('$') != std::string::npos) {
    if (reason != nullptr) {
      *reason = " under Xcode with multiple architectures";
    }
    return false;
  }
  return true;
}

bool cmGlobalXCodeGenerator::UseEffectivePlatformName(cmMakefile* mf) const
{
  cmValue epnValue = this->GetCMakeInstance()->GetState()->GetGlobalProperty(
    "XCODE_EMIT_EFFECTIVE_PLATFORM_NAME");

  if (!epnValue) {
    return mf->PlatformIsAppleEmbedded();
  }

  return cmIsOn(*epnValue);
}

bool cmGlobalXCodeGenerator::ShouldStripResourcePath(cmMakefile*) const
{
  // Xcode determines Resource location itself
  return true;
}

void cmGlobalXCodeGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  auto objectDirArch = GetTargetObjectDirArch(*gt, this->ObjectDirArch);
  gt->ObjectDirectory =
    cmStrCat(this->GetTargetTempDir(gt, this->GetCMakeCFGIntDir()),
             "/$(OBJECT_FILE_DIR_normal:base)/", objectDirArch, '/');
}

std::string cmGlobalXCodeGenerator::GetDeploymentPlatform(const cmMakefile* mf)
{
  switch (mf->GetAppleSDKType()) {
    case cmMakefile::AppleSDK::AppleTVOS:
    case cmMakefile::AppleSDK::AppleTVSimulator:
      return "TVOS_DEPLOYMENT_TARGET";

    case cmMakefile::AppleSDK::IPhoneOS:
    case cmMakefile::AppleSDK::IPhoneSimulator:
      return "IPHONEOS_DEPLOYMENT_TARGET";

    case cmMakefile::AppleSDK::WatchOS:
    case cmMakefile::AppleSDK::WatchSimulator:
      return "WATCHOS_DEPLOYMENT_TARGET";

    case cmMakefile::AppleSDK::XROS:
    case cmMakefile::AppleSDK::XRSimulator:
      return "XROS_DEPLOYMENT_TARGET";

    case cmMakefile::AppleSDK::MacOS:
    default:
      return "MACOSX_DEPLOYMENT_TARGET";
  }
}
