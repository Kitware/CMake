/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSubdirCommand.h"

// cmSubdirCommand
bool cmSubdirCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  bool res = true;
  bool excludeFromAll = false;
  bool preorder = false;

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    if(*i == "EXCLUDE_FROM_ALL")
      {
      excludeFromAll = true;
      continue;
      }
    if(*i == "PREORDER")
      {
      preorder = true;
      continue;
      }

    // if they specified a relative path then compute the full
    std::string srcPath = 
      std::string(this->Makefile->GetCurrentDirectory()) + 
        "/" + i->c_str();
    if (cmSystemTools::FileIsDirectory(srcPath.c_str()))
      {
      std::string binPath = 
        std::string(this->Makefile->GetCurrentOutputDirectory()) + 
        "/" + i->c_str();
      this->Makefile->AddSubDirectory(srcPath.c_str(), binPath.c_str(),
                                  excludeFromAll, preorder, false);
      }
    // otherwise it is a full path
    else if ( cmSystemTools::FileIsDirectory(i->c_str()) )
      {
      // we must compute the binPath from the srcPath, we just take the last
      // element from the source path and use that
      std::string binPath = 
        std::string(this->Makefile->GetCurrentOutputDirectory()) + 
        "/" + cmSystemTools::GetFilenameName(i->c_str());
      this->Makefile->AddSubDirectory(i->c_str(), binPath.c_str(),
                                  excludeFromAll, preorder, false);
      }
    else
      {
      std::string error = "Incorrect SUBDIRS command. Directory: ";
      error += *i + " does not exists.";
      this->SetError(error.c_str());   
      res = false;
      }
    }
  return res;
}

