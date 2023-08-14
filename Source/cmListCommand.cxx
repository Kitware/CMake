/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmListCommand.h"

#include <cassert>
#include <cstdio>
#include <functional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
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

cm::optional<cmList> GetList(const std::string& var,
                             const cmMakefile& makefile)
{
  cm::optional<cmList> list;

  std::string listString;
  if (!GetListString(listString, var, makefile)) {
    return list;
  }
  // if the size of the list
  if (listString.empty()) {
    list.emplace();
    return list;
  }
  // expand the variable into a list
  list.emplace(listString, cmList::EmptyElements::Yes);
  // if no empty elements then just return
  if (!cm::contains(*list, std::string())) {
    return list;
  }
  // if we have empty elements we need to check policy CMP0007
  switch (makefile.GetPolicyStatus(cmPolicies::CMP0007)) {
    case cmPolicies::WARN: {
      // Default is to warn and use old behavior
      // OLD behavior is to allow compatibility, so recall
      // ExpandListArgument without the true which will remove
      // empty values
      list->assign(listString);
      std::string warn =
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0007),
                 " List has value = [", listString, "].");
      makefile.IssueMessage(MessageType::AUTHOR_WARNING, warn);
      return list;
    }
    case cmPolicies::OLD:
      // OLD behavior is to allow compatibility, so recall
      // ExpandListArgument without the true which will remove
      // empty values
      list->assign(listString);
      return list;
    case cmPolicies::NEW:
      return list;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      makefile.IssueMessage(
        MessageType::FATAL_ERROR,
        cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0007));
      return {};
  }
  return list;
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

  auto list = GetList(listName, status.GetMakefile());
  status.GetMakefile().AddDefinition(variableName,
                                     std::to_string(list ? list->size() : 0));

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
  auto list = GetList(listName, status.GetMakefile());
  if (!list) {
    status.GetMakefile().AddDefinition(variableName, "NOTFOUND");
    return true;
  }
  // FIXME: Add policy to make non-existing lists an error like empty lists.
  if (list->empty()) {
    status.SetError("GET given empty list");
    return false;
  }

  std::vector<int> indexes;
  for (std::size_t cc = 2; cc < args.size() - 1; cc++) {
    int index;
    if (!GetIndexArg(args[cc], &index, status.GetMakefile())) {
      status.SetError(cmStrCat("index: ", args[cc], " is not a valid index"));
      return false;
    }
    indexes.push_back(index);
  }

  try {
    auto values = list->get_items(indexes.begin(), indexes.end());
    status.GetMakefile().AddDefinition(variableName, values.to_string());
    return true;
  } catch (std::out_of_range& e) {
    status.SetError(e.what());
    return false;
  }
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

  makefile.AddDefinition(
    listName, cmList::append(listString, args.begin() + 2, args.end()));
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

  makefile.AddDefinition(
    listName, cmList::prepend(listString, args.begin() + 2, args.end()));
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
  auto list = GetList(listName, makefile);

  if (!list) {
    // Can't get the list definition... undefine any vars given after.
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
    return true;
  }

  if (!list->empty()) {
    if (ai == args.cend()) {
      // No variables are given... Just remove one element.
      list->pop_back();
    } else {
      // Ok, assign elements to be removed to the given variables
      for (; !list->empty() && ai != args.cend(); ++ai) {
        assert(!ai->empty());
        makefile.AddDefinition(*ai, list->back());
        list->pop_back();
      }
      // Undefine the rest variables if the list gets empty earlier...
      for (; ai != args.cend(); ++ai) {
        makefile.RemoveDefinition(*ai);
      }
    }

    makefile.AddDefinition(listName, list->to_string());

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
  auto list = GetList(listName, makefile);

  if (!list) {
    // Can't get the list definition... undefine any vars given after.
    for (; ai != args.cend(); ++ai) {
      makefile.RemoveDefinition(*ai);
    }
    return true;
  }

  if (!list->empty()) {
    if (ai == args.cend()) {
      // No variables are given... Just remove one element.
      list->pop_front();
    } else {
      // Ok, assign elements to be removed to the given variables
      auto vi = list->begin();
      for (; vi != list->end() && ai != args.cend(); ++ai, ++vi) {
        assert(!ai->empty());
        makefile.AddDefinition(*ai, *vi);
      }
      list->erase(list->begin(), vi);
      // Undefine the rest variables if the list gets empty earlier...
      for (; ai != args.cend(); ++ai) {
        makefile.RemoveDefinition(*ai);
      }
    }

    makefile.AddDefinition(listName, list->to_string());

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
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    status.GetMakefile().AddDefinition(variableName, "-1");
    return true;
  }

  auto index = list->find(args[2]);
  status.GetMakefile().AddDefinition(
    variableName, index == cmList::npos ? "-1" : std::to_string(index));
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
  int index;
  if (!GetIndexArg(args[2], &index, status.GetMakefile())) {
    status.SetError(cmStrCat("index: ", args[2], " is not a valid index"));
    return false;
  }
  auto list = GetList(listName, status.GetMakefile());
  if (!list) {
    list = cmList{};
  }

  try {
    list->insert_items(index, args.begin() + 3, args.end());
    status.GetMakefile().AddDefinition(listName, list->to_string());
    return true;
  } catch (std::out_of_range& e) {
    status.SetError(e.what());
    return false;
  }
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    status.GetMakefile().AddDefinition(variableName, "");
    return true;
  }

  status.GetMakefile().AddDefinition(variableName, list->join(glue));
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    return true;
  }

  status.GetMakefile().AddDefinition(
    listName, list->remove_items(args.begin() + 2, args.end()).to_string());
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    return true;
  }

  status.GetMakefile().AddDefinition(listName, list->reverse().to_string());
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    return true;
  }

  status.GetMakefile().AddDefinition(listName,
                                     list->remove_duplicates().to_string());
  return true;
}

bool HandleTransformCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError(
      "sub-command TRANSFORM requires an action to be specified.");
    return false;
  }

  // Descriptor of action
  // Action: enum value identifying action
  // Arity: number of arguments required for the action
  struct ActionDescriptor
  {
    ActionDescriptor(std::string name)
      : Name(std::move(name))
    {
    }
    ActionDescriptor(std::string name, cmList::TransformAction action,
                     int arity)
      : Name(std::move(name))
      , Action(action)
      , Arity(arity)
    {
    }

    operator const std::string&() const { return this->Name; }

    std::string Name;
    cmList::TransformAction Action;
    int Arity = 0;
  };

  // Build a set of supported actions.
  std::set<ActionDescriptor,
           std::function<bool(const std::string&, const std::string&)>>
    descriptors{ { { "APPEND", cmList::TransformAction::APPEND, 1 },
                   { "PREPEND", cmList::TransformAction::PREPEND, 1 },
                   { "TOUPPER", cmList::TransformAction::TOUPPER, 0 },
                   { "TOLOWER", cmList::TransformAction::TOLOWER, 0 },
                   { "STRIP", cmList::TransformAction::STRIP, 0 },
                   { "GENEX_STRIP", cmList::TransformAction::GENEX_STRIP, 0 },
                   { "REPLACE", cmList::TransformAction::REPLACE, 2 } },
                 [](const std::string& x, const std::string& y) {
                   return x < y;
                 } };

  const std::string& listName = args[1];

  // Parse all possible function parameters
  using size_type = std::vector<std::string>::size_type;
  size_type index = 2;

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

  std::vector<std::string> arguments;
  index += descriptor->Arity;
  if (descriptor->Arity > 0) {
    arguments =
      std::vector<std::string>(args.begin() + 3, args.begin() + index);
  }

  const std::string REGEX{ "REGEX" };
  const std::string AT{ "AT" };
  const std::string FOR{ "FOR" };
  const std::string OUTPUT_VARIABLE{ "OUTPUT_VARIABLE" };
  std::unique_ptr<cmList::TransformSelector> selector;
  std::string outputName = listName;

  try {
    // handle optional arguments
    while (args.size() > index) {
      if ((args[index] == REGEX || args[index] == AT || args[index] == FOR) &&
          selector) {
        status.SetError(
          cmStrCat("sub-command TRANSFORM, selector already specified (",
                   selector->GetTag(), ")."));

        return false;
      }

      // REGEX selector
      if (args[index] == REGEX) {
        if (args.size() == ++index) {
          status.SetError("sub-command TRANSFORM, selector REGEX expects "
                          "'regular expression' argument.");
          return false;
        }

        selector =
          cmList::TransformSelector::New<cmList::TransformSelector::REGEX>(
            args[index]);

        index += 1;
        continue;
      }

      // AT selector
      if (args[index] == AT) {
        // get all specified indexes
        std::vector<cmList::index_type> indexes;
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

        selector =
          cmList::TransformSelector::New<cmList::TransformSelector::AT>(
            std::move(indexes));

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

        cmList::index_type start = 0;
        cmList::index_type stop = 0;
        cmList::index_type step = 1;
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

        selector =
          cmList::TransformSelector::New<cmList::TransformSelector::FOR>(
            { start, stop, step });

        continue;
      }

      // output variable
      if (args[index] == OUTPUT_VARIABLE) {
        if (args.size() == ++index) {
          status.SetError("sub-command TRANSFORM, OUTPUT_VARIABLE "
                          "expects variable name argument.");
          return false;
        }

        outputName = args[index++];
        continue;
      }

      status.SetError(cmStrCat("sub-command TRANSFORM, '",
                               cmJoin(cmMakeRange(args).advance(index), " "),
                               "': unexpected argument(s)."));
      return false;
    }

    // expand the list variable
    auto list = GetList(listName, status.GetMakefile());

    if (!list) {
      status.GetMakefile().AddDefinition(outputName, "");
      return true;
    }

    list->transform(descriptor->Action, arguments, std::move(selector));
    status.GetMakefile().AddDefinition(outputName, list->to_string());
    return true;
  } catch (cmList::transform_error& e) {
    status.SetError(e.what());
    return false;
  }
}

bool HandleSortCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  assert(args.size() >= 2);
  if (args.size() > 8) {
    status.SetError("sub-command SORT only takes up to six arguments.");
    return false;
  }

  using SortConfig = cmList::SortConfiguration;
  SortConfig sortConfig;

  size_t argumentIndex = 2;
  const std::string messageHint = "sub-command SORT ";

  while (argumentIndex < args.size()) {
    std::string const& option = args[argumentIndex++];
    if (option == "COMPARE") {
      if (sortConfig.Compare != SortConfig::CompareMethod::DEFAULT) {
        std::string error = cmStrCat(messageHint, "option \"", option,
                                     "\" has been specified multiple times.");
        status.SetError(error);
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "STRING") {
          sortConfig.Compare = SortConfig::CompareMethod::STRING;
        } else if (argument == "FILE_BASENAME") {
          sortConfig.Compare = SortConfig::CompareMethod::FILE_BASENAME;
        } else if (argument == "NATURAL") {
          sortConfig.Compare = SortConfig::CompareMethod::NATURAL;
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
      if (sortConfig.Case != SortConfig::CaseSensitivity::DEFAULT) {
        status.SetError(cmStrCat(messageHint, "option \"", option,
                                 "\" has been specified multiple times."));
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "SENSITIVE") {
          sortConfig.Case = SortConfig::CaseSensitivity::SENSITIVE;
        } else if (argument == "INSENSITIVE") {
          sortConfig.Case = SortConfig::CaseSensitivity::INSENSITIVE;
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

      if (sortConfig.Order != SortConfig::OrderMode::DEFAULT) {
        status.SetError(cmStrCat(messageHint, "option \"", option,
                                 "\" has been specified multiple times."));
        return false;
      }
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "ASCENDING") {
          sortConfig.Order = SortConfig::OrderMode::ASCENDING;
        } else if (argument == "DESCENDING") {
          sortConfig.Order = SortConfig::OrderMode::DESCENDING;
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

  const std::string& listName = args[1];
  // expand the variable
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    return true;
  }

  status.GetMakefile().AddDefinition(listName,
                                     list->sort(sortConfig).to_string());
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list || list->empty()) {
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

  if (start < 0) {
    status.SetError(cmStrCat("begin index: ", start, " is out of range 0 - ",
                             list->size() - 1));
    return false;
  }
  if (length < -1) {
    status.SetError(cmStrCat("length: ", length, " should be -1 or greater"));
    return false;
  }

  using size_type = cmList::size_type;

  try {
    auto sublist = list->sublist(static_cast<size_type>(start),
                                 static_cast<size_type>(length));
    status.GetMakefile().AddDefinition(variableName, sublist.to_string());
    return true;
  } catch (std::out_of_range& e) {
    status.SetError(e.what());
    return false;
  }
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
  auto list = GetList(listName, status.GetMakefile());

  if (!list || list->empty()) {
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
  std::vector<cmList::index_type> removed;
  for (cc = 2; cc < args.size(); ++cc) {
    int index;
    if (!GetIndexArg(args[cc], &index, status.GetMakefile())) {
      status.SetError(cmStrCat("index: ", args[cc], " is not a valid index"));
      return false;
    }
    removed.push_back(index);
  }

  try {
    status.GetMakefile().AddDefinition(
      listName,
      list->remove_items(removed.begin(), removed.end()).to_string());
    return true;
  } catch (std::out_of_range& e) {
    status.SetError(e.what());
    return false;
  }
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
  cmList::FilterMode filterMode;
  if (op == "INCLUDE") {
    filterMode = cmList::FilterMode::INCLUDE;
  } else if (op == "EXCLUDE") {
    filterMode = cmList::FilterMode::EXCLUDE;
  } else {
    status.SetError("sub-command FILTER does not recognize operator " + op);
    return false;
  }

  const std::string& listName = args[1];
  // expand the variable
  auto list = GetList(listName, status.GetMakefile());

  if (!list) {
    return true;
  }

  const std::string& mode = args[3];
  if (mode != "REGEX") {
    status.SetError("sub-command FILTER does not recognize mode " + mode);
    return false;
  }
  if (args.size() != 5) {
    status.SetError("sub-command FILTER, mode REGEX "
                    "requires five arguments.");
    return false;
  }
  const std::string& pattern = args[4];

  try {
    status.GetMakefile().AddDefinition(
      listName, list->filter(pattern, filterMode).to_string());
    return true;
  } catch (std::invalid_argument& e) {
    status.SetError(e.what());
    return false;
  }
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
