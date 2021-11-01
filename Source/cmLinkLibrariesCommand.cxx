/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLinkLibrariesCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmLinkLibrariesCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.empty()) {
    return true;
  }
  cmMakefile& mf = status.GetMakefile();
  // add libraries, note that there is an optional prefix
  // of debug and optimized than can be used
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "debug") {
      ++i;
      if (i == args.end()) {
        status.SetError("The \"debug\" argument must be followed by "
                        "a library");
        return false;
      }
      mf.AppendProperty("LINK_LIBRARIES", "debug");
    } else if (*i == "optimized") {
      ++i;
      if (i == args.end()) {
        status.SetError("The \"optimized\" argument must be followed by "
                        "a library");
        return false;
      }
      mf.AppendProperty("LINK_LIBRARIES", "optimized");
    }
    mf.AppendProperty("LINK_LIBRARIES", *i);
  }

  mf.CheckProperty("LINK_LIBRARIES");

  return true;
}
