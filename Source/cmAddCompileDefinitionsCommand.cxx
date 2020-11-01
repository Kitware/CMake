/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddCompileDefinitionsCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmAddCompileDefinitionsCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  for (std::string const& i : args) {
    mf.AddCompileDefinition(i);
  }
  return true;
}
