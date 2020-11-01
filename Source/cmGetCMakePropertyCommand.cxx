/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetCMakePropertyCommand.h"

#include <set>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"

// cmGetCMakePropertyCommand
bool cmGetCMakePropertyCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string const& variable = args[0];
  std::string output = "NOTFOUND";

  if (args[1] == "VARIABLES") {
    if (cmProp varsProp = status.GetMakefile().GetProperty("VARIABLES")) {
      output = *varsProp;
    }
  } else if (args[1] == "MACROS") {
    output.clear();
    if (cmProp macrosProp = status.GetMakefile().GetProperty("MACROS")) {
      output = *macrosProp;
    }
  } else if (args[1] == "COMPONENTS") {
    const std::set<std::string>* components =
      status.GetMakefile().GetGlobalGenerator()->GetInstallComponents();
    output = cmJoin(*components, ";");
  } else {
    cmProp prop = nullptr;
    if (!args[1].empty()) {
      prop = status.GetMakefile().GetState()->GetGlobalProperty(args[1]);
    }
    if (prop) {
      output = *prop;
    }
  }

  status.GetMakefile().AddDefinition(variable, output);

  return true;
}
