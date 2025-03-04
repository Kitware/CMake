/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmCTestCommand::operator()(std::vector<cmListFileArgument> const& args,
                                cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  std::vector<std::string> expandedArguments;
  if (!mf.ExpandArguments(args, expandedArguments)) {
    // There was an error expanding arguments.  It was already
    // reported, so we can skip this command without error.
    return true;
  }
  return this->InitialPass(expandedArguments, status);
}
