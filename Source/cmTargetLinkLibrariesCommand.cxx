/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetLinkLibrariesCommand.h"

const char* cmTargetLinkLibrariesCommand::LinkLibraryTypeNames[3] =
{
  "general",
  "debug",
  "optimized"
};

// cmTargetLinkLibrariesCommand
bool cmTargetLinkLibrariesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // must have one argument
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Lookup the target for which libraries are specified.
  this->Target =
    this->Makefile->GetCMakeInstance()
    ->GetGlobalGenerator()->FindTarget(0, args[0].c_str());
  if(!this->Target)
    {
    cmake::MessageType t = cmake::FATAL_ERROR;  // fail by default
    cmOStringStream e;
    e << "Cannot specify link libraries for target \"" << args[0] << "\" "
      << "which is not built by this project.";
    // The bad target is the only argument. Check how policy CMP0016 is set,
    // and accept, warn or fail respectively:
    if (args.size() < 2)
      {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0016))
        {
        case cmPolicies::WARN:
          t = cmake::AUTHOR_WARNING;
          // Print the warning.
          e << "\n"
            << "CMake does not support this but it used to work accidentally "
            << "and is being allowed for compatibility."
            << "\n" << this->Makefile->GetPolicies()->
                                        GetPolicyWarning(cmPolicies::CMP0016);
           break;
        case cmPolicies::OLD:          // OLD behavior does not warn.
          t = cmake::MESSAGE;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          e << "\n" << this->Makefile->GetPolicies()->
                                  GetRequiredPolicyError(cmPolicies::CMP0016);
          break;
        case cmPolicies::NEW:  // NEW behavior prints the error.
        default:
          break;
        }
      }

    // now actually print the message
    switch(t)
      {
      case cmake::AUTHOR_WARNING:
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, e.str());
        break;
      case cmake::FATAL_ERROR:
        this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
        cmSystemTools::SetFatalErrorOccured();
        break;
      default:
        break;
      }
    return true;
    }

  // but we might not have any libs after variable expansion
  if(args.size() < 2)
    {
    return true;
    }

  // Keep track of link configuration specifiers.
  cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
  bool haveLLT = false;

  // Start with primary linking and switch to link interface
  // specification if the keyword is encountered as the first argument.
  this->CurrentProcessingState = ProcessingLinkLibraries;

  // add libraries, nothe that there is an optional prefix
  // of debug and optimized than can be used
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "LINK_INTERFACE_LIBRARIES")
      {
      this->CurrentProcessingState = ProcessingLinkInterface;
      if(i != 1)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_INTERFACE_LIBRARIES option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      }
    else if(args[i] == "LINK_PUBLIC")
      {
      if(i != 1 && this->CurrentProcessingState != ProcessingPrivateInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingPublicInterface;
      }
    else if(args[i] == "LINK_PRIVATE")
      {
      if(i != 1 && this->CurrentProcessingState != ProcessingPublicInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingPrivateInterface;
      }
    else if(args[i] == "debug")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::DEBUG);
        }
      llt = cmTarget::DEBUG;
      haveLLT = true;
      }
    else if(args[i] == "optimized")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::OPTIMIZED);
        }
      llt = cmTarget::OPTIMIZED;
      haveLLT = true;
      }
    else if(args[i] == "general")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::GENERAL);
        }
      llt = cmTarget::GENERAL;
      haveLLT = true;
      }
    else if(haveLLT)
      {
      // The link type was specified by the previous argument.
      haveLLT = false;
      this->HandleLibrary(args[i].c_str(), llt);
      }
    else
      {
      // Lookup old-style cache entry if type is unspecified.  So if you
      // do a target_link_libraries(foo optimized bar) it will stay optimized
      // and not use the lookup.  As there maybe the case where someone has
      // specifed that a library is both debug and optimized.  (this check is
      // only there for backwards compatibility when mixing projects built
      // with old versions of CMake and new)
      llt = cmTarget::GENERAL;
      std::string linkType = args[0];
      linkType += "_LINK_TYPE";
      const char* linkTypeString =
        this->Makefile->GetDefinition( linkType.c_str() );
      if(linkTypeString)
        {
        if(strcmp(linkTypeString, "debug") == 0)
          {
          llt = cmTarget::DEBUG;
          }
        if(strcmp(linkTypeString, "optimized") == 0)
          {
          llt = cmTarget::OPTIMIZED;
          }
        }
      this->HandleLibrary(args[i].c_str(), llt);
      }
    }

  // Make sure the last argument was not a library type specifier.
  if(haveLLT)
    {
    cmOStringStream e;
    e << "The \"" << this->LinkLibraryTypeNames[llt]
      << "\" argument must be followed by a library.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    }

  // If any of the LINK_ options were given, make sure the
  // LINK_INTERFACE_LIBRARIES target property exists.
  // Use of any of the new keywords implies awareness of
  // this property. And if no libraries are named, it should
  // result in an empty link interface.
  if(this->CurrentProcessingState != ProcessingLinkLibraries &&
     !this->Target->GetProperty("LINK_INTERFACE_LIBRARIES"))
    {
    this->Target->SetProperty("LINK_INTERFACE_LIBRARIES", "");
    }

  return true;
}

//----------------------------------------------------------------------------
void
cmTargetLinkLibrariesCommand
::LinkLibraryTypeSpecifierWarning(int left, int right)
{
  cmOStringStream w;
  w << "Link library type specifier \""
    << this->LinkLibraryTypeNames[left] << "\" is followed by specifier \""
    << this->LinkLibraryTypeNames[right] << "\" instead of a library name.  "
    << "The first specifier will be ignored.";
  this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
}

//----------------------------------------------------------------------------
void
cmTargetLinkLibrariesCommand::HandleLibrary(const char* lib,
                                            cmTarget::LinkLibraryType llt)
{
  // Handle normal case first.
  if(this->CurrentProcessingState != ProcessingLinkInterface)
    {
    this->Makefile
      ->AddLinkLibraryForTarget(this->Target->GetName(), lib, llt);
    if (this->CurrentProcessingState != ProcessingPublicInterface)
      {
      // Not LINK_INTERFACE_LIBRARIES or LINK_PUBLIC, do not add to interface.
      return;
      }
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> const& debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();
  std::string prop;

  // Include this library in the link interface for the target.
  if(llt == cmTarget::DEBUG || llt == cmTarget::GENERAL)
    {
    // Put in the DEBUG configuration interfaces.
    for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
        i != debugConfigs.end(); ++i)
      {
      prop = "LINK_INTERFACE_LIBRARIES_";
      prop += *i;
      this->Target->AppendProperty(prop.c_str(), lib);
      }
    }
  if(llt == cmTarget::OPTIMIZED || llt == cmTarget::GENERAL)
    {
    // Put in the non-DEBUG configuration interfaces.
    this->Target->AppendProperty("LINK_INTERFACE_LIBRARIES", lib);

    // Make sure the DEBUG configuration interfaces exist so that the
    // general one will not be used as a fall-back.
    for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
        i != debugConfigs.end(); ++i)
      {
      prop = "LINK_INTERFACE_LIBRARIES_";
      prop += *i;
      if(!this->Target->GetProperty(prop.c_str()))
        {
        this->Target->SetProperty(prop.c_str(), "");
        }
      }
    }
}
