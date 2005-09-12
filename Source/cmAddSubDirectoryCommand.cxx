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
  std::string srcArg = args[0];
  std::string binArg;
  
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
    else if (!binArg.size())
      {
      binArg = *i;
      }
    else
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    }

  // check for relative arguments
  bool relativeSource = true;
  std::string binPath = binArg;
  std::string srcPath = std::string(m_Makefile->GetCurrentDirectory()) + 
    "/" + srcArg;
  // if the path does not exist then the arg was relative
  if (!cmSystemTools::FileIsDirectory(srcPath.c_str()))
    {
    relativeSource = false;
    srcPath = srcArg;
    if (!cmSystemTools::FileIsDirectory(srcPath.c_str()))
      {
      std::string error = "Incorrect ADD_SUBDIRECTORY command. Directory: ";
      error += srcArg + " does not exists.";
      this->SetError(error.c_str());   
      return false;
      }
    }
  
  // at this point srcPath has the full path to the source directory
  // now we need to compute the binPath if it was not provided
  
  // if the argument was provided then use it
  if (binArg.size())
    {
    if (!cmSystemTools::FileIsFullPath(binPath.c_str()))
      {
      binPath = std::string(m_Makefile->GetCurrentOutputDirectory()) + 
        "/" + binArg.c_str();
      }
    }
  // otherwise compute the binPath from the srcPath
  else
    {
    // if the srcArg was relative then we just do the same for the binPath
    if (relativeSource)
      {
      binPath = std::string(m_Makefile->GetCurrentOutputDirectory()) + 
        "/" + srcArg;
      }
    // otherwise we try to remove the CurrentDirectory from the srcPath and
    // replace it with the CurrentOutputDirectory. This may not really work
    // because the source dir they provided may not be "in" the source
    // tree. This is an error if this happens.
    else
      {
      // try replacing the home dir with the home output dir
      binPath = srcPath;
      if (!cmSystemTools::FindLastString(binPath.c_str(), 
                                         m_Makefile->GetHomeDirectory()))
        {
        this->SetError("A full source directory was specified that is not in the source tree but no binary directory was specified. If you specify an out of tree source directory then you must provide the binary directory as well.");   
        return false;
        }
      cmSystemTools::ReplaceString(binPath,m_Makefile->GetHomeDirectory(), 
                                   m_Makefile->GetHomeOutputDirectory());
      }
    }
  
  // now we have all the arguments
  m_Makefile->AddSubDirectory(srcPath.c_str(), binPath.c_str(),
                              intoplevel, false, true);

  return true;
}

