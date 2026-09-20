/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSubdirCommand.h"

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmSubdirCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                     "The 'subdirs' command has been superseded. "
                     "Use the 'add_subdirectory' command instead.");

  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  bool res = true;
  bool excludeFromAll = false;

  for (std::string const& i : args) {
    if (i == "EXCLUDE_FROM_ALL") {
      excludeFromAll = true;
      continue;
    }
    if (i == "PREORDER") {
      // Ignored
      continue;
    }

    // if they specified a relative path then compute the full
    std::string srcPath = cmStrCat(mf.GetCurrentSourceDirectory(), '/', i);
    if (cmSystemTools::FileIsDirectory(srcPath)) {
      std::string binPath = cmStrCat(mf.GetCurrentBinaryDirectory(), '/', i);
      mf.AddSubDirectory(srcPath, binPath, excludeFromAll, false, false);
    }
    // otherwise it is a full path
    else if (cmSystemTools::FileIsDirectory(i)) {
      // we must compute the binPath from the srcPath, we just take the last
      // element from the source path and use that
      std::string binPath = cmStrCat(mf.GetCurrentBinaryDirectory(), '/',
                                     cmSystemTools::GetFilenameName(i));
      mf.AddSubDirectory(i, binPath, excludeFromAll, false, false);
    } else {
      status.SetError(cmStrCat("Incorrect SUBDIRS command. Directory: ", i,
                               " does not exist."));
      res = false;
    }
  }
  return res;
}
