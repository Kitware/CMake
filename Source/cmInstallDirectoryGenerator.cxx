/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallDirectoryGenerator.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallDirectoryGenerator::cmInstallDirectoryGenerator(
  std::vector<std::string> const& dirs, std::string const& dest,
  std::string file_permissions, std::string dir_permissions,
  std::vector<std::string> const& configurations, std::string const& component,
  MessageLevel message, bool exclude_from_all, std::string literal_args,
  bool optional, cmListFileBacktrace backtrace)
  : cmInstallGenerator(dest, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , Directories(dirs)
  , FilePermissions(std::move(file_permissions))
  , DirPermissions(std::move(dir_permissions))
  , LiteralArguments(std::move(literal_args))
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
  std::vector<std::string> directories;
  if (this->ActionsPerConfig) {
    for (std::string const& f : this->Directories) {
      cmExpandList(
        cmGeneratorExpression::Evaluate(f, this->LocalGenerator, config),
        directories);
    }
  } else {
    directories = this->Directories;
  }
  return directories;
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
  std::ostream& os, const std::string& config, Indent indent)
{
  std::vector<std::string> dirs = this->GetDirectories(config);

  // Make sure all dirs have absolute paths.
  cmMakefile const& mf = *this->LocalGenerator->GetMakefile();
  for (std::string& d : dirs) {
    if (!cmSystemTools::FileIsFullPath(d)) {
      d = cmStrCat(mf.GetCurrentSourceDirectory(), "/", d);
    }
  }

  this->AddDirectoryInstallRule(os, config, indent, dirs);
}

void cmInstallDirectoryGenerator::AddDirectoryInstallRule(
  std::ostream& os, const std::string& config, Indent indent,
  std::vector<std::string> const& dirs)
{
  // Write code to install the directories.
  const char* no_rename = nullptr;
  this->AddInstallRule(os, this->GetDestination(config),
                       cmInstallType_DIRECTORY, dirs, this->Optional,
                       this->FilePermissions.c_str(),
                       this->DirPermissions.c_str(), no_rename,
                       this->LiteralArguments.c_str(), indent);
}

std::string cmInstallDirectoryGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(this->Destination,
                                         this->LocalGenerator, config);
}
