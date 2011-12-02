/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddLibraryCommand.h"

#include "cmake.h"

// cmLibraryCommand
bool cmAddLibraryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // Library type defaults to value of BUILD_SHARED_LIBS, if it exists,
  // otherwise it defaults to static library.
  cmTarget::TargetType type = cmTarget::SHARED_LIBRARY;
  if (cmSystemTools::IsOff(this->Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    type = cmTarget::STATIC_LIBRARY;
    }
  bool excludeFromAll = false;
  bool importTarget = false;

  std::vector<std::string>::const_iterator s = args.begin();

  std::string libName = *s;

  ++s;

  // If the second argument is "SHARED" or "STATIC", then it controls
  // the type of library.  Otherwise, it is treated as a source or
  // source list name. There may be two keyword arguments, check for them
  bool haveSpecifiedType = false;
  while ( s != args.end() )
    {
    std::string libType = *s;
    if(libType == "STATIC")
      {
      ++s;
      type = cmTarget::STATIC_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "SHARED")
      {
      ++s;
      type = cmTarget::SHARED_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "MODULE")
      {
      ++s;
      type = cmTarget::MODULE_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "UNKNOWN")
      {
      ++s;
      type = cmTarget::UNKNOWN_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(*s == "EXCLUDE_FROM_ALL")
      {
      ++s;
      excludeFromAll = true;
      }
    else if(*s == "IMPORTED")
      {
      ++s;
      importTarget = true;
      }
    else
      {
      break;
      }
    }

  /* ideally we should check whether for the linker language of the target
    CMAKE_${LANG}_CREATE_SHARED_LIBRARY is defined and if not default to
    STATIC. But at this point we know only the name of the target, but not
    yet its linker language. */
  if ((type != cmTarget::STATIC_LIBRARY) &&
       (this->Makefile->GetCMakeInstance()->GetPropertyAsBool(
                                      "TARGET_SUPPORTS_SHARED_LIBS") == false))
    {
    std::string msg = "ADD_LIBRARY for library ";
    msg += args[0];
    msg += " is used with the ";
    msg += type==cmTarget::SHARED_LIBRARY ? "SHARED" : "MODULE";
    msg += " option, but the target platform supports only STATIC libraries. "
           "Building it STATIC instead. This may lead to problems.";
    cmSystemTools::Message(msg.c_str() ,"Warning");
    type = cmTarget::STATIC_LIBRARY;
    }

  // Handle imported target creation.
  if(importTarget)
    {
    // The IMPORTED signature requires a type to be specified explicitly.
    if (!haveSpecifiedType)
      {
      this->SetError("called with IMPORTED argument but no library type.");
      return false;
      }

    // Make sure the target does not already exist.
    if(this->Makefile->FindTargetToUse(libName.c_str()))
      {
      cmOStringStream e;
      e << "cannot create imported target \"" << libName
        << "\" because another target with the same name already exists.";
      this->SetError(e.str().c_str());
      return false;
      }

    // Create the imported target.
    this->Makefile->AddImportedTarget(libName.c_str(), type);
    return true;
    }

  // A non-imported target may not have UNKNOWN type.
  if(type == cmTarget::UNKNOWN_LIBRARY)
    {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "The UNKNOWN library type may be used only for IMPORTED libraries."
      );
    return true;
    }

  // Enforce name uniqueness.
  {
  std::string msg;
  if(!this->Makefile->EnforceUniqueName(libName, msg))
    {
    this->SetError(msg.c_str());
    return false;
    }
  }

  if (s == args.end())
    {
    std::string msg = "You have called ADD_LIBRARY for library ";
    msg += args[0];
    msg += " without any source files. This typically indicates a problem ";
    msg += "with your CMakeLists.txt file";
    cmSystemTools::Message(msg.c_str() ,"Warning");
    }

  std::vector<std::string> srclists;
  while (s != args.end())
    {
    srclists.push_back(*s);
    ++s;
    }

  this->Makefile->AddLibrary(libName.c_str(), type, srclists, excludeFromAll);

  return true;
}


