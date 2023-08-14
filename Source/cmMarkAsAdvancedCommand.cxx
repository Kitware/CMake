/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMarkAsAdvancedCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

// cmMarkAsAdvancedCommand
bool cmMarkAsAdvancedCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  unsigned int i = 0;
  const char* value = "1";
  bool overwrite = false;
  if (args[0] == "CLEAR" || args[0] == "FORCE") {
    overwrite = true;
    if (args[0] == "CLEAR") {
      value = "0";
    }
    i = 1;
  }

  cmMakefile& mf = status.GetMakefile();
  cmState* state = mf.GetState();

  for (; i < args.size(); ++i) {
    std::string const& variable = args[i];

    bool issueMessage = false;
    bool oldBehavior = false;
    bool ignoreVariable = false;
    switch (mf.GetPolicyStatus(cmPolicies::CMP0102)) {
      case cmPolicies::WARN:
        if (mf.PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0102")) {
          if (!state->GetCacheEntryValue(variable)) {
            issueMessage = true;
          }
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        oldBehavior = true;
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        if (!state->GetCacheEntryValue(variable)) {
          ignoreVariable = true;
        }
        break;
    }

    // First see if we should issue a message about CMP0102
    if (issueMessage) {
      std::string err = cmStrCat(
        "Policy CMP0102 is not set: The variable named \"", variable,
        "\" is not in the cache. This results in an empty cache entry which "
        "is no longer created when policy CMP0102 is set to NEW. Run \"cmake "
        "--help-policy CMP0102\" for policy details. Use the cmake_policy "
        "command to set the policy and suppress this warning.");
      mf.IssueMessage(MessageType::AUTHOR_WARNING, err);
    }

    // If it's not in the cache and we're using the new behavior, nothing to
    // see here.
    if (ignoreVariable) {
      continue;
    }

    // Check if we want the old behavior of making a dummy cache entry.
    if (oldBehavior) {
      if (!state->GetCacheEntryValue(variable)) {
        status.GetMakefile().GetCMakeInstance()->AddCacheEntry(
          variable, cmValue{ nullptr }, cmValue{ nullptr },
          cmStateEnums::UNINITIALIZED);
        overwrite = true;
      }
    }

    // We need a cache entry to do this.
    if (!state->GetCacheEntryValue(variable)) {
      cmSystemTools::Error("This should never happen...");
      return false;
    }
    if (!state->GetCacheEntryProperty(variable, "ADVANCED") || overwrite) {
      state->SetCacheEntryProperty(variable, "ADVANCED", value);
    }
  }
  return true;
}
