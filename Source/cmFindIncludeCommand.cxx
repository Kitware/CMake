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
#include "cmFindIncludeCommand.h"
#include "cmCacheManager.h"

// cmFindIncludeCommand
bool cmFindIncludeCommand::Invoke(std::vector<std::string>& args)
{
  this->SetError("This has been deprecated, please use FIND_PATH command instead ");
  return false;
  
  
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue(args[0].c_str());
  if(cacheValue )
    {
    if(strcmp(cacheValue, "NOTFOUND") != 0)
      {
      m_Makefile->AddDefinition(args[0].c_str(), cacheValue);
      }
    cacheValue
      = cmCacheManager::GetInstance()->GetCacheValue(args[1].c_str());
    if(cacheValue)
      { 
      if(strcmp(cacheValue, "NOTFOUND") != 0)
        {
        m_Makefile->AddDefinition(args[1].c_str(), cacheValue);
        }
      }
    return true;
    }
  std::vector<std::string> path;
  // add any user specified paths
  for (unsigned int j = 3; j < args.size(); j++)
    {
    // expand variables
    std::string exp = args[j];
    m_Makefile->ExpandVariablesInString(exp);
    path.push_back(exp);
    }

  // add the standard path
  cmSystemTools::GetPath(path);
  unsigned int k;
  for(k=0; k < path.size(); k++)
    {
    std::string tryPath = path[k];
    tryPath += "/";
    tryPath += args[2];
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      // Save the value in the cache
      m_Makefile->AddDefinition(args[0].c_str(), path[k].c_str());
      cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                                   path[k].c_str(),
                                                   "Find an include path.",
                                                   cmCacheManager::PATH);
      m_Makefile->AddDefinition(args[1].c_str(), args[2].c_str());
      cmCacheManager::GetInstance()->AddCacheEntry(args[1].c_str(),
                                                   args[2].c_str(),
                                                   "Find an include path.",
                                                   cmCacheManager::PATH);
      return true;
      }
    }
  cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                               "NOTFOUND",
                                               "Find an include path.",
                                               cmCacheManager::PATH);
  cmCacheManager::GetInstance()->AddCacheEntry(args[1].c_str(),
                                               "NOTFOUND",
                                               "Find an include path.",
                                               cmCacheManager::PATH);
  std::string message = "Include not found: ";
  message += args[1];
  message += "\n";
  message += "looked in ";
  for(k=0; k < path.size(); k++)
    {
    message += path[k];
    message += "\n";
    }
  this->SetError(message.c_str());
  return false;
}

