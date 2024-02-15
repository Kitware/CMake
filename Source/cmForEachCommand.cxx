/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmForEachCommand.h"

#include <algorithm>
#include <cassert>
#include <cstddef> // IWYU pragma: keep
// NOTE The declaration of `std::abs` has moved to `cmath` since C++17
// See https://en.cppreference.com/w/cpp/numeric/math/abs
// ALERT But IWYU used to lint `#include`s do not "understand"
// conditional compilation (i.e. `#if __cplusplus >= 201703L`)
#include <cstdlib>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {
class cmForEachFunctionBlocker : public cmFunctionBlocker
{
public:
  explicit cmForEachFunctionBlocker(cmMakefile* mf);
  ~cmForEachFunctionBlocker() override;

  cm::string_view StartCommandName() const override { return "foreach"_s; }
  cm::string_view EndCommandName() const override { return "endforeach"_s; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

  void SetIterationVarsCount(const std::size_t varsCount)
  {
    this->IterationVarsCount = varsCount;
  }
  void SetZipLists() { this->ZipLists = true; }

  std::vector<std::string> Args;

private:
  struct InvokeResult
  {
    bool Restore;
    bool Break;
  };

  bool ReplayItems(std::vector<cmListFileFunction> const& functions,
                   cmExecutionStatus& inStatus);

  bool ReplayZipLists(std::vector<cmListFileFunction> const& functions,
                      cmExecutionStatus& inStatus);

  InvokeResult invoke(std::vector<cmListFileFunction> const& functions,
                      cmExecutionStatus& inStatus, cmMakefile& mf);

  cmMakefile* Makefile;
  std::size_t IterationVarsCount = 0u;
  bool ZipLists = false;
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
  mf.ExpandArguments(lff.Arguments(), expandedArguments);
  return expandedArguments.empty() ||
    expandedArguments.front() == this->Args.front();
}

bool cmForEachFunctionBlocker::Replay(
  std::vector<cmListFileFunction> functions, cmExecutionStatus& inStatus)
{
  return this->ZipLists ? this->ReplayZipLists(functions, inStatus)
                        : this->ReplayItems(functions, inStatus);
}

bool cmForEachFunctionBlocker::ReplayItems(
  std::vector<cmListFileFunction> const& functions,
  cmExecutionStatus& inStatus)
{
  assert("Unexpected number of iteration variables" &&
         this->IterationVarsCount == 1);

  auto& mf = inStatus.GetMakefile();

  // At end of for each execute recorded commands
  // store the old value
  cm::optional<std::string> oldDef;
  if (mf.GetPolicyStatus(cmPolicies::CMP0124) != cmPolicies::NEW) {
    oldDef = mf.GetSafeDefinition(this->Args.front());
  } else if (mf.IsNormalDefinitionSet(this->Args.front())) {
    oldDef = *mf.GetDefinition(this->Args.front());
  }

  auto restore = false;
  for (std::string const& arg : cmMakeRange(this->Args).advance(1)) {
    // Set the variable to the loop value
    mf.AddDefinition(this->Args.front(), arg);
    // Invoke all the functions that were collected in the block.
    auto r = this->invoke(functions, inStatus, mf);
    restore = r.Restore;
    if (r.Break) {
      break;
    }
  }

  if (restore) {
    if (oldDef) {
      // restore the variable to its prior value
      mf.AddDefinition(this->Args.front(), *oldDef);
    } else {
      mf.RemoveDefinition(this->Args.front());
    }
  }

  return true;
}

bool cmForEachFunctionBlocker::ReplayZipLists(
  std::vector<cmListFileFunction> const& functions,
  cmExecutionStatus& inStatus)
{
  assert("Unexpected number of iteration variables" &&
         this->IterationVarsCount >= 1);

  auto& mf = inStatus.GetMakefile();

  // Expand the list of list-variables into a list of lists of strings
  std::vector<cmList> values;
  values.reserve(this->Args.size() - this->IterationVarsCount);
  // Also track the longest list size
  std::size_t maxItems = 0u;
  for (auto const& var :
       cmMakeRange(this->Args).advance(this->IterationVarsCount)) {
    cmList items;
    auto const& value = mf.GetSafeDefinition(var);
    if (!value.empty()) {
      items.assign(value, cmList::EmptyElements::Yes);
    }
    maxItems = std::max(maxItems, items.size());
    values.emplace_back(std::move(items));
  }

  // Form the list of iteration variables
  std::vector<std::string> iterationVars;
  if (this->IterationVarsCount > 1) {
    // If multiple iteration variables has given,
    // just copy them to the `iterationVars` list.
    iterationVars.reserve(values.size());
    std::copy(this->Args.begin(),
              this->Args.begin() + this->IterationVarsCount,
              std::back_inserter(iterationVars));
  } else {
    // In case of the only iteration variable,
    // generate names as `var_name_N`,
    // where `N` is the count of lists to zip
    iterationVars.resize(values.size());
    const auto iter_var_prefix = this->Args.front() + "_";
    auto i = 0u;
    std::generate(
      iterationVars.begin(), iterationVars.end(),
      [&]() -> std::string { return iter_var_prefix + std::to_string(i++); });
  }
  assert("Sanity check" && iterationVars.size() == values.size());

  // Store old values for iteration variables
  std::map<std::string, cm::optional<std::string>> oldDefs;
  for (auto i = 0u; i < values.size(); ++i) {
    const auto& varName = iterationVars[i];
    if (mf.GetPolicyStatus(cmPolicies::CMP0124) != cmPolicies::NEW) {
      oldDefs.emplace(varName, mf.GetSafeDefinition(varName));
    } else if (mf.IsNormalDefinitionSet(varName)) {
      oldDefs.emplace(varName, *mf.GetDefinition(varName));
    } else {
      oldDefs.emplace(varName, cm::nullopt);
    }
  }

  // Form a vector of current positions in all lists (Ok, vectors) of values
  std::vector<decltype(values)::value_type::iterator> positions;
  positions.reserve(values.size());
  std::transform(
    values.begin(), values.end(), std::back_inserter(positions),
    // Set the initial position to the beginning of every list
    [](decltype(values)::value_type& list) { return list.begin(); });
  assert("Sanity check" && positions.size() == values.size());

  auto restore = false;
  // Iterate over all the lists simulateneously
  for (auto i = 0u; i < maxItems; ++i) {
    // Declare iteration variables
    for (auto j = 0u; j < values.size(); ++j) {
      // Define (or not) the iteration variable if the current position
      // still not at the end...
      if (positions[j] != values[j].end()) {
        mf.AddDefinition(iterationVars[j], *positions[j]);
        ++positions[j];
      } else {
        mf.RemoveDefinition(iterationVars[j]);
      }
    }
    // Invoke all the functions that were collected in the block.
    auto r = this->invoke(functions, inStatus, mf);
    restore = r.Restore;
    if (r.Break) {
      break;
    }
  }

  // Restore the variables to its prior value
  if (restore) {
    for (auto const& p : oldDefs) {
      if (p.second) {
        mf.AddDefinition(p.first, *p.second);
      } else {
        mf.RemoveDefinition(p.first);
      }
    }
  }
  return true;
}

auto cmForEachFunctionBlocker::invoke(
  std::vector<cmListFileFunction> const& functions,
  cmExecutionStatus& inStatus, cmMakefile& mf) -> InvokeResult
{
  InvokeResult result = { true, false };
  // Invoke all the functions that were collected in the block.
  for (cmListFileFunction const& func : functions) {
    cmExecutionStatus status(mf);
    mf.ExecuteCommand(func, status);
    if (status.GetReturnInvoked()) {
      inStatus.SetReturnInvoked(status.GetReturnVariables());
      result.Break = true;
      break;
    }
    if (status.GetBreakInvoked()) {
      result.Break = true;
      break;
    }
    if (status.GetContinueInvoked()) {
      break;
    }
    if (status.HasExitCode()) {
      inStatus.SetExitCode(status.GetExitCode());
      result.Break = true;
      break;
    }
    if (cmSystemTools::GetFatalErrorOccurred()) {
      result.Restore = false;
      result.Break = true;
      break;
    }
  }
  return result;
}

bool HandleInMode(std::vector<std::string> const& args,
                  std::vector<std::string>::const_iterator kwInIter,
                  cmMakefile& makefile)
{
  assert("A valid iterator expected" && kwInIter != args.end());

  auto fb = cm::make_unique<cmForEachFunctionBlocker>(&makefile);

  // Copy iteration variable names first
  std::copy(args.begin(), kwInIter, std::back_inserter(fb->Args));
  // Remember the count of given iteration variable names
  const auto varsCount = fb->Args.size();
  fb->SetIterationVarsCount(varsCount);

  enum Doing
  {
    DoingNone,
    DoingLists,
    DoingItems,
    DoingZipLists
  };
  Doing doing = DoingNone;
  // Iterate over arguments past the "IN" keyword
  for (std::string const& arg : cmMakeRange(++kwInIter, args.end())) {
    if (arg == "LISTS") {
      if (doing == DoingZipLists) {
        makefile.IssueMessage(MessageType::FATAL_ERROR,
                              "ZIP_LISTS can not be used with LISTS or ITEMS");
        return true;
      }
      if (varsCount != 1u) {
        makefile.IssueMessage(
          MessageType::FATAL_ERROR,
          "ITEMS or LISTS require exactly one iteration variable");
        return true;
      }
      doing = DoingLists;

    } else if (arg == "ITEMS") {
      if (doing == DoingZipLists) {
        makefile.IssueMessage(MessageType::FATAL_ERROR,
                              "ZIP_LISTS can not be used with LISTS or ITEMS");
        return true;
      }
      if (varsCount != 1u) {
        makefile.IssueMessage(
          MessageType::FATAL_ERROR,
          "ITEMS or LISTS require exactly one iteration variable");
        return true;
      }
      doing = DoingItems;

    } else if (arg == "ZIP_LISTS") {
      if (doing != DoingNone) {
        makefile.IssueMessage(MessageType::FATAL_ERROR,
                              "ZIP_LISTS can not be used with LISTS or ITEMS");
        return true;
      }
      doing = DoingZipLists;
      fb->SetZipLists();

    } else if (doing == DoingLists) {
      auto const& value = makefile.GetSafeDefinition(arg);
      if (!value.empty()) {
        cmExpandList(value, fb->Args, cmList::EmptyElements::Yes);
      }

    } else if (doing == DoingItems || doing == DoingZipLists) {
      fb->Args.push_back(arg);

    } else {
      makefile.IssueMessage(MessageType::FATAL_ERROR,
                            cmStrCat("Unknown argument:\n", "  ", arg, "\n"));
      return true;
    }
  }

  // If `ZIP_LISTS` given and variables count more than 1,
  // make sure the given lists count matches variables...
  if (doing == DoingZipLists && varsCount > 1u &&
      (2u * varsCount) != fb->Args.size()) {
    makefile.IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Expected ", std::to_string(varsCount),
               " list variables, but given ",
               std::to_string(fb->Args.size() - varsCount)));
    return true;
  }

  makefile.AddFunctionBlocker(std::move(fb));

  return true;
}

bool TryParseInteger(cmExecutionStatus& status, const std::string& str, int& i)
{
  try {
    i = std::stoi(str);
  } catch (std::invalid_argument&) {
    std::ostringstream e;
    e << "Invalid integer: '" << str << "'";
    status.SetError(e.str());
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  } catch (std::out_of_range&) {
    std::ostringstream e;
    e << "Integer out of range: '" << str << "'";
    status.SetError(e.str());
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

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
  auto kwInIter = std::find(args.begin(), args.end(), "IN");
  if (kwInIter != args.end()) {
    return HandleInMode(args, kwInIter, status.GetMakefile());
  }

  // create a function blocker
  auto fb = cm::make_unique<cmForEachFunctionBlocker>(&status.GetMakefile());
  if (args.size() > 1) {
    if (args[1] == "RANGE") {
      int start = 0;
      int stop = 0;
      int step = 0;
      if (args.size() == 3) {
        if (!TryParseInteger(status, args[2], stop)) {
          return false;
        }
      }
      if (args.size() == 4) {
        if (!TryParseInteger(status, args[2], start)) {
          return false;
        }
        if (!TryParseInteger(status, args[3], stop)) {
          return false;
        }
      }
      if (args.size() == 5) {
        if (!TryParseInteger(status, args[2], start)) {
          return false;
        }
        if (!TryParseInteger(status, args[3], stop)) {
          return false;
        }
        if (!TryParseInteger(status, args[4], step)) {
          return false;
        }
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
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }

      // Calculate expected iterations count and reserve enough space
      // in the `fb->Args` vector. The first item is the iteration variable
      // name...
      const std::size_t iter_cnt = 2u +
        static_cast<int>(start < stop) * (stop - start) / std::abs(step) +
        static_cast<int>(start > stop) * (start - stop) / std::abs(step);
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

  fb->SetIterationVarsCount(1u);
  status.GetMakefile().AddFunctionBlocker(std::move(fb));

  return true;
}
