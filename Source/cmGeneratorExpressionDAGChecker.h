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

#define CM_SELECT_BOTH(F, A1, A2) F(A1, A2)
#define CM_SELECT_FIRST(F, A1, A2) F(A1)
#define CM_SELECT_SECOND(F, A1, A2) F(A2)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, SELECT)                       \
  SELECT(F, EvaluatingIncludeDirectories, INCLUDE_DIRECTORIES)                \
  SELECT(F, EvaluatingSystemIncludeDirectories, SYSTEM_INCLUDE_DIRECTORIES)   \
  SELECT(F, EvaluatingCompileDefinitions, COMPILE_DEFINITIONS)                \
  SELECT(F, EvaluatingCompileOptions, COMPILE_OPTIONS)                        \
  SELECT(F, EvaluatingAutoUicOptions, AUTOUIC_OPTIONS)                        \
  SELECT(F, EvaluatingSources, SOURCES)                                       \
  SELECT(F, EvaluatingCompileFeatures, COMPILE_FEATURES)                      \
  SELECT(F, EvaluatingLinkOptions, LINK_OPTIONS)                              \
  SELECT(F, EvaluatingLinkDirectories, LINK_DIRECTORIES)                      \
  SELECT(F, EvaluatingLinkDepends, LINK_DEPENDS)                              \
  SELECT(F, EvaluatingPrecompileHeaders, PRECOMPILE_HEADERS)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY(F)                                    \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_BOTH)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(F)                             \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_FIRST)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(F)                               \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_SECOND)

struct cmGeneratorExpressionDAGChecker
{
  cmGeneratorExpressionDAGChecker(cmListFileBacktrace backtrace,
                                  cmGeneratorTarget const* target,
                                  std::string property,
                                  const GeneratorExpressionContent* content,
                                  cmGeneratorExpressionDAGChecker* parent);
  cmGeneratorExpressionDAGChecker(cmGeneratorTarget const* target,
                                  std::string property,
                                  const GeneratorExpressionContent* content,
                                  cmGeneratorExpressionDAGChecker* parent);

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

  bool EvaluatingGenexExpression() const;
  bool EvaluatingPICExpression() const;
  bool EvaluatingCompileExpression() const;
  bool EvaluatingLinkExpression() const;
  bool EvaluatingLinkOptionsExpression() const;

  bool EvaluatingLinkLibraries(cmGeneratorTarget const* tgt = nullptr) const;

#define DECLARE_TRANSITIVE_PROPERTY_METHOD(METHOD) bool METHOD() const;

  CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(DECLARE_TRANSITIVE_PROPERTY_METHOD)

#undef DECLARE_TRANSITIVE_PROPERTY_METHOD

  bool GetTransitivePropertiesOnly() const;
  void SetTransitivePropertiesOnly() { this->TransitivePropertiesOnly = true; }

  cmGeneratorExpressionDAGChecker const* Top() const;
  cmGeneratorTarget const* TopTarget() const;

private:
  Result CheckGraph() const;
  void Initialize();

private:
  const cmGeneratorExpressionDAGChecker* const Parent;
  cmGeneratorTarget const* Target;
  const std::string Property;
  mutable std::map<cmGeneratorTarget const*, std::set<std::string>> Seen;
  const GeneratorExpressionContent* const Content;
  const cmListFileBacktrace Backtrace;
  Result CheckResult;
  bool TransitivePropertiesOnly;
};
