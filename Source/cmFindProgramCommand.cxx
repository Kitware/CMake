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
#include "cmFindProgramCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>
#include <stdio.h>
  

// cmFindProgramCommand
bool cmFindProgramCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::iterator i = args.begin();
  // Use the first argument as the name of something to be defined
  const char* define = (*i).c_str();
  i++; // move iterator to next arg
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = m_Makefile->GetDefinition(define);
  if(cacheValue && strcmp(cacheValue, "NOTFOUND"))
    {
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
        cmSystemTools::ExpandRegistryValues(args[j]);
        // Glob the entry in case of wildcards.
        cmSystemTools::GlobDirs(args[j].c_str(), path);
        }
      }
    }
  // if it is not in the cache, then search the system path
  // add any user specified paths 
  if(!foundPath && !foundName)
    {
    path.clear();
    names.clear();
    names.push_back(args[1]);
    for (unsigned int j = 2; j < args.size(); j++)
      {
      // expand variables
      std::string exp = args[j];
      m_Makefile->ExpandVariablesInString(exp);
      cmSystemTools::ExpandRegistryValues(exp);
      
      // Glob the entry in case of wildcards.
      cmSystemTools::GlobDirs(exp.c_str(), path);
      }
    }
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end() ; ++i)
    {
    // Try to find the program.
    std::string result = cmSystemTools::FindProgram(i->c_str(), path);
    if(result != "")
      {
      // Save the value in the cache
      m_Makefile->AddCacheDefinition(define,
                                     result.c_str(),
                                     "Path to a program.",
                                     cmCacheManager::FILEPATH);
      
      return true;
      }
    }
  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 "NOTFOUND",
                                 "Path to a program",
                                 cmCacheManager::FILEPATH);
  return true;
}

