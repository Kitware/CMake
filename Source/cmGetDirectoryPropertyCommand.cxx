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
#include "cmGetDirectoryPropertyCommand.h"

#include "cmake.h"

// cmGetDirectoryPropertyCommand
bool cmGetDirectoryPropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::size_type cc;
  std::vector<std::string>::const_iterator i = args.begin();
  std::string variable = *i;
  ++i;
  std::string output = "";
    
  // get the directory argument if there is one
  cmMakefile *dir = this->Makefile;
  if (*i == "DIRECTORY")
    {
    ++i;
    if (i == args.end())
      {
      this->SetError
        ("DIRECTORY argument provided without subsequent arguments");
      return false;
      }
    std::string sd = *i;
    // make sure the start dir is a full path
    if (!cmSystemTools::FileIsFullPath(sd.c_str()))
      {
      sd = this->Makefile->GetStartDirectory();
      sd += "/";
      sd += *i;
      }
    // lookup the makefile from the directory name
    cmLocalGenerator *lg = 
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      FindLocalGenerator(sd.c_str());
    if (!lg)
      {
      this->SetError
        ("DIRECTORY argument provided but requested directory not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    dir = lg->GetMakefile();
    ++i;
    }

  // OK, now we have the directory to process, we just get the requested
  // information out of it
  
  if ( *i == "VARIABLES" || *i == "CACHE_VARIABLES" )
    {
    int cacheonly = 0;
    if ( *i == "CACHE_VARIABLES" )
      {
      cacheonly = 1;
      }
    std::vector<std::string> vars = dir->GetDefinitions(cacheonly);
    for ( cc = 0; cc < vars.size(); cc ++ )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += vars[cc];
      }
    }
  else if ( *i == "MACROS" )
    {
    dir->GetListOfMacros(output);
    }
  else if ( *i == "DEFINITIONS" )
    {
    output = dir->GetDefineFlags();
    }
  else if ( *i == "INCLUDE_DIRECTORIES" )
    {
    std::vector<std::string>::iterator it;
    int first = 1;
    cmOStringStream str;
    for ( it = dir->GetIncludeDirectories().begin();
      it != dir->GetIncludeDirectories().end();
      ++ it )
      {
      if ( !first )
        {
        str << ";";
        }
      str << it->c_str();
      first = 0;
      }
    output = str.str();
    }
  else if ( *i == "INCLUDE_REGULAR_EXPRESSION" )
    {
    output = dir->GetIncludeRegularExpression();
    }
  else if ( *i == "LINK_DIRECTORIES" )
    {
    std::vector<std::string>::iterator it;
    int first = 1;
    cmOStringStream str;
    for ( it = dir->GetLinkDirectories().begin();
      it != dir->GetLinkDirectories().end();
      ++ it )
      {
      if ( !first )
        {
        str << ";";
        }
      str << it->c_str();
      first = 0;
      }
    output = str.str();
    }
  else if ( *i == "DEFINITION" )
    {
    ++i;
    if (i == args.end())
      {
      this->SetError("A request for a variable definition was made without "
                     "providing the name of the variable to get.");
      return false;
      }
    output = dir->GetSafeDefinition(i->c_str());
    }
  else
    {
    const char *prop = dir->GetProperty(i->c_str());
    if (prop)
      {
      this->Makefile->AddDefinition(variable.c_str(), prop);
      return true;
      }
    this->Makefile->AddDefinition(variable.c_str(), "");
    return true;
    }
  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  
  return true;
}

