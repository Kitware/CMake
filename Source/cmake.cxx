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
#include "time.h"
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
  this->RegisterGenerator(new cmMSProjectGenerator);
  this->RegisterGenerator(new cmMSDotNETGenerator);
  this->RegisterGenerator(new cmNMakeMakefileGenerator);
  this->RegisterGenerator(new cmBorlandMakefileGenerator);
#else
  this->RegisterGenerator(new cmUnixMakefileGenerator);
#endif
}

cmake::~cmake()
{
  for(std::map<cmStdString, cmMakefileGenerator*>::iterator i
        = m_RegisteredGenerators.begin(); 
      i != m_RegisteredGenerators.end(); ++i)
    {
    delete i->second;
    }
   m_RegisteredGenerators = std::map<cmStdString, cmMakefileGenerator*>();
}


void cmake::Usage(const char* program)
{
  cmStringStream errorStream;

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
  this->GetRegisteredGenerators(names);
  for(std::vector<std::string>::iterator i =names.begin();
      i != names.end(); ++i)
    {
    errorStream << "\"" << i->c_str() << "\" ";
    }
	  errorStream << ")\n";

  cmSystemTools::Error(errorStream.str().c_str());
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
          this->m_CacheManager.AddCacheEntry(
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
        this->CreateGenerator(value.c_str());
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
int cmake::AddCMakePaths(const std::vector<std::string>& args)
{
  // Find our own executable.
  std::vector<cmStdString> failures;
  std::string cMakeSelf = args[0];
  cmSystemTools::ConvertToUnixSlashes(cMakeSelf);
  failures.push_back(cMakeSelf);
  cMakeSelf = cmSystemTools::FindProgram(cMakeSelf.c_str());
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
#ifdef CMAKE_BUILD_DIR
  std::string intdir = ".";
#ifdef  CMAKE_INTDIR
  intdir = CMAKE_INTDIR;
#endif
  cMakeSelf = CMAKE_BUILD_DIR;
  cMakeSelf += "/Source/";
  cMakeSelf += intdir;
  cMakeSelf += "/cmake";
  cMakeSelf += cmSystemTools::GetExecutableExtension();
#endif
    }
#ifdef CMAKE_PREFIX
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    failures.push_back(cMakeSelf);
    cMakeSelf = CMAKE_PREFIX "/bin/cmake";
    }
#endif
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    failures.push_back(cMakeSelf);
    cmStringStream msg;
    msg << "CMAKE can not find the command line program cmake.\n";
    msg << "  argv[0] = \"" << args[0].c_str() << "\"\n";
    msg << "  Attempted paths:\n";
    std::vector<cmStdString>::iterator i;
    for(i=failures.begin(); i != failures.end(); ++i)
      {
      msg << "    \"" << i->c_str() << "\"\n";
      }
    cmSystemTools::Error(msg.str().c_str());
    return 0;
    }
  // Save the value in the cache
  this->m_CacheManager.AddCacheEntry
    ("CMAKE_COMMAND",cMakeSelf.c_str(), "Path to CMake executable.",
     cmCacheManager::INTERNAL);

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
    this->m_CacheManager.AddCacheEntry
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
    return 0;
    }
  this->m_CacheManager.AddCacheEntry
    ("CMAKE_ROOT", cMakeRoot.c_str(),
     "Path to CMake installation.", cmCacheManager::INTERNAL);
  return 1;
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
  mf.SetCacheManager(&this->m_CacheManager);
  
  // extract the directory arguments, could create a Generator
  this->SetArgs(mf, args);
  // Read and parse the input makefile
  mf.MakeStartDirectoriesCurrent();
  this->m_CacheManager.LoadCache(&mf);
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

  // no generator specified on the command line
  if(!mf.GetMakefileGenerator())
    {
    cmMakefileGenerator* gen;
    const char* genName = mf.GetDefinition("CMAKE_GENERATOR");
    if(genName)
      {
      gen = this->CreateGenerator(genName);
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
  if(!this->AddCMakePaths(args))
    {
    return -3;
    }

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
  if(!this->m_CacheManager.GetCacheValue("LIBRARY_OUTPUT_PATH"))
    {
    this->m_CacheManager.AddCacheEntry("LIBRARY_OUTPUT_PATH", "",
                                                 "Single output directory for building all libraries.",
                                                 cmCacheManager::PATH);
    } 
  if(!this->m_CacheManager.GetCacheValue("EXECUTABLE_OUTPUT_PATH"))
    {
    this->m_CacheManager.AddCacheEntry("EXECUTABLE_OUTPUT_PATH", "",
                                                 "Single output directory for building all executables.",
                                                 cmCacheManager::PATH);
    }  
  
  this->m_CacheManager.SaveCache(&mf);
  
  if(m_Verbose)
    {
    this->m_CacheManager.PrintCache(std::cout);
    }
  
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}


void CMakeCommandUsage(const char* program)
{
  cmStringStream errorStream;

  errorStream 
    << "cmake version " << cmMakefile::GetMajorVersion()
    << "." << cmMakefile::GetMinorVersion() << "\n";

  errorStream 
    << "Usage: " << program << " -E [command] [arguments ...]\n"
    << "Available commands: \n"
    << "  chdir dir cmd [args]... - run command in a given directory\n"
    << "  copy file destination   - copy file to destination (either file or directory)\n"
    << "  remove file1 file2 ...  - remove the file(s)\n"
    << "  time command [args] ... - run command and return elapsed time\n";
#if defined(_WIN32) && !defined(__CYGWIN__)
  errorStream
    << "  write_regv key value    - write registry value\n"
    << "  delete_regv key         - delete registry value\n";
#endif

  cmSystemTools::Error(errorStream.str().c_str());
}

int cmake::CMakeCommand(std::vector<std::string>& args)
{
  if (args.size() > 1)
    {
    // Copy file
    if (args[1] == "copy" && args.size() == 4)
      {
      cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str());
      return cmSystemTools::GetErrorOccuredFlag();
      }

    // Remove file
    else if (args[1] == "remove" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
	{
        if(args[cc] != "-f")
          {
          if(args[cc] == "\\-f")
            {
            args[cc] = "-f";
            }
          cmSystemTools::RemoveFile(args[cc].c_str());
          }
	}
      return 0;
      }

    // Clock command
    else if (args[1] == "time" && args.size() > 2)
      {
      std::string command = args[2];
      std::string output;
      for (std::string::size_type cc = 3; cc < args.size(); cc ++)
	{
        command += " ";
        command += args[cc];
	}

      clock_t clock_start, clock_finish;
      time_t time_start, time_finish;

      time(&time_start);
      clock_start = clock();
      
      cmSystemTools::RunCommand(command.c_str(), output, 0, true);

      clock_finish = clock();
      time(&time_finish);

      std::cout << output.c_str();

      double clocks_per_sec = (double)CLOCKS_PER_SEC;
      std::cout << "Elapsed time: " 
                << (long)(time_finish - time_start) << " s. (time)"
                << ", " 
                << (double)(clock_finish - clock_start) / clocks_per_sec 
                << " s. (clock)"
                << "\n";
      return 0;
    }

    // Clock command
    else if (args[1] == "chdir" && args.size() > 2)
      {
      std::string directory = args[2];
      std::string command = args[3];
      std::string output;
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
	{
        command += " ";
        command += args[cc];
	}

      int retval = 0;
      if ( cmSystemTools::RunCommand(command.c_str(), output, retval, 
				     directory.c_str(), true) )
	{
	std::cout << output.c_str();
	return retval;
	}	

      return 1;
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Write registry value
    else if (args[1] == "write_regv" && args.size() > 3)
      {
      return cmSystemTools::WriteRegistryValue(args[2].c_str(), 
                                               args[3].c_str()) ? 0 : 1;
      }

    // Delete registry value
    else if (args[1] == "delete_regv" && args.size() > 2)
      {
      return cmSystemTools::DeleteRegistryValue(args[2].c_str()) ? 0 : 1;
      }
#endif
    }

  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}

void cmake::GetRegisteredGenerators(std::vector<std::string>& names)
{
  for(std::map<cmStdString, cmMakefileGenerator*>::iterator i
        = this->m_RegisteredGenerators.begin(); 
      i != this->m_RegisteredGenerators.end(); ++i)
    {
    names.push_back(i->first);
    }
}


void cmake::RegisterGenerator(cmMakefileGenerator* mg)
{
  std::map<cmStdString, cmMakefileGenerator*>::iterator i = 
    this->m_RegisteredGenerators.find(mg->GetName());
  // delete re-registered objects
  if(i != this->m_RegisteredGenerators.end())
    {
    delete i->second;
    }
  this->m_RegisteredGenerators[mg->GetName()] = mg;
}

cmMakefileGenerator* cmake::CreateGenerator(const char* name)
{
  std::map<cmStdString, cmMakefileGenerator*>::iterator i;
  for(i = this->m_RegisteredGenerators.begin();
      i != this->m_RegisteredGenerators.end(); ++i)
    {
    cmMakefileGenerator* gen = i->second;
    if(strcmp(name, gen->GetName()) == 0)
      {
      return gen->CreateObject();
      }
    }
  return 0;
}

