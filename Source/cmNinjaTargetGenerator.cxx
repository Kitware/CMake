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
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmNinjaNormalTargetGenerator.h"
#include "cmNinjaUtilityTargetGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmComputeLinkInformation.h"
#include "cmSourceFile.h"
#include "cmCustomCommandGenerator.h"
#include "cmAlgorithms.h"

#include <algorithm>

cmNinjaTargetGenerator *
cmNinjaTargetGenerator::New(cmGeneratorTarget* target)
{
  switch (target->GetType())
    {
      case cmTarget::EXECUTABLE:
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
      case cmTarget::OBJECT_LIBRARY:
        return new cmNinjaNormalTargetGenerator(target);

      case cmTarget::UTILITY:
        return new cmNinjaUtilityTargetGenerator(target);;

      case cmTarget::GLOBAL_TARGET: {
        // We only want to process global targets that live in the home
        // (i.e. top-level) directory.  CMake creates copies of these targets
        // in every directory, which we don't need.
        cmMakefile *mf = target->Target->GetMakefile();
        if (strcmp(mf->GetCurrentSourceDirectory(),
                   mf->GetHomeDirectory()) == 0)
          return new cmNinjaUtilityTargetGenerator(target);
        // else fallthrough
      }

      default:
        return 0;
    }
}

cmNinjaTargetGenerator::cmNinjaTargetGenerator(cmGeneratorTarget* target)
  : cmCommonTargetGenerator(target),
    MacOSXContentGenerator(0),
    OSXBundleGenerator(0),
    MacContentFolders(),
    LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(target->GetLocalGenerator())),
    Objects()
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
    cmGlobalNinjaGenerator::EncodeRuleName(this->Target->GetName());
}

std::string
cmNinjaTargetGenerator::OrderDependsTargetForTarget()
{
  return "cmake_order_depends_target_" + this->GetTargetName();
}

// TODO: Most of the code is picked up from
// void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink),
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
// Refactor it.
std::string
cmNinjaTargetGenerator::ComputeFlagsForObject(cmSourceFile const* source,
                                              const std::string& language)
{
  std::string flags = this->GetFlags(language);

  // Add Fortran format flags.
  if(language == "Fortran")
    {
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
  this->LocalGenerator->GetIncludeDirectories(includes,
                                              this->GeneratorTarget,
                                              language,
                                              this->GetConfigName());
  // Add include directory flags.
  std::string includeFlags =
    this->LocalGenerator->GetIncludeFlags(includes, this->GeneratorTarget,
                                          language,
        language == "RC" ? true : false,  // full include paths for RC
                                          // needed by cmcldeps
                                          false,
                                          this->GetConfigName());
  if (this->GetGlobalGenerator()->IsGCCOnWindows())
    cmSystemTools::ReplaceString(includeFlags, "\\", "/");

  this->LocalGenerator->AppendFlags(languageFlags, includeFlags);
}

bool cmNinjaTargetGenerator::NeedDepTypeMSVC(const std::string& lang) const
{
  if (lang == "C" || lang == "CXX")
    {
    cmMakefile* mf = this->GetMakefile();
    return (
      strcmp(mf->GetSafeDefinition("CMAKE_C_COMPILER_ID"), "MSVC") == 0 ||
      strcmp(mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID"), "MSVC") == 0 ||
      strcmp(mf->GetSafeDefinition("CMAKE_C_SIMULATE_ID"), "MSVC") == 0 ||
      strcmp(mf->GetSafeDefinition("CMAKE_CXX_SIMULATE_ID"), "MSVC") == 0
      );
    }
  return false;
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string
cmNinjaTargetGenerator::
ComputeDefines(cmSourceFile const* source, const std::string& language)
{
  std::set<std::string> defines;
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty("COMPILE_DEFINITIONS"));
  {
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(this->GetConfigName());
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty(defPropName));
  }

  std::string definesString = this->GetDefines(language);
  this->LocalGenerator->JoinDefines(defines, definesString,
     language);

  return definesString;
}

cmNinjaDeps cmNinjaTargetGenerator::ComputeLinkDeps() const
{
  // Static libraries never depend on other targets for linking.
  if (this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
      this->Target->GetType() == cmTarget::OBJECT_LIBRARY)
    return cmNinjaDeps();

  cmComputeLinkInformation* cli =
    this->Target->GetLinkInformation(this->GetConfigName());
  if(!cli)
    return cmNinjaDeps();

  const std::vector<std::string> &deps = cli->GetDepends();
  cmNinjaDeps result(deps.size());
  std::transform(deps.begin(), deps.end(), result.begin(), MapToNinjaPath());

  // Add a dependency on the link definitions file, if any.
  if(!this->ModuleDefinitionFile.empty())
    {
    result.push_back(this->ConvertToNinjaPath(this->ModuleDefinitionFile));
    }

  return result;
}

std::string
cmNinjaTargetGenerator
::GetSourceFilePath(cmSourceFile const* source) const
{
  return ConvertToNinjaPath(source->GetFullPath());
}

std::string
cmNinjaTargetGenerator
::GetObjectFilePath(cmSourceFile const* source) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if(!path.empty())
    path += "/";
  std::string const& objectName = this->GeneratorTarget
                                      ->GetObjectName(source);
  path += this->LocalGenerator->GetTargetDirectory(*this->Target);
  path += "/";
  path += objectName;
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetOutputDir() const
{
  std::string dir = this->Target->GetDirectory(this->GetConfigName());
  return ConvertToNinjaPath(dir);
}

std::string
cmNinjaTargetGenerator
::GetTargetFilePath(const std::string& name) const
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
  return this->Target->GetName();
}


bool cmNinjaTargetGenerator::SetMsvcTargetPdbVariable(cmNinjaVars& vars) const
{
  cmMakefile* mf = this->GetMakefile();
  if (mf->GetDefinition("MSVC_C_ARCHITECTURE_ID") ||
      mf->GetDefinition("MSVC_CXX_ARCHITECTURE_ID"))
    {
    std::string pdbPath;
    std::string compilePdbPath;
    if(this->Target->GetType() == cmTarget::EXECUTABLE ||
       this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
       this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
       this->Target->GetType() == cmTarget::MODULE_LIBRARY)
      {
      pdbPath = this->Target->GetPDBDirectory(this->GetConfigName());
      pdbPath += "/";
      pdbPath += this->Target->GetPDBName(this->GetConfigName());
      }
    if(this->Target->GetType() <= cmTarget::OBJECT_LIBRARY)
      {
      compilePdbPath = this->Target->GetCompilePDBPath(this->GetConfigName());
      if(compilePdbPath.empty())
        {
        compilePdbPath = this->Target->GetSupportDirectory() + "/";
        }
      }

    vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
                          ConvertToNinjaPath(pdbPath),
                          cmLocalGenerator::SHELL);
    vars["TARGET_COMPILE_PDB"] =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        ConvertToNinjaPath(compilePdbPath),
        cmLocalGenerator::SHELL);

    EnsureParentDirectoryExists(pdbPath);
    EnsureParentDirectoryExists(compilePdbPath);
    return true;
    }
  return false;
}

void
cmNinjaTargetGenerator
::WriteLanguageRules(const std::string& language)
{
#ifdef NINJA_GEN_VERBOSE_FILES
  this->GetRulesFileStream()
    << "# Rules for language " << language << "\n\n";
#endif
  this->WriteCompileRule(language);
}

void
cmNinjaTargetGenerator
::WriteCompileRule(const std::string& lang)
{
  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_COMPILE";
  vars.CMTarget = this->GetTarget();
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

  // Tell ninja dependency format so all deps can be loaded into a database
  std::string deptype;
  std::string depfile;
  std::string cldeps;
  std::string flags = "$FLAGS";
  if (this->NeedDepTypeMSVC(lang))
    {
    deptype = "msvc";
    depfile = "";
    flags += " /showIncludes";
    }
  else if (lang == "RC" && this->NeedDepTypeMSVC("C"))
    {
    // For the MS resource compiler we need cmcldeps, but skip dependencies
    // for source-file try_compile cases because they are always fresh.
    if (!mf->GetIsSourceFileTryCompile())
      {
      deptype = "gcc";
      depfile = "$DEP_FILE";
      const std::string cl = mf->GetDefinition("CMAKE_C_COMPILER") ?
                        mf->GetSafeDefinition("CMAKE_C_COMPILER") :
                        mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
      cldeps =  "\"";
      cldeps += mf->GetSafeDefinition("CMAKE_CMCLDEPS_EXECUTABLE");
      cldeps += "\" " + lang + " $in \"$DEP_FILE\" $out \"";
      cldeps += mf->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
      cldeps += "\" \"" + cl + "\" ";
      }
    }
  else
    {
    deptype = "gcc";
    const char* langdeptype = mf->GetDefinition("CMAKE_NINJA_DEPTYPE_" + lang);
    if (langdeptype)
      {
      deptype = langdeptype;
      }
    depfile = "$DEP_FILE";
    const std::string flagsName = "CMAKE_DEPFILE_FLAGS_" + lang;
    std::string depfileFlags = mf->GetSafeDefinition(flagsName);
    if (!depfileFlags.empty())
      {
      cmSystemTools::ReplaceString(depfileFlags, "<DEPFILE>", "$DEP_FILE");
      cmSystemTools::ReplaceString(depfileFlags, "<OBJECT>",  "$out");
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
  if (!compileCmds.empty() && (lang == "C" || lang == "CXX"))
    {
    std::string const iwyu_prop = lang + "_INCLUDE_WHAT_YOU_USE";
    const char *iwyu = this->Target->GetProperty(iwyu_prop);
    if (iwyu && *iwyu)
      {
      std::string run_iwyu =
        this->GetLocalGenerator()->ConvertToOutputFormat(
          cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
      run_iwyu += " -E __run_iwyu --iwyu=";
      run_iwyu += this->GetLocalGenerator()->EscapeForShell(iwyu);
      run_iwyu += " -- ";
      compileCmds.front().insert(0, run_iwyu);
      }
    }

  // Maybe insert a compiler launcher like ccache or distcc
  if (!compileCmds.empty() && (lang == "C" || lang == "CXX"))
    {
    std::string const clauncher_prop = lang + "_COMPILER_LAUNCHER";
    const char *clauncher = this->Target->GetProperty(clauncher_prop);
    if (clauncher && *clauncher)
      {
      std::vector<std::string> launcher_cmd;
      cmSystemTools::ExpandListArgument(clauncher, launcher_cmd, true);
      for (std::vector<std::string>::iterator i = launcher_cmd.begin(),
             e = launcher_cmd.end(); i != e; ++i)
        {
        *i = this->LocalGenerator->EscapeForShell(*i);
        }
      std::string const& run_launcher = cmJoin(launcher_cmd, " ") + " ";
      compileCmds.front().insert(0, run_launcher);
      }
    }

  if (!compileCmds.empty())
    {
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
  this->GetGlobalGenerator()->AddRule(this->LanguageCompilerRule(lang),
                                      cmdLine,
                                      description.str(),
                                      comment.str(),
                                      depfile,
                                      deptype,
                                      /*rspfile*/ "",
                                      /*rspcontent*/ "",
                                      /*restat*/ "",
                                      /*generator*/ false);
}

void
cmNinjaTargetGenerator
::WriteObjectBuildStatements()
{
  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  this->GetBuildFileStream()
    << "# Object build statements for "
    << cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    << " target "
    << this->GetTargetName()
    << "\n\n";

  std::string config = this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  std::vector<cmSourceFile const*> customCommands;
  this->GeneratorTarget->GetCustomCommands(customCommands, config);
  for(std::vector<cmSourceFile const*>::const_iterator
        si = customCommands.begin();
      si != customCommands.end(); ++si)
     {
     cmCustomCommand const* cc = (*si)->GetCustomCommand();
     this->GetLocalGenerator()->AddCustomCommandTarget(cc, this->GetTarget());
     // Record the custom commands for this target. The container is used
     // in WriteObjectBuildStatement when called in a loop below.
     this->CustomCommands.push_back(cc);
     }
  std::vector<cmSourceFile const*> headerSources;
  this->GeneratorTarget->GetHeaderSources(headerSources, config);
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    headerSources,
    this->MacOSXContentGenerator);
  std::vector<cmSourceFile const*> extraSources;
  this->GeneratorTarget->GetExtraSources(extraSources, config);
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    extraSources,
    this->MacOSXContentGenerator);
  std::vector<cmSourceFile const*> externalObjects;
  this->GeneratorTarget->GetExternalObjects(externalObjects, config);
  for(std::vector<cmSourceFile const*>::const_iterator
        si = externalObjects.begin();
      si != externalObjects.end(); ++si)
    {
    this->Objects.push_back(this->GetSourceFilePath(*si));
    }

  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->Target, orderOnlyDeps);

  // Add order-only dependencies on custom command outputs.
  for(std::vector<cmCustomCommand const*>::const_iterator
        cci = this->CustomCommands.begin();
      cci != this->CustomCommands.end(); ++cci)
    {
    cmCustomCommand const* cc = *cci;
    cmCustomCommandGenerator ccg(*cc, this->GetConfigName(),
                                 this->GetMakefile());
    const std::vector<std::string>& ccoutputs = ccg.GetOutputs();
    const std::vector<std::string>& ccbyproducts= ccg.GetByproducts();
    std::transform(ccoutputs.begin(), ccoutputs.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
    std::transform(ccbyproducts.begin(), ccbyproducts.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
    }

  if (!orderOnlyDeps.empty())
    {
    cmNinjaDeps orderOnlyTarget;
    orderOnlyTarget.push_back(this->OrderDependsTargetForTarget());
    this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                                "Order-only phony target for "
                                                  + this->GetTargetName(),
                                                orderOnlyTarget,
                                                cmNinjaDeps(),
                                                cmNinjaDeps(),
                                                orderOnlyDeps);
    }
  std::vector<cmSourceFile const*> objectSources;
  this->GeneratorTarget->GetObjectSources(objectSources, config);
  for(std::vector<cmSourceFile const*>::const_iterator
        si = objectSources.begin(); si != objectSources.end(); ++si)
    {
    this->WriteObjectBuildStatement(*si, !orderOnlyDeps.empty());
    }

  this->GetBuildFileStream() << "\n";
}

void
cmNinjaTargetGenerator
::WriteObjectBuildStatement(
  cmSourceFile const* source, bool writeOrderDependsTargetForTarget)
{
  std::string comment;
  const std::string language = source->GetLanguage();
  std::string rule = this->LanguageCompilerRule(language);

  cmNinjaDeps outputs;
  std::string objectFileName = this->GetObjectFilePath(source);
  outputs.push_back(objectFileName);
  // Add this object to the list of object files.
  this->Objects.push_back(objectFileName);

  cmNinjaDeps explicitDeps;
  std::string sourceFileName;
  if (language == "RC")
    sourceFileName = source->GetFullPath();
  else
    sourceFileName = this->GetSourceFilePath(source);
  explicitDeps.push_back(sourceFileName);

  cmNinjaDeps implicitDeps;
  if(const char* objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    std::vector<std::string> depList;
    cmSystemTools::ExpandListArgument(objectDeps, depList);
    for(std::vector<std::string>::iterator odi = depList.begin();
        odi != depList.end(); ++odi)
      {
      if (cmSystemTools::FileIsFullPath(*odi))
        {
        *odi = cmSystemTools::CollapseFullPath(*odi);
        }
      }
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(implicitDeps), MapToNinjaPath());
  }

  cmNinjaDeps orderOnlyDeps;
  if (writeOrderDependsTargetForTarget)
    {
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

  cmNinjaVars vars;
  vars["FLAGS"] = this->ComputeFlagsForObject(source, language);
  vars["DEFINES"] = this->ComputeDefines(source, language);
  vars["INCLUDES"] = this->GetIncludes(language);
  if (!this->NeedDepTypeMSVC(language)) {
    vars["DEP_FILE"] =
            cmGlobalNinjaGenerator::EncodeDepfileSpace(objectFileName + ".d");
  }
  EnsureParentDirectoryExists(objectFileName);

  std::string objectDir = this->Target->GetSupportDirectory();
  vars["OBJECT_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
                         ConvertToNinjaPath(objectDir),
                         cmLocalGenerator::SHELL);
  std::string objectFileDir = cmSystemTools::GetFilenamePath(objectFileName);
  vars["OBJECT_FILE_DIR"] = this->GetLocalGenerator()->ConvertToOutputFormat(
                              ConvertToNinjaPath(objectFileDir),
                              cmLocalGenerator::SHELL);

  this->addPoolNinjaVariable("JOB_POOL_COMPILE", this->GetTarget(), vars);

  this->SetMsvcTargetPdbVariable(vars);

  if(this->Makefile->IsOn("CMAKE_EXPORT_COMPILE_COMMANDS"))
    {
    cmLocalGenerator::RuleVariables compileObjectVars;
    std::string lang = language;
    compileObjectVars.Language = lang.c_str();

    std::string escapedSourceFileName = sourceFileName;

    if (!cmSystemTools::FileIsFullPath(sourceFileName.c_str()))
      {
      escapedSourceFileName = cmSystemTools::CollapseFullPath(
        escapedSourceFileName,
        this->GetGlobalGenerator()->GetCMakeInstance()->
          GetHomeOutputDirectory());
      }

    escapedSourceFileName =
      this->LocalGenerator->ConvertToOutputFormat(
        escapedSourceFileName, cmLocalGenerator::SHELL);

    compileObjectVars.Source = escapedSourceFileName.c_str();
    compileObjectVars.Object = objectFileName.c_str();
    compileObjectVars.ObjectDir = objectDir.c_str();
    compileObjectVars.ObjectFileDir = objectFileDir.c_str();
    compileObjectVars.Flags = vars["FLAGS"].c_str();
    compileObjectVars.Defines = vars["DEFINES"].c_str();
    compileObjectVars.Includes = vars["INCLUDES"].c_str();

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

    this->GetGlobalGenerator()->AddCXXCompileCommand(cmdLine,
                                                     sourceFileName);
    }

  this->GetGlobalGenerator()->WriteBuild(this->GetBuildFileStream(),
                                         comment,
                                         rule,
                                         outputs,
                                         explicitDeps,
                                         implicitDeps,
                                         orderOnlyDeps,
                                         vars);

  if(const char* objectOutputs = source->GetProperty("OBJECT_OUTPUTS")) {
    std::vector<std::string> outputList;
    cmSystemTools::ExpandListArgument(objectOutputs, outputList);
    std::transform(outputList.begin(), outputList.end(), outputList.begin(),
                   MapToNinjaPath());
    this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                                "Additional output files.",
                                                outputList,
                                                outputs);
  }
}

void
cmNinjaTargetGenerator
::EnsureDirectoryExists(const std::string& path) const
{
  if (cmSystemTools::FileIsFullPath(path.c_str()))
    {
    cmSystemTools::MakeDirectory(path.c_str());
    }
  else
    {
    const std::string fullPath = std::string(this->GetGlobalGenerator()->
                                 GetCMakeInstance()->GetHomeOutputDirectory())
                                   + "/" + path;
    cmSystemTools::MakeDirectory(fullPath.c_str());
    }
}

void
cmNinjaTargetGenerator
::EnsureParentDirectoryExists(const std::string& path) const
{
  EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path));
}


//----------------------------------------------------------------------------
void
cmNinjaTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile const& source, const char* pkgloc)
{
  // Skip OS X content when not building a Framework or Bundle.
  if(!this->Generator->GetTarget()->IsBundleOnApple())
    {
    return;
    }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc);

  // Get the input file location.
  std::string input = source.GetFullPath();
  input =
    this->Generator->GetLocalGenerator()->ConvertToNinjaPath(input);

  // Get the output file location.
  std::string output = macdir;
  output += "/";
  output += cmSystemTools::GetFilenameName(input);
  output =
    this->Generator->GetLocalGenerator()->ConvertToNinjaPath(output);

  // Write a build statement to copy the content into the bundle.
  this->Generator->GetGlobalGenerator()->WriteMacOSXContentBuild(input,
                                                                 output);

  // Add as a dependency of all target so that it gets called.
  this->Generator->GetGlobalGenerator()->AddDependencyToAll(output);
}

void cmNinjaTargetGenerator::addPoolNinjaVariable(
                                              const std::string& pool_property,
                                              cmTarget* target,
                                              cmNinjaVars& vars)
{
    const char* pool = target->GetProperty(pool_property);
    if (pool)
      {
      vars["pool"] = pool;
      }
}
