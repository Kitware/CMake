/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetTestPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSetPropertyCommand.h"
#include "cmTest.h"
#include "cmValue.h"

bool cmGetTestPropertyCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  std::vector<std::string>::size_type args_size = args.size();
  if (args_size != 3 && args_size != 5) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string test_directory;
  bool test_directory_option_enabled = false;

  int var_arg_index = 2;
  if (args[2] == "DIRECTORY" && args_size == 5) {
    var_arg_index = 4;
    test_directory_option_enabled = true;
    test_directory = args[3];
  }

  cmMakefile* test_directory_makefile = &status.GetMakefile();
  bool file_scopes_handled =
    SetPropertyCommand::HandleAndValidateTestDirectoryScopes(
      status, test_directory_option_enabled, test_directory,
      test_directory_makefile);
  if (!file_scopes_handled) {
    return false;
  }

  std::string const& testName = args[0];
  std::string const& var = args[var_arg_index];
  cmMakefile& mf = status.GetMakefile();
  cmTest* test = test_directory_makefile->GetTest(testName);
  if (test) {
    cmValue prop;
    if (!args[1].empty()) {
      prop = test->GetProperty(args[1]);
    }
    if (prop) {
      mf.AddDefinition(var, prop);
      return true;
    }
  }
  mf.AddDefinition(var, "NOTFOUND");
  return true;
}
