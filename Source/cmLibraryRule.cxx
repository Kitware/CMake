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
#include "cmLibraryRule.h"

// cmLibraryRule
bool cmLibraryRule::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 || args.size() > 1)
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  m_Makefile->SetLibraryName(args[0].c_str());
  return true;
}

