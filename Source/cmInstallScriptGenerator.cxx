/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallScriptGenerator.h"

#include "cmGeneratorExpression.h"
#include "cmScriptGenerator.h"

#include <ostream>
#include <vector>

cmInstallScriptGenerator::cmInstallScriptGenerator(const char* script,
                                                   bool code,
                                                   const char* component,
                                                   bool exclude_from_all)
  : cmInstallGenerator(nullptr, std::vector<std::string>(), component,
                       MessageDefault, exclude_from_all)
  , Script(script)
  , Code(code)
{
  // We need per-config actions if the script has generator expressions.
  if (cmGeneratorExpression::Find(Script) != std::string::npos) {
    this->ActionsPerConfig = true;
  }
}

cmInstallScriptGenerator::~cmInstallScriptGenerator()
{
}

void cmInstallScriptGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
}

void cmInstallScriptGenerator::AddScriptInstallRule(std::ostream& os,
                                                    Indent indent,
                                                    std::string const& script)
{
  if (this->Code) {
    os << indent.Next() << script << "\n";
  } else {
    os << indent.Next() << "include(\"" << script << "\")\n";
  }
}

void cmInstallScriptGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent indent)
{
  if (this->ActionsPerConfig) {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
  } else {
    this->AddScriptInstallRule(os, indent, this->Script);
  }
}

void cmInstallScriptGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  cmGeneratorExpression ge;
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(this->Script);
  this->AddScriptInstallRule(os, indent,
                             cge->Evaluate(this->LocalGenerator, config));
}
