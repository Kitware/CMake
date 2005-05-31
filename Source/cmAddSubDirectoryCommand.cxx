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
#include "cmAddSubDirectoryCommand.h"

// cmAddSubDirectoryCommand
bool cmAddSubDirectoryCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the binpath
  std::string binArg = args[0];
  std::string srcArg;
  
  bool intoplevel = true;

  // process the rest of the arguments looking for optional args
  std::vector<std::string>::const_iterator i = args.begin();
  ++i;
  for(;i != args.end(); ++i)
    {
    if(*i == "EXCLUDE_FROM_ALL")
      {
      intoplevel = false;
      continue;
      }
    else if (!srcArg.size())
      {
      srcArg = *i;
      }
    else
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    }

  // if srcArg not provided use binArg
  if (!srcArg.size())
    {
    srcArg = binArg;
    }
    
  // now we have all the arguments
  
  // if they specified a relative path then compute the full
  std::string srcPath = std::string(m_Makefile->GetCurrentDirectory()) + 
    "/" + srcArg;
  if (!cmSystemTools::FileIsDirectory(srcPath.c_str()))
    {
    srcPath = srcArg;
    if (!cmSystemTools::FileIsDirectory(srcPath.c_str()))
      {
      std::string error = "Incorrect ADD_SUBDIRECTORY command. Directory: ";
      error += srcArg + " does not exists.";
      this->SetError(error.c_str());   
      return false;
      }
    }
  
  std::string binPath = binArg;
  if (!cmSystemTools::FileIsFullPath(binPath.c_str()))
    {
    binPath = std::string(m_Makefile->GetCurrentOutputDirectory()) + 
      "/" + binArg.c_str();
    }
  
  m_Makefile->AddSubDirectory(srcPath.c_str(), binPath.c_str(),
                              intoplevel, false, true);

  return true;
}

