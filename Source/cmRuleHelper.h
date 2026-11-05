/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <stack>
#include <string>
#include <unordered_map>

#include "cmPlaceholderExpander.h"

class cmMakefile;

namespace cm {
using RulePatternSet = std::unordered_map<std::string, std::string>;

class RulePatternDAGChecker;

class RulePatternPlaceholderExpander : public cmPlaceholderExpander
{
public:
  RulePatternPlaceholderExpander(cmMakefile& makefile, std::string const& rule,
                                 RulePatternSet const& patterns)
    : Makefile(makefile)
    , Rule(rule)
    , Patterns(patterns)
  {
  }

  std::string ExpandPatterns(std::string const& string)
  {
    this->Expression = string;
    this->InError = false;
    return this->ExpandVariables(string);
  }

  std::string ExpandVariables(std::string const& string);

  bool InError = false;

private:
  cmMakefile& Makefile;
  std::string const& Rule;
  std::string Expression;
  RulePatternSet const& Patterns;
  std::stack<RulePatternDAGChecker*> DAGStack;

  std::string ExpandVariable(std::string const& variable) override;
};
}
