/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmAddCompileOptionsCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmAddCompileOptionsCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  for (std::string const& i : args) {
    mf.AddCompileOption(i);
  }
  return true;
}
