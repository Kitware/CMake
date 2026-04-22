/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetTestPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmTestPropertyHelper.h"
#include "cmValue.h"

bool cmGetTestPropertyCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  std::vector<std::string>::size_type args_size = args.size();
  if (args_size != 3 && args_size != 5) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string testDirectory;
  bool testDirectoryOptionEnabled = false;

  int var_arg_index = 2;
  if (args[2] == "DIRECTORY" && args_size == 5) {
    var_arg_index = 4;
    testDirectoryOptionEnabled = true;
    testDirectory = args[3];
  }

  std::string const& testName = args[0];
  std::string const& propertyName = args[1];
  std::string const& var = args[var_arg_index];

  cmValue prop;
  auto result =
    cmGetTestProperty(testName, propertyName, status,
                      testDirectoryOptionEnabled, testDirectory, prop);
  if (result == cmGetTestPropertyResult::Success) {
    if (prop) {
      status.GetMakefile().AddDefinition(var, prop);
      return true;
    }
    status.GetMakefile().AddDefinition(var, "NOTFOUND");
    return true;
  }
  if (result == cmGetTestPropertyResult::TestNotFound) {
    status.GetMakefile().AddDefinition(var, "NOTFOUND");
    return true;
  }
  return false;
}
