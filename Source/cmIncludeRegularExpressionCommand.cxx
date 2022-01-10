/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmIncludeRegularExpressionCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmIncludeRegularExpressionCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  if (args.empty() || args.size() > 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  mf.SetIncludeRegularExpression(args[0]);

  if (args.size() > 1) {
    mf.SetComplainRegularExpression(args[1]);
  }

  return true;
}
