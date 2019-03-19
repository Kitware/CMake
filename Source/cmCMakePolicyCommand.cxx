/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePolicyCommand.h"

#include <sstream>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateTypes.h"

class cmExecutionStatus;

// cmCMakePolicyCommand
bool cmCMakePolicyCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("requires at least one argument.");
    return false;
  }

  if (args[0] == "SET") {
    return this->HandleSetMode(args);
  }
  if (args[0] == "GET") {
    return this->HandleGetMode(args);
  }
  if (args[0] == "PUSH") {
    if (args.size() > 1) {
      this->SetError("PUSH may not be given additional arguments.");
      return false;
    }
    this->Makefile->PushPolicy();
    return true;
  }
  if (args[0] == "POP") {
    if (args.size() > 1) {
      this->SetError("POP may not be given additional arguments.");
      return false;
    }
    this->Makefile->PopPolicy();
    return true;
  }
  if (args[0] == "VERSION") {
    return this->HandleVersionMode(args);
  }
  if (args[0] == "GET_WARNING") {
    return this->HandleGetWarningMode(args);
  }

  std::ostringstream e;
  e << "given unknown first argument \"" << args[0] << "\"";
  this->SetError(e.str());
  return false;
}

bool cmCMakePolicyCommand::HandleSetMode(std::vector<std::string> const& args)
{
  if (args.size() != 3) {
    this->SetError("SET must be given exactly 2 additional arguments.");
    return false;
  }

  cmPolicies::PolicyStatus status;
  if (args[2] == "OLD") {
    status = cmPolicies::OLD;
  } else if (args[2] == "NEW") {
    status = cmPolicies::NEW;
  } else {
    std::ostringstream e;
    e << "SET given unrecognized policy status \"" << args[2] << "\"";
    this->SetError(e.str());
    return false;
  }

  if (!this->Makefile->SetPolicy(args[1].c_str(), status)) {
    this->SetError("SET failed to set policy.");
    return false;
  }
  if (args[1] == "CMP0001" &&
      (status == cmPolicies::WARN || status == cmPolicies::OLD)) {
    if (!(this->Makefile->GetState()->GetInitializedCacheValue(
          "CMAKE_BACKWARDS_COMPATIBILITY"))) {
      // Set it to 2.4 because that is the last version where the
      // variable had meaning.
      this->Makefile->AddCacheDefinition(
        "CMAKE_BACKWARDS_COMPATIBILITY", "2.4",
        "For backwards compatibility, what version of CMake "
        "commands and "
        "syntax should this version of CMake try to support.",
        cmStateEnums::STRING);
    }
  }
  return true;
}

bool cmCMakePolicyCommand::HandleGetMode(std::vector<std::string> const& args)
{
  bool parent_scope = false;
  if (args.size() == 4 && args[3] == "PARENT_SCOPE") {
    // Undocumented PARENT_SCOPE option for use within CMake.
    parent_scope = true;
  } else if (args.size() != 3) {
    this->SetError("GET must be given exactly 2 additional arguments.");
    return false;
  }

  // Get arguments.
  std::string const& id = args[1];
  std::string const& var = args[2];

  // Lookup the policy number.
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id.c_str(), pid)) {
    std::ostringstream e;
    e << "GET given policy \"" << id << "\" which is not known to this "
      << "version of CMake.";
    this->SetError(e.str());
    return false;
  }

  // Lookup the policy setting.
  cmPolicies::PolicyStatus status =
    this->Makefile->GetPolicyStatus(pid, parent_scope);
  switch (status) {
    case cmPolicies::OLD:
      // Report that the policy is set to OLD.
      this->Makefile->AddDefinition(var, "OLD");
      break;
    case cmPolicies::WARN:
      // Report that the policy is not set.
      this->Makefile->AddDefinition(var, "");
      break;
    case cmPolicies::NEW:
      // Report that the policy is set to NEW.
      this->Makefile->AddDefinition(var, "NEW");
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      // The policy is required to be set before anything needs it.
      {
        std::ostringstream e;
        e << cmPolicies::GetRequiredPolicyError(pid) << "\n"
          << "The call to cmake_policy(GET " << id << " ...) at which this "
          << "error appears requests the policy, and this version of CMake "
          << "requires that the policy be set to NEW before it is checked.";
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      }
  }

  return true;
}

bool cmCMakePolicyCommand::HandleVersionMode(
  std::vector<std::string> const& args)
{
  if (args.size() <= 1) {
    this->SetError("VERSION not given an argument");
    return false;
  }
  if (args.size() >= 3) {
    this->SetError("VERSION given too many arguments");
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
    std::ostringstream e;
    e << "VERSION \"" << version_string
      << "\" does not have a version on both sides of \"...\".";
    this->SetError(e.str());
    return false;
  }

  this->Makefile->SetPolicyVersion(version_min, version_max);
  return true;
}

bool cmCMakePolicyCommand::HandleGetWarningMode(
  std::vector<std::string> const& args)
{
  if (args.size() != 3) {
    this->SetError(
      "GET_WARNING must be given exactly 2 additional arguments.");
    return false;
  }

  // Get arguments.
  std::string const& id = args[1];
  std::string const& var = args[2];

  // Lookup the policy number.
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id.c_str(), pid)) {
    std::ostringstream e;
    e << "GET_WARNING given policy \"" << id
      << "\" which is not known to this version of CMake.";
    this->SetError(e.str());
    return false;
  }

  // Lookup the policy warning.
  this->Makefile->AddDefinition(var,
                                cmPolicies::GetPolicyWarning(pid).c_str());

  return true;
}
