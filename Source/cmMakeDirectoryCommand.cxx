/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmMakeDirectoryCommand.h"

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmMakeDirectoryCommand
bool cmMakeDirectoryCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                     "The 'make_directory' command has been superseded. "
                     "Use 'file(MAKE_DIRECTORY)' instead.");

  if (args.size() != 1) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  if (!mf.CanIWriteThisFile(args[0])) {
    std::string e = cmStrCat("attempted to create a directory: ", args[0],
                             " into a source directory.");
    status.SetError(e);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  cmSystemTools::MakeDirectory(args[0]);
  return true;
}
