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
#include "cmIncludeCommand.h"


// cmIncludeCommand
bool cmIncludeCommand::InitialPass(std::vector<std::string> const& args)
{
  if (args.size()< 1 || args.size() > 2)
    {
      this->SetError("called with wrong number of arguments.  "
                     "Include only takes one file.");
      return false;
    }
  bool optional = false;

  std::string fname = args[0].c_str();

  if(args.size() == 2)
    {
    optional = args[1] == "OPTIONAL";
    }
  
  if(fname.find("/") == fname.npos)
    {
    // Not a path. Maybe module.
    std::string module = fname;
    module += ".cmake";
    std::string mfile = m_Makefile->GetModulesFile(module.c_str());
    if ( mfile.size() )
      {
      std::cout << "Module found: " << mfile.c_str() << std::endl;
      fname = mfile.c_str();
      }
    }
  bool readit = m_Makefile->ReadListFile( m_Makefile->GetCurrentListFile(), 
                                          fname.c_str() );
  if(!optional && !readit)
    {
    std::string m = "Could not find include file: ";
    m += fname;
    this->SetError(m.c_str());
    return false;
    }
  return true;
}


