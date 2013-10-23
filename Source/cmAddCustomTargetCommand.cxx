/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddCustomTargetCommand.h"

// cmAddCustomTargetCommand
bool cmAddCustomTargetCommand
::InitialPass(std::vector<std::string> const& args,
              cmExecutionStatus&)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Check the target name.
  if(args[0].find_first_of("/\\") != args[0].npos)
    {
    cmOStringStream e;
    e << "called with invalid target name \"" << args[0]
      << "\".  Target names may not contain a slash.  "
      << "Use ADD_CUSTOM_COMMAND to generate files.";
    this->SetError(e.str().c_str());
    return false;
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
  std::vector<std::string> sources;

  // Keep track of parser state.
  enum tdoing {
    doing_command,
    doing_depends,
    doing_working_directory,
    doing_comment,
    doing_source,
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
    else if(copy == "SOURCES")
      {
      doing = doing_source;
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
          {
          std::string dep = copy;
          cmSystemTools::ConvertToUnixSlashes(dep);
          depends.push_back(dep);
          }
          break;
         case doing_comment:
           comment_buffer = copy;
           comment = comment_buffer.c_str();
           break;
        case doing_source:
          sources.push_back(copy);
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

  // Convert working directory to a full path.
  if(!working_directory.empty())
    {
    const char* build_dir = this->Makefile->GetCurrentOutputDirectory();
    working_directory =
      cmSystemTools::CollapseFullPath(working_directory.c_str(), build_dir);
    }

  // Add the utility target to the makefile.
  bool escapeOldStyle = !verbatim;
  cmTarget* target =
    this->Makefile->AddUtilityCommand(args[0].c_str(), excludeFromAll,
                                      working_directory.c_str(), depends,
                                      commandLines, escapeOldStyle, comment);

  // Add additional user-specified source files to the target.
  target->AddSources(sources);

  return true;
}
