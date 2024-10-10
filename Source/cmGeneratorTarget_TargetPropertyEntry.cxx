/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

class cmLocalGenerator;
class cmake;
struct cmGeneratorExpressionDAGChecker;

cmLinkImplItem cmGeneratorTarget::TargetPropertyEntry::NoLinkImplItem;

class TargetPropertyEntryString : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryString(BT<std::string> propertyValue,
                            cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , PropertyValue(std::move(propertyValue))
  {
  }

  const std::string& Evaluate(cmLocalGenerator*, const std::string&,
                              cmGeneratorTarget const*,
                              cmGeneratorExpressionDAGChecker*,
                              std::string const&) const override
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
                           cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , ge(std::move(cge))
  {
  }

  const std::string& Evaluate(cmLocalGenerator* lg, const std::string& config,
                              cmGeneratorTarget const* headTarget,
                              cmGeneratorExpressionDAGChecker* dagChecker,
                              std::string const& language) const override
  {
    return this->ge->Evaluate(lg, config, headTarget, dagChecker, nullptr,
                              language);
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
  const std::unique_ptr<cmCompiledGeneratorExpression> ge;
};

class TargetPropertyEntryFileSet
  : public cmGeneratorTarget::TargetPropertyEntry
{
public:
  TargetPropertyEntryFileSet(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
    const cmFileSet* fileSet, cmLinkImplItem const& item = NoLinkImplItem)
    : cmGeneratorTarget::TargetPropertyEntry(item)
    , BaseDirs(std::move(dirs))
    , ContextSensitiveDirs(contextSensitiveDirs)
    , EntryCge(std::move(entryCge))
    , FileSet(fileSet)
  {
  }

  const std::string& Evaluate(cmLocalGenerator* lg, const std::string& config,
                              cmGeneratorTarget const* headTarget,
                              cmGeneratorExpressionDAGChecker* dagChecker,
                              std::string const& /*lang*/) const override
  {
    std::map<std::string, std::vector<std::string>> filesPerDir;
    this->FileSet->EvaluateFileEntry(this->BaseDirs, filesPerDir,
                                     this->EntryCge, lg, config, headTarget,
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
  const std::vector<std::string> BaseDirs;
  const bool ContextSensitiveDirs;
  const std::unique_ptr<cmCompiledGeneratorExpression> EntryCge;
  const cmFileSet* FileSet;
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
  const cmFileSet* fileSet, cmLinkImplItem const& item)
{
  return cm::make_unique<TargetPropertyEntryFileSet>(
    std::move(dirs), contextSensitiveDirs, std::move(entryCge), fileSet, item);
}

cmGeneratorTarget::TargetPropertyEntry::TargetPropertyEntry(
  cmLinkImplItem const& item)
  : LinkImplItem(item)
{
}

bool cmGeneratorTarget::TargetPropertyEntry::GetHadContextSensitiveCondition()
  const
{
  return false;
}
