/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetDirectoryPropertyCommand.h"

#include "cmDirectoryPropertyHelper.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmValue.h"

// cmGetDirectoryPropertyCommand
bool cmGetDirectoryPropertyCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  auto i = args.begin();
  std::string const& variable = *i;
  ++i;

  // Get the directory argument if there is one.
  cmMakefile* mf = &status.GetMakefile();
  if (*i == "DIRECTORY") {
    ++i;
    if (i == args.end()) {
      status.SetError(
        "DIRECTORY argument provided without subsequent arguments");
      return false;
    }
    mf = cmResolveDirectoryMakefile(status.GetMakefile(), *i);
    if (!mf) {
      status.SetError(
        "DIRECTORY argument provided but requested directory not found. "
        "This could be because the directory argument was invalid or, "
        "it is valid but has not been processed yet.");
      return false;
    }
    ++i;
    if (i == args.end()) {
      status.SetError("called with incorrect number of arguments");
      return false;
    }
  }

  if (*i == "DEFINITION") {
    ++i;
    if (i == args.end()) {
      status.SetError("A request for a variable definition was made without "
                      "providing the name of the variable to get.");
      return false;
    }
    status.GetMakefile().AddDefinition(variable, mf->GetSafeDefinition(*i));
    return true;
  }

  if (i->empty()) {
    status.SetError("given empty string for the property name to get");
    return false;
  }

  cmValue prop;
  auto result = cmGetDirectoryProperty(*mf, *i, prop);
  if (result == cmGetDirectoryPropertyResult::Success) {
    status.GetMakefile().AddDefinition(variable, prop);
    return true;
  }
  status.SetError("unknown error retrieving directory property");
  return false;
}
