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
#include "cmAddCustomCommandCommand.h"
#include "cmTarget.h"

// cmAddCustomCommandCommand
bool cmAddCustomCommandCommand::InitialPass(std::vector<std::string> const& args)
{
  /* Let's complain at the end of this function about the lack of a particular
     arg. For the moment, let's say that COMMAND, and either TARGET or SOURCE
     are required.
  */
  if (args.size() < 4)
    {
      this->SetError("called with wrong number of arguments.");
      return false;
    }

  std::string source, command, target, comment, output, main_dependency;
  std::vector<std::string> command_args, depends, outputs;

  cmTarget::CustomCommandType cctype = cmTarget::POST_BUILD;
  
  enum tdoing {
    doing_source,
    doing_command,
    doing_target,
    doing_args,
    doing_depends,
    doing_main_dependency,
    doing_output,
    doing_outputs,
    doing_comment,
    doing_nothing
  };

  tdoing doing = doing_nothing;

  for (unsigned int j = 0; j < args.size(); ++j)
    {
    std::string const& copy = args[j];

    if(copy == "SOURCE")
      {
      doing = doing_source;
      }
    else if(copy == "COMMAND")
      {
      doing = doing_command;
      }
    else if(copy == "PRE_BUILD")
      {
      cctype = cmTarget::PRE_BUILD;
      }
    else if(copy == "PRE_LINK")
      {
      cctype = cmTarget::PRE_LINK;
      }
    else if(copy == "POST_BUILD")
      {
      cctype = cmTarget::POST_BUILD;
      }
    else if(copy == "TARGET")
      {
      doing = doing_target;
      }
    else if(copy == "ARGS")
      {
      doing = doing_args;
      }
    else if (copy == "DEPENDS")
      {
      doing = doing_depends;
      }
    else if (copy == "OUTPUTS")
      {
      doing = doing_outputs;
      }
    else if (copy == "OUTPUT")
      {
      doing = doing_output;
      }
    else if (copy == "MAIN_DEPENDENCY")
      {
      doing = doing_main_dependency;
      }
    else if (copy == "COMMENT")
      {
      doing = doing_comment;
      }
    else
      {
      switch (doing)
        {
        case doing_source:
          source = copy;
          break;
        case doing_output:
          output = copy;
          break;
        case doing_main_dependency:
          main_dependency = copy;
          break;
        case doing_command:
          command = copy;
          break;
        case doing_target:
          target = copy;
          break;
        case doing_args:
          command_args.push_back(copy);
          break;
        case doing_depends:
          depends.push_back(copy);
          break;
        case doing_outputs:
          outputs.push_back(copy);
          break;
        case doing_comment:
          comment = copy;
          break;
        default:
          this->SetError("Wrong syntax. Unknown type of argument.");
          return false;
        }
      }
    }

  /* At this point we could complain about the lack of arguments.
     For the moment, let's say that COMMAND, TARGET are always 
     required.
  */
  if (output.empty() && target.empty())
    {
    this->SetError("Wrong syntax. A TARGET or OUTPUT must be specified.");
    return false;
    }

  if (source.empty() 
      && !target.empty() 
      && !output.empty())
    {
    this->SetError("Wrong syntax. A TARGET and OUTPUT can not both be specified.");
    return false;
    }
  
  // If source is empty, use the target 
  if(source.empty() && output.empty())
    {
    m_Makefile->AddCustomCommandToTarget(target.c_str(), 
                                         command.c_str(), 
                                         command_args, 
                                         cctype,
                                         comment.c_str());
    return true;
    }

  // If target is empty, use the output
  if(target.empty())
    {
    m_Makefile->AddCustomCommandToOutput(output.c_str(), 
                                         command.c_str(), 
                                         command_args, 
                                         main_dependency.c_str(),
                                         depends, 
                                         comment.c_str());
    return true;
    }

  // otherwise backwards compatiblity mode
  m_Makefile->AddCustomCommand(source.c_str(), 
                               command.c_str(), 
                               command_args, 
                               depends, 
                               outputs, 
                               target.c_str(),
                               comment.c_str());
  
  return true;
}



