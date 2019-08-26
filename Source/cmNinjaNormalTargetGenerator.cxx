/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaNormalTargetGenerator.h"

#include <algorithm>
#include <assert.h>
#include <iterator>
#include <map>
#include <memory> // IWYU pragma: keep
#include <set>
#include <sstream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmCustomCommand.h" // IWYU pragma: keep
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
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
#include "cmSystemTools.h"

cmNinjaNormalTargetGenerator::cmNinjaNormalTargetGenerator(
  cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target)
  , TargetLinkLanguage("")
{
  this->TargetLinkLanguage = target->GetLinkerLanguage(this->GetConfigName());
  if (target->GetType() == cmStateEnums::EXECUTABLE) {
    this->TargetNames = this->GetGeneratorTarget()->GetExecutableNames(
      GetLocalGenerator()->GetConfigName());
  } else {
    this->TargetNames = this->GetGeneratorTarget()->GetLibraryNames(
      GetLocalGenerator()->GetConfigName());
  }

  if (target->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    EnsureDirectoryExists(target->GetDirectory(this->GetConfigName()));
  }

  this->OSXBundleGenerator =
    cm::make_unique<cmOSXBundleGenerator>(target, this->GetConfigName());
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator() = default;

void cmNinjaNormalTargetGenerator::Generate()
{
  if (this->TargetLinkLanguage.empty()) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: " +
                         this->GetGeneratorTarget()->GetName());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules();

  // Write the build statements
  this->WriteObjectBuildStatements();

  if (this->GetGeneratorTarget()->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    this->WriteObjectLibStatement();
  } else {
    // If this target has cuda language link inputs, and we need to do
    // device linking
    this->WriteDeviceLinkStatement();
    this->WriteLinkStatement();
  }

  // Find ADDITIONAL_CLEAN_FILES
  this->AdditionalCleanFiles();
}

void cmNinjaNormalTargetGenerator::WriteLanguagesRules()
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
  this->GetGeneratorTarget()->GetObjectSources(
    sourceFiles, this->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE"));
  for (cmSourceFile const* sf : sourceFiles) {
    std::string const lang = sf->GetLanguage();
    if (!lang.empty()) {
      languages.insert(lang);
    }
  }
  for (std::string const& language : languages) {
    this->WriteLanguageRules(language);
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

std::string cmNinjaNormalTargetGenerator::LanguageLinkerRule() const
{
  return this->TargetLinkLanguage + "_" +
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()) +
    "_LINKER__" +
    cmGlobalNinjaGenerator::EncodeRuleName(
           this->GetGeneratorTarget()->GetName());
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerDeviceRule() const
{
  return this->TargetLinkLanguage + "_" +
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()) +
    "_DEVICE_LINKER__" +
    cmGlobalNinjaGenerator::EncodeRuleName(
           this->GetGeneratorTarget()->GetName());
}

struct cmNinjaRemoveNoOpCommands
{
  bool operator()(std::string const& cmd)
  {
    return cmd.empty() || cmd[0] == ':';
  }
};

void cmNinjaNormalTargetGenerator::WriteDeviceLinkRule(bool useResponseFile)
{
  cmNinjaRule rule(this->LanguageLinkerDeviceRule());
  if (!this->GetGlobalGenerator()->HasRule(rule.Name)) {
    cmRulePlaceholderExpander::RuleVariables vars;
    vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
    vars.CMTargetType =
      cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType());

    vars.Language = "CUDA";

    // build response file name
    std::string responseFlag = this->GetMakefile()->GetSafeDefinition(
      "CMAKE_CUDA_RESPONSE_FILE_LINK_FLAG");

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

    std::string langFlags;
    if (this->GetGeneratorTarget()->GetType() != cmStateEnums::EXECUTABLE) {
      langFlags += "$LANGUAGE_COMPILE_FLAGS $ARCH_FLAGS";
      vars.LanguageCompileFlags = langFlags.c_str();
    }

    std::string launcher;
    const char* val = this->GetLocalGenerator()->GetRuleLauncher(
      this->GetGeneratorTarget(), "RULE_LAUNCH_LINK");
    if (val && *val) {
      launcher = val;
      launcher += " ";
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->GetLocalGenerator()->CreateRulePlaceholderExpander());

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeDeviceLinkCmd();
    for (std::string& linkCmd : linkCmds) {
      linkCmd = launcher + linkCmd;
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   linkCmd, vars);
    }

    // If there is no ranlib the command will be ":".  Skip it.
    cmEraseIf(linkCmds, cmNinjaRemoveNoOpCommands());

    rule.Command = this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule with response file if needed.
    rule.Comment = "Rule for linking ";
    rule.Comment += this->TargetLinkLanguage;
    rule.Comment += " ";
    rule.Comment += this->GetVisibleTypeName();
    rule.Comment += ".";
    rule.Description = "Linking ";
    rule.Description += this->TargetLinkLanguage;
    rule.Description += " ";
    rule.Description += this->GetVisibleTypeName();
    rule.Description += " $TARGET_FILE";
    rule.Restat = "$RESTAT";

    this->GetGlobalGenerator()->AddRule(rule);
  }
}

void cmNinjaNormalTargetGenerator::WriteLinkRule(bool useResponseFile)
{
  cmStateEnums::TargetType targetType = this->GetGeneratorTarget()->GetType();

  std::string linkRuleName = this->LanguageLinkerRule();
  if (!this->GetGlobalGenerator()->HasRule(linkRuleName)) {
    cmNinjaRule rule(std::move(linkRuleName));
    cmRulePlaceholderExpander::RuleVariables vars;
    vars.CMTargetName = this->GetGeneratorTarget()->GetName().c_str();
    vars.CMTargetType = cmState::GetTargetTypeName(targetType);

    vars.Language = this->TargetLinkLanguage.c_str();

    if (this->TargetLinkLanguage == "Swift") {
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

    std::string cmakeVarLang = "CMAKE_";
    cmakeVarLang += this->TargetLinkLanguage;

    // build response file name
    std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";
    const char* flag = GetMakefile()->GetDefinition(cmakeLinkVar);

    if (flag) {
      responseFlag = flag;
    } else if (this->TargetLinkLanguage != "CUDA") {
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
      if (this->TargetLinkLanguage == "Swift") {
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

    std::string launcher;
    const char* val = this->GetLocalGenerator()->GetRuleLauncher(
      this->GetGeneratorTarget(), "RULE_LAUNCH_LINK");
    if (val && *val) {
      launcher = val;
      launcher += " ";
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->GetLocalGenerator()->CreateRulePlaceholderExpander());

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd();
    for (std::string& linkCmd : linkCmds) {
      linkCmd = launcher + linkCmd;
      rulePlaceholderExpander->ExpandRuleVariables(this->GetLocalGenerator(),
                                                   linkCmd, vars);
    }

    // If there is no ranlib the command will be ":".  Skip it.
    cmEraseIf(linkCmds, cmNinjaRemoveNoOpCommands());

    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.emplace_back("$POST_BUILD");
    rule.Command = this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule with response file if needed.
    rule.Comment = "Rule for linking ";
    rule.Comment += this->TargetLinkLanguage;
    rule.Comment += " ";
    rule.Comment += this->GetVisibleTypeName();
    rule.Comment += ".";
    rule.Description = "Linking ";
    rule.Description += this->TargetLinkLanguage;
    rule.Description += " ";
    rule.Description += this->GetVisibleTypeName();
    rule.Description += " $TARGET_FILE";
    rule.Restat = "$RESTAT";
    this->GetGlobalGenerator()->AddRule(rule);
  }

  if (this->TargetNames.Output != this->TargetNames.Real &&
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
        rule.Command = this->GetLocalGenerator()->BuildCommandLine(cmd);
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
        rule.Command = this->GetLocalGenerator()->BuildCommandLine(cmd);
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
      const std::string cudaLinkCmd(
        this->GetMakefile()->GetDefinition("CMAKE_CUDA_DEVICE_LINK_LIBRARY"));
      cmSystemTools::ExpandListArgument(cudaLinkCmd, linkCmds);
    } break;
    case cmStateEnums::EXECUTABLE: {
      const std::string cudaLinkCmd(this->GetMakefile()->GetDefinition(
        "CMAKE_CUDA_DEVICE_LINK_EXECUTABLE"));
      cmSystemTools::ExpandListArgument(cudaLinkCmd, linkCmds);
    } break;
    default:
      break;
  }
  return linkCmds;
}

std::vector<std::string> cmNinjaNormalTargetGenerator::ComputeLinkCmd()
{
  std::vector<std::string> linkCmds;
  cmMakefile* mf = this->GetMakefile();
  {
    // If we have a rule variable prefer it. In the case of static libraries
    // this occurs when things like IPO is enabled, and we need to use the
    // CMAKE_<lang>_CREATE_STATIC_LIBRARY_IPO define instead.
    std::string linkCmdVar = this->GetGeneratorTarget()->GetCreateRuleVariable(
      this->TargetLinkLanguage, this->GetConfigName());
    const char* linkCmd = mf->GetDefinition(linkCmdVar);
    if (linkCmd) {
      std::string linkCmdStr = linkCmd;
      if (this->GetGeneratorTarget()->HasImplibGNUtoMS(this->ConfigName)) {
        std::string ruleVar = "CMAKE_";
        ruleVar += this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);
        ruleVar += "_GNUtoMS_RULE";
        if (const char* rule = this->Makefile->GetDefinition(ruleVar)) {
          linkCmdStr += rule;
        }
      }
      cmSystemTools::ExpandListArgument(linkCmdStr, linkCmds);
      if (this->GetGeneratorTarget()->GetPropertyAsBool("LINK_WHAT_YOU_USE")) {
        std::string cmakeCommand =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
        cmakeCommand += " -E __run_co_compile --lwyu=";
        cmGeneratorTarget& gt = *this->GetGeneratorTarget();
        const std::string cfgName = this->GetConfigName();
        std::string targetOutputReal = this->ConvertToNinjaPath(
          gt.GetFullPath(cfgName, cmStateEnums::RuntimeBinaryArtifact,
                         /*realname=*/true));
        cmakeCommand += targetOutputReal;
        linkCmds.push_back(std::move(cmakeCommand));
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
        linkCmds.push_back(cmakeCommand + " -E remove $TARGET_FILE");
      }
      // TODO: Use ARCHIVE_APPEND for archives over a certain size.
      {
        std::string linkCmdVar = "CMAKE_";
        linkCmdVar += this->TargetLinkLanguage;
        linkCmdVar += "_ARCHIVE_CREATE";

        linkCmdVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
          linkCmdVar, this->TargetLinkLanguage, this->GetConfigName());

        std::string const& linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      {
        std::string linkCmdVar = "CMAKE_";
        linkCmdVar += this->TargetLinkLanguage;
        linkCmdVar += "_ARCHIVE_FINISH";

        linkCmdVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
          linkCmdVar, this->TargetLinkLanguage, this->GetConfigName());

        std::string const& linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
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
      return linkCmds;
    }
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::EXECUTABLE:
      break;
    default:
      assert(false && "Unexpected target type");
  }
  return std::vector<std::string>();
}

void cmNinjaNormalTargetGenerator::WriteDeviceLinkStatement()
{
  cmGlobalNinjaGenerator* globalGen = this->GetGlobalGenerator();
  if (!globalGen->GetLanguageEnabled("CUDA")) {
    return;
  }

  cmGeneratorTarget* genTarget = this->GetGeneratorTarget();

  bool requiresDeviceLinking = requireDeviceLinking(
    *this->GeneratorTarget, *this->GetLocalGenerator(), this->ConfigName);
  if (!requiresDeviceLinking) {
    return;
  }

  // Now we can do device linking

  // First and very important step is to make sure while inside this
  // step our link language is set to CUDA
  std::string cudaLinkLanguage = "CUDA";
  std::string const& objExt =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_OUTPUT_EXTENSION");

  std::string const cfgName = this->GetConfigName();
  std::string const targetOutputReal = ConvertToNinjaPath(
    genTarget->ObjectDirectory + "cmake_device_link" + objExt);

  std::string const targetOutputImplib = ConvertToNinjaPath(
    genTarget->GetFullPath(cfgName, cmStateEnums::ImportLibraryArtifact));

  this->DeviceLinkObject = targetOutputReal;

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  const cmStateEnums::TargetType targetType = genTarget->GetType();
  this->GetBuildFileStream() << "# Device Link build statements for "
                             << cmState::GetTargetTypeName(targetType)
                             << " target " << this->GetTargetName() << "\n\n";

  // Compute the comment.
  cmNinjaBuild build(this->LanguageLinkerDeviceRule());
  build.Comment = "Link the ";
  build.Comment += this->GetVisibleTypeName();
  build.Comment += " ";
  build.Comment += targetOutputReal;

  cmNinjaVars& vars = build.Variables;

  // Compute outputs.
  build.Outputs.push_back(targetOutputReal);
  // Compute specific libraries to link with.
  build.ExplicitDeps = this->GetObjects();
  build.ImplicitDeps = this->ComputeLinkDeps(this->TargetLinkLanguage);

  std::string frameworkPath;
  std::string linkPath;

  std::string createRule = genTarget->GetCreateRuleVariable(
    this->TargetLinkLanguage, this->GetConfigName());
  const bool useWatcomQuote =
    this->GetMakefile()->IsOn(createRule + "_USE_WATCOM_QUOTE");
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(targetOutputReal, cmOutputConverter::SHELL);

  std::unique_ptr<cmLinkLineComputer> linkLineComputer(
    new cmNinjaLinkLineDeviceComputer(
      this->GetLocalGenerator(),
      this->GetLocalGenerator()->GetStateSnapshot().GetDirectory(),
      globalGen));
  linkLineComputer->SetUseWatcomQuote(useWatcomQuote);

  localGen.GetTargetFlags(
    linkLineComputer.get(), this->GetConfigName(), vars["LINK_LIBRARIES"],
    vars["FLAGS"], vars["LINK_FLAGS"], frameworkPath, linkPath, genTarget);

  this->addPoolNinjaVariable("JOB_POOL_LINK", genTarget, vars);

  vars["LINK_FLAGS"] =
    cmGlobalNinjaGenerator::EncodeLiteral(vars["LINK_FLAGS"]);

  vars["MANIFESTS"] = this->GetManifests();

  vars["LINK_PATH"] = frameworkPath + linkPath;

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  if (targetType == cmStateEnums::EXECUTABLE) {
    std::string t = vars["FLAGS"];
    localGen.AddArchitectureFlags(t, genTarget, cudaLinkLanguage, cfgName);
    vars["FLAGS"] = t;
  } else {
    std::string t = vars["ARCH_FLAGS"];
    localGen.AddArchitectureFlags(t, genTarget, cudaLinkLanguage, cfgName);
    vars["ARCH_FLAGS"] = t;
    t.clear();
    localGen.AddLanguageFlagsForLinking(t, genTarget, cudaLinkLanguage,
                                        cfgName);
    vars["LANGUAGE_COMPILE_FLAGS"] = t;
  }
  if (genTarget->HasSOName(cfgName)) {
    vars["SONAME_FLAG"] =
      this->GetMakefile()->GetSONameFlag(this->TargetLinkLanguage);
    vars["SONAME"] = this->TargetNames.SharedObject;
    if (targetType == cmStateEnums::SHARED_LIBRARY) {
      std::string install_dir =
        this->GetGeneratorTarget()->GetInstallNameDirForBuildTree(cfgName);
      if (!install_dir.empty()) {
        vars["INSTALLNAME_DIR"] = localGen.ConvertToOutputFormat(
          install_dir, cmOutputConverter::SHELL);
      }
    }
  }

  if (!this->TargetNames.ImportLibrary.empty()) {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
      targetOutputImplib, cmOutputConverter::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    EnsureParentDirectoryExists(impLibPath);
  }

  const std::string objPath = GetGeneratorTarget()->GetSupportDirectory();
  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(objPath), cmOutputConverter::SHELL);
  EnsureDirectoryExists(objPath);

  this->SetMsvcTargetPdbVariable(vars);

  if (globalGen->IsGCCOnWindows()) {
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::string& linkLibraries = vars["LINK_LIBRARIES"];
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::string& link_path = vars["LINK_PATH"];
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
  }

  // Device linking currently doesn't support response files so
  // do not check if the user has explicitly forced a response file.
  int const commandLineLengthLimit =
    static_cast<int>(cmSystemTools::CalculateCommandLineLengthLimit()) -
    globalGen->GetRuleCmdLength(this->LanguageLinkerDeviceRule());

  build.RspFile = this->ConvertToNinjaPath(std::string("CMakeFiles/") +
                                           genTarget->GetName() + ".rsp");

  // Gather order-only dependencies.
  this->GetLocalGenerator()->AppendTargetDepends(this->GetGeneratorTarget(),
                                                 build.OrderOnlyDeps);

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen->WriteBuild(this->GetBuildFileStream(), build,
                        commandLineLengthLimit, &usedResponseFile);
  this->WriteDeviceLinkRule(usedResponseFile);
}

void cmNinjaNormalTargetGenerator::WriteLinkStatement()
{
  cmMakefile* mf = this->GetMakefile();
  cmGlobalNinjaGenerator* globalGen = this->GetGlobalGenerator();
  cmGeneratorTarget* gt = this->GetGeneratorTarget();

  const std::string cfgName = this->GetConfigName();
  std::string targetOutput = ConvertToNinjaPath(gt->GetFullPath(cfgName));
  std::string targetOutputReal = ConvertToNinjaPath(
    gt->GetFullPath(cfgName, cmStateEnums::RuntimeBinaryArtifact,
                    /*realname=*/true));
  std::string targetOutputImplib = ConvertToNinjaPath(
    gt->GetFullPath(cfgName, cmStateEnums::ImportLibraryArtifact));

  if (gt->IsAppBundleOnApple()) {
    // Create the app bundle
    std::string outpath = gt->GetDirectory(cfgName);
    this->OSXBundleGenerator->CreateAppBundle(this->TargetNames.Output,
                                              outpath);

    // Calculate the output path
    targetOutput = outpath;
    targetOutput += "/";
    targetOutput += this->TargetNames.Output;
    targetOutput = this->ConvertToNinjaPath(targetOutput);
    targetOutputReal = outpath;
    targetOutputReal += "/";
    targetOutputReal += this->TargetNames.Real;
    targetOutputReal = this->ConvertToNinjaPath(targetOutputReal);
  } else if (gt->IsFrameworkOnApple()) {
    // Create the library framework.
    this->OSXBundleGenerator->CreateFramework(this->TargetNames.Output,
                                              gt->GetDirectory(cfgName));
  } else if (gt->IsCFBundleOnApple()) {
    // Create the core foundation bundle.
    this->OSXBundleGenerator->CreateCFBundle(this->TargetNames.Output,
                                             gt->GetDirectory(cfgName));
  }

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  const cmStateEnums::TargetType targetType = gt->GetType();
  this->GetBuildFileStream()
    << "# Link build statements for " << cmState::GetTargetTypeName(targetType)
    << " target " << this->GetTargetName() << "\n\n";

  cmNinjaBuild linkBuild(this->LanguageLinkerRule());
  cmNinjaVars& vars = linkBuild.Variables;

  // Compute the comment.
  linkBuild.Comment = "Link the ";
  linkBuild.Comment += this->GetVisibleTypeName();
  linkBuild.Comment += " ";
  linkBuild.Comment += targetOutputReal;

  // Compute outputs.
  linkBuild.Outputs.push_back(targetOutputReal);

  if (this->TargetLinkLanguage == "Swift") {
    vars["SWIFT_LIBRARY_NAME"] = [this]() -> std::string {
      cmGeneratorTarget::Names targetNames =
        this->GetGeneratorTarget()->GetLibraryNames(this->GetConfigName());
      return targetNames.Base;
    }();

    vars["SWIFT_MODULE_NAME"] = [gt]() -> std::string {
      if (const char* name = gt->GetProperty("Swift_MODULE_NAME")) {
        return name;
      }
      return gt->GetName();
    }();

    vars["SWIFT_MODULE"] = [this](const std::string& module) -> std::string {
      std::string directory =
        this->GetLocalGenerator()->GetCurrentBinaryDirectory();
      if (const char* prop = this->GetGeneratorTarget()->GetProperty(
            "Swift_MODULE_DIRECTORY")) {
        directory = prop;
      }

      std::string name = module + ".swiftmodule";
      if (const char* prop =
            this->GetGeneratorTarget()->GetProperty("Swift_MODULE")) {
        name = prop;
      }

      return this->GetLocalGenerator()->ConvertToOutputFormat(
        this->ConvertToNinjaPath(directory + "/" + name),
        cmOutputConverter::SHELL);
    }(vars["SWIFT_MODULE_NAME"]);

    vars["SWIFT_OUTPUT_FILE_MAP"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->ConvertToNinjaPath(gt->GetSupportDirectory() +
                                 "/output-file-map.json"),
        cmOutputConverter::SHELL);

    vars["SWIFT_SOURCES"] = [this]() -> std::string {
      std::vector<cmSourceFile const*> sources;
      std::stringstream oss;

      this->GetGeneratorTarget()->GetObjectSources(sources,
                                                   this->GetConfigName());
      cmLocalGenerator const* LocalGen = this->GetLocalGenerator();
      for (const auto& source : sources) {
        oss << " "
            << LocalGen->ConvertToOutputFormat(
                 this->ConvertToNinjaPath(this->GetSourceFilePath(source)),
                 cmOutputConverter::SHELL);
      }
      return oss.str();
    }();

    // Since we do not perform object builds, compute the
    // defines/flags/includes here so that they can be passed along
    // appropriately.
    vars["DEFINES"] = this->GetDefines("Swift");
    vars["FLAGS"] = this->GetFlags("Swift");
    vars["INCLUDES"] = this->GetIncludes("Swift");
  }

  // Compute specific libraries to link with.
  if (this->TargetLinkLanguage == "Swift") {
    std::vector<cmSourceFile const*> sources;
    gt->GetObjectSources(sources, this->GetConfigName());
    for (const auto& source : sources) {
      linkBuild.Outputs.push_back(
        this->ConvertToNinjaPath(this->GetObjectFilePath(source)));
      linkBuild.ExplicitDeps.push_back(
        this->ConvertToNinjaPath(this->GetSourceFilePath(source)));
    }

    linkBuild.Outputs.push_back(vars["SWIFT_MODULE"]);
  } else {
    linkBuild.ExplicitDeps = this->GetObjects();
  }
  linkBuild.ImplicitDeps = this->ComputeLinkDeps(this->TargetLinkLanguage);

  if (!this->DeviceLinkObject.empty()) {
    linkBuild.ExplicitDeps.push_back(this->DeviceLinkObject);
  }

  std::string frameworkPath;
  std::string linkPath;

  std::string createRule =
    gt->GetCreateRuleVariable(this->TargetLinkLanguage, this->GetConfigName());
  bool useWatcomQuote = mf->IsOn(createRule + "_USE_WATCOM_QUOTE");
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(targetOutputReal, cmOutputConverter::SHELL);

  std::unique_ptr<cmLinkLineComputer> linkLineComputer(
    globalGen->CreateLinkLineComputer(
      this->GetLocalGenerator(),
      this->GetLocalGenerator()->GetStateSnapshot().GetDirectory()));
  linkLineComputer->SetUseWatcomQuote(useWatcomQuote);

  localGen.GetTargetFlags(linkLineComputer.get(), this->GetConfigName(),
                          vars["LINK_LIBRARIES"], vars["FLAGS"],
                          vars["LINK_FLAGS"], frameworkPath, linkPath, gt);

  // Add OS X version flags, if any.
  if (this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(vars["LINK_FLAGS"], this->TargetLinkLanguage,
                           "COMPATIBILITY", true);
    this->AppendOSXVerFlag(vars["LINK_FLAGS"], this->TargetLinkLanguage,
                           "CURRENT", false);
  }

  this->addPoolNinjaVariable("JOB_POOL_LINK", gt, vars);

  this->AddModuleDefinitionFlag(linkLineComputer.get(), vars["LINK_FLAGS"]);
  vars["LINK_FLAGS"] =
    cmGlobalNinjaGenerator::EncodeLiteral(vars["LINK_FLAGS"]);

  vars["MANIFESTS"] = this->GetManifests();

  vars["LINK_PATH"] = frameworkPath + linkPath;
  std::string lwyuFlags;
  if (gt->GetPropertyAsBool("LINK_WHAT_YOU_USE")) {
    lwyuFlags = " -Wl,--no-as-needed";
  }

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  if (targetType == cmStateEnums::EXECUTABLE) {
    std::string t = vars["FLAGS"];
    localGen.AddArchitectureFlags(t, gt, TargetLinkLanguage, cfgName);
    t += lwyuFlags;
    vars["FLAGS"] = t;
  } else {
    std::string t = vars["ARCH_FLAGS"];
    localGen.AddArchitectureFlags(t, gt, TargetLinkLanguage, cfgName);
    vars["ARCH_FLAGS"] = t;
    t.clear();
    t += lwyuFlags;
    localGen.AddLanguageFlagsForLinking(t, gt, TargetLinkLanguage, cfgName);
    vars["LANGUAGE_COMPILE_FLAGS"] = t;
  }
  if (gt->HasSOName(cfgName)) {
    vars["SONAME_FLAG"] = mf->GetSONameFlag(this->TargetLinkLanguage);
    vars["SONAME"] = this->TargetNames.SharedObject;
    if (targetType == cmStateEnums::SHARED_LIBRARY) {
      std::string install_dir = gt->GetInstallNameDirForBuildTree(cfgName);
      if (!install_dir.empty()) {
        vars["INSTALLNAME_DIR"] = localGen.ConvertToOutputFormat(
          install_dir, cmOutputConverter::SHELL);
      }
    }
  }

  cmNinjaDeps byproducts;

  if (!this->TargetNames.ImportLibrary.empty()) {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
      targetOutputImplib, cmOutputConverter::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    EnsureParentDirectoryExists(impLibPath);
    if (gt->HasImportLibrary(cfgName)) {
      byproducts.push_back(targetOutputImplib);
    }
  }

  if (!this->SetMsvcTargetPdbVariable(vars)) {
    // It is common to place debug symbols at a specific place,
    // so we need a plain target name in the rule available.
    std::string prefix;
    std::string base;
    std::string suffix;
    gt->GetFullNameComponents(prefix, base, suffix);
    std::string dbg_suffix = ".dbg";
    // TODO: Where to document?
    if (mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX")) {
      dbg_suffix = mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX");
    }
    vars["TARGET_PDB"] = base + suffix + dbg_suffix;
  }

  const std::string objPath = gt->GetSupportDirectory();
  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(objPath), cmOutputConverter::SHELL);
  EnsureDirectoryExists(objPath);

  if (globalGen->IsGCCOnWindows()) {
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::string& linkLibraries = vars["LINK_LIBRARIES"];
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::string& link_path = vars["LINK_PATH"];
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
  }

  const std::vector<cmCustomCommand>* cmdLists[3] = {
    &gt->GetPreBuildCommands(), &gt->GetPreLinkCommands(),
    &gt->GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines, postBuildCmdLines;
  std::vector<std::string>* cmdLineLists[3] = { &preLinkCmdLines,
                                                &preLinkCmdLines,
                                                &postBuildCmdLines };

  for (unsigned i = 0; i != 3; ++i) {
    for (cmCustomCommand const& cc : *cmdLists[i]) {
      cmCustomCommandGenerator ccg(cc, cfgName, this->GetLocalGenerator());
      localGen.AppendCustomCommandLines(ccg, *cmdLineLists[i]);
      std::vector<std::string> const& ccByproducts = ccg.GetByproducts();
      std::transform(ccByproducts.begin(), ccByproducts.end(),
                     std::back_inserter(byproducts), MapToNinjaPath());
    }
  }

  // maybe create .def file from list of objects
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    gt->GetModuleDefinitionInfo(this->GetConfigName());
  if (mdi && mdi->DefFileGenerated) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
    std::string cmd = cmakeCommand;
    cmd += " -E __create_def ";
    cmd += this->GetLocalGenerator()->ConvertToOutputFormat(
      mdi->DefFile, cmOutputConverter::SHELL);
    cmd += " ";
    std::string obj_list_file = mdi->DefFile + ".objs";
    cmd += this->GetLocalGenerator()->ConvertToOutputFormat(
      obj_list_file, cmOutputConverter::SHELL);
    preLinkCmdLines.push_back(std::move(cmd));

    // create a list of obj files for the -E __create_def to read
    cmGeneratedFileStream fout(obj_list_file);

    if (mdi->WindowsExportAllSymbols) {
      cmNinjaDeps objs = this->GetObjects();
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

  vars["PRE_LINK"] = localGen.BuildCommandLine(preLinkCmdLines, "pre-link",
                                               this->GeneratorTarget);
  std::string postBuildCmdLine = localGen.BuildCommandLine(
    postBuildCmdLines, "post-build", this->GeneratorTarget);

  cmNinjaVars symlinkVars;
  bool const symlinkNeeded =
    (targetOutput != targetOutputReal && !gt->IsFrameworkOnApple());
  if (!symlinkNeeded) {
    vars["POST_BUILD"] = postBuildCmdLine;
  } else {
    vars["POST_BUILD"] = cmGlobalNinjaGenerator::SHELL_NOOP;
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
  }

  std::string cmakeVarLang = "CMAKE_";
  cmakeVarLang += this->TargetLinkLanguage;

  // build response file name
  std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";

  const char* flag = GetMakefile()->GetDefinition(cmakeLinkVar);

  bool const lang_supports_response =
    !(this->TargetLinkLanguage == "RC" ||
      (this->TargetLinkLanguage == "CUDA" && !flag));
  int commandLineLengthLimit = -1;
  if (!lang_supports_response || !this->ForceResponseFile()) {
    commandLineLengthLimit =
      static_cast<int>(cmSystemTools::CalculateCommandLineLengthLimit()) -
      globalGen->GetRuleCmdLength(linkBuild.Rule);
  }

  linkBuild.RspFile = this->ConvertToNinjaPath(std::string("CMakeFiles/") +
                                               gt->GetName() + ".rsp");

  // Gather order-only dependencies.
  this->GetLocalGenerator()->AppendTargetDepends(gt, linkBuild.OrderOnlyDeps);

  // Ninja should restat after linking if and only if there are byproducts.
  vars["RESTAT"] = byproducts.empty() ? "" : "1";

  for (std::string const& o : byproducts) {
    globalGen->SeenCustomCommandOutput(o);
    linkBuild.Outputs.push_back(o);
  }

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen->WriteBuild(this->GetBuildFileStream(), linkBuild,
                        commandLineLengthLimit, &usedResponseFile);
  this->WriteLinkRule(usedResponseFile);

  if (symlinkNeeded) {
    if (targetType == cmStateEnums::EXECUTABLE) {
      cmNinjaBuild build("CMAKE_SYMLINK_EXECUTABLE");
      build.Comment = "Create executable symlink " + targetOutput;
      build.Outputs.push_back(targetOutput);
      build.ExplicitDeps.push_back(targetOutputReal);
      build.Variables = std::move(symlinkVars);
      globalGen->WriteBuild(this->GetBuildFileStream(), build);
    } else {
      cmNinjaBuild build("CMAKE_SYMLINK_LIBRARY");
      build.Comment = "Create library symlink " + targetOutput;

      std::string const soName = this->ConvertToNinjaPath(
        this->GetTargetFilePath(this->TargetNames.SharedObject));
      // If one link has to be created.
      if (targetOutputReal == soName || targetOutput == soName) {
        symlinkVars["SONAME"] =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            soName, cmOutputConverter::SHELL);
      } else {
        symlinkVars["SONAME"].clear();
        build.Outputs.push_back(soName);
      }
      build.Outputs.push_back(targetOutput);
      build.ExplicitDeps.push_back(targetOutputReal);
      build.Variables = std::move(symlinkVars);

      globalGen->WriteBuild(this->GetBuildFileStream(), build);
    }
  }

  // Add aliases for the file name and the target name.
  globalGen->AddTargetAlias(this->TargetNames.Output, gt);
  globalGen->AddTargetAlias(this->GetTargetName(), gt);
}

void cmNinjaNormalTargetGenerator::WriteObjectLibStatement()
{
  // Write a phony output that depends on all object files.
  {
    cmNinjaBuild build("phony");
    build.Comment = "Object library " + this->GetTargetName();
    this->GetLocalGenerator()->AppendTargetOutputs(this->GetGeneratorTarget(),
                                                   build.Outputs);
    build.ExplicitDeps = this->GetObjects();
    this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(), build);
  }

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetGeneratorTarget());
}
