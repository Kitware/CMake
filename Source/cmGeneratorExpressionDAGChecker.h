/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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
  enum class ComputingLinkLibraries
  {
    No,
    Yes,
  };
  cmGeneratorExpressionDAGChecker(
    cmGeneratorTarget const* target, std::string property,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* parent, cmLocalGenerator const* contextLG,
    std::string const& contextConfig,
    cmListFileBacktrace backtrace = cmListFileBacktrace(),
    ComputingLinkLibraries computingLinkLibraries =
      ComputingLinkLibraries::No);

  enum Result
  {
    DAG,
    SELF_REFERENCE,
    CYCLIC_REFERENCE,
    ALREADY_SEEN
  };

  Result Check() const;

  void ReportError(cmGeneratorExpressionContext* context,
                   std::string const& expr);

  bool EvaluatingTransitiveProperty() const;
  bool EvaluatingGenexExpression() const;
  bool EvaluatingPICExpression() const;
  bool EvaluatingCompileExpression() const;
  bool EvaluatingLinkExpression() const;
  bool EvaluatingLinkOptionsExpression() const;
  bool EvaluatingLinkerLauncher() const;

  /** Returns true only when computing the actual link dependency
      graph for cmGeneratorTarget::GetLinkImplementationLibraries
      or cmGeneratorTarget::GetLinkInterfaceLibraries.  */
  bool IsComputingLinkLibraries() const;

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

  cmGeneratorExpressionDAGChecker const* const Parent;
  cmGeneratorExpressionDAGChecker const* const Top;
  cmGeneratorTarget const* Target;
  std::string const Property;
  mutable std::map<cmGeneratorTarget const*, std::set<std::string>> Seen;
  GeneratorExpressionContent const* const Content;
  cmListFileBacktrace const Backtrace;
  Result CheckResult;
  bool TransitivePropertiesOnly = false;
  bool CMP0131 = false;
  bool TopIsTransitiveProperty = false;
  ComputingLinkLibraries const ComputingLinkLibraries_ =
    ComputingLinkLibraries::No;
};
