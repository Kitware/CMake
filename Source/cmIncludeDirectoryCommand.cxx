/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
      this->SetError("given empty-string as include directory.");
      return false;
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
// ideally that should be three separate arguments but when sucking the
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
  std::string::size_type b = ret.find_first_not_of(" \r");
  std::string::size_type e = ret.find_last_not_of(" \r");
  if ((b!=ret.npos) && (e!=ret.npos))  
    {
    ret.assign(ret, b, 1+e-b);   // copy the remaining substring
    }
  else
    {
    return;         // if we get here, we had only whitespace in the string
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

