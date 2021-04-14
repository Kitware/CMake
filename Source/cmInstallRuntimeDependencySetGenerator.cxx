/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallRuntimeDependencySetGenerator.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "cmGeneratorExpression.h"
#include "cmInstallGenerator.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

cmInstallRuntimeDependencySetGenerator::cmInstallRuntimeDependencySetGenerator(
  DependencyType type, cmInstallRuntimeDependencySet* dependencySet,
  std::vector<std::string> installRPaths, bool noInstallRPath,
  std::string installNameDir, bool noInstallName, const char* depsVar,
  const char* rpathPrefix, const char* tmpVarPrefix, std::string destination,
  std::vector<std::string> const& configurations, std::string component,
  std::string permissions, MessageLevel message, bool exclude_from_all,
  cmListFileBacktrace backtrace)
  : cmInstallGenerator(std::move(destination), configurations,
                       std::move(component), message, exclude_from_all, false,
                       std::move(backtrace))
  , Type(type)
  , DependencySet(dependencySet)
  , InstallRPaths(std::move(installRPaths))
  , NoInstallRPath(noInstallRPath)
  , InstallNameDir(std::move(installNameDir))
  , NoInstallName(noInstallName)
  , Permissions(std::move(permissions))
  , DepsVar(depsVar)
  , RPathPrefix(rpathPrefix)
  , TmpVarPrefix(tmpVarPrefix)
{
  this->ActionsPerConfig = true;
}

bool cmInstallRuntimeDependencySetGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return true;
}

void cmInstallRuntimeDependencySetGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  if (!this->LocalGenerator->GetMakefile()
         ->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL")
         .empty() &&
      !this->NoInstallName) {
    std::string installNameDir = "@rpath/";
    if (!this->InstallNameDir.empty()) {
      installNameDir = this->InstallNameDir;
      cmGeneratorExpression::ReplaceInstallPrefix(installNameDir,
                                                  "${CMAKE_INSTALL_PREFIX}");
      installNameDir = cmGeneratorExpression::Evaluate(
        installNameDir, this->LocalGenerator, config);
      if (installNameDir.empty()) {
        this->LocalGenerator->GetMakefile()->GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          "INSTALL_NAME_DIR argument must not evaluate to an "
          "empty string",
          this->Backtrace);
        return;
      }
      if (installNameDir.back() != '/') {
        installNameDir += '/';
      }
    }
    os << indent << "set(" << this->TmpVarPrefix << "_install_name_dir \""
       << installNameDir << "\")\n";
  }

  os << indent << "foreach(" << this->TmpVarPrefix << "_dep IN LISTS "
     << this->DepsVar << ")\n";

  if (!this->LocalGenerator->GetMakefile()
         ->GetSafeDefinition("CMAKE_INSTALL_NAME_TOOL")
         .empty()) {
    std::vector<std::string> evaluatedRPaths;
    for (auto const& rpath : this->InstallRPaths) {
      std::string result =
        cmGeneratorExpression::Evaluate(rpath, this->LocalGenerator, config);
      if (!result.empty()) {
        evaluatedRPaths.push_back(std::move(result));
      }
    }

    switch (this->Type) {
      case DependencyType::Library:
        this->GenerateAppleLibraryScript(os, config, evaluatedRPaths,
                                         indent.Next());
        break;
      case DependencyType::Framework:
        this->GenerateAppleFrameworkScript(os, config, evaluatedRPaths,
                                           indent.Next());
        break;
    }
  } else {
    std::string depVar = cmStrCat(this->TmpVarPrefix, "_dep");

    this->AddInstallRule(
      os, this->GetDestination(config), cmInstallType_SHARED_LIBRARY, {},
      false, this->Permissions.c_str(), nullptr, nullptr,
      " FOLLOW_SYMLINK_CHAIN", indent.Next(), depVar.c_str());

    if (this->LocalGenerator->GetMakefile()->GetSafeDefinition(
          "CMAKE_SYSTEM_NAME") == "Linux" &&
        !this->NoInstallRPath) {
      std::string evaluatedRPath;
      for (auto const& rpath : this->InstallRPaths) {
        std::string result =
          cmGeneratorExpression::Evaluate(rpath, this->LocalGenerator, config);
        if (!result.empty()) {
          if (evaluatedRPath.empty()) {
            evaluatedRPath = std::move(result);
          } else {
            evaluatedRPath += ':';
            evaluatedRPath += result;
          }
        }
      }

      os << indent.Next() << "get_filename_component(" << this->TmpVarPrefix
         << "_dep_name \"${" << this->TmpVarPrefix << "_dep}\" NAME)\n";
      if (evaluatedRPath.empty()) {
        os << indent.Next() << "file(RPATH_REMOVE FILE \""
           << GetDestDirPath(
                ConvertToAbsoluteDestination(this->GetDestination(config)))
           << "/${" << this->TmpVarPrefix << "_dep_name}\")\n";
      } else {
        os << indent.Next() << "file(RPATH_SET FILE \""
           << GetDestDirPath(
                ConvertToAbsoluteDestination(this->GetDestination(config)))
           << "/${" << this->TmpVarPrefix << "_dep_name}\" NEW_RPATH "
           << cmOutputConverter::EscapeForCMake(evaluatedRPath) << ")\n";
      }
    }
  }

  os << indent << "endforeach()\n";
}

void cmInstallRuntimeDependencySetGenerator::GenerateAppleLibraryScript(
  std::ostream& os, const std::string& config,
  const std::vector<std::string>& evaluatedRPaths, Indent indent)
{
  os << indent << "if(NOT " << this->TmpVarPrefix
     << "_dep MATCHES \"\\\\.framework/\")\n";

  auto depName = cmStrCat(this->TmpVarPrefix, "_dep");
  this->AddInstallRule(
    os, this->GetDestination(config), cmInstallType_SHARED_LIBRARY, {}, false,
    this->Permissions.c_str(), nullptr, nullptr, " FOLLOW_SYMLINK_CHAIN",
    indent.Next(), depName.c_str());

  os << indent.Next() << "get_filename_component(" << this->TmpVarPrefix
     << "_dep_name \"${" << this->TmpVarPrefix << "_dep}\" NAME)\n";
  auto depNameVar = cmStrCat("${", this->TmpVarPrefix, "_dep_name}");
  this->GenerateInstallNameFixup(os, config, evaluatedRPaths,
                                 cmStrCat("${", this->TmpVarPrefix, "_dep}"),
                                 depNameVar, indent.Next());

  os << indent << "endif()\n";
}

void cmInstallRuntimeDependencySetGenerator::GenerateAppleFrameworkScript(
  std::ostream& os, const std::string& config,
  const std::vector<std::string>& evaluatedRPaths, Indent indent)
{
  os << indent << "if(" << this->TmpVarPrefix
     << "_dep MATCHES \"^(.*/)?([^/]*\\\\.framework)/(.*)$\")\n"
     << indent.Next() << "set(" << this->TmpVarPrefix
     << "_dir \"${CMAKE_MATCH_1}\")\n"
     << indent.Next() << "set(" << this->TmpVarPrefix
     << "_name \"${CMAKE_MATCH_2}\")\n"
     << indent.Next() << "set(" << this->TmpVarPrefix
     << "_file \"${CMAKE_MATCH_3}\")\n"
     << indent.Next() << "set(" << this->TmpVarPrefix << "_path \"${"
     << this->TmpVarPrefix << "_dir}${" << this->TmpVarPrefix << "_name}\")\n";

  auto depName = cmStrCat(this->TmpVarPrefix, "_path");
  this->AddInstallRule(
    os, this->GetDestination(config), cmInstallType_DIRECTORY, {}, false,
    this->Permissions.c_str(), nullptr, nullptr, " USE_SOURCE_PERMISSIONS",
    indent.Next(), depName.c_str());

  auto depNameVar = cmStrCat("${", this->TmpVarPrefix, "_name}/${",
                             this->TmpVarPrefix, "_file}");
  this->GenerateInstallNameFixup(os, config, evaluatedRPaths,
                                 cmStrCat("${", this->TmpVarPrefix, "_dep}"),
                                 depNameVar, indent.Next());

  os << indent << "endif()\n";
}

void cmInstallRuntimeDependencySetGenerator::GenerateInstallNameFixup(
  std::ostream& os, const std::string& config,
  const std::vector<std::string>& evaluatedRPaths, const std::string& filename,
  const std::string& depName, Indent indent)
{
  if (!(this->NoInstallRPath && this->NoInstallName)) {
    auto indent2 = indent;
    if (evaluatedRPaths.empty() && this->NoInstallName) {
      indent2 = indent2.Next();
      os << indent << "if(" << this->RPathPrefix << "_" << filename << ")\n";
    }
    os << indent2 << "set(" << this->TmpVarPrefix << "_rpath_args)\n";
    if (!this->NoInstallRPath) {
      os << indent2 << "foreach(" << this->TmpVarPrefix << "_rpath IN LISTS "
         << this->RPathPrefix << '_' << filename << ")\n"
         << indent2.Next() << "list(APPEND " << this->TmpVarPrefix
         << "_rpath_args -delete_rpath \"${" << this->TmpVarPrefix
         << "_rpath}\")\n"
         << indent2 << "endforeach()\n";
    }
    os << indent2 << "execute_process(COMMAND \""
       << this->LocalGenerator->GetMakefile()->GetSafeDefinition(
            "CMAKE_INSTALL_NAME_TOOL")
       << "\" ${" << this->TmpVarPrefix << "_rpath_args}\n";
    if (!this->NoInstallRPath) {
      for (auto const& rpath : evaluatedRPaths) {
        os << indent2 << "  -add_rpath "
           << cmOutputConverter::EscapeForCMake(rpath) << "\n";
      }
    }
    if (!this->NoInstallName) {
      os << indent2 << "  -id \"${" << this->TmpVarPrefix
         << "_install_name_dir}" << depName << "\"\n";
    }
    os << indent2 << "  \""
       << GetDestDirPath(
            ConvertToAbsoluteDestination(this->GetDestination(config)))
       << "/" << depName << "\")\n";
    if (evaluatedRPaths.empty() && this->NoInstallName) {
      os << indent << "endif()\n";
    }
  }
}

void cmInstallRuntimeDependencySetGenerator::GenerateStripFixup(
  std::ostream& os, const std::string& config, const std::string& depName,
  Indent indent)
{
  std::string strip =
    this->LocalGenerator->GetMakefile()->GetSafeDefinition("CMAKE_STRIP");
  if (!strip.empty()) {
    os << indent << "if(CMAKE_INSTALL_DO_STRIP)\n"
       << indent.Next() << "execute_process(COMMAND \"" << strip << "\" ";
    if (this->LocalGenerator->GetMakefile()->GetSafeDefinition(
          "CMAKE_HOST_SYSTEM_NAME") == "Darwin") {
      os << "-x ";
    }
    os << "\""
       << GetDestDirPath(
            ConvertToAbsoluteDestination(this->GetDestination(config)))
       << "/" << depName << "\")\n"
       << indent << "endif()\n";
  }
}

std::string cmInstallRuntimeDependencySetGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(this->Destination,
                                         this->LocalGenerator, config);
}
