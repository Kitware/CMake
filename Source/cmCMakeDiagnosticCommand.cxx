/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCMakeDiagnosticCommand.h"

#include <cm/optional>
#include <cmext/string_view>

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"

namespace {
using AlterFunction = bool (cmMakefile::*)(cmDiagnosticCategory,
                                           cmDiagnosticAction, bool);

bool HandleAlterMode(std::vector<std::string> const& args,
                     cmExecutionStatus& status, AlterFunction function,
                     bool recursive)
{
  if (args.size() < 3 || args.size() > 4) {
    status.SetError(cmStrCat(
      args[0], " must be given exactly 2 or 3 additional arguments."_s));
    return false;
  }

  if (args.size() == 4) {
    if (args[3] == "RECURSE") {
      recursive = true;
    } else if (args[3] == "NO_RECURSE") {
      recursive = false;
    } else {
      status.SetError(
        cmStrCat(args[0], " given unknown option \""_s, args[3], "\"."_s));
      return false;
    }
  }

  cm::optional<cmDiagnosticAction> const action =
    cmDiagnostics::GetDiagnosticAction(args[2]);
  if (!action) {
    status.SetError(cmStrCat(
      args[0], " given unrecognized diagnostic action \"", args[2], "\"."_s));
    return false;
  }

  cm::optional<cmDiagnosticCategory> const category =
    cmDiagnostics::GetDiagnosticCategory(args[1]);
  if (!category) {
    status.SetError(cmStrCat(args[0],
                             " given unrecognized diagnostic category \"",
                             args[1], "\"."_s));
    return false;
  }

  if (!(status.GetMakefile().*function)(*category, *action, recursive)) {
    status.SetError(cmStrCat(args[0], " failed to set diagnostic action."_s));
    return false;
  }
  return true;
}

bool HandleGetMode(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("GET must be given exactly 2 additional arguments.");
    return false;
  }

  cm::optional<cmDiagnosticCategory> const category =
    cmDiagnostics::GetDiagnosticCategory(args[1]);
  if (!category) {
    status.SetError(cmStrCat(args[0],
                             " given unrecognized diagnostic category \"",
                             args[1], "\"."_s));
    return false;
  }

  cmDiagnosticAction const action =
    status.GetMakefile().GetDiagnosticAction(*category);

  status.GetMakefile().AddDefinition(args[2],
                                     cmDiagnostics::GetActionString(action));
  return true;
}
}

// cmCMakeDiagnosticCommand
bool cmCMakeDiagnosticCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("requires at least one argument.");
    return false;
  }

  if (args[0] == "SET") {
    return HandleAlterMode(args, status, &cmMakefile::SetDiagnostic, false);
  }
  if (args[0] == "PROMOTE") {
    return HandleAlterMode(args, status, &cmMakefile::PromoteDiagnostic, true);
  }
  if (args[0] == "DEMOTE") {
    return HandleAlterMode(args, status, &cmMakefile::DemoteDiagnostic, true);
  }
  if (args[0] == "GET") {
    return HandleGetMode(args, status);
  }
  if (args[0] == "PUSH") {
    if (args.size() > 1) {
      status.SetError("PUSH may not be given additional arguments.");
      return false;
    }
    status.GetMakefile().PushDiagnostic();
    return true;
  }
  if (args[0] == "POP") {
    if (args.size() > 1) {
      status.SetError("POP may not be given additional arguments.");
      return false;
    }
    status.GetMakefile().PopDiagnostic();
    return true;
  }

  status.SetError(cmStrCat("given unknown first argument \"", args[0], '"'));
  return false;
}
