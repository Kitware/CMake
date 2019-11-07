/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmForEachCommand.h"

#include <algorithm>
#include <cstddef>
// NOTE The declaration of `std::abs` has moved to `cmath` since C++17
// See https://en.cppreference.com/w/cpp/numeric/math/abs
// ALERT But IWYU used to lint `#include`s do not "understand"
// conditional compilation (i.e. `#if __cplusplus >= 201703L`)
#include <cstdlib>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cm_static_string_view.hxx"

#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
class cmForEachFunctionBlocker : public cmFunctionBlocker
{
public:
  cmForEachFunctionBlocker(cmMakefile* mf);
  ~cmForEachFunctionBlocker() override;

  cm::string_view StartCommandName() const override { return "foreach"_s; }
  cm::string_view EndCommandName() const override { return "endforeach"_s; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

  std::vector<std::string> Args;

private:
  cmMakefile* Makefile;
};

cmForEachFunctionBlocker::cmForEachFunctionBlocker(cmMakefile* mf)
  : Makefile(mf)
{
  this->Makefile->PushLoopBlock();
}

cmForEachFunctionBlocker::~cmForEachFunctionBlocker()
{
  this->Makefile->PopLoopBlock();
}

bool cmForEachFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                              cmMakefile& mf) const
{
  std::vector<std::string> expandedArguments;
  mf.ExpandArguments(lff.Arguments, expandedArguments);
  return expandedArguments.empty() ||
    expandedArguments.front() == this->Args.front();
}

bool cmForEachFunctionBlocker::Replay(
  std::vector<cmListFileFunction> functions, cmExecutionStatus& inStatus)
{
  cmMakefile& mf = inStatus.GetMakefile();
  // at end of for each execute recorded commands
  // store the old value
  std::string oldDef;
  if (mf.GetDefinition(this->Args.front())) {
    oldDef = mf.GetDefinition(this->Args.front());
  }

  for (std::string const& arg : cmMakeRange(this->Args).advance(1)) {
    // set the variable to the loop value
    mf.AddDefinition(this->Args.front(), arg);
    // Invoke all the functions that were collected in the block.
    for (cmListFileFunction const& func : functions) {
      cmExecutionStatus status(mf);
      mf.ExecuteCommand(func, status);
      if (status.GetReturnInvoked()) {
        inStatus.SetReturnInvoked();
        // restore the variable to its prior value
        mf.AddDefinition(this->Args.front(), oldDef);
        return true;
      }
      if (status.GetBreakInvoked()) {
        // restore the variable to its prior value
        mf.AddDefinition(this->Args.front(), oldDef);
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

  // restore the variable to its prior value
  mf.AddDefinition(this->Args.front(), oldDef);
  return true;
}

bool HandleInMode(std::vector<std::string> const& args, cmMakefile& makefile)
{
  auto fb = cm::make_unique<cmForEachFunctionBlocker>(&makefile);
  fb->Args.push_back(args.front());

  enum Doing
  {
    DoingNone,
    DoingLists,
    DoingItems
  };
  Doing doing = DoingNone;
  for (std::string const& arg : cmMakeRange(args).advance(2)) {
    if (arg == "LISTS") {
      doing = DoingLists;
    } else if (arg == "ITEMS") {
      doing = DoingItems;
    } else if (doing == DoingLists) {
      auto const& value = makefile.GetSafeDefinition(arg);
      if (!value.empty()) {
        cmExpandList(value, fb->Args, true);
      }
    } else if (doing == DoingItems) {
      fb->Args.push_back(arg);
    } else {
      makefile.IssueMessage(MessageType::FATAL_ERROR,
                            cmStrCat("Unknown argument:\n", "  ", arg, "\n"));
      return true;
    }
  }

  makefile.AddFunctionBlocker(std::move(fb));

  return true;
}

} // anonymous namespace

bool cmForEachCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  if (args.size() > 1 && args[1] == "IN") {
    return HandleInMode(args, status.GetMakefile());
  }

  // create a function blocker
  auto fb = cm::make_unique<cmForEachFunctionBlocker>(&status.GetMakefile());
  if (args.size() > 1) {
    if (args[1] == "RANGE") {
      int start = 0;
      int stop = 0;
      int step = 0;
      if (args.size() == 3) {
        stop = std::stoi(args[2]);
      }
      if (args.size() == 4) {
        start = std::stoi(args[2]);
        stop = std::stoi(args[3]);
      }
      if (args.size() == 5) {
        start = std::stoi(args[2]);
        stop = std::stoi(args[3]);
        step = std::stoi(args[4]);
      }
      if (step == 0) {
        if (start > stop) {
          step = -1;
        } else {
          step = 1;
        }
      }
      if ((start > stop && step > 0) || (start < stop && step < 0) ||
          step == 0) {
        status.SetError(
          cmStrCat("called with incorrect range specification: start ", start,
                   ", stop ", stop, ", step ", step));
        return false;
      }

      // Calculate expected iterations count and reserve enough space
      // in the `fb->Args` vector. The first item is the iteration variable
      // name...
      const std::size_t iter_cnt = 2u +
        int(start < stop) * (stop - start) / std::abs(step) +
        int(start > stop) * (start - stop) / std::abs(step);
      fb->Args.resize(iter_cnt);
      fb->Args.front() = args.front();
      auto cc = start;
      auto generator = [&cc, step]() -> std::string {
        auto result = std::to_string(cc);
        cc += step;
        return result;
      };
      // Fill the `range` vector w/ generated string values
      // (starting from 2nd position)
      std::generate(++fb->Args.begin(), fb->Args.end(), generator);
    } else {
      fb->Args = args;
    }
  } else {
    fb->Args = args;
  }
  status.GetMakefile().AddFunctionBlocker(std::move(fb));

  return true;
}
