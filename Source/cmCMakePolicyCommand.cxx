/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCMakePolicyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"

namespace {
bool HandleSetMode(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
bool HandleGetMode(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
bool HandleVersionMode(std::vector<std::string> const& args,
                       cmExecutionStatus& status);
bool HandleGetWarningMode(std::vector<std::string> const& args,
                          cmExecutionStatus& status);
}

// cmCMakePolicyCommand
bool cmCMakePolicyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("requires at least one argument.");
    return false;
  }

  if (args[0] == "SET") {
    return HandleSetMode(args, status);
  }
  if (args[0] == "GET") {
    return HandleGetMode(args, status);
  }
  if (args[0] == "PUSH") {
    if (args.size() > 1) {
      status.SetError("PUSH may not be given additional arguments.");
      return false;
    }
    status.GetMakefile().PushPolicy();
    return true;
  }
  if (args[0] == "POP") {
    if (args.size() > 1) {
      status.SetError("POP may not be given additional arguments.");
      return false;
    }
    status.GetMakefile().PopPolicy();
    return true;
  }
  if (args[0] == "VERSION") {
    return HandleVersionMode(args, status);
  }
  if (args[0] == "GET_WARNING") {
    return HandleGetWarningMode(args, status);
  }

  status.SetError(cmStrCat("given unknown first argument \"", args[0], "\""));
  return false;
}

namespace {

bool HandleSetMode(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("SET must be given exactly 2 additional arguments.");
    return false;
  }

  cmPolicies::PolicyStatus policyStatus;
  if (args[2] == "OLD") {
    policyStatus = cmPolicies::OLD;
  } else if (args[2] == "NEW") {
    policyStatus = cmPolicies::NEW;
  } else {
    status.SetError(
      cmStrCat("SET given unrecognized policy status \"", args[2], "\""));
    return false;
  }

  if (!status.GetMakefile().SetPolicy(args[1].c_str(), policyStatus)) {
    status.SetError("SET failed to set policy.");
    return false;
  }
  return true;
}

bool HandleGetMode(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  bool parent_scope = false;
  if (args.size() == 4 && args[3] == "PARENT_SCOPE") {
    // Undocumented PARENT_SCOPE option for use within CMake.
    parent_scope = true;
  } else if (args.size() != 3) {
    status.SetError("GET must be given exactly 2 additional arguments.");
    return false;
  }

  // Get arguments.
  std::string const& id = args[1];
  std::string const& var = args[2];

  // Lookup the policy number.
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id.c_str(), pid)) {
    status.SetError(
      cmStrCat("GET given policy \"", id,
               "\" which is not known to this version of CMake."));
    return false;
  }

  // Lookup the policy setting.
  cmPolicies::PolicyStatus policyStatus =
    status.GetMakefile().GetPolicyStatus(pid, parent_scope);
  switch (policyStatus) {
    case cmPolicies::OLD:
      // Report that the policy is set to OLD.
      status.GetMakefile().AddDefinition(var, "OLD");
      break;
    case cmPolicies::WARN:
      // Report that the policy is not set.
      status.GetMakefile().AddDefinition(var, "");
      break;
    case cmPolicies::NEW:
      // Report that the policy is set to NEW.
      status.GetMakefile().AddDefinition(var, "NEW");
      break;
  }

  return true;
}

bool HandleVersionMode(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() <= 1) {
    status.SetError("VERSION not given an argument");
    return false;
  }
  if (args.size() >= 3) {
    status.SetError("VERSION given too many arguments");
    return false;
  }
  std::string const& version_string = args[1];

  // Separate the <min> version and any trailing ...<max> component.
  std::string::size_type const dd = version_string.find("...");
  std::string const version_min = version_string.substr(0, dd);
  std::string const version_max = dd != std::string::npos
    ? version_string.substr(dd + 3, std::string::npos)
    : std::string();
  if (dd != std::string::npos &&
      (version_min.empty() || version_max.empty())) {
    status.SetError(
      cmStrCat("VERSION \"", version_string,
               R"(" does not have a version on both sides of "...".)"));
    return false;
  }

  return status.GetMakefile().SetPolicyVersion(version_min, version_max);
}

bool HandleGetWarningMode(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError(
      "GET_WARNING must be given exactly 2 additional arguments.");
    return false;
  }

  // Get arguments.
  std::string const& id = args[1];
  std::string const& var = args[2];

  // Lookup the policy number.
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id.c_str(), pid)) {
    status.SetError(
      cmStrCat("GET_WARNING given policy \"", id,
               "\" which is not known to this version of CMake."));
    return false;
  }

  // Lookup the policy warning.
  status.GetMakefile().AddDefinition(var, cmPolicies::GetPolicyWarning(pid));

  return true;
}
}
