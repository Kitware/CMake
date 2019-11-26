/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmForEachCommand.h"

#include <cstdio>
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
bool HandleInMode(std::vector<std::string> const& args, cmMakefile& makefile);

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
  return expandedArguments.empty() || expandedArguments[0] == this->Args[0];
}

bool cmForEachFunctionBlocker::Replay(
  std::vector<cmListFileFunction> functions, cmExecutionStatus& inStatus)
{
  cmMakefile& mf = inStatus.GetMakefile();
  // at end of for each execute recorded commands
  // store the old value
  std::string oldDef;
  if (mf.GetDefinition(this->Args[0])) {
    oldDef = mf.GetDefinition(this->Args[0]);
  }

  for (std::string const& arg : cmMakeRange(this->Args).advance(1)) {
    // set the variable to the loop value
    mf.AddDefinition(this->Args[0], arg);
    // Invoke all the functions that were collected in the block.
    for (cmListFileFunction const& func : functions) {
      cmExecutionStatus status(mf);
      mf.ExecuteCommand(func, status);
      if (status.GetReturnInvoked()) {
        inStatus.SetReturnInvoked();
        // restore the variable to its prior value
        mf.AddDefinition(this->Args[0], oldDef);
        return true;
      }
      if (status.GetBreakInvoked()) {
        // restore the variable to its prior value
        mf.AddDefinition(this->Args[0], oldDef);
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
  mf.AddDefinition(this->Args[0], oldDef);
  return true;
}
}

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
        stop = atoi(args[2].c_str());
      }
      if (args.size() == 4) {
        start = atoi(args[2].c_str());
        stop = atoi(args[3].c_str());
      }
      if (args.size() == 5) {
        start = atoi(args[2].c_str());
        stop = atoi(args[3].c_str());
        step = atoi(args[4].c_str());
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
      std::vector<std::string> range;
      char buffer[100];
      range.push_back(args[0]);
      int cc;
      for (cc = start;; cc += step) {
        if ((step > 0 && cc > stop) || (step < 0 && cc < stop)) {
          break;
        }
        sprintf(buffer, "%d", cc);
        range.emplace_back(buffer);
        if (cc == stop) {
          break;
        }
      }
      fb->Args = range;
    } else {
      fb->Args = args;
    }
  } else {
    fb->Args = args;
  }
  status.GetMakefile().AddFunctionBlocker(std::move(fb));

  return true;
}

namespace {
bool HandleInMode(std::vector<std::string> const& args, cmMakefile& makefile)
{
  auto fb = cm::make_unique<cmForEachFunctionBlocker>(&makefile);
  fb->Args.push_back(args[0]);

  enum Doing
  {
    DoingNone,
    DoingLists,
    DoingItems
  };
  Doing doing = DoingNone;
  for (unsigned int i = 2; i < args.size(); ++i) {
    if (doing == DoingItems) {
      fb->Args.push_back(args[i]);
    } else if (args[i] == "LISTS") {
      doing = DoingLists;
    } else if (args[i] == "ITEMS") {
      doing = DoingItems;
    } else if (doing == DoingLists) {
      const char* value = makefile.GetDefinition(args[i]);
      if (value && *value) {
        cmExpandList(value, fb->Args, true);
      }
    } else {
      makefile.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Unknown argument:\n", "  ", args[i], "\n"));
      return true;
    }
  }

  makefile.AddFunctionBlocker(std::move(fb));

  return true;
}
}
