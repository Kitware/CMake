/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetTestPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmTest.h"
#include "cmValue.h"

bool cmGetTestPropertyCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string const& testName = args[0];
  std::string const& var = args[2];
  cmMakefile& mf = status.GetMakefile();
  cmTest* test = mf.GetTest(testName);
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
