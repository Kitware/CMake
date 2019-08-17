/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddLibraryCommand.h"

#include <sstream>

#include "cmAlgorithms.h"
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

  std::vector<std::string>::const_iterator s = args.begin();

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
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting STATIC type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::STATIC_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "SHARED") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting SHARED type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::SHARED_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "MODULE") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting MODULE type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::MODULE_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "OBJECT") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting OBJECT type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::OBJECT_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "UNKNOWN") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting UNKNOWN type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::UNKNOWN_LIBRARY;
      haveSpecifiedType = true;
    } else if (libType == "ALIAS") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting ALIAS type.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      isAlias = true;
    } else if (libType == "INTERFACE") {
      if (haveSpecifiedType) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting/multiple types.";
        status.SetError(e.str());
        return false;
      }
      if (isAlias) {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting ALIAS type.";
        status.SetError(e.str());
        return false;
      }
      if (excludeFromAll) {
        std::ostringstream e;
        e << "INTERFACE library may not be used with EXCLUDE_FROM_ALL.";
        status.SetError(e.str());
        return false;
      }
      ++s;
      type = cmStateEnums::INTERFACE_LIBRARY;
      haveSpecifiedType = true;
    } else if (*s == "EXCLUDE_FROM_ALL") {
      if (type == cmStateEnums::INTERFACE_LIBRARY) {
        std::ostringstream e;
        e << "INTERFACE library may not be used with EXCLUDE_FROM_ALL.";
        status.SetError(e.str());
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
      std::ostringstream e;
      e << "GLOBAL option may only be used with IMPORTED libraries.";
      status.SetError(e.str());
      return false;
    } else {
      break;
    }
  }

  if (type == cmStateEnums::INTERFACE_LIBRARY) {
    if (s != args.end()) {
      std::ostringstream e;
      e << "INTERFACE library requires no source arguments.";
      status.SetError(e.str());
      return false;
    }
    if (importGlobal && !importTarget) {
      std::ostringstream e;
      e << "INTERFACE library specified as GLOBAL, but not as IMPORTED.";
      status.SetError(e.str());
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
      std::ostringstream e;
      e << "ALIAS requires exactly one target argument.";
      status.SetError(e.str());
      return false;
    }

    std::string const& aliasedName = *s;
    if (mf.IsAlias(aliasedName)) {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName << "\" because target \""
        << aliasedName << "\" is itself an ALIAS.";
      status.SetError(e.str());
      return false;
    }
    cmTarget* aliasedTarget = mf.FindTargetToUse(aliasedName, true);
    if (!aliasedTarget) {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName << "\" because target \""
        << aliasedName
        << "\" does not already "
           "exist.";
      status.SetError(e.str());
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
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName << "\" because target \""
        << aliasedName << "\" is not a library.";
      status.SetError(e.str());
      return false;
    }
    if (aliasedTarget->IsImported() &&
        !aliasedTarget->IsImportedGloballyVisible()) {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName << "\" because target \""
        << aliasedName << "\" is imported but not globally visible.";
      status.SetError(e.str());
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
    std::ostringstream w;
    w << "ADD_LIBRARY called with "
      << (type == cmStateEnums::SHARED_LIBRARY ? "SHARED" : "MODULE")
      << " option but the target platform does not support dynamic linking. "
         "Building a STATIC library instead. This may lead to problems.";
    mf.IssueMessage(MessageType::AUTHOR_WARNING, w.str());
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
        std::ostringstream e;
        e << "Invalid name for IMPORTED INTERFACE library target: " << libName;
        status.SetError(e.str());
        return false;
      }
    }

    // Make sure the target does not already exist.
    if (mf.FindTargetToUse(libName)) {
      std::ostringstream e;
      e << "cannot create imported target \"" << libName
        << "\" because another target with the same name already exists.";
      status.SetError(e.str());
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
      std::ostringstream e;
      e << "Invalid name for INTERFACE library target: " << libName;
      status.SetError(e.str());
      return false;
    }

    mf.AddLibrary(libName, type, srclists, excludeFromAll);
    return true;
  }

  cmAppend(srclists, s, args.end());

  mf.AddLibrary(libName, type, srclists, excludeFromAll);

  return true;
}
