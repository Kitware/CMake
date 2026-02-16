/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyEntry.h"

#include <string>
#include <utility>

#include <cm/memory>

#include "cmGeneratorExpression.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"

class cmake;

namespace cm {
cmLinkItem TargetPropertyEntry::NoLinkItem;

class TargetPropertyEntryString : public TargetPropertyEntry
{
public:
  TargetPropertyEntryString(BT<std::string> propertyValue,
                            cmLinkItem const& item = NoLinkItem)
    : TargetPropertyEntry(item)
    , PropertyValue(std::move(propertyValue))
  {
  }

  std::string const& Evaluate(cm::GenEx::Context const&,
                              cmGeneratorTarget const*,
                              cmGeneratorExpressionDAGChecker*) const override
  {
    return this->PropertyValue.Value;
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->PropertyValue.Backtrace;
  }
  std::string const& GetInput() const override
  {
    return this->PropertyValue.Value;
  }

private:
  BT<std::string> PropertyValue;
};

class TargetPropertyEntryGenex : public TargetPropertyEntry
{
public:
  TargetPropertyEntryGenex(std::unique_ptr<cmCompiledGeneratorExpression> cge,
                           cmLinkItem const& item = NoLinkItem)
    : TargetPropertyEntry(item)
    , ge(std::move(cge))
  {
  }

  std::string const& Evaluate(
    cm::GenEx::Context const& context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    return this->ge->Evaluate(context, dagChecker, headTarget);
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->ge->GetBacktrace();
  }

  std::string const& GetInput() const override { return this->ge->GetInput(); }

  bool GetHadContextSensitiveCondition() const override
  {
    return this->ge->GetHadContextSensitiveCondition();
  }

private:
  std::unique_ptr<cmCompiledGeneratorExpression> const ge;
};

std::unique_ptr<TargetPropertyEntry> TargetPropertyEntry::Create(
  cmake& cmakeInstance, const BT<std::string>& propertyValue,
  bool evaluateForBuildsystem)
{
  if (cmGeneratorExpression::Find(propertyValue.Value) != std::string::npos) {
    cmGeneratorExpression ge(cmakeInstance, propertyValue.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(propertyValue.Value);
    cge->SetEvaluateForBuildsystem(evaluateForBuildsystem);
    return std::unique_ptr<TargetPropertyEntry>(
      cm::make_unique<TargetPropertyEntryGenex>(std::move(cge)));
  }

  return std::unique_ptr<TargetPropertyEntry>(
    cm::make_unique<TargetPropertyEntryString>(propertyValue));
}

TargetPropertyEntry::TargetPropertyEntry(cmLinkItem const& item)
  : LinkItem(item)
{
}

bool TargetPropertyEntry::GetHadContextSensitiveCondition() const
{
  return false;
}
}
