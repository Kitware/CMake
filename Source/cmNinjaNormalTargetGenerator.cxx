/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmNinjaNormalTargetGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"

#include <assert.h>
#include <algorithm>

#ifndef _WIN32
#include <unistd.h>
#endif


cmNinjaNormalTargetGenerator::
cmNinjaNormalTargetGenerator(cmTarget* target)
  : cmNinjaTargetGenerator(target)
  , TargetNameOut()
  , TargetNameSO()
  , TargetNameReal()
  , TargetNameImport()
  , TargetNamePDB()
  , TargetLinkLanguage(0)
{
  this->TargetLinkLanguage = target->GetLinkerLanguage(this->GetConfigName());
  if (target->GetType() == cmTarget::EXECUTABLE)
    target->GetExecutableNames(this->TargetNameOut,
                               this->TargetNameReal,
                               this->TargetNameImport,
                               this->TargetNamePDB,
                               GetLocalGenerator()->GetConfigName());
  else
    target->GetLibraryNames(this->TargetNameOut,
                            this->TargetNameSO,
                            this->TargetNameReal,
                            this->TargetNameImport,
                            this->TargetNamePDB,
                            GetLocalGenerator()->GetConfigName());

  if(target->GetType() != cmTarget::OBJECT_LIBRARY)
    {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    EnsureDirectoryExists(target->GetDirectory(this->GetConfigName()));
    }

  this->OSXBundleGenerator = new cmOSXBundleGenerator(target,
                                                      this->GetConfigName());
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator()
{
  delete this->OSXBundleGenerator;
}

void cmNinjaNormalTargetGenerator::Generate()
{
  if (!this->TargetLinkLanguage) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: ",
                         this->GetTarget()->GetName());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules();

  // Write the build statements
  this->WriteObjectBuildStatements();

  if(this->GetTarget()->GetType() == cmTarget::OBJECT_LIBRARY)
    {
    this->WriteObjectLibStatement();
    }
  else
    {
    this->WriteLinkRule(false); // write rule without rspfile support
    this->WriteLinkRule(true);  // write rule with rspfile support
    this->WriteLinkStatement();
    }
}

void cmNinjaNormalTargetGenerator::WriteLanguagesRules()
{
#ifdef NINJA_GEN_VERBOSE_FILES
  cmGlobalNinjaGenerator::WriteDivider(this->GetRulesFileStream());
  this->GetRulesFileStream()
    << "# Rules for each languages for "
    << cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    << " target "
    << this->GetTargetName()
    << "\n\n";
#endif

  std::set<cmStdString> languages;
  this->GetTarget()->GetLanguages(languages);
  for(std::set<cmStdString>::const_iterator l = languages.begin();
      l != languages.end();
      ++l)
    this->WriteLanguageRules(*l);
}

const char *cmNinjaNormalTargetGenerator::GetVisibleTypeName() const
{
  switch (this->GetTarget()->GetType()) {
    case cmTarget::STATIC_LIBRARY:
      return "static library";
    case cmTarget::SHARED_LIBRARY:
      return "shared library";
    case cmTarget::MODULE_LIBRARY:
      if (this->GetTarget()->IsCFBundleOnApple())
        return "CFBundle shared module";
      else
        return "shared module";
    case cmTarget::EXECUTABLE:
      return "executable";
    default:
      return 0;
  }
}

std::string
cmNinjaNormalTargetGenerator
::LanguageLinkerRule() const
{
  return std::string(this->TargetLinkLanguage)
    + "_"
    + cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    + "_LINKER";
}

void
cmNinjaNormalTargetGenerator
::WriteLinkRule(bool useResponseFile)
{
  cmTarget::TargetType targetType = this->GetTarget()->GetType();
  std::string ruleName = this->LanguageLinkerRule();
  if (useResponseFile)
    ruleName += "_RSP_FILE";

  // Select whether to use a response file for objects.
  std::string rspfile;
  std::string rspcontent;

  if (!this->GetGlobalGenerator()->HasRule(ruleName)) {
    cmLocalGenerator::RuleVariables vars;
    vars.RuleLauncher = "RULE_LAUNCH_LINK";
    vars.CMTarget = this->GetTarget();
    vars.Language = this->TargetLinkLanguage;

    std::string responseFlag;
    if (!useResponseFile) {
      vars.Objects = "$in";
      vars.LinkLibraries = "$LINK_PATH $LINK_LIBRARIES";
    } else {
        std::string cmakeVarLang = "CMAKE_";
        cmakeVarLang += this->TargetLinkLanguage;

        // build response file name
        std::string cmakeLinkVar =  cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";
        const char * flag = GetMakefile()->GetDefinition(cmakeLinkVar.c_str());
        if(flag) {
          responseFlag = flag;
        } else {
          responseFlag = "@";
        }
        rspfile = "$RSP_FILE";
        responseFlag += rspfile;

        // build response file content
        std::string linkOptionVar = cmakeVarLang;
        linkOptionVar += "_COMPILER_LINKER_OPTION_FLAG_";
        linkOptionVar += cmTarget::GetTargetTypeName(targetType);
        const std::string linkOption =
                GetMakefile()->GetSafeDefinition(linkOptionVar.c_str());
        rspcontent = "$in_newline "+linkOption+" $LINK_PATH $LINK_LIBRARIES";
        vars.Objects = responseFlag.c_str();
        vars.LinkLibraries = "";
    }

    vars.ObjectDir = "$OBJECT_DIR";

    // TODO:
    // Makefile generator expands <TARGET> to the plain target name
    // with suffix. $out expands to a relative path. This difference
    // could make trouble when switching to Ninja generator. Maybe
    // using TARGET_NAME and RuleVariables::TargetName is a fix.
    vars.Target = "$out";

    vars.SONameFlag = "$SONAME_FLAG";
    vars.TargetSOName = "$SONAME";
    vars.TargetInstallNameDir = "$INSTALLNAME_DIR";
    vars.TargetPDB = "$TARGET_PDB";

    // Setup the target version.
    std::string targetVersionMajor;
    std::string targetVersionMinor;
    {
    cmOStringStream majorStream;
    cmOStringStream minorStream;
    int major;
    int minor;
    this->GetTarget()->GetTargetVersion(major, minor);
    majorStream << major;
    minorStream << minor;
    targetVersionMajor = majorStream.str();
    targetVersionMinor = minorStream.str();
    }
    vars.TargetVersionMajor = targetVersionMajor.c_str();
    vars.TargetVersionMinor = targetVersionMinor.c_str();

    vars.Flags = "$FLAGS";
    vars.LinkFlags = "$LINK_FLAGS";

    std::string langFlags;
    if (targetType != cmTarget::EXECUTABLE) {
      this->GetLocalGenerator()->AddLanguageFlags(langFlags,
                                                  this->TargetLinkLanguage,
                                                  this->GetConfigName());
      langFlags += " $ARCH_FLAGS";
      vars.LanguageCompileFlags = langFlags.c_str();
    }

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd();
    for(std::vector<std::string>::iterator i = linkCmds.begin();
        i != linkCmds.end();
        ++i)
      {
      this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);
      }
    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.push_back("$POST_BUILD");
    std::string linkCmd =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule with response file if needed.
    cmOStringStream comment;
    comment << "Rule for linking " << this->TargetLinkLanguage << " "
            << this->GetVisibleTypeName() << ".";
    cmOStringStream description;
    description << "Linking " << this->TargetLinkLanguage << " "
                << this->GetVisibleTypeName() << " $out";
    this->GetGlobalGenerator()->AddRule(ruleName,
                                        linkCmd,
                                        description.str(),
                                        comment.str(),
                                        /*depfile*/ "",
                                        rspfile,
                                        rspcontent);
  }

  if (this->TargetNameOut != this->TargetNameReal) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND"),
        cmLocalGenerator::SHELL);
    if (targetType == cmTarget::EXECUTABLE)
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_EXECUTABLE",
                                          cmakeCommand +
                                          " -E cmake_symlink_executable"
                                          " $in $out && $POST_BUILD",
                                          "Creating executable symlink $out",
                                      "Rule for creating executable symlink.");
    else
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_LIBRARY",
                                          cmakeCommand +
                                          " -E cmake_symlink_library"
                                          " $in $SONAME $out && $POST_BUILD",
                                          "Creating library symlink $out",
                                         "Rule for creating library symlink.");
  }
}

std::vector<std::string>
cmNinjaNormalTargetGenerator
::ComputeLinkCmd()
{
  std::vector<std::string> linkCmds;
  cmTarget::TargetType targetType = this->GetTarget()->GetType();
  switch (targetType) {
    case cmTarget::STATIC_LIBRARY: {
      // Check if you have a non archive way to create the static library.
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_CREATE_STATIC_LIBRARY";
      if (const char *linkCmd =
            this->GetMakefile()->GetDefinition(linkCmdVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
        return linkCmds;
        }
      }

      // We have archive link commands set. First, delete the existing archive.
      std::string cmakeCommand =
        this->GetLocalGenerator()->ConvertToOutputFormat(
          this->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND"),
          cmLocalGenerator::SHELL);
      linkCmds.push_back(cmakeCommand + " -E remove $out");

      // TODO: Use ARCHIVE_APPEND for archives over a certain size.
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_CREATE";
      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_FINISH";
      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      return linkCmds;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::EXECUTABLE: {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      switch (targetType) {
      case cmTarget::SHARED_LIBRARY:
        linkCmdVar += "_CREATE_SHARED_LIBRARY";
        break;
      case cmTarget::MODULE_LIBRARY:
        linkCmdVar += "_CREATE_SHARED_MODULE";
        break;
      case cmTarget::EXECUTABLE:
        linkCmdVar += "_LINK_EXECUTABLE";
        break;
      default:
        assert(0 && "Unexpected target type");
      }

      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      return linkCmds;
    }
    default:
      assert(0 && "Unexpected target type");
  }
  return std::vector<std::string>();
}

void cmNinjaNormalTargetGenerator::WriteLinkStatement()
{
  cmTarget::TargetType targetType = this->GetTarget()->GetType();

  std::string targetOutput = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName()).c_str());
  std::string targetOutputReal = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName(),
                                   /*implib=*/false,
                                   /*realpath=*/true).c_str());
  std::string targetOutputImplib = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName(),
                                   /*implib=*/true).c_str());

  if (this->GetTarget()->IsAppBundleOnApple())
    {
    // Create the app bundle
    std::string outpath =
      this->GetTarget()->GetDirectory(this->GetConfigName());
    this->OSXBundleGenerator->CreateAppBundle(this->TargetNameOut, outpath);

    // Calculate the output path
    targetOutput = outpath;
    targetOutput += "/";
    targetOutput += this->TargetNameOut;
    targetOutput = this->ConvertToNinjaPath(targetOutput.c_str());
    targetOutputReal = outpath;
    targetOutputReal += "/";
    targetOutputReal += this->TargetNameReal;
    targetOutputReal = this->ConvertToNinjaPath(targetOutputReal.c_str());
    }
  else if (this->GetTarget()->IsFrameworkOnApple())
    {
    // Create the library framework.
    std::string outpath =
      this->GetTarget()->GetDirectory(this->GetConfigName());
    this->OSXBundleGenerator->CreateFramework(this->TargetNameOut, outpath);
    }
  else if(this->GetTarget()->IsCFBundleOnApple())
    {
    // Create the core foundation bundle.
    std::string outpath =
      this->GetTarget()->GetDirectory(this->GetConfigName());
    this->OSXBundleGenerator->CreateCFBundle(this->TargetNameOut, outpath);
    }

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  this->GetBuildFileStream()
    << "# Link build statements for "
    << cmTarget::GetTargetTypeName(targetType)
    << " target "
    << this->GetTargetName()
    << "\n\n";

  cmNinjaDeps emptyDeps;
  cmNinjaVars vars;

  // Compute the comment.
  cmOStringStream comment;
  comment << "Link the " << this->GetVisibleTypeName() << " "
          << targetOutputReal;

  // Compute outputs.
  cmNinjaDeps outputs;
  outputs.push_back(targetOutputReal);

  // Compute specific libraries to link with.
  cmNinjaDeps explicitDeps = this->GetObjects();
  cmNinjaDeps implicitDeps = this->ComputeLinkDeps();

  std::string frameworkPath;
  std::string linkPath;
  this->GetLocalGenerator()->GetTargetFlags(vars["LINK_LIBRARIES"],
                                            vars["FLAGS"],
                                            vars["LINK_FLAGS"],
                                            frameworkPath,
                                            linkPath,
                                            this->GetGeneratorTarget());

  this->AddModuleDefinitionFlag(vars["LINK_FLAGS"]);
  vars["LINK_FLAGS"] = cmGlobalNinjaGenerator
                        ::EncodeLiteral(vars["LINK_FLAGS"]);

  vars["LINK_PATH"] = frameworkPath + linkPath;

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  std::string flags = (targetType == cmTarget::EXECUTABLE
                               ? vars["FLAGS"]
                               : vars["ARCH_FLAGS"]);
  this->GetLocalGenerator()->AddArchitectureFlags(flags,
                             this->GetGeneratorTarget(),
                             this->TargetLinkLanguage,
                             this->GetConfigName());
  if (targetType == cmTarget::EXECUTABLE) {
    vars["FLAGS"] = flags;
  } else {
    vars["ARCH_FLAGS"] = flags;
  }
  if (this->GetTarget()->HasSOName(this->GetConfigName())) {
    vars["SONAME_FLAG"] =
      this->GetMakefile()->GetSONameFlag(this->TargetLinkLanguage);
    vars["SONAME"] = this->TargetNameSO;
    if (targetType == cmTarget::SHARED_LIBRARY) {
      std::string install_name_dir = this->GetTarget()
        ->GetInstallNameDirForBuildTree(this->GetConfigName());

      if (!install_name_dir.empty()) {
        vars["INSTALLNAME_DIR"] =
          this->GetLocalGenerator()->Convert(install_name_dir.c_str(),
              cmLocalGenerator::NONE,
              cmLocalGenerator::SHELL, false);
      }
    }
  }

  if (!this->TargetNameImport.empty()) {
    const std::string impLibPath = this->GetLocalGenerator()
      ->ConvertToOutputFormat(targetOutputImplib.c_str(),
                              cmLocalGenerator::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    EnsureParentDirectoryExists(impLibPath);
  }

  cmMakefile* mf = this->GetMakefile();
  if (!this->SetMsvcTargetPdbVariable(vars))
    {
    // It is common to place debug symbols at a specific place,
    // so we need a plain target name in the rule available.
    std::string prefix;
    std::string base;
    std::string suffix;
    this->GetTarget()->GetFullNameComponents(prefix, base, suffix);
    std::string dbg_suffix = ".dbg";
    // TODO: Where to document?
    if (mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX"))
      dbg_suffix = mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX");
    vars["TARGET_PDB"] = base + suffix + dbg_suffix;
    }

  if (mf->IsOn("CMAKE_COMPILER_IS_MINGW"))
    {
    const std::string objPath = GetTarget()->GetSupportDirectory();
    vars["OBJECT_DIR"] = ConvertToNinjaPath(objPath.c_str());
    EnsureDirectoryExists(objPath);
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::string& linkLibraries = vars["LINK_LIBRARIES"];
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    }

  std::vector<cmCustomCommand> *cmdLists[3] = {
    &this->GetTarget()->GetPreBuildCommands(),
    &this->GetTarget()->GetPreLinkCommands(),
    &this->GetTarget()->GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines, postBuildCmdLines;
  std::vector<std::string> *cmdLineLists[3] = {
    &preLinkCmdLines,
    &preLinkCmdLines,
    &postBuildCmdLines
  };

  for (unsigned i = 0; i != 3; ++i) {
    for (std::vector<cmCustomCommand>::const_iterator
         ci = cmdLists[i]->begin();
         ci != cmdLists[i]->end(); ++ci) {
      this->GetLocalGenerator()->AppendCustomCommandLines(&*ci,
                                                          *cmdLineLists[i]);
    }
  }

  // If we have any PRE_LINK commands, we need to go back to HOME_OUTPUT for
  // the link commands.
  if (!preLinkCmdLines.empty()) {
    const std::string homeOutDir = this->GetLocalGenerator()
      ->ConvertToOutputFormat(this->GetMakefile()->GetHomeOutputDirectory(),
                              cmLocalGenerator::SHELL);
    preLinkCmdLines.push_back("cd " + homeOutDir);
  }

  vars["PRE_LINK"] =
    this->GetLocalGenerator()->BuildCommandLine(preLinkCmdLines);
  std::string postBuildCmdLine =
    this->GetLocalGenerator()->BuildCommandLine(postBuildCmdLines);

  cmNinjaVars symlinkVars;
  if (targetOutput == targetOutputReal) {
    vars["POST_BUILD"] = postBuildCmdLine;
  } else {
    vars["POST_BUILD"] = ":";
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
  }

  int linkRuleLength = this->GetGlobalGenerator()->
                                 GetRuleCmdLength(this->LanguageLinkerRule());

  int commandLineLengthLimit = 1;
  const char* forceRspFile = "CMAKE_NINJA_FORCE_RESPONSE_FILE";
  if (!this->GetMakefile()->IsDefinitionSet(forceRspFile) &&
      cmSystemTools::GetEnv(forceRspFile) == 0) {
#ifdef _WIN32
    commandLineLengthLimit = 8000 - linkRuleLength;
#elif defined(__linux) || defined(__APPLE__)
    // for instance ARG_MAX is 2096152 on Ubuntu or 262144 on Mac
    commandLineLengthLimit = ((int)sysconf(_SC_ARG_MAX))-linkRuleLength-1000;
#else
    (void)linkRuleLength;
    commandLineLengthLimit = -1;
#endif
  }

  //Get the global generator as we are going to be call WriteBuild numerous
  //times in the following section
  cmGlobalNinjaGenerator* globalGenerator = this->GetGlobalGenerator();


  const std::string rspfile = std::string
                              (cmake::GetCMakeFilesDirectoryPostSlash()) +
                              this->GetTarget()->GetName() + ".rsp";

  // Write the build statement for this target.
  globalGenerator->WriteBuild(this->GetBuildFileStream(),
                              comment.str(),
                              this->LanguageLinkerRule(),
                              outputs,
                              explicitDeps,
                              implicitDeps,
                              emptyDeps,
                              vars,
                              rspfile,
                              commandLineLengthLimit);

  if (targetOutput != targetOutputReal) {
    if (targetType == cmTarget::EXECUTABLE) {
      globalGenerator->WriteBuild(this->GetBuildFileStream(),
                                  "Create executable symlink " + targetOutput,
                                  "CMAKE_SYMLINK_EXECUTABLE",
                                  cmNinjaDeps(1, targetOutput),
                                  cmNinjaDeps(1, targetOutputReal),
                                  emptyDeps,
                                  emptyDeps,
                                  symlinkVars);
    } else {
      cmNinjaDeps symlinks;
      const std::string soName = this->GetTargetFilePath(this->TargetNameSO);
      // If one link has to be created.
      if (targetOutputReal == soName || targetOutput == soName) {
        symlinkVars["SONAME"] = soName;
      } else {
        symlinkVars["SONAME"] = "";
        symlinks.push_back(soName);
      }
      symlinks.push_back(targetOutput);
      globalGenerator->WriteBuild(this->GetBuildFileStream(),
                                  "Create library symlink " + targetOutput,
                                     "CMAKE_SYMLINK_LIBRARY",
                                  symlinks,
                                  cmNinjaDeps(1, targetOutputReal),
                                  emptyDeps,
                                  emptyDeps,
                                  symlinkVars);
    }
  }

  if (!this->TargetNameImport.empty()) {
    // Since using multiple outputs would mess up the $out variable, use an
    // alias for the import library.
    globalGenerator->WritePhonyBuild(this->GetBuildFileStream(),
                                     "Alias for import library.",
                                     cmNinjaDeps(1, targetOutputImplib),
                                     cmNinjaDeps(1, targetOutputReal));
  }

  // Add aliases for the file name and the target name.
  globalGenerator->AddTargetAlias(this->TargetNameOut,
                                             this->GetTarget());
  globalGenerator->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}

//----------------------------------------------------------------------------
void cmNinjaNormalTargetGenerator::WriteObjectLibStatement()
{
  // Write a phony output that depends on all object files.
  cmNinjaDeps outputs;
  this->GetLocalGenerator()->AppendTargetOutputs(this->GetTarget(), outputs);
  cmNinjaDeps depends = this->GetObjects();
  this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                              "Object library "
                                                + this->GetTargetName(),
                                              outputs,
                                              depends);

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}
