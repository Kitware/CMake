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
#include "cmake.h"
#include "cmCacheManager.h"

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#include "cmMSProjectGenerator.h"
#else
#include "cmUnixMakefileGenerator.h"
#endif

int main(int ac, char** av)
{
  cmake foo;
  if(ac < 2)
    {
      foo.Usage(av[0]);
      return -1;
    }
  return foo.Generate(ac,av);
}

void cmake::Usage(const char* program)
{
  std::cerr << "Usage: " << program << " srcdir " 
            << "-Ssource_start_directory "
            << "-Ooutput_start_directory "
            << "-Hsource_home_directory "
            << "-Boutput_home_directory\n"
            << "Where start directories are the current place in the tree,"
    "and the home directories are the top.\n";
}

// Parse the args
void cmake::SetArgs(cmMakefile& builder, int ac, char** av)
{
  m_Local = false;

  // watch for cmake and cmake srcdir invocations
  if (ac <= 2)
    {
    builder.SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    builder.SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    if (ac == 2)
      {
      builder.SetHomeDirectory
	(cmSystemTools::CollapseFullPath(av[1]).c_str());
      builder.SetStartDirectory
	(cmSystemTools::CollapseFullPath(av[1]).c_str());
      }
    else
      {
      builder.SetHomeDirectory
	(cmSystemTools::GetCurrentWorkingDirectory().c_str());
      builder.SetStartDirectory
	(cmSystemTools::GetCurrentWorkingDirectory().c_str());
      }
    }

  for(int i =1; i < ac; i++)
    {
    std::string arg = av[i];
    if(arg.find("-H",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetHomeDirectory(path.c_str());
      }
    if(arg.find("-S",0) != std::string::npos)
      {
      m_Local = true;
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
      }
    if(arg.find("-D",0) == 0)
      {
	std::string value = arg.substr(2);
	builder.AddDefinition(value.c_str(), true);
      }
    if(arg.find("-V",0) == 0)
      {
	m_Verbose = true;
      }
    }
  
  if (!m_Local)
    {
    builder.SetStartDirectory(builder.GetHomeDirectory());
    builder.SetStartOutputDirectory(builder.GetHomeOutputDirectory());
    }
}

// at the end of this CMAKE_ROOT and CMAAKE_COMMAND should be added to the cache
void cmake::AddCMakePaths(char **av)
{
  // Find our own exectuable.
  std::string cMakeSelf = av[0];
  cmSystemTools::ConvertToUnixSlashes(cMakeSelf);
  cMakeSelf = cmSystemTools::FindProgram(cMakeSelf.c_str());

  // Save the value in the cache
  cmCacheManager::GetInstance()->AddCacheEntry
    ("CMAKE_COMMAND",
     cmSystemTools::EscapeSpaces(cMakeSelf.c_str()).c_str(),
     "Path to CMake executable.",
     cmCacheManager::INTERNAL);

  // do CMAKE_ROOT, look for the environment variable first
  std::string cMakeRoot;
  if (getenv("CMAKE_ROOT"))
    {
    cMakeRoot = getenv("CMAKE_ROOT");
    }
  else
    {
    // next try exe/..
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    std::string::size_type slashPos = cMakeRoot.rfind("/");
    if(slashPos != std::string::npos)      
      {
      cMakeRoot = cMakeRoot.substr(0, slashPos);
      }
    // is there no Modules direcory there?
    std::string modules = cMakeRoot + "/Modules";
    if (!cmSystemTools::FileIsDirectory(modules.c_str()))
      {
      // try exe/../share/cmake
      modules = cMakeRoot + "/share/CMake/Modules";
      if (!cmSystemTools::FileIsDirectory(modules.c_str()))
	{
#if !defined(_WIN32) || defined(__CYGWIN__)
	// try compiled in value on UNIX
        cMakeRoot = CMAKE_ROOT_DIR;
        modules = cMakeRoot + "/Modules";
#endif
	if (!cmSystemTools::FileIsDirectory(modules.c_str()))
	  {
	    // couldn't find modules
	    cmSystemTools::Error("Could not find CMAKE_ROOT !!!");
	    return;  
	  }
	}
      else
	{
	cMakeRoot = cMakeRoot + "/share/CMake";
	}
      }
    }
  
  cmCacheManager::GetInstance()->AddCacheEntry
    ("CMAKE_ROOT", cMakeRoot.c_str(),
     "Path to CMake installation.", cmCacheManager::INTERNAL);
}
 
int cmake::Generate(int ac, char **av)
{
  // Create a makefile
  cmMakefile mf;

  // extract the directory arguments
  cmake::SetArgs(mf, ac, av);

  // create the generator
#if defined(_WIN32) && !defined(__CYGWIN__)  
  cmMSProjectGenerator* gen = new cmMSProjectGenerator;
#else
  cmUnixMakefileGenerator* gen = new cmUnixMakefileGenerator;
#endif
  
  gen->SetLocal(m_Local);

  // Read and parse the input makefile
  mf.SetMakefileGenerator(gen);
  mf.MakeStartDirectoriesCurrent();
  cmCacheManager::GetInstance()->LoadCache(&mf);

  // setup CMAKE_ROOT and CMAKE_COMMAND
  this->AddCMakePaths(av);

  // compute system info
  gen->ComputeSystemInfo();

  // Transfer the cache into the makefile's definitions.
  cmCacheManager::GetInstance()->DefineCache(&mf);

  std::string lf = mf.GetStartDirectory();
  lf +=  "/CMakeLists.txt";
  if(!mf.ReadListFile(lf.c_str()))
    {
      this->Usage(av[0]);
      return -1;
    }
  mf.GenerateMakefile();
  cmCacheManager::GetInstance()->SaveCache(&mf);

  if(m_Verbose)
    {
    cmCacheManager::GetInstance()->PrintCache(std::cout);
    }
  
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

