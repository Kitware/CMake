/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeCommand.h"

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"

inline std::ostream& operator<<(std::ostream& os,
                                cmListFileArgument const& arg)
{
  os << arg.Value;
  return os;
}

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

  if (args[0].Value == "INVOKE") {
    if (args.size() == 1) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }

    // First argument is the name of the function to call
    cmListFileFunction func;
    func.Name = args[1].Value;
    func.Line = context.Line;

    // The rest of the arguments are passed to the function call above
    func.Arguments.resize(args.size() - 1);
    for (size_t i = 2; i < args.size(); ++i) {
      cmListFileArgument lfarg;
      lfarg.Delim = args[i].Delim;
      lfarg.Line = context.Line;
      lfarg.Value = args[i].Value;
      func.Arguments.emplace_back(lfarg);
    }

    result = makefile.ExecuteCommand(func, status);
  } else if (args[0].Value == "EVAL") {
    if (args.size() < 2) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }

    auto code_iter = std::find_if(
      args.begin(), args.end(),
      [](cmListFileArgument const& arg) { return arg.Value == "CODE"; });
    if (code_iter == args.end()) {
      status.SetError("called without CODE argument");
      return false;
    }

    const std::string code = cmJoin(cmMakeRange(++code_iter, args.end()), " ");
    result = makefile.ReadListFileAsString(
      code, cmStrCat(context.FilePath, ":", context.Line, ":EVAL"));
  } else {
    status.SetError("called with unknown meta-operation");
  }

  return result;
}
