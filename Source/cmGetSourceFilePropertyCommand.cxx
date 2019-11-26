/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetSourceFilePropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"

bool cmGetSourceFilePropertyCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string const& var = args[0];
  std::string const& file = args[1];
  cmMakefile& mf = status.GetMakefile();
  cmSourceFile* sf = mf.GetSource(file);

  // for the location we must create a source file first
  if (!sf && args[2] == "LOCATION") {
    sf = mf.CreateSource(file);
  }
  if (sf) {
    const char* prop = nullptr;
    if (!args[2].empty()) {
      prop = sf->GetPropertyForUser(args[2]);
    }
    if (prop) {
      mf.AddDefinition(var, prop);
      return true;
    }
  }

  mf.AddDefinition(var, "NOTFOUND");
  return true;
}
