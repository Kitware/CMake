/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetDirectoryPropertiesCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

namespace {
bool RunCommand(cmMakefile& mf, std::vector<std::string>::const_iterator ait,
                std::vector<std::string>::const_iterator aitend,
                std::string& errors);
}

// cmSetDirectoryPropertiesCommand
bool cmSetDirectoryPropertiesCommand(std::vector<std::string> const& args,
                                     cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string errors;
  bool ret =
    RunCommand(status.GetMakefile(), args.begin() + 1, args.end(), errors);
  if (!ret) {
    status.SetError(errors);
  }
  return ret;
}

namespace {
bool RunCommand(cmMakefile& mf, std::vector<std::string>::const_iterator ait,
                std::vector<std::string>::const_iterator aitend,
                std::string& errors)
{
  for (; ait != aitend; ait += 2) {
    if (ait + 1 == aitend) {
      errors = "Wrong number of arguments";
      return false;
    }
    const std::string& prop = *ait;
    const std::string& value = *(ait + 1);
    if (prop == "VARIABLES") {
      errors = "Variables and cache variables should be set using SET command";
      return false;
    }
    if (prop == "MACROS") {
      errors = "Commands and macros cannot be set using SET_CMAKE_PROPERTIES";
      return false;
    }
    mf.SetProperty(prop, value.c_str());
  }

  return true;
}
}
