/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmBreakCommand.h"

#include <sstream>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"

// cmBreakCommand
bool cmBreakCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  if (!status.GetMakefile().IsLoopBlock()) {
    bool issueMessage = true;
    std::ostringstream e;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0055)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0055) << "\n";
        break;
      case cmPolicies::OLD:
        issueMessage = false;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        messageType = MessageType::FATAL_ERROR;
        break;
    }

    if (issueMessage) {
      e << "A BREAK command was found outside of a proper "
           "FOREACH or WHILE loop scope.";
      status.GetMakefile().IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return false;
      }
    }
  }

  status.SetBreakInvoked();

  if (!args.empty()) {
    bool issueMessage = true;
    std::ostringstream e;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0055)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0055) << "\n";
        break;
      case cmPolicies::OLD:
        issueMessage = false;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        messageType = MessageType::FATAL_ERROR;
        break;
    }

    if (issueMessage) {
      e << "The BREAK command does not accept any arguments.";
      status.GetMakefile().IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return false;
      }
    }
  }

  return true;
}
