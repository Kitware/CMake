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
#include "cmIncludeDirectoryCommand.h"

// cmIncludeDirectoryCommand
bool cmIncludeDirectoryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    return true;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  bool before = this->Makefile->IsOn("CMAKE_INCLUDE_DIRECTORIES_BEFORE");
  bool system = false;

  if ((*i) == "BEFORE")
    {
    before = true;
    ++i;
    }
  else if ((*i) == "AFTER")
    {
    before = false;
    ++i;
    }

  for(; i != args.end(); ++i)
    {
    if(*i == "SYSTEM")
      {
      system = true;
      continue;
      }
    if(i->size() == 0)
      {
      const char* errorMessage
        = "Empty Include Directory Passed into INCLUDE_DIRECTORIES command.";
      if(this->Makefile->NeedBackwardsCompatibility(2,4))
        {
        cmSystemTools::Error(errorMessage);
        }
      else
        {
        this->SetError(errorMessage);
        return false;
        }
      }

    this->AddDirectory(i->c_str(),before,system);

    }
  return true;
}

// do a lot of cleanup on the arguments because this is one place where folks
// sometimes take the output of a program and pass it directly into this
// command not thinking that a single argument could be filled with spaces
// and newlines etc liek below:
//
// "   /foo/bar
//    /boo/hoo /dingle/berry "
//
// ideally that should be three seperate arguments but when sucking the
// output from a program and passing it into a command the cleanup doesn't
// always happen
//
void cmIncludeDirectoryCommand::AddDirectory(const char *i, 
                                             bool before, 
                                             bool system)
{
  // break apart any line feed arguments
  std::string ret = i;
  std::string::size_type pos = 0;
  if((pos = ret.find('\n', pos)) != std::string::npos)
    {
    if (pos)
      {
      this->AddDirectory(ret.substr(0,pos).c_str(), before, system);
      }
    if (ret.size()-pos-1)
      {
      this->AddDirectory(ret.substr(pos+1,ret.size()-pos-1).c_str(),
                         before, system);
      }
    return;
    }

  // remove any leading or trailing spaces and \r
  pos = ret.size()-1;
  while(ret[pos] == ' ' || ret[pos] == '\r')
    {
    ret.erase(pos);
    pos--;
    }
  pos = 0;
  while(ret.size() && ret[pos] == ' ' || ret[pos] == '\r')
    {
    ret.erase(pos,1);
    }
  if (!ret.size())
    {
    return;
    }
  
  if (!cmSystemTools::IsOff(ret.c_str()))
    {
    cmSystemTools::ConvertToUnixSlashes(ret);
    if(!cmSystemTools::FileIsFullPath(ret.c_str()))
      {
      std::string tmp = this->Makefile->GetStartDirectory();
      tmp += "/";
      tmp += ret;
      ret = tmp;
      }
    }
  this->Makefile->AddIncludeDirectory(ret.c_str(), before);
  if(system)
    {
    this->Makefile->AddSystemIncludeDirectory(ret.c_str());
    }
}

