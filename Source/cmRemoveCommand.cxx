/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmRemoveCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

// cmRemoveCommand
bool cmRemoveCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  if (args.empty()) {
    return true;
  }

  std::string const& variable = args[0]; // VAR is always first
  // get the old value
  cmValue cacheValue = status.GetMakefile().GetDefinition(variable);

  // if there is no old value then return
  if (!cacheValue) {
    return true;
  }

  // expand the variable
  std::vector<std::string> const varArgsExpanded = cmExpandedList(*cacheValue);

  // expand the args
  // check for REMOVE(VAR v1 v2 ... vn)
  std::vector<std::string> const argsExpanded =
    cmExpandedLists(args.begin() + 1, args.end());

  // now create the new value
  std::string value;
  for (std::string const& varArgExpanded : varArgsExpanded) {
    int found = 0;
    for (std::string const& argExpanded : argsExpanded) {
      if (varArgExpanded == argExpanded) {
        found = 1;
        break;
      }
    }
    if (!found) {
      if (!value.empty()) {
        value += ";";
      }
      value += varArgExpanded;
    }
  }

  // add the definition
  status.GetMakefile().AddDefinition(variable, value);

  return true;
}
