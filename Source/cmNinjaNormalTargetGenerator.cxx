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

#include "cmAlgorithms.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"
#include "cmSourceFile.h"

#include <algorithm>
#include <assert.h>
#include <limits>

#ifndef _WIN32
#include <unistd.h>
#endif

cmNinjaNormalTargetGenerator::cmNinjaNormalTargetGenerator(
  cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target)
  , TargetNameOut()
  , TargetNameSO()
  , TargetNameReal()
  , TargetNameImport()
  , TargetNamePDB()
  , TargetLinkLanguage("")
{
  this->TargetLinkLanguage = target->GetLinkerLanguage(this->GetConfigName());
  if (target->GetType() == cmState::EXECUTABLE)
    this->GetGeneratorTarget()->GetExecutableNames(
      this->TargetNameOut, this->TargetNameReal, this->TargetNameImport,
      this->TargetNamePDB, GetLocalGenerator()->GetConfigName());
  else
    this->GetGeneratorTarget()->GetLibraryNames(
      this->TargetNameOut, this->TargetNameSO, this->TargetNameReal,
      this->TargetNameImport, this->TargetNamePDB,
      GetLocalGenerator()->GetConfigName());

  if (target->GetType() != cmState::OBJECT_LIBRARY) {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    EnsureDirectoryExists(target->GetDirectory(this->GetConfigName()));
  }

  this->OSXBundleGenerator =
    new cmOSXBundleGenerator(target, this->GetConfigName());
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator()
{
  delete this->OSXBundleGenerator;
}

void cmNinjaNormalTargetGenerator::Generate()
{
  if (this->TargetLinkLanguage.empty()) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: ",
                         this->GetGeneratorTarget()->GetName().c_str());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules();

  // Write the build statements
  this->WriteObjectBuildStatements();

  if (this->GetGeneratorTarget()->GetType() == cmState::OBJECT_LIBRARY) {
    this->WriteObjectLibStatement();
  } else {
    this->WriteLinkStatement();
  }
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
  std::vector<cmSourceFile*> sourceFiles;
  this->GetGeneratorTarget()->GetSourceFiles(
    sourceFiles, this->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE"));
  for (std::vector<cmSourceFile*>::const_iterator i = sourceFiles.begin();
       i != sourceFiles.end(); ++i) {
    const std::string& lang = (*i)->GetLanguage();
    if (!lang.empty()) {
      languages.insert(lang);
    }
  }
  for (std::set<std::string>::const_iterator l = languages.begin();
       l != languages.end(); ++l) {
    this->WriteLanguageRules(*l);
  }
}

const char* cmNinjaNormalTargetGenerator::GetVisibleTypeName() const
{
  switch (this->GetGeneratorTarget()->GetType()) {
    case cmState::STATIC_LIBRARY:
      return "static library";
    case cmState::SHARED_LIBRARY:
      return "shared library";
    case cmState::MODULE_LIBRARY:
      if (this->GetGeneratorTarget()->IsCFBundleOnApple())
        return "CFBundle shared module";
      else
        return "shared module";
    case cmState::EXECUTABLE:
      return "executable";
    default:
      return 0;
  }
}

std::string cmNinjaNormalTargetGenerator::LanguageLinkerRule() const
{
  return this->TargetLinkLanguage + "_" +
    cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType()) +
    "_LINKER__" + cmGlobalNinjaGenerator::EncodeRuleName(
                    this->GetGeneratorTarget()->GetName());
}

void cmNinjaNormalTargetGenerator::WriteLinkRule(bool useResponseFile)
{
  cmState::TargetType targetType = this->GetGeneratorTarget()->GetType();
  std::string ruleName = this->LanguageLinkerRule();

  // Select whether to use a response file for objects.
  std::string rspfile;
  std::string rspcontent;

  if (!this->GetGlobalGenerator()->HasRule(ruleName)) {
    cmLocalGenerator::RuleVariables vars;
    vars.RuleLauncher = "RULE_LAUNCH_LINK";
    vars.CMTarget = this->GetGeneratorTarget();
    vars.Language = this->TargetLinkLanguage.c_str();

    std::string responseFlag;
    if (!useResponseFile) {
      vars.Objects = "$in";
      vars.LinkLibraries = "$LINK_PATH $LINK_LIBRARIES";
    } else {
      std::string cmakeVarLang = "CMAKE_";
      cmakeVarLang += this->TargetLinkLanguage;

      // build response file name
      std::string cmakeLinkVar = cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";
      const char* flag = GetMakefile()->GetDefinition(cmakeLinkVar);
      if (flag) {
        responseFlag = flag;
      } else {
        responseFlag = "@";
      }
      rspfile = "$RSP_FILE";
      responseFlag += rspfile;

      // build response file content
      if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
        rspcontent = "$in";
      } else {
        rspcontent = "$in_newline";
      }
      rspcontent += " $LINK_PATH $LINK_LIBRARIES";
      vars.Objects = responseFlag.c_str();
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
    if (targetType != cmState::EXECUTABLE) {
      langFlags += "$LANGUAGE_COMPILE_FLAGS $ARCH_FLAGS";
      vars.LanguageCompileFlags = langFlags.c_str();
    }

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd();
    for (std::vector<std::string>::iterator i = linkCmds.begin();
         i != linkCmds.end(); ++i) {
      this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);
    }
    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.push_back("$POST_BUILD");
    std::string linkCmd =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule with response file if needed.
    std::ostringstream comment;
    comment << "Rule for linking " << this->TargetLinkLanguage << " "
            << this->GetVisibleTypeName() << ".";
    std::ostringstream description;
    description << "Linking " << this->TargetLinkLanguage << " "
                << this->GetVisibleTypeName() << " $TARGET_FILE";
    this->GetGlobalGenerator()->AddRule(ruleName, linkCmd, description.str(),
                                        comment.str(),
                                        /*depfile*/ "",
                                        /*deptype*/ "", rspfile, rspcontent,
                                        /*restat*/ "$RESTAT",
                                        /*generator*/ false);
  }

  if (this->TargetNameOut != this->TargetNameReal &&
      !this->GetGeneratorTarget()->IsFrameworkOnApple()) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
    if (targetType == cmState::EXECUTABLE)
      this->GetGlobalGenerator()->AddRule(
        "CMAKE_SYMLINK_EXECUTABLE",
        cmakeCommand + " -E cmake_symlink_executable"
                       " $in $out && $POST_BUILD",
        "Creating executable symlink $out", "Rule for creating "
                                            "executable symlink.",
        /*depfile*/ "",
        /*deptype*/ "",
        /*rspfile*/ "",
        /*rspcontent*/ "",
        /*restat*/ "",
        /*generator*/ false);
    else
      this->GetGlobalGenerator()->AddRule(
        "CMAKE_SYMLINK_LIBRARY",
        cmakeCommand + " -E cmake_symlink_library"
                       " $in $SONAME $out && $POST_BUILD",
        "Creating library symlink $out", "Rule for creating "
                                         "library symlink.",
        /*depfile*/ "",
        /*deptype*/ "",
        /*rspfile*/ "",
        /*rspcontent*/ "",
        /*restat*/ "",
        /*generator*/ false);
  }
}

std::vector<std::string> cmNinjaNormalTargetGenerator::ComputeLinkCmd()
{
  std::vector<std::string> linkCmds;
  cmMakefile* mf = this->GetMakefile();
  {
    std::string linkCmdVar = this->GetGeneratorTarget()->GetCreateRuleVariable(
      this->TargetLinkLanguage, this->GetConfigName());
    const char* linkCmd = mf->GetDefinition(linkCmdVar);
    if (linkCmd) {
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      if( this->GetGeneratorTarget()->GetProperty("LINK_WHAT_YOU_USE")) {
        std::string cmakeCommand =
          this->GetLocalGenerator()->ConvertToOutputFormat(
            cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
        cmakeCommand += " -E __run_iwyu --lwyu=";
        cmGeneratorTarget& gt = *this->GetGeneratorTarget();
        const std::string cfgName = this->GetConfigName();
        std::string targetOutput = ConvertToNinjaPath(gt.GetFullPath(cfgName));
        std::string targetOutputReal =
          this->ConvertToNinjaPath(gt.GetFullPath(cfgName,
                                            /*implib=*/false,
                                            /*realpath=*/true));
        cmakeCommand += targetOutputReal;
        cmakeCommand += " || true";
        linkCmds.push_back(cmakeCommand);
      }
      return linkCmds;
    }
  }
  switch (this->GetGeneratorTarget()->GetType()) {
    case cmState::STATIC_LIBRARY: {
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
        const char* linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      {
        std::string linkCmdVar = "CMAKE_";
        linkCmdVar += this->TargetLinkLanguage;
        linkCmdVar += "_ARCHIVE_FINISH";
        const char* linkCmd = mf->GetRequiredDefinition(linkCmdVar);
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      return linkCmds;
    }
    case cmState::SHARED_LIBRARY:
    case cmState::MODULE_LIBRARY:
    case cmState::EXECUTABLE:
      break;
    default:
      assert(0 && "Unexpected target type");
  }
  return std::vector<std::string>();
}

static int calculateCommandLineLengthLimit(int linkRuleLength)
{
  static int const limits[] = {
#ifdef _WIN32
    8000,
#endif
#if defined(__APPLE__) || defined(__HAIKU__) || defined(__linux)
    // for instance ARG_MAX is 2096152 on Ubuntu or 262144 on Mac
    ((int)sysconf(_SC_ARG_MAX)) - 1000,
#endif
#if defined(__linux)
    // #define MAX_ARG_STRLEN (PAGE_SIZE * 32) in Linux's binfmts.h
    ((int)sysconf(_SC_PAGESIZE) * 32) - 1000,
#endif
    std::numeric_limits<int>::max()
  };

  size_t const arrSz = cmArraySize(limits);
  int const sz = *std::min_element(limits, limits + arrSz);
  if (sz == std::numeric_limits<int>::max()) {
    return -1;
  }

  return sz - linkRuleLength;
}

void cmNinjaNormalTargetGenerator::WriteLinkStatement()
{
  cmGeneratorTarget& gt = *this->GetGeneratorTarget();
  const std::string cfgName = this->GetConfigName();
  std::string targetOutput = ConvertToNinjaPath(gt.GetFullPath(cfgName));
  std::string targetOutputReal =
    ConvertToNinjaPath(gt.GetFullPath(cfgName,
                                      /*implib=*/false,
                                      /*realpath=*/true));
  std::string targetOutputImplib =
    ConvertToNinjaPath(gt.GetFullPath(cfgName,
                                      /*implib=*/true));

  if (gt.IsAppBundleOnApple()) {
    // Create the app bundle
    std::string outpath = gt.GetDirectory(cfgName);
    this->OSXBundleGenerator->CreateAppBundle(this->TargetNameOut, outpath);

    // Calculate the output path
    targetOutput = outpath;
    targetOutput += "/";
    targetOutput += this->TargetNameOut;
    targetOutput = this->ConvertToNinjaPath(targetOutput);
    targetOutputReal = outpath;
    targetOutputReal += "/";
    targetOutputReal += this->TargetNameReal;
    targetOutputReal = this->ConvertToNinjaPath(targetOutputReal);
  } else if (gt.IsFrameworkOnApple()) {
    // Create the library framework.
    this->OSXBundleGenerator->CreateFramework(this->TargetNameOut,
                                              gt.GetDirectory(cfgName));
  } else if (gt.IsCFBundleOnApple()) {
    // Create the core foundation bundle.
    this->OSXBundleGenerator->CreateCFBundle(this->TargetNameOut,
                                             gt.GetDirectory(cfgName));
  }

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  const cmState::TargetType targetType = gt.GetType();
  this->GetBuildFileStream() << "# Link build statements for "
                             << cmState::GetTargetTypeName(targetType)
                             << " target " << this->GetTargetName() << "\n\n";

  cmNinjaDeps emptyDeps;
  cmNinjaVars vars;

  // Compute the comment.
  std::ostringstream comment;
  comment << "Link the " << this->GetVisibleTypeName() << " "
          << targetOutputReal;

  // Compute outputs.
  cmNinjaDeps outputs;
  outputs.push_back(targetOutputReal);

  // Compute specific libraries to link with.
  cmNinjaDeps explicitDeps = this->GetObjects();
  cmNinjaDeps implicitDeps = this->ComputeLinkDeps();

  cmMakefile* mf = this->GetMakefile();

  std::string frameworkPath;
  std::string linkPath;
  cmGeneratorTarget& genTarget = *this->GetGeneratorTarget();

  std::string createRule = genTarget.GetCreateRuleVariable(
    this->TargetLinkLanguage, this->GetConfigName());
  bool useWatcomQuote = mf->IsOn(createRule + "_USE_WATCOM_QUOTE");
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(targetOutputReal, cmOutputConverter::SHELL);

  localGen.GetTargetFlags(vars["LINK_LIBRARIES"], vars["FLAGS"],
                          vars["LINK_FLAGS"], frameworkPath, linkPath,
                          &genTarget, useWatcomQuote);
  if (this->GetMakefile()->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS") &&
      gt.GetType() == cmState::SHARED_LIBRARY) {
    if (gt.GetPropertyAsBool("WINDOWS_EXPORT_ALL_SYMBOLS")) {
      std::string name_of_def_file = gt.GetSupportDirectory();
      name_of_def_file += "/" + gt.GetName();
      name_of_def_file += ".def ";
      vars["LINK_FLAGS"] += " /DEF:";
      vars["LINK_FLAGS"] += this->GetLocalGenerator()->ConvertToOutputFormat(
        name_of_def_file, cmOutputConverter::SHELL);
    }
  }

  // Add OS X version flags, if any.
  if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(vars["LINK_FLAGS"], this->TargetLinkLanguage,
                           "COMPATIBILITY", true);
    this->AppendOSXVerFlag(vars["LINK_FLAGS"], this->TargetLinkLanguage,
                           "CURRENT", false);
  }

  this->addPoolNinjaVariable("JOB_POOL_LINK", &gt, vars);

  this->AddModuleDefinitionFlag(vars["LINK_FLAGS"]);
  vars["LINK_FLAGS"] =
    cmGlobalNinjaGenerator::EncodeLiteral(vars["LINK_FLAGS"]);

  vars["MANIFESTS"] = this->GetManifests();

  vars["LINK_PATH"] = frameworkPath + linkPath;
  std::string lwyuFlags;
  if(genTarget.GetProperty("LINK_WHAT_YOU_USE")) {
    lwyuFlags = " -Wl,--no-as-needed";
  }

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  if (targetType == cmState::EXECUTABLE) {
    std::string t = vars["FLAGS"];
    localGen.AddArchitectureFlags(t, &genTarget, TargetLinkLanguage, cfgName);
    t += lwyuFlags;
    vars["FLAGS"] = t;
  } else {
    std::string t = vars["ARCH_FLAGS"];
    localGen.AddArchitectureFlags(t, &genTarget, TargetLinkLanguage, cfgName);
    vars["ARCH_FLAGS"] = t;
    t = "";
    t += lwyuFlags;
    localGen.AddLanguageFlags(t, TargetLinkLanguage, cfgName);
    vars["LANGUAGE_COMPILE_FLAGS"] = t;
  }
  if (this->GetGeneratorTarget()->HasSOName(cfgName)) {
    vars["SONAME_FLAG"] = mf->GetSONameFlag(this->TargetLinkLanguage);
    vars["SONAME"] = this->TargetNameSO;
    if (targetType == cmState::SHARED_LIBRARY) {
      std::string install_dir =
        this->GetGeneratorTarget()->GetInstallNameDirForBuildTree(cfgName);
      if (!install_dir.empty()) {
        vars["INSTALLNAME_DIR"] = localGen.Convert(
          install_dir, cmOutputConverter::NONE, cmOutputConverter::SHELL);
      }
    }
  }

  cmNinjaDeps byproducts;

  if (!this->TargetNameImport.empty()) {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
      targetOutputImplib, cmOutputConverter::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    EnsureParentDirectoryExists(impLibPath);
    if (genTarget.HasImportLibrary()) {
      byproducts.push_back(targetOutputImplib);
    }
  }

  if (!this->SetMsvcTargetPdbVariable(vars)) {
    // It is common to place debug symbols at a specific place,
    // so we need a plain target name in the rule available.
    std::string prefix;
    std::string base;
    std::string suffix;
    this->GetGeneratorTarget()->GetFullNameComponents(prefix, base, suffix);
    std::string dbg_suffix = ".dbg";
    // TODO: Where to document?
    if (mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX")) {
      dbg_suffix = mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX");
    }
    vars["TARGET_PDB"] = base + suffix + dbg_suffix;
  }

  const std::string objPath = GetGeneratorTarget()->GetSupportDirectory();
  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->ConvertToNinjaPath(objPath), cmOutputConverter::SHELL);
  EnsureDirectoryExists(objPath);

  if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::string& linkLibraries = vars["LINK_LIBRARIES"];
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::string& link_path = vars["LINK_PATH"];
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
  }

  const std::vector<cmCustomCommand>* cmdLists[3] = {
    &gt.GetPreBuildCommands(), &gt.GetPreLinkCommands(),
    &gt.GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines, postBuildCmdLines;
  std::vector<std::string>* cmdLineLists[3] = { &preLinkCmdLines,
                                                &preLinkCmdLines,
                                                &postBuildCmdLines };

  for (unsigned i = 0; i != 3; ++i) {
    for (std::vector<cmCustomCommand>::const_iterator ci =
           cmdLists[i]->begin();
         ci != cmdLists[i]->end(); ++ci) {
      cmCustomCommandGenerator ccg(*ci, cfgName, this->GetLocalGenerator());
      localGen.AppendCustomCommandLines(ccg, *cmdLineLists[i]);
      std::vector<std::string> const& ccByproducts = ccg.GetByproducts();
      std::transform(ccByproducts.begin(), ccByproducts.end(),
                     std::back_inserter(byproducts), MapToNinjaPath());
    }
  }

  // maybe create .def file from list of objects
  if (gt.GetType() == cmState::SHARED_LIBRARY &&
      this->GetMakefile()->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS")) {
    if (gt.GetPropertyAsBool("WINDOWS_EXPORT_ALL_SYMBOLS")) {
      std::string cmakeCommand =
        this->GetLocalGenerator()->ConvertToOutputFormat(
          cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
      std::string name_of_def_file = gt.GetSupportDirectory();
      name_of_def_file += "/" + gt.GetName();
      name_of_def_file += ".def";
      std::string cmd = cmakeCommand;
      cmd += " -E __create_def ";
      cmd += this->GetLocalGenerator()->ConvertToOutputFormat(
        name_of_def_file, cmOutputConverter::SHELL);
      cmd += " ";
      cmNinjaDeps objs = this->GetObjects();
      std::string obj_list_file = name_of_def_file;
      obj_list_file += ".objs";
      cmd += this->GetLocalGenerator()->ConvertToOutputFormat(
        obj_list_file, cmOutputConverter::SHELL);
      preLinkCmdLines.push_back(cmd);
      // create a list of obj files for the -E __create_def to read
      cmGeneratedFileStream fout(obj_list_file.c_str());
      for (cmNinjaDeps::iterator i = objs.begin(); i != objs.end(); ++i) {
        if (cmHasLiteralSuffix(*i, ".obj")) {
          fout << *i << "\n";
        }
      }
    }
  }
  // If we have any PRE_LINK commands, we need to go back to HOME_OUTPUT for
  // the link commands.
  if (!preLinkCmdLines.empty()) {
    const std::string homeOutDir = localGen.ConvertToOutputFormat(
      localGen.GetBinaryDirectory(), cmOutputConverter::SHELL);
    preLinkCmdLines.push_back("cd " + homeOutDir);
  }

  vars["PRE_LINK"] = localGen.BuildCommandLine(preLinkCmdLines);
  std::string postBuildCmdLine = localGen.BuildCommandLine(postBuildCmdLines);

  cmNinjaVars symlinkVars;
  if (targetOutput == targetOutputReal) {
    vars["POST_BUILD"] = postBuildCmdLine;
  } else {
    vars["POST_BUILD"] = ":";
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
  }
  cmGlobalNinjaGenerator& globalGen = *this->GetGlobalGenerator();

  int commandLineLengthLimit = -1;
  if (!this->ForceResponseFile()) {
    commandLineLengthLimit = calculateCommandLineLengthLimit(
      globalGen.GetRuleCmdLength(this->LanguageLinkerRule()));
  }

  const std::string rspfile =
    std::string(cmake::GetCMakeFilesDirectoryPostSlash()) + gt.GetName() +
    ".rsp";

  // Gather order-only dependencies.
  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->GetGeneratorTarget(),
                                                 orderOnlyDeps);

  // Ninja should restat after linking if and only if there are byproducts.
  vars["RESTAT"] = byproducts.empty() ? "" : "1";

  for (cmNinjaDeps::const_iterator oi = byproducts.begin(),
                                   oe = byproducts.end();
       oi != oe; ++oi) {
    this->GetGlobalGenerator()->SeenCustomCommandOutput(*oi);
    outputs.push_back(*oi);
  }

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen.WriteBuild(this->GetBuildFileStream(), comment.str(),
                       this->LanguageLinkerRule(), outputs, explicitDeps,
                       implicitDeps, orderOnlyDeps, vars, rspfile,
                       commandLineLengthLimit, &usedResponseFile);
  this->WriteLinkRule(usedResponseFile);

  if (targetOutput != targetOutputReal && !gt.IsFrameworkOnApple()) {
    if (targetType == cmState::EXECUTABLE) {
      globalGen.WriteBuild(
        this->GetBuildFileStream(),
        "Create executable symlink " + targetOutput,
        "CMAKE_SYMLINK_EXECUTABLE", cmNinjaDeps(1, targetOutput),
        cmNinjaDeps(1, targetOutputReal), emptyDeps, emptyDeps, symlinkVars);
    } else {
      cmNinjaDeps symlinks;
      std::string const soName =
        this->ConvertToNinjaPath(this->GetTargetFilePath(this->TargetNameSO));
      // If one link has to be created.
      if (targetOutputReal == soName || targetOutput == soName) {
        symlinkVars["SONAME"] = soName;
      } else {
        symlinkVars["SONAME"] = "";
        symlinks.push_back(soName);
      }
      symlinks.push_back(targetOutput);
      globalGen.WriteBuild(
        this->GetBuildFileStream(), "Create library symlink " + targetOutput,
        "CMAKE_SYMLINK_LIBRARY", symlinks, cmNinjaDeps(1, targetOutputReal),
        emptyDeps, emptyDeps, symlinkVars);
    }
  }

  // Add aliases for the file name and the target name.
  globalGen.AddTargetAlias(this->TargetNameOut, &gt);
  globalGen.AddTargetAlias(this->GetTargetName(), &gt);
}

void cmNinjaNormalTargetGenerator::WriteObjectLibStatement()
{
  // Write a phony output that depends on all object files.
  cmNinjaDeps outputs;
  this->GetLocalGenerator()->AppendTargetOutputs(this->GetGeneratorTarget(),
                                                 outputs);
  cmNinjaDeps depends = this->GetObjects();
  this->GetGlobalGenerator()->WritePhonyBuild(
    this->GetBuildFileStream(), "Object library " + this->GetTargetName(),
    outputs, depends);

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetGeneratorTarget());
}
