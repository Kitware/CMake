/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUnsetCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmUnsetCommand
bool cmUnsetCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  if (args.empty() || args.size() > 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  auto const& variable = args[0];

  // unset(ENV{VAR})
  if (cmHasLiteralPrefix(variable, "ENV{") && variable.size() > 5) {
    // what is the variable name
    auto const& envVarName = variable.substr(4, variable.size() - 5);

#ifndef CMAKE_BOOTSTRAP
    cmSystemTools::UnsetEnv(envVarName.c_str());
#endif
    return true;
  }
  // unset(VAR)
  if (args.size() == 1) {
    status.GetMakefile().RemoveDefinition(variable);
    return true;
  }
  // unset(VAR CACHE)
  if ((args.size() == 2) && (args[1] == "CACHE")) {
    status.GetMakefile().RemoveCacheDefinition(variable);
    return true;
  }
  // unset(VAR PARENT_SCOPE)
  if ((args.size() == 2) && (args[1] == "PARENT_SCOPE")) {
    status.GetMakefile().RaiseScope(variable, nullptr);
    return true;
  }
  // ERROR: second argument isn't CACHE or PARENT_SCOPE
  status.SetError("called with an invalid second argument");
  return false;
}
