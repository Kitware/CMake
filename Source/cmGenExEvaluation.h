/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <set>
#include <string>

#include "cmGenExContext.h"
#include "cmListFileCache.h"

class cmGeneratorTarget;

namespace cm {
namespace GenEx {

struct Evaluation final
{
  Evaluation(GenEx::Context context, bool quiet,
             cmGeneratorTarget const* headTarget,
             cmGeneratorTarget const* currentTarget,
             bool evaluateForBuildsystem, cmListFileBacktrace backtrace);

  GenEx::Context const Context;
  cmListFileBacktrace Backtrace;
  std::set<cmGeneratorTarget*> DependTargets;
  std::set<cmGeneratorTarget const*> AllTargets;
  std::set<std::string> SeenTargetProperties;
  std::set<cmGeneratorTarget const*> SourceSensitiveTargets;
  std::map<cmGeneratorTarget const*, std::map<std::string, std::string>>
    MaxLanguageStandard;
  // The target whose property is being evaluated.
  cmGeneratorTarget const* HeadTarget;
  // The dependent of HeadTarget which appears
  // directly or indirectly in the property.
  cmGeneratorTarget const* CurrentTarget;
  bool Quiet;
  bool HadError = false;
  bool HadContextSensitiveCondition = false;
  bool HadHeadSensitiveCondition = false;
  bool HadLinkLanguageSensitiveCondition = false;
  bool EvaluateForBuildsystem;
};

}
}
