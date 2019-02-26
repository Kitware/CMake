/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddDependenciesCommand.h"

#include <sstream>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmTarget.h"

class cmExecutionStatus;

// cmDependenciesCommand
bool cmAddDependenciesCommand::InitialPass(
  std::vector<std::string> const& args, cmExecutionStatus&)
{
  if (args.size() < 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  std::string const& target_name = args[0];
  if (this->Makefile->IsAlias(target_name)) {
    std::ostringstream e;
    e << "Cannot add target-level dependencies to alias target \""
      << target_name << "\".\n";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
  if (cmTarget* target = this->Makefile->FindTargetToUse(target_name)) {

    // skip over target_name
    for (std::string const& arg : cmMakeRange(args).advance(1)) {
      target->AddUtility(arg, this->Makefile);
    }
  } else {
    std::ostringstream e;
    e << "Cannot add target-level dependencies to non-existent target \""
      << target_name << "\".\n"
      << "The add_dependencies works for top-level logical targets created "
      << "by the add_executable, add_library, or add_custom_target commands.  "
      << "If you want to add file-level dependencies see the DEPENDS option "
      << "of the add_custom_target and add_custom_command commands.";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }

  return true;
}
