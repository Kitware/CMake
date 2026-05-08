/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallDirectoryGenerator.h"

#include <algorithm>
#include <utility>

#include "cmDiagnosticContext.h"
#include "cmGeneratorExpression.h"
#include "cmInstallType.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallDirectoryGenerator::cmInstallDirectoryGenerator(
  std::vector<std::string> const& dirs, std::string const& dest,
  std::string filePermissions, std::string dirPermissions,
  std::vector<std::string> const& configurations, std::string const& component,
  MessageLevel message, bool excludeFromAll, std::string literalArgs,
  bool optional, cmDiagnosticContext context)
  : cmInstallGenerator(dest, configurations, component, message,
                       excludeFromAll, false, std::move(context))
  , Directories(dirs)
  , FilePermissions(std::move(filePermissions))
  , DirPermissions(std::move(dirPermissions))
  , LiteralArguments(std::move(literalArgs))
  , Optional(optional)
{
  // We need per-config actions if destination have generator expressions.
  if (cmGeneratorExpression::Find(this->Destination) != std::string::npos) {
    this->ActionsPerConfig = true;
  }

  // We need per-config actions if any directories have generator expressions.
  if (!this->ActionsPerConfig) {
    for (std::string const& dir : dirs) {
      if (cmGeneratorExpression::Find(dir) != std::string::npos) {
        this->ActionsPerConfig = true;
        break;
      }
    }
  }
}

cmInstallDirectoryGenerator::~cmInstallDirectoryGenerator() = default;

bool cmInstallDirectoryGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return true;
}

std::vector<std::string> cmInstallDirectoryGenerator::GetDirectories(
  std::string const& config) const
{
  // If given only empty directories, collapse into a single specification to
  // avoid redundant calls. This supports the use case of installing an empty
  // directory into a destination when a directory is not specified.
  if (std::all_of(this->Directories.begin(), this->Directories.end(),
                  [](std::string const& d) { return d.empty(); })) {
    return std::vector<std::string>{ "" };
  }
  cmList directories;
  if (this->ActionsPerConfig) {
    for (std::string const& f : this->Directories) {
      directories.append(
        cmGeneratorExpression::Evaluate(f, this->LocalGenerator, config));
    }
  } else {
    directories = this->Directories;
  }
  return std::move(directories.data());
}

void cmInstallDirectoryGenerator::GenerateScriptActions(std::ostream& os,
                                                        Indent indent)
{
  if (this->ActionsPerConfig) {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
  } else {
    this->AddDirectoryInstallRule(os, "", indent, this->Directories);
  }
}

void cmInstallDirectoryGenerator::GenerateScriptForConfig(
  std::ostream& os, std::string const& config, Indent indent)
{
  std::vector<std::string> dirs = this->GetDirectories(config);

  if (!(dirs.size() == 1 && dirs[0].empty())) {
    // Make sure all dirs have absolute paths.
    cmMakefile const& mf = *this->LocalGenerator->GetMakefile();
    for (std::string& d : dirs) {
      if (!cmSystemTools::FileIsFullPath(d)) {
        d = cmStrCat(mf.GetCurrentSourceDirectory(), '/', d);
      }
    }
  }

  this->AddDirectoryInstallRule(os, config, indent, dirs);
}

void cmInstallDirectoryGenerator::AddDirectoryInstallRule(
  std::ostream& os, std::string const& config, Indent indent,
  std::vector<std::string> const& dirs)
{
  // Write code to install the directories.
  char const* noRename = nullptr;
  this->AddInstallRule(os, this->GetDestination(config),
                       cmInstallType_DIRECTORY, dirs, this->Optional,
                       this->FilePermissions.c_str(),
                       this->DirPermissions.c_str(), noRename,
                       this->LiteralArguments.c_str(), indent);
}

std::string cmInstallDirectoryGenerator::GetDestination(
  std::string const& config) const
{
  std::string dest = cmGeneratorExpression::Evaluate(
    this->Destination, this->LocalGenerator, config);
  this->CheckAbsoluteDestination(dest, this->LocalGenerator);
  return dest;
}
