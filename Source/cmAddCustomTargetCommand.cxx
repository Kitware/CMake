/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmAddCustomTargetCommand.h"

// cmAddCustomTargetCommand
bool cmAddCustomTargetCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Check the target name.
  if(args[0].find_first_of("/\\") != args[0].npos)
  {
    // slashes are not allowed anymore in taret names CMP_0001
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP_0001))
    {
      case cmPolicies::WARN:
        cmSystemTools::Message(
          this->Makefile->GetPolicies()->GetPolicyWarning
            (cmPolicies::CMP_0001).c_str(),"Warning");
      case cmPolicies::OLD:
//        if (this->Makefile->IsBWCompatibilityLessThan(2,2))
//        {
//          break;
//        }
      case cmPolicies::NEW:
        this->SetError("You included a / or \\ in your target name and "
          "this is not allowed according to policy CMP_0001. Run "
          "cmake --help-policy CMP_0001 for more information.");
        return false;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->SetError(
          this->Makefile->GetPolicies()->GetRequiredPolicyError
            (cmPolicies::CMP_0001).c_str());
        return false;      
    }
  }
    
  // Accumulate one command line at a time.
  cmCustomCommandLine currentLine;

  // Save all command lines.
  cmCustomCommandLines commandLines;

  // Accumulate dependencies.
  std::vector<std::string> depends;
  std::string working_directory;
  bool verbatim = false;
  std::string comment_buffer;
  const char* comment = 0;

  // Keep track of parser state.
  enum tdoing {
    doing_command,
    doing_depends,
    doing_working_directory,
    doing_comment,
    doing_verbatim
  };
  tdoing doing = doing_command;

  // Look for the ALL option.
  bool excludeFromAll = true;
  unsigned int start = 1;
  if(args.size() > 1)
    {
    if(args[1] == "ALL")
      {
      excludeFromAll = false;
      start = 2;
      }
    }

  // Parse the rest of the arguments.
  for(unsigned int j = start; j < args.size(); ++j)
    {
    std::string const& copy = args[j];

    if(copy == "DEPENDS")
      {
      doing = doing_depends;
      }
    else if(copy == "WORKING_DIRECTORY")
      {
      doing = doing_working_directory;
      }
    else if(copy == "VERBATIM")
      {
      doing = doing_verbatim;
      verbatim = true;
      }
    else if (copy == "COMMENT")
      {
      doing = doing_comment;
      }
    else if(copy == "COMMAND")
      {
      doing = doing_command;

      // Save the current command before starting the next command.
      if(!currentLine.empty())
        {
        commandLines.push_back(currentLine);
        currentLine.clear();
        }
      }
    else
      {
      switch (doing)
        {
        case doing_working_directory:
          working_directory = copy;
          break;
        case doing_command:
          currentLine.push_back(copy);
          break;
        case doing_depends:
          depends.push_back(copy);
          break;
         case doing_comment:
           comment_buffer = copy;
           comment = comment_buffer.c_str();
           break;
        default:
          this->SetError("Wrong syntax. Unknown type of argument.");
          return false;
        }
      }
    }

  std::string::size_type pos = args[0].find_first_of("#<>");
  if(pos != args[0].npos)
    {
    cmOStringStream msg;
    msg << "called with target name containing a \"" << args[0][pos]
        << "\".  This character is not allowed.";
    this->SetError(msg.str().c_str());
    return false;
    }

  // Store the last command line finished.
  if(!currentLine.empty())
    {
    commandLines.push_back(currentLine);
    currentLine.clear();
    }

  // Enforce name uniqueness.
  {
  std::string msg;
  if(!this->Makefile->EnforceUniqueName(args[0], msg, true))
    {
    this->SetError(msg.c_str());
    return false;
    }
  }

  // Add the utility target to the makefile.
  bool escapeOldStyle = !verbatim;
  this->Makefile->AddUtilityCommand(args[0].c_str(), excludeFromAll,
                                    working_directory.c_str(), depends,
                                    commandLines, escapeOldStyle, comment);

  return true;
}
