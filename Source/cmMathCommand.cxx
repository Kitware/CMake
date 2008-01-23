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
#include "cmMathCommand.h"

#include "cmExprParserHelper.h"

//----------------------------------------------------------------------------
bool cmMathCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if ( args.size() < 1 )
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }
  const std::string &subCommand = args[0];
  if(subCommand == "EXPR")
    {
    return this->HandleExprCommand(args);
    }
  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmMathCommand::HandleExprCommand(std::vector<std::string> const& args)
{
  if ( args.size() != 3 )
    {
    this->SetError("EXPR called with incorrect arguments.");
    return false;
    }

  const std::string& outputVariable = args[1];
  const std::string& expression = args[2];
  
  cmExprParserHelper helper;
  if ( !helper.ParseString(expression.c_str(), 0) )
    {
    std::string e = "cannot parse the expression: \""+expression+"\": ";
    e += helper.GetError();
    this->SetError(e.c_str());
    return false;
    }

  char buffer[1024];
  sprintf(buffer, "%d", helper.GetResult());

  this->Makefile->AddDefinition(outputVariable.c_str(), buffer);
  return true;
}
