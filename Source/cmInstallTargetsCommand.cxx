/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallTargetsCommand.h"

#include <unordered_map>
#include <utility>

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmInstallTargetsCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_DEPRECATED,
                     "The 'install_targets' command has been superseded. "
                     "Use 'install(TARGETS)' instead.");

  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // Enable the install target.
  mf.GetGlobalGenerator()->EnableInstallTarget();

  cmMakefile::cmTargetMap& tgts = mf.GetTargets();
  auto s = args.begin();
  ++s;
  std::string runtimeDir = "/bin";
  for (; s != args.end(); ++s) {
    if (*s == "RUNTIME_DIRECTORY") {
      ++s;
      if (s == args.end()) {
        status.SetError("called with RUNTIME_DIRECTORY but no actual "
                        "directory");
        return false;
      }

      runtimeDir = *s;
    } else {
      auto ti = tgts.find(*s);
      if (ti != tgts.end()) {
        ti->second.SetInstallPath(args[0]);
        ti->second.SetRuntimeInstallPath(runtimeDir);
        ti->second.SetHaveInstallRule(true);
      } else {
        status.SetError(
          cmStrCat("Cannot find target: \"", *s, "\" to install."));
        return false;
      }
    }
  }

  mf.GetGlobalGenerator()->AddInstallComponent(
    mf.GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME"));

  return true;
}
