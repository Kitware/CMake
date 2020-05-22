/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeLanguageCommand.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
std::array<cm::static_string_view, 12> InvalidCommands{
  { // clang-format off
  "function"_s, "endfunction"_s,
  "macro"_s, "endmacro"_s,
  "if"_s, "elseif"_s, "else"_s, "endif"_s,
  "while"_s, "endwhile"_s,
  "foreach"_s, "endforeach"_s
  } // clang-format on
};
}

bool cmCMakeLanguageCommand(std::vector<cmListFileArgument> const& args,
                            cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& makefile = status.GetMakefile();
  cmListFileContext context = makefile.GetExecutionContext();

  bool result = false;

  std::vector<std::string> dispatchExpandedArgs;
  std::vector<cmListFileArgument> dispatchArgs;
  dispatchArgs.emplace_back(args[0]);
  makefile.ExpandArguments(dispatchArgs, dispatchExpandedArgs);

  if (dispatchExpandedArgs.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  if (dispatchExpandedArgs[0] == "CALL") {
    if ((args.size() == 1 && dispatchExpandedArgs.size() != 2) ||
        dispatchExpandedArgs.size() > 2) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }

    // First argument is the name of the function to call
    std::string callCommand;
    size_t startArg;
    if (dispatchExpandedArgs.size() == 1) {
      std::vector<std::string> functionExpandedArg;
      std::vector<cmListFileArgument> functionArg;
      functionArg.emplace_back(args[1]);
      makefile.ExpandArguments(functionArg, functionExpandedArg);

      if (functionExpandedArg.size() != 1) {
        status.SetError("called with incorrect number of arguments");
        return false;
      }

      callCommand = functionExpandedArg[0];
      startArg = 2;
    } else {
      callCommand = dispatchExpandedArgs[1];
      startArg = 1;
    }

    // ensure specified command is valid
    // start/end flow control commands are not allowed
    auto cmd = cmSystemTools::LowerCase(callCommand);
    if (std::find(InvalidCommands.cbegin(), InvalidCommands.cend(), cmd) !=
        InvalidCommands.cend()) {
      status.SetError(cmStrCat("invalid command specified: "_s, callCommand));
      return false;
    }

    cmListFileFunction func;
    func.Name = callCommand;
    func.Line = context.Line;

    // The rest of the arguments are passed to the function call above
    for (size_t i = startArg; i < args.size(); ++i) {
      cmListFileArgument lfarg;
      lfarg.Delim = args[i].Delim;
      lfarg.Line = context.Line;
      lfarg.Value = args[i].Value;
      func.Arguments.emplace_back(lfarg);
    }

    result = makefile.ExecuteCommand(func, status);
  } else if (dispatchExpandedArgs[0] == "EVAL") {
    std::vector<std::string> expandedArgs;
    makefile.ExpandArguments(args, expandedArgs);

    if (expandedArgs.size() < 2) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }

    if (expandedArgs[1] != "CODE") {
      auto code_iter =
        std::find(expandedArgs.begin() + 2, expandedArgs.end(), "CODE");
      if (code_iter == expandedArgs.end()) {
        status.SetError("called without CODE argument");
      } else {
        status.SetError(
          "called with unsupported arguments between EVAL and CODE arguments");
      }
      return false;
    }

    const std::string code =
      cmJoin(cmMakeRange(expandedArgs.begin() + 2, expandedArgs.end()), " ");
    result = makefile.ReadListFileAsString(
      code, cmStrCat(context.FilePath, ":", context.Line, ":EVAL"));
  } else {
    status.SetError("called with unknown meta-operation");
  }

  return result;
}
