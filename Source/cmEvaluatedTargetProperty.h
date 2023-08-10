/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"

class cmLinkImplItem;
struct cmGeneratorExpressionDAGChecker;

// Represent a target property entry after evaluating generator expressions
// and splitting up lists.
struct EvaluatedTargetPropertyEntry
{
  EvaluatedTargetPropertyEntry(cmLinkImplItem const& item,
                               cmListFileBacktrace bt);

  // Move-only.
  EvaluatedTargetPropertyEntry(EvaluatedTargetPropertyEntry&&) = default;
  EvaluatedTargetPropertyEntry(EvaluatedTargetPropertyEntry const&) = delete;
  EvaluatedTargetPropertyEntry& operator=(EvaluatedTargetPropertyEntry&&) =
    delete;
  EvaluatedTargetPropertyEntry& operator=(
    EvaluatedTargetPropertyEntry const&) = delete;

  cmLinkImplItem const& LinkImplItem;
  cmListFileBacktrace Backtrace;
  std::vector<std::string> Values;
  bool ContextDependent = false;
};

EvaluatedTargetPropertyEntry EvaluateTargetPropertyEntry(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget::TargetPropertyEntry& entry);

struct EvaluatedTargetPropertyEntries
{
  std::vector<EvaluatedTargetPropertyEntry> Entries;
  bool HadContextSensitiveCondition = false;
};

EvaluatedTargetPropertyEntries EvaluateTargetPropertyEntries(
  cmGeneratorTarget const* thisTarget, std::string const& config,
  std::string const& lang, cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>> const&
    in);

// IncludeRuntimeInterface is used to break the cycle in computing
// the necessary transitive dependencies of targets that can occur
// now that we have implicit language runtime targets.
//
// To determine the set of languages that a target has we need to iterate
// all the sources which includes transitive INTERFACE sources.
// Therefore we can't determine what language runtimes are needed
// for a target until after all sources are computed.
//
// Therefore while computing the applicable INTERFACE_SOURCES we
// must ignore anything in LanguageRuntimeLibraries or we would
// create a cycle ( INTERFACE_SOURCES requires LanguageRuntimeLibraries,
// LanguageRuntimeLibraries requires INTERFACE_SOURCES).
//
enum class IncludeRuntimeInterface
{
  Yes,
  No
};

void AddInterfaceEntries(cmGeneratorTarget const* headTarget,
                         std::string const& config, std::string const& prop,
                         std::string const& lang,
                         cmGeneratorExpressionDAGChecker* dagChecker,
                         EvaluatedTargetPropertyEntries& entries,
                         IncludeRuntimeInterface searchRuntime,
                         cmGeneratorTarget::LinkInterfaceFor interfaceFor =
                           cmGeneratorTarget::LinkInterfaceFor::Usage);
