/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOptionCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

// cmOptionCommand
bool cmOptionCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  const bool argError = (args.size() < 2) || (args.size() > 3);
  if (argError) {
    std::string m = cmStrCat("called with incorrect number of arguments: ",
                             cmJoin(args, " "));
    status.SetError(m);
    return false;
  }

  // Determine the state of the option policy
  bool checkAndWarn = false;
  {
    auto policyStatus =
      status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0077);
    const auto& existsBeforeSet =
      status.GetMakefile().GetStateSnapshot().GetDefinition(args[0]);
    switch (policyStatus) {
      case cmPolicies::WARN:
        checkAndWarn = (existsBeforeSet != nullptr);
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior does not warn.
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW: {
        // See if a local variable with this name already exists.
        // If so we ignore the option command.
        if (existsBeforeSet) {
          return true;
        }
      } break;
    }
  }

  // See if a cache variable with this name already exists
  // If so just make sure the doc state is correct
  cmState* state = status.GetMakefile().GetState();
  cmValue existingValue = state->GetCacheEntryValue(args[0]);
  if (existingValue &&
      (state->GetCacheEntryType(args[0]) != cmStateEnums::UNINITIALIZED)) {
    state->SetCacheEntryProperty(args[0], "HELPSTRING", args[1]);
    return true;
  }

  // Nothing in the cache so add it
  std::string initialValue = existingValue ? *existingValue : "Off";
  if (args.size() == 3) {
    initialValue = args[2];
  }
  bool init = cmIsOn(initialValue);
  status.GetMakefile().AddCacheDefinition(args[0], init ? "ON" : "OFF",
                                          args[1].c_str(), cmStateEnums::BOOL);
  if (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0077) !=
        cmPolicies::NEW &&
      status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0126) ==
        cmPolicies::NEW) {
    // if there was a definition then remove it
    status.GetMakefile().GetStateSnapshot().RemoveDefinition(args[0]);
  }

  if (checkAndWarn) {
    const auto& existsAfterSet =
      status.GetMakefile().GetStateSnapshot().GetDefinition(args[0]);
    if (!existsAfterSet) {
      status.GetMakefile().IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0077),
                 "\n"
                 "For compatibility with older versions of CMake, option "
                 "is clearing the normal variable '",
                 args[0], "'."));
    }
  }
  return true;
}
