/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeCommand.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"

bool cmCMakeCommand(std::vector<cmListFileArgument> const& args,
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

  if (dispatchExpandedArgs[0] == "INVOKE") {
    if ((args.size() == 1 && dispatchExpandedArgs.size() != 2) ||
        dispatchExpandedArgs.size() > 2) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }

    // First argument is the name of the function to call
    std::string invokeCommand;
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

      invokeCommand = functionExpandedArg[0];
      startArg = 2;
    } else {
      invokeCommand = dispatchExpandedArgs[1];
      startArg = 1;
    }

    cmListFileFunction func;
    func.Name = invokeCommand;
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
