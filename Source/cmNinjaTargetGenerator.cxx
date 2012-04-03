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
  : Target(target),
    Makefile(target->GetMakefile()),
    LocalGenerator(
      static_cast<cmLocalNinjaGenerator*>(Makefile->GetLocalGenerator())),
    Objects()
{
  this->GeneratorTarget =
    this->GetGlobalGenerator()->GetGeneratorTarget(target);
}

cmNinjaTargetGenerator::~cmNinjaTargetGenerator()
{
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
  return this->LocalGenerator->ConfigName.c_str();
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
                                                  this->GetTarget(),
                                                  language.c_str(),
                                                  this->GetConfigName());

  // TODO: Fortran support.
  // // Fortran-specific flags computed for this target.
  // if(*l == "Fortran")
  //   {
  //   this->AddFortranFlags(flags);
  //   }

  // Add shared-library flags if needed.
  {
  bool shared = ((this->Target->GetType() == cmTarget::SHARED_LIBRARY) ||
                 (this->Target->GetType() == cmTarget::MODULE_LIBRARY));
  this->GetLocalGenerator()->AddSharedFlags(flags, language.c_str(), shared);
  }

  // TODO: Handle response file.
  // Add include directory flags.
  {
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->Target,
                                              language.c_str());
  std::string includeFlags =
    this->LocalGenerator->GetIncludeFlags(includes, language.c_str(), false);
  this->LocalGenerator->AppendFlags(flags, includeFlags.c_str());
  }

  // Append old-style preprocessor definition flags.
  this->LocalGenerator->AppendFlags(flags, this->Makefile->GetDefineFlags());

  // Add target-specific and source-specific flags.
  this->LocalGenerator->AppendFlags(flags,
                                   this->Target->GetProperty("COMPILE_FLAGS"));
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
  std::string defines;

  // Add the export symbol definition for shared library objects.
  if(const char* exportMacro = this->Target->GetExportMacro())
    {
    this->LocalGenerator->AppendDefines(defines, exportMacro,
                                        language.c_str());
    }

  // Add preprocessor definitions for this target and configuration.
  this->LocalGenerator->AppendDefines
    (defines,
     this->Makefile->GetProperty("COMPILE_DEFINITIONS"),
     language.c_str());
  this->LocalGenerator->AppendDefines
    (defines,
     this->Target->GetProperty("COMPILE_DEFINITIONS"),
     language.c_str());
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty("COMPILE_DEFINITIONS"),
     language.c_str());
  {
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(this->GetConfigName());
  this->LocalGenerator->AppendDefines
    (defines,
     this->Makefile->GetProperty(defPropName.c_str()),
     language.c_str());
  this->LocalGenerator->AppendDefines
    (defines,
     this->Target->GetProperty(defPropName.c_str()),
     language.c_str());
  this->LocalGenerator->AppendDefines
    (defines,
     source->GetProperty(defPropName.c_str()),
     language.c_str());
  }

  return defines;
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

std::string cmNinjaTargetGenerator::GetTargetPDB() const
{
  std::string targetFullPathPDB;
  if(this->Target->GetType() == cmTarget::EXECUTABLE ||
     this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
     this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
     this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    targetFullPathPDB = this->Target->GetDirectory(this->GetConfigName());
    targetFullPathPDB += "/";
    targetFullPathPDB += this->Target->GetPDBName(this->GetConfigName());
    }

  return ConvertToNinjaPath(targetFullPathPDB.c_str());
}


void
cmNinjaTargetGenerator
::WriteLanguageRules(const std::string& language)
{
  this->GetRulesFileStream()
    << "# Rules for language " << language << "\n\n";
  this->WriteCompileRule(language);
  this->GetRulesFileStream() << "\n";
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

  std::string depfile;
  std::string depfileFlagsName = "CMAKE_DEPFILE_FLAGS_" + language;
  const char *depfileFlags =
    this->GetMakefile()->GetDefinition(depfileFlagsName.c_str());
  if (depfileFlags) {
    std::string depfileFlagsStr = depfileFlags;
    depfile = "$out.d";
    cmSystemTools::ReplaceString(depfileFlagsStr, "<DEPFILE>",
                                 depfile.c_str());
    cmSystemTools::ReplaceString(depfileFlagsStr, "<OBJECT>",
                                 "$out");
    cmSystemTools::ReplaceString(depfileFlagsStr, "<CMAKE_C_COMPILER>",
                       this->GetMakefile()->GetDefinition("CMAKE_C_COMPILER"));
    flags += " " + depfileFlagsStr;
  }
  vars.Flags = flags.c_str();


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
    this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);

  std::string cmdLine =
    this->GetLocalGenerator()->BuildCommandLine(compileCmds);

  // Write the rule for compiling file of the given language.
  std::ostringstream comment;
  comment << "Rule for compiling " << language << " files.";
  std::ostringstream description;
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
  // TODO: this->GeneratorTarget->OSXContent
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
  cmNinjaDeps emptyDeps;

  std::string comment;
  const char* language = source->GetLanguage();
  std::string rule = this->LanguageCompilerRule(language);

  cmNinjaDeps outputs;
  std::string objectFileName = this->GetObjectFilePath(source);
  outputs.push_back(objectFileName);
  // Add this object to the list of object files.
  this->Objects.push_back(objectFileName);

  cmNinjaDeps explicitDeps;
  std::string sourceFileName = this->GetSourceFilePath(source);
  explicitDeps.push_back(sourceFileName);

  // Ensure that the target dependencies are built before any source file in
  // the target, using order-only dependencies.
  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->Target, orderOnlyDeps);

  if(const char* objectDeps = source->GetProperty("OBJECT_DEPENDS")) {
    std::vector<std::string> depList;
    cmSystemTools::ExpandListArgument(objectDeps, depList);
    std::transform(depList.begin(), depList.end(),
                   std::back_inserter(orderOnlyDeps), MapToNinjaPath());
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
  vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->GetTargetPDB().c_str(), cmLocalGenerator::SHELL);

  cmGlobalNinjaGenerator::WriteBuild(this->GetBuildFileStream(),
                                     comment,
                                     rule,
                                     outputs,
                                     explicitDeps,
                                     emptyDeps,
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
