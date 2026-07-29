/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmAddCompileDefinitionsCommand.h"

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmAddCompileDefinitionsCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_NON_TARGET_DIRECTIVE,
                     "Use of non-target directives is not recommended. "
                     "Consider target_compile_definitions instead.");

  for (std::string const& i : args) {
    mf.AddCompileDefinition(i);
  }

  return true;
}
