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
#include "cmStandardIncludes.h"
#include "cmMakefile.h"
#include "cmMSProjectGenerator.h"
#include "cmCacheManager.h"


// this is the command line version of CMakeSetup.
// It is called from Visual Studio when a CMakeLists.txt
// file is changed.


// Set the command line arguments
void SetArgs(cmMakefile& builder, int ac, char** av)
{
  for(int i =3; i < ac; i++)
    {
    std::string arg = av[i];
    if(arg.find("-H",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetHomeDirectory(path.c_str());
      }
    if(arg.find("-S",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetStartDirectory(path.c_str());
      }
    if(arg.find("-O",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetStartOutputDirectory(path.c_str());
      }
    if(arg.find("-B",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetHomeOutputDirectory(path.c_str());
      std::cout << "set output home to " << path.c_str() << std::endl;
      }
    }
}


int main(int ac, char** av)
{
  if(ac < 3)
    {
    std::cerr << "Usage: " << av[0] << 
      " CMakeLists.txt -[DSP|DSW] -Hsource_home  -Sstart_source_directory "
      " -Ostart_output_directory -Boutput_home" << std::endl;
    return -1;
    }
  std::string arg = av[2];
  cmMakefile makefile;
  SetArgs(makefile, ac, av);
  cmMSProjectGenerator* pg = new cmMSProjectGenerator;
  if(arg.find("-DSP", 0) != std::string::npos)
    {
    pg->BuildDSWOff();
    }
  else
    {
    pg->BuildDSWOn();
    }
  makefile.SetMakefileGenerator(pg);
  makefile.MakeStartDirectoriesCurrent();
  cmCacheManager::GetInstance()->LoadCache(&makefile);

  // Make sure the internal "CMAKE" cache entry is set.
  const char* cacheValue = cmCacheManager::GetInstance()->GetCacheValue("CMAKE");
  if(!cacheValue)
    {
    // Find our own exectuable.
    std::string cMakeSelf = "\""+cmSystemTools::FindProgram(av[0])+"\"";
    // Save the value in the cache
    cmCacheManager::GetInstance()->AddCacheEntry("CMAKE",
                                                 cMakeSelf.c_str(),
                                                 "Path to CMake executable.",
                                                 cmCacheManager::INTERNAL);
    }
  
  cmCacheManager::GetInstance()->DefineCache(&makefile);
  makefile.ReadListFile(av[1]);
  makefile.GenerateMakefile();
  cmCacheManager::GetInstance()->SaveCache(&makefile);
  return 0;
}

