/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmAddCustomCommandCommand.h"


// cmAddCustomCommandCommand
bool cmAddCustomCommandCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if (argsIn.size() < 6)
    {
      this->SetError("called with wrong number of arguments.");
      return false;
    }

  std::vector<std::string> args = argsIn;
  
  if(args[2] != "ARGS")
    {
    this->SetError("Wrong syntax. The third argument should be ARGS");
    return false;
    }
  int cc=3;

  std::vector<std::string> commandArgs;
  while(args[cc] != "DEPENDS" && cc < argsIn.size())
    {
    m_Makefile->ExpandVariablesInString(args[cc]);
    commandArgs.push_back(args[cc]);
    cc++;
    }

  if(cc == argsIn.size()-1)
    {
    this->SetError("Wrong syntax. Missing DEPENDS.");
    return false;
    }
  cc++ ; // Skip DEPENDS

  std::vector<std::string> depends;
  while(args[cc] != "OUTPUTS" && cc < argsIn.size())
    {
    m_Makefile->ExpandVariablesInString(args[cc]);
    depends.push_back(args[cc]);
    cc++;
    }

  if(cc == argsIn.size()-1)
    {
    this->SetError("Wrong syntax. Missing OUTPUTS.");
    return false;
    }
  cc ++; // Skip OUTPUTS

  std::vector<std::string> outputs;
  while(cc < argsIn.size()-1)
    {
    m_Makefile->ExpandVariablesInString(args[cc]);
    outputs.push_back(args[cc]);
    cc++;
    }

  m_Makefile->ExpandVariablesInString(args[0]);
  m_Makefile->ExpandVariablesInString(args[1]);
  m_Makefile->ExpandVariablesInString(args[argsIn.size()-1]);

  m_Makefile->AddCustomCommand(args[0].c_str(), 
                               args[1].c_str(), 
                               commandArgs, 
			       depends, 
                               outputs, 
                               args[argsIn.size()-1].c_str());
  return true;
}


