/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmExecProgramCommand.h"
#include "cmSystemTools.h"

// cmExecProgramCommand
bool cmExecProgramCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string arguments;
  bool doingargs = false;
  int count = 0;
  for(size_t i=0; i < args.size(); ++i)
    {
    if(doingargs)
      {
      arguments += args[i];
      arguments += " ";
      count++;
      }
    else if(args[i] == "ARGS")
      {
      count++;
      doingargs = true;
      }
    }

  std::string command;
  if(arguments.size())
    {
    command = cmSystemTools::ConvertToOutputPath(args[0].c_str());
    command += " ";
    command += arguments;
    }
  else
    {
    command = args[0];
    }
  std::string output;
  if(args.size() - count == 2)
    {
    cmSystemTools::MakeDirectory(args[1].c_str());
    cmSystemTools::RunCommand(command.c_str(), output, 
                              cmSystemTools::ConvertToOutputPath(args[1].c_str()).c_str());
    }
  else
    {
    cmSystemTools::RunCommand(command.c_str(), output);
    }
  return true;
}

