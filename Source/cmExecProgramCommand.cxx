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
  std::string output_variable;
  bool haveoutput_variable = false;
  std::string return_variable;
  bool havereturn_variable = false;
  for(size_t i=0; i < args.size(); ++i)
    {
    if(args[i] == "OUTPUT_VARIABLE")
      {
      count++;
      doingargs = false;
      havereturn_variable = false;
      haveoutput_variable = true;
      }    
    else if ( haveoutput_variable )
      {
      if ( output_variable.size() > 0 )
        {
        this->SetError("called with incorrect number of arguments");
        return false;
        }
      output_variable = args[i];
      count ++;
      }
    else if(args[i] == "RETURN_VALUE")
      {
      count++;
      doingargs = false;
      haveoutput_variable = false;
      havereturn_variable = true;
      }    
    else if ( havereturn_variable )
      {
      if ( return_variable.size() > 0 )
        {
        this->SetError("called with incorrect number of arguments");
        return false;
        }
      return_variable = args[i];
      count ++;
      }
    else if(args[i] == "ARGS")
      {
      count++;
      havereturn_variable = false;
      haveoutput_variable = false;
      doingargs = true;
      }
    else if(doingargs)
      {
      arguments += args[i];
      arguments += " ";
      count++;
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
  int retVal = 0;
  std::string output;
  if(args.size() - count == 2)
    {
    cmSystemTools::MakeDirectory(args[1].c_str());
    cmSystemTools::RunCommand(command.c_str(), output, retVal,
                              cmSystemTools::ConvertToOutputPath(args[1].c_str()).c_str());
    }
  else
    {
    cmSystemTools::RunCommand(command.c_str(), output, retVal);
    }

  if ( output_variable.size() > 0 )
    {    
    std::string::size_type first = output.find_first_not_of(" \n\t\r");
    std::string::size_type last = output.find_last_not_of(" \n\t\r");
    std::string coutput = std::string(output, first, last-first+1);
    m_Makefile->AddDefinition(output_variable.c_str(), coutput.c_str());
    }

  if ( return_variable.size() > 0 )
    {
    char buffer[100];
    sprintf(buffer, "%d", retVal);
    m_Makefile->AddDefinition(return_variable.c_str(), buffer);
    }
  
  return true;
}

