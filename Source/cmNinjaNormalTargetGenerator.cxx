/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaNormalTargetGenerator.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/vector>

#include "cmComputeLinkInformation.h"
#include "cmCustomCommand.h" // IWYU pragma: keep
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNinjaLinkLineDeviceComputer.h"
#include "cmNinjaTypes.h"
#include "cmOSXBundleGenerator.h"
#include "cmOutputConverter.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmNinjaNormalTargetGenerator::cmNinjaNormalTargetGenerator(
  cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target)
{
  if (target->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    for (auto const& config : this->GetConfigNames()) {
      this->EnsureDirectoryExists(target->GetDirectory(config));
    }
  }

  this->OSXBundleGenerator = cm::make_unique<cmOSXBundleGenerator>(target);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator() = default;

void cmNinjaNormalTargetGenerator::Generate(const std::string& config)
{
  std::string lang = this->GeneratorTarget->GetLinkerLanguage(config);
  if (this->TargetLinkLanguage(config).empty()) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: " +
                         this->GetGeneratorTarget()->GetName());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules(config);

  // Write the build statements
  bool firstForConfig = true;
  for (auto const& fileConfig : this->GetConfigNames()) {
    if (!this->GetGlobalGenerator()
           ->GetCrossConfigs(fileConfig)
           .count(config)) {
      continue;
    }
    this->WriteObjectBuildStatements(config, fileConfig, firstForConfig);
    firstForConfig = false;
  }

  if (this->GetGeneratorTarget()->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    this->WriteObjectLibStatement(config);
  } else {
    firstForConfig = true;
    for (auto const& fileConfig : this->GetConfigNames()) {
      if (!this->GetGlobalGenerator()
             ->GetCrossConfigs(fileConfig)
             .count(config)) {
        continue;
      }
      // If this target has cuda language link inputs, and we need to do
      // device linking
      this->WriteDeviceLinkStatement(config, fileConfig, firstForConfig);
      this->WriteLinkStatement(config, fileConfig, firstForConfig);
      firstForConfig = false;
    }
  }
  if (this->GetGlobalGenerator()->EnableCrossConfigBuild()) {
    this->GetGlobalGenerator()->AddTargetAlias(
      this->GetTargetName(), this->GetGeneratorTarget(), "all");
  }

  // Find ADDITIONAL_CLEAN_FILES
  this->AdditionalCleanFiles(config);
}

void cmNinjaNormalTargetGenerator::WriteLanguagesRules(
  const std::string& config)
{
#ifdef NINJA_GEN_VERBOSE_FILES
  cmGlobalNinjaGenerator::WriteDivider(this->GetRulesFileStream());
  this->GetRulesFileStream()
    << "# Rules for each languages for "
    << cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType())
    << " target " << this->GetTargetName() << "\n\n";
#endif

  // Write rules for languages compiled in this target.
  std::set<std::string> languages;
  std::vector<cmSourceFile const*> sourceFiles;
  this->GetGeneratorTarget()->GetObjectSources(sourceFiles, config);
  for (cmSourceFile const* sf : sourceFiles) {
    std::string const lang = sf->GetLanguage();
    if (!lang.empty()) {
      languages.insert(lang);
    }
  }
  for (std::string const& language : languages) {
    this->WriteLanguageRules(language, config);
  }
}

const char* cmNinjaNormalTargetGenerator::GetVisibleTypeName() const
{
  switch (this->GetGeneratorTarget()->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      return "static library";
    case cmStateEnums::SHARED_LIBRARY:
      return "shared library";
    case cmStateEnums::MODULE_LIBRARY:
      if (this->GetGeneratorTarget()->IsCFBundleOnApple()) {
        return "CFBundle shared module";
      } else {
        return "shared module";
      }
    case cmStateEnums::EXECUTABLE:
      return "executable";
    default:
      return nullptr;
  }
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerRule(
  const std::string& config) const
{
  return cmStrCat(
    this->TargetLinkLanguage(config), "_",
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()),
    "_LINKER__",
    cmGlobalNinjaGenerator::EncodeRuleName(
      this->GetGeneratorTarget()->GetName()),
    "_", config);
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerDeviceRule(
  const std::string& config) const
{
  return cmStrCat(
    this->TargetLinkLanguage(config), "_",
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()),
    "_DEVICE_LINKER__",
    cmGlobalNinjaGenerator::EncodeRuleName(
      this->GetGeneratorTarget()->GetName()),
    "_", config);
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerCudaDeviceRule(
  const std::string& config) const
{
  return cmStrCat(
    this->TargetLinkLanguage(config), "_DEVICE_LINK__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerCudaDeviceCompileRule(
  const std::string& config) const
{
  return cmStrCat(
    this->TargetLinkLanguage(config), "_DEVICE_LINK_COMPILE__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerCudaFatbinaryRule(
  const std::string& config) const
{
  return cmStrCat(
    this->TargetLinkLanguage(config), "_FATBINARY__",
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName()),
    '_', config);
}

struct cmNinjaRemoveNoOpCommands
{
  bool operator()(std::string const& cmd)
  {
    return cmd.empty() || cmd[0] == ':';
  }
};

void cmNinjaNormalTargetGenerator::WriteNvidiaDeviceLinkRule(
  bool useResponseFile, const std::string& config)
{
  cmNinjaRule rule(this->LanguageLinkerDeviceRule(config));
  if (!this->GetGlobalGenerator()->HasRule(rule.Name)) {
    cmRulePlaceholderExpander::RuleVariables vars;
    vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
    vars.CMTargetType =
      cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType())
        .c_str();

    vars.Language = "CUDA";

    // build response file name
    std::string responseFlag = this->GetMakefile()->GetSafeDefinition(
      "CMAKE_CUDA_RESPONSE_FILE_DEVICE_LINK_FLAG");

    if (!useResponseFile || responseFlag.empty()) {
      vars.Objects = "$in";
      vars.LinkLibraries = "$LINK_PATH $LINK_LIBRARIES";
    } else {
      rule.RspFile = "$RSP_FILE";
      responseFlag += rule.RspFile;

      // build response file content
      if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
        rule.RspContent = "$in";
      } else {
        rule.RspContent = "$in_newline";
      }
      rule.RspContent += " $LINK_LIBRARIES";
      vars.Objects = responseFlag.c_str();
      vars.LinkLibraries = "";
    }

    vars.ObjectDir = "$OBJECT_DIR";

    vars.Target = "$TARGET_FILE";

    vars.SONameFlag = "$SONAME_FLAG";
    vars.TargetSOName = "$SONAME";
    vars.TargetPDB = "$TARGET_PDB";
    vars.TargetCompilePDB = "$TARGET_COMPILE_PDB";

    vars.Flags = "$FLAGS";
    vars.LinkFlags = "$LINK_FLAGS";
    vars.Manifests = "$MANIFESTS";

    vars.LanguageCompileFlags = "$LANGUAGE_COMPILE_FLAGS";

    std::string launcher;
    cmValue val = this->GetLocalGenerator()->GetRuleLauncher(
      this->GetGeneratorTarget(), "RULE_LAUNCH_LINK");
    if (cmNonempty(val)) {
      launcher = cmStrCat(*val, ' ');
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->GetLocalGenerator()->CreateRulePlaceholderExpander());

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeDeviceLinkCmd();
    for (std::string& linkCmd : linkCmds) {
      linkCmd = cmStrCat(launcher, linkCmd);
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   linkCmd, vars);
    }

    // If there is no ranlib the command will be ":".  Skip it.
    cm::erase_if(linkCmds, cmNinjaRemoveNoOpCommands());

    rule.Command =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds, config, config);

    // Write the linker rule with response file if needed.
    rule.Comment =
      cmStrCat("Rule for linking ", this->TargetLinkLanguage(config), ' ',
               this->GetVisibleTypeName(), '.');
    rule.Description =
      cmStrCat("Linking ", this->TargetLinkLanguage(config), ' ',
               this->GetVisibleTypeName(), " $TARGET_FILE");
    rule.Restat = "$RESTAT";

    this->GetGlobalGenerator()->AddRule(rule);
  }
}

void cmNinjaNormalTargetGenerator::WriteDeviceLinkRules(
  const std::string& config)
{
  const cmMakefile* mf = this->GetMakefile();

  cmNinjaRule rule(this->LanguageLinkerCudaDeviceRule(config));
  rule.Command = this->GetLocalGenerator()->BuildCommandLine(
    { cmStrCat(mf->GetRequiredDefinition("CMAKE_CUDA_DEVICE_LINKER"),
               " -arch=$ARCH $REGISTER -o=$out $in") },
    config, config);
  rule.Comment = "Rule for CUDA device linking.";
  rule.Description = "Linking CUDA $out";
  this->GetGlobalGenerator()->AddRule(rule);

  cmRulePlaceholderExpander::RuleVariables vars;
  vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
  vars.CMTargetType =
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()).c_str();

  vars.Language = "CUDA";
  vars.Object = "$out";
  vars.Fatbinary = "$FATBIN";
  vars.RegisterFile = "$REGISTER";
  vars.LinkFlags = "$LINK_FLAGS";

  std::string flags = this->GetFlags("CUDA", config);
  vars.Flags = flags.c_str();

  std::string compileCmd = this->GetMakefile()->GetRequiredDefinition(
    "CMAKE_CUDA_DEVICE_LINK_COMPILE");
  std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
    this->GetLocalGenerator()->CreateRulePlaceholderExpander());
  rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                               compileCmd, vars);

  rule.Name = this->LanguageLinkerCudaDeviceCompileRule(config);
  rule.Command = this->GetLocalGenerator()->BuildCommandLine({ compileCmd },
                                                             config, config);
  rule.Comment = "Rule for compiling CUDA device stubs.";
  rule.Description = "Compiling CUDA device stub $out";
  this->GetGlobalGenerator()->AddRule(rule);

  rule.Name = this->LanguageLinkerCudaFatbinaryRule(config);
  rule.Command = this->GetLocalGenerator()->BuildCommandLine(
    { cmStrCat(mf->GetRequiredDefinition("CMAKE_CUDA_FATBINARY"),
               " -64 -cmdline=--compile-only -compress-all -link "
               "--embedded-fatbin=$out $PROFILES") },
    config, config);
  rule.Comment = "Rule for CUDA fatbinaries.";
  rule.Description = "Creating fatbinary $out";
  this->GetGlobalGenerator()->AddRule(rule);
}

void cmNinjaNormalTargetGenerator::WriteLinkRule(bool useResponseFile,
                                                 const std::string& config)
{
  cmStateEnums::TargetType targetType = this->GetGeneratorTarget()->GetType();

  std::string linkRuleName = this->LanguageLinkerRule(config);
  if (!this->GetGlobalGenerator()->HasRule(linkRuleName)) {
    cmNinjaRule rule(std::move(linkRuleName));
    cmRulePlaceholderExpander::RuleVariables vars;
    vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
    vars.CMTargetType = cmState::GetTargetTypeName(targetType).c_str();

    std::string lang = this->TargetLinkLanguage(config);
    vars.Language = lang.c_str();
    vars.AIXExports = "$AIX_EXPORTS";

    if (this->TargetLinkLanguage(config) == "Swift") {
      vars.SwiftLibraryName = "$SWIFT_LIBRARY_NAME";
      vars.SwiftModule = "$SWIFT_MODULE";
      vars.SwiftModuleName = "$SWIFT_MODULE_NAME";
      vars.SwiftOutputFileMap = "$SWIFT_OUTPUT_FILE_MAP";
      vars.SwiftSources = "$SWIFT_SOURCES";

      vars.Defines = "$DEFINES";
      vars.Flags = "$FLAGS";
      vars.Includes = "$INCLUDES";
    }

    std::string responseFlag;

    std::string cmakeVarLang =
      cmStrCat("CMAKE_", this->TargetLinkLanguage(config));

    // build response file name
    std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";
    cmValue flag = this->GetMakefile()->GetDefinition(cmakeLinkVar);

    if (flag) {
      responseFlag = *flag;
    } else {
      responseFlag = "@";
    }

    if (!useResponseFile || responseFlag.empty()) {
      vars.Objects = "$in";
      vars.LinkLibraries = "$LINK_PATH $LINK_LIBRARIES";
    } else {
      rule.RspFile = "$RSP_FILE";
      responseFlag += rule.RspFile;

      // build response file content
      if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
        rule.RspContent = "$in";
      } else {
        rule.RspContent = "$in_newline";
      }
      rule.RspContent += " $LINK_PATH $LINK_LIBRARIES";
      if (this->TargetLinkLanguage(config) == "Swift") {
        vars.SwiftSources = responseFlag.c_str();
      } else {
        vars.Objects = responseFlag.c_str();
      }
      vars.LinkLibraries = "";
    }

    vars.ObjectDir = "$OBJECT_DIR";

    vars.Target = "$TARGET_FILE";

    vars.SONameFlag = "$SONAME_FLAG";
    vars.TargetSOName = "$SONAME";
    vars.TargetInstallNameDir = "$INSTALLNAME_DIR";
    vars.TargetPDB = "$TARGET_PDB";

    // Setup the target version.
    std::string targetVersionMajor;
    std::string targetVersionMinor;
    {
      std::ostringstream majorStream;
      std::ostringstream minorStream;
      int major;
      int minor;
      this->GetGeneratorTarget()->GetTargetVersion(major, minor);
      majorStream << major;
      minorStream << minor;
      targetVersionMajor = majorStream.str();
      targetVersionMinor = minorStream.str();
    }
    vars.TargetVersionMajor = targetVersionMajor.c_str();
    vars.TargetVersionMinor = targetVersionMinor.c_str();

    vars.Flags = "$FLAGS";
    vars.LinkFlags = "$LINK_FLAGS";
    vars.Manifests = "$MANIFESTS";

    std::string langFlags;
    if (targetType != cmStateEnums::EXECUTABLE) {
      langFlags += "$LANGUAGE_COMPILE_FLAGS $ARCH_FLAGS";
      vars.LanguageCompileFlags = langFlags.c_str();
    }

    std::string linkerLauncher = this->GetLinkerLauncher(config);
    if (cmNonempty(linkerLauncher)) {
      vars.Launcher = linkerLauncher.c_str();
    }

    std::string launcher;
    cmValue val = this->GetLocalGenerator()->GetRuleLauncher(
      this->GetGeneratorTarget(), "RULE_LAUNCH_LINK");
    if (cmNonempty(val)) {
      launcher = cmStrCat(*val, ' ');
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->GetLocalGenerator()->CreateRulePlaceholderExpander());

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd(config);
    for (std::string& linkCmd : linkCmds) {
      linkCmd = cmStrCat(launcher, linkCmd);
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   linkCmd, vars);
    }

    // If there is no ranlib the command will be ":".  Skip it.
    cm::erase_if(linkCmds, cmNinjaRemoveNoOpCommands());

    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.emplace_back("$POST_BUILD");
    rule.Command =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds, config, config);

    // Write the linker rule with response file if needed.
    rule.Comment =
      cmStrCat("Rule for linking ", this->TargetLinkLanguage(config), ' ',
               this->GetVisibleTypeName(), '.');
    rule.Description =
      cmStrCat("Linking ", this->TargetLinkLanguage(config), ' ',
               this->GetVisibleTypeName(), " $TARGET_FILE");
    rule.Restat = "$RESTAT";
    this->GetGlobalGenerator()->AddRule(rule);
  }

  auto const tgtNames = this->TargetNames(config);
  if (tgtNames.Output != tgtNames.Real &&
      !this->GetGeneratorTarget()->IsFrameworkOnApple()) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
    if (targetType == cmStateEnums::EXECUTABLE) {
      cmNinjaRule rule("CMAKE_SYMLINK_EXECUTABLE");
      {
        std::vector<std::string> cmd;
        cmd.push_back(cmakeCommand + " -E cmake_symlink_executable $in $out");
        cmd.emplace_back("$POST_BUILD");
        rule.Command =
          this->GetLocalGenerator()->BuildCommandLine(cmd, config, config);
      }
      rule.Description = "Creating executable symlink $out";
      rule.Comment = "Rule for creating executable symlink.";
      this->GetGlobalGenerator()->AddRule(rule);
    } else {
      cmNinjaRule rule("CMAKE_SYMLINK_LIBRARY");
      {
        std::vector<std::string> cmd;
        cmd.push_back(cmakeCommand +
                      " -E cmake_symlink_library $in $SONAME $out");
        cmd.emplace_back("$POST_BUILD");
        rule.Command =
          this->GetLocalGenerator()->BuildCommandLine(cmd, config, config);
      }
      rule.Description = "Creating library symlink $out";
      rule.Comment = "Rule for creating library symlink.";
      this->GetGlobalGenerator()->AddRule(rule);
    }
  }
}

std::vector<std::string> cmNinjaNormalTargetGenerator::ComputeDeviceLinkCmd()
{
  std::vector<std::string> linkCmds;

  // this target requires separable cuda compilation
  // now build the correct command depending on if the target is
  // an executable or a dynamic library.
  std::string linkCmd;
  switch (this->GetGeneratorTarget()->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY: {
      this->GetMakefile()->GetDefExpandList("CMAKE_CUDA_DEVICE_LINK_LIBRARY",
                                            linkCmds);
    } break;
    case cmStateEnums::EXECUTABLE: {
      this->GetMakefile()->GetDefExpandList(
        "CMAKE_CUDA_DEVICE_LINK_EXECUTABLE", linkCmds);
    } break;
    default:
      break;
  }
  return linkCmds;
}

std::vector<std::string> cmNinjaNormalTargetGenerator::ComputeLinkCmd(
  const std::string& config)
{
  std::vector<std::string> linkCmds;
  cmMakefile* mf = this->GetMakefile();
  {
    // If we have a rule variable prefer it. In the case of static libraries
    // this occurs when things like IPO is enabled, and we need to use the
    // CMAKE_<lang>_CREATE_STATIC_LIBRARY_IPO define instead.
    std::string linkCmdVar = this->GetGeneratorTarget()->GetCreateRuleVariable(
      this->TargetLinkLanguage(config), config);
    cmValue linkCmd = mf->GetDefinition(linkCmdVar);
    if (linkCmd) {
      std::string linkCmdStr = *linkCmd;
      if (this->GetGeneratorTarget()->HasImplibGNUtoMS(config)) {
        std::string ruleVar =
          cmStrCat("CMAKE_", this->GeneratorTarget->GetLinkerLanguage(config),
                   "_GNUtoMS_RULE");
        if (cmValue rule = this->Makefile->GetDefinition(ruleVar)) {
          linkCmdStr += *rule;
        }
      }
      cmExpandList(linkCmdStr, linkCmds);
      if (this->UseLWYU) {
        cmValue lwyuCheck = mf->GetDefinition("CMAKE_LINK_WHAT_YOU_USE_CHECK");
        if (lwyuCheck) {
          std::string cmakeCommand = cmStrCat(
            this->GetLocalGenerator()->ConvertToOutputFormat(
              cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL),
            " -E __run_co_compile --lwyu=");
          cmakeCommand +=
            this->GetLocalGenerator()->EscapeForShell(*lwyuCheck);

          std::string targetOutputReal =
            this->ConvertToNinjaPath(this->GetGeneratorTarget()->GetFullPath(
              config, cmStateEnums::RuntimeBinaryArtifact,
              /*realname=*/true));
          cmakeCommand += cmStrCat(" --source=", targetOutputReal);
          linkCmds.push_back(std::move(cmakeCommand));
        }
      }
      return linkCmds;
    }
  }
  switch (this->GetGeneratorTarget()->GetType()) {
    case cmStateEnums::STATIC_LIBRARY: {
      // We have archive link commands set. First, delete the existing archive.
      {
        std::string cmakeCommand =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
        linkCmds.push_back(cmakeCommand + " -E rm -f $TARGET_FILE");
      }
      // TODO: Use ARCHIVE_APPEND for archives over a certain size.
      {
        std::string linkCmdVar = cmStrCat(
          "CMAKE_", this->TargetLinkLanguage(config), "_ARCHIVE_CREATE");

        linkCmdVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
          linkCmdVar, this->TargetLinkLanguage(config), config);

        std::string const& linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmExpandList(linkCmd, linkCmds);
      }
      {
        std::string linkCmdVar = cmStrCat(
          "CMAKE_", this->TargetLinkLanguage(config), "_ARCHIVE_FINISH");

        linkCmdVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
          linkCmdVar, this->TargetLinkLanguage(config), config);

        std::string const& linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmExpandList(linkCmd, linkCmds);
      }
#ifdef __APPLE__
      // On macOS ranlib truncates the fractional part of the static archive
      // file modification time.  If the archive and at least one contained
      // object file were created within the same second this will make look
      // the archive older than the object file. On subsequent ninja runs this
      // leads to re-achiving and updating dependent targets.
      // As a work-around we touch the archive after ranlib (see #19222).
      {
        std::string cmakeCommand =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
        linkCmds.push_back(cmakeCommand + " -E touch $TARGET_FILE");
      }
#endif
    } break;
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      break;
    case cmStateEnums::EXECUTABLE:
      if (this->TargetLinkLanguage(config) == "Swift") {
        if (this->GeneratorTarget->IsExecutableWithExports()) {
          this->Makefile->GetDefExpandList("CMAKE_EXE_EXPORTS_Swift_FLAG",
                                           linkCmds);
        }
      }
      break;
    default:
      assert(false && "Unexpected target type");
  }
  return linkCmds;
}

void cmNinjaNormalTargetGenerator::WriteDeviceLinkStatement(
  const std::string& config, const std::string& fileConfig,
  bool firstForConfig)
{
  cmGlobalNinjaGenerator* globalGen = this->GetGlobalGenerator();
  if (!globalGen->GetLanguageEnabled("CUDA")) {
    return;
  }

  cmGeneratorTarget* genTarget = this->GetGeneratorTarget();

  bool requiresDeviceLinking = requireDeviceLinking(
    *this->GeneratorTarget, *this->GetLocalGenerator(), config);
  if (!requiresDeviceLinking) {
    return;
  }

  // First and very important step is to make sure while inside this
  // step our link language is set to CUDA
  std::string const& objExt =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_OUTPUT_EXTENSION");

  std::string targetOutputDir =
    cmStrCat(this->GetLocalGenerator()->GetTargetDirectory(genTarget),
             globalGen->ConfigDirectory(config), "/");
  targetOutputDir = globalGen->ExpandCFGIntDir(targetOutputDir, config);

  std::string targetOutputReal =
    this->ConvertToNinjaPath(targetOutputDir + "cmake_device_link" + objExt);

  if (firstForConfig) {
    globalGen->GetByproductsForCleanTarget(config).push_back(targetOutputReal);
  }
  this->DeviceLinkObject = targetOutputReal;

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetCommonFileStream());
  this->GetCommonFileStream()
    << "# Device Link build statements for "
    << cmState::GetTargetTypeName(genTarget->GetType()) << " target "
    << this->GetTargetName() << "\n\n";

  if (this->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_ID") == "Clang") {
    std::string architecturesStr =
      this->GeneratorTarget->GetSafeProperty("CUDA_ARCHITECTURES");

    if (cmIsOff(architecturesStr)) {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   "CUDA_SEPARABLE_COMPILATION on Clang "
                                   "requires CUDA_ARCHITECTURES to be set.");
      return;
    }

    this->WriteDeviceLinkRules(config);
    this->WriteDeviceLinkStatements(config, cmExpandedList(architecturesStr),
                                    targetOutputReal);
  } else {
    this->WriteNvidiaDeviceLinkStatement(config, fileConfig, targetOutputDir,
                                         targetOutputReal);
  }
}

void cmNinjaNormalTargetGenerator::WriteDeviceLinkStatements(
  const std::string& config, const std::vector<std::string>& architectures,
  const std::string& output)
{
  // Ensure there are no duplicates.
  const cmNinjaDeps explicitDeps = [&]() -> std::vector<std::string> {
    std::unordered_set<std::string> depsSet;
    const cmNinjaDeps linkDeps =
      this->ComputeLinkDeps(this->TargetLinkLanguage(config), config, true);
    const cmNinjaDeps objects = this->GetObjects(config);
    depsSet.insert(linkDeps.begin(), linkDeps.end());
    depsSet.insert(objects.begin(), objects.end());

    std::vector<std::string> deps;
    std::copy(depsSet.begin(), depsSet.end(), std::back_inserter(deps));
    return deps;
  }();

  cmGlobalNinjaGenerator* globalGen{ this->GetGlobalGenerator() };
  const std::string objectDir =
    cmStrCat(this->GeneratorTarget->GetSupportDirectory(),
             globalGen->ConfigDirectory(config));
  const std::string ninjaOutputDir = this->ConvertToNinjaPath(objectDir);

  cmNinjaBuild fatbinary(this->LanguageLinkerCudaFatbinaryRule(config));

  // Link device code for each architecture.
  for (const std::string& architectureKind : architectures) {
    // Clang always generates real code, so strip the specifier.
    const std::string architecture =
      architectureKind.substr(0, architectureKind.find('-'));
    const std::string cubin =
      cmStrCat(ninjaOutputDir, "/sm_", architecture, ".cubin");

    cmNinjaBuild dlink(this->LanguageLinkerCudaDeviceRule(config));
    dlink.ExplicitDeps = explicitDeps;
    dlink.Outputs = { cubin };
    dlink.Variables["ARCH"] = cmStrCat("sm_", architecture);

    // The generated register file contains macros that when expanded register
    // the device routines. Because the routines are the same for all
    // architectures the register file will be the same too. Thus generate it
    // only on the first invocation to reduce overhead.
    if (fatbinary.ExplicitDeps.empty()) {
      dlink.Variables["REGISTER"] = cmStrCat(
        "--register-link-binaries=", ninjaOutputDir, "/cmake_cuda_register.h");
    }

    fatbinary.Variables["PROFILES"] +=
      cmStrCat(" -im=profile=sm_", architecture, ",file=", cubin);
    fatbinary.ExplicitDeps.emplace_back(cubin);

    globalGen->WriteBuild(this->GetCommonFileStream(), dlink);
  }

  // Combine all architectures into a single fatbinary.
  fatbinary.Outputs = { cmStrCat(ninjaOutputDir, "/cmake_cuda_fatbin.h") };
  globalGen->WriteBuild(this->GetCommonFileStream(), fatbinary);

  // Compile the stub that registers the kernels and contains the fatbinaries.
  cmLocalNinjaGenerator* localGen{ this->GetLocalGenerator() };
  cmNinjaBuild dcompile(this->LanguageLinkerCudaDeviceCompileRule(config));
  dcompile.Outputs = { output };
  dcompile.ExplicitDeps = { cmStrCat(ninjaOutputDir, "/cmake_cuda_fatbin.h") };
  dcompile.Variables["FATBIN"] = localGen->ConvertToOutputFormat(
    cmStrCat(objectDir, "/cmake_cuda_fatbin.h"), cmOutputConverter::SHELL);
  dcompile.Variables["REGISTER"] = localGen->ConvertToOutputFormat(
    cmStrCat(objectDir, "/cmake_cuda_register.h"), cmOutputConverter::SHELL);

  cmNinjaLinkLineDeviceComputer linkLineComputer(
    localGen, localGen->GetStateSnapshot().GetDirectory(), globalGen);
  linkLineComputer.SetUseNinjaMulti(globalGen->IsMultiConfig());

  // Link libraries and paths are only used during the final executable/library
  // link.
  std::string frameworkPath;
  std::string linkPath;
  std::string linkLibs;
  localGen->GetDeviceLinkFlags(linkLineComputer, config, linkLibs,
                               dcompile.Variables["LINK_FLAGS"], frameworkPath,
                               linkPath, this->GetGeneratorTarget());

  globalGen->WriteBuild(this->GetCommonFileStream(), dcompile);
}

void cmNinjaNormalTargetGenerator::WriteNvidiaDeviceLinkStatement(
  const std::string& config, const std::string& fileConfig,
  const std::string& outputDir, const std::string& output)
{
  cmGeneratorTarget* genTarget = this->GetGeneratorTarget();
  cmGlobalNinjaGenerator* globalGen = this->GetGlobalGenerator();

  std::string targetOutputImplib = this->ConvertToNinjaPath(
    genTarget->GetFullPath(config, cmStateEnums::ImportLibraryArtifact));

  if (config != fileConfig) {
    std::string targetOutputFileConfigDir =
      cmStrCat(this->GetLocalGenerator()->GetTargetDirectory(genTarget),
               globalGen->ConfigDirectory(fileConfig), "/");
    targetOutputFileConfigDir =
      globalGen->ExpandCFGIntDir(outputDir, fileConfig);
    if (outputDir == targetOutputFileConfigDir) {
      return;
    }

    if (!genTarget->GetFullName(config, cmStateEnums::ImportLibraryArtifact)
           .empty() &&
        !genTarget
           ->GetFullName(fileConfig, cmStateEnums::ImportLibraryArtifact)
           .empty() &&
        targetOutputImplib ==
          this->ConvertToNinjaPath(genTarget->GetFullPath(
            fileConfig, cmStateEnums::ImportLibraryArtifact))) {
      return;
    }
  }

  // Compute the comment.
  cmNinjaBuild build(this->LanguageLinkerDeviceRule(config));
  build.Comment =
    cmStrCat("Link the ", this->GetVisibleTypeName(), ' ', output);

  cmNinjaVars& vars = build.Variables;

  // Compute outputs.
  build.Outputs.push_back(output);
  // Compute specific libraries to link with.
  build.ExplicitDeps = this->GetObjects(config);
  build.ImplicitDeps =
    this->ComputeLinkDeps(this->TargetLinkLanguage(config), config);

  std::string frameworkPath;
  std::string linkPath;

  std::string createRule =
    genTarget->GetCreateRuleVariable(this->TargetLinkLanguage(config), config);
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(output, cmOutputConverter::SHELL);

  cmNinjaLinkLineDeviceComputer linkLineComputer(
    this->GetLocalGenerator(),
    this->GetLocalGenerator()->GetStateSnapshot().GetDirectory(), globalGen);
  linkLineComputer.SetUseNinjaMulti(globalGen->IsMultiConfig());

  localGen.GetDeviceLinkFlags(linkLineComputer, config, vars["LINK_LIBRARIES"],
                              vars["LINK_FLAGS"], frameworkPath, linkPath,
                              genTarget);

  this->addPoolNinjaVariable("JOB_POOL_LINK", genTarget, vars);

  vars["LINK_FLAGS"] = globalGen->EncodeLiteral(vars["LINK_FLAGS"]);

  vars["MANIFESTS"] = this->GetManifests(config);

  vars["LINK_PATH"] = frameworkPath + linkPath;

  // Compute language specific link flags.
  std::string langFlags;
  localGen.AddLanguageFlagsForLinking(langFlags, genTarget, "CUDA", config);
  vars["LANGUAGE_COMPILE_FLAGS"] = langFlags;

  auto const tgtNames = this->TargetNames(config);
  if (genTarget->HasSOName(config)) {
    vars["SONAME_FLAG"] =
      this->GetMakefile()->GetSONameFlag(this->TargetLinkLanguage(config));
    vars["SONAME"] = localGen.ConvertToOutputFormat(tgtNames.SharedObject,
                                                    cmOutputConverter::SHELL);
    if (genTarget->GetType() == cmStateEnums::SHARED_LIBRARY) {
      std::string install_dir =
        this->GetGeneratorTarget()->GetInstallNameDirForBuildTree(config);
      if (!install_dir.empty()) {
        vars["INSTALLNAME_DIR"] = localGen.ConvertToOutputFormat(
          install_dir, cmOutputConverter::SHELL);
      }
    }
  }

  if (!tgtNames.ImportLibrary.empty()) {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
      targetOutputImplib, cmOutputConverter::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    this->EnsureParentDirectoryExists(targetOutputImplib);
  }

  const std::string objPath =
    cmStrCat(this->GetGeneratorTarget()->GetSupportDirectory(),
             globalGen->ConfigDirectory(config));

  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(objPath), cmOutputConverter::SHELL);
  this->EnsureDirectoryExists(objPath);

  this->SetMsvcTargetPdbVariable(vars, config);

  std::string& linkLibraries = vars["LINK_LIBRARIES"];
  std::string& link_path = vars["LINK_PATH"];
  if (globalGen->IsGCCOnWindows()) {
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
  }

  // Device linking currently doesn't support response files so
  // do not check if the user has explicitly forced a response file.
  int const commandLineLengthLimit =
    static_cast<int>(cmSystemTools::CalculateCommandLineLengthLimit()) -
    globalGen->GetRuleCmdLength(build.Rule);

  build.RspFile = this->ConvertToNinjaPath(
    cmStrCat("CMakeFiles/", genTarget->GetName(),
             globalGen->IsMultiConfig() ? cmStrCat('.', config) : "", ".rsp"));

  // Gather order-only dependencies.
  this->GetLocalGenerator()->AppendTargetDepends(
    this->GetGeneratorTarget(), build.OrderOnlyDeps, config, config,
    DependOnTargetArtifact);

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen->WriteBuild(this->GetCommonFileStream(), build,
                        commandLineLengthLimit, &usedResponseFile);
  this->WriteNvidiaDeviceLinkRule(usedResponseFile, config);
}

void cmNinjaNormalTargetGenerator::WriteLinkStatement(
  const std::string& config, const std::string& fileConfig,
  bool firstForConfig)
{
  cmMakefile* mf = this->GetMakefile();
  cmGlobalNinjaGenerator* globalGen = this->GetGlobalGenerator();
  cmGeneratorTarget* gt = this->GetGeneratorTarget();

  std::string targetOutput = this->ConvertToNinjaPath(gt->GetFullPath(config));
  std::string targetOutputReal = this->ConvertToNinjaPath(
    gt->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact,
                    /*realname=*/true));
  std::string targetOutputImplib = this->ConvertToNinjaPath(
    gt->GetFullPath(config, cmStateEnums::ImportLibraryArtifact));

  if (config != fileConfig) {
    if (targetOutput ==
        this->ConvertToNinjaPath(gt->GetFullPath(fileConfig))) {
      return;
    }
    if (targetOutputReal ==
        this->ConvertToNinjaPath(
          gt->GetFullPath(fileConfig, cmStateEnums::RuntimeBinaryArtifact,
                          /*realname=*/true))) {
      return;
    }
    if (!gt->GetFullName(config, cmStateEnums::ImportLibraryArtifact)
           .empty() &&
        !gt->GetFullName(fileConfig, cmStateEnums::ImportLibraryArtifact)
           .empty() &&
        targetOutputImplib ==
          this->ConvertToNinjaPath(gt->GetFullPath(
            fileConfig, cmStateEnums::ImportLibraryArtifact))) {
      return;
    }
  }

  auto const tgtNames = this->TargetNames(config);
  if (gt->IsAppBundleOnApple()) {
    // Create the app bundle
    std::string outpath = gt->GetDirectory(config);
    this->OSXBundleGenerator->CreateAppBundle(tgtNames.Output, outpath,
                                              config);

    // Calculate the output path
    targetOutput = cmStrCat(outpath, '/', tgtNames.Output);
    targetOutput = this->ConvertToNinjaPath(targetOutput);
    targetOutputReal = cmStrCat(outpath, '/', tgtNames.Real);
    targetOutputReal = this->ConvertToNinjaPath(targetOutputReal);
  } else if (gt->IsFrameworkOnApple()) {
    // Create the library framework.

    cmOSXBundleGenerator::SkipParts bundleSkipParts;
    if (globalGen->GetName() == "Ninja Multi-Config") {
      const auto postFix = this->GeneratorTarget->GetFilePostfix(config);
      // Skip creating Info.plist when there are multiple configurations, and
      // the current configuration has a postfix. The non-postfix configuration
      // Info.plist can be used by all the other configurations.
      if (!postFix.empty()) {
        bundleSkipParts.infoPlist = true;
      }
    }

    this->OSXBundleGenerator->CreateFramework(
      tgtNames.Output, gt->GetDirectory(config), config, bundleSkipParts);
  } else if (gt->IsCFBundleOnApple()) {
    // Create the core foundation bundle.
    this->OSXBundleGenerator->CreateCFBundle(tgtNames.Output,
                                             gt->GetDirectory(config), config);
  }

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetImplFileStream(fileConfig));
  const cmStateEnums::TargetType targetType = gt->GetType();
  this->GetImplFileStream(fileConfig)
    << "# Link build statements for " << cmState::GetTargetTypeName(targetType)
    << " target " << this->GetTargetName() << "\n\n";

  cmNinjaBuild linkBuild(this->LanguageLinkerRule(config));
  cmNinjaVars& vars = linkBuild.Variables;

  // Compute the comment.
  linkBuild.Comment =
    cmStrCat("Link the ", this->GetVisibleTypeName(), ' ', targetOutputReal);

  // Compute outputs.
  linkBuild.Outputs.push_back(targetOutputReal);
  if (firstForConfig) {
    globalGen->GetByproductsForCleanTarget(config).push_back(targetOutputReal);
  }

  if (this->TargetLinkLanguage(config) == "Swift") {
    vars["SWIFT_LIBRARY_NAME"] = [this, config]() -> std::string {
      cmGeneratorTarget::Names targetNames =
        this->GetGeneratorTarget()->GetLibraryNames(config);
      return targetNames.Base;
    }();

    vars["SWIFT_MODULE_NAME"] = [gt]() -> std::string {
      if (cmValue name = gt->GetProperty("Swift_MODULE_NAME")) {
        return *name;
      }
      return gt->GetName();
    }();

    vars["SWIFT_MODULE"] = [this](const std::string& module) -> std::string {
      std::string directory =
        this->GetLocalGenerator()->GetCurrentBinaryDirectory();
      if (cmValue prop = this->GetGeneratorTarget()->GetProperty(
            "Swift_MODULE_DIRECTORY")) {
        directory = *prop;
      }

      std::string name = module + ".swiftmodule";
      if (cmValue prop =
            this->GetGeneratorTarget()->GetProperty("Swift_MODULE")) {
        name = *prop;
      }

      return this->GetLocalGenerator()->ConvertToOutputFormat(
        this->ConvertToNinjaPath(directory + "/" + name),
        cmOutputConverter::SHELL);
    }(vars["SWIFT_MODULE_NAME"]);

    const std::string map = cmStrCat(gt->GetSupportDirectory(), '/', config,
                                     '/', "output-file-map.json");
    vars["SWIFT_OUTPUT_FILE_MAP"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->ConvertToNinjaPath(map), cmOutputConverter::SHELL);

    vars["SWIFT_SOURCES"] = [this, config]() -> std::string {
      std::vector<cmSourceFile const*> sources;
      std::stringstream oss;

      this->GetGeneratorTarget()->GetObjectSources(sources, config);
      cmLocalGenerator const* LocalGen = this->GetLocalGenerator();
      for (const auto& source : sources) {
        oss << " "
            << LocalGen->ConvertToOutputFormat(
                 this->GetCompiledSourceNinjaPath(source),
                 cmOutputConverter::SHELL);
      }
      return oss.str();
    }();

    // Since we do not perform object builds, compute the
    // defines/flags/includes here so that they can be passed along
    // appropriately.
    vars["DEFINES"] = this->GetDefines("Swift", config);
    vars["FLAGS"] = this->GetFlags("Swift", config);
    vars["INCLUDES"] = this->GetIncludes("Swift", config);
  }

  // Compute specific libraries to link with.
  if (this->TargetLinkLanguage(config) == "Swift") {
    std::vector<cmSourceFile const*> sources;
    gt->GetObjectSources(sources, config);
    for (const auto& source : sources) {
      linkBuild.Outputs.push_back(
        this->ConvertToNinjaPath(this->GetObjectFilePath(source, config)));
      linkBuild.ExplicitDeps.emplace_back(
        this->GetCompiledSourceNinjaPath(source));
    }
    linkBuild.Outputs.push_back(vars["SWIFT_MODULE"]);
  } else {
    linkBuild.ExplicitDeps = this->GetObjects(config);
  }

  std::vector<std::string> extraISPCObjects =
    this->GetGeneratorTarget()->GetGeneratedISPCObjects(config);
  std::transform(extraISPCObjects.begin(), extraISPCObjects.end(),
                 std::back_inserter(linkBuild.ExplicitDeps),
                 this->MapToNinjaPath());

  linkBuild.ImplicitDeps =
    this->ComputeLinkDeps(this->TargetLinkLanguage(config), config);

  if (!this->DeviceLinkObject.empty()) {
    linkBuild.ExplicitDeps.push_back(this->DeviceLinkObject);
  }

  std::string frameworkPath;
  std::string linkPath;

  std::string createRule =
    gt->GetCreateRuleVariable(this->TargetLinkLanguage(config), config);
  bool useWatcomQuote = mf->IsOn(createRule + "_USE_WATCOM_QUOTE");
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(targetOutputReal, cmOutputConverter::SHELL);

  std::unique_ptr<cmLinkLineComputer> linkLineComputer =
    globalGen->CreateLinkLineComputer(
      this->GetLocalGenerator(),
      this->GetLocalGenerator()->GetStateSnapshot().GetDirectory());
  linkLineComputer->SetUseWatcomQuote(useWatcomQuote);
  linkLineComputer->SetUseNinjaMulti(globalGen->IsMultiConfig());

  localGen.GetTargetFlags(linkLineComputer.get(), config,
                          vars["LINK_LIBRARIES"], vars["FLAGS"],
                          vars["LINK_FLAGS"], frameworkPath, linkPath, gt);

  // Add OS X version flags, if any.
  if (this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(vars["LINK_FLAGS"],
                           this->TargetLinkLanguage(config), "COMPATIBILITY",
                           true);
    this->AppendOSXVerFlag(vars["LINK_FLAGS"],
                           this->TargetLinkLanguage(config), "CURRENT", false);
  }

  this->addPoolNinjaVariable("JOB_POOL_LINK", gt, vars);

  this->AddModuleDefinitionFlag(linkLineComputer.get(), vars["LINK_FLAGS"],
                                config);

  this->UseLWYU = this->GetLocalGenerator()->AppendLWYUFlags(
    vars["LINK_FLAGS"], this->GetGeneratorTarget(),
    this->TargetLinkLanguage(config));

  vars["LINK_FLAGS"] = globalGen->EncodeLiteral(vars["LINK_FLAGS"]);

  vars["MANIFESTS"] = this->GetManifests(config);
  vars["AIX_EXPORTS"] = this->GetAIXExports(config);

  vars["LINK_PATH"] = frameworkPath + linkPath;

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  if (targetType == cmStateEnums::EXECUTABLE) {
    std::string t = vars["FLAGS"];
    localGen.AddArchitectureFlags(t, gt, this->TargetLinkLanguage(config),
                                  config);
    vars["FLAGS"] = t;
  } else {
    std::string t = vars["ARCH_FLAGS"];
    localGen.AddArchitectureFlags(t, gt, this->TargetLinkLanguage(config),
                                  config);
    vars["ARCH_FLAGS"] = t;
    t.clear();
    localGen.AddLanguageFlagsForLinking(
      t, gt, this->TargetLinkLanguage(config), config);
    vars["LANGUAGE_COMPILE_FLAGS"] = t;
  }
  if (gt->HasSOName(config)) {
    vars["SONAME_FLAG"] = mf->GetSONameFlag(this->TargetLinkLanguage(config));
    vars["SONAME"] = localGen.ConvertToOutputFormat(tgtNames.SharedObject,
                                                    cmOutputConverter::SHELL);
    if (targetType == cmStateEnums::SHARED_LIBRARY) {
      std::string install_dir = gt->GetInstallNameDirForBuildTree(config);
      if (!install_dir.empty()) {
        vars["INSTALLNAME_DIR"] = localGen.ConvertToOutputFormat(
          install_dir, cmOutputConverter::SHELL);
      }
    }
  }

  cmGlobalNinjaGenerator::CCOutputs byproducts(this->GetGlobalGenerator());

  if (!tgtNames.ImportLibrary.empty()) {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
      targetOutputImplib, cmOutputConverter::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    this->EnsureParentDirectoryExists(targetOutputImplib);
    if (gt->HasImportLibrary(config)) {
      // Some linkers may update a binary without touching its import lib.
      byproducts.ExplicitOuts.emplace_back(targetOutputImplib);
      if (firstForConfig) {
        globalGen->GetByproductsForCleanTarget(config).push_back(
          targetOutputImplib);
      }
    }
  }

  if (!this->SetMsvcTargetPdbVariable(vars, config)) {
    // It is common to place debug symbols at a specific place,
    // so we need a plain target name in the rule available.
    std::string prefix;
    std::string base;
    std::string suffix;
    gt->GetFullNameComponents(prefix, base, suffix, config);
    std::string dbg_suffix = ".dbg";
    // TODO: Where to document?
    if (cmValue d = mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX")) {
      dbg_suffix = *d;
    }
    vars["TARGET_PDB"] = base + suffix + dbg_suffix;
  }

  const std::string objPath =
    cmStrCat(gt->GetSupportDirectory(), globalGen->ConfigDirectory(config));
  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(objPath), cmOutputConverter::SHELL);
  this->EnsureDirectoryExists(objPath);

  std::string& linkLibraries = vars["LINK_LIBRARIES"];
  std::string& link_path = vars["LINK_PATH"];
  if (globalGen->IsGCCOnWindows()) {
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
  }

  const std::vector<cmCustomCommand>* cmdLists[3] = {
    &gt->GetPreBuildCommands(), &gt->GetPreLinkCommands(),
    &gt->GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines;
  std::vector<std::string> postBuildCmdLines;

  std::vector<std::string>* cmdLineLists[3] = { &preLinkCmdLines,
                                                &preLinkCmdLines,
                                                &postBuildCmdLines };

  for (unsigned i = 0; i != 3; ++i) {
    for (cmCustomCommand const& cc : *cmdLists[i]) {
      if (config == fileConfig ||
          this->GetLocalGenerator()->HasUniqueByproducts(cc.GetByproducts(),
                                                         cc.GetBacktrace())) {
        cmCustomCommandGenerator ccg(cc, fileConfig, this->GetLocalGenerator(),
                                     true, config);
        localGen.AppendCustomCommandLines(ccg, *cmdLineLists[i]);
        std::vector<std::string> const& ccByproducts = ccg.GetByproducts();
        byproducts.Add(ccByproducts);
        std::transform(
          ccByproducts.begin(), ccByproducts.end(),
          std::back_inserter(globalGen->GetByproductsForCleanTarget()),
          this->MapToNinjaPath());
      }
    }
  }

  // maybe create .def file from list of objects
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    gt->GetModuleDefinitionInfo(config);
  if (mdi && mdi->DefFileGenerated) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
    std::string cmd =
      cmStrCat(cmakeCommand, " -E __create_def ",
               this->GetLocalGenerator()->ConvertToOutputFormat(
                 mdi->DefFile, cmOutputConverter::SHELL),
               ' ');
    std::string obj_list_file = mdi->DefFile + ".objs";
    cmd += this->GetLocalGenerator()->ConvertToOutputFormat(
      obj_list_file, cmOutputConverter::SHELL);

    cmValue nm_executable = this->GetMakefile()->GetDefinition("CMAKE_NM");
    if (cmNonempty(nm_executable)) {
      cmd += " --nm=";
      cmd += this->LocalCommonGenerator->ConvertToOutputFormat(
        *nm_executable, cmOutputConverter::SHELL);
    }
    preLinkCmdLines.push_back(std::move(cmd));

    // create a list of obj files for the -E __create_def to read
    cmGeneratedFileStream fout(obj_list_file);

    if (mdi->WindowsExportAllSymbols) {
      cmNinjaDeps objs = this->GetObjects(config);
      for (std::string const& obj : objs) {
        if (cmHasLiteralSuffix(obj, ".obj")) {
          fout << obj << "\n";
        }
      }
    }

    for (cmSourceFile const* src : mdi->Sources) {
      fout << src->GetFullPath() << "\n";
    }
  }
  // If we have any PRE_LINK commands, we need to go back to CMAKE_BINARY_DIR
  // for the link commands.
  if (!preLinkCmdLines.empty()) {
    const std::string homeOutDir = localGen.ConvertToOutputFormat(
      localGen.GetBinaryDirectory(), cmOutputConverter::SHELL);
    preLinkCmdLines.push_back("cd " + homeOutDir);
  }

  vars["PRE_LINK"] = localGen.BuildCommandLine(
    preLinkCmdLines, config, fileConfig, "pre-link", this->GeneratorTarget);
  std::string postBuildCmdLine =
    localGen.BuildCommandLine(postBuildCmdLines, config, fileConfig,
                              "post-build", this->GeneratorTarget);

  cmNinjaVars symlinkVars;
  bool const symlinkNeeded =
    (targetOutput != targetOutputReal && !gt->IsFrameworkOnApple());
  if (!symlinkNeeded) {
    vars["POST_BUILD"] = postBuildCmdLine;
  } else {
    vars["POST_BUILD"] = cmGlobalNinjaGenerator::SHELL_NOOP;
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
  }

  std::string cmakeVarLang =
    cmStrCat("CMAKE_", this->TargetLinkLanguage(config));

  // build response file name
  std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";

  cmValue flag = this->GetMakefile()->GetDefinition(cmakeLinkVar);

  bool const lang_supports_response =
    !(this->TargetLinkLanguage(config) == "RC" ||
      (this->TargetLinkLanguage(config) == "CUDA" && !flag));
  int commandLineLengthLimit = -1;
  if (!lang_supports_response || !this->ForceResponseFile()) {
    commandLineLengthLimit =
      static_cast<int>(cmSystemTools::CalculateCommandLineLengthLimit()) -
      globalGen->GetRuleCmdLength(linkBuild.Rule);
  }

  linkBuild.RspFile = this->ConvertToNinjaPath(
    cmStrCat("CMakeFiles/", gt->GetName(),
             globalGen->IsMultiConfig() ? cmStrCat('.', config) : "", ".rsp"));

  // Gather order-only dependencies.
  this->GetLocalGenerator()->AppendTargetDepends(
    gt, linkBuild.OrderOnlyDeps, config, fileConfig, DependOnTargetArtifact);

  // Add order-only dependencies on versioning symlinks of shared libs we link.
  if (!this->GeneratorTarget->IsDLLPlatform()) {
    if (cmComputeLinkInformation* cli =
          this->GeneratorTarget->GetLinkInformation(config)) {
      for (auto const& item : cli->GetItems()) {
        if (item.Target &&
            item.Target->GetType() == cmStateEnums::SHARED_LIBRARY &&
            !item.Target->IsFrameworkOnApple()) {
          std::string const& lib =
            this->ConvertToNinjaPath(item.Target->GetFullPath(config));
          if (std::find(linkBuild.ImplicitDeps.begin(),
                        linkBuild.ImplicitDeps.end(),
                        lib) == linkBuild.ImplicitDeps.end()) {
            linkBuild.OrderOnlyDeps.emplace_back(lib);
          }
        }
      }
    }
  }

  // Ninja should restat after linking if and only if there are byproducts.
  vars["RESTAT"] = byproducts.ExplicitOuts.empty() ? "" : "1";

  linkBuild.Outputs.reserve(linkBuild.Outputs.size() +
                            byproducts.ExplicitOuts.size());
  std::move(byproducts.ExplicitOuts.begin(), byproducts.ExplicitOuts.end(),
            std::back_inserter(linkBuild.Outputs));
  linkBuild.WorkDirOuts = std::move(byproducts.WorkDirOuts);

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen->WriteBuild(this->GetImplFileStream(fileConfig), linkBuild,
                        commandLineLengthLimit, &usedResponseFile);
  this->WriteLinkRule(usedResponseFile, config);

  if (symlinkNeeded) {
    if (targetType == cmStateEnums::EXECUTABLE) {
      cmNinjaBuild build("CMAKE_SYMLINK_EXECUTABLE");
      build.Comment = "Create executable symlink " + targetOutput;
      build.Outputs.push_back(targetOutput);
      if (firstForConfig) {
        globalGen->GetByproductsForCleanTarget(config).push_back(targetOutput);
      }
      build.ExplicitDeps.push_back(targetOutputReal);
      build.Variables = std::move(symlinkVars);
      globalGen->WriteBuild(this->GetImplFileStream(fileConfig), build);
    } else {
      cmNinjaBuild build("CMAKE_SYMLINK_LIBRARY");
      build.Comment = "Create library symlink " + targetOutput;

      std::string const soName = this->ConvertToNinjaPath(
        this->GetTargetFilePath(tgtNames.SharedObject, config));
      // If one link has to be created.
      if (targetOutputReal == soName || targetOutput == soName) {
        symlinkVars["SONAME"] =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            soName, cmOutputConverter::SHELL);
      } else {
        symlinkVars["SONAME"].clear();
        build.Outputs.push_back(soName);
        if (firstForConfig) {
          globalGen->GetByproductsForCleanTarget(config).push_back(soName);
        }
      }
      build.Outputs.push_back(targetOutput);
      if (firstForConfig) {
        globalGen->GetByproductsForCleanTarget(config).push_back(targetOutput);
      }
      build.ExplicitDeps.push_back(targetOutputReal);
      build.Variables = std::move(symlinkVars);

      globalGen->WriteBuild(this->GetImplFileStream(fileConfig), build);
    }
  }

  // Add aliases for the file name and the target name.
  globalGen->AddTargetAlias(tgtNames.Output, gt, config);
  globalGen->AddTargetAlias(this->GetTargetName(), gt, config);
}

void cmNinjaNormalTargetGenerator::WriteObjectLibStatement(
  const std::string& config)
{
  // Write a phony output that depends on all object files.
  {
    cmNinjaBuild build("phony");
    build.Comment = "Object library " + this->GetTargetName();
    this->GetLocalGenerator()->AppendTargetOutputs(this->GetGeneratorTarget(),
                                                   build.Outputs, config);
    this->GetLocalGenerator()->AppendTargetOutputs(
      this->GetGeneratorTarget(),
      this->GetGlobalGenerator()->GetByproductsForCleanTarget(config), config);
    build.ExplicitDeps = this->GetObjects(config);
    this->GetGlobalGenerator()->WriteBuild(this->GetCommonFileStream(), build);
  }

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(
    this->GetTargetName(), this->GetGeneratorTarget(), config);
}

cmGeneratorTarget::Names cmNinjaNormalTargetGenerator::TargetNames(
  const std::string& config) const
{
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
    return this->GeneratorTarget->GetExecutableNames(config);
  }
  return this->GeneratorTarget->GetLibraryNames(config);
}

std::string cmNinjaNormalTargetGenerator::TargetLinkLanguage(
  const std::string& config) const
{
  return this->GeneratorTarget->GetLinkerLanguage(config);
}
