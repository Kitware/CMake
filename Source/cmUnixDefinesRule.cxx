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
#include "cmUnixDefinesRule.h"

cmUnixDefinesRule::cmUnixDefinesRule()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->EnabledOff();
#endif
}

// cmUNIXDefinesRule
bool cmUnixDefinesRule::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("Win32Defines called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddDefineFlag((*i).c_str());
    }
  return true;
}

