/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEvaluatedTargetProperty.h"

#include <unordered_map>
#include <utility>

#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorTarget.h"
#include "cmLinkItem.h"
#include "cmList.h"

struct cmGeneratorExpressionDAGChecker;

EvaluatedTargetPropertyEntry::EvaluatedTargetPropertyEntry(
  cmLinkImplItem const& item, cmListFileBacktrace bt)
  : LinkImplItem(item)
  , Backtrace(std::move(bt))
{
}

EvaluatedTargetPropertyEntry EvaluateTargetPropertyEntry(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget::TargetPropertyEntry& entry)
{
  EvaluatedTargetPropertyEntry ee(entry.LinkImplItem, entry.GetBacktrace());
  cmExpandList(entry.Evaluate(thisTarget->GetLocalGenerator(), config,
                              thisTarget, dagChecker, lang),
               ee.Values);
  if (entry.GetHadContextSensitiveCondition()) {
    ee.ContextDependent = true;
  }
  return ee;
}

EvaluatedTargetPropertyEntries EvaluateTargetPropertyEntries(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>> const&
    in)
{
  EvaluatedTargetPropertyEntries out;
  out.Entries.reserve(in.size());
  for (auto const& entry : in) {
    out.Entries.emplace_back(EvaluateTargetPropertyEntry(
      thisTarget, config, lang, dagChecker, *entry));
  }
  return out;
}

namespace {
void addInterfaceEntry(cmGeneratorTarget const* headTarget,
                       std::string const& config, std::string const& prop,
                       std::string const& lang,
                       cmGeneratorExpressionDAGChecker* dagChecker,
                       EvaluatedTargetPropertyEntries& entries,
                       cmGeneratorTarget::LinkInterfaceFor interfaceFor,
                       std::vector<cmLinkImplItem> const& libraries)
{
  for (cmLinkImplItem const& lib : libraries) {
    if (lib.Target) {
      EvaluatedTargetPropertyEntry ee(lib, lib.Backtrace);
      // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in our
      // caller's property and hand-evaluate it as if it were compiled.
      // Create a context as cmCompiledGeneratorExpression::Evaluate does.
      cmGeneratorExpressionContext context(
        headTarget->GetLocalGenerator(), config, false, headTarget, headTarget,
        true, lib.Backtrace, lang);
      cmExpandList(lib.Target->EvaluateInterfaceProperty(
                     prop, &context, dagChecker, interfaceFor),
                   ee.Values);
      ee.ContextDependent = context.HadContextSensitiveCondition;
      entries.Entries.emplace_back(std::move(ee));
    }
  }
}
}

void AddInterfaceEntries(cmGeneratorTarget const* headTarget,
                         std::string const& config, std::string const& prop,
                         std::string const& lang,
                         cmGeneratorExpressionDAGChecker* dagChecker,
                         EvaluatedTargetPropertyEntries& entries,
                         IncludeRuntimeInterface searchRuntime,
                         cmGeneratorTarget::LinkInterfaceFor interfaceFor)
{
  if (searchRuntime == IncludeRuntimeInterface::Yes) {
    if (cmLinkImplementation const* impl =
          headTarget->GetLinkImplementation(config, interfaceFor)) {
      entries.HadContextSensitiveCondition =
        impl->HadContextSensitiveCondition;

      auto runtimeLibIt = impl->LanguageRuntimeLibraries.find(lang);
      if (runtimeLibIt != impl->LanguageRuntimeLibraries.end()) {
        addInterfaceEntry(headTarget, config, prop, lang, dagChecker, entries,
                          interfaceFor, runtimeLibIt->second);
      }
      addInterfaceEntry(headTarget, config, prop, lang, dagChecker, entries,
                        interfaceFor, impl->Libraries);
    }
  } else {
    if (cmLinkImplementationLibraries const* impl =
          headTarget->GetLinkImplementationLibraries(config, interfaceFor)) {
      entries.HadContextSensitiveCondition =
        impl->HadContextSensitiveCondition;
      addInterfaceEntry(headTarget, config, prop, lang, dagChecker, entries,
                        interfaceFor, impl->Libraries);
    }
  }
}
