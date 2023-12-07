/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallCxxModuleBmiGenerator.h"

#include <ostream>
#include <utility>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmOutputConverter.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"

cmInstallCxxModuleBmiGenerator::cmInstallCxxModuleBmiGenerator(
  std::string target, std::string const& dest, std::string file_permissions,
  std::vector<std::string> const& configurations, std::string const& component,
  MessageLevel message, bool exclude_from_all, bool optional,
  cmListFileBacktrace backtrace)
  : cmInstallGenerator(dest, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , TargetName(std::move(target))
  , FilePermissions(std::move(file_permissions))
  , Optional(optional)
{
  this->ActionsPerConfig = true;
}

cmInstallCxxModuleBmiGenerator::~cmInstallCxxModuleBmiGenerator() = default;

bool cmInstallCxxModuleBmiGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;

  this->Target = lg->FindLocalNonAliasGeneratorTarget(this->TargetName);
  if (!this->Target) {
    // If no local target has been found, find it in the global scope.
    this->Target =
      lg->GetGlobalGenerator()->FindGeneratorTarget(this->TargetName);
  }

  return true;
}

std::string cmInstallCxxModuleBmiGenerator::GetScriptLocation(
  std::string const& config) const
{
  char const* config_name = config.c_str();
  if (config.empty()) {
    config_name = "noconfig";
  }
  return cmStrCat(this->Target->GetSupportDirectory(),
                  "/install-cxx-module-bmi-", config_name, ".cmake");
}

std::string cmInstallCxxModuleBmiGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(this->Destination,
                                         this->LocalGenerator, config);
}

void cmInstallCxxModuleBmiGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  auto const& loc = this->GetScriptLocation(config);
  if (loc.empty()) {
    return;
  }
  os << indent << "include(\""
     << cmOutputConverter::EscapeForCMake(
          loc, cmOutputConverter::WrapQuotes::NoWrap)
     << "\" OPTIONAL)\n";
}
