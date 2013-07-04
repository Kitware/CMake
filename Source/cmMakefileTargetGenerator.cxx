/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakefileTargetGenerator.h"

#include "cmGeneratorTarget.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"
#include "cmake.h"
#include "cmComputeLinkInformation.h"

#include "cmMakefileExecutableTargetGenerator.h"
#include "cmMakefileLibraryTargetGenerator.h"
#include "cmMakefileUtilityTargetGenerator.h"


cmMakefileTargetGenerator::cmMakefileTargetGenerator(cmTarget* target)
  : OSXBundleGenerator(0)
  , MacOSXContentGenerator(0)
{
  this->BuildFileStream = 0;
  this->InfoFileStream = 0;
  this->FlagFileStream = 0;
  this->CustomCommandDriver = OnBuild;
  this->FortranModuleDirectoryComputed = false;
  this->Target = target;
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator =
    static_cast<cmLocalUnixMakefileGenerator3*>(
      this->Makefile->GetLocalGenerator());
  this->ConfigName = this->LocalGenerator->ConfigurationName.c_str();
  this->GlobalGenerator =
    static_cast<cmGlobalUnixMakefileGenerator3*>(
      this->LocalGenerator->GetGlobalGenerator());
  this->GeneratorTarget = this->GlobalGenerator->GetGeneratorTarget(target);
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  this->NoRuleMessages = false;
  if(const char* ruleStatus = cm->GetProperty("RULE_MESSAGES"))
    {
    this->NoRuleMessages = cmSystemTools::IsOff(ruleStatus);
    }
  MacOSXContentGenerator = new MacOSXContentGeneratorType(this);
}

cmMakefileTargetGenerator::~cmMakefileTargetGenerator()
{
  delete MacOSXContentGenerator;
}

cmMakefileTargetGenerator *
cmMakefileTargetGenerator::New(cmTarget *tgt)
{
  cmMakefileTargetGenerator *result = 0;

  switch (tgt->GetType())
    {
    case cmTarget::EXECUTABLE:
      result = new cmMakefileExecutableTargetGenerator(tgt);
      break;
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::OBJECT_LIBRARY:
      result = new cmMakefileLibraryTargetGenerator(tgt);
      break;
    case cmTarget::UTILITY:
      result = new cmMakefileUtilityTargetGenerator(tgt);
      break;
    default:
      return result;
      // break; /* unreachable */
    }
  return result;
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::CreateRuleFile()
{
  // Create a directory for this target.
  this->TargetBuildDirectory =
    this->LocalGenerator->GetTargetDirectory(*this->Target);
  this->TargetBuildDirectoryFull =
    this->LocalGenerator->ConvertToFullPath(this->TargetBuildDirectory);
  cmSystemTools::MakeDirectory(this->TargetBuildDirectoryFull.c_str());

  // Construct the rule file name.
  this->BuildFileName = this->TargetBuildDirectory;
  this->BuildFileName += "/build.make";
  this->BuildFileNameFull = this->TargetBuildDirectoryFull;
  this->BuildFileNameFull += "/build.make";

  // Construct the rule file name.
  this->ProgressFileNameFull = this->TargetBuildDirectoryFull;
  this->ProgressFileNameFull += "/progress.make";

  // reset the progress count
  this->NumberOfProgressActions = 0;

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->BuildFileStream =
    new cmGeneratedFileStream(this->BuildFileNameFull.c_str());
  this->BuildFileStream->SetCopyIfDifferent(true);
  if(!this->BuildFileStream)
    {
    return;
    }
  this->LocalGenerator->WriteDisclaimer(*this->BuildFileStream);
  this->LocalGenerator->WriteSpecialTargetsTop(*this->BuildFileStream);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetBuildRules()
{
  // write the custom commands for this target
  // Look for files registered for cleaning in this directory.
  if(const char* additional_clean_files =
     this->Makefile->GetProperty
     ("ADDITIONAL_MAKE_CLEAN_FILES"))
    {
    cmSystemTools::ExpandListArgument(additional_clean_files,
                                      this->CleanFiles);
    }

  // add custom commands to the clean rules?
  const char* clean_no_custom =
    this->Makefile->GetProperty("CLEAN_NO_CUSTOM");
  bool clean = cmSystemTools::IsOff(clean_no_custom);

  // First generate the object rule files.  Save a list of all object
  // files for this target.
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->CustomCommands.begin();
      si != this->GeneratorTarget->CustomCommands.end(); ++si)
    {
    cmCustomCommand const* cc = (*si)->GetCustomCommand();
    this->GenerateCustomRuleFile(*cc);
    if (clean)
      {
      const std::vector<std::string>& outputs = cc->GetOutputs();
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        this->CleanFiles.push_back
          (this->Convert(o->c_str(),
                         cmLocalGenerator::START_OUTPUT,
                         cmLocalGenerator::UNCHANGED));
        }
      }
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
    this->ExternalObjects.push_back((*si)->GetFullPath());
    }
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->GeneratorTarget->ObjectSources.begin();
      si != this->GeneratorTarget->ObjectSources.end(); ++si)
    {
    // Generate this object file's rule file.
    this->WriteObjectRuleFiles(**si);
    }

  // Add object library contents as external objects.
  this->GeneratorTarget->UseObjectLibraries(this->ExternalObjects);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteCommonCodeRules()
{
  const char* root = (this->Makefile->IsOn("CMAKE_MAKE_INCLUDE_FROM_ROOT")?
                      "$(CMAKE_BINARY_DIR)/" : "");

  // Include the dependencies for the target.
  std::string dependFileNameFull = this->TargetBuildDirectoryFull;
  dependFileNameFull += "/depend.make";
  *this->BuildFileStream
    << "# Include any dependencies generated for this target.\n"
    << this->LocalGenerator->IncludeDirective << " " << root
    << this->Convert(dependFileNameFull.c_str(),
                     cmLocalGenerator::HOME_OUTPUT,
                     cmLocalGenerator::MAKEFILE)
    << "\n\n";

  if(!this->NoRuleMessages)
    {
    // Include the progress variables for the target.
    *this->BuildFileStream
      << "# Include the progress variables for this target.\n"
      << this->LocalGenerator->IncludeDirective << " " << root
      << this->Convert(this->ProgressFileNameFull.c_str(),
                       cmLocalGenerator::HOME_OUTPUT,
                       cmLocalGenerator::MAKEFILE)
      << "\n\n";
    }

  // make sure the depend file exists
  if (!cmSystemTools::FileExists(dependFileNameFull.c_str()))
    {
    // Write an empty dependency file.
    cmGeneratedFileStream depFileStream(dependFileNameFull.c_str());
    depFileStream
      << "# Empty dependencies file for " << this->Target->GetName() << ".\n"
      << "# This may be replaced when dependencies are built." << std::endl;
    }

  // Open the flags file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->FlagFileNameFull = this->TargetBuildDirectoryFull;
  this->FlagFileNameFull += "/flags.make";
  this->FlagFileStream =
    new cmGeneratedFileStream(this->FlagFileNameFull.c_str());
  this->FlagFileStream->SetCopyIfDifferent(true);
  if(!this->FlagFileStream)
    {
    return;
    }
  this->LocalGenerator->WriteDisclaimer(*this->FlagFileStream);

  // Include the flags for the target.
  *this->BuildFileStream
    << "# Include the compile flags for this target's objects.\n"
    << this->LocalGenerator->IncludeDirective << " " << root
    << this->Convert(this->FlagFileNameFull.c_str(),
                                     cmLocalGenerator::HOME_OUTPUT,
                                     cmLocalGenerator::MAKEFILE)
    << "\n\n";
}

//----------------------------------------------------------------------------
std::string cmMakefileTargetGenerator::GetFlags(const std::string &l)
{
  ByLanguageMap::iterator i = this->FlagsByLanguage.find(l);
  if (i == this->FlagsByLanguage.end())
    {
    std::string flags;
    const char *lang = l.c_str();

    // Add language feature flags.
    this->AddFeatureFlags(flags, lang);

    this->LocalGenerator->AddArchitectureFlags(flags, this->GeneratorTarget,
                                               lang, this->ConfigName);

    // Fortran-specific flags computed for this target.
    if(l == "Fortran")
      {
      this->AddFortranFlags(flags);
      }

    this->LocalGenerator->AddCMP0018Flags(flags, this->Target,
                                          lang, this->ConfigName);

    this->LocalGenerator->AddVisibilityPresetFlags(flags, this->Target,
                                                   lang);

    // Add include directory flags.
    this->AddIncludeFlags(flags, lang);

    // Append old-style preprocessor definition flags.
    this->LocalGenerator->
      AppendFlags(flags, this->Makefile->GetDefineFlags());

    // Add include directory flags.
    this->LocalGenerator->
      AppendFlags(flags,this->GetFrameworkFlags().c_str());

    // Add target-specific flags.
    this->LocalGenerator->AddCompileOptions(flags, this->Target,
                                            lang, this->ConfigName);

    ByLanguageMap::value_type entry(l, flags);
    i = this->FlagsByLanguage.insert(entry).first;
    }
  return i->second;
}

std::string cmMakefileTargetGenerator::GetDefines(const std::string &l)
{
  ByLanguageMap::iterator i = this->DefinesByLanguage.find(l);
  if (i == this->DefinesByLanguage.end())
    {
    std::set<std::string> defines;
    const char *lang = l.c_str();
    // Add the export symbol definition for shared library objects.
    if(const char* exportMacro = this->Target->GetExportMacro())
      {
      this->LocalGenerator->AppendDefines(defines, exportMacro);
      }

    // Add preprocessor definitions for this target and configuration.
    this->LocalGenerator->AppendDefines
      (defines, this->Target->GetCompileDefinitions(
                            this->LocalGenerator->ConfigurationName.c_str()));

    std::string definesString;
    this->LocalGenerator->JoinDefines(defines, definesString, lang);

    ByLanguageMap::value_type entry(l, definesString);
    i = this->DefinesByLanguage.insert(entry).first;
    }
  return i->second;
}

void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
{
  // write language flags for target
  std::set<cmStdString> languages;
  this->Target->GetLanguages(languages);
    // put the compiler in the rules.make file so that if it changes
  // things rebuild
  for(std::set<cmStdString>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    cmStdString compiler = "CMAKE_";
    compiler += *l;
    compiler += "_COMPILER";
    *this->FlagFileStream << "# compile " << l->c_str() << " with " <<
      this->Makefile->GetSafeDefinition(compiler.c_str()) << "\n";
    }

  for(std::set<cmStdString>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    *this->FlagFileStream << *l << "_FLAGS = " << this->GetFlags(*l) << "\n\n";
    *this->FlagFileStream << *l << "_DEFINES = " << this->GetDefines(*l) <<
      "\n\n";
    }
}


//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator::MacOSXContentGeneratorType::operator()
  (cmSourceFile& source, const char* pkgloc)
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

  // Get the output file location.
  std::string output = macdir;
  output += "/";
  output += cmSystemTools::GetFilenameName(input);
  this->Generator->CleanFiles.push_back(
    this->Generator->Convert(output.c_str(),
                             cmLocalGenerator::START_OUTPUT));
  output = this->Generator->Convert(output.c_str(),
                                    cmLocalGenerator::HOME_OUTPUT);

  // Create a rule to copy the content into the bundle.
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  depends.push_back(input);
  std::string copyEcho = "Copying OS X content ";
  copyEcho += output;
  this->Generator->LocalGenerator->AppendEcho(
    commands, copyEcho.c_str(),
    cmLocalUnixMakefileGenerator3::EchoBuild);
  std::string copyCommand = "$(CMAKE_COMMAND) -E copy ";
  copyCommand += this->Generator->Convert(input.c_str(),
                                          cmLocalGenerator::NONE,
                                          cmLocalGenerator::SHELL);
  copyCommand += " ";
  copyCommand += this->Generator->Convert(output.c_str(),
                                          cmLocalGenerator::NONE,
                                          cmLocalGenerator::SHELL);
  commands.push_back(copyCommand);
  this->Generator->LocalGenerator->WriteMakeRule(
    *this->Generator->BuildFileStream, 0,
    output.c_str(),
    depends, commands, false);
  this->Generator->ExtraFiles.insert(output);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteObjectRuleFiles(cmSourceFile& source)
{
  // Identify the language of the source file.
  const char* lang = this->LocalGenerator->GetSourceFileLanguage(source);
  if(!lang)
    {
    // don't know anything about this file so skip it
    return;
    }

  // Get the full path name of the object file.
  std::string const& objectName = this->GeneratorTarget->Objects[&source];
  std::string obj = this->LocalGenerator->GetTargetDirectory(*this->Target);
  obj += "/";
  obj += objectName;

  // Avoid generating duplicate rules.
  if(this->ObjectFiles.find(obj) == this->ObjectFiles.end())
    {
    this->ObjectFiles.insert(obj);
    }
  else
    {
    cmOStringStream err;
    err << "Warning: Source file \""
        << source.GetFullPath()
        << "\" is listed multiple times for target \""
        << this->Target->GetName()
        << "\".";
    cmSystemTools::Message(err.str().c_str(), "Warning");
    return;
    }

  // Create the directory containing the object file.  This may be a
  // subdirectory under the target's directory.
  std::string dir = cmSystemTools::GetFilenamePath(obj.c_str());
  cmSystemTools::MakeDirectory
    (this->LocalGenerator->ConvertToFullPath(dir).c_str());

  // Save this in the target's list of object files.
  this->Objects.push_back(obj);
  this->CleanFiles.push_back(obj);

  // TODO: Remove
  //std::string relativeObj
  //= this->LocalGenerator->GetHomeRelativeOutputPath();
  //relativeObj += obj;

  // we compute some depends when writing the depend.make that we will also
  // use in the build.make, same with depMakeFile
  std::vector<std::string> depends;
  std::string depMakeFile;

  // generate the build rule file
  this->WriteObjectBuildFile(obj, lang, source, depends);

  // The object file should be checked for dependency integrity.
  std::string objFullPath = this->Makefile->GetCurrentOutputDirectory();
  objFullPath += "/";
  objFullPath += obj;
  objFullPath =
    this->Convert(objFullPath.c_str(), cmLocalGenerator::FULL);
  std::string srcFullPath =
    this->Convert(source.GetFullPath().c_str(), cmLocalGenerator::FULL);
  this->LocalGenerator->
    AddImplicitDepends(*this->Target, lang,
                       objFullPath.c_str(),
                       srcFullPath.c_str());
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::AppendFortranFormatFlags(std::string& flags, cmSourceFile& source)
{
  const char* srcfmt = source.GetProperty("Fortran_FORMAT");
  cmLocalGenerator::FortranFormat format =
    this->LocalGenerator->GetFortranFormat(srcfmt);
  if(format == cmLocalGenerator::FortranFormatNone)
    {
    const char* tgtfmt = this->Target->GetProperty("Fortran_FORMAT");
    format = this->LocalGenerator->GetFortranFormat(tgtfmt);
    }
  const char* var = 0;
  switch (format)
    {
    case cmLocalGenerator::FortranFormatFixed:
      var = "CMAKE_Fortran_FORMAT_FIXED_FLAG"; break;
    case cmLocalGenerator::FortranFormatFree:
      var = "CMAKE_Fortran_FORMAT_FREE_FLAG"; break;
    default: break;
    }
  if(var)
    {
    this->LocalGenerator->AppendFlags(
      flags, this->Makefile->GetDefinition(var));
    }
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectBuildFile(std::string &obj,
                       const char *lang,
                       cmSourceFile& source,
                       std::vector<std::string>& depends)
{
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->FlagFileNameFull.c_str());
  this->LocalGenerator->AppendRuleDepends(depends,
                                          this->FlagFileDepends[lang]);

  // generate the depend scanning rule
  this->WriteObjectDependRules(source, depends);

  std::string relativeObj = this->LocalGenerator->GetHomeRelativeOutputPath();
  relativeObj += obj;
  // Write the build rule.

  // Build the set of compiler flags.
  std::string flags;

  // Add language-specific flags.
  std::string langFlags = "$(";
  langFlags += lang;
  langFlags += "_FLAGS)";
  this->LocalGenerator->AppendFlags(flags, langFlags.c_str());

  std::string configUpper =
    cmSystemTools::UpperCase(this->LocalGenerator->ConfigurationName);

  // Add Fortran format flags.
  if(strcmp(lang, "Fortran") == 0)
    {
    this->AppendFortranFormatFlags(flags, source);
    }

  // Add flags from source file properties.
  if (source.GetProperty("COMPILE_FLAGS"))
    {
    this->LocalGenerator->AppendFlags
      (flags, source.GetProperty("COMPILE_FLAGS"));
    *this->FlagFileStream << "# Custom flags: "
                          << relativeObj << "_FLAGS = "
                          << source.GetProperty("COMPILE_FLAGS")
                          << "\n"
                          << "\n";
    }

  // Add language-specific defines.
  std::set<std::string> defines;

  // Add source-sepcific preprocessor definitions.
  if(const char* compile_defs = source.GetProperty("COMPILE_DEFINITIONS"))
    {
    this->LocalGenerator->AppendDefines(defines, compile_defs);
    *this->FlagFileStream << "# Custom defines: "
                          << relativeObj << "_DEFINES = "
                          << compile_defs << "\n"
                          << "\n";
    }
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += configUpper;
  if(const char* config_compile_defs =
     source.GetProperty(defPropName.c_str()))
    {
    this->LocalGenerator->AppendDefines(defines, config_compile_defs);
    *this->FlagFileStream
      << "# Custom defines: "
      << relativeObj << "_DEFINES_" << configUpper
      << " = " << config_compile_defs << "\n"
      << "\n";
    }

  // Get the output paths for source and object files.
  std::string sourceFile = source.GetFullPath();
  if(this->LocalGenerator->UseRelativePaths)
    {
    sourceFile = this->Convert(sourceFile.c_str(),
                               cmLocalGenerator::START_OUTPUT);
    }
  sourceFile = this->Convert(sourceFile.c_str(),
                             cmLocalGenerator::NONE,
                             cmLocalGenerator::SHELL);

  // Construct the build message.
  std::vector<std::string> no_commands;
  std::vector<std::string> commands;

  // add in a progress call if needed
  this->AppendProgress(commands);

  if(!this->NoRuleMessages)
    {
    std::string buildEcho = "Building ";
    buildEcho += lang;
    buildEcho += " object ";
    buildEcho += relativeObj;
    this->LocalGenerator->AppendEcho
      (commands, buildEcho.c_str(), cmLocalUnixMakefileGenerator3::EchoBuild);
    }

  std::string targetOutPathReal;
  std::string targetOutPathPDB;
  {
  std::string targetFullPathReal;
  std::string targetFullPathPDB;
  if(this->Target->GetType() == cmTarget::EXECUTABLE ||
     this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
     this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
     this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    targetFullPathReal =
      this->Target->GetFullPath(this->ConfigName, false, true);
    targetFullPathPDB = this->Target->GetPDBDirectory(this->ConfigName);
    targetFullPathPDB += "/";
    targetFullPathPDB += this->Target->GetPDBName(this->ConfigName);
    }
  targetOutPathReal = this->Convert(targetFullPathReal.c_str(),
                                    cmLocalGenerator::START_OUTPUT,
                                    cmLocalGenerator::SHELL);
  targetOutPathPDB =
    this->Convert(targetFullPathPDB.c_str(),cmLocalGenerator::NONE,
                  cmLocalGenerator::SHELL);
  }
  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_COMPILE";
  vars.CMTarget = this->Target;
  vars.Language = lang;
  vars.Target = targetOutPathReal.c_str();
  vars.TargetPDB = targetOutPathPDB.c_str();
  vars.Source = sourceFile.c_str();
  std::string shellObj =
    this->Convert(obj.c_str(),
                  cmLocalGenerator::NONE,
                  cmLocalGenerator::SHELL).c_str();
  vars.Object = shellObj.c_str();
  std::string objectDir = cmSystemTools::GetFilenamePath(obj);
  objectDir = this->Convert(objectDir.c_str(),
                            cmLocalGenerator::START_OUTPUT,
                            cmLocalGenerator::SHELL);
  vars.ObjectDir = objectDir.c_str();

  if (const char *rootPath =
                    this->Makefile->GetSafeDefinition("CMAKE_SYSROOT"))
    {
    if (*rootPath)
      {
      std::string sysrootDef = "CMAKE_";
      sysrootDef += lang;
      sysrootDef += "_COMPILE_OPTIONS_SYSROOT";
      if (const char *sysrootFlag =
                      this->Makefile->GetDefinition(sysrootDef.c_str()))
        {
        flags += " ";
        flags += sysrootFlag;
        flags += this->LocalGenerator->EscapeForShell(rootPath);
        flags += " ";
        }
      }
    }

  vars.Flags = flags.c_str();

  std::string definesString = "$(";
  definesString += lang;
  definesString += "_DEFINES)";

  this->LocalGenerator->JoinDefines(defines, definesString, lang);

  vars.Defines = definesString.c_str();

  bool lang_is_c_or_cxx = ((strcmp(lang, "C") == 0) ||
                           (strcmp(lang, "CXX") == 0));

  // Construct the compile rules.
  {
  std::string compileRuleVar = "CMAKE_";
  compileRuleVar += lang;
  compileRuleVar += "_COMPILE_OBJECT";
  std::string compileRule =
    this->Makefile->GetRequiredDefinition(compileRuleVar.c_str());
  std::vector<std::string> compileCommands;
  cmSystemTools::ExpandListArgument(compileRule, compileCommands);

  if (this->Makefile->IsOn("CMAKE_EXPORT_COMPILE_COMMANDS") &&
      lang_is_c_or_cxx && compileCommands.size() == 1)
    {
    std::string compileCommand = compileCommands[0];
    this->LocalGenerator->ExpandRuleVariables(compileCommand, vars);
    std::string workingDirectory =
      this->LocalGenerator->Convert(
        this->Makefile->GetStartOutputDirectory(), cmLocalGenerator::FULL);
    compileCommand.replace(compileCommand.find(langFlags),
                           langFlags.size(), this->GetFlags(lang));
    std::string langDefines = std::string("$(") + lang + "_DEFINES)";
    compileCommand.replace(compileCommand.find(langDefines),
                           langDefines.size(), this->GetDefines(lang));
    this->GlobalGenerator->AddCXXCompileCommand(
      source.GetFullPath(), workingDirectory, compileCommand);
    }

  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = compileCommands.begin();
      i != compileCommands.end(); ++i)
    {
    this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }

  // Change the command working directory to the local build tree.
  this->LocalGenerator->CreateCDCommand
    (compileCommands,
     this->Makefile->GetStartOutputDirectory(),
     cmLocalGenerator::HOME_OUTPUT);
  commands.insert(commands.end(),
                  compileCommands.begin(), compileCommands.end());
  }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      relativeObj.c_str(),
                                      depends, commands, false);

  // Check for extra outputs created by the compilation.
  if(const char* extra_outputs_str =
     source.GetProperty("OBJECT_OUTPUTS"))
    {
    std::vector<std::string> extra_outputs;
    cmSystemTools::ExpandListArgument(extra_outputs_str, extra_outputs);
    for(std::vector<std::string>::const_iterator eoi = extra_outputs.begin();
        eoi != extra_outputs.end(); ++eoi)
      {
      // Register this as an extra output for the object file rule.
      // This will cause the object file to be rebuilt if the extra
      // output is missing.
      this->GenerateExtraOutput(eoi->c_str(), relativeObj.c_str(), false);

      // Register this as an extra file to clean.
      this->CleanFiles.push_back(eoi->c_str());
      }
    }

  bool do_preprocess_rules = lang_is_c_or_cxx &&
    this->LocalGenerator->GetCreatePreprocessedSourceRules();
  bool do_assembly_rules = lang_is_c_or_cxx &&
    this->LocalGenerator->GetCreateAssemblySourceRules();
  if(do_preprocess_rules || do_assembly_rules)
    {
    std::vector<std::string> force_depends;
    force_depends.push_back("cmake_force");
    std::string::size_type dot_pos = relativeObj.rfind(".");
    std::string relativeObjBase = relativeObj.substr(0, dot_pos);
    dot_pos = obj.rfind(".");
    std::string objBase = obj.substr(0, dot_pos);

    if(do_preprocess_rules)
      {
      commands.clear();
      std::string relativeObjI = relativeObjBase + ".i";
      std::string objI = objBase + ".i";

      std::string preprocessEcho = "Preprocessing ";
      preprocessEcho += lang;
      preprocessEcho += " source to ";
      preprocessEcho += objI;
      this->LocalGenerator->AppendEcho(
        commands, preprocessEcho.c_str(),
        cmLocalUnixMakefileGenerator3::EchoBuild
        );

      std::string preprocessRuleVar = "CMAKE_";
      preprocessRuleVar += lang;
      preprocessRuleVar += "_CREATE_PREPROCESSED_SOURCE";
      if(const char* preprocessRule =
         this->Makefile->GetDefinition(preprocessRuleVar.c_str()))
        {
        std::vector<std::string> preprocessCommands;
        cmSystemTools::ExpandListArgument(preprocessRule, preprocessCommands);

        std::string shellObjI =
          this->Convert(objI.c_str(),
                        cmLocalGenerator::NONE,
                        cmLocalGenerator::SHELL).c_str();
        vars.PreprocessedSource = shellObjI.c_str();

        // Expand placeholders in the commands.
        for(std::vector<std::string>::iterator i = preprocessCommands.begin();
            i != preprocessCommands.end(); ++i)
          {
          this->LocalGenerator->ExpandRuleVariables(*i, vars);
          }

        this->LocalGenerator->CreateCDCommand
          (preprocessCommands,
           this->Makefile->GetStartOutputDirectory(),
           cmLocalGenerator::HOME_OUTPUT);
        commands.insert(commands.end(),
                        preprocessCommands.begin(),
                        preprocessCommands.end());
        }
      else
        {
        std::string cmd = "$(CMAKE_COMMAND) -E cmake_unimplemented_variable ";
        cmd += preprocessRuleVar;
        commands.push_back(cmd);
        }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                          relativeObjI.c_str(),
                                          force_depends, commands, false);
      }

    if(do_assembly_rules)
      {
      commands.clear();
      std::string relativeObjS = relativeObjBase + ".s";
      std::string objS = objBase + ".s";

      std::string assemblyEcho = "Compiling ";
      assemblyEcho += lang;
      assemblyEcho += " source to assembly ";
      assemblyEcho += objS;
      this->LocalGenerator->AppendEcho(
        commands, assemblyEcho.c_str(),
        cmLocalUnixMakefileGenerator3::EchoBuild
        );

      std::string assemblyRuleVar = "CMAKE_";
      assemblyRuleVar += lang;
      assemblyRuleVar += "_CREATE_ASSEMBLY_SOURCE";
      if(const char* assemblyRule =
         this->Makefile->GetDefinition(assemblyRuleVar.c_str()))
        {
        std::vector<std::string> assemblyCommands;
        cmSystemTools::ExpandListArgument(assemblyRule, assemblyCommands);

        std::string shellObjS =
          this->Convert(objS.c_str(),
                        cmLocalGenerator::NONE,
                        cmLocalGenerator::SHELL).c_str();
        vars.AssemblySource = shellObjS.c_str();

        // Expand placeholders in the commands.
        for(std::vector<std::string>::iterator i = assemblyCommands.begin();
            i != assemblyCommands.end(); ++i)
          {
          this->LocalGenerator->ExpandRuleVariables(*i, vars);
          }

        this->LocalGenerator->CreateCDCommand
          (assemblyCommands,
           this->Makefile->GetStartOutputDirectory(),
           cmLocalGenerator::HOME_OUTPUT);
        commands.insert(commands.end(),
                        assemblyCommands.begin(),
                        assemblyCommands.end());
        }
      else
        {
        std::string cmd = "$(CMAKE_COMMAND) -E cmake_unimplemented_variable ";
        cmd += assemblyRuleVar;
        commands.push_back(cmd);
        }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                          relativeObjS.c_str(),
                                          force_depends, commands, false);
      }
    }

  // If the language needs provides-requires mode, create the
  // corresponding targets.
  std::string objectRequires = relativeObj;
  objectRequires += ".requires";
  std::vector<std::string> p_depends;
  // always provide an empty requires target
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      objectRequires.c_str(), p_depends,
                                      no_commands, true);

  // write a build rule to recursively build what this obj provides
  std::string objectProvides = relativeObj;
  objectProvides += ".provides";
  std::string temp = relativeObj;
  temp += ".provides.build";
  std::vector<std::string> r_commands;
  std::string tgtMakefileName =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  tgtMakefileName += "/build.make";
  r_commands.push_back
    (this->LocalGenerator->GetRecursiveMakeCall(tgtMakefileName.c_str(),
                                                temp.c_str()));

  p_depends.clear();
  p_depends.push_back(objectRequires);
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      objectProvides.c_str(), p_depends,
                                      r_commands, true);

  // write the provides.build rule dependency on the obj file
  p_depends.clear();
  p_depends.push_back(relativeObj);
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      temp.c_str(), p_depends, no_commands,
                                      false);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetRequiresRules()
{
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;

  // Construct the name of the dependency generation target.
  std::string depTarget =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  depTarget += "/requires";

  // This target drives dependency generation for all object files.
  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget;
  for(std::vector<std::string>::const_iterator obj = this->Objects.begin();
      obj != this->Objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    objTarget += ".requires";
    depends.push_back(objTarget);
    }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depTarget.c_str(),
                                      depends, no_commands, true);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetCleanRules()
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the clean target name.
  std::string cleanTarget =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  cleanTarget += "/clean";

  // Construct the clean command.
  this->LocalGenerator->AppendCleanCommand(commands, this->CleanFiles,
                                           *this->Target);
  this->LocalGenerator->CreateCDCommand
    (commands,
     this->Makefile->GetStartOutputDirectory(),
     cmLocalGenerator::HOME_OUTPUT);

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      cleanTarget.c_str(),
                                      depends, commands, true);
}


//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetDependRules()
{
  // must write the targets depend info file
  std::string dir = this->LocalGenerator->GetTargetDirectory(*this->Target);
  this->InfoFileNameFull = dir;
  this->InfoFileNameFull += "/DependInfo.cmake";
  this->InfoFileNameFull =
    this->LocalGenerator->ConvertToFullPath(this->InfoFileNameFull);
  this->InfoFileStream =
    new cmGeneratedFileStream(this->InfoFileNameFull.c_str());
  this->InfoFileStream->SetCopyIfDifferent(true);
  if(!*this->InfoFileStream)
    {
    return;
    }
  this->LocalGenerator->
    WriteDependLanguageInfo(*this->InfoFileStream,*this->Target);

  // Store multiple output pairs in the depend info file.
  if(!this->MultipleOutputPairs.empty())
    {
    *this->InfoFileStream
      << "\n"
      << "# Pairs of files generated by the same build rule.\n"
      << "SET(CMAKE_MULTIPLE_OUTPUT_PAIRS\n";
    for(MultipleOutputPairsType::const_iterator pi =
          this->MultipleOutputPairs.begin();
        pi != this->MultipleOutputPairs.end(); ++pi)
      {
      *this->InfoFileStream
        << "  " << this->LocalGenerator->EscapeForCMake(pi->first.c_str())
        << " "  << this->LocalGenerator->EscapeForCMake(pi->second.c_str())
        << "\n";
      }
    *this->InfoFileStream << "  )\n\n";
    }

  // Store list of targets linked directly or transitively.
  {
  *this->InfoFileStream
    << "\n"
    << "# Targets to which this target links.\n"
    << "SET(CMAKE_TARGET_LINKED_INFO_FILES\n";
  std::set<cmTarget const*> emitted;
  const char* cfg = this->LocalGenerator->ConfigurationName.c_str();
  if(cmComputeLinkInformation* cli = this->Target->GetLinkInformation(cfg))
    {
    cmComputeLinkInformation::ItemVector const& items = cli->GetItems();
    for(cmComputeLinkInformation::ItemVector::const_iterator
          i = items.begin(); i != items.end(); ++i)
      {
      cmTarget const* linkee = i->Target;
      if(linkee && !linkee->IsImported() && emitted.insert(linkee).second)
        {
        cmMakefile* mf = linkee->GetMakefile();
        cmLocalGenerator* lg = mf->GetLocalGenerator();
        std::string di = mf->GetStartOutputDirectory();
        di += "/";
        di += lg->GetTargetDirectory(*linkee);
        di += "/DependInfo.cmake";
        *this->InfoFileStream << "  \"" << di << "\"\n";
        }
      }
    }
  *this->InfoFileStream
    << "  )\n";
  }

  // Check for a target-specific module output directory.
  if(const char* mdir = this->GetFortranModuleDirectory())
    {
    *this->InfoFileStream
      << "\n"
      << "# Fortran module output directory.\n"
      << "SET(CMAKE_Fortran_TARGET_MODULE_DIR \"" << mdir << "\")\n";
    }

  // Target-specific include directories:
  *this->InfoFileStream
    << "\n"
    << "# The include file search paths:\n";
  *this->InfoFileStream
    << "SET(CMAKE_C_TARGET_INCLUDE_PATH\n";
  std::vector<std::string> includes;

  const char *config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  this->LocalGenerator->GetIncludeDirectories(includes,
                                              this->GeneratorTarget,
                                              "C", config);
  for(std::vector<std::string>::iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    *this->InfoFileStream
      << "  \""
      << this->LocalGenerator->Convert(i->c_str(),
                                       cmLocalGenerator::HOME_OUTPUT)
      << "\"\n";
    }
  *this->InfoFileStream
    << "  )\n";
  *this->InfoFileStream
    << "SET(CMAKE_CXX_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";
  *this->InfoFileStream
    << "SET(CMAKE_Fortran_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";
  *this->InfoFileStream
    << "SET(CMAKE_ASM_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";

  // and now write the rule to use it
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the name of the dependency generation target.
  std::string depTarget =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  depTarget += "/depend";

  // Add a command to call CMake to scan dependencies.  CMake will
  // touch the corresponding depends file after scanning dependencies.
  cmOStringStream depCmd;
  // TODO: Account for source file properties and directory-level
  // definitions when scanning for dependencies.
#if !defined(_WIN32) || defined(__CYGWIN__)
  // This platform supports symlinks, so cmSystemTools will translate
  // paths.  Make sure PWD is set to the original name of the home
  // output directory to help cmSystemTools to create the same
  // translation table for the dependency scanning process.
  depCmd << "cd "
         << (this->LocalGenerator->Convert(
               this->Makefile->GetHomeOutputDirectory(),
               cmLocalGenerator::FULL, cmLocalGenerator::SHELL))
         << " && ";
#endif
  // Generate a call this signature:
  //
  //   cmake -E cmake_depends <generator>
  //                          <home-src-dir> <start-src-dir>
  //                          <home-out-dir> <start-out-dir>
  //                          <dep-info> --color=$(COLOR)
  //
  // This gives the dependency scanner enough information to recreate
  // the state of our local generator sufficiently for its needs.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends \""
         << this->GlobalGenerator->GetName() << "\" "
         << this->Convert(this->Makefile->GetHomeDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetStartDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetHomeOutputDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetStartOutputDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->InfoFileNameFull.c_str(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL);
  if(this->LocalGenerator->GetColorMakefile())
    {
    depCmd << " --color=$(COLOR)";
    }
  commands.push_back(depCmd.str());

  // Make sure all custom command outputs in this target are built.
  if(this->CustomCommandDriver == OnDepends)
    {
    this->DriveCustomCommands(depends);
    }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depTarget.c_str(),
                                      depends, commands, true);
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::DriveCustomCommands(std::vector<std::string>& depends)
{
  // Depend on all custom command outputs.
  const std::vector<cmSourceFile*>& sources =
    this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(cmCustomCommand* cc = (*source)->GetCustomCommand())
      {
      const std::vector<std::string>& outputs = cc->GetOutputs();
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        depends.push_back(*o);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::WriteObjectDependRules(cmSourceFile& source,
                         std::vector<std::string>& depends)
{
  // Create the list of dependencies known at cmake time.  These are
  // shared between the object file and dependency scanning rule.
  depends.push_back(source.GetFullPath());
  if(const char* objectDeps = source.GetProperty("OBJECT_DEPENDS"))
    {
    std::vector<std::string> deps;
    cmSystemTools::ExpandListArgument(objectDeps, deps);
    for(std::vector<std::string>::iterator i = deps.begin();
        i != deps.end(); ++i)
      {
      depends.push_back(i->c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::GenerateCustomRuleFile(const cmCustomCommand& cc)
{
  // Collect the commands.
  std::vector<std::string> commands;
  std::string comment = this->LocalGenerator->ConstructComment(cc);
  if(!comment.empty())
    {
    // add in a progress call if needed
    this->AppendProgress(commands);
    if(!this->NoRuleMessages)
      {
      this->LocalGenerator
        ->AppendEcho(commands, comment.c_str(),
                     cmLocalUnixMakefileGenerator3::EchoGenerate);
      }
    }

  // Now append the actual user-specified commands.
  cmOStringStream content;
  this->LocalGenerator->AppendCustomCommand(commands, cc, this->Target, false,
                                            cmLocalGenerator::HOME_OUTPUT,
                                            &content);

  // Collect the dependencies.
  std::vector<std::string> depends;
  this->LocalGenerator->AppendCustomDepend(depends, cc);

  // Check whether we need to bother checking for a symbolic output.
  bool need_symbolic = this->GlobalGenerator->GetNeedSymbolicMark();

  // Write the rule.
  const std::vector<std::string>& outputs = cc.GetOutputs();
  std::vector<std::string>::const_iterator o = outputs.begin();
  {
  bool symbolic = false;
  if(need_symbolic)
    {
    if(cmSourceFile* sf = this->Makefile->GetSource(o->c_str()))
      {
      symbolic = sf->GetPropertyAsBool("SYMBOLIC");
      }
    }
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      o->c_str(), depends, commands,
                                      symbolic);

  // If the rule has changed make sure the output is rebuilt.
  if(!symbolic)
    {
    this->GlobalGenerator->AddRuleHash(cc.GetOutputs(), content.str());
    }
  }

  // Write rules to drive building any outputs beyond the first.
  const char* in = o->c_str();
  for(++o; o != outputs.end(); ++o)
    {
    bool symbolic = false;
    if(need_symbolic)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(o->c_str()))
        {
        symbolic = sf->GetPropertyAsBool("SYMBOLIC");
        }
      }
    this->GenerateExtraOutput(o->c_str(), in, symbolic);
    }

  // Setup implicit dependency scanning.
  for(cmCustomCommand::ImplicitDependsList::const_iterator
        idi = cc.GetImplicitDepends().begin();
      idi != cc.GetImplicitDepends().end(); ++idi)
    {
    std::string objFullPath =
      this->Convert(outputs[0].c_str(), cmLocalGenerator::FULL);
    std::string srcFullPath =
      this->Convert(idi->second.c_str(), cmLocalGenerator::FULL);
    this->LocalGenerator->
      AddImplicitDepends(*this->Target, idi->first.c_str(),
                         objFullPath.c_str(),
                         srcFullPath.c_str());
    }
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::GenerateExtraOutput(const char* out, const char* in, bool symbolic)
{
  // Add a rule to build the primary output if the extra output needs
  // to be created.
  std::vector<std::string> commands;
  std::vector<std::string> depends;
  std::string emptyCommand = this->GlobalGenerator->GetEmptyRuleHackCommand();
  if(!emptyCommand.empty())
    {
    commands.push_back(emptyCommand);
    }
  depends.push_back(in);
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      out, depends, commands,
                                      symbolic);

  // Register the extra output as paired with the first output so that
  // the check-build-system step will remove the primary output if any
  // extra outputs are missing.  This forces the rule to regenerate
  // all outputs.
  this->AddMultipleOutputPair(out, in);
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator::AppendProgress(std::vector<std::string>& commands)
{
  this->NumberOfProgressActions++;
  if(this->NoRuleMessages)
    {
    return;
    }
  std::string progressDir = this->Makefile->GetHomeOutputDirectory();
  progressDir += cmake::GetCMakeFilesDirectory();
  cmOStringStream progCmd;
  progCmd << "$(CMAKE_COMMAND) -E cmake_progress_report ";
  progCmd << this->LocalGenerator->Convert(progressDir.c_str(),
                                           cmLocalGenerator::FULL,
                                           cmLocalGenerator::SHELL);
  progCmd << " $(CMAKE_PROGRESS_" << this->NumberOfProgressActions << ")";
  commands.push_back(progCmd.str());
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectsVariable(std::string& variableName,
                       std::string& variableNameExternal)
{
  // Write a make variable assignment that lists all objects for the
  // target.
  variableName =
    this->LocalGenerator->CreateMakeVariable(this->Target->GetName(),
                                             "_OBJECTS");
  *this->BuildFileStream
    << "# Object files for target " << this->Target->GetName() << "\n"
    << variableName.c_str() << " =";
  std::string object;
  const char* objName =
    this->Makefile->GetDefinition("CMAKE_NO_QUOTED_OBJECTS");
  const char* lineContinue =
    this->Makefile->GetDefinition("CMAKE_MAKE_LINE_CONTINUE");
  if(!lineContinue)
    {
    lineContinue = "\\";
    }
  for(std::vector<std::string>::const_iterator i = this->Objects.begin();
      i != this->Objects.end(); ++i)
    {
    *this->BuildFileStream << " " << lineContinue << "\n";
    if(objName)
      {
      *this->BuildFileStream <<
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::MAKEFILE);
      }
    else
      {
      *this->BuildFileStream  <<
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
  *this->BuildFileStream << "\n";

  // Write a make variable assignment that lists all external objects
  // for the target.
  variableNameExternal =
    this->LocalGenerator->CreateMakeVariable(this->Target->GetName(),
                                             "_EXTERNAL_OBJECTS");
  *this->BuildFileStream
    << "\n"
    << "# External object files for target "
    << this->Target->GetName() << "\n"
    << variableNameExternal.c_str() << " =";
  for(std::vector<std::string>::const_iterator i =
        this->ExternalObjects.begin();
      i != this->ExternalObjects.end(); ++i)
    {
    object = this->Convert(i->c_str(),cmLocalGenerator::START_OUTPUT);
    *this->BuildFileStream
      << " " << lineContinue << "\n"
      << this->Makefile->GetSafeDefinition("CMAKE_OBJECT_NAME");
    if(objName)
      {
      *this->BuildFileStream  <<
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::MAKEFILE);
      }
    else
      {
      *this->BuildFileStream  <<
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
  *this->BuildFileStream << "\n" << "\n";
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectsString(std::string& buildObjs)
{
  std::vector<std::string> objStrings;
  this->WriteObjectsStrings(objStrings);
  buildObjs = objStrings[0];
}

//----------------------------------------------------------------------------
class cmMakefileTargetGeneratorObjectStrings
{
public:
  cmMakefileTargetGeneratorObjectStrings(std::vector<std::string>& strings,
                                         cmLocalUnixMakefileGenerator3* lg,
                                         std::string::size_type limit):
    Strings(strings), LocalGenerator(lg), LengthLimit(limit)
    {
    this->Space = "";
    }
  void Feed(std::string const& obj)
    {
    // Construct the name of the next object.
    this->NextObject =
      this->LocalGenerator->Convert(obj.c_str(),
                                    cmLocalGenerator::START_OUTPUT,
                                    cmLocalGenerator::RESPONSE);

    // Roll over to next string if the limit will be exceeded.
    if(this->LengthLimit != std::string::npos &&
       (this->CurrentString.length() + 1 + this->NextObject.length()
        > this->LengthLimit))
      {
      this->Strings.push_back(this->CurrentString);
      this->CurrentString = "";
      this->Space = "";
      }

    // Separate from previous object.
    this->CurrentString += this->Space;
    this->Space = " ";

    // Append this object.
    this->CurrentString += this->NextObject;
    }
  void Done()
    {
    this->Strings.push_back(this->CurrentString);
    }
private:
  std::vector<std::string>& Strings;
  cmLocalUnixMakefileGenerator3* LocalGenerator;
  std::string::size_type LengthLimit;
  std::string CurrentString;
  std::string NextObject;
  const char* Space;
};

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectsStrings(std::vector<std::string>& objStrings,
                      std::string::size_type limit)
{
  cmMakefileTargetGeneratorObjectStrings
    helper(objStrings, this->LocalGenerator, limit);
  for(std::vector<std::string>::const_iterator i = this->Objects.begin();
      i != this->Objects.end(); ++i)
    {
    helper.Feed(*i);
    }
  for(std::vector<std::string>::const_iterator i =
        this->ExternalObjects.begin();
      i != this->ExternalObjects.end(); ++i)
    {
    helper.Feed(*i);
    }
  helper.Done();
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetDriverRule(const char* main_output,
                                                      bool relink)
{
  // Compute the name of the driver target.
  std::string dir =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += relink?"/preinstall":"/build";
  buildTargetRuleName = this->Convert(buildTargetRuleName.c_str(),
                                      cmLocalGenerator::HOME_OUTPUT,
                                      cmLocalGenerator::UNCHANGED);

  // Build the list of target outputs to drive.
  std::vector<std::string> depends;
  if(main_output)
    {
    depends.push_back(main_output);
    }

  const char* comment = 0;
  if(relink)
    {
    // Setup the comment for the preinstall driver.
    comment = "Rule to relink during preinstall.";
    }
  else
    {
    // Setup the comment for the main build driver.
    comment = "Rule to build all files generated by this target.";

    // Make sure all custom command outputs in this target are built.
    if(this->CustomCommandDriver == OnBuild)
      {
      this->DriveCustomCommands(depends);
      }

    // Make sure the extra files are built.
    for(std::set<cmStdString>::const_iterator i = this->ExtraFiles.begin();
        i != this->ExtraFiles.end(); ++i)
      {
      depends.push_back(*i);
      }
    }

  // Write the driver rule.
  std::vector<std::string> no_commands;
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, comment,
                                      buildTargetRuleName.c_str(),
                                      depends, no_commands, true);
}

//----------------------------------------------------------------------------
std::string cmMakefileTargetGenerator::GetFrameworkFlags()
{
 if(!this->Makefile->IsOn("APPLE"))
   {
   return std::string();
   }

 std::set<cmStdString> emitted;
#ifdef __APPLE__  /* don't insert this when crosscompiling e.g. to iphone */
  emitted.insert("/System/Library/Frameworks");
#endif
  std::vector<std::string> includes;

  const char *config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  this->LocalGenerator->GetIncludeDirectories(includes,
                                              this->GeneratorTarget,
                                              "C", config);
  // check all include directories for frameworks as this
  // will already have added a -F for the framework
  for(std::vector<std::string>::iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    if(this->Target->NameResolvesToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      emitted.insert(frameworkDir);
      }
    }

  std::string flags;
  const char* cfg = this->LocalGenerator->ConfigurationName.c_str();
  if(cmComputeLinkInformation* cli = this->Target->GetLinkInformation(cfg))
    {
    std::vector<std::string> const& frameworks = cli->GetFrameworkPaths();
    for(std::vector<std::string>::const_iterator i = frameworks.begin();
        i != frameworks.end(); ++i)
      {
      if(emitted.insert(*i).second)
        {
        flags += "-F";
        flags += this->Convert(i->c_str(),
                               cmLocalGenerator::START_OUTPUT,
                               cmLocalGenerator::SHELL, true);
        flags += " ";
        }
      }
    }
  return flags;
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::AppendTargetDepends(std::vector<std::string>& depends)
{
  // Static libraries never depend on anything for linking.
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY)
    {
    return;
    }

  // Loop over all library dependencies.
  const char* cfg = this->LocalGenerator->ConfigurationName.c_str();
  if(cmComputeLinkInformation* cli = this->Target->GetLinkInformation(cfg))
    {
    std::vector<std::string> const& libDeps = cli->GetDepends();
    for(std::vector<std::string>::const_iterator j = libDeps.begin();
        j != libDeps.end(); ++j)
      {
      depends.push_back(*j);
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::AppendObjectDepends(std::vector<std::string>& depends)
{
  // Add dependencies on the compiled object files.
  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget;
  for(std::vector<std::string>::const_iterator obj = this->Objects.begin();
      obj != this->Objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    depends.push_back(objTarget);
    }

  // Add dependencies on the external object files.
  for(std::vector<std::string>::const_iterator obj
        = this->ExternalObjects.begin();
      obj != this->ExternalObjects.end(); ++obj)
    {
    depends.push_back(*obj);
    }

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->BuildFileNameFull.c_str());
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::AppendLinkDepends(std::vector<std::string>& depends)
{
  this->AppendObjectDepends(depends);

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the link definitions file, if any.
  if(!this->GeneratorTarget->ModuleDefinitionFile.empty())
    {
    depends.push_back(this->GeneratorTarget->ModuleDefinitionFile);
    }

  // Add user-specified dependencies.
  if(const char* linkDepends =
     this->Target->GetProperty("LINK_DEPENDS"))
    {
    cmSystemTools::ExpandListArgument(linkDepends, depends);
    }
}

//----------------------------------------------------------------------------
std::string cmMakefileTargetGenerator::GetLinkRule(const char* linkRuleVar)
{
  std::string linkRule = this->Makefile->GetRequiredDefinition(linkRuleVar);
  if(this->Target->HasImplibGNUtoMS())
    {
    std::string ruleVar = "CMAKE_";
    ruleVar += this->Target->GetLinkerLanguage(this->ConfigName);
    ruleVar += "_GNUtoMS_RULE";
    if(const char* rule = this->Makefile->GetDefinition(ruleVar.c_str()))
      {
      linkRule += rule;
      }
    }
  return linkRule;
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::CloseFileStreams()
{
  delete this->BuildFileStream;
  delete this->InfoFileStream;
  delete this->FlagFileStream;
}

void cmMakefileTargetGenerator::RemoveForbiddenFlags(const char* flagVar,
                                                     const char* linkLang,
                                                     std::string& linkFlags)
{
  // check for language flags that are not allowed at link time, and
  // remove them, -w on darwin for gcc -w -dynamiclib sends -w to libtool
  // which fails, there may be more]

  std::string removeFlags = "CMAKE_";
  removeFlags += linkLang;
  removeFlags += flagVar;
  std::string removeflags =
    this->Makefile->GetSafeDefinition(removeFlags.c_str());
  std::vector<std::string> removeList;
  cmSystemTools::ExpandListArgument(removeflags, removeList);
  for(std::vector<std::string>::iterator i = removeList.begin();
      i != removeList.end(); ++i)
    {
    cmSystemTools::ReplaceString(linkFlags, i->c_str(), "");
    }
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::AddMultipleOutputPair(const char* depender, const char* dependee)
{
  MultipleOutputPairsType::value_type p(depender, dependee);
  this->MultipleOutputPairs.insert(p);
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::CreateLinkScript(const char* name,
                   std::vector<std::string> const& link_commands,
                   std::vector<std::string>& makefile_commands,
                   std::vector<std::string>& makefile_depends)
{
  // Create the link script file.
  std::string linkScriptName = this->TargetBuildDirectoryFull;
  linkScriptName += "/";
  linkScriptName += name;
  cmGeneratedFileStream linkScriptStream(linkScriptName.c_str());
  linkScriptStream.SetCopyIfDifferent(true);
  for(std::vector<std::string>::const_iterator cmd = link_commands.begin();
      cmd != link_commands.end(); ++cmd)
    {
    // Do not write out empty commands or commands beginning in the
    // shell no-op ":".
    if(!cmd->empty() && (*cmd)[0] != ':')
      {
      linkScriptStream << *cmd << "\n";
      }
    }

  // Create the makefile command to invoke the link script.
  std::string link_command = "$(CMAKE_COMMAND) -E cmake_link_script ";
  link_command += this->Convert(linkScriptName.c_str(),
                                cmLocalGenerator::START_OUTPUT,
                                cmLocalGenerator::SHELL);
  link_command += " --verbose=$(VERBOSE)";
  makefile_commands.push_back(link_command);
  makefile_depends.push_back(linkScriptName);
}

//----------------------------------------------------------------------------
std::string
cmMakefileTargetGenerator
::CreateResponseFile(const char* name, std::string const& options,
                     std::vector<std::string>& makefile_depends)
{
  // Create the response file.
  std::string responseFileNameFull = this->TargetBuildDirectoryFull;
  responseFileNameFull += "/";
  responseFileNameFull += name;
  cmGeneratedFileStream responseStream(responseFileNameFull.c_str());
  responseStream.SetCopyIfDifferent(true);
  responseStream << options << "\n";

  // Add a dependency so the target will rebuild when the set of
  // objects changes.
  makefile_depends.push_back(responseFileNameFull);

  // Construct the name to be used on the command line.
  std::string responseFileName = this->TargetBuildDirectory;
  responseFileName += "/";
  responseFileName += name;
  return responseFileName;
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::CreateObjectLists(bool useLinkScript, bool useArchiveRules,
                    bool useResponseFile, std::string& buildObjs,
                    std::vector<std::string>& makefile_depends)
{
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(variableName, variableNameExternal);
  if(useResponseFile)
    {
    // MSVC response files cannot exceed 128K.
    std::string::size_type const responseFileLimit = 131000;

    // Construct the individual object list strings.
    std::vector<std::string> object_strings;
    this->WriteObjectsStrings(object_strings, responseFileLimit);

    // Lookup the response file reference flag.
    std::string responseFlagVar = "CMAKE_";
    responseFlagVar += this->Target->GetLinkerLanguage(this->ConfigName);
    responseFlagVar += "_RESPONSE_FILE_LINK_FLAG";
    const char* responseFlag =
      this->Makefile->GetDefinition(responseFlagVar.c_str());
    if(!responseFlag)
      {
      responseFlag = "@";
      }

    // Write a response file for each string.
    const char* sep = "";
    for(unsigned int i = 0; i < object_strings.size(); ++i)
      {
      // Number the response files.
      char rsp[32];
      sprintf(rsp, "objects%u.rsp", i+1);

      // Create this response file.
      std::string objects_rsp =
        this->CreateResponseFile(rsp, object_strings[i], makefile_depends);

      // Separate from previous response file references.
      buildObjs += sep;
      sep = " ";

      // Reference the response file.
      buildObjs += responseFlag;
      buildObjs += this->Convert(objects_rsp.c_str(),
                                 cmLocalGenerator::NONE,
                                 cmLocalGenerator::SHELL);
      }
    }
  else if(useLinkScript)
    {
    if(!useArchiveRules)
      {
      this->WriteObjectsString(buildObjs);
      }
    }
  else
    {
    buildObjs = "$(";
    buildObjs += variableName;
    buildObjs += ") $(";
    buildObjs += variableNameExternal;
    buildObjs += ")";
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::AddIncludeFlags(std::string& flags,
                                                const char* lang)
{
  std::string responseVar = "CMAKE_";
  responseVar += lang;
  responseVar += "_USE_RESPONSE_FILE_FOR_INCLUDES";
  bool useResponseFile = this->Makefile->IsOn(responseVar.c_str());


  std::vector<std::string> includes;
  const char *config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  this->LocalGenerator->GetIncludeDirectories(includes,
                                              this->GeneratorTarget,
                                              lang, config);

  std::string includeFlags =
    this->LocalGenerator->GetIncludeFlags(includes, this->GeneratorTarget,
                                          lang, useResponseFile);
  if(includeFlags.empty())
    {
    return;
    }

  if(useResponseFile)
    {
    std::string name = "includes_";
    name += lang;
    name += ".rsp";
    std::string arg = "@" +
      this->CreateResponseFile(name.c_str(), includeFlags,
                               this->FlagFileDepends[lang]);
    this->LocalGenerator->AppendFlags(flags, arg.c_str());
    }
  else
    {
    this->LocalGenerator->AppendFlags(flags, includeFlags.c_str());
    }
}

//----------------------------------------------------------------------------
const char* cmMakefileTargetGenerator::GetFortranModuleDirectory()
{
  // Compute the module directory.
  if(!this->FortranModuleDirectoryComputed)
    {
    const char* target_mod_dir =
      this->Target->GetProperty("Fortran_MODULE_DIRECTORY");
    const char* moddir_flag =
      this->Makefile->GetDefinition("CMAKE_Fortran_MODDIR_FLAG");
    if(target_mod_dir && moddir_flag)
      {
      // Compute the full path to the module directory.
      if(cmSystemTools::FileIsFullPath(target_mod_dir))
        {
        // Already a full path.
        this->FortranModuleDirectory = target_mod_dir;
        }
      else
        {
        // Interpret relative to the current output directory.
        this->FortranModuleDirectory =
          this->Makefile->GetCurrentOutputDirectory();
        this->FortranModuleDirectory += "/";
        this->FortranModuleDirectory += target_mod_dir;
        }

      // Make sure the module output directory exists.
      cmSystemTools::MakeDirectory(this->FortranModuleDirectory.c_str());
      }
    this->FortranModuleDirectoryComputed = true;
    }

  // Return the computed directory.
  if(this->FortranModuleDirectory.empty())
    {
    return 0;
    }
  else
    {
    return this->FortranModuleDirectory.c_str();
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::AddFortranFlags(std::string& flags)
{
  // Enable module output if necessary.
  if(const char* modout_flag =
     this->Makefile->GetDefinition("CMAKE_Fortran_MODOUT_FLAG"))
    {
    this->LocalGenerator->AppendFlags(flags, modout_flag);
    }

  // Add a module output directory flag if necessary.
  const char* mod_dir = this->GetFortranModuleDirectory();
  if(!mod_dir)
    {
    mod_dir = this->Makefile->GetDefinition("CMAKE_Fortran_MODDIR_DEFAULT");
    }
  if(mod_dir)
    {
    const char* moddir_flag =
      this->Makefile->GetRequiredDefinition("CMAKE_Fortran_MODDIR_FLAG");
    std::string modflag = moddir_flag;
    modflag += this->Convert(mod_dir,
                             cmLocalGenerator::START_OUTPUT,
                             cmLocalGenerator::SHELL);
    this->LocalGenerator->AppendFlags(flags, modflag.c_str());
    }

  // If there is a separate module path flag then duplicate the
  // include path with it.  This compiler does not search the include
  // path for modules.
  if(const char* modpath_flag =
     this->Makefile->GetDefinition("CMAKE_Fortran_MODPATH_FLAG"))
    {
    std::vector<std::string> includes;
    const char *config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
    this->LocalGenerator->GetIncludeDirectories(includes,
                                                this->GeneratorTarget,
                                                "C", config);
    for(std::vector<std::string>::const_iterator idi = includes.begin();
        idi != includes.end(); ++idi)
      {
      std::string flg = modpath_flag;
      flg += this->Convert(idi->c_str(),
                           cmLocalGenerator::NONE,
                           cmLocalGenerator::SHELL);
      this->LocalGenerator->AppendFlags(flags, flg.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::AddModuleDefinitionFlag(std::string& flags)
{
  if(this->GeneratorTarget->ModuleDefinitionFile.empty())
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
             this->GeneratorTarget->ModuleDefinitionFile.c_str()));
  this->LocalGenerator->AppendFlags(flags, flag.c_str());
}

//----------------------------------------------------------------------------
const char* cmMakefileTargetGenerator::GetFeature(const char* feature)
{
  return this->Target->GetFeature(feature, this->ConfigName);
}

//----------------------------------------------------------------------------
bool cmMakefileTargetGenerator::GetFeatureAsBool(const char* feature)
{
  return cmSystemTools::IsOn(this->GetFeature(feature));
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::AddFeatureFlags(
  std::string& flags, const char* lang
  )
{
  // Add language-specific flags.
  this->LocalGenerator->AddLanguageFlags(flags, lang, this->ConfigName);

  if(this->GetFeatureAsBool("INTERPROCEDURAL_OPTIMIZATION"))
    {
    this->LocalGenerator->AppendFeatureOptions(flags, lang, "IPO");
    }
}
