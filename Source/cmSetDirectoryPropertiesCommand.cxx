/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetDirectoryPropertiesCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

// cmSetDirectoryPropertiesCommand
bool cmSetDirectoryPropertiesCommand(std::vector<std::string> const& args,
                                     cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // PROPERTIES followed by prop value pairs
  if (args.size() % 2 != 1) {
    status.SetError("Wrong number of arguments");
    return false;
  }

  for (auto iter = args.begin() + 1; iter != args.end(); iter += 2) {
    const std::string& prop = *iter;
    if (prop == "VARIABLES") {
      status.SetError(
        "Variables and cache variables should be set using SET command");
      return false;
    }
    if (prop == "MACROS") {
      status.SetError(
        "Commands and macros cannot be set using SET_CMAKE_PROPERTIES");
      return false;
    }
    status.GetMakefile().SetProperty(prop, *(iter + 1));
  }

  return true;
}
