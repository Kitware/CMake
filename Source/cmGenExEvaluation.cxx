/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGenExEvaluation.h"

#include <utility>

#include "cmGenExContext.h"

namespace cm {
namespace GenEx {

Evaluation::Evaluation(GenEx::Context context, bool quiet,
                       cmGeneratorTarget const* headTarget,
                       cmGeneratorTarget const* currentTarget,
                       bool evaluateForBuildsystem,
                       cmListFileBacktrace backtrace)
  : Context(std::move(context))
  , Backtrace(std::move(backtrace))
  , HeadTarget(headTarget)
  , CurrentTarget(currentTarget)
  , Quiet(quiet)
  , EvaluateForBuildsystem(evaluateForBuildsystem)
{
}

}
}
