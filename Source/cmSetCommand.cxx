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
#include "cmSetCommand.h"

// cmSetCommand
bool cmSetCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // watch for ENV signatures
  const char* variable = args[0].c_str(); // VAR is always first
  bool haveEnvVariable = false;
  if (!strncmp(variable,"ENV{",4) && strlen(variable) > 5)
    {
    // what is the variable name
    char *varName = new char [strlen(variable)];
    strncpy(varName,variable+4,strlen(variable)-5);
    varName[strlen(variable)-5] = '\0';
    std::string putEnvArg = varName;
    putEnvArg += "=";
    
    // what is the current value if any
    const char *currValue = getenv(varName);

    // will it be set to something, then set it
    if (args.size() > 1 && args[1].size())
      {
      // but only if it is different from current value
      if (!currValue || strcmp(currValue,args[1].c_str()))
        {
        putEnvArg += args[1];
        cmSystemTools::PutEnv(putEnvArg.c_str());
        }
      return true;
      }
    
    // if it will be cleared, then clear it if it isn;t already clear
    if (currValue)
      {
      cmSystemTools::PutEnv(putEnvArg.c_str());
      }
    return true;
    }
  
  // SET (VAR) // Removes the definition of VAR.
  if (args.size() == 1)
    {
    m_Makefile->RemoveDefinition(args[0].c_str());
    return true;
    }

  // here are the remaining options 
  //  SET (VAR value )
  //  SET (VAR CACHE TYPE "doc String" [FORCE])
  //  SET (VAR value CACHE TYPE "doc string" [FORCE])
  std::string value;  // optional
  bool cache = false; // optional
  bool force = false; // optional
  cmCacheManager::CacheEntryType type 
    = cmCacheManager::STRING; // required if cache
  const char* docstring = 0; // required if cache
  std::string::size_type cacheStart = 0;
  
  // look for FORCE argument
  if (args.size() > 4 && args[args.size()-1] == "FORCE")
    {
    force = true;
    }

  // check for cache signature
  if (args.size() > 3 && args[args.size() - 3 - (force ? 1 : 0)] == "CACHE")
    {
    cache = true;
    }
  
  // collect any values into a single semi-colon seperated value list
  if(args.size() > 
     static_cast<unsigned short>(1 + (cache ? 3 : 0) + (force ? 1 : 0)))
    {
    value = args[1];
    size_t endPos = args.size() - (cache ? 3 : 0) - (force ? 1 : 0);
    for(size_t i = 2; i < endPos; ++i)
      {
      value += ";";
      value += args[i];
      }
    }

  // we should be nice and try to catch some simple screwups if the last or
  // next to last args are CACHE then they screwed up.  If they used FORCE
  // without CACHE they screwed up
  if (args[args.size() - 1] == "CACHE" ||
      args.size() > 1 && args[args.size() - 2] == "CACHE" ||
      force && !cache)
    {
    std::string message;
    message += "Syntax error in SET:\n";
    message += "See the help for the SET command:\n";
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
    cacheStart = args.size() - 3 - (force ? 1 : 0);
    type = cmCacheManager::StringToType(args[cacheStart+1].c_str());
    docstring = args[cacheStart+2].c_str();
    }

  // see if this is already in the cache
  cmCacheManager::CacheIterator it = 
    m_Makefile->GetCacheManager()->GetCacheIterator(variable);
  if(!it.IsAtEnd() && (it.GetType() != cmCacheManager::UNINITIALIZED))
    {
    // if the set is trying to CACHE the value but the value
    // is already in the cache and the type is not internal
    // then leave now without setting any definitions in the cache
    // or the makefile
    if(cache && type != cmCacheManager::INTERNAL && !force)
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

