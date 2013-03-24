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

#include <algorithm>

cmNinjaTargetGenerator *
cmNinjaTargetGenerator::New(cmTarget* target)
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
        cmMakefile *mf = target->GetMakefile();
        if (strcmp(mf->GetStartDirectory(), mf->GetHomeDirectory()) == 0)
          return new cmNinjaUtilityTargetGenerator(target);
        // else fallthrough
      }

      default:
        return 0;
    }
}

cmNinjaTargetGenerator::cmNinjaTargetGenerator(cmTarget* target)
  :
    MacOSXContentGenerator(0),
    OSXBundleGenerator(0),
    MacContentFolders(),
    Target(target),
    Makefile(target->GetMakefile()),
    LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(Makefile->GetLocalGenerator())),
    Objects()
{
  this->GeneratorTarget =
    this->GetGlobalGenerator()->GetGeneratorTarget(target);
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

const char* cmNinjaTargetGenerator::GetConfigName() const
{
  return this->LocalGenerator->GetConfigName();
}

// TODO: Picked up from cmMakefileTargetGenerator.  Refactor it.
const char* cmNinjaTargetGenerator::GetFeature(const char* feature)
{
  return this->Target->GetFeature(feature, this->GetConfigName());
}

// TODO: Picked up from cmMakefileTargetGenerator.  Refactor it.
bool cmNinjaTargetGenerator::GetFeatureAsBool(const char* feature)
{
  return cmSystemTools::IsOn(this->GetFeature(feature));
}

// TODO: Picked up from cmMakefileTargetGenerator.  Refactor it.
void cmNinjaTargetGenerator::AddFeatureFlags(std::string& flags,
                                             const char* lang)
{
  // Add language-specific flags.
  this->LocalGenerator->AddLanguageFlags(flags, lang, this->GetConfigName());

  if(this->GetFeatureAsBool("INTERPROCEDURAL_OPTIMIZATION"))
    {
    this->LocalGenerator->AppendFeatureOptions(flags, lang, "IPO");
    }
}

// TODO: Most of the code is picked up from
// void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink),
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
// Refactor it.
std::string
cmNinjaTargetGenerator::ComputeFlagsForObject(cmSourceFile *source,
                                              const std::string& language)
{
  std::string flags;

  this->AddFeatureFlags(flags, language.c_str());

  this->GetLocalGenerator()->AddArchitectureFlags(flags,
                                                  this->GeneratorTarget,
                                                  language.c_str(),
                                                  this->GetConfigName());

  // TODO: Fortran support.
  // // Fortran-specific flags computed for this target.
  // if(*l == "Fortran")
  //   {
  //   this->AddFortranFlags(flags);
  //   }

  // Add shared-library flags if needed.
  this->LocalGenerator->AddCMP0018Flags(flags, this->Target,
                                        language.c_str(),
                                        this->GetConfigName());

  // Add include directory flags.
  {
  std::vector<std::string> includes;
  const char *config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  this->LocalGenerator->GetIncludeDirectories(includes,
                                              this->GeneratorTarget,
                                              language.c_str(), config);
  std::string includeFlags =
    this->LocalGenerator->GetIncludeFlags(includes, language.c_str(),
    language == "RC" ? true : false); // full include paths for RC
                                      // needed by cmcldeps
  if(cmGlobalNinjaGenerator::IsMinGW())
    cmSystemTools::ReplaceString(includeFlags, "\\", "/");

  this->LocalGenerator->AppendFlags(flags, includeFlags.c_str());
  }

  // Append old-style preprocessor definition flags.
  this->LocalGenerator->AppendFlags(flags, this->Makefile->GetDefineFlags());

  // Add target-specific flags.
  if(this->Target->GetProperty("COMPILE_FLAGS"))
    {
    std::string langIncludeExpr = "CMAKE_";
    langIncludeExpr += language;
    langIncludeExpr += "_FLAG_REGEX";
    const char* regex = this->Makefile->
      GetDefinition(langIncludeExpr.c_str());
    if(regex)
      {
      cmsys::RegularExpression r(regex);
      std::vector<std::string> args;
      cmSystemTools::ParseWindowsCommandLine(
        this->Target->GetProperty("COMPILE_FLAGS"),
        args);
      for(std::vector<std::string>::iterator i = args.begin();
          i != args.end(); ++i)
        {
        if(r.find(i->c_str()))
          {
          this->LocalGenerator->AppendFlags
            (flags, i->c_str());
          }
        }
      }
    else
      {
      this->LocalGenerator->AppendFlags
        (flags, this->Target->GetProperty("COMPILE_FLAGS"));
      }
    }

    // Add source file specific flags.
    this->LocalGenerator->AppendFlags(flags,
      source->GetProperty("COMPILE_FLAGS"));

  // TODO: Handle Apple frameworks.

  return flags;
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string
cmNinjaTargetGenerator::
ComputeDefines(cmSourceFile *source, const std::string& language)
{
  std::set<std::string> defines;

  // Add the export symbol definition for shared library objects.
  if(const char* exportMacro = this->Target->GetExportMacro())
    {
    this->LocalGenerator->AppendDefines(defines, exportMacro);
    }

  // Add preprocessor definitions for this target and configuration.
  this->LocalGenerator->AppendDefines
    (defines,
     this->Target->GetCompileDefinitions(this->GetConfigName()));
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty("COMPILE_DEFINITIONS"));
  {
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(this->GetConfigName());
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty(defPropName.c_str()));
  }

  std::string definesString;
  this->LocalGenerator->JoinDefines(defines, definesString,
     language.c_str());

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
    result.push_back(this->ModuleDefinitionFile);
    }

  return result;
}

std::string
cmNinjaTargetGenerator
::GetSourceFilePath(cmSourceFile* source) const
{
  return ConvertToNinjaPath(source->GetFullPath().c_str());
}

std::string
cmNinjaTargetGenerator
::GetObjectFilePath(cmSourceFile* source) const
{
  std::string path = this->LocalGenerator->GetHomeRelativeOutputPath();
  if(!path.empty())
    path += "/";
  std::string const& objectName = this->GeneratorTarget->Objects[source];
  path += this->LocalGenerator->GetTargetDirectory(*this->Target);
  path += "/";
  path += objectName;
  return path;
}

std::string cmNinjaTargetGenerator::GetTargetOutputDir() const
{
  std::string dir = this->Target->GetDirectory(this->GetConfigName());
  return ConvertToNinjaPath(dir.c_str());
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
    if(this->Target->GetType() == cmTarget::EXECUTABLE ||
       this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
       this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
       this->Target->GetType() == cmTarget::MODULE_LIBRARY)
      {
      pdbPath = this->Target->GetPDBDirectory(this->GetConfigName());
      pdbPath += "/";
      pdbPath += this->Target->GetPDBName(this->GetConfigName());
      }

    vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
                          ConvertToNinjaPath(pdbPath.c_str()).c_str(),
                          cmLocalGenerator::SHELL);
    EnsureParentDirectoryExists(pdbPath);
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
::WriteCompileRule(const std::string& language)
{
  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_COMPILE";
  vars.CMTarget = this->GetTarget();
  std::string lang = language;
  vars.Language = lang.c_str();
  vars.Source = "$in";
  vars.Object = "$out";
  std::string flags = "$FLAGS";
  vars.Defines = "$DEFINES";
  vars.TargetPDB = "$TARGET_PDB";
  vars.ObjectDir = "$OBJECT_DIR";

  cmMakefile* mf = this->GetMakefile();

  bool useClDeps = false;
  std::string clBinary;
  std::string clDepsBinary;
  std::string clShowPrefix;
  if (lang == "C" || lang == "CXX" || lang == "RC")
    {
    clDepsBinary = mf->GetSafeDefinition("CMAKE_CMCLDEPS_EXECUTABLE");
    if (!clDepsBinary.empty() && !mf->GetIsSourceFileTryCompile())
      {
      clShowPrefix = mf->GetSafeDefinition("CMAKE_CL_SHOWINCLUDE_PREFIX");
      clBinary = mf->GetDefinition("CMAKE_C_COMPILER") ?
                 mf->GetSafeDefinition("CMAKE_C_COMPILER") :
                 mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
      if (!clBinary.empty() && !clShowPrefix.empty())
        {
        useClDeps = true;
        const std::string quote = " \"";
        clBinary     = quote + clBinary     + "\" ";
        clDepsBinary = quote + clDepsBinary + "\" ";
        clShowPrefix = quote + clShowPrefix + "\" ";
        vars.DependencyFile = "$DEP_FILE";
        }
      }
    }


  std::string depfile;
  std::string depfileFlagsName = "CMAKE_DEPFILE_FLAGS_" + language;
  const char *depfileFlags = mf->GetDefinition(depfileFlagsName.c_str());
  if (depfileFlags || useClDeps) {
    std::string depFlagsStr = depfileFlags ? depfileFlags : "";
    depfile = "$DEP_FILE";
    cmSystemTools::ReplaceString(depFlagsStr, "<DEPFILE>", "\"$DEP_FILE\"");
    cmSystemTools::ReplaceString(depFlagsStr, "<OBJECT>",  "$out");
    cmSystemTools::ReplaceString(depFlagsStr, "<CMAKE_C_COMPILER>",
                       mf->GetDefinition("CMAKE_C_COMPILER"));
    flags += " " + depFlagsStr;
  }
  vars.Flags = flags.c_str();


  // Rule for compiling object file.
  std::string compileCmdVar = "CMAKE_";
  compileCmdVar += language;
  compileCmdVar += "_COMPILE_OBJECT";
  std::string compileCmd = mf->GetRequiredDefinition(compileCmdVar.c_str());
  std::vector<std::string> compileCmds;
  cmSystemTools::ExpandListArgument(compileCmd, compileCmds);

  if(useClDeps)
    {
    std::string cmdPrefix = clDepsBinary + lang + " $in \"$DEP_FILE\" $out " +
                            clShowPrefix + clBinary;
    compileCmds.front().insert(0, cmdPrefix);
    }

  for (std::vector<std::string>::iterator i = compileCmds.begin();
       i != compileCmds.end(); ++i)
    this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);

  std::string cmdLine =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds);


  // Write the rule for compiling file of the given language.
  cmOStringStream comment;
  comment << "Rule for compiling " << language << " files.";
  cmOStringStream description;
  description << "Building " << language << " object $out";
  this->GetGlobalGenerator()->AddRule(this->LanguageCompilerRule(language),
                                      cmdLine,
                                      description.str(),
                                      comment.str(),
                                      depfile);
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

  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->CustomCommands.begin();
      si != this->GeneratorTarget->CustomCommands.end(); ++si)
     {
     cmCustomCommand const* cc = (*si)->GetCustomCommand();
     this->GetLocalGenerator()->AddCustomCommandTarget(cc, this->GetTarget());
     }
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    this->GeneratorTarget->HeaderSources,
    this->MacOSXContentGenerator);
  this->OSXBundleGenerator->GenerateMacOSXContentStatements(
    this->GeneratorTarget->ExtraSources,
    this->MacOSXContentGenerator);
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->ExternalObjects.begin();
      si != this->GeneratorTarget->ExternalObjects.end(); ++si)
    {
    this->Objects.push_back(this->GetSourceFilePath(*si));
    }
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->ObjectSources.begin();
      si != this->GeneratorTarget->ObjectSources.end(); ++si)
    {
    this->WriteObjectBuildStatement(*si);
    }
  if(!this->GeneratorTarget->ModuleDefinitionFile.empty())
    {
    this->ModuleDefinitionFile = this->ConvertToNinjaPath(
      this->GeneratorTarget->ModuleDefinitionFile.c_str());
    }

  {
  // Add object library contents as external objects.
  std::vector<std::string> objs;
  this->GeneratorTarget->UseObjectLibraries(objs);
  for(std::vector<std::string>::iterator oi = objs.begin();
      oi != objs.end(); ++oi)
    {
    this->Objects.push_back(ConvertToNinjaPath(oi->c_str()));
    }
  }

  this->GetBuildFileStream() << "\n";
}

void
cmNinjaTargetGenerator
::WriteObjectBuildStatement(cmSourceFile* source)
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

  // Ensure that the target dependencies are built before any source file in
  // the target, using order-only dependencies.
  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->Target, orderOnlyDeps);

  cmNinjaDeps implicitDeps;
  if(const char* objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    std::vector<std::string> depList;
    cmSystemTools::ExpandListArgument(objectDeps, depList);
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(implicitDeps), MapToNinjaPath());
  }

  // Add order-only dependencies on custom command outputs.
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->CustomCommands.begin();
      si != this->GeneratorTarget->CustomCommands.end(); ++si)
    {
    cmCustomCommand const* cc = (*si)->GetCustomCommand();
    const std::vector<std::string>& ccoutputs = cc->GetOutputs();
    std::transform(ccoutputs.begin(), ccoutputs.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
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
  vars["DEP_FILE"] = objectFileName + ".d";;
  EnsureParentDirectoryExists(objectFileName);

  std::string objectDir = cmSystemTools::GetFilenamePath(objectFileName);
  objectDir = this->GetLocalGenerator()->Convert(objectDir.c_str(),
                            cmLocalGenerator::START_OUTPUT,
                            cmLocalGenerator::SHELL);
  vars["OBJECT_DIR"] = objectDir;


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
        escapedSourceFileName.c_str(),
        this->GetGlobalGenerator()->GetCMakeInstance()->
          GetHomeOutputDirectory());
      }

    escapedSourceFileName =
      this->LocalGenerator->ConvertToOutputFormat(
        escapedSourceFileName.c_str(), cmLocalGenerator::SHELL);

    compileObjectVars.Source = escapedSourceFileName.c_str();
    compileObjectVars.Object = objectFileName.c_str();
    compileObjectVars.ObjectDir = objectDir.c_str();
    compileObjectVars.Flags = vars["FLAGS"].c_str();
    compileObjectVars.Defines = vars["DEFINES"].c_str();

    // Rule for compiling object file.
    std::string compileCmdVar = "CMAKE_";
    compileCmdVar += language;
    compileCmdVar += "_COMPILE_OBJECT";
    std::string compileCmd =
      this->GetMakefile()->GetRequiredDefinition(compileCmdVar.c_str());
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

  cmGlobalNinjaGenerator::WriteBuild(this->GetBuildFileStream(),
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
    cmGlobalNinjaGenerator::WritePhonyBuild(this->GetBuildFileStream(),
                                            "Additional output files.",
                                            outputList,
                                            outputs);
  }
}

//----------------------------------------------------------------------------
void
cmNinjaTargetGenerator
::AddModuleDefinitionFlag(std::string& flags)
{
  if(this->ModuleDefinitionFile.empty())
    {
    return;
    }

  // TODO: Create a per-language flag variable.
  const char* defFileFlag =
    this->Makefile->GetDefinition("CMAKE_LINK_DEF_FILE_FLAG");
  if(!defFileFlag)
    {
    return;
    }

  // Append the flag and value.  Use ConvertToLinkReference to help
  // vs6's "cl -link" pass it to the linker.
  std::string flag = defFileFlag;
  flag += (this->LocalGenerator->ConvertToLinkReference(
             this->ModuleDefinitionFile.c_str()));
  this->LocalGenerator->AppendFlags(flags, flag.c_str());
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
  EnsureDirectoryExists(cmSystemTools::GetParentDirectory(path.c_str()));
}


//----------------------------------------------------------------------------
void
cmNinjaTargetGenerator::MacOSXContentGeneratorType::operator()(
  cmSourceFile& source, const char* pkgloc)
{
  // Skip OS X content when not building a Framework or Bundle.
  if(this->Generator->OSXBundleGenerator->GetMacContentDirectory().empty())
    {
    return;
    }

  std::string macdir =
    this->Generator->OSXBundleGenerator->InitMacOSXContentDirectory(pkgloc);

  // Get the input file location.
  std::string input = source.GetFullPath();
  input =
    this->Generator->GetLocalGenerator()->ConvertToNinjaPath(input.c_str());

  // Get the output file location.
  std::string output = macdir;
  output += "/";
  output += cmSystemTools::GetFilenameName(input);
  output =
    this->Generator->GetLocalGenerator()->ConvertToNinjaPath(output.c_str());

  // Write a build statement to copy the content into the bundle.
  this->Generator->GetGlobalGenerator()->WriteMacOSXContentBuild(input,
                                                                 output);

  // Add as a dependency of all target so that it gets called.
  this->Generator->GetGlobalGenerator()->AddDependencyToAll(output);
}
