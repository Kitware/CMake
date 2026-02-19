/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <string>

#include "cmListFileCache.h"

class cmLinkItem;
class cmake;
class cmGeneratorTarget;
class cmFileSet;
struct cmGeneratorExpressionDAGChecker;
class cmCompiledGeneratorExpression;

namespace cm {
namespace GenEx {
struct Context;
}

class TargetPropertyEntry
{
protected:
  static cmLinkItem NoLinkItem;

public:
  TargetPropertyEntry(cmLinkItem const& item);
  virtual ~TargetPropertyEntry() = default;

  static std::unique_ptr<TargetPropertyEntry> Create(
    cmake& cmakeInstance, const BT<std::string>& propertyValue,
    bool evaluateForBuildsystem = false);

  static std::unique_ptr<TargetPropertyEntry> CreateFileSet(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
    cmFileSet const* fileSet, cmLinkItem const& item = NoLinkItem);

  virtual std::string const& Evaluate(
    cm::GenEx::Context const& context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker) const = 0;

  virtual cmListFileBacktrace GetBacktrace() const = 0;
  virtual std::string const& GetInput() const = 0;
  virtual bool GetHadContextSensitiveCondition() const;

  cmLinkItem const& LinkItem;
};
}
