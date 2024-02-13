/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBlockCommand.h"

#include <cstdint>
#include <initializer_list>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/enum_set>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
enum class ScopeType : std::uint8_t
{
  VARIABLES,
  POLICIES
};
using ScopeSet = cm::enum_set<ScopeType>;

class BlockScopePushPop
{
public:
  BlockScopePushPop(cmMakefile* m, const ScopeSet& scopes);
  ~BlockScopePushPop() = default;

  BlockScopePushPop(const BlockScopePushPop&) = delete;
  BlockScopePushPop& operator=(const BlockScopePushPop&) = delete;

private:
  std::unique_ptr<cmMakefile::PolicyPushPop> PolicyScope;
  std::unique_ptr<cmMakefile::VariablePushPop> VariableScope;
};

BlockScopePushPop::BlockScopePushPop(cmMakefile* mf, const ScopeSet& scopes)
{
  if (scopes.contains(ScopeType::POLICIES)) {
    this->PolicyScope = cm::make_unique<cmMakefile::PolicyPushPop>(mf);
  }
  if (scopes.contains(ScopeType::VARIABLES)) {
    this->VariableScope = cm::make_unique<cmMakefile::VariablePushPop>(mf);
  }
}

class cmBlockFunctionBlocker : public cmFunctionBlocker
{
public:
  cmBlockFunctionBlocker(cmMakefile* mf, const ScopeSet& scopes,
                         std::vector<std::string> variableNames);
  ~cmBlockFunctionBlocker() override;

  cm::string_view StartCommandName() const override { return "block"_s; }
  cm::string_view EndCommandName() const override { return "endblock"_s; }

  bool EndCommandSupportsArguments() const override { return false; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

private:
  cmMakefile* Makefile;
  ScopeSet Scopes;
  BlockScopePushPop BlockScope;
  std::vector<std::string> VariableNames;
};

cmBlockFunctionBlocker::cmBlockFunctionBlocker(
  cmMakefile* const mf, const ScopeSet& scopes,
  std::vector<std::string> variableNames)
  : Makefile{ mf }
  , Scopes{ scopes }
  , BlockScope{ mf, scopes }
  , VariableNames{ std::move(variableNames) }
{
}

cmBlockFunctionBlocker::~cmBlockFunctionBlocker()
{
  if (this->Scopes.contains(ScopeType::VARIABLES)) {
    this->Makefile->RaiseScope(this->VariableNames);
  }
}

bool cmBlockFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                            cmMakefile&) const
{
  // no arguments expected for endblock()
  // but this method should not be called because EndCommandHasArguments()
  // returns false.
  return lff.Arguments().empty();
}

bool cmBlockFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
                                    cmExecutionStatus& inStatus)
{
  auto& mf = inStatus.GetMakefile();

  // Invoke all the functions that were collected in the block.
  for (cmListFileFunction const& fn : functions) {
    cmExecutionStatus status(mf);
    mf.ExecuteCommand(fn, status);
    if (status.GetReturnInvoked()) {
      mf.RaiseScope(status.GetReturnVariables());
      inStatus.SetReturnInvoked(status.GetReturnVariables());
      return true;
    }
    if (status.GetBreakInvoked()) {
      inStatus.SetBreakInvoked();
      return true;
    }
    if (status.GetContinueInvoked()) {
      inStatus.SetContinueInvoked();
      return true;
    }
    if (status.HasExitCode()) {
      inStatus.SetExitCode(status.GetExitCode());
      return true;
    }
    if (cmSystemTools::GetFatalErrorOccurred()) {
      return true;
    }
  }
  return true;
}

} // anonymous namespace

bool cmBlockCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> ScopeFor;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Propagate;
  };
  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("SCOPE_FOR"_s, &Arguments::ScopeFor)
                               .Bind("PROPAGATE"_s, &Arguments::Propagate);
  std::vector<std::string> unrecognizedArguments;
  auto parsedArgs = parser.Parse(args, &unrecognizedArguments);

  if (!unrecognizedArguments.empty()) {
    status.SetError(cmStrCat("called with unsupported argument \"",
                             unrecognizedArguments[0], '"'));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  ScopeSet scopes;

  if (parsedArgs.ScopeFor) {
    for (auto const& scope : *parsedArgs.ScopeFor) {
      if (scope == "VARIABLES"_s) {
        scopes.insert(ScopeType::VARIABLES);
        continue;
      }
      if (scope == "POLICIES"_s) {
        scopes.insert(ScopeType::POLICIES);
        continue;
      }
      status.SetError(cmStrCat("SCOPE_FOR unsupported scope \"", scope, '"'));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  } else {
    scopes = { ScopeType::VARIABLES, ScopeType::POLICIES };
  }
  if (!scopes.contains(ScopeType::VARIABLES) &&
      !parsedArgs.Propagate.empty()) {
    status.SetError(
      "PROPAGATE cannot be specified without a new scope for VARIABLES");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  // create a function blocker
  auto fb = cm::make_unique<cmBlockFunctionBlocker>(
    &status.GetMakefile(), scopes, parsedArgs.Propagate);
  status.GetMakefile().AddFunctionBlocker(std::move(fb));

  return true;
}
