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
    else
      {
      break;
      }
    }

  std::vector<std::string> srclists(s, args.end());
  cmTarget* tgt = m_Makefile->AddExecutable(exename.c_str(), srclists); 
  if ( use_win32 )
    {
    tgt->SetProperty("WIN32_EXECUTABLE", "ON");
    }
  if ( use_macbundle)
    {
    tgt->SetProperty("MACOSX_BUNDLE", "ON");
#ifdef __APPLE__
    std::string f1 = m_Makefile->GetModulesFile("MacOSXBundleInfo.plist.in");
    if ( f1.size() == 0 )
      {
      this->SetError("could not find Mac OSX bundle template file.");
      return false;
      }
    std::string macdir = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    if ( macdir.size() == 0 )
      {
      macdir = m_Makefile->GetCurrentOutputDirectory();
      if(macdir.size() && macdir[macdir.size()-1] != '/')
        {
        macdir += "/";
        }
      }
    macdir += exename + ".app/Contents/";
    std::string f2 = macdir + "Info.plist";
    macdir += "MacOS";
    cmSystemTools::MakeDirectory(macdir.c_str());
    m_Makefile->AddDefinition("MACOSX_BUNDLE_EXECUTABLE_NAME", exename.c_str());
    m_Makefile->ConfigureFile(f1.c_str(), f2.c_str(), false, false, false);
#endif
    }

  return true;
}
