/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyComputer.h"

#include <sstream>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"

bool cmTargetPropertyComputer::HandleLocationPropertyPolicy(
  std::string const& tgtName, cmMakefile const& mf)
{
  std::ostringstream e;
  const char* modal = nullptr;
  MessageType messageType = MessageType::AUTHOR_WARNING;
  switch (mf.GetPolicyStatus(cmPolicies::CMP0026)) {
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
    mf.IssueMessage(messageType, e.str());
  }

  return messageType != MessageType::FATAL_ERROR;
}
