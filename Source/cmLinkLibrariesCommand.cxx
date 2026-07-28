/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLinkLibrariesCommand.h"

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmLinkLibrariesCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_NON_TARGET_DIRECTIVE,
                     "Use of non-target directives is not recommended. "
                     "Consider target_link_libraries instead.");

  if (args.empty()) {
    return true;
  }

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

  return true;
}
