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
#include "cmAddExecutableCommand.h"

// cmExecutableCommand
bool cmAddExecutableCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::const_iterator s = args.begin();

  std::string exename = *s;

  ++s;
  bool use_win32 = false;
  bool use_macbundle = false;
  bool in_all = true;
  while ( s != args.end() )
    {
    if (*s == "WIN32")
      {
      ++s;
      use_win32 = true;
      }
    else if ( *s == "MACOSX_BUNDLE" )
      {
      ++s;
      use_macbundle = true;
      }
    else if(*s == "NOT_IN_ALL")
      {
      ++s;
      in_all = false;
      }
    else
      {
      break;
      }
    }

  if (s == args.end())
    {
    this->SetError
      ("called with incorrect number of arguments, no sources provided");
    return false;
    }

  std::vector<std::string> srclists(s, args.end());
  cmTarget* tgt = this->Makefile->AddExecutable(exename.c_str(), srclists,
                                                in_all);
  if ( use_win32 )
    {
    tgt->SetProperty("WIN32_EXECUTABLE", "ON");
    }
  if ( use_macbundle)
    {
    tgt->SetProperty("MACOSX_BUNDLE", "ON");
    }

  return true;
}
