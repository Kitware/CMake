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
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmMakeDepend.h"
#include "cmUnixMakefileGenerator.h"
#include "cmCacheManager.h"

void Usage(const char* program)
{
  std::cerr << "Usage: " << program << " CMakeLists.txt " 
            << "-Ssource_start_directory "
            << "-Ooutput_start_directory "
            << "-Hsource_home_directory "
            << "-Boutput_home_directory\n"
            << "Where start directories are the current place in the tree,"
    "and the home directories are the top.\n";
}

  
// This is the main program used to gentrate makefile fragments 
// from CMakeLists.txt input files.   
int main(int ac, char** av)
{
  if(ac < 2)
    {
    Usage(av[0]);
    return -1;
    }
  // Create a makefile
  cmMakefile mf;
  mf.AddDefinition("UNIX", "1");
  bool makeCache = false;
  // Parse the command line
  if(ac > 2)
    {
    for(int i =2; i < ac; i++)
      {
      std::string arg = av[i];
      // Set the start source directory with a -S dir options
      if(arg.find("-MakeCache",0) == 0)
	{
        makeCache = true;
	}
      // Set the start source directory with a -S dir options
      if(arg.find("-S",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetStartDirectory(path.c_str());
	}
      // Set the start output directory with a -O dir options
      if(arg.find("-O",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetStartOutputDirectory(path.c_str());
	}
      // Set the source home directory with a -H dir option
      if(arg.find("-H",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetHomeDirectory(path.c_str());
	}
      // Set the output or binary directory with a -B dir option
      if(arg.find("-B",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetHomeOutputDirectory(path.c_str());
	}
      if(arg.find("-D",0) == 0)
	{
	std::string value = arg.substr(2);
        mf.AddDefinition(value.c_str(), true);
	}
      }
    }
  // Only generate makefiles if not trying to make the cache
  cmUnixMakefileGenerator* gen = new cmUnixMakefileGenerator;
  mf.SetMakefileGenerator(gen);
  if(makeCache)
    {
    // generate only enough for the cache
    gen->SetCacheOnlyOn();
    // generate for this makefile and all below it
    gen->SetRecurseOn();
    }

  // Read and parse the input makefile
  mf.MakeStartDirectoriesCurrent();
  cmCacheManager::GetInstance()->LoadCache(&mf);
  
  // Make sure the internal "CMAKE" cache entry is set.
  const char* cacheValue = cmCacheManager::GetInstance()->GetCacheValue("CMAKE");
  if(!cacheValue)
    {
    // Find our own exectuable.
    std::string cMakeSelf = cmSystemTools::FindProgram(av[0]);
    // Save the value in the cache
    cmCacheManager::GetInstance()->AddCacheEntry("CMAKE",
                                                 cMakeSelf.c_str(),
                                                 "Path to CMake executable.",
                                                 cmCacheManager::INTERNAL);
    }
  
  // Transfer the cache into the makefile's definitions.
  cmCacheManager::GetInstance()->DefineCache(&mf);
  if(!mf.ReadListFile(av[1]))
    {
    Usage(av[0]);
    return -1;
    }
  mf.GenerateMakefile();
  cmCacheManager::GetInstance()->SaveCache(&mf);
  if(makeCache)
    {
    cmCacheManager::GetInstance()->PrintCache(std::cout);
    }
  
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

