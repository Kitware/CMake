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
bool cmSetCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  if (args.size() == 1)
    {
      return true;
    }
// here are the options with the num
//  SET VAR                               
//  SET VAR value                             
//  SET VAR CACHE|CACHE_NO_REPLACE        
//  SET VAR value CACHE|CACHE_NO_REPLACE      
//  SET VAR CACHE|CACHE_NO_REPLACE TYPE   
//  SET VAR value CACHE|CACHE_NO_REPLACE TYPE 
  const char* type = "STRING"; // set a default type of STRING
  const char* value = "";// set a default value of the empty string
  if(args.size() > 1)
    {
    // always expand the first argument
    m_Makefile->ExpandVariablesInString(args[1]);
    value = args[1].c_str();
    }
  // get the current cache value for the variable
  const char* cacheValue = 
    cmCacheManager::GetInstance()->GetCacheValue(args[0].c_str());
  // assume this will not be cached
  bool cache = false;
  // search the arguments for key words CACHE and CACHE_NO_REPLACE
  for(int i = 1; i < args.size() && !cache; ++i)
    {
    if(args[i] == "CACHE_NO_REPLACE")
      {
      // if already in cache, ignore entire command
      if(cacheValue)
        {
        return true;
        }
      cache = true;
      }
    if(args[i] == "CACHE")
      {
      cache == true;
      }
    // if this is to be cached, find the value and type
    if(cache)
      {
      // if this is the 
      if(i == 1)
        {
        value = "";
        }
      if(i+1 < args.size())
        {
        type = args[i+1].c_str();
        }
      }
    }
  m_Makefile->AddDefinition(args[0].c_str(), value);
  if(cache)
    {
    cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                                 value,
                                                 "Value Computed by CMake",
                                                 cmCacheManager::StringToType(type));
    }
  return true;
}

