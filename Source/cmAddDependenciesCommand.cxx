/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddDependenciesCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmAddDependenciesCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  std::string const& target_name = args[0];
  if (mf.IsAlias(target_name)) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot add target-level dependencies to alias target \"",
               target_name, "\".\n"));
  }
  if (cmTarget* target = mf.FindTargetToUse(target_name)) {

    // skip over target_name
    for (std::string const& arg : cmMakeRange(args).advance(1)) {
      target->AddUtility(arg, false, &mf);
    }
  } else {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(
        "Cannot add target-level dependencies to non-existent "
        "target \"",
        target_name,
        "\".\nThe add_dependencies works for "
        "top-level logical targets created by the add_executable, "
        "add_library, or add_custom_target commands.  If you want to add "
        "file-level dependencies see the DEPENDS option of the "
        "add_custom_target and add_custom_command commands."));
  }

  return true;
}
