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
  if (argsIn.size()< 9)
    {
      this->SetError("called with wrong number of arguments.");
      return false;
    }
  std::vector<std::string> args = argsIn;
  std::vector<std::string> commandArgs;
  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  
  const char* source = args[0].c_str();
  const char* command = args[1].c_str();
  if(args[2] != "ARGS")
    {
    this->SetError("Wrong syntax. The third argument should be ARGS");
    return false;
    }
  int cc=3;
  while(args[cc] != "DEPENDS" && cc < argsIn.size())
    {
    commandArgs.push_back(args[cc]);
    cc++;
    }
  if(cc == argsIn.size()-1)
    {
    this->SetError("Wrong syntax. Missing DEPENDS.");
    return false;
    }
  cc ++ ; // Skip DEPENDS
  while(args[cc] != "OUTPUTS" && cc < argsIn.size())
    {
    depends.push_back(args[cc]);
    cc++;
    }
  if(cc == argsIn.size()-1)
    {
    this->SetError("Wrong syntax. Missing OUTPUTS.");
    return false;
    }
  cc ++; // Skip OUTPUTS
  while(cc < argsIn.size()-1)
    {
    outputs.push_back(args[cc]);
    cc++;
    }
  const char *target = args[argsIn.size()-1].c_str();
  m_Makefile->AddCustomCommand( source, command, commandArgs, 
				depends, outputs, target );
  return true;
}


