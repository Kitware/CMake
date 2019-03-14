/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionContext.h"

#include <utility>

cmGeneratorExpressionContext::cmGeneratorExpressionContext(
  cmLocalGenerator* lg, std::string config, bool quiet,
  cmGeneratorTarget const* headTarget, const cmGeneratorTarget* currentTarget,
  bool evaluateForBuildsystem, cmListFileBacktrace backtrace,
  std::string language)
  : Backtrace(std::move(backtrace))
  , LG(lg)
  , Config(std::move(config))
  , Language(std::move(language))
  , HeadTarget(headTarget)
  , CurrentTarget(currentTarget)
  , Quiet(quiet)
  , HadError(false)
  , HadContextSensitiveCondition(false)
  , HadHeadSensitiveCondition(false)
  , EvaluateForBuildsystem(evaluateForBuildsystem)
{
}
