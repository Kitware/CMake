/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakeDirectoryCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

// cmMakeDirectoryCommand
bool cmMakeDirectoryCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() != 1) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  if (!status.GetMakefile().CanIWriteThisFile(args[0])) {
    std::string e = "attempted to create a directory: " + args[0] +
      " into a source directory.";
    status.SetError(e);
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  cmSystemTools::MakeDirectory(args[0]);
  return true;
}
