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
#include "cmUtilitySourceCommand.h"

// cmUtilitySourceCommand
bool cmUtilitySourceCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the cache entry name.
  std::string cacheEntry = *arg++;
  const char* cacheValue =
    cmCacheManager::GetInstance()->GetCacheValue(cacheEntry.c_str());
  // If it exists already, we are done.
  if(cacheValue)
    {
    // Set the makefile's definition with the cache value.
    m_Makefile->AddDefinition(cacheEntry.c_str(), cacheValue);
    return true;
    }
  
  // The second argument is the utility's executable name, which will be
  // needed later.
  std::string utilityName = *arg++;
  
  // The third argument specifies the relative directory of the source
  // of the utility.
  std::string relativeSource = *arg++;
  std::string utilitySource = m_Makefile->GetCurrentDirectory();
  utilitySource = utilitySource+"/"+relativeSource;
  
  // If the directory doesn't exist, the source has not been included.
  if(!cmSystemTools::FileExists(utilitySource.c_str()))
    { return true; }
  
  // Make sure all the files exist in the source directory.
  while(arg != args.end())
    {
    std::string file = utilitySource+"/"+*arg++;
    if(!cmSystemTools::FileExists(file.c_str()))
      { return true; }
    }
  
  // The source exists.
  std::string cmakeCFGout = m_Makefile->GetDefinition("CMAKE_CFG_OUTDIR");
  std::string utilityDirectory = m_Makefile->GetCurrentOutputDirectory();
  utilityDirectory += "/"+relativeSource;
  
  // Tell the makefile where to look for this utility.
  m_Makefile->AddUtilityDirectory(utilityDirectory.c_str());
  
  // Construct the cache entry for the executable's location.
  std::string utilityExecutable =
    utilityDirectory+"/"+cmakeCFGout+"/"
    +utilityName+cmSystemTools::GetExecutableExtension();
  
  // Enter the value into the cache.
  cmCacheManager::GetInstance()->AddCacheEntry(cacheEntry.c_str(),
                                               utilityExecutable.c_str(),
                                               "Path to an internal program.",
                                               cmCacheManager::FILEPATH);
  
  // Set the definition in the makefile.
  m_Makefile->AddDefinition(cacheEntry.c_str(), utilityExecutable.c_str());
  
  return true;
}

