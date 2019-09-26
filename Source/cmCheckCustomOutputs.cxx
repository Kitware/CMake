/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCheckCustomOutputs.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmCheckCustomOutputs(const std::vector<std::string>& outputs,
                          cm::string_view keyword, cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  for (std::string const& o : outputs) {
    // Make sure the file will not be generated into the source
    // directory during an out of source build.
    if (!mf.CanIWriteThisFile(o)) {
      status.SetError(
        cmStrCat("attempted to have a file\n  ", o,
                 "\nin a source directory as an output of custom command."));
      cmSystemTools::SetFatalErrorOccured();
      return false;
    }

    // Make sure the output file name has no invalid characters.
    std::string::size_type pos = o.find_first_of("#<>");
    if (pos != std::string::npos) {
      status.SetError(cmStrCat("called with ", keyword, " containing a \"",
                               o[pos], "\".  This character is not allowed."));
      return false;
    }
  }

  return true;
}
