/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallTargetsCommand.h"

#include <unordered_map>
#include <utility>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

bool cmInstallTargetsCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Enable the install target.
  mf.GetGlobalGenerator()->EnableInstallTarget();

  cmMakefile::cmTargetMap& tgts = mf.GetTargets();
  auto s = args.begin();
  ++s;
  std::string runtime_dir = "/bin";
  for (; s != args.end(); ++s) {
    if (*s == "RUNTIME_DIRECTORY") {
      ++s;
      if (s == args.end()) {
        status.SetError("called with RUNTIME_DIRECTORY but no actual "
                        "directory");
        return false;
      }

      runtime_dir = *s;
    } else {
      auto ti = tgts.find(*s);
      if (ti != tgts.end()) {
        ti->second.SetInstallPath(args[0]);
        ti->second.SetRuntimeInstallPath(runtime_dir);
        ti->second.SetHaveInstallRule(true);
      } else {
        std::string str = "Cannot find target: \"" + *s + "\" to install.";
        status.SetError(str);
        return false;
      }
    }
  }

  mf.GetGlobalGenerator()->AddInstallComponent(
    mf.GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME"));

  return true;
}
