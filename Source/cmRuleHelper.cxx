/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmRuleHelper.h"

#include <utility>

#include <cmext/algorithm>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"

namespace cm {
class RulePatternDAGChecker
{
public:
  RulePatternDAGChecker(std::string pattern, RulePatternDAGChecker* parent)
    : Parent(parent)
    , Top(parent ? parent->Top : this)
    , Pattern(std::move(pattern))
  {
    this->CheckResult = this->CheckGraph();
  }

  enum Result
  {
    DAG,
    SELF_REFERENCE,
    CYCLIC_REFERENCE
  };

  Result Check() const { return this->CheckResult; }

  void ReportError(cmMakefile& makefile, std ::string const& rule,
                   std::string const& expression)
  {
    if (this->CheckResult == DAG) {
      return;
    }

    RulePatternDAGChecker const* parent = this->Parent;

    if (parent && !parent->Parent) {
      makefile.IssueMessage(MessageType::FATAL_ERROR,
                            cmStrCat("Error evaluating the pattern \"",
                                     this->Pattern,
                                     "\" in the expression:\n"
                                     "  ",
                                     expression,
                                     "\n"
                                     "Self reference on rule \"",
                                     rule, "\".\n"));
      return;
    }

    {
      makefile.IssueMessage(MessageType::FATAL_ERROR,
                            cmStrCat("Error evaluating the pattern \"",
                                     this->Pattern,
                                     "\" in the expression:\n"
                                     "  ",
                                     expression,
                                     "\n"
                                     "Dependency loop found."));
    }
  }

private:
  Result CheckGraph() const
  {
    RulePatternDAGChecker const* parent = this->Parent;
    while (parent) {
      if (this->Pattern == parent->Pattern) {
        return (parent == this->Parent) ? SELF_REFERENCE : CYCLIC_REFERENCE;
      }
      parent = parent->Parent;
    }
    return DAG;
  }

  RulePatternDAGChecker const* const Parent;
  RulePatternDAGChecker const* const Top;
  std::string const Pattern;
  Result CheckResult;
};

std::string RulePatternPlaceholderExpander::ExpandVariables(
  std::string const& string)
{
  std::string value{ string };
  return cmPlaceholderExpander::ExpandVariables(
    value, cmPlaceholderExpander::HandleGenex::Yes);
}

std::string RulePatternPlaceholderExpander::ExpandVariable(
  std::string const& variable)
{
  if (cm::contains(this->Patterns, variable)) {
    RulePatternDAGChecker dagChecker{
      variable, this->DAGStack.empty() ? nullptr : this->DAGStack.top()
    };

    switch (dagChecker.Check()) {
      case RulePatternDAGChecker::SELF_REFERENCE:
      case RulePatternDAGChecker::CYCLIC_REFERENCE: {
        dagChecker.ReportError(this->Makefile, this->Rule, this->Expression);
        this->InError = true;
        return cmStrCat('<', variable, '>');
      }
      case RulePatternDAGChecker::DAG:
        break;
    }

    this->DAGStack.push(&dagChecker);
    auto result = this->ExpandVariables(this->Patterns.at(variable));
    this->DAGStack.pop();

    return result;
  }

  this->InError = true;
  this->Makefile.IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat("The pattern \"", variable, "\" is not defined."));
  // If there is no variable defined, mark unresolved variable by '<' and
  // '>'
  return cmStrCat('<', variable, '>');
}
}
