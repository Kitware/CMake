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
#include "cmIncludeRegularExpressionCommand.h"

// cmIncludeRegularExpressionCommand
bool cmIncludeRegularExpressionCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  m_Makefile->SetIncludeRegularExpression(args[0].c_str());
  
  return true;
}

