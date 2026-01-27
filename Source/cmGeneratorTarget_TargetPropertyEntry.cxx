/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"

class cmake;

cmLinkItem cmGeneratorTarget::TargetPropertyEntry::NoLinkItem;

class TargetPropertyEntryString : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryString(BT<std::string> propertyValue,
                            cmLinkItem const& item = NoLinkItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
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

class TargetPropertyEntryGenex : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryGenex(std::unique_ptr<cmCompiledGeneratorExpression> cge,
                           cmLinkItem const& item = NoLinkItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
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

class TargetPropertyEntryFileSet
  : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryFileSet(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
    cmFileSet const* fileSet, cmLinkItem const& item = NoLinkItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , BaseDirs(std::move(dirs))
    , ContextSensitiveDirs(contextSensitiveDirs)
    , EntryCge(std::move(entryCge))
    , FileSet(fileSet)
  {
  }

  std::string const& Evaluate(
    cm::GenEx::Context const& context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    std::map<std::string, std::vector<std::string>> filesPerDir;
    this->FileSet->EvaluateFileEntry(this->BaseDirs, filesPerDir,
                                     this->EntryCge, context, headTarget,
                                     dagChecker);

    std::vector<std::string> files;
    for (auto const& it : filesPerDir) {
      files.insert(files.end(), it.second.begin(), it.second.end());
    }

    static std::string filesStr;
    filesStr = cmList::to_string(files);
    return filesStr;
  }

  cmListFileBacktrace GetBacktrace() const override
  {
    return this->EntryCge->GetBacktrace();
  }

  std::string const& GetInput() const override
  {
    return this->EntryCge->GetInput();
  }

  bool GetHadContextSensitiveCondition() const override
  {
    return this->ContextSensitiveDirs ||
      this->EntryCge->GetHadContextSensitiveCondition();
  }

private:
  std::vector<std::string> const BaseDirs;
  bool const ContextSensitiveDirs;
  std::unique_ptr<cmCompiledGeneratorExpression> const EntryCge;
  cmFileSet const* FileSet;
};

std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>
cmGeneratorTarget::TargetPropertyEntry::Create(
  cmake& cmakeInstance, const BT<std::string>& propertyValue,
  bool evaluateForBuildsystem)
{
  if (cmGeneratorExpression::Find(propertyValue.Value) != std::string::npos) {
    cmGeneratorExpression ge(cmakeInstance, propertyValue.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(propertyValue.Value);
    cge->SetEvaluateForBuildsystem(evaluateForBuildsystem);
    return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
      cm::make_unique<TargetPropertyEntryGenex>(std::move(cge)));
  }

  return std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>(
    cm::make_unique<TargetPropertyEntryString>(propertyValue));
}

std::unique_ptr<cmGeneratorTarget::TargetPropertyEntry>
cmGeneratorTarget::TargetPropertyEntry::CreateFileSet(
  std::vector<std::string> dirs, bool contextSensitiveDirs,
  std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
  cmFileSet const* fileSet, cmLinkItem const& item)
{
  return cm::make_unique<TargetPropertyEntryFileSet>(
    std::move(dirs), contextSensitiveDirs, std::move(entryCge), fileSet, item);
}

cmGeneratorTarget::TargetPropertyEntry::TargetPropertyEntry(
  cmLinkItem const& item)
  : LinkItem(item)
{
}

bool cmGeneratorTarget::TargetPropertyEntry::GetHadContextSensitiveCondition()
  const
{
  return false;
}
