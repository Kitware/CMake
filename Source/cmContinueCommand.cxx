/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmContinueCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

// cmContinueCommand
bool cmContinueCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (!status.GetMakefile().IsLoopBlock()) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      "A CONTINUE command was found outside of a "
      "proper FOREACH or WHILE loop scope.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
  }

  status.SetContinueInvoked();

  if (!args.empty()) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      "The CONTINUE command does not accept any "
      "arguments.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
  }

  return true;
}
