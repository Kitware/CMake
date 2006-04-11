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
bool cmAddCustomCommandCommand::InitialPass(
  std::vector<std::string> const& args)
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

  std::string source, target, comment, main_dependency,
    working;
  std::vector<std::string> depends, outputs, output;

  // Accumulate one command line at a time.
  cmCustomCommandLine currentLine;

  // Save all command lines.
  cmCustomCommandLines commandLines;

  cmTarget::CustomCommandType cctype = cmTarget::POST_BUILD;

  enum tdoing {
    doing_source,
    doing_command,
    doing_target,
    doing_depends,
    doing_main_dependency,
    doing_output,
    doing_outputs,
    doing_comment,
    doing_working_directory,
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

      // Save the current command before starting the next command.
      if(!currentLine.empty())
        {
        commandLines.push_back(currentLine);
        currentLine.clear();
        }
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
      // Ignore this old keyword.
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
    else if (copy == "WORKING_DIRECTORY")
      {
      doing = doing_working_directory;
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
      std::string filename;
      switch (doing)
        {
        case doing_output:
        case doing_outputs:
          if (!cmSystemTools::FileIsFullPath(copy.c_str()))
            {
            filename = this->Makefile->GetStartDirectory();
            filename += "/";
            }
          filename += copy;
          break;
        case doing_source:
          // We do not want to convert the argument to SOURCE because
          // that option is only available for backward compatibility.
          // Old-style use of this command may use the SOURCE==TARGET
          // trick which we must preserve.  If we convert the source
          // to a full path then it will no longer equal the target.
        default:
          break;
        }

       switch (doing)
         {
         case doing_working_directory:
           working = copy;
           break;
         case doing_source:
           source = copy;
           break;
         case doing_output:
           output.push_back(filename);
           break;
         case doing_main_dependency:
           main_dependency = copy;
           break;
         case doing_command:
           currentLine.push_back(copy);
           break;
         case doing_target:
           target = copy;
           break;
         case doing_depends:
           depends.push_back(copy);
           break;
         case doing_outputs:
           outputs.push_back(filename);
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

  // Store the last command line finished.
  if(!currentLine.empty())
    {
    commandLines.push_back(currentLine);
    currentLine.clear();
    }

  // At this point we could complain about the lack of arguments.  For
  // the moment, let's say that COMMAND, TARGET are always required.
  if(output.empty() && target.empty())
    {
    this->SetError("Wrong syntax. A TARGET or OUTPUT must be specified.");
    return false;
    }

  if(source.empty() && !target.empty() && !output.empty())
    {
    this->SetError(
      "Wrong syntax. A TARGET and OUTPUT can not both be specified.");
    return false;
    }

  // Make sure the output names and locations are safe.
  if(!this->CheckOutputs(output) || !this->CheckOutputs(outputs))
    {
    return false;
    }

  // Choose which mode of the command to use.
  if(source.empty() && output.empty())
    {
    // Source is empty, use the target.
    std::vector<std::string> no_depends;
    this->Makefile->AddCustomCommandToTarget(target.c_str(), no_depends,
                                         commandLines, cctype,
                                         comment.c_str(), working.c_str());
    }
  else if(target.empty())
    {
    // Target is empty, use the output.
    this->Makefile->AddCustomCommandToOutput(output, depends,
                                         main_dependency.c_str(),
                                         commandLines, comment.c_str(),
                                         working.c_str());
    }
  else
    {
    // Use the old-style mode for backward compatibility.
    this->Makefile->AddCustomCommandOldStyle(target.c_str(), outputs, depends,
                                         source.c_str(), commandLines,
                                         comment.c_str());
    }
  return true;
}

//----------------------------------------------------------------------------
bool
cmAddCustomCommandCommand
::CheckOutputs(const std::vector<std::string>& outputs)
{
  for(std::vector<std::string>::const_iterator o = outputs.begin();
      o != outputs.end(); ++o)
    {
    // Make sure the file will not be generated into the source
    // directory during an out of source build.
    if(!this->Makefile->CanIWriteThisFile(o->c_str()))
      {
      std::string e = "attempted to have a file \"" + *o +
        "\" in a source directory as an output of custom command.";
      this->SetError(e.c_str());
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }

    // Make sure the output file name has no invalid characters.
    std::string::size_type pos = o->find_first_of("#<>");
    if(pos != o->npos)
      {
      cmOStringStream msg;
      msg << "called with OUTPUT containing a \"" << (*o)[pos]
          << "\".  This character is not allowed.";
      this->SetError(msg.str().c_str());
      return false;
      }
    }
  return true;
}
