/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSetCommand.h"

// cmSetCommand
bool cmSetCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // SET (VAR ) // this is a no-op
  if (args.size() == 1)
    {
    return true;
    }
// here are the options 
//  SET (VAR) 
//  SET (VAR value )
//  SET (VAR CACHE TYPE "doc String")
//  SET (VAR value CACHE TYPE "doc string")

  const char* variable = args[0].c_str(); // VAR is always first
  std::string value;  // optional
  bool cache = false; // optional
  cmCacheManager::CacheEntryType type = cmCacheManager::STRING; // required if cache
  const char* docstring = 0; // required if cache
  std::string::size_type cacheStart = 0;
  
  // check for SET(VAR v1 v2 ... vn) 
  // and create
  if(args.size() > 2)
    {
    if(args[1] != "CACHE" && args[2] != "CACHE")
      {
      value = args[1];
      for(unsigned int i =2; i < args.size(); ++i)
        {
        value += ";";
        value += args[i];
        }
      m_Makefile->AddDefinition(variable, value.c_str());
      return true;
      }
    }
  
    
  if(args.size() == 2)
    {
      // SET (VAR value )
      value= args[1];
    }
  else if(args.size() == 4)
    {
    // SET (VAR CACHE TYPE "doc String")
    cache = true;
    cacheStart = 1;
    }
  else if(args.size() == 5)
    {
    //  SET (VAR value CACHE TYPE "doc string")
    cache = true;
    value = args[1];
    cacheStart = 2;
    }
  else
    {
    std::string message;
    message += "Syntax error in SET:\n";
    message += "CACHE requires TYPE and document string SET command:\n";
    message += "SET (";
    for(std::vector<std::string>::const_iterator i = args.begin();
        i != args.end(); ++i)
      {
      message += *i;
      }
    message += ")\n";
    this->SetError(message.c_str());
    return false;
    }
  if(cache)
    {
    if(args[cacheStart] != "CACHE")
      {
      std::string error = "Error in arguments to cache, expected CACHE found:";
      error += args[cacheStart];
      error += "\n";
      this->SetError(error.c_str());
      return false;
      }
    type = cmCacheManager::StringToType(args[cacheStart+1].c_str());
    docstring = args[cacheStart+2].c_str();
    }
  // get the current cache value for the variable
  const char* cacheValue = 
    m_Makefile->GetDefinition(variable);
  if(cacheValue)
    {
    // if it is not a cached value, or it is a cached
    // value that is not internal keep the value found
    // in the cache
    if(cache && type != cmCacheManager::INTERNAL)
      {
      return true;
      }
    }
  // if it is meant to be in the cache then define it in the cache
  if(cache)
    {
    m_Makefile->AddCacheDefinition(variable,
                                   value.c_str(),
                                   docstring,
                                   type);
    }
  else
    {
    // add the definition
    m_Makefile->AddDefinition(variable, value.c_str());
    }
  return true;
}

