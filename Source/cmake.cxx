/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmake.h"
#include "cmCacheManager.h"

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#include "cmMSProjectGenerator.h"
#include "cmMSDotNETGenerator.h"
#include "cmBorlandMakefileGenerator.h"
#include "cmNMakeMakefileGenerator.h"
#else
#include "cmUnixMakefileGenerator.h"
#endif

cmake::cmake()
{
  m_Verbose = false;
#if defined(_WIN32) && !defined(__CYGWIN__)  
  cmMakefileGenerator::RegisterGenerator(new cmMSProjectGenerator);
  cmMakefileGenerator::RegisterGenerator(new cmMSDotNETGenerator);
  cmMakefileGenerator::RegisterGenerator(new cmNMakeMakefileGenerator);
  cmMakefileGenerator::RegisterGenerator(new cmBorlandMakefileGenerator);
#else
  cmMakefileGenerator::RegisterGenerator(new cmUnixMakefileGenerator);
#endif
}

void cmake::Usage(const char* program)
{
  std::strstream errorStream;

  errorStream << "cmake version " << cmMakefile::GetMajorVersion()
            << "." << cmMakefile::GetMinorVersion() << "\n";
  errorStream << "Usage: " << program << " [srcdir] [options]\n" 
            << "Where cmake is run from the directory where you want the object files written.  If srcdir is not specified, the current directory is used for both source and object files.\n";
  errorStream << "Options are:\n";
  errorStream << "\n-i (puts cmake in wizard mode, not available for ccmake)\n";
  errorStream << "\n-DVAR:TYPE=VALUE (create a cache file entry)\n";
  errorStream << "\n-Cpath_to_initial_cache (a cmake list file that is used to pre-load the cache with values.)\n";
  errorStream << "\n[-GgeneratorName] (where generator name can be one of these: ";
  std::vector<std::string> names;
  cmMakefileGenerator::GetRegisteredGenerators(names);
  for(std::vector<std::string>::iterator i =names.begin();
      i != names.end(); ++i)
    {
    errorStream << "\"" << i->c_str() << "\" ";
    }
	  errorStream << ")\n" << std::ends;

  cmSystemTools::Error(errorStream.str());
}

// Parse the args
void cmake::SetCacheArgs(cmMakefile& builder,
                         const std::vector<std::string>& args)
{ 
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0)
      {
      std::string entry = arg.substr(2);
      std::string var, value;
      cmCacheManager::CacheEntryType type;
      if(cmCacheManager::ParseEntry(entry.c_str(), var, value, type))
        {
          cmCacheManager::GetInstance()->AddCacheEntry(
            var.c_str(), 
            value.c_str(),
            "No help, variable specified on the command line.",
            type);
        }
      else
        {
        std::cerr << "Parse error in command line argument: " << arg << "\n"
                  << "Should be: VAR:type=value\n";
        }        
      }
    else if(arg.find("-C",0) == 0)
      {
      std::string path = arg.substr(2);
      std::cerr << "loading initial cache file " << path.c_str() << "\n";
      if(!builder.ReadListFile(path.c_str()))
        {
        std::cerr << "Error in reading cmake initial cache file:"
                  << path.c_str() << "\n";
        }
      }
    }
}

// Parse the args
void cmake::SetArgs(cmMakefile& builder, const std::vector<std::string>& args)
{
  m_Local = false;
  bool directoriesSet = false;
  // watch for cmake and cmake srcdir invocations
  if (args.size() <= 2)
    {
    directoriesSet = true;
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
      directoriesSet = true;
      std::string path = arg.substr(2);
      builder.SetHomeDirectory(path.c_str());
      }
    else if(arg.find("-S",0) == 0)
      {
      directoriesSet = true;
      m_Local = true;
      std::string path = arg.substr(2);
      builder.SetStartDirectory(path.c_str());
      }
    else if(arg.find("-O",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      builder.SetStartOutputDirectory(path.c_str());
      }
    else if(arg.find("-B",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      builder.SetHomeOutputDirectory(path.c_str());
      }
    else if(arg.find("-V",0) == 0)
      {
	m_Verbose = true;
      }
    else if(arg.find("-D",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-C",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      cmMakefileGenerator* gen = 
        cmMakefileGenerator::CreateGenerator(value.c_str());
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        }
      else
        {
        builder.SetMakefileGenerator(gen);
        }
      }
    // no option assume it is the path to the source
    else
      {
      directoriesSet = true;
      builder.SetHomeOutputDirectory
        (cmSystemTools::GetCurrentWorkingDirectory().c_str());
      builder.SetStartOutputDirectory
        (cmSystemTools::GetCurrentWorkingDirectory().c_str());
      builder.SetHomeDirectory
        (cmSystemTools::CollapseFullPath(arg.c_str()).c_str());
      builder.SetStartDirectory
        (cmSystemTools::CollapseFullPath(arg.c_str()).c_str());
      }
    }
  if(!directoriesSet)
    {
    builder.SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    builder.SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    builder.SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    builder.SetStartDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
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
    ("CMAKE_COMMAND",cMakeSelf.c_str(), "Path to CMake executable.",
     cmCacheManager::INTERNAL);

  // Find ccommand
  std::string cCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
    "/ccommand" + cmSystemTools::GetFilenameExtension(cMakeSelf);
  if( cmSystemTools::FileExists(cCommand.c_str()))
    {
    // Save the value in the cache
    cmCacheManager::GetInstance()->AddCacheEntry
      ("CCOMMAND_COMMAND",cCommand.c_str(),
       "Path to CMakeCommand executable.", cmCacheManager::INTERNAL);
    }

  // Find and save the command to edit the cache
  std::string editCacheCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
    "/ccmake" + cmSystemTools::GetFilenameExtension(cMakeSelf);
  if( !cmSystemTools::FileExists(editCacheCommand.c_str()))
    {
    editCacheCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
      "/CMakeSetup" + cmSystemTools::GetFilenameExtension(cMakeSelf);
    }
  if(cmSystemTools::FileExists(editCacheCommand.c_str()))
    {
    cmCacheManager::GetInstance()->AddCacheEntry
      ("CMAKE_EDIT_COMMAND", editCacheCommand.c_str(),
       "Path to cache edit program executable.", cmCacheManager::INTERNAL);
    }
  
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
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/FindVTK.cmake"; 
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


void cmake::HandleBootstrap(cmMakefile& mf, const std::string& args0)
{
  if (cmSystemTools::GetFilenameNameWithoutExtension(args0) == 
      "bootstrap")
    {
    int done = 0;
    
    while (!done)
      {
      int choice = 0;
      std::cout << 
        "\n\nPlease select the tool you wish to use to build CMake."
        "\nPlease note that selecting a tool here will not limit"
        "\nwhat tools the resulting CMake executable supports.\n\n";
      std::vector<std::string> names;
      cmMakefileGenerator::GetRegisteredGenerators(names);
      int count = 1;
      for(std::vector<std::string>::iterator i =names.begin();
          i != names.end(); ++i, ++count)
        {
        std::cout << "\t" << count << ") " << i->c_str() << "\n";
        }
      std::cin >> choice;
      if (choice > 0 && choice < count)
        {
        done = 1;
        cmMakefileGenerator* gen = 
          cmMakefileGenerator::CreateGenerator(names[choice-1].c_str());
        if(!gen)
          {
          cmSystemTools::Error("Could not create named generator ",
                               names[choice-1].c_str());
          }
        else
          {
          mf.SetMakefileGenerator(gen);
          mf.AddDefinition("CMAKE_BOOTSTRAP","1");
          std::cout << 
            "\n\nThank You. CMake will now generate the appropriate files for\nbeing built with " << names[choice-1].c_str() << "\n\n";
          }
        }
      else
        {
        std::cout << "Please make a selection between 1 and " << 
          count -1 << "\n";
        }
      }
    }
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
  // extract the directory arguments, could create a Generator
  this->SetArgs(mf, args);
  // Read and parse the input makefile
  mf.MakeStartDirectoriesCurrent();
  cmCacheManager::GetInstance()->LoadCache(&mf);
  if(mf.GetDefinition("CMAKE_HOME_DIRECTORY"))
    {
    std::string cacheStart = mf.GetDefinition("CMAKE_HOME_DIRECTORY");
    cacheStart += "/CMakeLists.txt";
    std::string currentStart = mf.GetHomeDirectory();
    currentStart += "/CMakeLists.txt";
    if(!cmSystemTools::SameFile(cacheStart.c_str(), currentStart.c_str()))
      {
      std::string message = "Error: source : ";
      message += currentStart;
      message += "\nDoes not match source used to generate cache: ";
      message += cacheStart;
      message += "\nRe-run cmake with a different source directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  mf.AddCacheDefinition("CMAKE_HOME_DIRECTORY", mf.GetHomeDirectory(),
                        "Start directory with the top level CMakeLists.txt file for this project",
                        cmCacheManager::INTERNAL);
  
  // extract command line arguments that might add cache entries
  this->SetCacheArgs(mf, args);

  // handle bootstraping command
  this->HandleBootstrap(mf,args[0]);
  
    
  // no generator specified on the command line
  if(!mf.GetMakefileGenerator())
    {
    cmMakefileGenerator* gen;
    const char* genName = mf.GetDefinition("CMAKE_GENERATOR");
    if(genName)
      {
      gen = cmMakefileGenerator::CreateGenerator(genName);
      }
    else
      {
#if defined(__BORLANDC__)
      gen = new cmBorlandMakefileGenerator;
#elif defined(_WIN32) && !defined(__CYGWIN__)  
      gen = new cmMSProjectGenerator;
#else
      gen = new cmUnixMakefileGenerator;
#endif
      }
    if(!gen)
      {
      cmSystemTools::Error("Could not create generator");
      return -1;
      }
    mf.SetMakefileGenerator(gen);
    // add the 
    }
  cmMakefileGenerator* gen = mf.GetMakefileGenerator();
  gen->SetLocal(m_Local);
  const char* genName = mf.GetDefinition("CMAKE_GENERATOR");
  if(genName)
    {
    if(strcmp(gen->GetName(), genName) != 0)
      {
      std::string message = "Error: generator : ";
      message += gen->GetName();
      message += "\nDoes not match the generator used previously: ";
      message += genName;
      message +=
        "\nEither remove the CMakeCache.txt file or choose a different"
        " binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  if(!mf.GetDefinition("CMAKE_GENERATOR"))
    {
    mf.AddCacheDefinition("CMAKE_GENERATOR",
                          gen->GetName(),
                          "Name of generator.",
                          cmCacheManager::INTERNAL);
    }
  

  // setup CMAKE_ROOT and CMAKE_COMMAND
  this->AddCMakePaths(args);

  // reset any system configuration information
  cmMakefileGenerator::ClearEnabledLanguages();

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

