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
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmVersion.h"

#include <algorithm>

const char* cmGlobalNinjaGenerator::NINJA_BUILD_FILE = "build.ninja";
const char* cmGlobalNinjaGenerator::NINJA_RULES_FILE = "rules.ninja";
const char* cmGlobalNinjaGenerator::INDENT = "  ";

void cmGlobalNinjaGenerator::Indent(std::ostream& os, int count)
{
  for(int i = 0; i < count; ++i)
    os << cmGlobalNinjaGenerator::INDENT;
}

void cmGlobalNinjaGenerator::WriteDivider(std::ostream& os)
{
  os
    << "# ======================================"
    << "=======================================\n";
}

void cmGlobalNinjaGenerator::WriteComment(std::ostream& os,
                                          const std::string& comment)
{
  if (comment.empty())
    return;

  std::string replace = comment;
  std::string::size_type lpos = 0;
  std::string::size_type rpos;
  os << "\n#############################################\n";
  while((rpos = replace.find('\n', lpos)) != std::string::npos)
    {
    os << "# " << replace.substr(lpos, rpos - lpos) << "\n";
    lpos = rpos + 1;
    }
  os << "# " << replace.substr(lpos) << "\n\n";
}

static bool IsIdentChar(char c)
{
  return
    ('a' <= c && c <= 'z') ||
    ('+' <= c && c <= '9') ||  // +,-./ and numbers
    ('A' <= c && c <= 'Z') ||
    (c == '_') || (c == '$') || (c == '\\') ||
    (c == ' ') || (c == ':');
}

std::string cmGlobalNinjaGenerator::EncodeIdent(const std::string &ident,
                                                std::ostream &vars) {
  if (std::find_if(ident.begin(), ident.end(),
                   std::not1(std::ptr_fun(IsIdentChar))) != ident.end()) {
    static unsigned VarNum = 0;
    cmOStringStream names;
    names << "ident" << VarNum++;
    vars << names.str() << " = " << ident << "\n";
    return "$" + names.str();
  } else {
    std::string result = ident;
    cmSystemTools::ReplaceString(result, " ", "$ ");
    cmSystemTools::ReplaceString(result, ":", "$:");
    return result;
  }
}

std::string cmGlobalNinjaGenerator::EncodeLiteral(const std::string &lit)
{
  std::string result = lit;
  cmSystemTools::ReplaceString(result, "$", "$$");
  cmSystemTools::ReplaceString(result, "\n", "$\n");
  return result;
}

std::string cmGlobalNinjaGenerator::EncodePath(const std::string &path)
{
  std::string result = path;
#ifdef _WIN32
  if(UsingMinGW)
    cmSystemTools::ReplaceString(result, "\\", "/");
  else
    cmSystemTools::ReplaceString(result, "/", "\\");
#endif
  return EncodeLiteral(result);
}

void cmGlobalNinjaGenerator::WriteBuild(std::ostream& os,
                                        const std::string& comment,
                                        const std::string& rule,
                                        const cmNinjaDeps& outputs,
                                        const cmNinjaDeps& explicitDeps,
                                        const cmNinjaDeps& implicitDeps,
                                        const cmNinjaDeps& orderOnlyDeps,
                                        const cmNinjaVars& variables,
                                        const std::string& rspfile,
                                        int cmdLineLimit)
{
  // Make sure there is a rule.
  if(rule.empty())
    {
    cmSystemTools::Error("No rule for WriteBuildStatement! called "
                         "with comment: ",
                         comment.c_str());
    return;
    }

  // Make sure there is at least one output file.
  if(outputs.empty())
    {
    cmSystemTools::Error("No output files for WriteBuildStatement! called "
                         "with comment: ",
                         comment.c_str());
    return;
    }

  cmGlobalNinjaGenerator::WriteComment(os, comment);

  cmOStringStream arguments;

  // TODO: Better formatting for when there are multiple input/output files.

  // Write explicit dependencies.
  for(cmNinjaDeps::const_iterator i = explicitDeps.begin();
      i != explicitDeps.end();
      ++i)
    {
    arguments  << " " << EncodeIdent(EncodePath(*i), os);

    //we need to track every dependency that comes in, since we are trying
    //to find dependencies that are side effects of build commands
    //
    this->CombinedBuildExplicitDependencies.insert(*i);
    }

  // Write implicit dependencies.
  if(!implicitDeps.empty())
    {
    arguments << " |";
    for(cmNinjaDeps::const_iterator i = implicitDeps.begin();
        i != implicitDeps.end();
        ++i)
      arguments  << " " << EncodeIdent(EncodePath(*i), os);
    }

  // Write order-only dependencies.
  if(!orderOnlyDeps.empty())
    {
    arguments << " ||";
    for(cmNinjaDeps::const_iterator i = orderOnlyDeps.begin();
        i != orderOnlyDeps.end();
        ++i)
      arguments  << " " << EncodeIdent(EncodePath(*i), os);
    }

  arguments << "\n";

  cmOStringStream build;

  // Write outputs files.
  build << "build";
  for(cmNinjaDeps::const_iterator i = outputs.begin();
      i != outputs.end(); ++i)
    {
    build << " " << EncodeIdent(EncodePath(*i), os);
    this->CombinedBuildOutputs.insert(*i);
    }
  build << ":";

  // Write the rule.
  build << " " << rule;

  // Write the variables bound to this build statement.
  cmOStringStream variable_assignments;
  for(cmNinjaVars::const_iterator i = variables.begin();
      i != variables.end(); ++i)
    cmGlobalNinjaGenerator::WriteVariable(variable_assignments,
                                          i->first, i->second, "", 1);

  // check if a response file rule should be used
  std::string buildstr = build.str();
  std::string assignments = variable_assignments.str();
  const std::string args = arguments.str();
  if (cmdLineLimit > 0
      && args.size() + buildstr.size() + assignments.size()
                                                    > (size_t) cmdLineLimit) {
    buildstr += "_RSP_FILE";
    variable_assignments.clear();
    cmGlobalNinjaGenerator::WriteVariable(variable_assignments,
                                          "RSP_FILE", rspfile, "", 1);
    assignments += variable_assignments.str();
  }

  os << buildstr << args << assignments;
}

void cmGlobalNinjaGenerator::WritePhonyBuild(std::ostream& os,
                                             const std::string& comment,
                                             const cmNinjaDeps& outputs,
                                             const cmNinjaDeps& explicitDeps,
                                             const cmNinjaDeps& implicitDeps,
                                             const cmNinjaDeps& orderOnlyDeps,
                                             const cmNinjaVars& variables)
{
  this->WriteBuild(os,
                   comment,
                   "phony",
                   outputs,
                   explicitDeps,
                   implicitDeps,
                   orderOnlyDeps,
                   variables);
}

void cmGlobalNinjaGenerator::AddCustomCommandRule()
{
  this->AddRule("CUSTOM_COMMAND",
                "$COMMAND",
                "$DESC",
                "Rule for running custom commands.",
                /*depfile*/ "",
                /*rspfile*/ "",
                /*rspcontent*/ "",
                /*restat*/ true);
}

void
cmGlobalNinjaGenerator::WriteCustomCommandBuild(const std::string& command,
                                                const std::string& description,
                                                const std::string& comment,
                                                const cmNinjaDeps& outputs,
                                                const cmNinjaDeps& deps,
                                              const cmNinjaDeps& orderOnlyDeps)
{
  std::string cmd = command;
#ifdef _WIN32
   if (cmd.empty())
      // TODO Shouldn't an empty command be handled by ninja?
      cmd = "cmd.exe /c";
#endif

  this->AddCustomCommandRule();

  cmNinjaVars vars;
  vars["COMMAND"] = cmd;
  vars["DESC"] = EncodeLiteral(description);

  this->WriteBuild(*this->BuildFileStream,
                   comment,
                   "CUSTOM_COMMAND",
                   outputs,
                   deps,
                   cmNinjaDeps(),
                   orderOnlyDeps,
                   vars);
}

void
cmGlobalNinjaGenerator::AddMacOSXContentRule()
{
  cmLocalGenerator *lg = this->LocalGenerators[0];
  cmMakefile* mfRoot = lg->GetMakefile();

  cmOStringStream cmd;
  cmd << lg->ConvertToOutputFormat(
           mfRoot->GetRequiredDefinition("CMAKE_COMMAND"),
           cmLocalGenerator::SHELL)
      << " -E copy $in $out";

  this->AddRule("COPY_OSX_CONTENT",
                cmd.str(),
                "Copying OS X Content $out",
                "Rule for copying OS X bundle content file."
                /*depfile*/ "",
                /*rspfile*/ "");
}

void
cmGlobalNinjaGenerator::WriteMacOSXContentBuild(const std::string& input,
                                                const std::string& output)
{
  this->AddMacOSXContentRule();

  cmNinjaDeps outputs;
  outputs.push_back(output);
  cmNinjaDeps deps;
  deps.push_back(input);
  cmNinjaVars vars;

  this->WriteBuild(*this->BuildFileStream,
                   "",
                   "COPY_OSX_CONTENT",
                   outputs,
                   deps,
                   cmNinjaDeps(),
                   cmNinjaDeps(),
                   cmNinjaVars());
}

void cmGlobalNinjaGenerator::WriteRule(std::ostream& os,
                                       const std::string& name,
                                       const std::string& command,
                                       const std::string& description,
                                       const std::string& comment,
                                       const std::string& depfile,
                                       const std::string& rspfile,
                                       const std::string& rspcontent,
                                       bool restat,
                                       bool generator)
{
  // Make sure the rule has a name.
  if(name.empty())
    {
    cmSystemTools::Error("No name given for WriteRuleStatement! called "
                         "with comment: ",
                         comment.c_str());
    return;
    }

  // Make sure a command is given.
  if(command.empty())
    {
    cmSystemTools::Error("No command given for WriteRuleStatement! called "
                         "with comment: ",
                         comment.c_str());
    return;
    }

  cmGlobalNinjaGenerator::WriteComment(os, comment);

  // Write the rule.
  os << "rule " << name << "\n";

  // Write the depfile if any.
  if(!depfile.empty())
    {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "depfile = " << depfile << "\n";
    }

  // Write the command.
  cmGlobalNinjaGenerator::Indent(os, 1);
  os << "command = " << command << "\n";

  // Write the description if any.
  if(!description.empty())
    {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "description = " << description << "\n";
    }

  if(!rspfile.empty())
    {
    if (rspcontent.empty())
      {
      cmSystemTools::Error("No rspfile_content given!", comment.c_str());
      return;
      }
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "rspfile = " << rspfile << "\n";
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "rspfile_content = " << rspcontent << "\n";
    }

  if(restat)
    {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "restat = 1\n";
    }

  if(generator)
    {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "generator = 1\n";
    }

  os << "\n";
}

void cmGlobalNinjaGenerator::WriteVariable(std::ostream& os,
                                           const std::string& name,
                                           const std::string& value,
                                           const std::string& comment,
                                           int indent)
{
  // Make sure we have a name.
  if(name.empty())
    {
    cmSystemTools::Error("No name given for WriteVariable! called "
                         "with comment: ",
                         comment.c_str());
    return;
    }

  // Do not add a variable if the value is empty.
  std::string val = cmSystemTools::TrimWhitespace(value);
  if(val.empty())
    {
    return;
    }

  cmGlobalNinjaGenerator::WriteComment(os, comment);
  cmGlobalNinjaGenerator::Indent(os, indent);
  os << name << " = " << val << "\n";
}

void cmGlobalNinjaGenerator::WriteInclude(std::ostream& os,
                                          const std::string& filename,
                                          const std::string& comment)
{
  cmGlobalNinjaGenerator::WriteComment(os, comment);
  os << "include " << filename << "\n";
}

void cmGlobalNinjaGenerator::WriteDefault(std::ostream& os,
                                          const cmNinjaDeps& targets,
                                          const std::string& comment)
{
  cmGlobalNinjaGenerator::WriteComment(os, comment);
  os << "default";
  for(cmNinjaDeps::const_iterator i = targets.begin(); i != targets.end(); ++i)
    os << " " << *i;
  os << "\n";
}


cmGlobalNinjaGenerator::cmGlobalNinjaGenerator()
  : cmGlobalGenerator()
  , BuildFileStream(0)
  , RulesFileStream(0)
  , CompileCommandsStream(0)
  , Rules()
  , AllDependencies()
{
  // // Ninja is not ported to non-Unix OS yet.
  // this->ForceUnixPaths = true;
  this->FindMakeProgramFile = "CMakeNinjaFindMake.cmake";
}


//----------------------------------------------------------------------------
// Virtual public methods.

cmLocalGenerator* cmGlobalNinjaGenerator::CreateLocalGenerator()
{
  cmLocalGenerator* lg = new cmLocalNinjaGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalNinjaGenerator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalNinjaGenerator::GetActualName();
  entry.Brief = "Generates build.ninja files (experimental).";
  entry.Full =
    "A build.ninja file is generated into the build tree. Recent "
    "versions of the ninja program can build the project through the "
    "\"all\" target.  An \"install\" target is also provided.";
}

// Implemented in all cmGlobaleGenerator sub-classes.
// Used in:
//   Source/cmLocalGenerator.cxx
//   Source/cmake.cxx
void cmGlobalNinjaGenerator::Generate()
{
  this->OpenBuildFileStream();
  this->OpenRulesFileStream();

  this->cmGlobalGenerator::Generate();

  this->WriteAssumedSourceDependencies();
  this->WriteTargetAliases(*this->BuildFileStream);
  this->WriteUnknownExplicitDependencies(*this->BuildFileStream);
  this->WriteBuiltinTargets(*this->BuildFileStream);

  if (cmSystemTools::GetErrorOccuredFlag()) {
    this->RulesFileStream->setstate(std::ios_base::failbit);
    this->BuildFileStream->setstate(std::ios_base::failbit);
  }

  this->CloseCompileCommandsStream();
  this->CloseRulesFileStream();
  this->CloseBuildFileStream();
}

// Implemented in all cmGlobaleGenerator sub-classes.
// Used in:
//   Source/cmMakefile.cxx:
void cmGlobalNinjaGenerator
::EnableLanguage(std::vector<std::string>const& langs,
                 cmMakefile* makefile,
                 bool optional)
{
  if (makefile->IsOn("CMAKE_COMPILER_IS_MINGW"))
    {
    UsingMinGW = true;
    this->EnableMinGWLanguage(makefile);
    }
  if (std::find(langs.begin(), langs.end(), "Fortran") != langs.end())
    {
    cmSystemTools::Error("The Ninja generator does not support Fortran yet.");
    }
  this->cmGlobalGenerator::EnableLanguage(langs, makefile, optional);
}

bool cmGlobalNinjaGenerator::UsingMinGW = false;

// Implemented by:
//   cmGlobalUnixMakefileGenerator3
//   cmGlobalVisualStudio10Generator
//   cmGlobalVisualStudio6Generator
//   cmGlobalVisualStudio7Generator
//   cmGlobalXCodeGenerator
// Called by:
//   cmGlobalGenerator::Build()
std::string cmGlobalNinjaGenerator
::GenerateBuildCommand(const char* makeProgram,
                       const char* projectName,
                       const char* projectDir,
                       const char* additionalOptions,
                       const char* targetName,
                       const char* config,
                       bool ignoreErrors,
                       bool fast)
{
  // Project name & dir and config are not used yet.
  (void)projectName;
  (void)projectDir;
  (void)config;
  // Ninja does not have -i equivalent option yet.
  (void)ignoreErrors;
  // We do not handle fast build yet.
  (void)fast;

  std::string makeCommand =
    cmSystemTools::ConvertToUnixOutputPath(makeProgram);

  if(additionalOptions)
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
    }
  if(targetName)
    {
    if(strcmp(targetName, "clean") == 0)
      {
      makeCommand += " -t clean";
      }
    else
      {
      makeCommand += " ";
      makeCommand += targetName;
      }
    }

  return makeCommand;
}

//----------------------------------------------------------------------------
// Non-virtual public methods.

void cmGlobalNinjaGenerator::AddRule(const std::string& name,
                                     const std::string& command,
                                     const std::string& description,
                                     const std::string& comment,
                                     const std::string& depfile,
                                     const std::string& rspfile,
                                     const std::string& rspcontent,
                                     bool restat,
                                     bool generator)
{
  // Do not add the same rule twice.
  if (this->HasRule(name))
    {
    return;
    }

  this->Rules.insert(name);
  cmGlobalNinjaGenerator::WriteRule(*this->RulesFileStream,
                                    name,
                                    command,
                                    description,
                                    comment,
                                    depfile,
                                    rspfile,
                                    rspcontent,
                                    restat,
                                    generator);

  this->RuleCmdLength[name] = (int) command.size();
}

bool cmGlobalNinjaGenerator::HasRule(const std::string &name)
{
  RulesSetType::const_iterator rule = this->Rules.find(name);
  return (rule != this->Rules.end());
}

//----------------------------------------------------------------------------
// Private virtual overrides

// TODO: Refactor to combine with cmGlobalUnixMakefileGenerator3 impl.
void cmGlobalNinjaGenerator::ComputeTargetObjects(cmGeneratorTarget* gt) const
{
  cmTarget* target = gt->Target;

  // Compute full path to object file directory for this target.
  std::string dir_max;
  dir_max += gt->Makefile->GetCurrentOutputDirectory();
  dir_max += "/";
  dir_max += gt->LocalGenerator->GetTargetDirectory(*target);
  dir_max += "/";
  gt->ObjectDirectory = dir_max;

  // Compute the name of each object file.
  for(std::vector<cmSourceFile*>::iterator
        si = gt->ObjectSources.begin();
      si != gt->ObjectSources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    std::string objectName = gt->LocalGenerator
      ->GetObjectFileNameWithoutTarget(*sf, dir_max);
    gt->Objects[sf] = objectName;
    }
}

//----------------------------------------------------------------------------
// Private methods

void cmGlobalNinjaGenerator::OpenBuildFileStream()
{
  // Compute Ninja's build file path.
  std::string buildFilePath =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  buildFilePath += "/";
  buildFilePath += cmGlobalNinjaGenerator::NINJA_BUILD_FILE;

  // Get a stream where to generate things.
  if (!this->BuildFileStream)
    {
    this->BuildFileStream = new cmGeneratedFileStream(buildFilePath.c_str());
    if (!this->BuildFileStream)
      {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return;
      }
    }

  // Write the do not edit header.
  this->WriteDisclaimer(*this->BuildFileStream);

  // Write a comment about this file.
  *this->BuildFileStream
    << "# This file contains all the build statements describing the\n"
    << "# compilation DAG.\n\n"
    ;
}

void cmGlobalNinjaGenerator::CloseBuildFileStream()
{
  if (this->BuildFileStream)
    {
    delete this->BuildFileStream;
    this->BuildFileStream = 0;
    }
  else
    {
    cmSystemTools::Error("Build file stream was not open.");
   }
}

void cmGlobalNinjaGenerator::OpenRulesFileStream()
{
  // Compute Ninja's build file path.
  std::string rulesFilePath =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  rulesFilePath += "/";
  rulesFilePath += cmGlobalNinjaGenerator::NINJA_RULES_FILE;

  // Get a stream where to generate things.
  if (!this->RulesFileStream)
    {
    this->RulesFileStream = new cmGeneratedFileStream(rulesFilePath.c_str());
    if (!this->RulesFileStream)
      {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return;
      }
    }

  // Write the do not edit header.
  this->WriteDisclaimer(*this->RulesFileStream);

  // Write comment about this file.
  *this->RulesFileStream
    << "# This file contains all the rules used to get the outputs files\n"
    << "# built from the input files.\n"
    << "# It is included in the main '" << NINJA_BUILD_FILE << "'.\n\n"
    ;
}

void cmGlobalNinjaGenerator::CloseRulesFileStream()
{
  if (this->RulesFileStream)
    {
    delete this->RulesFileStream;
    this->RulesFileStream = 0;
    }
  else
    {
    cmSystemTools::Error("Rules file stream was not open.");
   }
}

void cmGlobalNinjaGenerator::AddCXXCompileCommand(
                                      const std::string &commandLine,
                                      const std::string &sourceFile)
{
  // Compute Ninja's build file path.
  std::string buildFileDir =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  if (!this->CompileCommandsStream)
    {
    std::string buildFilePath = buildFileDir + "/compile_commands.json";

    // Get a stream where to generate things.
    this->CompileCommandsStream =
      new cmGeneratedFileStream(buildFilePath.c_str());
    *this->CompileCommandsStream << "[";
    } else {
    *this->CompileCommandsStream << "," << std::endl;
    }

  std::string sourceFileName = sourceFile;
  if (!cmSystemTools::FileIsFullPath(sourceFileName.c_str()))
    {
    sourceFileName = cmSystemTools::CollapseFullPath(
      sourceFileName.c_str(),
      this->GetCMakeInstance()->GetHomeOutputDirectory());
    }


  *this->CompileCommandsStream << "\n{\n"
     << "  \"directory\": \""
     << cmGlobalGenerator::EscapeJSON(buildFileDir) << "\",\n"
     << "  \"command\": \""
     << cmGlobalGenerator::EscapeJSON(commandLine) << "\",\n"
     << "  \"file\": \""
     << cmGlobalGenerator::EscapeJSON(sourceFileName) << "\"\n"
     << "}";
}

void cmGlobalNinjaGenerator::CloseCompileCommandsStream()
{
  if (this->CompileCommandsStream)
    {
    *this->CompileCommandsStream << "\n]";
    delete this->CompileCommandsStream;
    this->CompileCommandsStream = 0;
    }

}

void cmGlobalNinjaGenerator::WriteDisclaimer(std::ostream& os)
{
  os
    << "# CMAKE generated file: DO NOT EDIT!\n"
    << "# Generated by \"" << this->GetName() << "\""
    << " Generator, CMake Version "
    << cmVersion::GetMajorVersion() << "."
    << cmVersion::GetMinorVersion() << "\n\n";
}

void cmGlobalNinjaGenerator::AddDependencyToAll(cmTarget* target)
{
  this->AppendTargetOutputs(target, this->AllDependencies);
}

void cmGlobalNinjaGenerator::AddDependencyToAll(const std::string& input)
{
  this->AllDependencies.push_back(input);
}

void cmGlobalNinjaGenerator::WriteAssumedSourceDependencies()
{
  for (std::map<std::string, std::set<std::string> >::iterator
       i = this->AssumedSourceDependencies.begin();
       i != this->AssumedSourceDependencies.end(); ++i) {
    cmNinjaDeps deps;
    std::copy(i->second.begin(), i->second.end(), std::back_inserter(deps));
    WriteCustomCommandBuild(/*command=*/"", /*description=*/"",
                            "Assume dependencies for generated source file.",
                            cmNinjaDeps(1, i->first), deps);
  }
}

void
cmGlobalNinjaGenerator
::AppendTargetOutputs(cmTarget* target, cmNinjaDeps& outputs)
{
  const char* configName =
    target->GetMakefile()->GetDefinition("CMAKE_BUILD_TYPE");
  cmLocalNinjaGenerator *ng =
    static_cast<cmLocalNinjaGenerator *>(this->LocalGenerators[0]);

  switch (target->GetType()) {
  case cmTarget::EXECUTABLE:
  case cmTarget::SHARED_LIBRARY:
  case cmTarget::STATIC_LIBRARY:
  case cmTarget::MODULE_LIBRARY:
    outputs.push_back(ng->ConvertToNinjaPath(
      target->GetFullPath(configName).c_str()));
    break;

  case cmTarget::OBJECT_LIBRARY:
  case cmTarget::UTILITY: {
    std::string path = ng->ConvertToNinjaPath(
      target->GetMakefile()->GetStartOutputDirectory());
    if (path.empty() || path == ".")
      outputs.push_back(target->GetName());
    else {
      path += "/";
      path += target->GetName();
      outputs.push_back(path);
    }
    break;
  }

  case cmTarget::GLOBAL_TARGET:
    // Always use the target in HOME instead of an unused duplicate in a
    // subdirectory.
    outputs.push_back(target->GetName());
    break;

  default:
    return;
  }
}

void
cmGlobalNinjaGenerator
::AppendTargetDepends(cmTarget* target, cmNinjaDeps& outputs)
{
  if (target->GetType() == cmTarget::GLOBAL_TARGET) {
    // Global targets only depend on other utilities, which may not appear in
    // the TargetDepends set (e.g. "all").
    std::set<cmStdString> const& utils = target->GetUtilities();
    std::copy(utils.begin(), utils.end(), std::back_inserter(outputs));
  } else {
    cmTargetDependSet const& targetDeps =
      this->GetTargetDirectDepends(*target);
    for (cmTargetDependSet::const_iterator i = targetDeps.begin();
         i != targetDeps.end(); ++i) {
      this->AppendTargetOutputs(*i, outputs);
    }
  }
}

void cmGlobalNinjaGenerator::AddTargetAlias(const std::string& alias,
                                            cmTarget* target) {
  cmNinjaDeps outputs;
  this->AppendTargetOutputs(target, outputs);
  // Mark the target's outputs as ambiguous to ensure that no other target uses
  // the output as an alias.
  for (cmNinjaDeps::iterator i = outputs.begin(); i != outputs.end(); ++i)
    TargetAliases[*i] = 0;

  // Insert the alias into the map.  If the alias was already present in the
  // map and referred to another target, mark it as ambiguous.
  std::pair<TargetAliasMap::iterator, bool> newAlias =
    TargetAliases.insert(std::make_pair(alias, target));
  if (newAlias.second && newAlias.first->second != target)
    newAlias.first->second = 0;
}

void cmGlobalNinjaGenerator::WriteTargetAliases(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Target aliases.\n\n";

  for (TargetAliasMap::const_iterator i = TargetAliases.begin();
       i != TargetAliases.end(); ++i) {
    // Don't write ambiguous aliases.
    if (!i->second)
      continue;

    cmNinjaDeps deps;
    this->AppendTargetOutputs(i->second, deps);

    this->WritePhonyBuild(os,
                          "",
                          cmNinjaDeps(1, i->first),
                          deps);
  }
}

void cmGlobalNinjaGenerator::WriteUnknownExplicitDependencies(std::ostream& os)
{
  //now write out the unknown explicit dependencies.

  //union the configured files, evaluations files and the CombinedBuildOutputs,
  //and then difference with CombinedExplicitDependencies to find the explicit
  //dependencies that we have no rule for

  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Unknown Build Time Dependencies.\n"
     << "# Tell Ninja that they may appear as side effects of build rules\n"
     << "# otherwise ordered by order-only dependencies.\n\n";

  //get the list of files that cmake itself has generated as a
  //product of configuration.
  cmLocalNinjaGenerator *ng =
    static_cast<cmLocalNinjaGenerator *>(this->LocalGenerators[0]);

  std::set<std::string> knownDependencies;
  for (std::vector<cmLocalGenerator *>::const_iterator i =
       this->LocalGenerators.begin(); i != this->LocalGenerators.end(); ++i)
    {
    //get the vector of files created by this makefile and convert them
    //to ninja paths, which are all relative in respect to the build directory
    const std::vector<std::string>& files =
                                    (*i)->GetMakefile()->GetOutputFiles();
    typedef std::vector<std::string>::const_iterator vect_it;
    for(vect_it j = files.begin(); j != files.end(); ++j)
      {
      knownDependencies.insert(ng->ConvertToNinjaPath( j->c_str() ));
      }
    }

  for(std::vector<cmGeneratorExpressionEvaluationFile*>::const_iterator
      li = this->EvaluationFiles.begin();
      li != this->EvaluationFiles.end();
      ++li)
    {
    //get all the files created by generator expressions and convert them
    //to ninja paths
    std::vector<std::string> files = (*li)->GetFiles();
    typedef std::vector<std::string>::const_iterator vect_it;
    for(vect_it j = files.begin(); j != files.end(); ++j)
      {
      knownDependencies.insert(ng->ConvertToNinjaPath( j->c_str() ));
      }
    }

  //insert outputs from all WirteBuild commands
  knownDependencies.insert(this->CombinedBuildOutputs.begin(),
                           this->CombinedBuildOutputs.end());

  //after we have combined the data into knownDependencies we have no need
  //to keep this data around
  this->CombinedBuildOutputs.clear();

  for(TargetAliasMap::const_iterator i= this->TargetAliases.begin();
      i != this->TargetAliases.end();
      ++i)
    {
    knownDependencies.insert(i->first);
    }

  //remove all source files we know will exist.
  typedef std::map<std::string, std::set<std::string> >::const_iterator map_it;
  for(map_it i = this->AssumedSourceDependencies.begin();
      i != this->AssumedSourceDependencies.end();
      ++i)
    {
    knownDependencies.insert(i->first);
    }

  //now we difference with CombinedBuildExplicitDependencies to find
  //the list of items we know nothing about

  std::vector<std::string> unkownExplicitDepends;
  this->CombinedBuildExplicitDependencies.erase("all");

  std::set_difference(this->CombinedBuildExplicitDependencies.begin(),
                      this->CombinedBuildExplicitDependencies.end(),
                      knownDependencies.begin(),
                      knownDependencies.end(),
                      std::back_inserter(unkownExplicitDepends));


  std::string const rootBuildDirectory =
      this->GetCMakeInstance()->GetHomeOutputDirectory();
  for (std::vector<std::string>::const_iterator
       i = unkownExplicitDepends.begin();
       i != unkownExplicitDepends.end();
       ++i)
    {
    //verify the file is in the build directory
    std::string const absDepPath = cmSystemTools::CollapseFullPath(
                                     i->c_str(), rootBuildDirectory.c_str());
    bool const inBuildDir = cmSystemTools::IsSubDirectory(absDepPath.c_str(),
                                                  rootBuildDirectory.c_str());
    if(inBuildDir)
      {
      cmNinjaDeps deps(1,*i);
      this->WritePhonyBuild(os,
                            "",
                            deps,
                            deps);
      }
   }
}

void cmGlobalNinjaGenerator::WriteBuiltinTargets(std::ostream& os)
{
  // Write headers.
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Built-in targets\n\n";

  this->WriteTargetAll(os);
  this->WriteTargetRebuildManifest(os);
  this->WriteTargetClean(os);
  this->WriteTargetHelp(os);
}

void cmGlobalNinjaGenerator::WriteTargetAll(std::ostream& os)
{
  cmNinjaDeps outputs;
  outputs.push_back("all");

  this->WritePhonyBuild(os,
                        "The main all target.",
                        outputs,
                        this->AllDependencies);

  cmGlobalNinjaGenerator::WriteDefault(os,
                                       outputs,
                                       "Make the all target the default.");
}

void cmGlobalNinjaGenerator::WriteTargetRebuildManifest(std::ostream& os)
{
  cmLocalGenerator *lg = this->LocalGenerators[0];
  cmMakefile* mfRoot = lg->GetMakefile();

  cmOStringStream cmd;
  cmd << lg->ConvertToOutputFormat(
           mfRoot->GetRequiredDefinition("CMAKE_COMMAND"),
           cmLocalGenerator::SHELL)
      << " -H"
      << lg->ConvertToOutputFormat(mfRoot->GetHomeDirectory(),
                                   cmLocalGenerator::SHELL)
      << " -B"
      << lg->ConvertToOutputFormat(mfRoot->GetHomeOutputDirectory(),
                                   cmLocalGenerator::SHELL);
  WriteRule(*this->RulesFileStream,
            "RERUN_CMAKE",
            cmd.str(),
            "Re-running CMake...",
            "Rule for re-running cmake.",
            /*depfile=*/ "",
            /*rspfile=*/ "",
            /*rspcontent*/ "",
            /*restat=*/ false,
            /*generator=*/ true);

  cmNinjaDeps implicitDeps;
  for (std::vector<cmLocalGenerator *>::const_iterator i =
       this->LocalGenerators.begin(); i != this->LocalGenerators.end(); ++i) {
    const std::vector<std::string>& lf = (*i)->GetMakefile()->GetListFiles();
    implicitDeps.insert(implicitDeps.end(), lf.begin(), lf.end());

    const std::vector<std::string>& of = (*i)->GetMakefile()->GetOutputFiles();
    implicitDeps.insert(implicitDeps.end(), of.begin(), of.end());
  }
  std::sort(implicitDeps.begin(), implicitDeps.end());
  implicitDeps.erase(std::unique(implicitDeps.begin(), implicitDeps.end()),
                     implicitDeps.end());
  implicitDeps.push_back("CMakeCache.txt");

  this->WriteBuild(os,
                   "Re-run CMake if any of its inputs changed.",
                   "RERUN_CMAKE",
                   /*outputs=*/ cmNinjaDeps(1, NINJA_BUILD_FILE),
                   /*explicitDeps=*/ cmNinjaDeps(),
                   implicitDeps,
                   /*orderOnlyDeps=*/ cmNinjaDeps(),
                   /*variables=*/ cmNinjaVars());

  this->WritePhonyBuild(os,
                        "A missing CMake input file is not an error.",
                        implicitDeps,
                        cmNinjaDeps());
}

std::string cmGlobalNinjaGenerator::ninjaCmd() const
{
  cmLocalGenerator* lgen = this->LocalGenerators[0];
  if (lgen) {
    return lgen->ConvertToOutputFormat(
             lgen->GetMakefile()->GetRequiredDefinition("CMAKE_MAKE_PROGRAM"),
                                    cmLocalGenerator::SHELL);
  }
  return "ninja";
}

void cmGlobalNinjaGenerator::WriteTargetClean(std::ostream& os)
{
  WriteRule(*this->RulesFileStream,
            "CLEAN",
            (ninjaCmd() + " -t clean").c_str(),
            "Cleaning all built files...",
            "Rule for cleaning all built files.",
            /*depfile=*/ "",
            /*rspfile=*/ "",
            /*rspcontent*/ "",
            /*restat=*/ false,
            /*generator=*/ false);
  WriteBuild(os,
             "Clean all the built files.",
             "CLEAN",
             /*outputs=*/ cmNinjaDeps(1, "clean"),
             /*explicitDeps=*/ cmNinjaDeps(),
             /*implicitDeps=*/ cmNinjaDeps(),
             /*orderOnlyDeps=*/ cmNinjaDeps(),
             /*variables=*/ cmNinjaVars());
}

void cmGlobalNinjaGenerator::WriteTargetHelp(std::ostream& os)
{
  WriteRule(*this->RulesFileStream,
            "HELP",
            (ninjaCmd() + " -t targets").c_str(),
            "All primary targets available:",
            "Rule for printing all primary targets available.",
            /*depfile=*/ "",
            /*rspfile=*/ "",
            /*rspcontent*/ "",
            /*restat=*/ false,
            /*generator=*/ false);
  WriteBuild(os,
             "Print all primary targets available.",
             "HELP",
             /*outputs=*/ cmNinjaDeps(1, "help"),
             /*explicitDeps=*/ cmNinjaDeps(),
             /*implicitDeps=*/ cmNinjaDeps(),
             /*orderOnlyDeps=*/ cmNinjaDeps(),
             /*variables=*/ cmNinjaVars());
}
