/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmListCommand.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <functional>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmStringReplaceHelper.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {

bool GetIndexArg(const std::string& arg, int* idx, cmMakefile& mf)
{
  long value;
  if (!cmStrToLong(arg, &value)) {
    switch (mf.GetPolicyStatus(cmPolicies::CMP0121)) {
      case cmPolicies::WARN: {
        // Default is to warn and use old behavior OLD behavior is to allow
        // compatibility, so issue a warning and use the previous behavior.
        std::string warn =
          cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0121),
                   " Invalid list index \"", arg, "\".");
        mf.IssueMessage(MessageType::AUTHOR_WARNING, warn);
        CM_FALLTHROUGH;
      }
      case cmPolicies::OLD:
        // OLD behavior is to allow compatibility, so just ignore the
        // situation.
        break;
      case cmPolicies::NEW:
        return false;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        std::string msg =
          cmStrCat(cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0121),
                   " Invalid list index \"", arg, "\".");
        mf.IssueMessage(MessageType::FATAL_ERROR, msg);
        break;
    }
  }

  // Truncation is happening here, but it had always been happening here.
  *idx = static_cast<int>(value);

  return true;
}

bool FilterRegex(std::vector<std::string> const& args, bool includeMatches,
                 std::string const& listName,
                 std::vector<std::string>& varArgsExpanded,
                 cmExecutionStatus& status);

bool GetListString(std::string& listString, const std::string& var,
                   const cmMakefile& makefile)
{
  // get the old value
  cmValue cacheValue = makefile.GetDefinition(var);
  if (!cacheValue) {
    return false;
  }
  listString = *cacheValue;
  return true;
}

bool GetList(std::vector<std::string>& list, const std::string& var,
             const cmMakefile& makefile)
{
  std::string listString;
  if (!GetListString(listString, var, makefile)) {
    return false;
  }
  // if the size of the list
  if (listString.empty()) {
    return true;
  }
  // expand the variable into a list
  cmExpandList(listString, list, true);
  // if no empty elements then just return
  if (!cm::contains(list, std::string())) {
    return true;
  }
  // if we have empty elements we need to check policy CMP0007
  switch (makefile.GetPolicyStatus(cmPolicies::CMP0007)) {
    case cmPolicies::WARN: {
      // Default is to warn and use old behavior
      // OLD behavior is to allow compatibility, so recall
      // ExpandListArgument without the true which will remove
      // empty values
      list.clear();
      cmExpandList(listString, list);
      std::string warn =
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0007),
                 " List has value = [", listString, "].");
      makefile.IssueMessage(MessageType::AUTHOR_WARNING, warn);
      return true;
    }
    case cmPolicies::OLD:
      // OLD behavior is to allow compatibility, so recall
      // ExpandListArgument without the true which will remove
      // empty values
      list.clear();
      cmExpandList(listString, list);
      return true;
    case cmPolicies::NEW:
      return true;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      makefile.IssueMessage(
        MessageType::FATAL_ERROR,
        cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0007));
      return false;
  }
  return true;
}

bool HandleLengthCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command LENGTH requires two arguments.");
    return false;
  }

  const std::string& listName = args[1];
  const std::string& variableName = args.back();
  std::vector<std::string> varArgsExpanded;
  // do not check the return value here
  // if the list var is not found varArgsExpanded will have size 0
  // and we will return 0
  GetList(varArgsExpanded, listName, status.GetMakefile());
  size_t length = varArgsExpanded.size();
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(length));

  status.GetMakefile().AddDefinition(variableName, buffer);
  return true;
}

bool HandleGetCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.size() < 4) {
    status.SetError("sub-command GET requires at least three arguments.");
    return false;
  }

  const std::string& listName = args[1];
  const std::string& variableName = args.back();
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    status.GetMakefile().AddDefinition(variableName, "NOTFOUND");
    return true;
  }
  // FIXME: Add policy to make non-existing lists an error like empty lists.
  if (varArgsExpanded.empty()) {
    status.SetError("GET given empty list");
    return false;
  }

  std::string value;
  size_t cc;
  const char* sep = "";
  size_t nitem = varArgsExpanded.size();
  for (cc = 2; cc < args.size() - 1; cc++) {
    int item;
    if (!GetIndexArg(args[cc], &item, status.GetMakefile())) {
      status.SetError(cmStrCat("index: ", args[cc], " is not a valid index"));
      return false;
    }
    value += sep;
    sep = ";";
    if (item < 0) {
      item = static_cast<int>(nitem) + item;
    }
    if (item < 0 || nitem <= static_cast<size_t>(item)) {
      status.SetError(cmStrCat("index: ", item, " out of range (-", nitem,
                               ", ", nitem - 1, ")"));
      return false;
    }
    value += varArgsExpanded[item];
  }

  status.GetMakefile().AddDefinition(variableName, value);
  return true;
}

bool HandleAppendCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  assert(args.size() >= 2);

  // Skip if nothing to append.
  if (args.size() < 3) {
    return true;
  }

  cmMakefile& makefile = status.GetMakefile();
  std::string const& listName = args[1];
  // expand the variable
  std::string listString;
  GetListString(listString, listName, makefile);

  // If `listString` or `args` is empty, no need to append `;`,
  // then index is going to be `1` and points to the end-of-string ";"
  auto const offset =
    static_cast<std::string::size_type>(listString.empty() || args.empty());
  listString += &";"[offset] + cmJoin(cmMakeRange(args).advance(2), ";");

  makefile.AddDefinition(listName, listString);
  return true;
}

bool HandlePrependCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  assert(args.size() >= 2);

  // Skip if nothing to prepend.
  if (args.size() < 3) {
    return true;
  }

  cmMakefile& makefile = status.GetMakefile();
  std::string const& listName = args[1];
  // expand the variable
  std::string listString;
  GetListString(listString, listName, makefile);

  // If `listString` or `args` is empty, no need to append `;`,
  // then `offset` is going to be `1` and points to the end-of-string ";"
  auto const offset =
    static_cast<std::string::size_type>(listString.empty() || args.empty());
  listString.insert(0,
                    cmJoin(cmMakeRange(args).advance(2), ";") + &";"[offset]);

  makefile.AddDefinition(listName, listString);
  return true;
}

bool HandlePopBackCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  assert(args.size() >= 2);

  cmMakefile& makefile = status.GetMakefile();
  auto ai = args.cbegin();
  ++ai; // Skip subcommand name
  std::string const& listName = *ai++;
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, makefile)) {
    // Can't get the list definition... undefine any vars given after.
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
    return true;
  }

  if (!varArgsExpanded.empty()) {
    if (ai == args.cend()) {
      // No variables are given... Just remove one element.
      varArgsExpanded.pop_back();
    } else {
      // Ok, assign elements to be removed to the given variables
      for (; !varArgsExpanded.empty() && ai != args.cend(); ++ai) {
        assert(!ai->empty());
        makefile.AddDefinition(*ai, varArgsExpanded.back());
        varArgsExpanded.pop_back();
      }
      // Undefine the rest variables if the list gets empty earlier...
      for (; ai != args.cend(); ++ai) {
        makefile.RemoveDefinition(*ai);
      }
    }

    makefile.AddDefinition(listName, cmJoin(varArgsExpanded, ";"));

  } else if (ai !=
             args.cend()) { // The list is empty, but some args were given
    // Need to *undefine* 'em all, cuz there are no items to assign...
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
  }

  return true;
}

bool HandlePopFrontCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  assert(args.size() >= 2);

  cmMakefile& makefile = status.GetMakefile();
  auto ai = args.cbegin();
  ++ai; // Skip subcommand name
  std::string const& listName = *ai++;
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, makefile)) {
    // Can't get the list definition... undefine any vars given after.
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
    return true;
  }

  if (!varArgsExpanded.empty()) {
    if (ai == args.cend()) {
      // No variables are given... Just remove one element.
      varArgsExpanded.erase(varArgsExpanded.begin());
    } else {
      // Ok, assign elements to be removed to the given variables
      auto vi = varArgsExpanded.begin();
      for (; vi != varArgsExpanded.end() && ai != args.cend(); ++ai, ++vi) {
        assert(!ai->empty());
        makefile.AddDefinition(*ai, *vi);
      }
      varArgsExpanded.erase(varArgsExpanded.begin(), vi);
      // Undefine the rest variables if the list gets empty earlier...
      for (; ai != args.cend(); ++ai) {
        makefile.RemoveDefinition(*ai);
      }
    }

    makefile.AddDefinition(listName, cmJoin(varArgsExpanded, ";"));

  } else if (ai !=
             args.cend()) { // The list is empty, but some args were given
    // Need to *undefine* 'em all, cuz there are no items to assign...
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
  }

  return true;
}

bool HandleFindCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 4) {
    status.SetError("sub-command FIND requires three arguments.");
    return false;
  }

  const std::string& listName = args[1];
  const std::string& variableName = args.back();
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    status.GetMakefile().AddDefinition(variableName, "-1");
    return true;
  }

  auto it = std::find(varArgsExpanded.begin(), varArgsExpanded.end(), args[2]);
  if (it != varArgsExpanded.end()) {
    status.GetMakefile().AddDefinition(
      variableName,
      std::to_string(std::distance(varArgsExpanded.begin(), it)));
    return true;
  }

  status.GetMakefile().AddDefinition(variableName, "-1");
  return true;
}

bool HandleInsertCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 4) {
    status.SetError("sub-command INSERT requires at least three arguments.");
    return false;
  }

  const std::string& listName = args[1];

  // expand the variable
  int item;
  if (!GetIndexArg(args[2], &item, status.GetMakefile())) {
    status.SetError(cmStrCat("index: ", args[2], " is not a valid index"));
    return false;
  }
  std::vector<std::string> varArgsExpanded;
  if ((!GetList(varArgsExpanded, listName, status.GetMakefile()) ||
       varArgsExpanded.empty()) &&
      item != 0) {
    status.SetError(cmStrCat("index: ", item, " out of range (0, 0)"));
    return false;
  }

  if (!varArgsExpanded.empty()) {
    size_t nitem = varArgsExpanded.size();
    if (item < 0) {
      item = static_cast<int>(nitem) + item;
    }
    if (item < 0 || nitem < static_cast<size_t>(item)) {
      status.SetError(cmStrCat("index: ", item, " out of range (-",
                               varArgsExpanded.size(), ", ",
                               varArgsExpanded.size(), ")"));
      return false;
    }
  }

  varArgsExpanded.insert(varArgsExpanded.begin() + item, args.begin() + 3,
                         args.end());

  std::string value = cmJoin(varArgsExpanded, ";");
  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

bool HandleJoinCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 4) {
    status.SetError(cmStrCat("sub-command JOIN requires three arguments (",
                             args.size() - 1, " found)."));
    return false;
  }

  const std::string& listName = args[1];
  const std::string& glue = args[2];
  const std::string& variableName = args[3];

  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    status.GetMakefile().AddDefinition(variableName, "");
    return true;
  }

  std::string value =
    cmJoin(cmMakeRange(varArgsExpanded.begin(), varArgsExpanded.end()), glue);

  status.GetMakefile().AddDefinition(variableName, value);
  return true;
}

bool HandleRemoveItemCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  assert(args.size() >= 2);

  if (args.size() == 2) {
    return true;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    return true;
  }

  std::vector<std::string> remove(args.begin() + 2, args.end());
  std::sort(remove.begin(), remove.end());
  auto remEnd = std::unique(remove.begin(), remove.end());
  auto remBegin = remove.begin();

  auto argsEnd =
    cmRemoveMatching(varArgsExpanded, cmMakeRange(remBegin, remEnd));
  auto argsBegin = varArgsExpanded.cbegin();
  std::string value = cmJoin(cmMakeRange(argsBegin, argsEnd), ";");
  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

bool HandleReverseCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  assert(args.size() >= 2);
  if (args.size() > 2) {
    status.SetError("sub-command REVERSE only takes one argument.");
    return false;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    return true;
  }

  std::string value = cmJoin(cmReverseRange(varArgsExpanded), ";");

  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

bool HandleRemoveDuplicatesCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  assert(args.size() >= 2);
  if (args.size() > 2) {
    status.SetError("sub-command REMOVE_DUPLICATES only takes one argument.");
    return false;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    return true;
  }

  auto argsEnd = cmRemoveDuplicates(varArgsExpanded);
  auto argsBegin = varArgsExpanded.cbegin();
  std::string value = cmJoin(cmMakeRange(argsBegin, argsEnd), ";");

  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

// Helpers for list(TRANSFORM <list> ...)
using transform_type = std::function<std::string(const std::string&)>;

class transform_error : public std::runtime_error
{
public:
  transform_error(const std::string& error)
    : std::runtime_error(error)
  {
  }
};

class TransformSelector
{
public:
  virtual ~TransformSelector() = default;

  std::string Tag;

  virtual bool Validate(std::size_t count = 0) = 0;

  virtual bool InSelection(const std::string&) = 0;

  virtual void Transform(std::vector<std::string>& list,
                         const transform_type& transform)
  {
    std::transform(list.begin(), list.end(), list.begin(), transform);
  }

protected:
  TransformSelector(std::string&& tag)
    : Tag(std::move(tag))
  {
  }
};
class TransformNoSelector : public TransformSelector
{
public:
  TransformNoSelector()
    : TransformSelector("NO SELECTOR")
  {
  }

  bool Validate(std::size_t) override { return true; }

  bool InSelection(const std::string&) override { return true; }
};
class TransformSelectorRegex : public TransformSelector
{
public:
  TransformSelectorRegex(const std::string& regex)
    : TransformSelector("REGEX")
    , Regex(regex)
  {
  }

  bool Validate(std::size_t) override { return this->Regex.is_valid(); }

  bool InSelection(const std::string& value) override
  {
    return this->Regex.find(value);
  }

  cmsys::RegularExpression Regex;
};
class TransformSelectorIndexes : public TransformSelector
{
public:
  std::vector<int> Indexes;

  bool InSelection(const std::string&) override { return true; }

  void Transform(std::vector<std::string>& list,
                 const transform_type& transform) override
  {
    this->Validate(list.size());

    for (auto index : this->Indexes) {
      list[index] = transform(list[index]);
    }
  }

protected:
  TransformSelectorIndexes(std::string&& tag)
    : TransformSelector(std::move(tag))
  {
  }
  TransformSelectorIndexes(std::string&& tag, std::vector<int>&& indexes)
    : TransformSelector(std::move(tag))
    , Indexes(indexes)
  {
  }

  int NormalizeIndex(int index, std::size_t count)
  {
    if (index < 0) {
      index = static_cast<int>(count) + index;
    }
    if (index < 0 || count <= static_cast<std::size_t>(index)) {
      throw transform_error(cmStrCat(
        "sub-command TRANSFORM, selector ", this->Tag, ", index: ", index,
        " out of range (-", count, ", ", count - 1, ")."));
    }
    return index;
  }
};
class TransformSelectorAt : public TransformSelectorIndexes
{
public:
  TransformSelectorAt(std::vector<int>&& indexes)
    : TransformSelectorIndexes("AT", std::move(indexes))
  {
  }

  bool Validate(std::size_t count) override
  {
    decltype(this->Indexes) indexes;

    for (auto index : this->Indexes) {
      indexes.push_back(this->NormalizeIndex(index, count));
    }
    this->Indexes = std::move(indexes);

    return true;
  }
};
class TransformSelectorFor : public TransformSelectorIndexes
{
public:
  TransformSelectorFor(int start, int stop, int step)
    : TransformSelectorIndexes("FOR")
    , Start(start)
    , Stop(stop)
    , Step(step)
  {
  }

  bool Validate(std::size_t count) override
  {
    this->Start = this->NormalizeIndex(this->Start, count);
    this->Stop = this->NormalizeIndex(this->Stop, count);

    // Does stepping move us further from the end?
    if (this->Start > this->Stop) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, selector FOR "
                 "expects <start> to be no greater than <stop> (",
                 this->Start, " > ", this->Stop, ")"));
    }

    // compute indexes
    auto size = (this->Stop - this->Start + 1) / this->Step;
    if ((this->Stop - this->Start + 1) % this->Step != 0) {
      size += 1;
    }

    this->Indexes.resize(size);
    auto start = this->Start;
    auto step = this->Step;
    std::generate(this->Indexes.begin(), this->Indexes.end(),
                  [&start, step]() -> int {
                    auto r = start;
                    start += step;
                    return r;
                  });

    return true;
  }

private:
  int Start, Stop, Step;
};

class TransformAction
{
public:
  virtual ~TransformAction() = default;

  virtual std::string Transform(const std::string& input) = 0;
};
class TransformReplace : public TransformAction
{
public:
  TransformReplace(const std::vector<std::string>& arguments,
                   cmMakefile* makefile)
    : ReplaceHelper(arguments[0], arguments[1], makefile)
  {
    makefile->ClearMatches();

    if (!this->ReplaceHelper.IsRegularExpressionValid()) {
      throw transform_error(
        cmStrCat("sub-command TRANSFORM, action REPLACE: Failed to compile "
                 "regex \"",
                 arguments[0], "\"."));
    }
    if (!this->ReplaceHelper.IsReplaceExpressionValid()) {
      throw transform_error(cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                                     this->ReplaceHelper.GetError(), "."));
    }
  }

  std::string Transform(const std::string& input) override
  {
    // Scan through the input for all matches.
    std::string output;

    if (!this->ReplaceHelper.Replace(input, output)) {
      throw transform_error(cmStrCat("sub-command TRANSFORM, action REPLACE: ",
                                     this->ReplaceHelper.GetError(), "."));
    }

    return output;
  }

private:
  cmStringReplaceHelper ReplaceHelper;
};

bool HandleTransformCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError(
      "sub-command TRANSFORM requires an action to be specified.");
    return false;
  }

  // Structure collecting all elements of the command
  struct Command
  {
    Command(const std::string& listName)
      : ListName(listName)
      , OutputName(listName)
    {
    }

    std::string Name;
    std::string ListName;
    std::vector<std::string> Arguments;
    std::unique_ptr<TransformAction> Action;
    std::unique_ptr<TransformSelector> Selector;
    std::string OutputName;
  } command(args[1]);

  // Descriptor of action
  // Arity: number of arguments required for the action
  // Transform: lambda function implementing the action
  struct ActionDescriptor
  {
    ActionDescriptor(std::string name)
      : Name(std::move(name))
    {
    }
    ActionDescriptor(std::string name, int arity, transform_type transform)
      : Name(std::move(name))
      , Arity(arity)
#if defined(__GNUC__) && __GNUC__ == 6 && defined(__aarch64__)
      // std::function move constructor miscompiles on this architecture
      , Transform(transform)
#else
      , Transform(std::move(transform))
#endif
    {
    }

    operator const std::string&() const { return this->Name; }

    std::string Name;
    int Arity = 0;
    transform_type Transform;
  };

  // Build a set of supported actions.
  std::set<ActionDescriptor,
           std::function<bool(const std::string&, const std::string&)>>
    descriptors(
      [](const std::string& x, const std::string& y) { return x < y; });
  descriptors = { { "APPEND", 1,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return s + command.Arguments[0];
                      }

                      return s;
                    } },
                  { "PREPEND", 1,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return command.Arguments[0] + s;
                      }

                      return s;
                    } },
                  { "TOUPPER", 0,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return cmSystemTools::UpperCase(s);
                      }

                      return s;
                    } },
                  { "TOLOWER", 0,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return cmSystemTools::LowerCase(s);
                      }

                      return s;
                    } },
                  { "STRIP", 0,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return cmTrimWhitespace(s);
                      }

                      return s;
                    } },
                  { "GENEX_STRIP", 0,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return cmGeneratorExpression::Preprocess(
                          s,
                          cmGeneratorExpression::StripAllGeneratorExpressions);
                      }

                      return s;
                    } },
                  { "REPLACE", 2,
                    [&command](const std::string& s) -> std::string {
                      if (command.Selector->InSelection(s)) {
                        return command.Action->Transform(s);
                      }

                      return s;
                    } } };

  using size_type = std::vector<std::string>::size_type;
  size_type index = 2;

  // Parse all possible function parameters
  auto descriptor = descriptors.find(args[index]);

  if (descriptor == descriptors.end()) {
    status.SetError(
      cmStrCat(" sub-command TRANSFORM, ", args[index], " invalid action."));
    return false;
  }

  // Action arguments
  index += 1;
  if (args.size() < index + descriptor->Arity) {
    status.SetError(cmStrCat("sub-command TRANSFORM, action ",
                             descriptor->Name, " expects ", descriptor->Arity,
                             " argument(s)."));
    return false;
  }

  command.Name = descriptor->Name;
  index += descriptor->Arity;
  if (descriptor->Arity > 0) {
    command.Arguments =
      std::vector<std::string>(args.begin() + 3, args.begin() + index);
  }

  if (command.Name == "REPLACE") {
    try {
      command.Action = cm::make_unique<TransformReplace>(
        command.Arguments, &status.GetMakefile());
    } catch (const transform_error& e) {
      status.SetError(e.what());
      return false;
    }
  }

  const std::string REGEX{ "REGEX" };
  const std::string AT{ "AT" };
  const std::string FOR{ "FOR" };
  const std::string OUTPUT_VARIABLE{ "OUTPUT_VARIABLE" };

  // handle optional arguments
  while (args.size() > index) {
    if ((args[index] == REGEX || args[index] == AT || args[index] == FOR) &&
        command.Selector) {
      status.SetError(
        cmStrCat("sub-command TRANSFORM, selector already specified (",
                 command.Selector->Tag, ")."));

      return false;
    }

    // REGEX selector
    if (args[index] == REGEX) {
      if (args.size() == ++index) {
        status.SetError("sub-command TRANSFORM, selector REGEX expects "
                        "'regular expression' argument.");
        return false;
      }

      command.Selector = cm::make_unique<TransformSelectorRegex>(args[index]);
      if (!command.Selector->Validate()) {
        status.SetError(
          cmStrCat("sub-command TRANSFORM, selector REGEX failed to compile "
                   "regex \"",
                   args[index], "\"."));
        return false;
      }

      index += 1;
      continue;
    }

    // AT selector
    if (args[index] == AT) {
      // get all specified indexes
      std::vector<int> indexes;
      while (args.size() > ++index) {
        std::size_t pos;
        int value;

        try {
          value = std::stoi(args[index], &pos);
          if (pos != args[index].length()) {
            // this is not a number, stop processing
            break;
          }
          indexes.push_back(value);
        } catch (const std::invalid_argument&) {
          // this is not a number, stop processing
          break;
        }
      }

      if (indexes.empty()) {
        status.SetError(
          "sub-command TRANSFORM, selector AT expects at least one "
          "numeric value.");
        return false;
      }

      command.Selector =
        cm::make_unique<TransformSelectorAt>(std::move(indexes));

      continue;
    }

    // FOR selector
    if (args[index] == FOR) {
      if (args.size() <= ++index + 1) {
        status.SetError(
          "sub-command TRANSFORM, selector FOR expects, at least,"
          " two arguments.");
        return false;
      }

      int start = 0;
      int stop = 0;
      int step = 1;
      bool valid = true;
      try {
        std::size_t pos;

        start = std::stoi(args[index], &pos);
        if (pos != args[index].length()) {
          // this is not a number
          valid = false;
        } else {
          stop = std::stoi(args[++index], &pos);
          if (pos != args[index].length()) {
            // this is not a number
            valid = false;
          }
        }
      } catch (const std::invalid_argument&) {
        // this is not numbers
        valid = false;
      }
      if (!valid) {
        status.SetError("sub-command TRANSFORM, selector FOR expects, "
                        "at least, two numeric values.");
        return false;
      }
      // try to read a third numeric value for step
      if (args.size() > ++index) {
        try {
          std::size_t pos;

          step = std::stoi(args[index], &pos);
          if (pos != args[index].length()) {
            // this is not a number
            step = 1;
          } else {
            index += 1;
          }
        } catch (const std::invalid_argument&) {
          // this is not number, ignore exception
        }
      }

      if (step <= 0) {
        status.SetError("sub-command TRANSFORM, selector FOR expects "
                        "positive numeric value for <step>.");
        return false;
      }

      command.Selector =
        cm::make_unique<TransformSelectorFor>(start, stop, step);

      continue;
    }

    // output variable
    if (args[index] == OUTPUT_VARIABLE) {
      if (args.size() == ++index) {
        status.SetError("sub-command TRANSFORM, OUTPUT_VARIABLE "
                        "expects variable name argument.");
        return false;
      }

      command.OutputName = args[index++];
      continue;
    }

    status.SetError(cmStrCat("sub-command TRANSFORM, '",
                             cmJoin(cmMakeRange(args).advance(index), " "),
                             "': unexpected argument(s)."));
    return false;
  }

  // expand the list variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, command.ListName, status.GetMakefile())) {
    status.GetMakefile().AddDefinition(command.OutputName, "");
    return true;
  }

  if (!command.Selector) {
    // no selector specified, apply transformation to all elements
    command.Selector = cm::make_unique<TransformNoSelector>();
  }

  try {
    command.Selector->Transform(varArgsExpanded, descriptor->Transform);
  } catch (const transform_error& e) {
    status.SetError(e.what());
    return false;
  }

  status.GetMakefile().AddDefinition(command.OutputName,
                                     cmJoin(varArgsExpanded, ";"));

  return true;
}

class cmStringSorter
{
public:
  enum class Order
  {
    UNINITIALIZED,
    ASCENDING,
    DESCENDING,
  };

  enum class Compare
  {
    UNINITIALIZED,
    STRING,
    FILE_BASENAME,
    NATURAL,
  };
  enum class CaseSensitivity
  {
    UNINITIALIZED,
    SENSITIVE,
    INSENSITIVE,
  };

protected:
  using StringFilter = std::string (*)(const std::string&);
  StringFilter GetCompareFilter(Compare compare)
  {
    return (compare == Compare::FILE_BASENAME) ? cmSystemTools::GetFilenameName
                                               : nullptr;
  }

  StringFilter GetCaseFilter(CaseSensitivity sensitivity)
  {
    return (sensitivity == CaseSensitivity::INSENSITIVE)
      ? cmSystemTools::LowerCase
      : nullptr;
  }

  using ComparisonFunction =
    std::function<bool(const std::string&, const std::string&)>;
  ComparisonFunction GetComparisonFunction(Compare compare)
  {
    if (compare == Compare::NATURAL) {
      return std::function<bool(const std::string&, const std::string&)>(
        [](const std::string& x, const std::string& y) {
          return cmSystemTools::strverscmp(x, y) < 0;
        });
    }
    return std::function<bool(const std::string&, const std::string&)>(
      [](const std::string& x, const std::string& y) { return x < y; });
  }

public:
  cmStringSorter(Compare compare, CaseSensitivity caseSensitivity,
                 Order desc = Order::ASCENDING)
    : filters{ this->GetCompareFilter(compare),
               this->GetCaseFilter(caseSensitivity) }
    , sortMethod(this->GetComparisonFunction(compare))
    , descending(desc == Order::DESCENDING)
  {
  }

  std::string ApplyFilter(const std::string& argument)
  {
    std::string result = argument;
    for (auto filter : this->filters) {
      if (filter != nullptr) {
        result = filter(result);
      }
    }
    return result;
  }

  bool operator()(const std::string& a, const std::string& b)
  {
    std::string af = this->ApplyFilter(a);
    std::string bf = this->ApplyFilter(b);
    bool result;
    if (this->descending) {
      result = this->sortMethod(bf, af);
    } else {
      result = this->sortMethod(af, bf);
    }
    return result;
  }

protected:
  StringFilter filters[2] = { nullptr, nullptr };
  ComparisonFunction sortMethod;
  bool descending;
};

bool HandleSortCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  assert(args.size() >= 2);
  if (args.size() > 8) {
    status.SetError("sub-command SORT only takes up to six arguments.");
    return false;
  }

  auto sortCompare = cmStringSorter::Compare::UNINITIALIZED;
  auto sortCaseSensitivity = cmStringSorter::CaseSensitivity::UNINITIALIZED;
  auto sortOrder = cmStringSorter::Order::UNINITIALIZED;

  size_t argumentIndex = 2;
  const std::string messageHint = "sub-command SORT ";

  while (argumentIndex < args.size()) {
    std::string const& option = args[argumentIndex++];
    if (option == "COMPARE") {
      if (sortCompare != cmStringSorter::Compare::UNINITIALIZED) {
        std::string error = cmStrCat(messageHint, "option \"", option,
                                     "\" has been specified multiple times.");
        status.SetError(error);
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "STRING") {
          sortCompare = cmStringSorter::Compare::STRING;
        } else if (argument == "FILE_BASENAME") {
          sortCompare = cmStringSorter::Compare::FILE_BASENAME;
        } else if (argument == "NATURAL") {
          sortCompare = cmStringSorter::Compare::NATURAL;
        } else {
          std::string error =
            cmStrCat(messageHint, "value \"", argument, "\" for option \"",
                     option, "\" is invalid.");
          status.SetError(error);
          return false;
        }
      } else {
        status.SetError(cmStrCat(messageHint, "missing argument for option \"",
                                 option, "\"."));
        return false;
      }
    } else if (option == "CASE") {
      if (sortCaseSensitivity !=
          cmStringSorter::CaseSensitivity::UNINITIALIZED) {
        status.SetError(cmStrCat(messageHint, "option \"", option,
                                 "\" has been specified multiple times."));
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "SENSITIVE") {
          sortCaseSensitivity = cmStringSorter::CaseSensitivity::SENSITIVE;
        } else if (argument == "INSENSITIVE") {
          sortCaseSensitivity = cmStringSorter::CaseSensitivity::INSENSITIVE;
        } else {
          status.SetError(cmStrCat(messageHint, "value \"", argument,
                                   "\" for option \"", option,
                                   "\" is invalid."));
          return false;
        }
      } else {
        status.SetError(cmStrCat(messageHint, "missing argument for option \"",
                                 option, "\"."));
        return false;
      }
    } else if (option == "ORDER") {

      if (sortOrder != cmStringSorter::Order::UNINITIALIZED) {
        status.SetError(cmStrCat(messageHint, "option \"", option,
                                 "\" has been specified multiple times."));
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "ASCENDING") {
          sortOrder = cmStringSorter::Order::ASCENDING;
        } else if (argument == "DESCENDING") {
          sortOrder = cmStringSorter::Order::DESCENDING;
        } else {
          status.SetError(cmStrCat(messageHint, "value \"", argument,
                                   "\" for option \"", option,
                                   "\" is invalid."));
          return false;
        }
      } else {
        status.SetError(cmStrCat(messageHint, "missing argument for option \"",
                                 option, "\"."));
        return false;
      }
    } else {
      status.SetError(
        cmStrCat(messageHint, "option \"", option, "\" is unknown."));
      return false;
    }
  }
  // set Default Values if Option is not given
  if (sortCompare == cmStringSorter::Compare::UNINITIALIZED) {
    sortCompare = cmStringSorter::Compare::STRING;
  }
  if (sortCaseSensitivity == cmStringSorter::CaseSensitivity::UNINITIALIZED) {
    sortCaseSensitivity = cmStringSorter::CaseSensitivity::SENSITIVE;
  }
  if (sortOrder == cmStringSorter::Order::UNINITIALIZED) {
    sortOrder = cmStringSorter::Order::ASCENDING;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    return true;
  }

  if ((sortCompare == cmStringSorter::Compare::STRING) &&
      (sortCaseSensitivity == cmStringSorter::CaseSensitivity::SENSITIVE) &&
      (sortOrder == cmStringSorter::Order::ASCENDING)) {
    std::sort(varArgsExpanded.begin(), varArgsExpanded.end());
  } else {
    cmStringSorter sorter(sortCompare, sortCaseSensitivity, sortOrder);
    std::sort(varArgsExpanded.begin(), varArgsExpanded.end(), sorter);
  }

  std::string value = cmJoin(varArgsExpanded, ";");
  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

bool HandleSublistCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() != 5) {
    status.SetError(cmStrCat("sub-command SUBLIST requires four arguments (",
                             args.size() - 1, " found)."));
    return false;
  }

  const std::string& listName = args[1];
  const std::string& variableName = args.back();

  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile()) ||
      varArgsExpanded.empty()) {
    status.GetMakefile().AddDefinition(variableName, "");
    return true;
  }

  int start;
  int length;
  if (!GetIndexArg(args[2], &start, status.GetMakefile())) {
    status.SetError(cmStrCat("index: ", args[2], " is not a valid index"));
    return false;
  }
  if (!GetIndexArg(args[3], &length, status.GetMakefile())) {
    status.SetError(cmStrCat("index: ", args[3], " is not a valid index"));
    return false;
  }

  using size_type = decltype(varArgsExpanded)::size_type;

  if (start < 0 || static_cast<size_type>(start) >= varArgsExpanded.size()) {
    status.SetError(cmStrCat("begin index: ", start, " is out of range 0 - ",
                             varArgsExpanded.size() - 1));
    return false;
  }
  if (length < -1) {
    status.SetError(cmStrCat("length: ", length, " should be -1 or greater"));
    return false;
  }

  const size_type end =
    (length == -1 ||
     static_cast<size_type>(start + length) > varArgsExpanded.size())
    ? varArgsExpanded.size()
    : static_cast<size_type>(start + length);
  std::vector<std::string> sublist(varArgsExpanded.begin() + start,
                                   varArgsExpanded.begin() + end);
  status.GetMakefile().AddDefinition(variableName, cmJoin(sublist, ";"));
  return true;
}

bool HandleRemoveAtCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("sub-command REMOVE_AT requires at least "
                    "two arguments.");
    return false;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile()) ||
      varArgsExpanded.empty()) {
    std::ostringstream str;
    str << "index: ";
    for (size_t i = 1; i < args.size(); ++i) {
      str << args[i];
      if (i != args.size() - 1) {
        str << ", ";
      }
    }
    str << " out of range (0, 0)";
    status.SetError(str.str());
    return false;
  }

  size_t cc;
  std::vector<size_t> removed;
  size_t nitem = varArgsExpanded.size();
  for (cc = 2; cc < args.size(); ++cc) {
    int item;
    if (!GetIndexArg(args[cc], &item, status.GetMakefile())) {
      status.SetError(cmStrCat("index: ", args[cc], " is not a valid index"));
      return false;
    }
    if (item < 0) {
      item = static_cast<int>(nitem) + item;
    }
    if (item < 0 || nitem <= static_cast<size_t>(item)) {
      status.SetError(cmStrCat("index: ", item, " out of range (-", nitem,
                               ", ", nitem - 1, ")"));
      return false;
    }
    removed.push_back(static_cast<size_t>(item));
  }

  std::sort(removed.begin(), removed.end());
  auto remEnd = std::unique(removed.begin(), removed.end());
  auto remBegin = removed.begin();

  auto argsEnd =
    cmRemoveIndices(varArgsExpanded, cmMakeRange(remBegin, remEnd));
  auto argsBegin = varArgsExpanded.cbegin();
  std::string value = cmJoin(cmMakeRange(argsBegin, argsEnd), ";");

  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

bool HandleFilterCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command FILTER requires a list to be specified.");
    return false;
  }

  if (args.size() < 3) {
    status.SetError(
      "sub-command FILTER requires an operator to be specified.");
    return false;
  }

  if (args.size() < 4) {
    status.SetError("sub-command FILTER requires a mode to be specified.");
    return false;
  }

  const std::string& op = args[2];
  bool includeMatches;
  if (op == "INCLUDE") {
    includeMatches = true;
  } else if (op == "EXCLUDE") {
    includeMatches = false;
  } else {
    status.SetError("sub-command FILTER does not recognize operator " + op);
    return false;
  }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if (!GetList(varArgsExpanded, listName, status.GetMakefile())) {
    return true;
  }

  const std::string& mode = args[3];
  if (mode == "REGEX") {
    if (args.size() != 5) {
      status.SetError("sub-command FILTER, mode REGEX "
                      "requires five arguments.");
      return false;
    }
    return FilterRegex(args, includeMatches, listName, varArgsExpanded,
                       status);
  }

  status.SetError("sub-command FILTER does not recognize mode " + mode);
  return false;
}

class MatchesRegex
{
public:
  MatchesRegex(cmsys::RegularExpression& in_regex, bool in_includeMatches)
    : regex(in_regex)
    , includeMatches(in_includeMatches)
  {
  }

  bool operator()(const std::string& target)
  {
    return this->regex.find(target) ^ this->includeMatches;
  }

private:
  cmsys::RegularExpression& regex;
  const bool includeMatches;
};

bool FilterRegex(std::vector<std::string> const& args, bool includeMatches,
                 std::string const& listName,
                 std::vector<std::string>& varArgsExpanded,
                 cmExecutionStatus& status)
{
  const std::string& pattern = args[4];
  cmsys::RegularExpression regex(pattern);
  if (!regex.is_valid()) {
    std::string error =
      cmStrCat("sub-command FILTER, mode REGEX failed to compile regex \"",
               pattern, "\".");
    status.SetError(error);
    return false;
  }

  auto argsBegin = varArgsExpanded.begin();
  auto argsEnd = varArgsExpanded.end();
  auto newArgsEnd =
    std::remove_if(argsBegin, argsEnd, MatchesRegex(regex, includeMatches));

  std::string value = cmJoin(cmMakeRange(argsBegin, newArgsEnd), ";");
  status.GetMakefile().AddDefinition(listName, value);
  return true;
}

} // namespace

bool cmListCommand(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("must be called with at least two arguments.");
    return false;
  }

  static cmSubcommandTable const subcommand{
    { "LENGTH"_s, HandleLengthCommand },
    { "GET"_s, HandleGetCommand },
    { "APPEND"_s, HandleAppendCommand },
    { "PREPEND"_s, HandlePrependCommand },
    { "POP_BACK"_s, HandlePopBackCommand },
    { "POP_FRONT"_s, HandlePopFrontCommand },
    { "FIND"_s, HandleFindCommand },
    { "INSERT"_s, HandleInsertCommand },
    { "JOIN"_s, HandleJoinCommand },
    { "REMOVE_AT"_s, HandleRemoveAtCommand },
    { "REMOVE_ITEM"_s, HandleRemoveItemCommand },
    { "REMOVE_DUPLICATES"_s, HandleRemoveDuplicatesCommand },
    { "TRANSFORM"_s, HandleTransformCommand },
    { "SORT"_s, HandleSortCommand },
    { "SUBLIST"_s, HandleSublistCommand },
    { "REVERSE"_s, HandleReverseCommand },
    { "FILTER"_s, HandleFilterCommand },
  };

  return subcommand(args[0], args, status);
}
