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
#include "cmNinjaTargetGenerator.h"

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmNinjaNormalTargetGenerator.h"
#include "cmNinjaUtilityTargetGenerator.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"

#include <algorithm>

cmNinjaTargetGenerator* cmNinjaTargetGenerator::New(cmGeneratorTarget* target)
{
  switch (target->GetType()) {
    case cmState::EXECUTABLE:
    case cmState::SHARED_LIBRARY:
    case cmState::STATIC_LIBRARY:
    case cmState::MODULE_LIBRARY:
    case cmState::OBJECT_LIBRARY:
      return new cmNinjaNormalTargetGenerator(target);

    case cmState::UTILITY:
      return new cmNinjaUtilityTargetGenerator(target);
      ;

    case cmState::GLOBAL_TARGET: {
      // We only want to process global targets that live in the home
      // (i.e. top-level) directory.  CMake creates copies of these targets
      // in every directory, which we don't need.
      if (strcmp(target->GetLocalGenerator()->GetCurrentSourceDirectory(),
                 target->GetLocalGenerator()->GetSourceDirectory()) == 0)
        return new cmNinjaUtilityTargetGenerator(target);
      // else fallthrough
    }

    default:
      return 0;
  }
}

cmNinjaTargetGenerator::cmNinjaTargetGenerator(cmGeneratorTarget* target)
  : cmCommonTargetGenerator(cmOutputConverter::HOME_OUTPUT, target)
  , MacOSXContentGenerator(0)
  , OSXBundleGenerator(0)
  , MacContentFolders()
  , LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(target->GetLocalGenerator()))
  , Objects()
{
  MacOSXContentGenerator = new MacOSXContentGeneratorType(this);
}

cmNinjaTargetGenerator::~cmNinjaTargetGenerator()
{
  delete this->MacOSXContentGenerator;
}

cmGeneratedFileStream& cmNinjaTargetGenerator::GetBuildFileStream() const
{
  return *this->GetGlobalGenerator()->GetBuildFileStream();
}

cmGeneratedFileStream& cmNinjaTargetGenerator::GetRulesFileStream() const
{
  return *this->GetGlobalGenerator()->GetRulesFileStream();
}

cmGlobalNinjaGenerator* cmNinjaTargetGenerator::GetGlobalGenerator() const
{
  return this->LocalGenerator->GetGlobalNinjaGenerator();
}

std::string cmNinjaTargetGenerator::LanguageCompilerRule(
  const std::string& lang) const
{
  return lang + "_COMPILER__" +
    cmGlobalNinjaGenerator::EncodeRuleName(this->GeneratorTarget->GetName());
}

std::string cmNinjaTargetGenerator::OrderDependsTargetForTarget()
{
  return "cmake_order_depends_target_" + this->GetTargetName();
}

// TODO: Most of the code is picked up from
// void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink),
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
// Refactor it.
std::string cmNinjaTargetGenerator::ComputeFlagsForObject(
  cmSourceFile const* source, const std::string& language)
{
  std::string flags = this->GetFlags(language);

  // Add Fortran format flags.
  if (language == "Fortran") {
    this->AppendFortranFormatFlags(flags, *source);
  }

  // Add source file specific flags.
  this->LocalGenerator->AppendFlags(flags,
                                    source->GetProperty("COMPILE_FLAGS"));

  return flags;
}

void cmNinjaTargetGenerator::AddIncludeFlags(std::string& languageFlags,
                                             std::string const& language)
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              language, this->GetConfigName());
  // Add include directory flags.
  std::string includeFlags = this->LocalGenerator->GetIncludeFlags(
    includes, this->GeneratorTarget, language,
    language == "RC" ? true : false, // full include paths for RC
    // needed by cmcldeps
    false, this->GetConfigName());
  if (this->GetGlobalGenerator()->IsGCCOnWindows())
    std::replace(includeFlags.begin(), includeFlags.end(), '\\', '/');

  this->LocalGenerator->AppendFlags(languageFlags, includeFlags);
}

bool cmNinjaTargetGenerator::NeedDepTypeMSVC(const std::string& lang) const
{
  return strcmp(this->GetMakefile()->GetSafeDefinition("CMAKE_NINJA_DEPTYPE_" +
                                                       lang),
                "msvc") == 0;
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string cmNinjaTargetGenerator::ComputeDefines(cmSourceFile const* source,
                                                   const std::string& language)
{
  std::set<std::string> defines;
  this->LocalGenerator->AppendDefines(
    defines, source->GetProperty("COMPILE_DEFINITIONS"));
  {
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += cmSystemTools::UpperCase(this->GetConfigName());
    this->LocalGenerator->AppendDefines(defines,
                                        source->GetProperty(defPropName));
  }

  std::string definesString = this->GetDefines(language);
  this->LocalGenerator->JoinDefines(defines, definesString, language);

  return definesString;
}

cmNinjaDeps cmNinjaTargetGenerator::ComputeLinkDeps() const
{
  // Static libraries never depend on other targets for linking.
  if (this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() == cmState::OBJECT_LIBRARY)
    return cmNinjaDeps();

  cmComputeLinkInformation* cli =
    this->GeneratorTarget->GetLinkInformation(this->GetConfigName());
  if (!cli)
    return cmNinjaDeps();

  const std::vector<std::string>& deps = cli->GetDepends();
  cmNinjaDeps result(deps.size());
  std::transform(deps.begin(), deps.end(), result.begin(), MapToNinjaPath());

  // Add a dependency on the link definitions file, if any.
  if (this->ModuleDefinitionFile) {
    result.push_back(
      this->ConvertToNinjaPath(this->ModuleDefinitionFile->GetFullPath()));
  }

  // Add a dependency on user-specified manifest files, if any.
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, this->ConfigName);
  for (std::vector<cmSourceFile const*>::iterator mi = manifest_srcs.begin();
       mi != manifest_srcs.end(); ++mi) {
    result.push_back(this->ConvertToNinjaPath((*mi)->GetFullPath()));
  }

  // Add user-specified dependencies.
  if (const char* linkDepends =
        this->GeneratorTarget->GetProperty("LINK_DEPENDS")) {
    std::vector<std::string> linkDeps;
    cmSystemTools::ExpandListArgument(linkDepends, linkDeps);
    std::transform(linkDeps.begin(), linkDeps.end(),
                   std::back_inserter(result), MapToNinjaPath());
  }

  return result;
}

std::string cmNinjaTargetGenerator::GetSourceFilePath(
  cmSourceFile const* source) const
{
  return ConvertToNinjaPath(source->GetFullPath());
}

std::string cmNinjaTargetGenerator::GetObjectFilePath(
  cmSourceFile const* source) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if (!path.empty())
    path += "/";
  std::string const& objectName = this->GeneratorTarget->GetObjectName(source);
  path += this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  path += "/";
  path += objectName;
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetOutputDir() const
{
  std::string dir = this->GeneratorTarget->GetDirectory(this->GetConfigName());
  return ConvertToNinjaPath(dir);
}

std::string cmNinjaTargetGenerator::GetTargetFilePath(
  const std::string& name) const
{
  std::string path = this->GetTargetOutputDir();
  if (path.empty() || path == ".")
    return name;
  path += "/";
  path += name;
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetName() const
{
  return this->GeneratorTarget->GetName();
}

bool cmNinjaTargetGenerator::SetMsvcTargetPdbVariable(cmNinjaVars& vars) const
{
  cmMakefile* mf = this->GetMakefile();
  if (mf->GetDefinition("MSVC_C_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CXX_ARCHITECTURE_ID")) {
    std::string pdbPath;
    std::string compilePdbPath;
    if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE ||
        this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY ||
        this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY ||
        this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
      pdbPath = this->GeneratorTarget->GetPDBDirectory(this->GetConfigName());
      pdbPath += "/";
      pdbPath += this->GeneratorTarget->GetPDBName(this->GetConfigName());
    }
    if (this->GeneratorTarget->GetType() <= cmState::OBJECT_LIBRARY) {
      compilePdbPath =
        this->GeneratorTarget->GetCompilePDBPath(this->GetConfigName());
      if (compilePdbPath.empty()) {
        compilePdbPath = this->GeneratorTarget->GetSupportDirectory() + "/";
      }
    }

    vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
      ConvertToNinjaPath(pdbPath), cmOutputConverter::SHELL);
    vars["TARGET_COMPILE_PDB"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        ConvertToNinjaPath(compilePdbPath), cmOutputConverter::SHELL);

    EnsureParentDirectoryExists(pdbPath);
    EnsureParentDirectoryExists(compilePdbPath);
    return true;
  }
  return false;
}

void cmNinjaTargetGenerator::WriteLanguageRules(const std::string& language)
{
#ifdef NINJA_GEN_VERBOSE_FILES
  this->GetRulesFileStream() << "# Rules for language " << language << "\n\n";
#endif
  this->WriteCompileRule(language);
}

void cmNinjaTargetGenerator::WriteCompileRule(const std::string& lang)
{
  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_COMPILE";
  vars.CMTarget = this->GetGeneratorTarget();
  vars.Language = lang.c_str();
  vars.Source = "$in";
  vars.Object = "$out";
  vars.Defines = "$DEFINES";
  vars.Includes = "$INCLUDES";
  vars.TargetPDB = "$TARGET_PDB";
  vars.TargetCompilePDB = "$TARGET_COMPILE_PDB";
  vars.ObjectDir = "$OBJECT_DIR";
  vars.ObjectFileDir = "$OBJECT_FILE_DIR";

  cmMakefile* mf = this->GetMakefile();

  std::string flags = "$FLAGS";
  std::string rspfile;
  std::string rspcontent;
  std::string responseFlag;

  if (this->ForceResponseFile()) {
    rspfile = "$RSP_FILE";
    responseFlag = "@" + rspfile;
    rspcontent = " $DEFINES $INCLUDES $FLAGS";
    flags = responseFlag;
    vars.Defines = "";
    vars.Includes = "";
  }

  // Tell ninja dependency format so all deps can be loaded into a database
  std::string deptype;
  std::string depfile;
  std::string cldeps;
  if (this->NeedDepTypeMSVC(lang)) {
    deptype = "msvc";
    depfile = "";
    flags += " /showIncludes";
  } else if (mf->IsOn("CMAKE_NINJA_CMCLDEPS_" + lang)) {
    // For the MS resource compiler we need cmcldeps, but skip dependencies
    // for source-file try_compile cases because they are always fresh.
    if (!mf->GetIsSourceFileTryCompile()) {
      deptype = "gcc";
      depfile = "$DEP_FILE";
      const std::string cl = mf->GetDefinition("CMAKE_C_COMPILER")
        ? mf->GetSafeDefinition("CMAKE_C_COMPILER")
        : mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
      cldeps = "\"";
      cldeps += cmSystemTools::GetCMClDepsCommand();
      cldeps += "\" " + lang + " $in \"$DEP_FILE\" $out \"";
      cldeps += mf->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
      cldeps += "\" \"" + cl + "\" ";
    }
  } else {
    deptype = "gcc";
    const char* langdeptype = mf->GetDefinition("CMAKE_NINJA_DEPTYPE_" + lang);
    if (langdeptype) {
      deptype = langdeptype;
    }
    depfile = "$DEP_FILE";
    const std::string flagsName = "CMAKE_DEPFILE_FLAGS_" + lang;
    std::string depfileFlags = mf->GetSafeDefinition(flagsName);
    if (!depfileFlags.empty()) {
      cmSystemTools::ReplaceString(depfileFlags, "<DEPFILE>", "$DEP_FILE");
      cmSystemTools::ReplaceString(depfileFlags, "<OBJECT>", "$out");
      cmSystemTools::ReplaceString(depfileFlags, "<CMAKE_C_COMPILER>",
                                   mf->GetDefinition("CMAKE_C_COMPILER"));
      flags += " " + depfileFlags;
    }
  }

  vars.Flags = flags.c_str();
  vars.DependencyFile = depfile.c_str();

  // Rule for compiling object file.
  const std::string cmdVar = std::string("CMAKE_") + lang + "_COMPILE_OBJECT";
  std::string compileCmd = mf->GetRequiredDefinition(cmdVar);
  std::vector<std::string> compileCmds;
  cmSystemTools::ExpandListArgument(compileCmd, compileCmds);

  // Maybe insert an include-what-you-use runner.
  if (!compileCmds.empty() && (lang == "C" || lang == "CXX")) {
    std::string const iwyu_prop = lang + "_INCLUDE_WHAT_YOU_USE";
    const char* iwyu = this->GeneratorTarget->GetProperty(iwyu_prop);
    std::string const tidy_prop = lang + "_CLANG_TIDY";
    const char* tidy = this->GeneratorTarget->GetProperty(tidy_prop);
    if ((iwyu && *iwyu) || (tidy && *tidy)) {
      std::string run_iwyu = this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
      run_iwyu += " -E __run_iwyu";
      if (iwyu && *iwyu) {
        run_iwyu += " --iwyu=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(iwyu);
      }
      if (tidy && *tidy) {
        run_iwyu += " --tidy=";
        run_iwyu += this->GetLocalGenerator()->EscapeForShell(tidy);
        run_iwyu += " --source=$in";
      }
      run_iwyu += " -- ";
      compileCmds.front().insert(0, run_iwyu);
    }
  }

  // Maybe insert a compiler launcher like ccache or distcc
  if (!compileCmds.empty() && (lang == "C" || lang == "CXX")) {
    std::string const clauncher_prop = lang + "_COMPILER_LAUNCHER";
    const char* clauncher = this->GeneratorTarget->GetProperty(clauncher_prop);
    if (clauncher && *clauncher) {
      std::vector<std::string> launcher_cmd;
      cmSystemTools::ExpandListArgument(clauncher, launcher_cmd, true);
      for (std::vector<std::string>::iterator i = launcher_cmd.begin(),
                                              e = launcher_cmd.end();
           i != e; ++i) {
        *i = this->LocalGenerator->EscapeForShell(*i);
      }
      std::string const& run_launcher = cmJoin(launcher_cmd, " ") + " ";
      compileCmds.front().insert(0, run_launcher);
    }
  }

  if (!compileCmds.empty()) {
    compileCmds.front().insert(0, cldeps);
  }

  for (std::vector<std::string>::iterator i = compileCmds.begin();
       i != compileCmds.end(); ++i)
    this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);

  std::string cmdLine =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds);

  // Write the rule for compiling file of the given language.
  std::ostringstream comment;
  comment << "Rule for compiling " << lang << " files.";
  std::ostringstream description;
  description << "Building " << lang << " object $out";
  this->GetGlobalGenerator()->AddRule(
    this->LanguageCompilerRule(lang), cmdLine, description.str(),
    comment.str(), depfile, deptype, rspfile, rspcontent,
    /*restat*/ "",
    /*generator*/ false);
}

void cmNinjaTargetGenerator::WriteObjectBuildStatements()
{
  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  this->GetBuildFileStream()
    << "# Object build statements for "
    << cmState::GetTargetTypeName(this->GetGeneratorTarget()->GetType())
    << " target " << this->GetTargetName() << "\n\n";

  std::string config = this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  std::vector<cmSourceFile const*> customCommands;
  this->GeneratorTarget->GetCustomCommands(customCommands, config);
  for (std::vector<cmSourceFile const*>::const_iterator si =
         customCommands.begin();
       si != customCommands.end(); ++si) {
    cmCustomCommand const* cc = (*si)->GetCustomCommand();
    this->GetLocalGenerator()->AddCustomCommandTarget(
      cc, this->GetGeneratorTarget());
    // Record the custom commands for this target. The container is used
    // in WriteObjectBuildStatement when called in a loop below.
    this->CustomCommands.push_back(cc);
  }
  std::vector<cmSourceFile const*> headerSources;
  this->GeneratorTarget->GetHeaderSources(headerSources, config);
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    headerSources, this->MacOSXContentGenerator);
  std::vector<cmSourceFile const*> extraSources;
  this->GeneratorTarget->GetExtraSources(extraSources, config);
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    extraSources, this->MacOSXContentGenerator);
  std::vector<cmSourceFile const*> externalObjects;
  this->GeneratorTarget->GetExternalObjects(externalObjects, config);
  for (std::vector<cmSourceFile const*>::const_iterator si =
         externalObjects.begin();
       si != externalObjects.end(); ++si) {
    this->Objects.push_back(this->GetSourceFilePath(*si));
  }

  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->GeneratorTarget,
                                                 orderOnlyDeps);

  // Add order-only dependencies on custom command outputs.
  for (std::vector<cmCustomCommand const*>::const_iterator cci =
         this->CustomCommands.begin();
       cci != this->CustomCommands.end(); ++cci) {
    cmCustomCommand const* cc = *cci;
    cmCustomCommandGenerator ccg(*cc, this->GetConfigName(),
                                 this->GetLocalGenerator());
    const std::vector<std::string>& ccoutputs = ccg.GetOutputs();
    const std::vector<std::string>& ccbyproducts = ccg.GetByproducts();
    std::transform(ccoutputs.begin(), ccoutputs.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
    std::transform(ccbyproducts.begin(), ccbyproducts.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
  }

  if (!orderOnlyDeps.empty()) {
    cmNinjaDeps orderOnlyTarget;
    orderOnlyTarget.push_back(this->OrderDependsTargetForTarget());
    this->GetGlobalGenerator()->WritePhonyBuild(
      this->GetBuildFileStream(),
      "Order-only phony target for " + this->GetTargetName(), orderOnlyTarget,
      cmNinjaDeps(), cmNinjaDeps(), orderOnlyDeps);
  }
  std::vector<cmSourceFile const*> objectSources;
  this->GeneratorTarget->GetObjectSources(objectSources, config);
  for (std::vector<cmSourceFile const*>::const_iterator si =
         objectSources.begin();
       si != objectSources.end(); ++si) {
    this->WriteObjectBuildStatement(*si, !orderOnlyDeps.empty());
  }

  this->GetBuildFileStream() << "\n";
}

void cmNinjaTargetGenerator::WriteObjectBuildStatement(
  cmSourceFile const* source, bool writeOrderDependsTargetForTarget)
{
  std::string const language = source->GetLanguage();
  std::string const sourceFileName =
    language == "RC" ? source->GetFullPath() : this->GetSourceFilePath(source);
  std::string const objectDir =
    this->ConvertToNinjaPath(this->GeneratorTarget->GetSupportDirectory());
  std::string const objectFileName =
    this->ConvertToNinjaPath(this->GetObjectFilePath(source));
  std::string const objectFileDir =
    cmSystemTools::GetFilenamePath(objectFileName);

  cmNinjaVars vars;
  vars["FLAGS"] = this->ComputeFlagsForObject(source, language);
  vars["DEFINES"] = this->ComputeDefines(source, language);
  vars["INCLUDES"] = this->GetIncludes(language);
  if (!this->NeedDepTypeMSVC(language)) {
    vars["DEP_FILE"] =
      cmGlobalNinjaGenerator::EncodeDepfileSpace(objectFileName + ".d");
  }

  this->ExportObjectCompileCommand(
    language, sourceFileName, objectDir, objectFileName, objectFileDir,
    vars["FLAGS"], vars["DEFINES"], vars["INCLUDES"]);

  std::string comment;
  std::string rule = this->LanguageCompilerRule(language);

  cmNinjaDeps outputs;
  outputs.push_back(objectFileName);
  // Add this object to the list of object files.
  this->Objects.push_back(objectFileName);

  cmNinjaDeps explicitDeps;
  explicitDeps.push_back(sourceFileName);

  cmNinjaDeps implicitDeps;
  if (const char* objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    std::vector<std::string> depList;
    cmSystemTools::ExpandListArgument(objectDeps, depList);
    for (std::vector<std::string>::iterator odi = depList.begin();
         odi != depList.end(); ++odi) {
      if (cmSystemTools::FileIsFullPath(*odi)) {
        *odi = cmSystemTools::CollapseFullPath(*odi);
      }
    }
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(implicitDeps), MapToNinjaPath());
  }

  cmNinjaDeps orderOnlyDeps;
  if (writeOrderDependsTargetForTarget) {
    orderOnlyDeps.push_back(this->OrderDependsTargetForTarget());
  }

  // If the source file is GENERATED and does not have a custom command
  // (either attached to this source file or another one), assume that one of
  // the target dependencies, OBJECT_DEPENDS or header file custom commands
  // will rebuild the file.
  if (source->GetPropertyAsBool("GENERATED") && !source->GetCustomCommand() &&
      !this->GetGlobalGenerator()->HasCustomCommandOutput(sourceFileName)) {
    this->GetGlobalGenerator()->AddAssumedSourceDependencies(sourceFileName,
                                                             orderOnlyDeps);
  }

  EnsureParentDirectoryExists(objectFileName);

  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectDir, cmOutputConverter::SHELL);
  vars["OBJECT_FILE_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    objectFileDir, cmOutputConverter::SHELL);

  this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetGeneratorTarget(),
                             vars);

  this->SetMsvcTargetPdbVariable(vars);

  int const commandLineLengthLimit = this->ForceResponseFile() ? -1 : 0;
  std::string const rspfile = objectFileName + ".rsp";

  this->GetGlobalGenerator()->WriteBuild(
    this->GetBuildFileStream(), comment, rule, outputs, explicitDeps,
    implicitDeps, orderOnlyDeps, vars, rspfile, commandLineLengthLimit);

  if (const char* objectOutputs = source->GetProperty("OBJECT_OUTPUTS")) {
    std::vector<std::string> outputList;
    cmSystemTools::ExpandListArgument(objectOutputs, outputList);
    std::transform(outputList.begin(), outputList.end(), outputList.begin(),
                   MapToNinjaPath());
    this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                                "Additional output files.",
                                                outputList, outputs);
  }
}

void cmNinjaTargetGenerator::ExportObjectCompileCommand(
  std::string const& language, std::string const& sourceFileName,
  std::string const& objectDir, std::string const& objectFileName,
  std::string const& objectFileDir, std::string const& flags,
  std::string const& defines, std::string const& includes)
{
  if (!this->Makefile->IsOn("CMAKE_EXPORT_COMPILE_COMMANDS")) {
    return;
  }

  cmLocalGenerator::RuleVariables compileObjectVars;
  compileObjectVars.Language = language.c_str();

  std::string escapedSourceFileName = sourceFileName;

  if (!cmSystemTools::FileIsFullPath(sourceFileName.c_str())) {
    escapedSourceFileName = cmSystemTools::CollapseFullPath(
      escapedSourceFileName, this->GetGlobalGenerator()
                               ->GetCMakeInstance()
                               ->GetHomeOutputDirectory());
  }

  escapedSourceFileName = this->LocalGenerator->ConvertToOutputFormat(
    escapedSourceFileName, cmOutputConverter::SHELL);

  compileObjectVars.Source = escapedSourceFileName.c_str();
  compileObjectVars.Object = objectFileName.c_str();
  compileObjectVars.ObjectDir = objectDir.c_str();
  compileObjectVars.ObjectFileDir = objectFileDir.c_str();
  compileObjectVars.Flags = flags.c_str();
  compileObjectVars.Defines = defines.c_str();
  compileObjectVars.Includes = includes.c_str();

  // Rule for compiling object file.
  std::string compileCmdVar = "CMAKE_";
  compileCmdVar += language;
  compileCmdVar += "_COMPILE_OBJECT";
  std::string compileCmd =
    this->GetMakefile()->GetRequiredDefinition(compileCmdVar);
  std::vector<std::string> compileCmds;
  cmSystemTools::ExpandListArgument(compileCmd, compileCmds);

  for (std::vector<std::string>::iterator i = compileCmds.begin();
       i != compileCmds.end(); ++i)
    this->GetLocalGenerator()->ExpandRuleVariables(*i, compileObjectVars);

  std::string cmdLine =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds);

  this->GetGlobalGenerator()->AddCXXCompileCommand(cmdLine, sourceFileName);
}

void cmNinjaTargetGenerator::EnsureDirectoryExists(
  const std::string& path) const
{
  if (cmSystemTools::FileIsFullPath(path.c_str())) {
    cmSystemTools::MakeDirectory(path.c_str());
  } else {
    cmGlobalNinjaGenerator* gg = this->GetGlobalGenerator();
    std::string fullPath =
      std::string(gg->GetCMakeInstance()->GetHomeOutputDirectory());
    // Also ensures their is a trailing slash.
    gg->StripNinjaOutputPathPrefixAsSuffix(fullPath);
    fullPath += path;
    cmSystemTools::MakeDirectory(fullPath.c_str());
  }
}

void cmNinjaTargetGenerator::EnsureParentDirectoryExists(
  const std::string& path) const
{
  EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path));
}

void cmNinjaTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile const& source, const char* pkgloc)
{
  // Skip OS X content when not building a Framework or Bundle.
  if (!this->Generator->GetGeneratorTarget()->IsBundleOnApple()) {
    return;
  }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc);

  // Get the input file location.
  std::string input = source.GetFullPath();
  input = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(input);

  // Get the output file location.
  std::string output = macdir;
  output += "/";
  output += cmSystemTools::GetFilenameName(input);
  output = this->Generator->GetGlobalGenerator()->ConvertToNinjaPath(output);

  // Write a build statement to copy the content into the bundle.
  this->Generator->GetGlobalGenerator()->WriteMacOSXContentBuild(input,
                                                                 output);

  // Add as a dependency of all target so that it gets called.
  this->Generator->GetGlobalGenerator()->AddDependencyToAll(output);
}

void cmNinjaTargetGenerator::addPoolNinjaVariable(
  const std::string& pool_property, cmGeneratorTarget* target,
  cmNinjaVars& vars)
{
  const char* pool = target->GetProperty(pool_property);
  if (pool) {
    vars["pool"] = pool;
  }
}

bool cmNinjaTargetGenerator::ForceResponseFile()
{
  static std::string const forceRspFile = "CMAKE_NINJA_FORCE_RESPONSE_FILE";
  return (this->GetMakefile()->IsDefinitionSet(forceRspFile) ||
          cmSystemTools::GetEnv(forceRspFile) != 0);
}
