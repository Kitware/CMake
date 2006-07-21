/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmake.h"
#include "time.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCommand.h"
#include "cmFileTimeComparison.h"
#include "cmGeneratedFileStream.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include "cmDependsFortran.h" // For -E cmake_copy_f90_mod callback.
# include "cmVariableWatch.h"
# include "cmVersion.h"
# include <cmsys/Terminal.h>
#endif

# include <cmsys/Directory.hxx>
#include <cmsys/Process.h>

// only build kdevelop generator on non-windows platforms
// when not bootstrapping cmake
#if !defined(_WIN32)
# if defined(CMAKE_BUILD_WITH_CMAKE)
#   define CMAKE_USE_KDEVELOP
# endif
#endif

#if defined(__MINGW32__) && !defined(CMAKE_BUILD_WITH_CMAKE)
# define CMAKE_BOOT_MINGW
#endif

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(CMAKE_BOOT_MINGW)
#    include "cmGlobalVisualStudio6Generator.h"
#    include "cmGlobalVisualStudio7Generator.h"
#    include "cmGlobalVisualStudio71Generator.h"
#    include "cmGlobalVisualStudio8Generator.h"
#    include "cmGlobalVisualStudio8Win64Generator.h"
#    include "cmGlobalBorlandMakefileGenerator.h"
#    include "cmGlobalNMakeMakefileGenerator.h"
#    include "cmGlobalWatcomWMakeGenerator.h"
#  endif
#  include "cmGlobalMSYSMakefileGenerator.h"
#  include "cmGlobalMinGWMakefileGenerator.h"
#  include "cmWin32ProcessExecution.h"
#else
#endif
#include "cmGlobalUnixMakefileGenerator3.h"

#ifdef CMAKE_USE_KDEVELOP
# include "cmGlobalKdevelopGenerator.h"
#endif

#include <stdlib.h> // required for atoi

#if defined( __APPLE__ )
#  if defined(CMAKE_BUILD_WITH_CMAKE)
#    include "cmGlobalXCodeGenerator.h"
#    define CMAKE_USE_XCODE 1
#  endif
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <sys/stat.h> // struct stat

#include <memory> // auto_ptr

void cmNeedBackwardsCompatibility(const std::string& variable,
                                  int access_type, void* )
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  if (access_type == cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS)
    {
    std::string message = "An attempt was made to access a variable: ";
    message += variable;
    message +=
      " that has not been defined. Some variables were always defined "
      "by CMake in versions prior to 1.6. To fix this you might need to set "
      "the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.4 or less. If "
      "you are writing a CMakeList file, (or have already set "
      "CMAKE_BACKWARDS_COMPATABILITY to 1.4 or less) then you probably need "
      "to include a CMake module to test for the feature this variable "
      "defines.";
    cmSystemTools::Error(message.c_str());
    }
#else
  (void)variable;
  (void)access_type;
#endif
}

cmake::cmake()
{
  this->DebugTryCompile = false;
  this->ClearBuildSystem = false;
  this->FileComparison = new cmFileTimeComparison;

#ifdef __APPLE__
  struct rlimit rlp;
  if(!getrlimit(RLIMIT_STACK, &rlp))
    {
    if(rlp.rlim_cur != rlp.rlim_max)
      {
        rlp.rlim_cur = rlp.rlim_max;
         setrlimit(RLIMIT_STACK, &rlp);
      }
    }
#endif

  // If MAKEFLAGS are given in the environment, remove the environment
  // variable.  This will prevent try-compile from succeeding when it
  // should fail (if "-i" is an option).  We cannot simply test
  // whether "-i" is given and remove it because some make programs
  // encode the MAKEFLAGS variable in a strange way.
  if(getenv("MAKEFLAGS"))
    {
    cmSystemTools::PutEnv("MAKEFLAGS=");
    }

  this->Verbose = false;
  this->InTryCompile = false;
  this->CacheManager = new cmCacheManager;
  this->GlobalGenerator = 0;
  this->ProgressCallback = 0;
  this->ProgressCallbackClientData = 0;
  this->ScriptMode = false;

#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch = new cmVariableWatch;
  this->VariableWatch->AddWatch("CMAKE_WORDS_BIGENDIAN",
                            cmNeedBackwardsCompatibility);
  this->VariableWatch->AddWatch("CMAKE_SIZEOF_INT",
                            cmNeedBackwardsCompatibility);
  this->VariableWatch->AddWatch("CMAKE_X_LIBS",
                            cmNeedBackwardsCompatibility);
#endif

  this->AddDefaultGenerators();
  this->AddDefaultCommands();

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

cmake::~cmake()
{
  delete this->CacheManager;
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    this->GlobalGenerator = 0;
    }
  for(RegisteredCommandsMap::iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    delete (*j).second;
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  delete this->VariableWatch;
#endif
  delete this->FileComparison;
}

void cmake::CleanupCommandsAndMacros()
{
  std::vector<cmCommand*> commands;
  for(RegisteredCommandsMap::iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    if ( !j->second->IsA("cmMacroHelperCommand") )
      {
      commands.push_back(j->second);
      }
    else
      {
      delete j->second;
      }
    }
  this->Commands.erase(this->Commands.begin(), this->Commands.end());
  std::vector<cmCommand*>::iterator it;
  for ( it = commands.begin(); it != commands.end();
    ++ it )
    {
    this->Commands[cmSystemTools::LowerCase((*it)->GetName())] = *it;
    }
}

bool cmake::CommandExists(const char* name) const
{
  std::string sName = cmSystemTools::LowerCase(name);
  return (this->Commands.find(sName) != this->Commands.end());
}

cmCommand *cmake::GetCommand(const char *name)
{
  cmCommand* rm = 0;
  std::string sName = cmSystemTools::LowerCase(name);
  RegisteredCommandsMap::iterator pos = this->Commands.find(sName);
  if (pos != this->Commands.end())
    {
    rm = (*pos).second;
    }
  return rm;
}

void cmake::RenameCommand(const char*oldName, const char* newName)
{
  // if the command already exists, free the old one
  std::string sOldName = cmSystemTools::LowerCase(oldName);
  std::string sNewName = cmSystemTools::LowerCase(newName);
  RegisteredCommandsMap::iterator pos = this->Commands.find(sOldName);
  if ( pos == this->Commands.end() )
    {
    return;
    }
  cmCommand* cmd = pos->second;

  pos = this->Commands.find(sNewName);
  if (pos != this->Commands.end())
    {
    delete pos->second;
    this->Commands.erase(pos);
    }
  this->Commands.insert(RegisteredCommandsMap::value_type(sNewName, cmd));
  pos = this->Commands.find(sOldName);
  this->Commands.erase(pos);
}

void cmake::AddCommand(cmCommand* wg)
{
  std::string name = cmSystemTools::LowerCase(wg->GetName());
  // if the command already exists, free the old one
  RegisteredCommandsMap::iterator pos = this->Commands.find(name);
  if (pos != this->Commands.end())
    {
    delete pos->second;
    this->Commands.erase(pos);
    }
  this->Commands.insert( RegisteredCommandsMap::value_type(name, wg));
}

// Parse the args
bool cmake::SetCacheArgs(const std::vector<std::string>& args)
{
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0)
      {
      std::string entry = arg.substr(2);
      if(entry.size() == 0)
        {
        ++i;
        if(i < args.size())
          {
          entry = args[i];
          }
        else
          {
          cmSystemTools::Error("-D must be followed with VAR=VALUE.");
          return false;
          }
        }
      std::string var, value;
      cmCacheManager::CacheEntryType type = cmCacheManager::UNINITIALIZED;
      if(cmCacheManager::ParseEntry(entry.c_str(), var, value, type) ||
        cmCacheManager::ParseEntry(entry.c_str(), var, value))
        {
        this->CacheManager->AddCacheEntry(var.c_str(), value.c_str(),
          "No help, variable specified on the command line.",
          type);
        }
      else
        {
        std::cerr << "Parse error in command line argument: " << arg << "\n"
                  << "Should be: VAR:type=value\n";
        cmSystemTools::Error("No cmake scrpt provided.");
        return false;
        }
      }
    else if(arg.find("-C",0) == 0)
      {
      std::string path = arg.substr(2);
      if ( path.size() == 0 )
        {
        ++i;
        if(i < args.size())
          {
          path = args[i];
          }
        else
          {
          cmSystemTools::Error("-C must be followed by a file name.");
          return false;
          }
        }
      std::cerr << "loading initial cache file " << path.c_str() << "\n";
      this->ReadListFile(path.c_str());
      }
    else if(arg.find("-P",0) == 0)
      {
      i++;
      if(i >= args.size())
        {
        cmSystemTools::Error("-P must be followed by a file name.");
        return false;
        }
      std::string path = args[i];
      if ( path.size() == 0 )
        {
        cmSystemTools::Error("No cmake script provided.");
        return false;
        }
      this->ReadListFile(path.c_str());
      }
    }
  return true;
}

void cmake::ReadListFile(const char *path)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator *gg = this->GetGlobalGenerator();
  bool created = false;

  // if a generator was not specified use a generic one
  if (!gg)
    {
    gg = new cmGlobalGenerator;
    gg->SetCMakeInstance(this);
    created = true;
    }

  // read in the list file to fill the cache
  if(path)
    {
    std::auto_ptr<cmLocalGenerator> lg(gg->CreateLocalGenerator());
    lg->SetGlobalGenerator(gg);
    lg->GetMakefile()->SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetStartDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    if (!lg->GetMakefile()->ReadListFile(0, path))
      {
      std::cerr << "Error processing file:" << path << "\n";
      }
    }

  // free generic one if generated
  if (created)
    {
    delete gg;
    }
}

// Parse the args
void cmake::SetArgs(const std::vector<std::string>& args)
{
  bool directoriesSet = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-H",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeDirectory(path.c_str());
      }
    else if(arg.find("-S",0) == 0)
      {
      // There is no local generate anymore.  Ignore -S option.
      }
    else if(arg.find("-O",0) == 0)
      {
      // There is no local generate anymore.  Ignore -O option.
      }
    else if(arg.find("-B",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeOutputDirectory(path.c_str());
      }
    else if((i < args.size()-1) && (arg.find("--check-build-system",0) == 0))
      {
      this->CheckBuildSystemArgument = args[++i];
      this->ClearBuildSystem = (atoi(args[++i].c_str()) > 0);
      }
    else if(arg.find("-V",0) == 0)
      {
        this->Verbose = true;
      }
    else if(arg.find("-D",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-C",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-P",0) == 0)
      {
      // skip for now
      i++;
      }
    else if(arg.find("--graphviz=",0) == 0)
      {
      std::string path = arg.substr(strlen("--graphviz="));
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->GraphVizFile = path;
      if ( this->GraphVizFile.empty() )
        {
        cmSystemTools::Error("No file specified for --graphviz");
        }
      }
    else if(arg.find("--debug-trycompile",0) == 0)
      {
      std::cout << "debug trycompile on\n";
      this->DebugTryCompileOn();
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.size() == 0)
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No generator specified for -G");
          return;
          }
        value = args[i];
        }
      cmGlobalGenerator* gen =
        this->CreateGlobalGenerator(value.c_str());
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        }
      else
        {
        this->SetGlobalGenerator(gen);
        }
      }
    // no option assume it is the path to the source
    else
      {
      directoriesSet = true;
      this->SetDirectoriesFromFile(arg.c_str());
      }
    }
  if(!directoriesSet)
    {
    this->SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetStartDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    }

  this->SetStartDirectory(this->GetHomeDirectory());
  this->SetStartOutputDirectory(this->GetHomeOutputDirectory());
}

//----------------------------------------------------------------------------
void cmake::SetDirectoriesFromFile(const char* arg)
{
  // Check if the argument refers to a CMakeCache.txt or
  // CMakeLists.txt file.
  std::string listPath;
  std::string cachePath;
  bool argIsFile = false;
  if(cmSystemTools::FileIsDirectory(arg))
    {
    std::string path = cmSystemTools::CollapseFullPath(arg);
    cmSystemTools::ConvertToUnixSlashes(path);
    std::string cacheFile = path;
    cacheFile += "/CMakeCache.txt";
    std::string listFile = path;
    listFile += "/CMakeLists.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cachePath = path;
      }
    if(cmSystemTools::FileExists(listFile.c_str()))
      {
      listPath = path;
      }
    }
  else if(cmSystemTools::FileExists(arg))
    {
    argIsFile = true;
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath.c_str());
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt")
      {
      cachePath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    else if(name == "cmakelists.txt")
      {
      listPath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    }
  else
    {
    // Specified file or directory does not exist.  Try to set things
    // up to produce a meaningful error message.
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath.c_str());
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt" || name == "cmakelists.txt")
      {
      argIsFile = true;
      listPath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    else
      {
      listPath = fullPath;
      }
    }

  // If there is a CMakeCache.txt file, use its settings.
  if(cachePath.length() > 0)
    {
    cmCacheManager* cachem = this->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    if(cachem->LoadCache(cachePath.c_str()) &&
      it.Find("CMAKE_HOME_DIRECTORY"))
      {
      this->SetHomeOutputDirectory(cachePath.c_str());
      this->SetStartOutputDirectory(cachePath.c_str());
      this->SetHomeDirectory(it.GetValue());
      this->SetStartDirectory(it.GetValue());
      return;
      }
    }

  // If there is a CMakeLists.txt file, use it as the source tree.
  if(listPath.length() > 0)
    {
    this->SetHomeDirectory(listPath.c_str());
    this->SetStartDirectory(listPath.c_str());

    if(argIsFile)
      {
      // Source CMakeLists.txt file given.  It was probably dropped
      // onto the executable in a GUI.  Default to an in-source build.
      this->SetHomeOutputDirectory(listPath.c_str());
      this->SetStartOutputDirectory(listPath.c_str());
      }
    else
      {
      // Source directory given on command line.  Use current working
      // directory as build tree.
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      this->SetHomeOutputDirectory(cwd.c_str());
      this->SetStartOutputDirectory(cwd.c_str());
      }
    return;
    }

  // We didn't find a CMakeLists.txt or CMakeCache.txt file from the
  // argument.  Assume it is the path to the source tree, and use the
  // current working directory as the build tree.
  std::string full = cmSystemTools::CollapseFullPath(arg);
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  this->SetHomeDirectory(full.c_str());
  this->SetStartDirectory(full.c_str());
  this->SetHomeOutputDirectory(cwd.c_str());
  this->SetStartOutputDirectory(cwd.c_str());
}

// at the end of this CMAKE_ROOT and CMAKE_COMMAND should be added to the
// cache
int cmake::AddCMakePaths(const char *arg0)
{
  // Find our own executable.
  std::vector<cmStdString> failures;
  std::string cMakeSelf = arg0;
  cmSystemTools::ConvertToUnixSlashes(cMakeSelf);
  failures.push_back(cMakeSelf);
  cMakeSelf = cmSystemTools::FindProgram(cMakeSelf.c_str());
  cmSystemTools::ConvertToUnixSlashes(cMakeSelf);
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
#ifdef CMAKE_BUILD_DIR
  std::string intdir = ".";
#ifdef  CMAKE_INTDIR
  intdir = CMAKE_INTDIR;
#endif
  cMakeSelf = CMAKE_BUILD_DIR;
  cMakeSelf += "/bin/";
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
    cmOStringStream msg;
    msg << "CMAKE can not find the command line program cmake.\n";
    msg << "  argv[0] = \"" << arg0 << "\"\n";
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
  this->CacheManager->AddCacheEntry
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
  std::string ctestCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
    "/ctest" + cmSystemTools::GetFilenameExtension(cMakeSelf);
  if(cmSystemTools::FileExists(ctestCommand.c_str()))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_CTEST_COMMAND", ctestCommand.c_str(),
       "Path to ctest program executable.", cmCacheManager::INTERNAL);
    }
  if(cmSystemTools::FileExists(editCacheCommand.c_str()))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_EDIT_COMMAND", editCacheCommand.c_str(),
       "Path to cache edit program executable.", cmCacheManager::INTERNAL);
    }

  // do CMAKE_ROOT, look for the environment variable first
  std::string cMakeRoot;
  std::string modules;
  if (getenv("CMAKE_ROOT"))
    {
    cMakeRoot = getenv("CMAKE_ROOT");
    modules = cMakeRoot + "/Modules/CMake.cmake";
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
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }

  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try exe/../share/cmake
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#ifdef CMAKE_ROOT_DIR
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in root directory
    cMakeRoot = CMAKE_ROOT_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#endif
#ifdef CMAKE_PREFIX
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in install prefix
    cMakeRoot = CMAKE_PREFIX CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#endif
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot +  "/Modules/CMake.cmake";
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // couldn't find modules
    cmSystemTools::Error("Could not find CMAKE_ROOT !!!\n"
      "CMake has most likely not been installed correctly.\n"
      "Modules directory not found in\n",
      cMakeRoot.c_str());
    return 0;
    }
  this->CacheManager->AddCacheEntry
    ("CMAKE_ROOT", cMakeRoot.c_str(),
     "Path to CMake installation.", cmCacheManager::INTERNAL);

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  return 1;
}



void CMakeCommandUsage(const char* program)
{
  cmOStringStream errorStream;

#ifdef CMAKE_BUILD_WITH_CMAKE
  errorStream
    << "cmake version " << cmVersion::GetCMakeVersion() << "\n";
#else
  errorStream
    << "cmake bootstrap\n";
#endif

  errorStream
    << "Usage: " << program << " -E [command] [arguments ...]\n"
    << "Available commands: \n"
    << "  chdir dir cmd [args]... - run command in a given directory\n"
    << "  copy file destination   - copy file to destination (either file or "
    "directory)\n"
    << "  copy_if_different in-file out-file   - copy file if input has "
    "changed\n"
    << "  copy_directory source destination    - copy directory 'source' "
    "content to directory 'destination'\n"
    << "  compare_files file1 file2 - check if file1 is same as file2\n"
    << "  echo [string]...        - displays arguments as text\n"
    << "  echo_append [string]... - displays arguments as text but no new"
    " line\n"
    << "  environment             - display the current enviroment\n"
    << "  remove file1 file2 ...  - remove the file(s)\n"
    << "  tar [cxt][vfz] file.tar file/dir1 file/dir2 ... - create a tar.\n"
    << "  time command [args] ... - run command and return elapsed time\n"
#if defined(_WIN32) && !defined(__CYGWIN__)
    << "  write_regv key value    - write registry value\n"
    << "  delete_regv key         - delete registry value\n"
    << "  comspec                 - on windows 9x use this for RunCommand\n"
#else
    << "  create_symlink old new  - create a symbolic link new -> old\n"
#endif
    ;

  cmSystemTools::Error(errorStream.str().c_str());
}

int cmake::ExecuteCMakeCommand(std::vector<std::string>& args)
{
  if (args.size() > 1)
    {
    // Copy file
    if (args[1] == "copy" && args.size() == 4)
      {
      if(!cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Error copying file \"" << args[2].c_str()
                  << "\" to \"" << args[3].c_str() << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy file if different.
    if (args[1] == "copy_if_different" && args.size() == 4)
      {
      if(!cmSystemTools::CopyFileIfDifferent(args[2].c_str(),
          args[3].c_str()))
        {
        std::cerr << "Error copying file (if different) from \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy directory content
    if (args[1] == "copy_directory" && args.size() == 4)
      {
      if(!cmSystemTools::CopyADirectory(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Error copying directory from \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Compare files
    if (args[1] == "compare_files" && args.size() == 4)
      {
      if(cmSystemTools::FilesDiffer(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Files \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\" are different.\n";
        return 1;
        }
      return 0;
      }

    // Echo string
    else if (args[1] == "echo" )
      {
      unsigned int cc;
      const char* space = "";
      for ( cc = 2; cc < args.size(); cc ++ )
        {
        std::cout << space << args[cc];
        space = " ";
        }
      std::cout << std::endl;
      return 0;
      }

    // Echo string no new line
    else if (args[1] == "echo_append" )
      {
      unsigned int cc;
      const char* space = "";
      for ( cc = 2; cc < args.size(); cc ++ )
        {
        std::cout << space << args[cc];
        space = " ";
        }
      return 0;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    else if (args[1] == "environment" )
      {
      std::vector<std::string> env = cmSystemTools::GetEnvironmentVariables();
      std::vector<std::string>::iterator it;
      for ( it = env.begin(); it != env.end(); ++ it )
        {
        std::cout << it->c_str() << std::endl;
        }
      return 0;
      }
#endif

    // Remove file
    else if (args[1] == "remove" && args.size() > 2)
      {
      bool force = false;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        if(args[cc] == "\\-f" || args[cc] == "-f")
          {
          force = true;
          }
        else
          {
          // Complain if the file could not be removed, still exists,
          // and the -f option was not given.
          if(!cmSystemTools::RemoveFile(args[cc].c_str()) && !force &&
             cmSystemTools::FileExists(args[cc].c_str()))
            {
            return 1;
            }
          }
        }
      return 0;
      }

    // Clock command
    else if (args[1] == "time" && args.size() > 2)
      {
      std::string command = args[2];
      for (std::string::size_type cc = 3; cc < args.size(); cc ++)
        {
        command += " ";
        command += args[cc];
        }

      clock_t clock_start, clock_finish;
      time_t time_start, time_finish;

      time(&time_start);
      clock_start = clock();

      cmSystemTools::RunSingleCommand(command.c_str());

      clock_finish = clock();
      time(&time_finish);

      double clocks_per_sec = static_cast<double>(CLOCKS_PER_SEC);
      std::cout << "Elapsed time: "
        << static_cast<long>(time_finish - time_start) << " s. (time)"
        << ", "
        << static_cast<double>(clock_finish - clock_start) / clocks_per_sec
        << " s. (clock)"
        << "\n";
      return 0;
    }

    // Command to change directory and run a program.
    else if (args[1] == "chdir" && args.size() >= 4)
      {
      std::string directory = args[2];
      if(!cmSystemTools::FileExists(directory.c_str()))
        {
        cmSystemTools::Error("Directory does not exist for chdir command: ",
                             args[2].c_str());
        return 0;
        }

      std::string command = "\"";
      command += args[3];
      command += "\"";
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
        {
        command += " \"";
        command += args[cc];
        command += "\"";
        }
      int retval = 0;
      int timeout = 0;
      if ( cmSystemTools::RunSingleCommand(command.c_str(), 0, &retval,
                                           directory.c_str(), true, timeout) )
        {
        return retval;
        }

      return 1;
      }

    // Command to start progress for a build
    else if (args[1] == "cmake_progress_start" && args.size() == 4)
      {
      // bascially remove the directory
      std::string dirName = args[2];
      dirName += "/Progress";
      cmSystemTools::RemoveADirectory(dirName.c_str());
      cmSystemTools::MakeDirectory(dirName.c_str());
      // write the count into the directory
      std::string fName = dirName;
      fName += "/count.txt";
      FILE *progFile = fopen(fName.c_str(),"w");
      if (progFile)
        {
        int count = atoi(args[3].c_str());
        fprintf(progFile,"%i\n",count);
        fclose(progFile);
        }
      return 0;
      }

    // Command to report progress for a build
    else if (args[1] == "cmake_progress_report" && args.size() >= 3)
      {
      std::string dirName = args[2];
      dirName += "/Progress";
      std::string fName;
      FILE *progFile;
      unsigned int i;
      for (i = 3; i < args.size(); ++i)
        {
        fName = dirName;
        fName += "/";
        fName += args[i];
        progFile = fopen(fName.c_str(),"w");
        if (progFile)
          {
          fprintf(progFile,"empty");
          fclose(progFile);
          }
        }
      int fileNum = static_cast<int>
        (cmsys::Directory::GetNumberOfFilesInDirectory(dirName.c_str()));
      // read the count
      fName = dirName;
      fName += "/count.txt";
      progFile = fopen(fName.c_str(),"r");
      if (progFile)
        {
        int count = 0;
        fscanf(progFile,"%i",&count);
        if (count > 0)
          {
          // print the progress
          fprintf(stdout,"[%3i%%] ",((fileNum-3)*100)/count);
          }
        fclose(progFile);
        }
      return 0;
      }
    
    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    else if (args[1] == "create_symlink" && args.size() == 4)
      {
      return cmSystemTools::CreateSymlink(args[2].c_str(),
                                          args[3].c_str())? 0:1;
      }

    // Internal CMake shared library support.
    else if (args[1] == "cmake_symlink_library" && args.size() == 5)
      {
      int result = 0;
      std::string realName = args[2];
      std::string soName = args[3];
      std::string name = args[4];
      if(soName != realName)
        {
        std::string fname = cmSystemTools::GetFilenameName(realName);
        if(cmSystemTools::FileExists(soName.c_str()))
          {
          cmSystemTools::RemoveFile(soName.c_str());
          }
        if(!cmSystemTools::CreateSymlink(fname.c_str(), soName.c_str()))
          {
          result = 1;
          }
        }
      if(name != soName)
        {
        std::string fname = cmSystemTools::GetFilenameName(soName);
        if(cmSystemTools::FileExists(soName.c_str()))
          {
          cmSystemTools::RemoveFile(name.c_str());
          }
        if(!cmSystemTools::CreateSymlink(fname.c_str(), name.c_str()))
          {
          result = 1;
          }
        }
      return result;
      }
    // Internal CMake versioned executable support.
    else if (args[1] == "cmake_symlink_executable" && args.size() == 4)
      {
      int result = 0;
      std::string realName = args[2];
      std::string name = args[3];
      if(name != realName)
        {
        std::string fname = cmSystemTools::GetFilenameName(realName);
        if(cmSystemTools::FileExists(realName.c_str()))
          {
          cmSystemTools::RemoveFile(name.c_str());
          }
        if(!cmSystemTools::CreateSymlink(fname.c_str(), name.c_str()))
          {
          result = 1;
          }
        }
      return result;
      }

    // Internal CMake dependency scanning support.
    else if (args[1] == "cmake_depends" && args.size() >= 6)
      {
      // Create a cmake object instance to process dependencies.
      cmake cm;
      std::string gen;
      std::string homeDir;
      std::string startDir;
      std::string homeOutDir;
      std::string startOutDir;
      std::string depInfo;
      if(args.size() >= 8)
        {
        // Full signature:
        //
        //   -E cmake_depends <generator>
        //                    <home-src-dir> <start-src-dir>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info>
        //
        // All paths are provided.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[5];
        startOutDir = args[6];
        depInfo = args[7];
        }
      else
        {
        // Support older signature for existing makefiles:
        //
        //   -E cmake_depends <generator>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info>
        //
        // Just pretend the source directories are the same as the
        // binary directories so at least scanning will work.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[3];
        startOutDir = args[3];
        depInfo = args[5];
        }

      // Create a local generator configured for the directory in
      // which dependencies will be scanned.
      homeDir = cmSystemTools::CollapseFullPath(homeDir.c_str());
      startDir = cmSystemTools::CollapseFullPath(startDir.c_str());
      homeOutDir = cmSystemTools::CollapseFullPath(homeOutDir.c_str());
      startOutDir = cmSystemTools::CollapseFullPath(startOutDir.c_str());
      cm.SetHomeDirectory(homeDir.c_str());
      cm.SetStartDirectory(startDir.c_str());
      cm.SetHomeOutputDirectory(homeOutDir.c_str());
      cm.SetStartOutputDirectory(startOutDir.c_str());
      if(cmGlobalGenerator* ggd = cm.CreateGlobalGenerator(gen.c_str()))
        {
        cm.SetGlobalGenerator(ggd);
        std::auto_ptr<cmLocalGenerator> lgd(ggd->CreateLocalGenerator());
        lgd->SetGlobalGenerator(ggd);
        lgd->GetMakefile()->SetStartDirectory(startDir.c_str());
        lgd->GetMakefile()->SetStartOutputDirectory(startOutDir.c_str());
        lgd->GetMakefile()->MakeStartDirectoriesCurrent();
        lgd->SetupPathConversions();

        // Actually scan dependencies.
        return lgd->ScanDependencies(depInfo.c_str())? 0 : 2;
        }
      return 1;
      }

    // Internal CMake link script support.
    else if (args[1] == "cmake_link_script" && args.size() >= 3)
      {
      return cmake::ExecuteLinkScript(args);
      }

#ifdef CMAKE_BUILD_WITH_CMAKE
    // Internal CMake color makefile support.
    else if (args[1] == "cmake_echo_color")
      {
      return cmake::ExecuteEchoColor(args);
      }
#endif

    // Tar files
    else if (args[1] == "tar" && args.size() > 3)
      {
      std::string flags = args[2];
      std::string outFile = args[3];
      std::vector<cmStdString> files;
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
        {
        files.push_back(args[cc]);
        }
      bool gzip = false;
      bool verbose = false;
      if ( flags.find_first_of('z') != flags.npos )
        {
        gzip = true;
        }
      if ( flags.find_first_of('v') != flags.npos )
        {
        verbose = true;
        }

      if ( flags.find_first_of('t') != flags.npos )
        {
        if ( !cmSystemTools::ListTar(outFile.c_str(), files, gzip, verbose) )
          {
          cmSystemTools::Error("Problem creating tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('c') != flags.npos )
        {
        if ( !cmSystemTools::CreateTar(
            outFile.c_str(), files, gzip, verbose) )
          {
          cmSystemTools::Error("Problem creating tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('x') != flags.npos )
        {
        if ( !cmSystemTools::ExtractTar(
            outFile.c_str(), files, gzip, verbose) )
          {
          cmSystemTools::Error("Problem extracting tar: ", outFile.c_str());
          return 1;
          }
        }
      return 0;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    // Internal CMake Fortran module support.
    else if (args[1] == "cmake_copy_f90_mod" && args.size() >= 4)
      {
      return cmDependsFortran::CopyModule(args)? 0 : 1;
      }
#endif

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
    // Remove file
    else if (args[1] == "comspec" && args.size() > 2)
      {
      unsigned int cc;
      std::string command = args[2];
      for ( cc = 3; cc < args.size(); cc ++ )
        {
        command += " " + args[cc];
        }
      return cmWin32ProcessExecution::Windows9xHack(command.c_str());
      }
#endif
    }

  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}

void cmake::GetRegisteredGenerators(std::vector<std::string>& names)
{
  for(RegisteredGeneratorsMap::const_iterator i = this->Generators.begin();
      i != this->Generators.end(); ++i)
    {
    names.push_back(i->first);
    }
}

cmGlobalGenerator* cmake::CreateGlobalGenerator(const char* name)
{
  RegisteredGeneratorsMap::const_iterator i = this->Generators.find(name);
  if(i != this->Generators.end())
    {
    cmGlobalGenerator* generator = (i->second)();
    generator->SetCMakeInstance(this);
    return generator;
    }
  else
    {
    return 0;
    }
}

void cmake::SetHomeDirectory(const char* dir)
{
  this->cmHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(this->cmHomeDirectory);
}

void cmake::SetHomeOutputDirectory(const char* lib)
{
  this->HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(this->HomeOutputDirectory);
}

void cmake::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  // delete the old generator
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    // restore the original environment variables CXX and CC
    // Restor CC
    std::string env = "CC=";
    if(this->CCEnvironment.size())
      {
      env += this->CCEnvironment;
      }
    cmSystemTools::PutEnv(env.c_str());
    env = "CXX=";
    if(this->CXXEnvironment.size())
      {
      env += this->CXXEnvironment;
      }
    cmSystemTools::PutEnv(env.c_str());
    }

  // set the new
  this->GlobalGenerator = gg;

  // set the global flag for unix style paths on cmSystemTools as soon as
  // the generator is set.  This allows gmake to be used on windows.
  cmSystemTools::SetForceUnixPaths
    (this->GlobalGenerator->GetForceUnixPaths());

  // Save the environment variables CXX and CC
  const char* cxx = getenv("CXX");
  const char* cc = getenv("CC");
  if(cxx)
    {
    this->CXXEnvironment = cxx;
    }
  else
    {
    this->CXXEnvironment = "";
    }
  if(cc)
    {
    this->CCEnvironment = cc;
    }
  else
    {
    this->CCEnvironment = "";
    }
  // set the cmake instance just to be sure
  gg->SetCMakeInstance(this);
}

int cmake::DoPreConfigureChecks()
{
  // Make sure the Start directory contains a CMakeLists.txt file.
  std::string srcList = this->GetHomeDirectory();
  srcList += "/CMakeLists.txt";
  if(!cmSystemTools::FileExists(srcList.c_str()))
    {
    cmOStringStream err;
    if(cmSystemTools::FileIsDirectory(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not appear to contain CMakeLists.txt.\n";
      }
    else if(cmSystemTools::FileExists(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" is a file, not a directory.\n";
      }
    else
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not exist.\n";
      }
    err << "Specify --help for usage, or press the help button on the CMake "
      "GUI.";
    cmSystemTools::Error(err.str().c_str());
    return -2;
    }

  // do a sanity check on some values
  if(this->CacheManager->GetCacheValue("CMAKE_HOME_DIRECTORY"))
    {
    std::string cacheStart = 
      this->CacheManager->GetCacheValue("CMAKE_HOME_DIRECTORY");
    cacheStart += "/CMakeLists.txt";
    std::string currentStart = this->GetHomeDirectory();
    currentStart += "/CMakeLists.txt";
    if(!cmSystemTools::SameFile(cacheStart.c_str(), currentStart.c_str()))
      {
      std::string message = "The source \"";
      message += currentStart;
      message += "\" does not match the source \"";
      message += cacheStart;
      message += "\" used to generate cache.  ";
      message += "Re-run cmake with a different source directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    return 0;
    }
  return 1;
}

int cmake::Configure()
{
  // Construct right now our path conversion table before it's too late:
  this->UpdateConversionPathTable();
  this->CleanupCommandsAndMacros();

  int res = 0;
  if ( !this->ScriptMode )
    {
    res = this->DoPreConfigureChecks();
    }
  if ( res < 0 )
    {
    return -2;
    }
  if ( !res )
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_HOME_DIRECTORY", 
       this->GetHomeDirectory(),
       "Start directory with the top level CMakeLists.txt file for this "
       "project",
       cmCacheManager::INTERNAL);
    }

  // set the default BACKWARDS compatibility to the current version
  if(!this->CacheManager->GetCacheValue("CMAKE_BACKWARDS_COMPATIBILITY"))
    {
    char ver[256];
    sprintf(ver,"%i.%i",cmMakefile::GetMajorVersion(),
            cmMakefile::GetMinorVersion());
    this->CacheManager->AddCacheEntry
      ("CMAKE_BACKWARDS_COMPATIBILITY",ver, 
       "For backwards compatibility, what version of CMake commands and "
       "syntax should this version of CMake allow.",
       cmCacheManager::STRING);
    }

  // no generator specified on the command line
  if(!this->GlobalGenerator)
    {
    const char* genName = 
      this->CacheManager->GetCacheValue("CMAKE_GENERATOR");
    if(genName)
      {
      this->GlobalGenerator = this->CreateGlobalGenerator(genName);
      }
    if(this->GlobalGenerator)
      {
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths
        (this->GlobalGenerator->GetForceUnixPaths());
      }
    else
      {
#if defined(__BORLANDC__) && defined(_WIN32)
      this->SetGlobalGenerator(new cmGlobalBorlandMakefileGenerator);
#elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
      std::string installedCompiler;
      std::string mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
        "\\VisualStudio\\8.0\\Setup;Dbghelp_path]";
      cmSystemTools::ExpandRegistryValues(mp);
      if (!(mp == "/registry"))
        {
        installedCompiler = "Visual Studio 8 2005";
        }
      else
        {
        mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
          "\\VisualStudio\\7.1;InstallDir]";
        cmSystemTools::ExpandRegistryValues(mp);
        if (!(mp == "/registry"))
          {
          installedCompiler = "Visual Studio 7 .NET 2003";
          }
        else
          {
          mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
            "\\VisualStudio\\7.0;InstallDir]";
          cmSystemTools::ExpandRegistryValues(mp);
          if (!(mp == "/registry"))
            {
            installedCompiler = "Visual Studio 7";
            }
          else
            {
            installedCompiler = "Visual Studio 6";
            }
          }
        }
      cmGlobalGenerator* gen
        = this->CreateGlobalGenerator(installedCompiler.c_str());
      if(!gen)
        {
        gen = new cmGlobalNMakeMakefileGenerator;
        }
      this->SetGlobalGenerator(gen);
#else
      this->SetGlobalGenerator(new cmGlobalUnixMakefileGenerator3);
#endif
      }
    if(!this->GlobalGenerator)
      {
      cmSystemTools::Error("Could not create generator");
      return -1;
      }
    }

  const char* genName = this->CacheManager->GetCacheValue("CMAKE_GENERATOR");
  if(genName)
    {
    if(strcmp(this->GlobalGenerator->GetName(), genName) != 0)
      {
      std::string message = "Error: generator : ";
      message += this->GlobalGenerator->GetName();
      message += "\nDoes not match the generator used previously: ";
      message += genName;
      message +=
        "\nEither remove the CMakeCache.txt file or choose a different"
        " binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  if(!this->CacheManager->GetCacheValue("CMAKE_GENERATOR"))
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR", 
                                      this->GlobalGenerator->GetName(),
                                      "Name of generator.",
                                      cmCacheManager::INTERNAL);
    }

  // reset any system configuration information, except for when we are
  // InTryCompile. With TryCompile the system info is taken from the parent's
  // info to save time
  if (!this->InTryCompile)
    {
    this->GlobalGenerator->ClearEnabledLanguages();
    }

  this->CleanupWrittenFiles();

  // Truncate log files
  if (!this->InTryCompile)
    {
    this->TruncateOutputLog("CMakeOutput.log");
    this->TruncateOutputLog("CMakeError.log");
    }

  // actually do the configure
  this->GlobalGenerator->Configure();
  
  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:
  // LIBRARY_OUTPUT_PATH
  // EXECUTABLE_OUTPUT_PATH
  if(!this->CacheManager->GetCacheValue("LIBRARY_OUTPUT_PATH"))
    {
    this->CacheManager->AddCacheEntry
      ("LIBRARY_OUTPUT_PATH", "",
       "Single output directory for building all libraries.",
       cmCacheManager::PATH);
    } 
  if(!this->CacheManager->GetCacheValue("EXECUTABLE_OUTPUT_PATH"))
    {
    this->CacheManager->AddCacheEntry
      ("EXECUTABLE_OUTPUT_PATH", "",
       "Single output directory for building all executables.",
       cmCacheManager::PATH);
    }  
  if(!this->CacheManager->GetCacheValue("CMAKE_USE_RELATIVE_PATHS"))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_USE_RELATIVE_PATHS", false,
       "If true, cmake will use relative paths in makefiles and projects.");
    cmCacheManager::CacheIterator it =
      this->CacheManager->GetCacheIterator("CMAKE_USE_RELATIVE_PATHS");
    if ( !it.PropertyExists("ADVANCED") )
      {
      it.SetProperty("ADVANCED", "1");
      }
    }

  if(cmSystemTools::GetFatalErrorOccured() &&
     (!this->CacheManager->GetCacheValue("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(this->CacheManager->
                           GetCacheValue("CMAKE_MAKE_PROGRAM"))))
    {
    // We must have a bad generator selection.  Wipe the cache entry so the
    // user can select another.
    this->CacheManager->RemoveCacheEntry("CMAKE_GENERATOR");
    }
  // only save the cache if there were no fatal errors
  if ( !this->ScriptMode )
    {
    this->CacheManager->SaveCache(this->GetHomeOutputDirectory());
    }
  if ( !this->GraphVizFile.empty() )
    {
    std::cout << "Generate graphviz: " << this->GraphVizFile << std::endl;
    this->GenerateGraphViz(this->GraphVizFile.c_str());
    }
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

bool cmake::CacheVersionMatches()
{
  const char* majv = 
    this->CacheManager->GetCacheValue("CMAKE_CACHE_MAJOR_VERSION");
  const char* minv = 
    this->CacheManager->GetCacheValue("CMAKE_CACHE_MINOR_VERSION");
  const char* relv = 
    this->CacheManager->GetCacheValue("CMAKE_CACHE_RELEASE_VERSION");
  bool cacheSameCMake = false;
  if(majv &&
     atoi(majv) == static_cast<int>(cmMakefile::GetMajorVersion())
     && minv &&
     atoi(minv) == static_cast<int>(cmMakefile::GetMinorVersion())
     && relv && (strcmp(relv, cmMakefile::GetReleaseVersion()) == 0))
    {
    cacheSameCMake = true;
    }
  return cacheSameCMake;
}

void cmake::PreLoadCMakeFiles()
{
  std::string pre_load = this->GetHomeDirectory();
  if ( pre_load.size() > 0 )
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(pre_load.c_str());
      }
    }
  pre_load = this->GetHomeOutputDirectory();
  if ( pre_load.size() > 0 )
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(pre_load.c_str());
      }
    }
}

// handle a command line invocation
int cmake::Run(const std::vector<std::string>& args, bool noconfigure)
{
  // Process the arguments
  this->SetArgs(args);
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    CMakeCommandUsage(args[0].c_str());
    return -1;
    }

  // set the cmake command
  this->CMakeCommand = args[0];
  
  if ( !this->ScriptMode )
    {
    // load the cache
    if(this->LoadCache() < 0)
      {
      cmSystemTools::Error("Error executing cmake::LoadCache(). Aborting.\n");
      return -1;
      }
    }
  else
    {
    this->AddCMakePaths(this->CMakeCommand.c_str());
    }

  // Add any cache args
  if ( !this->SetCacheArgs(args) )
    {
    cmSystemTools::Error("Problem processing arguments. Aborting.\n");
    return -1;
    }

  // In script mode we terminate after running the script.
  if(this->ScriptMode)
    {
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      return -1;
      }
    else
      {
      return 0;
      }
    }

  this->PreLoadCMakeFiles();

  std::string systemFile = this->GetHomeOutputDirectory();
  systemFile += "/CMakeSystem.cmake";

  if ( noconfigure )
    {
    return 0;
    }

  // now run the global generate
  // Check the state of the build system to see if we need to regenerate.
  if(!this->CheckBuildSystem())
    {
    return 0;
    }

  // If we are doing global generate, we better set start and start
  // output directory to the root of the project.
  std::string oldstartdir = this->GetStartDirectory();
  std::string oldstartoutputdir = this->GetStartOutputDirectory();
  this->SetStartDirectory(this->GetHomeDirectory());
  this->SetStartOutputDirectory(this->GetHomeOutputDirectory());
  int ret = this->Configure();
  if (ret || this->ScriptMode)
    {
    return ret;
    }
  ret = this->Generate();
  std::string message = "Build files have been written to: ";
  message += this->GetHomeOutputDirectory();
  this->UpdateProgress(message.c_str(), -1);
  if(ret)
    {
    return ret;
    }
  this->SetStartDirectory(oldstartdir.c_str());
  this->SetStartOutputDirectory(oldstartoutputdir.c_str());

  return ret;
}

int cmake::Generate()
{
  if(!this->GlobalGenerator)
    {
    return -1;
    }
  this->GlobalGenerator->Generate();
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

unsigned int cmake::GetMajorVersion()
{
  return cmMakefile::GetMajorVersion();
}

unsigned int cmake::GetMinorVersion()
{
  return cmMakefile::GetMinorVersion();
}

const char *cmake::GetReleaseVersion()
{
  return cmMakefile::GetReleaseVersion();
}

void cmake::AddCacheEntry(const char* key, const char* value,
                          const char* helpString,
                          int type)
{
  this->CacheManager->AddCacheEntry(key, value, 
                                    helpString,
                                    cmCacheManager::CacheEntryType(type));
}

const char* cmake::GetCacheDefinition(const char* name) const
{
  return this->CacheManager->GetCacheValue(name);
}

int cmake::DumpDocumentationToFile(std::ostream& f)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  // Loop over all registered commands and print out documentation
  const char *name;
  const char *terse;
  const char *full;
  char tmp[1024];
  sprintf(tmp,"Version %d.%d (%s)", cmake::GetMajorVersion(),
          cmake::GetMinorVersion(), cmVersion::GetReleaseVersion().c_str());
  f << "<html>\n";
  f << "<h1>Documentation for commands of CMake " << tmp << "</h1>\n";
  f << "<ul>\n";
  for(RegisteredCommandsMap::iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    name = (*j).second->GetName();
    terse = (*j).second->GetTerseDocumentation();
    full = (*j).second->GetFullDocumentation();
    f << "<li><b>" << name << "</b> - " << terse << std::endl
      << "<br><i>Usage:</i> " << full << "</li>" << std::endl << std::endl;
    }
  f << "</ul></html>\n";
#else
  (void)f;
#endif
  return 1;
}

void cmake::AddDefaultCommands()
{
  std::list<cmCommand*> commands;
  GetBootstrapCommands(commands);
  GetPredefinedCommands(commands);
  for(std::list<cmCommand*>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->AddCommand(*i);
    }
}

void cmake::AddDefaultGenerators()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
# if !defined(CMAKE_BOOT_MINGW)
  this->Generators[cmGlobalVisualStudio6Generator::GetActualName()] =
    &cmGlobalVisualStudio6Generator::New;
  this->Generators[cmGlobalVisualStudio7Generator::GetActualName()] =
    &cmGlobalVisualStudio7Generator::New;
  this->Generators[cmGlobalVisualStudio71Generator::GetActualName()] =
    &cmGlobalVisualStudio71Generator::New;
  this->Generators[cmGlobalVisualStudio8Generator::GetActualName()] =
    &cmGlobalVisualStudio8Generator::New;
  this->Generators[cmGlobalVisualStudio8Win64Generator::GetActualName()] =
    &cmGlobalVisualStudio8Win64Generator::New;
  this->Generators[cmGlobalBorlandMakefileGenerator::GetActualName()] =
    &cmGlobalBorlandMakefileGenerator::New;
  this->Generators[cmGlobalNMakeMakefileGenerator::GetActualName()] =
    &cmGlobalNMakeMakefileGenerator::New;
  this->Generators[cmGlobalWatcomWMakeGenerator::GetActualName()] =
    &cmGlobalWatcomWMakeGenerator::New;
# endif
  this->Generators[cmGlobalMSYSMakefileGenerator::GetActualName()] =
    &cmGlobalMSYSMakefileGenerator::New;
  this->Generators[cmGlobalMinGWMakefileGenerator::GetActualName()] =
    &cmGlobalMinGWMakefileGenerator::New;
#endif
  this->Generators[cmGlobalUnixMakefileGenerator3::GetActualName()] =
    &cmGlobalUnixMakefileGenerator3::New;
#ifdef CMAKE_USE_XCODE
  this->Generators[cmGlobalXCodeGenerator::GetActualName()] =
    &cmGlobalXCodeGenerator::New;
#endif
#ifdef CMAKE_USE_KDEVELOP
  this->Generators[cmGlobalKdevelopGenerator::GetActualName()] =
     &cmGlobalKdevelopGenerator::New;
#endif
}

int cmake::LoadCache()
{
  // could we not read the cache
  if (!this->CacheManager->LoadCache(this->GetHomeOutputDirectory()))
    {
    // if it does exist, but isn;t readable then warn the user
    std::string cacheFile = this->GetHomeOutputDirectory();
    cacheFile += "/CMakeCache.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cmSystemTools::Error(
        "There is a CMakeCache.txt file for the current binary tree but "
        "cmake does not have permission to read it. Please check the "
        "permissions of the directory you are trying to run CMake on.");
      return -1;
      }
    }

  if (this->CMakeCommand.size() < 2)
    {
    cmSystemTools::Error(
      "cmake command was not specified prior to loading the cache in "
      "cmake.cxx");
    return -1;
    }

  // setup CMAKE_ROOT and CMAKE_COMMAND
  if(!this->AddCMakePaths(this->CMakeCommand.c_str()))
    {
    return -3;
    }

  // set the default BACKWARDS compatibility to the current version
  if(!this->CacheManager->GetCacheValue("CMAKE_BACKWARDS_COMPATIBILITY"))
    {
    char ver[256];
    sprintf(ver,"%i.%i",cmMakefile::GetMajorVersion(),
            cmMakefile::GetMinorVersion());
    this->CacheManager->AddCacheEntry
      ("CMAKE_BACKWARDS_COMPATIBILITY",ver, 
       "For backwards compatibility, what version of CMake commands and "
       "syntax should this version of CMake allow.",
       cmCacheManager::STRING);
    }

  return 0;
}

void cmake::SetProgressCallback(ProgressCallbackType f, void *cd)
{
  this->ProgressCallback = f;
  this->ProgressCallbackClientData = cd;
}

void cmake::UpdateProgress(const char *msg, float prog)
{
  if(this->ProgressCallback && !this->InTryCompile)
    {
    (*this->ProgressCallback)(msg, prog, this->ProgressCallbackClientData);
    return;
    }
}

void cmake::GetCommandDocumentation(
  std::vector<cmDocumentationEntry>& v) const
{
  for(RegisteredCommandsMap::const_iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    cmDocumentationEntry e =
      {
        (*j).second->GetName(),
        (*j).second->GetTerseDocumentation(),
        (*j).second->GetFullDocumentation()
      };
    v.push_back(e);
    }
  cmDocumentationEntry empty = {0,0,0};
  v.push_back(empty);
}

void cmake::GetGeneratorDocumentation(std::vector<cmDocumentationEntry>& v)
{
  for(RegisteredGeneratorsMap::const_iterator i = this->Generators.begin();
      i != this->Generators.end(); ++i)
    {
    cmDocumentationEntry e;
    cmGlobalGenerator* generator = (i->second)();
    generator->GetDocumentation(e);
    delete generator;
    v.push_back(e);
    }
  cmDocumentationEntry empty = {0,0,0};
  v.push_back(empty);
}

void cmake::AddWrittenFile(const char* file)
{
  this->WrittenFiles.insert(file);
}

bool cmake::HasWrittenFile(const char* file)
{
  return this->WrittenFiles.find(file) != this->WrittenFiles.end();
}

void cmake::CleanupWrittenFiles()
{
  this->WrittenFiles.clear();
}

void cmake::UpdateConversionPathTable()
{
  // Update the path conversion table with any specified file:
  const char* tablepath = 
    this->CacheManager->GetCacheValue("CMAKE_PATH_TRANSLATION_FILE");

  if(tablepath)
    {
    std::ifstream table( tablepath );
    if(!table)
      {
      cmSystemTools::Error("CMAKE_PATH_TRANSLATION_FILE set to ", tablepath,
        ". CMake can not open file.");
      cmSystemTools::ReportLastSystemError("CMake can not open file.");
      }
    else
      {
      std::string a, b;
      while(!table.eof())
        {
        // two entries per line
        table >> a; table >> b;
        cmSystemTools::AddTranslationPath( a.c_str(), b.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
int cmake::CheckBuildSystem()
{
  // We do not need to rerun CMake.  Check dependency integrity.  Use
  // the make system's VERBOSE environment variable to enable verbose
  // output.
  bool verbose = cmSystemTools::GetEnv("VERBOSE") != 0;
  
  // This method will check the integrity of the build system if the
  // option was given on the command line.  It reads the given file to
  // determine whether CMake should rerun.

  // If no file is provided for the check, we have to rerun.
  if(this->CheckBuildSystemArgument.size() == 0)
    { 
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake no build system arguments\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // If the file provided does not exist, we have to rerun.
  if(!cmSystemTools::FileExists(this->CheckBuildSystemArgument.c_str()))
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake missing file: " 
          << this->CheckBuildSystemArgument.c_str() << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // Read the rerun check file and use it to decide whether to do the
  // global generate.
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile* mf = lg->GetMakefile();
  if(!mf->ReadListFile(0, this->CheckBuildSystemArgument.c_str()) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake error reading : " 
          << this->CheckBuildSystemArgument.c_str() << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    // There was an error reading the file.  Just rerun.
    return 1;
    }

  // Now that we know the generator used to build the project, use it
  // to check the dependency integrity.
  const char* genName = mf->GetDefinition("CMAKE_DEPENDS_GENERATOR");
  if (!genName || genName[0] == '\0')
    {
    genName = "Unix Makefiles";
    }
  cmGlobalGenerator *ggd = this->CreateGlobalGenerator(genName);
  if (ggd)
    {
    // Check the dependencies in case source files were removed.
    std::auto_ptr<cmLocalGenerator> lgd(ggd->CreateLocalGenerator());
    lgd->SetGlobalGenerator(ggd);
    lgd->CheckDependencies(mf, verbose, this->ClearBuildSystem);

    // Check for multiple output pairs.
    ggd->CheckMultipleOutputs(mf, verbose);
    }

  // Get the set of dependencies and outputs.
  const char* dependsStr = mf->GetDefinition("CMAKE_MAKEFILE_DEPENDS");
  const char* outputsStr = mf->GetDefinition("CMAKE_MAKEFILE_OUTPUTS");
  if(!dependsStr || !outputsStr)
    {
    // Not enough information was provided to do the test.  Just rerun.
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake no CMAKE_MAKEFILE_DEPENDS "
        "or CMAKE_MAKEFILE_OUTPUTS :\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }
  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  cmSystemTools::ExpandListArgument(dependsStr, depends);
  cmSystemTools::ExpandListArgument(outputsStr, outputs);

  // If any output is older than any dependency then rerun.
  for(std::vector<std::string>::iterator dep = depends.begin();
      dep != depends.end(); ++dep)
    {
    for(std::vector<std::string>::iterator out = outputs.begin();
        out != outputs.end(); ++out)
      {
      int result = 0;
      if(!this->FileComparison->FileTimeCompare(out->c_str(), 
                                                dep->c_str(), &result) ||
         result < 0)
        {
        if(verbose)
          {
          cmOStringStream msg;
          msg << "Re-run cmake file: " << out->c_str()
              << " older than: " << dep->c_str() << "\n";
          cmSystemTools::Stdout(msg.str().c_str());
          }
        return 1;
        }
      }
    }

  // No need to rerun.
  return 0;
}

//----------------------------------------------------------------------------
void cmake::TruncateOutputLog(const char* fname)
{
  std::string fullPath = this->GetHomeOutputDirectory();
  fullPath += "/";
  fullPath += fname;
  struct stat st;
  if ( ::stat(fullPath.c_str(), &st) )
    {
    return;
    }
  if ( !this->CacheManager->GetCacheValue("CMAKE_CACHEFILE_DIR") )
    {
    cmSystemTools::RemoveFile(fullPath.c_str());
    return;
    }
  size_t fsize = st.st_size;
  const size_t maxFileSize = 50 * 1024;
  if ( fsize < maxFileSize )
    {
    //TODO: truncate file
    return;
    }
}

inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

const char* cmake::GetCTestCommand()
{
  if ( !this->CTestCommand.empty() )
    {
    return this->CTestCommand.c_str();
    }

  cmMakefile* mf
    = this->GetGlobalGenerator()->GetLocalGenerator(0)->GetMakefile();
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->CTestCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
  this->CTestCommand = removeQuotes(this->CTestCommand);
  this->CTestCommand = 
    cmSystemTools::GetFilenamePath(this->CTestCommand.c_str());
  this->CTestCommand += "/";
  this->CTestCommand += "ctest";
  this->CTestCommand += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(this->CTestCommand.c_str()))
    {
    this->CTestCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
    this->CTestCommand = 
      cmSystemTools::GetFilenamePath(this->CTestCommand.c_str());
    this->CTestCommand += "/Debug/";
    this->CTestCommand += "ctest";
    this->CTestCommand += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(this->CTestCommand.c_str()))
    {
    this->CTestCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
    this->CTestCommand = 
      cmSystemTools::GetFilenamePath(this->CTestCommand.c_str());
    this->CTestCommand += "/Release/";
    this->CTestCommand += "ctest";
    this->CTestCommand += cmSystemTools::GetExecutableExtension();
    }
#else
  // Only for bootstrap
  this->CTestCommand += mf->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
  this->CTestCommand += "/ctest";
  this->CTestCommand += cmSystemTools::GetExecutableExtension();
#endif
  if ( this->CTestCommand.empty() )
    {
    cmSystemTools::Error("Cannot find the CTest executable");
    this->CTestCommand = "CTEST-COMMAND-NOT-FOUND";
    }
  return this->CTestCommand.c_str();
}

const char* cmake::GetCPackCommand()
{
  if ( !this->CPackCommand.empty() )
    {
    return this->CPackCommand.c_str();
    }

  cmMakefile* mf
    = this->GetGlobalGenerator()->GetLocalGenerator(0)->GetMakefile();

#ifdef CMAKE_BUILD_WITH_CMAKE
  this->CPackCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
  this->CPackCommand = removeQuotes(this->CPackCommand);
  this->CPackCommand = 
    cmSystemTools::GetFilenamePath(this->CPackCommand.c_str());
  this->CPackCommand += "/";
  this->CPackCommand += "cpack";
  this->CPackCommand += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(this->CPackCommand.c_str()))
    {
    this->CPackCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
    this->CPackCommand = 
      cmSystemTools::GetFilenamePath(this->CPackCommand.c_str());
    this->CPackCommand += "/Debug/";
    this->CPackCommand += "cpack";
    this->CPackCommand += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(this->CPackCommand.c_str()))
    {
    this->CPackCommand = mf->GetRequiredDefinition("CMAKE_COMMAND");
    this->CPackCommand = 
      cmSystemTools::GetFilenamePath(this->CPackCommand.c_str());
    this->CPackCommand += "/Release/";
    this->CPackCommand += "cpack";
    this->CPackCommand += cmSystemTools::GetExecutableExtension();
    }
  if (!cmSystemTools::FileExists(this->CPackCommand.c_str()))
    {
    cmSystemTools::Error("Cannot find the CPack executable");
    this->CPackCommand = "CPACK-COMMAND-NOT-FOUND";
    }
#else
  // Only for bootstrap
  this->CPackCommand += mf->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
  this->CPackCommand += "/cpack";
  this->CPackCommand += cmSystemTools::GetExecutableExtension();
#endif
  return this->CPackCommand.c_str();
}

void cmake::GenerateGraphViz(const char* fileName)
{
  cmGeneratedFileStream str(fileName);
  if ( !str )
    {
    return;
    }
  cmake cm;
  cmGlobalGenerator ggi;
  ggi.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(ggi.CreateLocalGenerator());
  lg->SetGlobalGenerator(&ggi);
  cmMakefile *mf = lg->GetMakefile();

  std::string infile = this->GetHomeOutputDirectory();
  infile += "/CMakeGraphVizOptions.cmake";
  if ( !cmSystemTools::FileExists(infile.c_str()) )
    {
    infile = this->GetHomeDirectory();
    infile += "/CMakeGraphVizOptions.cmake";
    if ( !cmSystemTools::FileExists(infile.c_str()) )
      {
      infile = "";
      }
    }

  if ( !infile.empty() )
    {
    if ( !mf->ReadListFile(0, infile.c_str()) )
      {
      cmSystemTools::Error("Problem opening GraphViz options file: ",
        infile.c_str());
      return;
      }
    std::cout << "Read GraphViz options file: " << infile.c_str()
      << std::endl;
    }

#define __set_if_not_set(var, value, cmakeDefinition) \
  const char* var = mf->GetDefinition(cmakeDefinition); \
  if ( !var ) \
    { \
    var = value; \
    }
  __set_if_not_set(graphType, "digraph", "GRAPHVIZ_GRAPH_TYPE");
  __set_if_not_set(graphName, "GG", "GRAPHVIZ_GRAPH_NAME");
  __set_if_not_set(graphHeader, "node [\n  fontsize = \"12\"\n];",
    "GRAPHVIZ_GRAPH_HEADER");
  __set_if_not_set(graphNodePrefix, "node", "GRAPHVIZ_NODE_PREFIX");
  const char* ignoreTargets = mf->GetDefinition("GRAPHVIZ_IGNORE_TARGETS");
  std::set<cmStdString> ignoreTargetsSet;
  if ( ignoreTargets )
    {
    std::vector<std::string> ignoreTargetsVector;
    cmSystemTools::ExpandListArgument(ignoreTargets,ignoreTargetsVector);
    std::vector<std::string>::iterator itvIt;
    for ( itvIt = ignoreTargetsVector.begin();
      itvIt != ignoreTargetsVector.end();
      ++ itvIt )
      {
      ignoreTargetsSet.insert(itvIt->c_str());
      }
    }

  str << graphType << " " << graphName << " {" << std::endl;
  str << graphHeader << std::endl;

  cmGlobalGenerator* gg = this->GetGlobalGenerator();
  std::vector<cmLocalGenerator*> localGenerators;
  gg->GetLocalGenerators(localGenerators);
  std::vector<cmLocalGenerator*>::iterator lit;
  // for target deps
  // 1 - cmake target
  // 2 - external target
  // 0 - no deps
  std::map<cmStdString, int> targetDeps;
  std::map<cmStdString, cmTarget*> targetPtrs;
  std::map<cmStdString, cmStdString> targetNamesNodes;
  int cnt = 0;
  // First pass get the list of all cmake targets
  for ( lit = localGenerators.begin(); lit != localGenerators.end(); ++ lit )
    {
    cmTargets* targets = &((*lit)->GetMakefile()->GetTargets());
    cmTargets::iterator tit;
    for ( tit = targets->begin(); tit != targets->end(); ++ tit )
      {
      const char* realTargetName = tit->first.c_str();
      if ( ignoreTargetsSet.find(realTargetName) != ignoreTargetsSet.end() )
        {
        // Skip ignored targets
        continue;
        }
      //std::cout << "Found target: " << tit->first.c_str() << std::endl;
      cmOStringStream ostr;
      ostr << graphNodePrefix << cnt++;
      targetNamesNodes[realTargetName] = ostr.str();
      targetPtrs[realTargetName] = &tit->second;
      }
    }
  // Ok, now find all the stuff we link to that is not in cmake
  for ( lit = localGenerators.begin(); lit != localGenerators.end(); ++ lit )
    {
    cmTargets* targets = &((*lit)->GetMakefile()->GetTargets());
    cmTargets::iterator tit;
    for ( tit = targets->begin(); tit != targets->end(); ++ tit )
      {
      const cmTarget::LinkLibraryVectorType* ll
        = &(tit->second.GetOriginalLinkLibraries());
      cmTarget::LinkLibraryVectorType::const_iterator llit;
      const char* realTargetName = tit->first.c_str();
      if ( ignoreTargetsSet.find(realTargetName) != ignoreTargetsSet.end() )
        {
        // Skip ignored targets
        continue;
        }
      if ( ll->size() > 0 )
        {
        targetDeps[realTargetName] = 1;
        }
      for ( llit = ll->begin(); llit != ll->end(); ++ llit )
        {
        const char* libName = llit->first.c_str();
        std::map<cmStdString, cmStdString>::iterator tarIt
          = targetNamesNodes.find(libName);
        if ( ignoreTargetsSet.find(libName) != ignoreTargetsSet.end() )
          {
          // Skip ignored targets
          continue;
          }
        if ( tarIt == targetNamesNodes.end() )
          {
          cmOStringStream ostr;
          ostr << graphNodePrefix << cnt++;
          targetDeps[libName] = 2;
          targetNamesNodes[libName] = ostr.str();
          //str << "    \"" << ostr.c_str() << "\" [ label=\"" << libName
          //<<  "\" shape=\"ellipse\"];" << std::endl;
          }
        else
          {
          std::map<cmStdString, int>::iterator depIt
            = targetDeps.find(libName);
          if ( depIt == targetDeps.end() )
            {
            targetDeps[libName] = 1;
            }
          }
        }
      }
    }

  // Write out nodes
  std::map<cmStdString, int>::iterator depIt;
  for ( depIt = targetDeps.begin(); depIt != targetDeps.end(); ++ depIt )
    {
    const char* newTargetName = depIt->first.c_str();
    std::map<cmStdString, cmStdString>::iterator tarIt
      = targetNamesNodes.find(newTargetName);
    if ( tarIt == targetNamesNodes.end() )
      {
      // We should not be here.
      std::cout << __LINE__ << " Cannot find library: " << newTargetName
        << " even though it was added in the previous pass" << std::endl;
      abort();
      }

    str << "    \"" << tarIt->second.c_str() << "\" [ label=\""
      << newTargetName <<  "\" shape=\"";
    if ( depIt->second == 1 )
      {
      std::map<cmStdString, cmTarget*>::iterator tarTypeIt= targetPtrs.find(
        newTargetName);
      if ( tarTypeIt == targetPtrs.end() )
        {
        // We should not be here.
        std::cout << __LINE__ << " Cannot find library: " << newTargetName
          << " even though it was added in the previous pass" << std::endl;
        abort();
        }
      cmTarget* tg = tarTypeIt->second;
      switch ( tg->GetType() )
        {
      case cmTarget::EXECUTABLE:
        str << "house";
        break;
      case cmTarget::STATIC_LIBRARY:
        str << "diamond";
        break;
      case cmTarget::SHARED_LIBRARY:
        str << "polygon";
        break;
      case cmTarget::MODULE_LIBRARY:
        str << "octagon";
        break;
      default:
        str << "box";
        }
      }
    else
      {
      str << "ellipse";
      }
    str << "\"];" << std::endl;
    }

  // Now generate the connectivity
  for ( lit = localGenerators.begin(); lit != localGenerators.end(); ++ lit )
    {
    cmTargets* targets = &((*lit)->GetMakefile()->GetTargets());
    cmTargets::iterator tit;
    for ( tit = targets->begin(); tit != targets->end(); ++ tit )
      {
      std::map<cmStdString, int>::iterator dependIt
        = targetDeps.find(tit->first.c_str());
      if ( dependIt == targetDeps.end() )
        {
        continue;
        }
      std::map<cmStdString, cmStdString>::iterator cmakeTarIt
        = targetNamesNodes.find(tit->first.c_str());
      const cmTarget::LinkLibraryVectorType* ll
        = &(tit->second.GetOriginalLinkLibraries());
      cmTarget::LinkLibraryVectorType::const_iterator llit;
      for ( llit = ll->begin(); llit != ll->end(); ++ llit )
        {
        const char* libName = llit->first.c_str();
        std::map<cmStdString, cmStdString>::iterator tarIt
          = targetNamesNodes.find(libName);
        if ( tarIt == targetNamesNodes.end() )
          {
          // We should not be here.
          std::cout << __LINE__ << " Cannot find library: " << libName
            << " even though it was added in the previous pass" << std::endl;
          abort();
          }
        str << "    \"" << cmakeTarIt->second.c_str() << "\" -> \""
          << tarIt->second.c_str() << "\"" << std::endl;
        }
      }
    }

  // TODO: Use dotted or something for external libraries
  //str << "    \"node0\":f4 -> \"node12\"[color=\"#0000ff\" style=dotted]"
  //<< std::endl;
  //
  str << "}" << std::endl;
}

//----------------------------------------------------------------------------
#ifdef CMAKE_BUILD_WITH_CMAKE
int cmake::ExecuteEchoColor(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_echo_color

  // On some platforms (an MSYS prompt) cmsysTerminal may not be able
  // to determine whether the stream is displayed on a tty.  In this
  // case it assumes no unless we tell it otherwise.  Since we want
  // color messages to be displayed for users we will assume yes.
  // However, we can test for some situations when the answer is most
  // likely no.
  int assumeTTY = cmsysTerminal_Color_AssumeTTY;
  if(cmSystemTools::GetEnv("DART_TEST_FROM_DART") ||
     cmSystemTools::GetEnv("DASHBOARD_TEST_FROM_CTEST"))
    {
    // Avoid printing color escapes during dashboard builds.
    assumeTTY = 0;
    }

  bool enabled = true;
  int color = cmsysTerminal_Color_Normal;
  bool newline = true;
  for(unsigned int i=2; i < args.size(); ++i)
    {
    if(args[i].find("--switch=") == 0)
      {
      // Enable or disable color based on the switch value.
      std::string value = args[i].substr(9);
      if(!value.empty())
        {
        if(cmSystemTools::IsOn(value.c_str()))
          {
          enabled = true;
          }
        else
          {
          enabled = false;
          }
        }
      }
    else if(args[i] == "--normal")
      {
      color = cmsysTerminal_Color_Normal;
      }
    else if(args[i] == "--black")
      {
      color = cmsysTerminal_Color_ForegroundBlack;
      }
    else if(args[i] == "--red")
      {
      color = cmsysTerminal_Color_ForegroundRed;
      }
    else if(args[i] == "--green")
      {
      color = cmsysTerminal_Color_ForegroundGreen;
      }
    else if(args[i] == "--yellow")
      {
      color = cmsysTerminal_Color_ForegroundYellow;
      }
    else if(args[i] == "--blue")
      {
      color = cmsysTerminal_Color_ForegroundBlue;
      }
    else if(args[i] == "--magenta")
      {
      color = cmsysTerminal_Color_ForegroundMagenta;
      }
    else if(args[i] == "--cyan")
      {
      color = cmsysTerminal_Color_ForegroundCyan;
      }
    else if(args[i] == "--white")
      {
      color = cmsysTerminal_Color_ForegroundWhite;
      }
    else if(args[i] == "--bold")
      {
      color |= cmsysTerminal_Color_ForegroundBold;
      }
    else if(args[i] == "--no-newline")
      {
      newline = false;
      }
    else if(args[i] == "--newline")
      {
      newline = true;
      }
    else if(enabled)
      {
      // Color is enabled.  Print with the current color.
      cmsysTerminal_cfprintf(color | assumeTTY, stdout, "%s%s",
                             args[i].c_str(), newline? "\n" : "");
      }
    else
      {
      // Color is disabled.  Print without color.
      fprintf(stdout, "%s%s", args[i].c_str(), newline? "\n" : "");
      }
    }

  return 0;
}
#else
int cmake::ExecuteEchoColor(std::vector<std::string>&)
{
  return 1;
}
#endif

//----------------------------------------------------------------------------
int cmake::ExecuteLinkScript(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_link_script
  //   argv[2] == <link-script-name>
  //   argv[3] == --verbose=?
  bool verbose = false;
  if(args.size() >= 4)
    {
    if(args[3].find("--verbose=") == 0)
      {
      if(!cmSystemTools::IsOff(args[3].substr(10).c_str()))
        {
        verbose = true;
        }
      }
    }

  // Allocate a process instance.
  cmsysProcess* cp = cmsysProcess_New();
  if(!cp)
    {
    std::cerr << "Error allocating process instance in link script."
              << std::endl;
    return 1;
    }

  // Children should share stdout and stderr with this process.
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);

  // Run the command lines verbatim.
  cmsysProcess_SetOption(cp, cmsysProcess_Option_Verbatim, 1);

  // Read command lines from the script.
  std::ifstream fin(args[2].c_str());
  if(!fin)
    {
    std::cerr << "Error opening link script \""
              << args[2] << "\"" << std::endl;
    return 1;
    }

  // Run one command at a time.
  std::string command;
  int result = 0;
  while(result == 0 && cmSystemTools::GetLineFromStream(fin, command))
    {
    // Setup this command line.
    const char* cmd[2] = {command.c_str(), 0};
    cmsysProcess_SetCommand(cp, cmd);

    // Report the command if verbose output is enabled.
    if(verbose)
      {
      std::cout << command << std::endl;
      }

    // Run the command and wait for it to exit.
    cmsysProcess_Execute(cp);
    cmsysProcess_WaitForExit(cp, 0);

    // Report failure if any.
    switch(cmsysProcess_GetState(cp))
      {
      case cmsysProcess_State_Exited:
        {
        int value = cmsysProcess_GetExitValue(cp);
        if(value != 0)
          {
          result = value;
          }
        }
        break;
      case cmsysProcess_State_Exception:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetExceptionString(cp) << std::endl;
        result = 1;
        break;
      case cmsysProcess_State_Error:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetErrorString(cp) << std::endl;
        result = 2;
        break;
      default:
        break;
      };
    }

  // Free the process instance.
  cmsysProcess_Delete(cp);

  // Return the final resulting return value.
  return result;
}
