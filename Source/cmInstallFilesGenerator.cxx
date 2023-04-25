/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallFilesGenerator.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmInstallType.h"
#include "cmList.h"
#include "cmListFileCache.h"

class cmLocalGenerator;

cmInstallFilesGenerator::cmInstallFilesGenerator(
  std::vector<std::string> const& files, std::string const& dest,
  bool programs, std::string file_permissions,
  std::vector<std::string> const& configurations, std::string const& component,
  MessageLevel message, bool exclude_from_all, std::string rename,
  bool optional, cmListFileBacktrace backtrace)
  : cmInstallGenerator(dest, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , Files(files)
  , FilePermissions(std::move(file_permissions))
  , Rename(std::move(rename))
  , Programs(programs)
  , Optional(optional)
{
  // We need per-config actions if the destination and rename have generator
  // expressions.
  if (cmGeneratorExpression::Find(this->Destination) != std::string::npos) {
    this->ActionsPerConfig = true;
  }
  if (cmGeneratorExpression::Find(this->Rename) != std::string::npos) {
    this->ActionsPerConfig = true;
  }

  // We need per-config actions if any directories have generator expressions.
  if (!this->ActionsPerConfig) {
    for (std::string const& file : files) {
      if (cmGeneratorExpression::Find(file) != std::string::npos) {
        this->ActionsPerConfig = true;
        break;
      }
    }
  }
}

cmInstallFilesGenerator::~cmInstallFilesGenerator() = default;

bool cmInstallFilesGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return true;
}

std::string cmInstallFilesGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(this->Destination,
                                         this->LocalGenerator, config);
}

std::string cmInstallFilesGenerator::GetRename(std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(this->Rename, this->LocalGenerator,
                                         config);
}

std::vector<std::string> cmInstallFilesGenerator::GetFiles(
  std::string const& config) const
{
  if (this->ActionsPerConfig) {
    cmList files;
    for (std::string const& f : this->Files) {
      files.append(
        cmGeneratorExpression::Evaluate(f, this->LocalGenerator, config));
    }
    return std::move(files.data());
  }
  return this->Files;
}

void cmInstallFilesGenerator::AddFilesInstallRule(
  std::ostream& os, std::string const& config, Indent indent,
  std::vector<std::string> const& files)
{
  // Write code to install the files.
  const char* no_dir_permissions = nullptr;
  this->AddInstallRule(
    os, this->GetDestination(config),
    (this->Programs ? cmInstallType_PROGRAMS : cmInstallType_FILES), files,
    this->Optional, this->FilePermissions.c_str(), no_dir_permissions,
    this->GetRename(config).c_str(), nullptr, indent);
}

void cmInstallFilesGenerator::GenerateScriptActions(std::ostream& os,
                                                    Indent indent)
{
  if (this->ActionsPerConfig) {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
  } else {
    this->AddFilesInstallRule(os, "", indent, this->Files);
  }
}

void cmInstallFilesGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  std::vector<std::string> files = this->GetFiles(config);
  this->AddFilesInstallRule(os, config, indent, files);
}
