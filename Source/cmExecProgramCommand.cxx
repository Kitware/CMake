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
bool cmExecProgramCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  std::vector<std::string>  args = argsIn;
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string output;
  m_Makefile->ExpandVariablesInString(args[0]);
  if(args.size() == 2)
    {
    m_Makefile->ExpandVariablesInString(args[1]);
    cmSystemTools::MakeDirectory(args[1].c_str());
    std::string command;
    command = "cd ";
    command += args[1].c_str();
    command += " && ";
    command += args[0].c_str();
    cmSystemTools::RunCommand(command.c_str(), output);
    }
  else
    {
    cmSystemTools::RunCommand(args[0].c_str(), output);
    }
  return true;
}

