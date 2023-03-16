/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmAlgorithms.h"
#include "cmPolicies.h"

class cmFileSet;
class cmGlobalGenerator;
class cmMakefile;
class cmTarget;
class cmTest;

namespace cmDebugger {
class cmDebuggerStackFrame;
class cmDebuggerVariables;
class cmDebuggerVariablesManager;
}

template <typename T>
class BT;

namespace cmDebugger {

class cmDebuggerVariablesHelper
{
  cmDebuggerVariablesHelper() = default;

public:
  static std::shared_ptr<cmDebuggerVariables> Create(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    cmPolicies::PolicyMap const& policyMap);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<std::pair<std::string, std::string>> const& list);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    cmBTStringRange const& entries);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::set<std::string> const& values);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<std::string> const& values);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<BT<std::string>> const& list);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType, cmFileSet* fileSet);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<cmFileSet*> const& fileSets);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType, cmTest* test);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<cmTest*> const& tests);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::vector<cmTarget*> const& targets);

  static std::shared_ptr<cmDebuggerVariables> Create(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    std::shared_ptr<cmDebuggerStackFrame> const& frame);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType, cmMakefile* mf);

  static std::shared_ptr<cmDebuggerVariables> CreateIfAny(
    std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
    std::string const& name, bool supportsVariableType,
    cmGlobalGenerator* gen);
};

} // namespace cmDebugger
