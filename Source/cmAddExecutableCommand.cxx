/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddExecutableCommand.h"

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmAddExecutableCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  auto s = args.begin();

  std::string const& exename = *s;

  ++s;
  bool use_win32 = false;
  bool use_macbundle = false;
  bool excludeFromAll = false;
  bool importTarget = false;
  bool importGlobal = false;
  bool isAlias = false;
  while (s != args.end()) {
    if (*s == "WIN32") {
      ++s;
      use_win32 = true;
    } else if (*s == "MACOSX_BUNDLE") {
      ++s;
      use_macbundle = true;
    } else if (*s == "EXCLUDE_FROM_ALL") {
      ++s;
      excludeFromAll = true;
    } else if (*s == "IMPORTED") {
      ++s;
      importTarget = true;
    } else if (importTarget && *s == "GLOBAL") {
      ++s;
      importGlobal = true;
    } else if (*s == "ALIAS") {
      ++s;
      isAlias = true;
    } else {
      break;
    }
  }

  bool nameOk = cmGeneratorExpression::IsValidTargetName(exename) &&
    !cmGlobalGenerator::IsReservedTarget(exename);

  if (nameOk && !importTarget && !isAlias) {
    nameOk = exename.find(':') == std::string::npos;
  }
  if (!nameOk && !mf.CheckCMP0037(exename, cmStateEnums::EXECUTABLE)) {
    return false;
  }

  // Special modifiers are not allowed with IMPORTED signature.
  if (importTarget && (use_win32 || use_macbundle || excludeFromAll)) {
    if (use_win32) {
      status.SetError("may not be given WIN32 for an IMPORTED target.");
    } else if (use_macbundle) {
      status.SetError(
        "may not be given MACOSX_BUNDLE for an IMPORTED target.");
    } else // if(excludeFromAll)
    {
      status.SetError(
        "may not be given EXCLUDE_FROM_ALL for an IMPORTED target.");
    }
    return false;
  }
  if (isAlias) {
    if (!cmGeneratorExpression::IsValidTargetName(exename)) {
      status.SetError("Invalid name for ALIAS: " + exename);
      return false;
    }
    if (excludeFromAll) {
      status.SetError("EXCLUDE_FROM_ALL with ALIAS makes no sense.");
      return false;
    }
    if (importTarget || importGlobal) {
      status.SetError("IMPORTED with ALIAS is not allowed.");
      return false;
    }
    if (args.size() != 3) {
      status.SetError("ALIAS requires exactly one target argument.");
      return false;
    }

    std::string const& aliasedName = *s;
    if (mf.IsAlias(aliasedName)) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", exename,
                               "\" because target \"", aliasedName,
                               "\" is itself an ALIAS."));
      return false;
    }
    cmTarget* aliasedTarget = mf.FindTargetToUse(aliasedName, true);
    if (!aliasedTarget) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", exename,
                               "\" because target \"", aliasedName,
                               "\" does not already exist."));
      return false;
    }
    cmStateEnums::TargetType type = aliasedTarget->GetType();
    if (type != cmStateEnums::EXECUTABLE) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", exename,
                               "\" because target \"", aliasedName,
                               "\" is not an executable."));
      return false;
    }
    if (aliasedTarget->IsImported() &&
        !aliasedTarget->IsImportedGloballyVisible()) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", exename,
                               "\" because target \"", aliasedName,
                               "\" is imported but not globally visible."));
      return false;
    }
    mf.AddAlias(exename, aliasedName);
    return true;
  }

  // Handle imported target creation.
  if (importTarget) {
    // Make sure the target does not already exist.
    if (mf.FindTargetToUse(exename)) {
      status.SetError(cmStrCat(
        "cannot create imported target \"", exename,
        "\" because another target with the same name already exists."));
      return false;
    }

    // Create the imported target.
    mf.AddImportedTarget(exename, cmStateEnums::EXECUTABLE, importGlobal);
    return true;
  }

  // Enforce name uniqueness.
  {
    std::string msg;
    if (!mf.EnforceUniqueName(exename, msg)) {
      status.SetError(msg);
      return false;
    }
  }

  std::vector<std::string> srclists(s, args.end());
  cmTarget* tgt = mf.AddExecutable(exename, srclists, excludeFromAll);
  if (use_win32) {
    tgt->SetProperty("WIN32_EXECUTABLE", "ON");
  }
  if (use_macbundle) {
    tgt->SetProperty("MACOSX_BUNDLE", "ON");
  }

  return true;
}
