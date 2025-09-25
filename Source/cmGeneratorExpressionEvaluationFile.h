/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cm_sys_stat.h"

#include "cmGeneratorExpression.h"
#include "cmPolicies.h"

class cmGeneratorTarget;
class cmLocalGenerator;

class cmGeneratorExpressionEvaluationFile
{
public:
  cmGeneratorExpressionEvaluationFile(
    std::string input, std::string target,
    std::unique_ptr<cmCompiledGeneratorExpression> outputFileExpr,
    std::unique_ptr<cmCompiledGeneratorExpression> condition,
    bool inputIsContent, std::string newLineCharacter, mode_t permissions,
    cmPolicies::PolicyStatus policyStatusCMP0070);

  void Generate(cmLocalGenerator* lg);

  std::vector<std::string> GetFiles() const { return this->Files; }

  void CreateOutputFile(cmLocalGenerator* lg, std::string const& config);

private:
  void Generate(cmLocalGenerator* lg, std::string const& config,
                std::string const& lang,
                cmCompiledGeneratorExpression* inputExpression,
                std::map<std::string, std::string>& outputFiles, mode_t perm);

  std::string GetInputFileName(cmLocalGenerator* lg);
  std::string GetOutputFileName(cmLocalGenerator* lg,
                                cmGeneratorTarget* target,
                                std::string const& config,
                                std::string const& lang);
  enum PathRole
  {
    PathForInput,
    PathForOutput
  };
  std::string FixRelativePath(std::string const& filePath, PathRole role,
                              cmLocalGenerator* lg);

  std::string const Input;
  std::string const Target;
  std::unique_ptr<cmCompiledGeneratorExpression> const OutputFileExpr;
  std::unique_ptr<cmCompiledGeneratorExpression> const Condition;
  std::vector<std::string> Files;
  bool const InputIsContent;
  std::string const NewLineCharacter;
  cmPolicies::PolicyStatus PolicyStatusCMP0070;
  mode_t Permissions;
};
