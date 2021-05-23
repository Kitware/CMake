/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallScriptGenerator.h"

#include <ostream>
#include <utility>
#include <vector>

#include "cmGeneratorExpression.h"
#include "cmLocalGenerator.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmScriptGenerator.h"

cmInstallScriptGenerator::cmInstallScriptGenerator(
  std::string script, bool code, std::string const& component,
  bool exclude_from_all, bool all_components, cmListFileBacktrace backtrace)
  : cmInstallGenerator("", std::vector<std::string>(), component,
                       MessageDefault, exclude_from_all, all_components,
                       std::move(backtrace))
  , Script(std::move(script))
  , Code(code)
  , AllowGenex(false)
{
  // We need per-config actions if the script has generator expressions.
  if (cmGeneratorExpression::Find(this->Script) != std::string::npos) {
    this->ActionsPerConfig = true;
  }
}

cmInstallScriptGenerator::~cmInstallScriptGenerator() = default;

bool cmInstallScriptGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;

  if (this->ActionsPerConfig) {
    switch (this->LocalGenerator->GetPolicyStatus(cmPolicies::CMP0087)) {
      case cmPolicies::WARN:
        this->LocalGenerator->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0087));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
        this->AllowGenex = true;
        break;
    }
  }

  return true;
}

std::string cmInstallScriptGenerator::GetScript(
  std::string const& config) const
{
  std::string script;
  if (this->AllowGenex && this->ActionsPerConfig) {
    script = cmGeneratorExpression::Evaluate(this->Script,
                                             this->LocalGenerator, config);
  } else {
    script = this->Script;
  }
  return script;
}

void cmInstallScriptGenerator::AddScriptInstallRule(
  std::ostream& os, Indent indent, std::string const& script) const
{
  if (this->Code) {
    os << indent << script << "\n";
  } else {
    os << indent << "include(\"" << script << "\")\n";
  }
}

void cmInstallScriptGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent indent)
{
  if (this->AllowGenex && this->ActionsPerConfig) {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
  } else {
    this->AddScriptInstallRule(os, indent, this->Script);
  }
}

void cmInstallScriptGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  this->AddScriptInstallRule(os, indent, this->GetScript(config));
}
