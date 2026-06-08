/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallCxxModuleBmiGenerator.h"

#include <ostream>
#include <utility>

#include "cmDiagnosticContext.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"

cmInstallCxxModuleBmiGenerator::cmInstallCxxModuleBmiGenerator(
  std::string target, std::string const& dest, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string const& component,
  MessageLevel message, bool excludeFromAll, bool optional,
  cmDiagnosticContext context)
  : cmInstallGenerator(dest, configurations, component, message,
                       excludeFromAll, false, std::move(context))
  , TargetName(std::move(target))
  , FilePermissions(std::move(filePermissions))
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
  char const* configName = config.c_str();
  if (config.empty()) {
    configName = "noconfig";
  }

  return cmStrCat(this->Target->GetCMFSupportDirectory(),
                  "/install-cxx-module-bmi-", configName, ".cmake");
}

std::string cmInstallCxxModuleBmiGenerator::GetDestination(
  std::string const& config) const
{
  std::string dest = cmGeneratorExpression::Evaluate(
    this->Destination, this->LocalGenerator, config);
  this->CheckAbsoluteDestination(dest, this->LocalGenerator);
  return dest;
}

void cmInstallCxxModuleBmiGenerator::GenerateScriptForConfig(
  std::ostream& os, std::string const& config, Indent indent)
{
  auto const& loc = this->GetScriptLocation(config);
  if (loc.empty()) {
    return;
  }
  os << indent << "include(" << cmScriptGenerator::Quote(loc)
     << " OPTIONAL)\n";
}
