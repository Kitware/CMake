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
#include "cmFindLibraryCommand.h"
#include "cmCacheManager.h"

// cmFindLibraryCommand
bool cmFindLibraryCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  std::string helpString = "Where can the ";
  helpString += args[1] + " library be found";
  const char* cacheValue
    = cmCacheManager::GetInstance()->GetCacheValue(args[0].c_str());
  if(cacheValue && strcmp(cacheValue, "NOTFOUND"))
    { 
    m_Makefile->AddDefinition(args[0].c_str(), cacheValue);
    cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                                 cacheValue,
                                                 helpString.c_str(),
                                                 cmCacheManager::FILEPATH);
    return true;
    }
  std::vector<std::string> path;
  std::vector<std::string> names;
  bool foundName = false;
  bool foundPath = false;
  bool doingNames = true;
  for (unsigned int j = 1; j < args.size(); ++j)
    {
    if(args[j] == "NAMES")
      {
      doingNames = true;
      foundName = true;
      }
    else if (args[j] == "PATHS")
      {
      doingNames = false;
      foundPath = true;
      }
    else
      { 
      m_Makefile->ExpandVariablesInString(args[j]);
      if(doingNames)
        {
        names.push_back(args[j]);
        }
      else
        {
        path.push_back(args[j]);
        }
      }
    }
  // old style name path1 path2 path3
  if(!foundPath && !foundName)
    {
    names.clear();
    path.clear();
    names.push_back(args[1]);
    // add any user specified paths
    for (unsigned int j = 2; j < args.size(); j++)
      {
      // expand variables
      std::string exp = args[j];
      m_Makefile->ExpandVariablesInString(exp);
      path.push_back(exp);
      }
    }
  std::string library;
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end() ; ++i)
    {
    library = cmSystemTools::FindLibrary(i->c_str(),
                                         path);
    if(library != "")
      {  
      m_Makefile->AddDefinition(args[0].c_str(), library.c_str());  
      cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                                   library.c_str(),
                                                   helpString.c_str(),
                                                   cmCacheManager::FILEPATH);
      return true;
      } 
    }
  cmCacheManager::GetInstance()->AddCacheEntry(args[0].c_str(),
                                               "NOTFOUND",
                                               helpString.c_str(),
                                               cmCacheManager::FILEPATH);
  return true;
}

