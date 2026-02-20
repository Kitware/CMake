/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmEvaluatedTargetProperty.h"

#include <unordered_map>
#include <utility>

#include "cmGenExContext.h"
#include "cmGenExEvaluation.h"
#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmTargetPropertyEntry.h"

struct cmGeneratorExpressionDAGChecker;

EvaluatedTargetPropertyEntry::EvaluatedTargetPropertyEntry(
  cmLinkItem const& item, cmListFileBacktrace bt)
  : LinkItem(item)
  , Backtrace(std::move(bt))
{
}

EvaluatedTargetPropertyEntry EvaluateTargetPropertyEntry(
  cmGeneratorTarget const* thisTarget, cm::GenEx::Context const& context,
  cmGeneratorExpressionDAGChecker* dagChecker, cm::TargetPropertyEntry& entry)
{
  EvaluatedTargetPropertyEntry ee(entry.LinkItem, entry.GetBacktrace());
  cmExpandList(entry.Evaluate(context, thisTarget, dagChecker), ee.Values);
  if (entry.GetHadContextSensitiveCondition()) {
    ee.ContextDependent = true;
  }
  return ee;
}

EvaluatedTargetPropertyEntries EvaluateTargetPropertyEntries(
  cmGeneratorTarget const* thisTarget, cm::GenEx::Context const& context,
  cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::unique_ptr<cm::TargetPropertyEntry>> const& in)
{
  EvaluatedTargetPropertyEntries out;
  out.Entries.reserve(in.size());
  for (auto const& entry : in) {
    out.Entries.emplace_back(
      EvaluateTargetPropertyEntry(thisTarget, context, dagChecker, *entry));
  }
  return out;
}

namespace {
void addInterfaceEntry(cmGeneratorTarget const* headTarget,
                       std::string const& prop,
                       cm::GenEx::Context const& context,
                       cmGeneratorExpressionDAGChecker* dagChecker,
                       EvaluatedTargetPropertyEntries& entries,
                       cmGeneratorTarget::UseTo usage,
                       std::vector<cmLinkItem> const& libraries)
{
  for (cmLinkItem const& lib : libraries) {
    if (lib.Target) {
      EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
      // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in our
      // caller's property and hand-evaluate it as if it were compiled.
      // Create a context as cmCompiledGeneratorExpression::Evaluate does.
      cm::GenEx::Evaluation eval(context, false, headTarget, headTarget, true,
                                 lib.Backtrace);
      cmExpandList(
        lib.Target->EvaluateInterfaceProperty(prop, &eval, dagChecker, usage),
        ee.Values);
      ee.ContextDependent = eval.HadContextSensitiveCondition;
      entries.Entries.emplace_back(std::move(ee));
    }
  }
}
}

void AddInterfaceEntries(cmGeneratorTarget const* headTarget,
                         std::string const& prop,
                         cm::GenEx::Context const& context,
                         cmGeneratorExpressionDAGChecker* dagChecker,
                         EvaluatedTargetPropertyEntries& entries,
                         IncludeRuntimeInterface searchRuntime,
                         cmGeneratorTarget::UseTo usage)
{
  if (searchRuntime == IncludeRuntimeInterface::Yes) {
    if (cmLinkImplementation const* impl =
          headTarget->GetLinkImplementation(context.Config, usage)) {
      entries.HadContextSensitiveCondition =
        impl->HadContextSensitiveCondition;

      auto runtimeLibIt =
        impl->LanguageRuntimeLibraries.find(context.Language);
      if (runtimeLibIt != impl->LanguageRuntimeLibraries.end()) {
        addInterfaceEntry(headTarget, prop, context, dagChecker, entries,
                          usage, runtimeLibIt->second);
      }
      addInterfaceEntry(headTarget, prop, context, dagChecker, entries, usage,
                        impl->Libraries);
    }
  } else {
    if (cmLinkImplementationLibraries const* impl =
          headTarget->GetLinkImplementationLibraries(context.Config, usage)) {
      entries.HadContextSensitiveCondition =
        impl->HadContextSensitiveCondition;
      addInterfaceEntry(headTarget, prop, context, dagChecker, entries, usage,
                        impl->Libraries);
    }
  }
}
