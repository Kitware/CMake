/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmSetCommand.h"

// cmSetCommand
bool cmSetCommand::InitialPass(std::vector<std::string>& args)
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
    for(std::vector<std::string>::iterator i = args.begin();
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
      this->SetError(error.c_str());
      return false;
      }
    type = cmCacheManager::StringToType(args[cacheStart+1].c_str());
    docstring = args[cacheStart+2].c_str();
    }
  // always expand the first argument
  m_Makefile->ExpandVariablesInString(value);
  // get the current cache value for the variable
  const char* cacheValue = 
    cmCacheManager::GetInstance()->GetCacheValue(variable);
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
  // add the definition
  m_Makefile->AddDefinition(variable, value.c_str());
  // if it is meant to be in the cache then define it in the cache
  if(cache)
    {
    cmCacheManager::GetInstance()->AddCacheEntry(variable,
                                                 value.c_str(),
                                                 docstring,
                                                 type);
    }
  return true;
}

