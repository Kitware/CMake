/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>

#include "cmListFileCache.h"

struct GeneratorExpressionContent;
struct cmGeneratorExpressionContext;
class cmGeneratorTarget;
class cmLocalGenerator;

struct cmGeneratorExpressionDAGChecker
{
  cmGeneratorExpressionDAGChecker(cmListFileBacktrace backtrace,
                                  cmGeneratorTarget const* target,
                                  std::string property,
                                  const GeneratorExpressionContent* content,
                                  cmGeneratorExpressionDAGChecker* parent,
                                  cmLocalGenerator const* contextLG);
  cmGeneratorExpressionDAGChecker(cmGeneratorTarget const* target,
                                  std::string property,
                                  const GeneratorExpressionContent* content,
                                  cmGeneratorExpressionDAGChecker* parent,
                                  cmLocalGenerator const* contextLG);

  enum Result
  {
    DAG,
    SELF_REFERENCE,
    CYCLIC_REFERENCE,
    ALREADY_SEEN
  };

  Result Check() const;

  void ReportError(cmGeneratorExpressionContext* context,
                   const std::string& expr);

  bool EvaluatingTransitiveProperty() const;
  bool EvaluatingGenexExpression() const;
  bool EvaluatingPICExpression() const;
  bool EvaluatingCompileExpression() const;
  bool EvaluatingLinkExpression() const;
  bool EvaluatingLinkOptionsExpression() const;
  bool EvaluatingLinkerLauncher() const;

  enum class ForGenex
  {
    ANY,
    LINK_LIBRARY,
    LINK_GROUP
  };
  bool EvaluatingLinkLibraries(cmGeneratorTarget const* tgt = nullptr,
                               ForGenex genex = ForGenex::ANY) const;

  bool EvaluatingSources() const;

  bool GetTransitivePropertiesOnly() const;
  void SetTransitivePropertiesOnly() { this->TransitivePropertiesOnly = true; }

  bool GetTransitivePropertiesOnlyCMP0131() const;
  void SetTransitivePropertiesOnlyCMP0131() { this->CMP0131 = true; }

  cmGeneratorTarget const* TopTarget() const;

private:
  Result CheckGraph() const;

  const cmGeneratorExpressionDAGChecker* const Parent;
  const cmGeneratorExpressionDAGChecker* const Top;
  cmGeneratorTarget const* Target;
  const std::string Property;
  mutable std::map<cmGeneratorTarget const*, std::set<std::string>> Seen;
  const GeneratorExpressionContent* const Content;
  const cmListFileBacktrace Backtrace;
  Result CheckResult;
  bool TransitivePropertiesOnly = false;
  bool CMP0131 = false;
  bool TopIsTransitiveProperty = false;
};
