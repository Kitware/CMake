/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetTargetPropertyCommand.h"

#include <sstream>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmTarget.h"
#include "cmTargetPropertyComputer.h"

class cmMessenger;

bool cmGetTargetPropertyCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string const& var = args[0];
  std::string const& targetName = args[1];
  std::string prop;
  bool prop_exists = false;
  cmMakefile& mf = status.GetMakefile();

  if (cmTarget* tgt = mf.FindTargetToUse(targetName)) {
    if (args[2] == "ALIASED_TARGET") {
      if (mf.IsAlias(targetName)) {
        prop = tgt->GetName();
        prop_exists = true;
      }
    } else if (!args[2].empty()) {
      const char* prop_cstr = nullptr;
      cmListFileBacktrace bt = mf.GetBacktrace();
      cmMessenger* messenger = mf.GetMessenger();
      if (cmTargetPropertyComputer::PassesWhitelist(tgt->GetType(), args[2],
                                                    messenger, bt)) {
        prop_cstr = tgt->GetComputedProperty(args[2], messenger, bt);
        if (!prop_cstr) {
          prop_cstr = tgt->GetProperty(args[2]);
        }
      }
      if (prop_cstr) {
        prop = prop_cstr;
        prop_exists = true;
      }
    }
  } else {
    bool issueMessage = false;
    std::ostringstream e;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (mf.GetPolicyStatus(cmPolicies::CMP0045)) {
      case cmPolicies::WARN:
        issueMessage = true;
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0045) << "\n";
      case cmPolicies::OLD:
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        issueMessage = true;
        messageType = MessageType::FATAL_ERROR;
    }
    if (issueMessage) {
      e << "get_target_property() called with non-existent target \""
        << targetName << "\".";
      mf.IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return false;
      }
    }
  }
  if (prop_exists) {
    mf.AddDefinition(var, prop);
    return true;
  }
  mf.AddDefinition(var, var + "-NOTFOUND");
  return true;
}
