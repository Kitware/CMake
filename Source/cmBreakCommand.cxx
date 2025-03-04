/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmBreakCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"

// cmBreakCommand
bool cmBreakCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  if (!status.GetMakefile().IsLoopBlock()) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      "A BREAK command was found outside of a proper "
      "FOREACH or WHILE loop scope.");
    return false;
  }

  status.SetBreakInvoked();

  if (!args.empty()) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      "The BREAK command does not accept any arguments.");
    return false;
  }

  return true;
}
