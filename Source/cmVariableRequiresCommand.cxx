/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVariableRequiresCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmLibraryCommand
bool cmVariableRequiresCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string const& testVariable = args[0];
  if (!status.GetMakefile().IsOn(testVariable)) {
    return true;
  }
  std::string const& resultVariable = args[1];
  bool requirementsMet = true;
  std::string notSet;
  bool hasAdvanced = false;
  cmState* state = status.GetMakefile().GetState();
  for (unsigned int i = 2; i < args.size(); ++i) {
    if (!status.GetMakefile().IsOn(args[i])) {
      requirementsMet = false;
      notSet += args[i];
      notSet += "\n";
      if (state->GetCacheEntryValue(args[i]) &&
          state->GetCacheEntryPropertyAsBool(args[i], "ADVANCED")) {
        hasAdvanced = true;
      }
    }
  }
  const char* reqVar = status.GetMakefile().GetDefinition(resultVariable);
  // if reqVar is unset, then set it to requirementsMet
  // if reqVar is set to true, but requirementsMet is false , then
  // set reqVar to false.
  if (!reqVar || (!requirementsMet && status.GetMakefile().IsOn(reqVar))) {
    status.GetMakefile().AddDefinitionBool(resultVariable, requirementsMet);
  }

  if (!requirementsMet) {
    std::string message =
      cmStrCat("Variable assertion failed:\n", testVariable,
               " Requires that the following unset variables are set:\n",
               notSet, "\nPlease set them, or set ", testVariable,
               " to false, and re-configure.\n");
    if (hasAdvanced) {
      message +=
        "One or more of the required variables is advanced."
        "  To set the variable, you must turn on advanced mode in cmake.";
    }
    cmSystemTools::Error(message);
  }

  return true;
}
