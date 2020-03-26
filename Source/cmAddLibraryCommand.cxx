/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddLibraryCommand.h"

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

bool cmAddLibraryCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  // Library type defaults to value of BUILD_SHARED_LIBS, if it exists,
  // otherwise it defaults to static library.
  cmStateEnums::TargetType type = cmStateEnums::SHARED_LIBRARY;
  if (cmIsOff(mf.GetDefinition("BUILD_SHARED_LIBS"))) {
    type = cmStateEnums::STATIC_LIBRARY;
  }
  bool excludeFromAll = false;
  bool importTarget = false;
  bool importGlobal = false;

  auto s = args.begin();

  std::string const& libName = *s;

  ++s;

  // If the second argument is "SHARED" or "STATIC", then it controls
  // the type of library.  Otherwise, it is treated as a source or
  // source list name. There may be two keyword arguments, check for them
  bool haveSpecifiedType = false;
  bool isAlias = false;
  while (s != args.end()) {
    std::string libType = *s;
    if (libType == "STATIC") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting STATIC type.");
        return false;
      }
      ++s;
      type = cmStateEnums::STATIC_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "SHARED") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting SHARED type.");
        return false;
      }
      ++s;
      type = cmStateEnums::SHARED_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "MODULE") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting MODULE type.");
        return false;
      }
      ++s;
      type = cmStateEnums::MODULE_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "OBJECT") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting OBJECT type.");
        return false;
      }
      ++s;
      type = cmStateEnums::OBJECT_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "UNKNOWN") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting UNKNOWN type.");
        return false;
      }
      ++s;
      type = cmStateEnums::UNKNOWN_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "ALIAS") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library specified with conflicting ALIAS type.");
        return false;
      }
      ++s;
      isAlias = true;
    } else if (libType == "INTERFACE") {
      if (haveSpecifiedType) {
        status.SetError(
          "INTERFACE library specified with conflicting/multiple types.");
        return false;
      }
      if (isAlias) {
        status.SetError(
          "INTERFACE library specified with conflicting ALIAS type.");
        return false;
      }
      if (excludeFromAll) {
        status.SetError(
          "INTERFACE library may not be used with EXCLUDE_FROM_ALL.");
        return false;
      }
      ++s;
      type = cmStateEnums::INTERFACE_LIBRARY;
      haveSpecifiedType = true;
    } else if (*s == "EXCLUDE_FROM_ALL") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        status.SetError(
          "INTERFACE library may not be used with EXCLUDE_FROM_ALL.");
        return false;
      }
      ++s;
      excludeFromAll = true;
    } else if (*s == "IMPORTED") {
      ++s;
      importTarget = true;
    } else if (importTarget && *s == "GLOBAL") {
      ++s;
      importGlobal = true;
    } else if (type == cmStateEnums::INTERFACE_LIBRARY && *s == "GLOBAL") {
      status.SetError(
        "GLOBAL option may only be used with IMPORTED libraries.");
      return false;
    } else {
      break;
    }
  }

  if (type == cmStateEnums::INTERFACE_LIBRARY) {
    if (s != args.end()) {
      status.SetError("INTERFACE library requires no source arguments.");
      return false;
    }
    if (importGlobal && !importTarget) {
      status.SetError(
        "INTERFACE library specified as GLOBAL, but not as IMPORTED.");
      return false;
    }
  }

  bool nameOk = cmGeneratorExpression::IsValidTargetName(libName) &&
    !cmGlobalGenerator::IsReservedTarget(libName);

  if (nameOk && !importTarget && !isAlias) {
    nameOk = libName.find(':') == std::string::npos;
  }
  if (!nameOk && !mf.CheckCMP0037(libName, type)) {
    return false;
  }

  if (isAlias) {
    if (!cmGeneratorExpression::IsValidTargetName(libName)) {
      status.SetError("Invalid name for ALIAS: " + libName);
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
      status.SetError(cmStrCat("cannot create ALIAS target \"", libName,
                               "\" because target \"", aliasedName,
                               "\" is itself an ALIAS."));
      return false;
    }
    cmTarget* aliasedTarget = mf.FindTargetToUse(aliasedName, true);
    if (!aliasedTarget) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", libName,
                               "\" because target \"", aliasedName,
                               "\" does not already exist."));
      return false;
    }
    cmStateEnums::TargetType aliasedType = aliasedTarget->GetType();
    if (aliasedType != cmStateEnums::SHARED_LIBRARY &&
        aliasedType != cmStateEnums::STATIC_LIBRARY &&
        aliasedType != cmStateEnums::MODULE_LIBRARY &&
        aliasedType != cmStateEnums::OBJECT_LIBRARY &&
        aliasedType != cmStateEnums::INTERFACE_LIBRARY &&
        !(aliasedType == cmStateEnums::UNKNOWN_LIBRARY &&
          aliasedTarget->IsImported())) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", libName,
                               "\" because target \"", aliasedName,
                               "\" is not a library."));
      return false;
    }
    if (aliasedTarget->IsImported() &&
        !aliasedTarget->IsImportedGloballyVisible()) {
      status.SetError(cmStrCat("cannot create ALIAS target \"", libName,
                               "\" because target \"", aliasedName,
                               "\" is imported but not globally visible."));
      return false;
    }
    mf.AddAlias(libName, aliasedName);
    return true;
  }

  if (importTarget && excludeFromAll) {
    status.SetError("excludeFromAll with IMPORTED target makes no sense.");
    return false;
  }

  /* ideally we should check whether for the linker language of the target
    CMAKE_${LANG}_CREATE_SHARED_LIBRARY is defined and if not default to
    STATIC. But at this point we know only the name of the target, but not
    yet its linker language. */
  if ((type == cmStateEnums::SHARED_LIBRARY ||
       type == cmStateEnums::MODULE_LIBRARY) &&
      !mf.GetState()->GetGlobalPropertyAsBool("TARGET_SUPPORTS_SHARED_LIBS")) {
    mf.IssueMessage(
      MessageType::AUTHOR_WARNING,
      cmStrCat(
        "ADD_LIBRARY called with ",
        (type == cmStateEnums::SHARED_LIBRARY ? "SHARED" : "MODULE"),
        " option but the target platform does not support dynamic linking. ",
        "Building a STATIC library instead. This may lead to problems."));
    type = cmStateEnums::STATIC_LIBRARY;
  }

  // Handle imported target creation.
  if (importTarget) {
    // The IMPORTED signature requires a type to be specified explicitly.
    if (!haveSpecifiedType) {
      status.SetError("called with IMPORTED argument but no library type.");
      return false;
    }
    if (type == cmStateEnums::OBJECT_LIBRARY) {
      std::string reason;
      if (!mf.GetGlobalGenerator()->HasKnownObjectFileLocation(&reason)) {
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "The OBJECT library type may not be used for IMPORTED libraries" +
            reason + ".");
        return true;
      }
    }
    if (type == cmStateEnums::INTERFACE_LIBRARY) {
      if (!cmGeneratorExpression::IsValidTargetName(libName)) {
        status.SetError(cmStrCat(
          "Invalid name for IMPORTED INTERFACE library target: ", libName));
        return false;
      }
    }

    // Make sure the target does not already exist.
    if (mf.FindTargetToUse(libName)) {
      status.SetError(cmStrCat(
        "cannot create imported target \"", libName,
        "\" because another target with the same name already exists."));
      return false;
    }

    // Create the imported target.
    mf.AddImportedTarget(libName, type, importGlobal);
    return true;
  }

  // A non-imported target may not have UNKNOWN type.
  if (type == cmStateEnums::UNKNOWN_LIBRARY) {
    mf.IssueMessage(
      MessageType::FATAL_ERROR,
      "The UNKNOWN library type may be used only for IMPORTED libraries.");
    return true;
  }

  // Enforce name uniqueness.
  {
    std::string msg;
    if (!mf.EnforceUniqueName(libName, msg)) {
      status.SetError(msg);
      return false;
    }
  }

  std::vector<std::string> srclists;

  if (type == cmStateEnums::INTERFACE_LIBRARY) {
    if (!cmGeneratorExpression::IsValidTargetName(libName) ||
        libName.find("::") != std::string::npos) {
      status.SetError(
        cmStrCat("Invalid name for INTERFACE library target: ", libName));
      return false;
    }

    mf.AddLibrary(libName, type, srclists, excludeFromAll);
    return true;
  }

  cm::append(srclists, s, args.end());

  mf.AddLibrary(libName, type, srclists, excludeFromAll);

  return true;
}
