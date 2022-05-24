/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWhileCommand.h"

#include <string>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmConditionEvaluator.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmSystemTools.h"
#include "cmake.h"

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf, std::vector<cmListFileArgument> args);
  ~cmWhileFunctionBlocker() override;

  cm::string_view StartCommandName() const override { return "while"_s; }
  cm::string_view EndCommandName() const override { return "endwhile"_s; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

private:
  cmMakefile* Makefile;
  std::vector<cmListFileArgument> Args;
};

cmWhileFunctionBlocker::cmWhileFunctionBlocker(
  cmMakefile* const mf, std::vector<cmListFileArgument> args)
  : Makefile{ mf }
  , Args{ std::move(args) }
{
  this->Makefile->PushLoopBlock();
}

cmWhileFunctionBlocker::~cmWhileFunctionBlocker()
{
  this->Makefile->PopLoopBlock();
}

bool cmWhileFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                            cmMakefile&) const
{
  return lff.Arguments().empty() || lff.Arguments() == this->Args;
}

bool cmWhileFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
                                    cmExecutionStatus& inStatus)
{
  auto& mf = inStatus.GetMakefile();

  cmListFileBacktrace whileBT =
    mf.GetBacktrace().Push(this->GetStartingContext());

  std::vector<cmExpandedCommandArgument> expandedArguments;
  // At least same size expected for `expandedArguments` as `Args`
  expandedArguments.reserve(this->Args.size());

  auto expandArgs = [&mf](std::vector<cmListFileArgument> const& args,
                          std::vector<cmExpandedCommandArgument>& out)
    -> std::vector<cmExpandedCommandArgument>& {
    out.clear();
    mf.ExpandArguments(args, out);
    return out;
  };

  // FIXME(#23296): For compatibility with older versions of CMake, we
  // tolerate condition errors that evaluate to false.  We should add
  // a policy to enforce such errors.
  bool enforceError = true;
  std::string errorString;
  MessageType messageType;

  for (cmConditionEvaluator conditionEvaluator(mf, whileBT);
       (enforceError = /* enforce condition errors that evaluate to true */
        conditionEvaluator.IsTrue(expandArgs(this->Args, expandedArguments),
                                  errorString, messageType));) {
    // Invoke all the functions that were collected in the block.
    for (cmListFileFunction const& fn : functions) {
      cmExecutionStatus status(mf);
      mf.ExecuteCommand(fn, status);
      if (status.GetReturnInvoked()) {
        inStatus.SetReturnInvoked();
        return true;
      }
      if (status.GetBreakInvoked()) {
        return true;
      }
      if (status.GetContinueInvoked()) {
        break;
      }
      if (cmSystemTools::GetFatalErrorOccured()) {
        return true;
      }
    }
  }

  if (!errorString.empty() && enforceError) {
    std::string err = "had incorrect arguments:\n ";
    for (auto const& i : expandedArguments) {
      err += " ";
      err += cmOutputConverter::EscapeForCMake(i.GetValue());
    }
    err += "\n";
    err += errorString;
    mf.GetCMakeInstance()->IssueMessage(messageType, err, whileBT);
    if (messageType == MessageType::FATAL_ERROR) {
      cmSystemTools::SetFatalErrorOccured();
    }
  }

  return true;
}

bool cmWhileCommand(std::vector<cmListFileArgument> const& args,
                    cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  auto& makefile = status.GetMakefile();
  makefile.AddFunctionBlocker(
    cm::make_unique<cmWhileFunctionBlocker>(&makefile, args));

  return true;
}
