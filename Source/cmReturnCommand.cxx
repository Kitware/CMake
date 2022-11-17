/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmReturnCommand.h"

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmReturnCommand
bool cmReturnCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  if (!args.empty()) {
    switch (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0140)) {
      case cmPolicies::WARN:
        status.GetMakefile().IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat(
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0140), '\n',
            "return() checks its arguments when the policy is set to NEW. "
            "Since the policy is not set the OLD behavior will be used so "
            "the arguments will be ignored."));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        return true;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat('\n', cmPolicies::GetPolicyWarning(cmPolicies::CMP0140)));
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      default:
        break;
    }
    if (args[0] != "PROPAGATE"_s) {
      status.SetError(
        cmStrCat("called with unsupported argument \"", args[0], '"'));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    status.SetReturnInvoked({ args.begin() + 1, args.end() });
  } else {
    status.SetReturnInvoked();
  }
  return true;
}
