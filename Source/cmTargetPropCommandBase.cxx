/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetPropCommandBase.h"

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

cmTargetPropCommandBase::cmTargetPropCommandBase(cmExecutionStatus& status)
  : Makefile(&status.GetMakefile())
  , Status(status)
{
}

void cmTargetPropCommandBase::SetError(std::string const& e)
{
  this->Status.SetError(e);
}

bool cmTargetPropCommandBase::HandleArguments(
  std::vector<std::string> const& args, const std::string& prop,
  ArgumentFlags flags)
{
  if (args.size() < 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  if (this->Makefile->IsAlias(args[0])) {
    this->SetError("can not be used on an ALIAS target.");
    return false;
  }
  // Lookup the target for which property-values are specified.
  this->Target =
    this->Makefile->GetCMakeInstance()->GetGlobalGenerator()->FindTarget(
      args[0]);
  if (!this->Target) {
    this->Target = this->Makefile->FindTargetToUse(args[0]);
  }
  if (!this->Target) {
    this->HandleMissingTarget(args[0]);
    return false;
  }
  const bool isRegularTarget =
    (this->Target->GetType() == cmStateEnums::EXECUTABLE) ||
    (this->Target->GetType() == cmStateEnums::STATIC_LIBRARY) ||
    (this->Target->GetType() == cmStateEnums::SHARED_LIBRARY) ||
    (this->Target->GetType() == cmStateEnums::MODULE_LIBRARY) ||
    (this->Target->GetType() == cmStateEnums::OBJECT_LIBRARY) ||
    (this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY) ||
    (this->Target->GetType() == cmStateEnums::UNKNOWN_LIBRARY);
  const bool isCustomTarget = this->Target->GetType() == cmStateEnums::UTILITY;

  if (prop == "SOURCES") {
    if (!isRegularTarget && !isCustomTarget) {
      this->SetError("called with non-compilable target type");
      return false;
    }
  } else {
    if (!isRegularTarget) {
      this->SetError("called with non-compilable target type");
      return false;
    }
  }

  bool system = false;
  unsigned int argIndex = 1;

  if ((flags & PROCESS_SYSTEM) && args[argIndex] == "SYSTEM") {
    if (args.size() < 3) {
      this->SetError("called with incorrect number of arguments");
      return false;
    }
    system = true;
    ++argIndex;
  }

  bool prepend = false;
  if ((flags & PROCESS_BEFORE) && args[argIndex] == "BEFORE") {
    if (args.size() < 3) {
      this->SetError("called with incorrect number of arguments");
      return false;
    }
    prepend = true;
    ++argIndex;
  } else if ((flags & PROCESS_AFTER) && args[argIndex] == "AFTER") {
    if (args.size() < 3) {
      this->SetError("called with incorrect number of arguments");
      return false;
    }
    prepend = false;
    ++argIndex;
  }

  if ((flags & PROCESS_REUSE_FROM) && args[argIndex] == "REUSE_FROM") {
    if (args.size() != 3) {
      this->SetError("called with incorrect number of arguments");
      return false;
    }
    ++argIndex;

    this->Target->SetProperty("PRECOMPILE_HEADERS_REUSE_FROM", args[argIndex]);
    ++argIndex;
  }

  this->Property = prop;

  while (argIndex < args.size()) {
    if (!this->ProcessContentArgs(args, argIndex, prepend, system)) {
      return false;
    }
  }
  return true;
}

bool cmTargetPropCommandBase::ProcessContentArgs(
  std::vector<std::string> const& args, unsigned int& argIndex, bool prepend,
  bool system)
{
  std::string const& scope = args[argIndex];

  if (scope != "PUBLIC" && scope != "PRIVATE" && scope != "INTERFACE") {
    this->SetError("called with invalid arguments");
    return false;
  }

  ++argIndex;

  std::vector<std::string> content;

  for (unsigned int i = argIndex; i < args.size(); ++i, ++argIndex) {
    if (args[i] == "PUBLIC" || args[i] == "PRIVATE" ||
        args[i] == "INTERFACE") {
      break;
    }
    content.push_back(args[i]);
  }
  if (!content.empty()) {
    if (this->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY &&
        scope != "INTERFACE" && this->Property != "SOURCES") {
      this->SetError("may only set INTERFACE properties on INTERFACE targets");
      return false;
    }
    if (this->Target->IsImported() && scope != "INTERFACE") {
      this->SetError("may only set INTERFACE properties on IMPORTED targets");
      return false;
    }
    if (this->Target->GetType() == cmStateEnums::UTILITY &&
        scope != "PRIVATE") {
      this->SetError("may only set PRIVATE properties on custom targets");
      return false;
    }
  }
  return this->PopulateTargetProperties(scope, content, prepend, system);
}

bool cmTargetPropCommandBase::PopulateTargetProperties(
  const std::string& scope, const std::vector<std::string>& content,
  bool prepend, bool system)
{
  if (content.empty()) {
    return true;
  }
  if (scope == "PRIVATE" || scope == "PUBLIC") {
    if (!this->HandleDirectContent(this->Target, content, prepend, system)) {
      return false;
    }
  }
  if (scope == "INTERFACE" || scope == "PUBLIC") {
    this->HandleInterfaceContent(this->Target, content, prepend, system);
  }
  return true;
}

void cmTargetPropCommandBase::HandleInterfaceContent(
  cmTarget* tgt, const std::vector<std::string>& content, bool prepend, bool)
{
  if (prepend) {
    const std::string propName = std::string("INTERFACE_") + this->Property;
    cmValue propValue = tgt->GetProperty(propName);
    const std::string totalContent =
      this->Join(content) + (propValue ? (";" + *propValue) : std::string());
    tgt->SetProperty(propName, totalContent);
  } else {
    tgt->AppendProperty("INTERFACE_" + this->Property, this->Join(content));
  }
}
