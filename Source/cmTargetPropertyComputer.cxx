/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyComputer.h"

#include <sstream>

#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmPolicies.h"
#include "cmStateSnapshot.h"

bool cmTargetPropertyComputer::HandleLocationPropertyPolicy(
  std::string const& tgtName, cmMessenger* messenger,
  cmListFileBacktrace const& context)
{
  std::ostringstream e;
  const char* modal = nullptr;
  MessageType messageType = MessageType::AUTHOR_WARNING;
  switch (context.GetBottom().GetPolicy(cmPolicies::CMP0026)) {
    case cmPolicies::WARN:
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0026) << "\n";
      modal = "should";
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      break;
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::NEW:
      modal = "may";
      messageType = MessageType::FATAL_ERROR;
      break;
  }

  if (modal) {
    e << "The LOCATION property " << modal << " not be read from target \""
      << tgtName
      << "\".  Use the target name directly with "
         "add_custom_command, or use the generator expression $<TARGET_FILE>, "
         "as appropriate.\n";
    messenger->IssueMessage(messageType, e.str(), context);
  }

  return messageType != MessageType::FATAL_ERROR;
}
