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

void cmake::Usage(const char* program)
{
  std::cerr << "cmake version " << cmMakefile::GetMajorVersion()
            << "." << cmMakefile::GetMinorVersion() << "\n";
  std::cerr << "Usage: " << program << " srcdir \n" 
            << "Where cmake is run from the directory where you want the object files written\n";
}

// Parse the args
void cmake::SetArgs(cmMakefile& builder, const std::vector<std::string>& args)
{
  m_Local = false;

  // watch for cmake and cmake srcdir invocations
  if (args.size() <= 2)
    {
    builder.SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    builder.SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    if (args.size() == 2)
      {
      builder.SetHomeDirectory
	(cmSystemTools::CollapseFullPath(args[1].c_str()).c_str());
      builder.SetStartDirectory
	(cmSystemTools::CollapseFullPath(args[1].c_str()).c_str());
      }
    else
      {
      builder.SetHomeDirectory
	(cmSystemTools::GetCurrentWorkingDirectory().c_str());
      builder.SetStartDirectory
	(cmSystemTools::GetCurrentWorkingDirectory().c_str());
      }
    }

  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-H",0) == 0)
      {
      std::string path = arg.substr(2);
      builder.SetHomeDirectory(path.c_str());
      }
    if(arg.find("-S",0) == 0)
      {
      m_Local = true;
      std::string path = arg.substr(2);
      builder.SetStartDirectory(path.c_str());
      }
    if(arg.find("-O",0) == 0)
      {
      std::string path = arg.substr(2);
      builder.SetStartOutputDirectory(path.c_str());
      }
    if(arg.find("-B",0) == 0)
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

// at the end of this CMAKE_ROOT and CMAKE_COMMAND should be added to the cache
void cmake::AddCMakePaths(const std::vector<std::string>& args)
{
  // Find our own executable.
  std::string cMakeSelf = args[0];
  cmSystemTools::ConvertToUnixSlashes(cMakeSelf);
  cMakeSelf = cmSystemTools::FindProgram(cMakeSelf.c_str());
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
#ifdef CMAKE_BUILD_DIR
    cMakeSelf = CMAKE_BUILD_DIR;
    cMakeSelf += "/Source/cmake";
#endif
    }
#ifdef CMAKE_PREFIX
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    cMakeSelf = CMAKE_PREFIX "/bin/cmake";
    }
#endif
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    cmSystemTools::Error("CMAKE can not find the command line program cmake. "
                         "Attempted path: ", cMakeSelf.c_str());
    return;
    }
  // Save the value in the cache
  cmCacheManager::GetInstance()->AddCacheEntry
    ("CMAKE_COMMAND",
     cmSystemTools::EscapeSpaces(cMakeSelf.c_str()).c_str(),
     "Path to CMake executable.",
     cmCacheManager::INTERNAL);
  
  // do CMAKE_ROOT, look for the environment variable first
  std::string cMakeRoot;
  std::string modules;
  if (getenv("CMAKE_ROOT"))
    {
    cMakeRoot = getenv("CMAKE_ROOT");
    modules = cMakeRoot + "/Modules/FindVTK.cmake";
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe/..
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    std::string::size_type slashPos = cMakeRoot.rfind("/");
    if(slashPos != std::string::npos)      
      {
      cMakeRoot = cMakeRoot.substr(0, slashPos);
      }
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/FindVTK.cmake"; 
    }
  
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try exe/../share/cmake
    cMakeRoot += "/share/CMake";
    modules = cMakeRoot + "/Modules/FindVTK.cmake";
    }
#ifdef CMAKE_ROOT_DIR
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in root directory
    cMakeRoot = CMAKE_ROOT_DIR;
    modules = cMakeRoot + "/Modules/FindVTK.cmake";
    }
#endif
#ifdef CMAKE_PREFIX
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in install prefix
    cMakeRoot = CMAKE_PREFIX "/share/CMake";
    modules = cMakeRoot + "/Modules/FindVTK.cmake";
    }
#endif
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try 
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    cMakeRoot += "/share/CMake";
    modules = cMakeRoot +  "/Modules/FindVTK.cmake";
    }
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // couldn't find modules
    cmSystemTools::Error("Could not find CMAKE_ROOT !!!\n", 
                         "Modules directory not in directory:\n",
                         modules.c_str());
    return;  
    }
  cmCacheManager::GetInstance()->AddCacheEntry
    ("CMAKE_ROOT", cMakeRoot.c_str(),
     "Path to CMake installation.", cmCacheManager::INTERNAL);
}
 
int cmake::Generate(const std::vector<std::string>& args, bool buildMakefiles)
{
  if(args.size() == 1 && !cmSystemTools::FileExists("CMakeLists.txt"))
    {
    this->Usage(args[0].c_str());
    return -1;
    }
  // look for obvious request for help
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-help",0) != std::string::npos ||
       arg.find("--help",0) != std::string::npos ||
       arg.find("/?",0) != std::string::npos ||
       arg.find("-usage",0) != std::string::npos)
      {
      this->Usage(args[0].c_str());
      return -1;
      }
    }
  // Create a makefile
  cmMakefile mf;

  // extract the directory arguments
  cmake::SetArgs(mf, args);

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
  this->AddCMakePaths(args);

  // compute system info
  gen->ComputeSystemInfo();

  // Transfer the cache into the makefile's definitions.
  cmCacheManager::GetInstance()->DefineCache(&mf);

  std::string lf = mf.GetStartDirectory();
  lf +=  "/CMakeLists.txt";
  if(!mf.ReadListFile(lf.c_str()))
    {
    this->Usage(args[0].c_str());
    return -1;
    }
  // if buildMakefiles, then call GenerateMakefile
  if(buildMakefiles)
    {
    mf.GenerateMakefile();
    }
  else  // do not build, but let the commands finalize
    {
    std::vector<cmMakefile*> makefiles;
    mf.FindSubDirectoryCMakeListsFiles(makefiles);
    for(std::vector<cmMakefile*>::iterator i = makefiles.begin();
      i != makefiles.end(); ++i)
      {
      cmMakefile* mf = *i;
      mf->FinalPass();
      delete mf;
      }
    mf.FinalPass();
    }
  
  
  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:
  // LIBRARY_OUTPUT_PATH
  // EXECUTABLE_OUTPUT_PATH
  if(!cmCacheManager::GetInstance()->GetCacheValue("LIBRARY_OUTPUT_PATH"))
    {
    cmCacheManager::GetInstance()->AddCacheEntry("LIBRARY_OUTPUT_PATH", "",
                                                 "Single output directory for building all libraries.",
                                                 cmCacheManager::PATH);
    } 
  if(!cmCacheManager::GetInstance()->GetCacheValue("EXECUTABLE_OUTPUT_PATH"))
    {
    cmCacheManager::GetInstance()->AddCacheEntry("EXECUTABLE_OUTPUT_PATH", "",
                                                 "Single output directory for building all executables.",
                                                 cmCacheManager::PATH);
    }  
  
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

