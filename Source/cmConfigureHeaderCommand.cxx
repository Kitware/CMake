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
#include "cmConfigureHeaderCommand.h"

// cmConfigureHeaderCommand
bool cmConfigureHeaderCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  m_InputFile = args[0];
  m_OuputFile = args[1];
  return true;
}

void cmConfigureHeaderCommand::FinalPass()
{
  
}
