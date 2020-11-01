/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
    bool inputIsContent, cmPolicies::PolicyStatus policyStatusCMP0070);

  void Generate(cmLocalGenerator* lg);

  std::vector<std::string> GetFiles() const { return this->Files; }

  void CreateOutputFile(cmLocalGenerator* lg, std::string const& config);

private:
  void Generate(cmLocalGenerator* lg, const std::string& config,
                const std::string& lang,
                cmCompiledGeneratorExpression* inputExpression,
                std::map<std::string, std::string>& outputFiles, mode_t perm);

  std::string GetInputFileName(cmLocalGenerator* lg);
  std::string GetOutputFileName(cmLocalGenerator* lg,
                                cmGeneratorTarget* target,
                                const std::string& config,
                                const std::string& lang);
  enum PathRole
  {
    PathForInput,
    PathForOutput
  };
  std::string FixRelativePath(std::string const& filePath, PathRole role,
                              cmLocalGenerator* lg);

private:
  const std::string Input;
  const std::string Target;
  const std::unique_ptr<cmCompiledGeneratorExpression> OutputFileExpr;
  const std::unique_ptr<cmCompiledGeneratorExpression> Condition;
  std::vector<std::string> Files;
  const bool InputIsContent;
  cmPolicies::PolicyStatus PolicyStatusCMP0070;
};
