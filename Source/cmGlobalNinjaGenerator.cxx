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
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmVersion.h"

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
  while((rpos = replace.find('\n', lpos)) != std::string::npos)
    {
    os << "# " << replace.substr(lpos, rpos - lpos) << "\n";
    lpos = rpos + 1;
    }
  os << "# " << replace.substr(lpos) << "\n";
}

static bool IsIdentChar(char c)
{
  return
    ('a' <= c && c <= 'z') ||
    ('+' <= c && c <= '9') ||  // +,-./ and numbers
    ('A' <= c && c <= 'Z') ||
    (c == '_') || (c == '$') || (c == '\\');
}

std::string cmGlobalNinjaGenerator::EncodeIdent(const std::string &ident,
                                                std::ostream &vars) {
  if (std::find_if(ident.begin(), ident.end(),
                   std::not1(std::ptr_fun(IsIdentChar))) != ident.end()) {
    static unsigned VarNum = 0;
    std::ostringstream names;
    names << "ident" << VarNum++;
    vars << names.str() << " = " << ident << "\n";
    return "$" + names.str();
  } else {
    return ident;
  }
}

std::string cmGlobalNinjaGenerator::EncodeLiteral(const std::string &lit)
{
  std::string result = lit;
  cmSystemTools::ReplaceString(result, "$", "$$");
  return result;
}

void cmGlobalNinjaGenerator::WriteBuild(std::ostream& os,
                                        const std::string& comment,
                                        const std::string& rule,
                                        const cmNinjaDeps& outputs,
                                        const cmNinjaDeps& explicitDeps,
                                        const cmNinjaDeps& implicitDeps,
                                        const cmNinjaDeps& orderOnlyDeps,
                                        const cmNinjaVars& variables)
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

  std::ostringstream builds;

  // TODO: Better formatting for when there are multiple input/output files.

  // Write outputs files.
  builds << "build";
  for(cmNinjaDeps::const_iterator i = outputs.begin();
      i != outputs.end();
      ++i)
    builds << " " << EncodeIdent(*i, os);
  builds << ":";

  // Write the rule.
  builds << " " << rule;

  // Write explicit dependencies.
  for(cmNinjaDeps::const_iterator i = explicitDeps.begin();
      i != explicitDeps.end();
      ++i)
    builds  << " " << EncodeIdent(*i, os);

  // Write implicit dependencies.
  if(!implicitDeps.empty())
    {
    builds << " |";
    for(cmNinjaDeps::const_iterator i = implicitDeps.begin();
        i != implicitDeps.end();
        ++i)
      builds  << " " << EncodeIdent(*i, os);
    }

  // Write order-only dependencies.
  if(!orderOnlyDeps.empty())
    {
    builds << " ||";
    for(cmNinjaDeps::const_iterator i = orderOnlyDeps.begin();
        i != orderOnlyDeps.end();
        ++i)
      builds  << " " << EncodeIdent(*i, os);
    }

  builds << "\n";

  os << builds.str();

  // Write the variables bound to this build statement.
  for(cmNinjaVars::const_iterator i = variables.begin();
      i != variables.end();
      ++i)
    cmGlobalNinjaGenerator::WriteVariable(os, i->first, i->second, "", 1);
}

void cmGlobalNinjaGenerator::WritePhonyBuild(std::ostream& os,
                                             const std::string& comment,
                                             const cmNinjaDeps& outputs,
                                             const cmNinjaDeps& explicitDeps,
                                             const cmNinjaDeps& implicitDeps,
                                             const cmNinjaDeps& orderOnlyDeps,
                                             const cmNinjaVars& variables)
{
  cmGlobalNinjaGenerator::WriteBuild(os,
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
  this->AddCustomCommandRule();

  cmNinjaVars vars;
  vars["COMMAND"] = command;
  vars["DESC"] = EncodeLiteral(description);

  cmGlobalNinjaGenerator::WriteBuild(*this->BuildFileStream,
                                     comment,
                                     "CUSTOM_COMMAND",
                                     outputs,
                                     deps,
                                     cmNinjaDeps(),
                                     orderOnlyDeps,
                                     vars);
}

void cmGlobalNinjaGenerator::WriteRule(std::ostream& os,
                                       const std::string& name,
                                       const std::string& command,
                                       const std::string& description,
                                       const std::string& comment,
                                       const std::string& depfile,
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
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
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

  this->WriteAssumedSourceDependencies(*this->BuildFileStream);
  this->WriteTargetAliases(*this->BuildFileStream);
  this->WriteBuiltinTargets(*this->BuildFileStream);

  this->CloseRulesFileStream();
  this->CloseBuildFileStream();
}

// Implemented in all cmGlobaleGenerator sub-classes.
// Used in:
//   Source/cmMakefile.cxx:
void cmGlobalNinjaGenerator
::EnableLanguage(std::vector<std::string>const& languages,
                 cmMakefile *mf,
                 bool optional)
{
  this->cmGlobalGenerator::EnableLanguage(languages, mf, optional);
  std::string path;
  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    if(*l == "NONE")
      {
      continue;
      }
    if(*l == "Fortran")
      {
      std::string message = "The \"";
      message += this->GetName();
      message += "\" generator does not support the language \"";
      message += *l;
      message += "\" yet.";
      cmSystemTools::Error(message.c_str());
      }
    this->ResolveLanguageCompiler(*l, mf, optional);
    }
}

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
                       const char* additionalOptions,
                       const char* targetName,
                       const char* config,
                       bool ignoreErrors,
                       bool fast)
{
  // Project name and config are not used yet.
  (void)projectName;
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
                                     bool restat,
                                     bool generator)
{
  // Do not add the same rule twice.
  if (this->HasRule(name))
    return;

  this->Rules.insert(name);
  cmGlobalNinjaGenerator::WriteRule(*this->RulesFileStream,
                                    name,
                                    command,
                                    description,
                                    comment,
                                    depfile,
                                    restat,
                                    generator);
}

bool cmGlobalNinjaGenerator::HasRule(const std::string &name)
{
  RulesSetType::const_iterator rule = this->Rules.find(name);
  return (rule != this->Rules.end());
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

void cmGlobalNinjaGenerator::WriteAssumedSourceDependencies(std::ostream& os)
{
  for (std::map<std::string, std::set<std::string> >::iterator
       i = this->AssumedSourceDependencies.begin();
       i != this->AssumedSourceDependencies.end(); ++i) {
    WriteCustomCommandBuild(/*command=*/"", /*description=*/"",
                            "Assume dependencies for generated source file.",
                            cmNinjaDeps(1, i->first),
                            cmNinjaDeps(i->second.begin(), i->second.end()));
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
    outputs.insert(outputs.end(), utils.begin(), utils.end());
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
    TargetAliases.insert(make_pair(alias, target));
  if (newAlias.second && newAlias.first->second != target)
    newAlias.first->second = 0;
}

void cmGlobalNinjaGenerator::WriteTargetAliases(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Target aliases.\n\n";

  for (TargetAliasMap::iterator i = TargetAliases.begin();
       i != TargetAliases.end(); ++i) {
    // Don't write ambiguous aliases.
    if (!i->second)
      continue;

    cmNinjaDeps deps;
    this->AppendTargetOutputs(i->second, deps);

    cmGlobalNinjaGenerator::WritePhonyBuild(os,
                                            "",
                                            cmNinjaDeps(1, i->first),
                                            deps);
  }
}

void cmGlobalNinjaGenerator::WriteBuiltinTargets(std::ostream& os)
{
  // Write headers.
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Built-in targets\n\n";

  this->WriteTargetAll(os);
  this->WriteTargetRebuildManifest(os);
}

void cmGlobalNinjaGenerator::WriteTargetAll(std::ostream& os)
{
  cmNinjaDeps outputs;
  outputs.push_back("all");

  cmGlobalNinjaGenerator::WritePhonyBuild(os,
                                          "The main all target.",
                                          outputs,
                                          this->AllDependencies);

  cmGlobalNinjaGenerator::WriteDefault(os,
                                       outputs,
                                       "Make the all target the default.");
}

void cmGlobalNinjaGenerator::WriteTargetRebuildManifest(std::ostream& os)
{
  cmMakefile* mfRoot = this->LocalGenerators[0]->GetMakefile();

  std::ostringstream cmd;
  cmd << mfRoot->GetRequiredDefinition("CMAKE_COMMAND")
      << " -H" << mfRoot->GetHomeDirectory()
      << " -B" << mfRoot->GetHomeOutputDirectory();
  WriteRule(*this->RulesFileStream,
            "RERUN_CMAKE",
            cmd.str(),
            "Re-running CMake...",
            "Rule for re-running cmake.",
            /*depfile=*/ "",
            /*restat=*/ false,
            /*generator=*/ true);

  cmNinjaDeps implicitDeps;
  for (std::vector<cmLocalGenerator *>::const_iterator i =
       this->LocalGenerators.begin(); i != this->LocalGenerators.end(); ++i) {
    const std::vector<std::string>& lf = (*i)->GetMakefile()->GetListFiles();
    implicitDeps.insert(implicitDeps.end(), lf.begin(), lf.end());
  }
  std::sort(implicitDeps.begin(), implicitDeps.end());
  implicitDeps.erase(std::unique(implicitDeps.begin(), implicitDeps.end()),
                     implicitDeps.end());
  implicitDeps.push_back("CMakeCache.txt");

  WriteBuild(os,
             "Re-run CMake if any of its inputs changed.",
             "RERUN_CMAKE",
             /*outputs=*/ cmNinjaDeps(1, NINJA_BUILD_FILE),
             /*explicitDeps=*/ cmNinjaDeps(),
             implicitDeps,
             /*orderOnlyDeps=*/ cmNinjaDeps(),
             /*variables=*/ cmNinjaVars());

  WritePhonyBuild(os,
                  "A missing CMake input file is not an error.",
                  implicitDeps,
                  cmNinjaDeps(),
                  cmNinjaDeps(),
                  cmNinjaDeps(),
                  cmNinjaVars());
}
